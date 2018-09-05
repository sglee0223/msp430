//******************************************************************************
//   MSP430FR69xx Demo - eUSCI_A0, SPI 4-Wire Master multiple byte RX/TX
//
//   Description: SPI master communicates to SPI slave sending and receiving
//   3 different messages of different length. SPI master will enter LPM0 mode
//   while waiting for the messages to be sent/receiving using SPI interrupt.
//   SPI Master will initially wait for a port interrupt in LPM0 mode before
//   starting the SPI communication.
//   ACLK = NA, MCLK = SMCLK = DCO 16MHz.
//
//
//                   MSP430FR6989
//                 -----------------
//            /|\ |             P1.1|-> Comms LED
//             |  |                 |
//             ---|RST          P1.6|-> Slave Reset (GPIO)
//                |                 |
//                |             P2.0|-> Data Out (UCA0SIMO)
//                |                 |
//       Button ->|P1.3         P2.1|<- Data In (UCA0SOMI)
//   Button LED <-|P1.0             |
//                |             P1.5|-> Serial Clock Out (UCA0CLK)
//                |                 |
//                |             P1.4|-> Slave Chip Select (GPIO)
//
//   Nima Eskandari and Ryan Meredith
//   Texas Instruments Inc.
//   January 2018
//   Built with CCS V7.3
//******************************************************************************

#include <msp430.h>
#include <stdint.h>

#include "spi_gpio.h"
#include "spi_interface.h"
#include "myprintf.h"

#include "at45dbxx.h"

#define LED0_OUT   P1OUT
#define LED0_DIR   P1DIR
#define LED0_PIN   BIT0

void initGPIO()
{
    //LEDs
    LED0_DIR |= LED0_PIN;
    LED0_OUT &= ~LED0_PIN;

#ifdef USE_SPI_INTERFACE
    // Configure SPI
    //P1SEL0 &= ~(BIT5 | BIT4);                 // Configure slave select and clk
    //P1SEL1 |= BIT5 | BIT4;
    P1SEL0 &= ~(BIT5);                 // Configure slave select and clk
    P1SEL1 |= BIT5;

    P2SEL0 |= BIT0 | BIT1;                    // Configure SOMI and MISO
    P2SEL1 &= ~(BIT0 | BIT1);

    SPI_RST_DIR |= SPI_RST_PIN;
    SPI_RST_OUT |= SPI_RST_PIN;

    SPI_CS_DIR |= SPI_CS_PIN;
    SPI_CS_OUT |= SPI_CS_PIN;
#endif

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}

void initClockTo16MHz()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System Setup
    CSCTL0_H = CSKEY_H;                       // Unlock CS registers
    CSCTL1 = DCOFSEL_4 | DCORSEL;             // Set DCO to 16MHz
    CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK; // Set SMCLK = MCLK = DCO,
                                              // ACLK = LFXTCLK (VLOCLK if unavailable)
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
    CSCTL0_H = 0;                             // Lock CS registers
}

int main(void)
{
    unsigned int i;
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    initClockTo16MHz();
    initGPIO();
    initSPI();

    uart_init();

    myprintf("\r\n\r\nat45dbxx program start\r\n");

    SPI_RST_OUT &= ~SPI_RST_PIN;        // Now with SPI signals initialized,
    __delay_cycles(16000); // 1ms
    SPI_RST_OUT |= SPI_RST_PIN;         // reset slave
    __delay_cycles(16000);                 // Wait for slave to initialize

    LED0_OUT |= LED0_PIN;

    for (i=0; i<30; i++)
        __delay_cycles(16000); // 1ms

    at45db_initialize();
    at45db_test();

    __bis_SR_register(LPM0_bits + GIE);
    __no_operation();

    return 0;
}

