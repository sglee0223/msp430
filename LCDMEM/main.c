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
//  MSP430FR69x Demo - LCD_C, RTC_C, Display real time clock on LCD in LPM3 mode.
//
//  Description: This examples configures the LCD in 4-Mux mode and displays
//  "430" on a TI LCD and blinks it at 1Hz interval. This example was tested
//  on MSP-EXP430FR6989 Launchpad. This example would need to be modified based
//  on the final LCD and GPIO routing where lcd_num[] would need to be modified.
//  The internal voltage is sourced to v2 through v4 and v5 is connected to
//  ground. Charge pump is enabled.
//
//  It uses LCD pin L0~L21 and L26~L43 as segment pins.
//  f(LCD) = 32768Hz/((1+1)*2^4) = 1024Hz, ACLK = ~32768Hz,
//  MCLK = SMCLK = default DCODIV ~1MHz.
//
//      MSP430FR6989 / MSP-EXP430FR6989 Launchpad
//              -----------------
//          /|\|                 |
//           | |              XIN|--
//  GND      --|RST              |  32768Hz
//   |         |             XOUT|--
//   |         |                 |
//   |         |             COM3|----------------|
//   |         |             COM2|---------------||
//   |--4.7uF -|LCDCAP       COM1|--------------|||
//             |             COM0|-------------||||
//             |                 |    -------------
//             |           Sx~Sxx|---| 1 2 3 4 5 6 |
//             |                 |    -------------
//             |                 |       TI LCD
//                                 (See MSP-EXP430FR6989 Schematic)
//
//   William Goh
//   Texas Instruments Inc.
//   May 2015
//   Built with IAR Embedded Workbench V6.30 & Code Composer Studio V6.1
//*****************************************************************************
#include "msp430.h"

#define pos1 9      /* Digit A1 begins at S18 */
#define pos2 5      /* Digit A2 begins at S10 */
#define pos3 3      /* Digit A3 begins at S6  */
#define pos4 18     /* Digit A4 begins at S36 */
#define pos5 14     /* Digit A5 begins at S28 */
#define pos6 7      /* Digit A6 begins at S14 */

// LCD memory map for numeric digits
const char digit[10][2] =
{
    {0xFC, 0x28},  /* "0" LCD segments a+b+c+d+e+f+k+q */
    {0x60, 0x20},  /* "1" */
    {0xDB, 0x00},  /* "2" */
    {0xF3, 0x00},  /* "3" */
    {0x67, 0x00},  /* "4" */
    {0xB7, 0x00},  /* "5" */
    {0xBF, 0x00},  /* "6" */
    {0xE4, 0x00},  /* "7" */
    {0xFF, 0x00},  /* "8" */
    /*
          b7
       b2    b6
        b1 b0
       b3    b5
          b4
    */
    {0xF7, 0x00}   /* "9" */
};

// LCD memory map for uppercase letters
const char alphabetBig[26][2] =
{
    {0xEF, 0x00},  /* "A" LCD segments a+b+c+e+f+g+m */
    {0xF1, 0x50},  /* "B" */
    {0x9C, 0x00},  /* "C" */
    {0xF0, 0x50},  /* "D" */
    {0x9F, 0x00},  /* "E" */
    {0x8F, 0x00},  /* "F" */
    {0xBD, 0x00},  /* "G" */
    {0x6F, 0x00},  /* "H" */
    {0x90, 0x50},  /* "I" */
    {0x78, 0x00},  /* "J" */
    {0x0E, 0x22},  /* "K" */
    {0x1C, 0x00},  /* "L" */
    {0x6C, 0xA0},  /* "M" */
    {0x6C, 0x82},  /* "N" */
    {0xFC, 0x00},  /* "O" */
    {0xCF, 0x00},  /* "P" */
    {0xFC, 0x02},  /* "Q" */
    {0xCF, 0x02},  /* "R" */
    {0xB7, 0x00},  /* "S" */
    {0x80, 0x50},  /* "T" */
    {0x7C, 0x00},  /* "U" */
    {0x0C, 0x28},  /* "V" */
    {0x6C, 0x0A},  /* "W" */
    {0x00, 0xAA},  /* "X" */
    {0x00, 0xB0},  /* "Y" */
    {0x90, 0x28}   /* "Z" */
};

void digitTest(void);
void alphabetTest(void);
void batteryTest(void);
void antenaIconTest(void);
void centerIconTest(void);
void dotEtcTest(void);

int main(void)
{
    unsigned char i;

    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT

    PJSEL0 = BIT4 | BIT5;                   // For LFXT

    // Initialize LCD segments 0 - 21; 26 - 43
    LCDCPCTL0 = 0xFFFF;
    LCDCPCTL1 = 0xFC3F;
    LCDCPCTL2 = 0x0FFF;

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure LFXT 32kHz crystal
    CSCTL0_H = CSKEY >> 8;                  // Unlock CS registers
    CSCTL4 &= ~LFXTOFF;                     // Enable LFXT
    do
    {
      CSCTL5 &= ~LFXTOFFG;                  // Clear LFXT fault flag
      SFRIFG1 &= ~OFIFG;
    }while (SFRIFG1 & OFIFG);               // Test oscillator fault flag
    CSCTL0_H = 0;                           // Lock CS registers

    // Initialize LCD_C
    // ACLK, Divider = 1, Pre-divider = 16; 4-pin MUX
    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;

    // VLCD generated internally,
    // V2-V4 generated internally, v5 to ground
    // Set VLCD voltage to 2.60v
    // Enable charge pump and select internal reference for it
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;

    LCDCCPCTL = LCDCPCLKSYNC;               // Clock synchronization enabled

    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory

    //Turn LCD on
    LCDCCTL0 |= LCDON;

    for(i=0; i<20; i++)
        LCDMEM[i] = 0xFF;
    __delay_cycles(1000000);
    __delay_cycles(1000000);

    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory

    digitTest();
    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory

    alphabetTest();
    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory

    batteryTest();
    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory

    antenaIconTest();
    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory

    centerIconTest();
    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory

    dotEtcTest();
    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory

    for(i=0; i<20; i++)
        LCDMEM[i] = 0xFF;

    __bis_SR_register(LPM3_bits | GIE);
    __no_operation();

    return 0;
}

