
#include <msp430.h>
#include <string.h>

#include "spi_interface.h"
#include "myprintf.h"

#include "at45dbxx.h"

#define CONFIG_AT45DB_PREWAIT
//#define CONFIG_AT45DB_PWRSAVE

struct at45db_dev_s
{
    uint8_t  pageshift;        /* log2 of the page size (eg. 1 << 9 = 512) */
    uint32_t npages;           /* Number of pages in the device */
};

struct at45db_dev_s priv;

/* Lock and per-transaction configuration */

void at45db_active(void);
void at45db_deactive(void);

/* Power management */

#ifdef CONFIG_AT45DB_PWRSAVE
void at45db_pwrdown(void);
void at45db_resume(void);
#else
#define  at45db_pwrdown()
#define  at45db_resume()
#endif

int at45db_rdid(void);
uint8_t at45db_rdsr(void);
uint8_t at45db_waitbusy(void);
void at45db_pgerase(long offset);
void at32db_chiperase(void);
void at45db_pgwrite(uint8_t *buffer, long offset);

int at45db_erase(long startblock, unsigned int nblocks);
int at45db_bread(long startblock, unsigned int nblocks, uint8_t *buf);
int at45db_bwrite(long startblock, unsigned int nblocks, uint8_t *buf);
int at45db_read(long offset, unsigned int nbytes, uint8_t *buffer);

/* Chip erase sequence */
#define CHIP_ERASE_SIZE 4
uint8_t g_chiperase[CHIP_ERASE_SIZE] = {0xc7, 0x94, 0x80, 0x9a};

/* Sequence to program the device to binary page sizes{256, 512, 1024} */
#define BINPGSIZE_SIZE 4
uint8_t g_binpgsize[BINPGSIZE_SIZE] = {0x3d, 0x2a, 0x80, 0xa6};

void at45db_active(void)
{
    SPI_CS_OUT &= ~SPI_CS_PIN;

    //__delay_cycles(1600); // 160=1us, 10us
}

void at45db_deactive(void)
{
    SPI_CS_OUT |= SPI_CS_PIN;

    //__delay_cycles(1600); // 160=1us, 10us
}

#ifdef CONFIG_AT45DB_PWRSAVE
void at45db_pwrdown(void)
{
    HAL_StatusTypeDef res;
    uint8_t cmd[1] = {0, };

    at45db_active();

    cmd[0] = AT45DB_PWRDOWN;
    res = HAL_SPI_Transmit(&hspi1, cmd, sizeof(cmd), HAL_MAX_DELAY);

    at45db_deactive();
}

void at45db_resume(void)
{
    HAL_StatusTypeDef res;
    uint8_t cmd[1] = {0, };

    at45db_active();

    cmd[0] = AT45DB_RESUME;
    res = HAL_SPI_Transmit(&hspi1, cmd, sizeof(cmd), HAL_MAX_DELAY);

    at45db_deactive();
    DWT_Delay_us(50);
}
#endif

/************************************************************************************
 * Name: at45db_rdid
 ************************************************************************************/

