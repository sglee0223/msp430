
/********************************************************************************
Module      : I2C_SW
Author      : 05/04/2015, by KienLTb - https://kienltb.wordpress.com/
Description : I2C software using bit-banging.
 ********************************************************************************/

/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include <msp430.h>

#include "i2c_gpio.h"

/*-----------------------------------------------------------------------------*/
/* Local Macro definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Function prototypes */
/*-----------------------------------------------------------------------------*/
unsigned char Read_SCL(void);
unsigned char Read_SDA(void);
void Clear_SCL(void);
void Clear_SDA(void);

void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);

void I2C_Writebit(unsigned char bit);
unsigned char I2C_ReadByte(void);

void I2C_WriteByte(unsigned char Data);
unsigned char I2C_ReadByte(void);

void I2C_WriteData(unsigned char DevideAddr, unsigned char Register, unsigned char *Data, unsigned char nLength);
void I2C_ReadData(unsigned char DevideAddr, unsigned char Register, unsigned char *Buff, unsigned char nLength);

/*-----------------------------------------------------------------------------*/
/* Function implementations */
/*-----------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------
Function    : Read_SCL
Purpose     : Set SCL as input and return current Logic level of SCL (0 or 1)
              nomal is 1 because pullup by resistor
Parameters  : None
Return      : Logic level of SCL pin
--------------------------------------------------------------------------------*/
unsigned char Read_SCL(void)
{
    I2C_PxDIR  &= ~SCL;
    return((I2C_PxIN & SCL) != 0);

}
/*--------------------------------------------------------------------------------
Function    : Read_SDA
Purpose     : Set SDA as input and return current Logic level of SDA (0 or 1),
              nomal is 1 because pullup by resistor
Parameters  : None
Return      : Logic level of SDA pin
--------------------------------------------------------------------------------*/
unsigned char Read_SDA(void)
{
    I2C_PxDIR  &= ~SDA;
    return((I2C_PxIN & SDA) != 0);
}

