#ifndef __I2C_INTERFACE_H
#define __I2C_INTERFACE_H

/*
    important!!!

    nack received after start condition using i2c interface
    so i think maybe some hw work is needed
*/
//#define USE_I2C_INTERFACE

#define MAX_BUFFER_SIZE     20

//******************************************************************************
// General I2C State Machine ***************************************************
//******************************************************************************

typedef enum I2C_ModeEnum{
    IDLE_MODE,
    NACK_MODE,
    TX_REG_ADDRESS_MODE,
    RX_REG_ADDRESS_MODE,
    TX_DATA_MODE,
    RX_DATA_MODE,
    SWITCH_TO_RX_MODE,
    SWITHC_TO_TX_MODE,
    TIMEOUT_MODE
} I2C_Mode;

extern uint8_t ReceiveBuffer[MAX_BUFFER_SIZE];
extern I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count);
extern I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t count);
extern void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count);

#endif
