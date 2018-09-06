
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

            //SMCLK = MCLK = 16MHz, ACLK = VLOCLK = ~9.4kHz
            CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
            CSCTL1 = DCOFSEL_4 | DCORSEL;            // Set DCO to 16MHz
            CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK; // Set SMCLK = MCLK = DCO,
                                                        // ACLK = LFXTCLK (VLOCLK if unavailable)
            CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
            CSCTL0_H = 0;                             // Lock CS registers
            break;
    }
}
