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
//  MSP430FR69xx Demo - eUSCI_A1 UART echo at 9600 baud using BRCLK = 8MHz
//
//  Description: This demo echoes back characters received via a PC serial port.
//  SMCLK/ DCO is used as a clock source and the device is put in LPM3
//  The auto-clock enable feature is used by the eUSCI and SMCLK is turned off
//  when the UART is idle and turned on when a receive edge is detected.
//  Note that level shifter hardware is needed to shift between RS232 and MSP
//  voltage levels.
//
//  The example code shows proper initialization of registers
//  and interrupts to receive and transmit data.
//  To test code in LPM3, disconnect the debugger.
//
//  ACLK = VLO, MCLK =  DCO = SMCLK = 8MHz
//
//                MSP430FR6989
//             -----------------
//       RST -|     P3.4/UCA1TXD|----> PC (echo)
//            |                 |
//            |                 |
//            |     P3.5/UCA1RXD|<---- PC
//            |                 |
//
//   William Goh
//   Texas Instruments Inc.
//   April 2014
//   Built with IAR Embedded Workbench V5.60 & Code Composer Studio V6.0
//******************************************************************************
#include <msp430.h>

#include <stdio.h>

#include "myprintf.h"
#include "myclock.h"

typedef struct _uartArg {
    unsigned char br0[3][2];
    unsigned char br1[3][2];
    unsigned long mctlw[3][2];
} uartArg;

uartArg uArg;

/*-------------------------------------------------------------------
DESCRIPTION: Send one char in TX buffer, if it is not busy. Wait until not busy.
INPUTS:      One char.
OUTPUTS:     Send all the char in TX buffer.
RETURNS:     None.
---------------------------------------------------------------------*/

// Modify this routine so that it points to YOUR UART (zeke)
void putChar(unsigned char byte)
{
    while(!(UCA1IFG & UCTXIFG));    // USCI_A0 TX buffer ready?
    UCA1TXBUF = byte;                    // Load Tx register that clear UCA0TXIFG
}

/*-------------------------------------------------------------------
DESCRIPTION: Move numbers of lines up in the HyperTerminal.
INPUTS:      None.
OUTPUTS:     Line up to TX buffer.
RETURNS:     None.
---------------------------------------------------------------------*/

void linesUp(unsigned char lines)
{
    unsigned char i;
    for (i = 0; i < lines; ++i)
    {
        putChar(0x1b);
        putChar(0x5b);
        putChar(0x41);
    }
}

/*-------------------------------------------------------------------
DESCRIPTION: Send out charater strings with defined width, justification
                      and padding. Width = 0 or width < string length means
                      unlimited width. Normal padding is space and left justification,
                      but it can pad '0' or pad to the right side, depending on pad value.
                                                             pad                        justification        padding char
                                                      bxxxxxx00            left                                     ' '
                                                      bxxxxxx1x            right                                    ' ' or '0'
                                                      bxxxxxxx1            left or right        '0'
INPUTS:      Valid string and special charater in form of "\n" for example
                                               refered by pointer *string. Output field width. Justification
                                               and padding flag pad.
OUTPUTS:     Sent formated string to com port output.
RETURNS:     Total of chars sent.
---------------------------------------------------------------------*/

#define PAD_RIGHT    0x01
#define PAD_ZERO           0x02

int prints(char *string, unsigned char width, unsigned char pad)
{
    int pc = 0;
    unsigned char padchar = ' ';                           // The pading char is space normally

    if (width > 0)                                                                           // If output width is defined
    {
        unsigned char len = 0;
        char *ptr;
        for (ptr = string; *ptr; ++ptr) ++len;      // Calculate string length and put it in len
        if (len >= width) width = 0;    // If string is longer than width, then width is not applicable define as zero
        else width -= len;              // Else redefine width as padding spaces
        if (pad & PAD_ZERO) padchar = '0';      // If padding char is zero, then get padchar as zero ready instead of original space
    }
    if (!(pad & PAD_RIGHT))                                // If not right padding - left justification
    {
        for (; width > 0; --width)               // If ther is padding width. Output padding char as '0' or ' '.
        {
            putChar (padchar);
            ++pc;
        }
    }
    for (; *string ; ++string)                      // Output the full string
    {
        putChar (*string);
        ++pc;
    }
    for (; width > 0; --width) {      // Write padding char to the right if normal left justification
        putChar (padchar);
        ++pc;
    }
    return pc;                                                                 // Return the output char number
}

/*-------------------------------------------------------------------
 * DESCRIPTION: Print 32 bit signed interger in dec or hex. In specific
 *                   width, padding and justification using prints(). Use 12 byte buffer
 *                   which is enough for 32 bit int.
 * INPUTS: Up to 32 byte signed interger i. Counting base: 10 or 16.
 *                   Sign flag sg. Output string width. padding and justification flag.
 *                   Leter base for number conversion.
 * OUTPUTS:     Sent formated interger as string to com port output.
 * RETURNS:     Total of chars sent.
---------------------------------------------------------------------*/

