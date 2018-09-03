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

#if 1

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void)
{
    if (P1IFG & BIT1)
    {
        P1IFG &= ~BIT1;                       //Clear Pin 1.1 flag
        P1OUT ^= BIT0;                   // Toggle P1.0
    }
}

#else

// next isr is working well
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    switch(__even_in_range(P1IV, P1IV_P1IFG7))
    {
        case P1IV_NONE : break;
        case P1IV_P1IFG0 : break;
        case P1IV_P1IFG1 :
            P1OUT ^= BIT0;                   // Toggle P1.0
            break;
        case P1IV_P1IFG3 : break;
        case P1IV_P1IFG4 : break;
        case P1IV_P1IFG5 : break;
        case P1IV_P1IFG6 : break;
        case P1IV_P1IFG7 : break;
    }
}

#endif

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
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    gpioInit();

    P1DIR |= BIT0;                  // Set P1.0 to output direction

    P1DIR &= ~BIT1;                 // Set P1.1 to input
    P1REN |= BIT1;                  // Enable internal pull-up/down resistors
    P1OUT |= BIT1;                  //Select pull-up mode for P1.1

    P1IES |= BIT1;                  //Have flag set on High to Low
    P1IE = BIT1;                    //enable interrupts for Pin 1.1
    P1IFG &= ~BIT1;                 //Clear Pin 1.1 flag

    _BIS_SR(LPM0_bits + GIE);          // Enter LPM0 w/ interrupt
}
