#ifndef __SPI_INTERFACE_H
#define __SPI_INTERFACE_H

#include <stdint.h>

#define USE_SPI_INTERFACE

#define SPI_CS_OUT    P1OUT
#define SPI_CS_DIR    P1DIR
#define SPI_CS_PIN    BIT4

#define SPI_RST_OUT   P1OUT
#define SPI_RST_DIR   P1DIR
#define SPI_RST_PIN   BIT6

//******************************************************************************
// General SPI State Machine ***************************************************
//******************************************************************************

typedef enum SPI_ModeEnum{
    IDLE_MODE,
    TX_REG_ADDRESS_MODE,
    RX_REG_ADDRESS_MODE,
    TX_DATA_MODE,
    RX_DATA_MODE,
    TIMEOUT_MODE
} SPI_Mode;

#define DUMMY   0xFF

#define MAX_BUFFER_SIZE     20

extern uint8_t ReceiveBuffer[MAX_BUFFER_SIZE];

void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count);

SPI_Mode SPI_Master_WriteReg(uint8_t *reg_data, uint8_t count);
//SPI_Mode SPI_Master_WriteReg(uint8_t reg_addr, uint8_t *reg_data, uint8_t count);
SPI_Mode SPI_Master_ReadReg(uint8_t *reg_data, uint8_t count, uint8_t rxCount);
void initSPI(void);
void spiTest(void);

#endif