/*--------------------------------------------------------------------------------
Function    : Clear_SCL
Purpose     : Set SCL as Out put, logic Low
Parameters  : None
Return      : None
--------------------------------------------------------------------------------*/
void Clear_SCL(void)
{
    I2C_PxDIR  |= SCL;
}
/*--------------------------------------------------------------------------------
Function    : Clear_SDA
Purpose     : Set SDA as Out put, logic LOW
Parameters  : None
Return      : None
--------------------------------------------------------------------------------*/
void Clear_SDA(void)
{
    I2C_PxDIR  |= SDA;
}
/*--------------------------------------------------------------------------------
Function    : I2C_Init
Purpose     : Initialize I2C block
Parameters  : None
Return      : None
--------------------------------------------------------------------------------*/
void I2C_Gpio_Init(void)
{
    // Config SCL and SDA as GPIO
    //I2C_PxSEL0   &= ~(SCL + SDA);
    //I2C_PxSEL2  &= ~(SCL + SDA);
    // Set SCL and SDA is logic HIGH
    I2C_PxDIR   &= ~(SCL + SDA);
    I2C_PxOUT   &= ~(SCL + SDA);
}
/*--------------------------------------------------------------------------------
Function    : I2C_Start
Purpose     : Send start signal
Parameters  : None
Return      : None
--------------------------------------------------------------------------------*/
void I2C_Start(void)
{
    Read_SDA();             //set SDA to 1
    I2C_DELAY();
    Clear_SDA();            //set SDA to 0, currently SCL is 1
    I2C_DELAY();
    Clear_SCL();            //set SCL to 0
}
/*--------------------------------------------------------------------------------
Function    : I2C_Stop
Purpose     : Send Stop signal
Parameters  : None
Return      : None
--------------------------------------------------------------------------------*/
void I2C_Stop(void)
{
    Clear_SDA();            //set SDA to 0
    I2C_DELAY();
    Read_SCL();             //set SCL to 1
    I2C_DELAY();
    Read_SDA();             //set SDA to 1
}
/*--------------------------------------------------------------------------------
Function    : I2C_Writebit
Purpose     : Write bit to I2C bus
Parameters  : a bit need to write
Return      : None
--------------------------------------------------------------------------------*/
void I2C_Writebit(unsigned char bit)
{
    if(bit)
        Read_SDA();
    else
        Clear_SDA();
    I2C_DELAY();
    Read_SCL();
    I2C_DELAY();
    Clear_SCL();
}
/*--------------------------------------------------------------------------------
Function    : I2C_Readbit
Purpose     : Read bit to I2C bus
Parameters  : None
Return      : unsigned char
--------------------------------------------------------------------------------*/
unsigned char I2C_Readbit(void)
{
    unsigned char bit;
    //Let the slave driver data
    Read_SDA();
    I2C_DELAY();
    Read_SCL();
    bit = Read_SDA();
    I2C_DELAY();
    Clear_SCL();
    return bit;
}
/*--------------------------------------------------------------------------------
Function    : I2C_WriteByte
Purpose     : Write a Byte to I2C bus
Parameters  : unsigned char Data
Return      : None
--------------------------------------------------------------------------------*/
void I2C_WriteByte(unsigned char Data)
{
    unsigned char nBit;

    for(nBit = 0; nBit < 8; nBit++)
    {
        I2C_Writebit((Data & 0x80) != 0);
        Data <<= 1;
    }
    I2C_Readbit(); // Waite NACK
}
/*--------------------------------------------------------------------------------
Function    : I2C_ReadByte
Purpose     : Read a Byte to I2C bus
Parameters  : None
Return      : unsigned char
--------------------------------------------------------------------------------*/
unsigned char I2C_ReadByte(void)
{
    unsigned char Buff = 0;
    unsigned char nBit;

    for(nBit = 0; nBit < 8; nBit++)
    {
        Buff = (Buff << 1) | I2C_Readbit();
    }
    return Buff;
}
/*--------------------------------------------------------------------------------
Function    : I2C_WriteData
Purpose     : Write n Byte to I2C bus
Parameters  : DevideAddr    - Devide Address
              Register      - Register Address
              Data          - Pointer to Data need to write
              nLength       - Number of Byte need to write
Return      : None
--------------------------------------------------------------------------------*/
void I2C_WriteData(unsigned char DevideAddr, unsigned char Register, unsigned char *Data, unsigned char nLength)
{
    unsigned char nIndex;
    I2C_Start();
    I2C_WriteByte(DevideAddr << 1);  // byDeviceAddr is 7 bit and command is write
    I2C_WriteByte(Register);
    for(nIndex = 0; nIndex < nLength; nIndex++)
    {
        I2C_WriteByte(*(Data + nIndex));
    }
    I2C_Readbit();
    I2C_Stop();
}
/*--------------------------------------------------------------------------------
Function    : I2C_ReadData
Purpose     : Read n Byte from I2C bus
Parameters  : DevideAddr    - Devide Address
              Register      - Register Address
              Buff          - Pointer to Buffer store value
              nLength       - Number of Byte need to read
Return      : None
--------------------------------------------------------------------------------*/
void I2C_ReadData(unsigned char DevideAddr, unsigned char Register, unsigned char *Buff, unsigned char nLength)
{
    unsigned char nIndex;
    I2C_Start();
    I2C_WriteByte(DevideAddr << 1);
    I2C_WriteByte(Register);
    I2C_Stop();
    _NOP();                                 // Short delay
    I2C_Start();
    _NOP();                                 // Short delay
    I2C_WriteByte((DevideAddr << 1) | 1);
    for(nIndex = 0; nIndex < nLength; nIndex++)
    {
        *(Buff + nIndex) = I2C_ReadByte();
        if(nIndex > 0)I2C_Writebit(ACK);
    }
    I2C_Writebit(NACK);
    I2C_Stop();
}