void digitTest(void)
{
    unsigned char i;

    for(i=0; i<10; i++)
    {
        LCDMEM[pos1] = digit[i][0];
        LCDMEM[pos1+1] = digit[i][1];

        LCDMEM[pos2] = digit[i][0];
        LCDMEM[pos2+1] = digit[i][1];

        LCDMEM[pos3] = digit[i][0];
        LCDMEM[pos3+1] = digit[i][1];

        LCDMEM[pos4] = digit[i][0];
        LCDMEM[pos4+1] = digit[i][1];

        LCDMEM[pos5] = digit[i][0];
        LCDMEM[pos5+1] = digit[i][1];

        LCDMEM[pos6] = digit[i][0];
        LCDMEM[pos6+1] = digit[i][1];

        __delay_cycles(1000000);
    }
}

void alphabetTest(void)
{
    unsigned char i;

    for(i=0; i<26; i++)
    {
        LCDMEM[pos1] = alphabetBig[i][0];
        LCDMEM[pos1+1] = alphabetBig[i][1];

        LCDMEM[pos2] = alphabetBig[i][0];
        LCDMEM[pos2+1] = alphabetBig[i][1];

        LCDMEM[pos3] = alphabetBig[i][0];
        LCDMEM[pos3+1] = alphabetBig[i][1];

        LCDMEM[pos4] = alphabetBig[i][0];
        LCDMEM[pos4+1] = alphabetBig[i][1];

        LCDMEM[pos5] = alphabetBig[i][0];
        LCDMEM[pos5+1] = alphabetBig[i][1];

        LCDMEM[pos6] = alphabetBig[i][0];
        LCDMEM[pos6+1] = alphabetBig[i][1];

        __delay_cycles(1000000);
    }
}

void batteryTest(void)
{
    unsigned char i;

    unsigned char batLevel[6][2] =
    {
     {0x20, 0x00},
     {0x00, 0x20},
     {0x40, 0x00},
     {0x00, 0x40},
     {0x80, 0x00},
     {0x00, 0x80},
    };

    unsigned char batShape[1][2] =
    {
     {0x10, 0x10},
    };

    //LCDMEM[17] = 0xF0; // battery    [I I I ]    b4 b5 b6 b7
    //LCDMEM[13] = 0xF0; //              I I I -     b5 b6 b7 b4

    for(i=0; i<6; i++)
    {
        LCDMEM[17] = batLevel[i][0] + batShape[0][0];
        LCDMEM[13] = batLevel[i][1] + batShape[0][1];

        __delay_cycles(1000000);
    }
}

void antenaIconTest(void)
{
    LCDMEM[4] = 0x04; // special char // 0x04 = antenna
    __delay_cycles(1000000);

    LCDMEM[8] = 0x04; // special char // 0x04 = tx, 0x01 = rx
    __delay_cycles(1000000);

    LCDMEM[8] = 0x01; // special char // 0x04 = tx, 0x01 = rx
    __delay_cycles(1000000);
}

void centerIconTest(void)
{
    LCDMEM[2] = 0x04; // heart
    __delay_cycles(1000000);

    LCDMEM[2] = 0x01; // !
    __delay_cycles(1000000);

    LCDMEM[2] = 0x08; // alarm
    __delay_cycles(1000000);

    LCDMEM[2] = 0x02; // ring
    __delay_cycles(1000000);
}

void dotEtcTest(void)
{
    LCDMEM[pos1] = digit[8][0];
    LCDMEM[pos2] = digit[8][0];
    LCDMEM[pos3] = digit[8][0];
    LCDMEM[pos4] = digit[8][0];
    LCDMEM[pos5] = digit[8][0];
    LCDMEM[pos6] = digit[8][0];

    LCDMEM[pos1+1] = 0x04; // special char // 0x04 = -(sign), 0x01 = .(dot)
    __delay_cycles(1000000);
    LCDMEM[pos1+1] = 0x01; // special char // 0x04 = -(sign), 0x01 = .(dot)
    __delay_cycles(1000000);

    LCDMEM[pos2+1] = 0x04; // special char // 0x04 = :(semicolon), 0x01 = .(dot)
    __delay_cycles(1000000);
    LCDMEM[pos2+1] = 0x01; // special char // 0x04 = :(semicolon), 0x01 = .(dot)
    __delay_cycles(1000000);

    LCDMEM[pos3+1] = 0x01; // special char // 0x01 = .(dot)
    __delay_cycles(1000000);

    LCDMEM[pos4+1] = 0x04; // special char // 0x04 = :(semicolon), 0x01 = .(dot)
    __delay_cycles(1000000);
    LCDMEM[pos4+1] = 0x01; // special char // 0x04 = :(semicolon), 0x01 = .(dot)
    __delay_cycles(1000000);

    LCDMEM[pos5+1] = 0x04; // special char // 0x04 = o(degree), 0x01 = .(dot)
    __delay_cycles(1000000);
    LCDMEM[pos5+1] = 0x01; // special char // 0x04 = o(degree), 0x01 = .(dot)
    __delay_cycles(1000000);
}