int at45db_rdid(void)
{
    uint8_t capacity;

    uint8_t cmd[1] = {0, };
    uint8_t devid[4] = {0, };

    at45db_active();

    cmd[0] = AT45DB_RDDEVID;

    SPI_Master_ReadReg(cmd, sizeof(cmd), sizeof(devid));
    CopyArray(ReceiveBuffer, devid, sizeof(devid));
    memset(ReceiveBuffer, 0x00, sizeof(ReceiveBuffer));

    at45db_deactive();

    myprintf("manufacturer: %02x devid1: %02x devid2: %02x\r\n", (unsigned long)devid[0], (unsigned long)devid[1], (unsigned long)devid[2]);

    /* Check for a valid manufacturer and memory family */

    if (devid[0] == AT45DB_MANUFACTURER &&
            (devid[1] & AT45DB_DEVID1_FAMMSK) == AT45DB_DEVID1_DFLASH)
    {
        /* Okay.. is it a FLASH capacity that we understand? */

        capacity = devid[1] & AT45DB_DEVID1_CAPMSK;

        myprintf("capacity: %02x\r\n", (unsigned long)capacity);

        switch (capacity)
        {
        case AT45DB_DEVID1_1MBIT:
            /* Save the FLASH geometry for the 16Mbit AT45DB011 */

            priv.pageshift   = 8;    /* Page size = 256 bytes */
            priv.npages      = 512;  /* 512 pages */
            return 1;

        case AT45DB_DEVID1_2MBIT:
            /* Save the FLASH geometry for the 16Mbit AT45DB021 */

            priv.pageshift   = 8;    /* Page size = 256/264 bytes */
            priv.npages      = 1024; /* 1024 pages */
            return 1;

        case AT45DB_DEVID1_4MBIT:
            /* Save the FLASH geometry for the 16Mbit AT45DB041 */

            priv.pageshift   = 8;    /* Page size = 256/264 bytes */
            priv.npages      = 2048; /* 2048 pages */

            myprintf("4MBIT DataFlash\r\n");
            return 1;

        case AT45DB_DEVID1_8MBIT:
            /* Save the FLASH geometry for the 16Mbit AT45DB081 */

            priv.pageshift   = 8;    /* Page size = 256/264 bytes */
            priv.npages      = 4096; /* 4096 pages */
            return 1;

        case AT45DB_DEVID1_16MBIT:
            /* Save the FLASH geometry for the 16Mbit AT45DB161 */

            priv.pageshift   = 9;    /* Page size = 512/528 bytes */
            priv.npages      = 4096; /* 4096 pages */
            return 1;

        case AT45DB_DEVID1_32MBIT:
            /* Save the FLASH geometry for the 16Mbit AT45DB321 */

            priv.pageshift   = 9;    /* Page size = 512/528 bytes */
            priv.npages      = 8192; /* 8192 pages */
            return 1;

        case AT45DB_DEVID1_64MBIT:
            /* Save the FLASH geometry for the 16Mbit AT45DB321 */

            priv.pageshift   = 10;   /* Page size = 1024/1056 bytes */
            priv.npages      = 8192; /* 8192 pages */
            return 1;

        default:
            return -1;
        }
    }

    return -1;
}

/************************************************************************************
 * Name: at45db_rdsr
 ************************************************************************************/

uint8_t at45db_rdsr(void)
{
    uint8_t cmd[1] = {0, };
    uint8_t status[1] = {0, };

    at45db_active();

    cmd[0] = AT45DB_RDSR;
    SPI_Master_ReadReg(cmd, sizeof(cmd), sizeof(status));
    CopyArray(ReceiveBuffer, status, sizeof(status));
    memset(ReceiveBuffer, 0x00, sizeof(ReceiveBuffer));

    at45db_deactive();

    return status[0];
}

/************************************************************************************
 * Name: at45db_waitbusy
 ************************************************************************************/

uint8_t at45db_waitbusy(void)
{
    uint8_t sr;

    /* Poll the device, waiting for it to report that it is ready */

    do
    {
        __delay_cycles(1600); // 10us
        sr = (uint8_t)at45db_rdsr();
        //myprintf(".");
    }
    while ((sr & AT45DB_SR_RDY) == 0);

    return sr;
}


/************************************************************************************
 * Name:  at45db_pgerase
 ************************************************************************************/

void at45db_pgerase(long sector)
{
    uint8_t cmd[4] = {0, };
    uint32_t offset = sector << priv.pageshift;

    myprintf("sector: %08lx\r\n", (unsigned long)sector);

    /* Higher performance write logic:  We leave the chip busy after write and erase
     * operations.  This improves write and erase performance because we do not have
     * to wait as long between transactions (other processing can occur while the chip
     * is busy) but means that the chip must stay powered and that we must check if
     * the chip is still busy on each entry point.
     */

#ifdef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    /* "The Page Erase command can be used to individually erase any page in the main
     *  memory array allowing the Buffer to Main Memory Page Program to be utilized at a
     *  later time. ... To perform a page erase in the binary page size ..., the
     *  opcode 81H must be loaded into the device, followed by three address bytes
     *  ... When a low-to-high transition occurs on the CS pin, the part will erase the
     *  selected page (the erased state is a logical 1). ... the status register and the
     *  RDY/BUSY pin will indicate that the part is busy."
     */

    cmd[0] = AT45DB_PGERASE;   /* Page erase command */
    cmd[1] = (offset >> 16) & 0xff; /* 24-bit offset MS bytes */
    cmd[2] = (offset >>  8) & 0xff; /* 24-bit offset middle bytes */
    cmd[3] =  offset        & 0xff; /* 24-bit offset LS bytes */

    /* Erase the page */

    at45db_active();

    SPI_Master_WriteReg(cmd, sizeof(cmd));

    at45db_deactive();

    /* Wait for any erase to complete if we are not trying to improve write
     * performance. (see comments above).
     */

#ifndef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    myprintf("Erased\r\n");
}

