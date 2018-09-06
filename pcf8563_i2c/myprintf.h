
#ifndef __MYPRINTF_H
#define __MYPRINTF_H

#include <stdarg.h>

#define USCI_A0     0
#define USCI_A1     1

#define BR_9600     0
#define BR_115200   1

void uart_gpio_init(unsigned char ch);
void uart_baudrate_init(unsigned char clkType, unsigned char ch, unsigned char brType);
int myprintf(char *format, ...);
void printTest(void);

#endif