#define PRINT_BUF_LEN 12

int printi(long int i, unsigned char b, unsigned char sg, unsigned char width, unsigned char pad, unsigned char letbase)
{
    char print_buf[PRINT_BUF_LEN];                         // Interger as string array
    char *s;
    char neg = 0;
    unsigned long int t;
    unsigned long int u = i;
    int pc = 0;

    if (i == 0)                                                                       // If output char is 0, then just output it with padding and width.
    {
        print_buf[0] = '0';
        print_buf[1] = '\0';                     // Always remenber to put string end
        return prints(print_buf, width, pad);           //Print out zero and done.
    }

    if (sg && (b == 10) && (i < 0))                 // If it is a negative int, then record the '-' and number as positive
    {
        neg = 1;
        u = -i;
    }

    s = print_buf + PRINT_BUF_LEN-1;  // Point s to the end of the output buffer and put a null there.
    *s = '\0';

    while (u)                                // Convert the positive int to string with whatever counting base, dec, or hex.
    {
        t = u % b;
        if( t >= 10 )
            t += letbase - '0' - 10;
        *--s = t + '0';
        u /= b;
    }

    if (neg)
    {                          // If it is a negative number
        if( width && (pad & PAD_ZERO) )
        {     // If there is width, right justified and pad with zero, output negative sign.
            putChar ('-');
            ++pc;
            --width;
        }
        else *--s = '-';                  // Otherwise put the '-' to string buffer.

    }
    return pc + prints (s, width, pad);          // Output the string buffer and return the output counter.
}

/*-------------------------------------------------------------------
 * DESCRIPTION: short form of printf. Print argument strings with mixed
 *                                              varables (string or interger)inside formated.
 * INPUTS:      Argument string pointer.
 * OUTPUTS:     print out the argument with style using prints() and printi().
 * RETURNS:     Total of chars sent.
 * Warning!!!        varables and constant numbers even 0, must casted with
 *                         (long int)in printf(), if it is going to print out using
 *                         format "u", "d", "X" and "x"! Or the complier will assigned
 *                         16-bit for data smaller than 16 bit and the argument pointer
 *                         will fetch a wrong 32-bit data and the argument point
 *                         increament will be in wrong size.
 * Limitations:      1) It treats all interger as 32 bit data only.
 *                         2) No floating point data presentation.
 *                         3) Has left/right alignment with 0 padding.
 *                         4) Has format code "s", "d", "X", "x", "u" and "c" only.
---------------------------------------------------------------------*/

int myprintf(char *format, ...)
{
    int width, pad;
    int pc = 0;
    char scr[2];
    va_list args;
    va_start(args, format);

    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            width = pad = 0;
            if (*format == '\0') break;
            if (*format == '%') goto out;
            if (*format == '-') {
                ++format;
                pad = PAD_RIGHT;
            }
            while (*format == '0') {
                ++format;
                pad |= PAD_ZERO;
            }
            for ( ; *format >= '0' && *format <= '9'; ++format) {
                width *= 10;
                width += *format - '0';
            }
            if( *format == 's' ) {
                char *s = (char *)va_arg( args, int );
                pc += prints (s?s:"(null)", width, pad);
                continue;
            }
            if( *format == 'd' ) {
                pc += printi (va_arg( args, long int ), 10, 1, width, pad, 'a');
                continue;
            }
            if( *format == 'x' ) {
                pc += printi (va_arg( args, long int ), 16, 0, width, pad, 'a');
                continue;
            }
            if( *format == 'X' ) {
                pc += printi (va_arg( args, long int ), 16, 0, width, pad, 'A');
                continue;
            }
            if( *format == 'u' ) {
                pc += printi (va_arg( args, long int ), 10, 0, width, pad, 'a');
                continue;
            }
            if( *format == 'c' ) {                                 // char are converted to int then pushed on the stack
                scr[0] = (char)va_arg( args, int );
                scr[1] = '\0';
                pc += prints (scr, width, pad);
                continue;
            }
        }
        else {
            out:
            putChar(*format);
            ++pc;
        }
    }
    va_end( args );
    return pc;
}