/************************************************************************************
 * Name:  at32db_chiperase
 ************************************************************************************/

void at32db_chiperase(void)
{
    /* Higher performance write logic:  We leave the chip busy after write and erase
     * operations.  This improves write and erase performance because we do not have
     * to wait as long between transactions (other processing can occur while the chip
     * is busy) but means that the chip must stay powered and that we must check if
     * the chip is still busy on each entry point.
     */

#ifdef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    /* "The entire main memory can be erased at one time by using the Chip Erase
     * command. To execute the Chip Erase command, a 4-byte command sequence C7H, 94H,
     * 80H and 9AH must be clocked into the device. ... After the last bit of the opcode
     * sequence has been clocked in, the CS pin can be deasserted to start the erase
     * process. ... the Status Register will indicate that the device is busy. The Chip
     * Erase command will not affect sectors that are protected or locked down...
     */

    at45db_active();

    SPI_Master_WriteReg(g_chiperase, CHIP_ERASE_SIZE);

    at45db_deactive();

    /* Wait for any erase to complete if we are not trying to improve write
     * performance. (see comments above).
     */

#ifndef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif
}

/************************************************************************************
 * Name:  at45db_pgwrite
 ************************************************************************************/

void at45db_pgwrite(uint8_t *buffer, long page)
{
    //uint8_t cmd[4] = {0, };
    uint8_t cmd[128] = {0, };
    uint32_t offset = page << priv.pageshift;

    myprintf("page: %08lx offset: %08lx\r\n", (unsigned long)page, (unsigned long)offset);

    /* We assume that sectors are not write protected */

    cmd[0] = AT45DB_MNTHRUBF1;      /* To main memory through buffer 1 */
    cmd[1] = (offset >> 16) & 0xff; /* 24-bit address MS byte */
    cmd[2] = (offset >>  8) & 0xff; /* 24-bit address middle byte */
    cmd[3] =  offset        & 0xff; /* 24-bit address LS byte */

    /* Higher performance write logic:  We leave the chip busy after write and erase
     * operations.  This improves write and erase performance because we do not have
     * to wait as long between transactions (other processing can occur while the chip
     * is busy) but means that the chip must stay powered and that we must check if
     * the chip is still busy on each entry point.
     */

#ifdef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    at45db_active();

    CopyArray(&buffer[0], &cmd[4], (1 << priv.pageshift));

    SPI_Master_WriteReg(cmd, 4 + (1 << priv.pageshift));

    //SPI_Master_WriteReg(buffer, 1 << priv.pageshift);

    at45db_deactive();

    /* Wait for any erase to complete if we are not trying to improve write
     * performance. (see comments above).
     */

#ifndef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif
    myprintf("Written\r\n");
}

/************************************************************************************
 * Name: at45db_erase
 ************************************************************************************/

int at45db_erase(long startblock, unsigned int nblocks)
{
    unsigned int pgsleft = nblocks;

    myprintf("startblock: %08lx nblocks: %d\r\n", (unsigned long)startblock, (unsigned long)nblocks);

    /* Take the lock so that we have exclusive access to the bus, then power up the
     * FLASH device.
     */

    at45db_resume();

    /* Then erase each page */

    while (pgsleft-- > 0)
    {
        /* Erase each sector */

        at45db_pgerase(startblock);
        startblock++;
    }

    at45db_pwrdown();

    return (int)nblocks;
}

/************************************************************************************
 * Name: at45db_bread
 ************************************************************************************/

int at45db_bread(long startblock, unsigned int nblocks, uint8_t *buffer)
{
    int nbytes;

    /* On this device, we can handle the block read just like the byte-oriented read */

    nbytes = at45db_read(startblock << priv.pageshift, nblocks << priv.pageshift, buffer);
    if (nbytes > 0)
    {
        return nbytes >> priv.pageshift;
    }
    return nbytes;
}

/************************************************************************************
 * Name: at45db_bwrite
 ************************************************************************************/

