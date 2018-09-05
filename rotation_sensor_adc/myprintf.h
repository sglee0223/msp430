
#ifndef __MYPRINTF_H
#define __MYPRINTF_H

#include <stdarg.h>

void uart_init(void);
int myprintf(char *format, ...);

#endif
