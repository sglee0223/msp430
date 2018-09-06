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

#include "myclock.h"
#include "myprintf.h"

#define LED0_OUT     P1OUT
#define LED0_DIR     P1DIR
#define LED0_PIN     BIT1

int main(void)
{
    unsigned int i;
    unsigned char myClock = CLOCK_16M;
    unsigned char uartCH = USCI_A1;
    unsigned char uartBR = BR_115200;

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

	clock_init(myClock);

    // Configure GPIO
    LED0_OUT &= ~(LED0_PIN); // P1 setup for LED & reset output
    LED0_DIR |= (LED0_PIN);

    uart_gpio_init(uartCH);

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

#ifdef USE_I2C_INTERFACE
    i2c_interface_init();
#endif
#ifdef USE_I2C_GPIO
    I2C_Gpio_Init();
#endif

    uart_baudrate_init(myClock, uartCH, uartBR);
	
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