int at45db_bwrite(long startblock, unsigned int nblocks, uint8_t *buffer)
{
    unsigned int pgsleft = nblocks;

    myprintf("startblock: %08lx nblocks: %d\r\n", (unsigned long)startblock, (unsigned long)nblocks);

    /* Take the lock so that we have exclusive access to the bus, then power up the
     * FLASH device.
     */

    at45db_active();
    at45db_resume();

    /* Write each page to FLASH */
    while (pgsleft-- > 0)
    {
        at45db_pgwrite(buffer, startblock);
        startblock++;
    }

    at45db_pwrdown();
    at45db_deactive();

    return nblocks;
}

/************************************************************************************
 * Name: at45db_read
 ************************************************************************************/

int at45db_read(long offset, unsigned int nbytes, uint8_t *buffer)
{
    uint8_t cmd[5] = {0, };

    myprintf("offset: %08lx nbytes: %d\r\n", (unsigned long)offset, (unsigned long)nbytes);

    /* Set up for the read */

    cmd[0] = AT45DB_RDARRAYHF;       /* FAST_READ is safe at all supported SPI speeds. */
    cmd[1] = (offset >> 16) & 0xff;  /* 24-bit address upper byte */
    cmd[2] = (offset >>  8) & 0xff;  /* 24-bit address middle byte */
    cmd[3] =  offset        & 0xff;  /* 24-bit address least significant byte */
    cmd[4] = 0;                      /* Dummy byte */

    /* Take the lock so that we have exclusive access to the bus, then power up the
     * FLASH device.
     */
    at45db_resume();

    /* Higher performance write logic:  We leave the chip busy after write and erase
     * operations.  This improves write and erase performance because we do not have
     * to wait as long between transactions (other processing can occur while the chip
     * is busy) but means that the chip must stay powered and that we must check if
     * the chip is still busy on each entry point.
     */

#ifdef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    /* Perform the read */
    at45db_active();

    SPI_Master_ReadReg(cmd, sizeof(cmd), nbytes);
    CopyArray(ReceiveBuffer, buffer, nbytes);
    memset(ReceiveBuffer, 0x00, sizeof(ReceiveBuffer));

    at45db_deactive();

    at45db_pwrdown();

    myprintf("return nbytes: %d\r\n", (unsigned long)nbytes);

    return nbytes;
}

/************************************************************************************
 * Name: at45db_initialize
 *
 * Description:
 *   Create an initialize MTD device instance.  MTD devices are not registered
 *   in the file system, but are created as instances that can be bound to
 *   other functions (such as a block or character driver front end).
 *
 ************************************************************************************/

int at45db_initialize(void)
{
    uint8_t sr;
    int ret;

    /* Deselect the FLASH */
    at45db_deactive();

    at45db_resume();

    //while(1)
    {
    /* Identify the FLASH chip and get its capacity */
    ret = at45db_rdid();
    if (ret != 1)
    {
        /* Unrecognized! Discard all of that work we just did and return NULL */

        myprintf("Unrecognized\r\n");
        return -1;
    }

        //for (ret = 0 ; ret < 100; ret++)
        //    __delay_cycles(160000); // 1ms
    }

    /* Get the value of the status register (as soon as the device is ready) */
    sr = at45db_waitbusy();

    myprintf("status register: %02x\r\n", (unsigned long)sr);

    /* Check if the device is configured as 256, 512 or 1024 bytes-per-page device */
    if ((sr & AT45DB_SR_PGSIZE) == 0)
    {
        /* No, re-program it for the binary page size.  NOTE:  A power cycle
         * is required after the device has be re-programmed.
         */

        myprintf("Reprogramming page size\r\n");

        at45db_active();

        SPI_Master_WriteReg(g_binpgsize, BINPGSIZE_SIZE);

        at45db_deactive();

        return -1;
    }

    /* power down the device */
    at45db_pwrdown();

    return 1;
}

void ErasingDataFlash(void)
{
    uint8_t cmd[4] = {0, };
    uint16_t block_counter = 0;

    myprintf("ErasingDataFlash Start\r\n");

    while (block_counter < 512)
    {
        //myprintf("block_counter: %d, 0x%x, 0x%x\r\n", block_counter, (uint8_t)(block_counter>>5), (uint8_t)(block_counter<<3));
        at45db_active();

        cmd[0] = AT45DB_BLKERASE;
        cmd[1] = (uint8_t)(block_counter>>5);
        cmd[2] = (uint8_t)(block_counter<<3);
        cmd[3] = 0x00;

        SPI_Master_WriteReg(cmd, sizeof(cmd));

        block_counter++;

        at45db_deactive();

        at45db_waitbusy();
    }

    myprintf("ErasingDataFlash End\r\n");
}

