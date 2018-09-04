
/********************************************************************************
Module      : I2C_SW
Author      : 05/04/2015, by KienLTb - https://kienltb.wordpress.com/
Description : I2C software using bit-banging.
 ********************************************************************************/

#ifndef __I2C_GPIO_H
#define __I2C_GPIO_H

#include <msp430.h>

#define USE_I2C_GPIO

/*-----------------------------------------------------------------------------*/
/* Macro definitions  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Macro definitions */
/*-----------------------------------------------------------------------------*/
//#define I2C_PxSEL      P1SEL
//#define I2C_PxSEL2      P1SEL2
#define I2C_PxDIR       P1DIR
#define I2C_PxOUT       P1OUT
#define I2C_PxIN        P1IN

#define SCL             BIT7
#define SDA             BIT6

#define ACK             0x00
#define NACK            0x01

#define TIME_DELAY 100
#define I2C_DELAY() __delay_cycles(TIME_DELAY)
/*-----------------------------------------------------------------------------*/
/* Function prototypes  */
/*-----------------------------------------------------------------------------*/

//NOTE: Need custom Read_SCL(), Read_SDA(), Clear_SCL(), Clear_SDA() to compatible Hardware.

unsigned char Read_SCL(void); // Set SCL as input and return current level of line, 0 or 1, nomal is 1 because pullup by res
unsigned char Read_SDA(void); // Set SDA as input and return current level of line, 0 or 1, nomal is 0 because pull by res

void Clear_SCL(void); // Actively drive SCL signal Low
void Clear_SDA(void); // Actively drive SDA signal Low

void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);

void I2C_Writebit(unsigned char bit);
unsigned char I2C_Readbit(void);

void I2C_WriteByte(unsigned char Data);
unsigned char I2C_ReadByte(void);

void I2C_WriteData(unsigned char DevideAddr, unsigned char Register, unsigned char *Data, unsigned char nLength);
void I2C_ReadData(unsigned char DevideAddr, unsigned char Register, unsigned char *Buff, unsigned char nLength);

#endif