void printTest(void)
{
    char *ptr = "Hello world!";
    char *np = 0;
    long int i = 5;
    int bs = sizeof(long int)*8;                           // Bit to shift
    long int mi = ((long int)1 << (bs-1)) + 1;             // Maximum negative number

    myprintf("%s\r\n", ptr);
    myprintf("printf test\r\n");
    myprintf("%s is null pointer\r\n", np);
    myprintf("%d = 5\r\n", (long int)i);
    myprintf("%d = - max int\r\n", mi);
    myprintf("Long int 123456789 print out is %u", (long int)123456789);
    myprintf("\r\nmi in hex is %x\r\n", (long int)mi);
    myprintf("bs in dec is %u\r\n", (long int)bs);
    myprintf("char %c = 'a'\r\n", 'a');
    myprintf("hex %x = ff\r\n", (long int)0xff);
    myprintf("hex %02x = 00\r\n", (long int)0);
    myprintf("signed %d = unsigned %u = hex %x\r\n", (long int)-32767, (long int)-32767, (long int)-32767);
    myprintf("signed %d = unsigned %u = hex %x\r\n", (long int)-3, (long int)-3, (long int)-3);
    myprintf("%d %s(s)%", (long int)0, "message");
    myprintf("\r\n");
    myprintf("%d %s(s) with %%\r\n", (long int)0, "message");
    myprintf("justif: \"%-10s\"\r\n", "left");
    myprintf("justif: \"%10s\"\r\n", "right");
    myprintf(" 3: %04d zero padded\r\n", (long int)3);
    myprintf(" 3: %-4d left justif.\r\n", (long int)3);
    myprintf(" 3: %4d right justif.\r\n", (long int)3);
    myprintf("-3: %04d zero padded\r\n", (long int)-3);
    myprintf("-3: %-4d left justif.\r\n", (long int)-3);
    myprintf("-3: %4d right justif.\r\n\r\n\r\n", (long int)-3);
}

void uart_gpio_init(unsigned char ch)
{
    // Configure GPIO
    switch(ch)
    {
        case USCI_A0:
            break;
        case USCI_A1:
            // launchpad backchannel
            P3SEL0 |= BIT4 | BIT5; // USCI_A1 UART operation
            P3SEL1 &= ~(BIT4 | BIT5);
            break;
        default:
            // error
            break;
    }
}

void uart_baudrate_init(unsigned char clkType, unsigned char ch, unsigned char brType)
{
    // MSP430FR6xx Family User's Guide
    // 779page
    // Table 30-5. Recommended Settings for Typical Crystals and Baud Rates

    uArg.br0[CLOCK_1M][BR_9600] = 6;
    uArg.br1[CLOCK_1M][BR_9600] = 0;
    uArg.mctlw[CLOCK_1M][BR_9600] = UCOS16 | UCBRF_8 | 0x20000;

    uArg.br0[CLOCK_1M][BR_115200] = 0;
    uArg.br1[CLOCK_1M][BR_115200] = 0;
    uArg.mctlw[CLOCK_1M][BR_115200] = 0;

    uArg.br0[CLOCK_8M][BR_9600] = 52;
    uArg.br1[CLOCK_8M][BR_9600] = 0;
    uArg.mctlw[CLOCK_8M][BR_9600] = UCOS16 | UCBRF_1 | 0x4900;;

    uArg.br0[CLOCK_8M][BR_115200] = 4;
    uArg.br1[CLOCK_8M][BR_115200] = 0;
    uArg.mctlw[CLOCK_8M][BR_115200] = UCOS16 | UCBRF_5 | 0x5500;

    uArg.br0[CLOCK_16M][BR_9600] = 104;
    uArg.br1[CLOCK_16M][BR_9600] = 0;
    uArg.mctlw[CLOCK_16M][BR_9600] = UCOS16 | UCBRF_2 | 0xD600;

    uArg.br0[CLOCK_16M][BR_115200] = 8;
    uArg.br1[CLOCK_16M][BR_115200] = 0;
    uArg.mctlw[CLOCK_16M][BR_115200] = UCOS16 | UCBRF_10 | 0xF700;

    switch(ch)
    {
        case USCI_A0:
            // Configure USCI_A0 for UART mode
            UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
            UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK

            UCA0BR0 = uArg.br0[clkType][brType];
            UCA0BR1 = uArg.br1[clkType][brType];
            UCA0MCTLW |= uArg.mctlw[clkType][brType];

            UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
            UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
            break;
        case USCI_A1:
            // Configure USCI_A1 for UART mode
            UCA1CTLW0 = UCSWRST;                      // Put eUSCI in reset
            UCA1CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK

            UCA1BR0 = uArg.br0[clkType][brType];
            UCA1BR1 = uArg.br1[clkType][brType];
            UCA1MCTLW |= uArg.mctlw[clkType][brType];

            UCA1CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
            UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt
            break;
    }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(UCA1IV, USCI_UART_UCTXCPTIFG))
    {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        while(!(UCA1IFG & UCTXIFG));
        UCA1TXBUF = UCA1RXBUF;
        __no_operation();
        break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
    }
}
