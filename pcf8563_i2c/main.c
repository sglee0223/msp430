//******************************************************************************
//   MSP430FR69xx Demo - eUSCI_B0, I2C Master multiple byte TX/RX
//
//   Description: I2C master communicates to I2C slave sending and receiving
//   3 different messages of different length. I2C master will enter LPM0 mode
//   while waiting for the messages to be sent/receiving using I2C interrupt.
//   ACLK = NA, MCLK = SMCLK = DCO 16MHz.
//
//                                     /|\ /|\
//                   MSP430FR6989      4.7k |
//                 -----------------    |  4.7k
//            /|\ |             P1.7|---+---|-- I2C Clock (UCB0SCL)
//             |  |                 |       |
//             ---|RST          P1.6|-------+-- I2C Data (UCB0SDA)
//                |                 |
//                |                 |
//                |                 |
//                |                 |
//                |                 |
//                |                 |
//
//   Nima Eskandari and Ryan Meredith
//   Texas Instruments Inc.
//   January 2018
//   Built with CCS V7.3
//******************************************************************************

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "i2c_interface.h"
#include "i2c_gpio.h"
#include "pcf8563.h"
#include "myprintf.h"

#define LED0_OUT     P1OUT
#define LED0_DIR     P1DIR
#define LED0_PIN    BIT1

void initGPIO()
{
    // Configure GPIO
    LED0_OUT &= ~(LED0_PIN); // P1 setup for LED & reset output
    LED0_DIR |= (LED0_PIN);

    // UART pins
    P3SEL0 |= BIT4 | BIT5;                    // USCI_A1 UART operation
    P3SEL1 &= ~(BIT4 | BIT5);

#ifdef USE_I2C_INTERFACE
    // I2C pins
    P1SEL0 |= BIT6 | BIT7;
    P1SEL1 &= ~(BIT6 | BIT7);

    //P1REN |= 0x00C0; // P1.6 & P1.7 Pullups
    //P1OUT |= 0x00C0; // P1.6 & P1.7 Pullups
#endif

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}

void initClockTo8MHz()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Startup clock system with max DCO setting ~8MHz
    CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
    CSCTL0_H = 0;                             // Lock CS registers
}

void initClockTo16MHz()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System Setup
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz

    // Set SMCLK = MCLK = DCO, ACLK = LFXTCLK (VLOCLK if unavailable)
    CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK;

    // Per Device Errata set divider to 4 before changing frequency to
    // prevent out of spec operation from overshoot transient
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;   // Set all corresponding clk sources to divide by 4 for errata
    CSCTL1 = DCOFSEL_4 | DCORSEL;           // Set DCO to 16MHz

    // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
    __delay_cycles(60);
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1 for 16MHz operation
    CSCTL0_H = 0;                           // Lock CS registers
}

#ifdef USE_I2C_INTERFACE
void initI2C()
{
    UCB0CTLW0 = UCSWRST;                      // Enable SW reset
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK | UCSYNC; // I2C master mode, SMCLK
    UCB0BRW = 160;                            // fSCL = SMCLK/160 = ~100kHz
    UCB0I2CSA = RTC_ADDR << 1;                   // Slave Address
    UCB0CTLW0 &= ~UCSWRST;                    // Clear SW reset, resume operation
    UCB0IE |= UCNACKIE;
}
#endif

int main(void)
{
    unsigned int i;

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    //initClockTo8MHz();
    initClockTo16MHz();

    initGPIO();
#ifdef USE_I2C_INTERFACE
    initI2C();
#endif
#ifdef USE_I2C_GPIO
    I2C_Init();
#endif

    uart_init();
    myprintf("pcf8563 Program Start\r\n");

    RTC_Init();
    PCF8563_setDate(1999, 12, 31);
    PCF8563_setTime(23, 59, 50);

    while (1)
    {
        for(i=0; i<999; i++)
            __delay_cycles(16000);

        PCF8563_getDate();
        PCF8563_getTime();
    }
}
