/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//   MSP430FR69xx Demo - Configure MCLK for 8MHz operation
//
//   Description: Configure SMCLK = MCLK = 8MHz, ACLK = VLOCLK.
//
//           MSP430FR6989
//         ---------------
//     /|\|               |
//      | |               |
//      --|RST            |
//        |               |
//        |           P1.0|---> LED
//        |           P5.2|---> ACLK = ~9.4kHz
//        |           P7.4|---> SMCLK = MCLK = 8MHz
//
//   William Goh
//   Texas Instruments Inc.
//   April 2014
//   Built with IAR Embedded Workbench V5.60 & Code Composer Studio V6.0
//******************************************************************************
#include <msp430.h>

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

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

    gpioInit();

    // Configure GPIO
    P1OUT &= ~BIT0;                           // Clear P1.0 output latch for a defined power-on state
    P1DIR |= BIT0;                            // Set P1.0 to output direction

    //P5DIR |= BIT2;
    //P5SEL0 |= BIT2;                           // Output ACLK
    //P5SEL1 |= BIT2;

    //P7DIR |= BIT4;
    //P7SEL1 |= BIT4;                           // Output SMCLK
    //P7SEL0 |= BIT4;

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Clock System Setup
    CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
    CSCTL1 = DCOFSEL_6;                       // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;  // Set SMCLK = MCLK = DCO
    // ACLK = VLOCLK
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers to 1
    CSCTL0_H = 0;                             // Lock CS registers

    while (1)
    {
        P1OUT ^= BIT0;                          // Toggle LED
        __delay_cycles(8000);                // Wait 8,000 CPU Cycles, 1ms
    }
}
