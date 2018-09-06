
#include <msp430.h>
#include <stdint.h>

#include "i2c_interface.h"
#include "i2c_gpio.h"
#include "pcf8563.h"
#include "myprintf.h"

void I2C_Scan(void) 
{

}

void readyI2C(void)
{
}

void setI2C(uint8_t reg, uint8_t data)
{
    uint8_t txData[1] = {0,};

    readyI2C();

    txData[0] = data;

#ifdef USE_I2C_INTERFACE
    I2C_Master_WriteReg(RTC_ADDR, reg, txData, sizeof(txData));
#endif
#ifdef USE_I2C_GPIO
    I2C_WriteData((RTC_ADDR), reg, txData, sizeof(txData));
#endif
}

uint8_t getI2C(uint8_t reg)
{
    uint8_t rxData[1] = {0,};

    readyI2C();

#ifdef USE_I2C_INTERFACE
    I2C_Master_ReadReg(RTC_ADDR, reg, sizeof(rxData));
    CopyArray(ReceiveBuffer, rxData, sizeof(rxData));
#endif
#ifdef USE_I2C_GPIO
    I2C_ReadData((RTC_ADDR), reg, rxData, sizeof(rxData));
#endif
    return rxData[0];
}

void RTC_Init(void)
{
    setI2C(REG_CTRL_STATUS_1, 0x00);
    setI2C(REG_CTRL_STATUS_2, 0x00);
}

void PCF8563_setDate(uint16_t year, uint8_t mon, uint8_t day)
{
    year	= changeIntToHex(year % 100);
    mon 	= changeIntToHex(mon) | ((year > 1999) ? 0x00 : 0x80);
    day 	= changeIntToHex(day);

    setI2C(REG_YEAR, year);
    setI2C(REG_MON, mon);
    setI2C(REG_DAY, day);
}

void PCF8563_getDate(void)
{
    uint8_t buf[3] = {0,};
    uint8_t century = 0;
    uint16_t year;
    uint8_t mon, day;

    buf[0] = getI2C(REG_YEAR);
    buf[1] = getI2C(REG_MON);
    buf[2] = getI2C(REG_DAY);

    if (buf[1] & 0x80)
        century = 1;

    buf[0] = buf[0]&0xff;
    buf[1] = buf[1]&0x1f;
    buf[2] = buf[2]&0x3f;

    year = (century == 1) ? (1900 + changeHexToInt(buf[0])) : (2000 + changeHexToInt(buf[0]));
    mon = changeHexToInt(buf[1]);
    day = changeHexToInt(buf[2]);

    myprintf("%d-%d-%d ", (long int)year, (long int)mon, (long int)day);
}

void PCF8563_setTime(uint8_t hour, uint8_t min, uint8_t sec)
{
    setI2C(REG_HOUR, changeIntToHex(hour));
    setI2C(REG_MIN, changeIntToHex(min));
    setI2C(REG_SEC, changeIntToHex(sec));
}

void PCF8563_getTime(void)
{
    uint8_t hour, min, sec;

    hour = changeHexToInt(getI2C(REG_HOUR) & 0x3f);
    min = changeHexToInt(getI2C(REG_MIN) & 0x7f);
    sec = changeHexToInt(getI2C(REG_SEC) & 0x7f);

    myprintf("%d:%d:%d\r\n", (long int)hour, (long int)min, (long int)sec);
}
