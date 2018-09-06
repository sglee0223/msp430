
#include <msp430.h>

#include "myclock.h"
#include "myprintf.h"

void clock_init(unsigned char clkType)
{
    switch(clkType)
    {
        case CLOCK_1M:
            // ACLK = n/a, MCLK = SMCLK = default DCO
            break;

        case CLOCK_8M:
            //SMCLK = MCLK = 8MHz, ACLK = VLOCLK = ~9.4kHz
            CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
            CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
            CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
            CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
            CSCTL0_H = 0;                             // Lock CS registers
            break;

        case CLOCK_16M:
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
            break;
    }
}