void FlashBuffer1Write(uint16_t start_addr, uint16_t len, uint8_t *buffer)
{
    //uint8_t cmd[4] = {0, };
    uint8_t cmd[128] = {0, };

    myprintf("FlashBuffer1Write Start\r\n");

#ifdef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    at45db_active();

    cmd[0] = AT45DB_WRBF1;
    cmd[1] = 0x00;
    cmd[2] = (uint8_t)(start_addr>>8);
    cmd[3] = (uint8_t)(start_addr);

    CopyArray(&buffer[0], &cmd[4], len);

    SPI_Master_WriteReg(cmd, 4 + len);

    //SPI_Master_WriteReg(buffer, len);

    at45db_deactive();

#ifndef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    myprintf("FlashBuffer1Write End\r\n");
}

void FlashBuffer1Read(uint16_t start_addr, uint16_t len, uint8_t *buffer)
{
    uint8_t cmd[5] = {0, };

    myprintf("FlashBuffer1Read Start\r\n");

#ifdef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    at45db_active();

    cmd[0] = AT45DB_RDBF1;
    cmd[1] = 0x00;
    cmd[2] = (uint8_t)(start_addr>>8);
    cmd[3] = (uint8_t)(start_addr);
    cmd[4] = 0x00;  // Additional Don't Care Bytes

    SPI_Master_ReadReg(cmd, sizeof(cmd), len);
    CopyArray(ReceiveBuffer, buffer, len);
    memset(ReceiveBuffer, 0x00, sizeof(ReceiveBuffer));

    at45db_deactive();

#ifndef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    myprintf("FlashBuffer1Read End\r\n");
}

void FlashBuffer1ProgAutoErase(uint16_t star_addr)
{
    uint8_t cmd[4] = {0, };

    myprintf("FlashBuffer1ProgAutoErase Start\r\n");

#ifdef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    at45db_active();

    cmd[0] = AT45DB_BF1TOMNE;
    cmd[1] = (uint8_t)(star_addr>>8);
    cmd[2] = (uint8_t)(star_addr);
    cmd[3] = 0x00;

    SPI_Master_WriteReg(cmd, sizeof(cmd));

    at45db_deactive();

#ifndef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    myprintf("FlashBuffer1ProgAutoErase End\r\n");
}

void PageToBuffer1(uint16_t star_addr)
{
    uint8_t cmd[4] = {0, };

    myprintf("PageToBuffer1 Start\r\n");

#ifdef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    at45db_active();

    cmd[0] = AT45DB_MNTOBF1XFR;
    cmd[1] = (uint8_t)(star_addr>>7);
    cmd[2] = (uint8_t)(star_addr<<1);
    cmd[3] = 0x00;

    SPI_Master_WriteReg(cmd, sizeof(cmd));

    at45db_deactive();

#ifndef CONFIG_AT45DB_PREWAIT
    at45db_waitbusy();
#endif

    myprintf("PageToBuffer1 End\r\n");
}

void at45db_test(void)
{
    uint8_t string1[] = "1234567890UUAA";
    uint8_t string2[] = "              ";
    uint8_t  MyBuff1[64]={0,};

    myprintf("at45db_test Start\r\n");

    ErasingDataFlash();

    FlashBuffer1Write(0, sizeof(string1), string1);//(uint8_t *)"1234567890UUAA");

    memset(MyBuff1, 0, sizeof(MyBuff1));
    FlashBuffer1Read(0, sizeof(string1), MyBuff1);

    myprintf("First: %s\r\n", MyBuff1);

    FlashBuffer1ProgAutoErase(0);

    FlashBuffer1Write(0, sizeof(string2), string2);//(uint8_t *)"              ");

    PageToBuffer1(0);

    memset(MyBuff1, 0, sizeof(MyBuff1));
    FlashBuffer1Read(0, sizeof(string2), MyBuff1);

    myprintf("Second: %s\r\n", MyBuff1);

    myprintf("at45db_test End\r\n");
}

