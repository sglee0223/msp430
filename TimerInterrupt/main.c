//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  Texas Instruments, Inc
//  July 2013
//***************************************************************************************

#include <msp430.h>

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerA0_ISR (void)
{
    P1OUT ^= BIT0;                   // Toggle P1.0
}

void gpioInit(void)
{
    P1OUT &= 0x00;                  // Shut down everything
    P1DIR &= 0x00;
    P2OUT &= 0x00;
    P2DIR &= 0x00;
    P3OUT &= 0x00;
    P3DIR &= 0x00;
    P4OUT &= 0x00;
    P4DIR &= 0x00;
    P5OUT &= 0x00;
    P5DIR &= 0x00;
    P6OUT &= 0x00;
    P6DIR &= 0x00;
    P7OUT &= 0x00;
    P7DIR &= 0x00;
    P8OUT &= 0x00;
    P8DIR &= 0x00;
    P9OUT &= 0x00;
    P9DIR &= 0x00;
    P10OUT &= 0x00;
    P10DIR &= 0x00;
}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;           // Disable the GPIO power-on default high-impedance mode
                                    // to activate previously configured port settings

    gpioInit();

    P1DIR |= BIT0;                  // Set P1.0 to output direction

    TA0CCTL0 = CCIE;                   // CCR0 interrupt enabled
    TA0CTL = TASSEL_2 + MC_1 + ID_0; // SMCLK / 1 = 1 MHz / 1 = 1 MHz = 0.001 ms, upmode
    TA0CCR0 =  1000; // 1ms

    _BIS_SR(LPM0_bits + GIE);          // Enter LPM0 w/ interrupt
}
