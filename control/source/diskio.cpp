/*
This program is created based on the following mbed code.
https://os.mbed.com/users/mbed_official/code/SDFileSystem/
Below is the license terms of the original code.

/* mbed Microcontroller Library
 * Copyright (c) 2006-2012 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "MicroBit.h"
#include "diskio.h"		/* FatFs lower layer API */

extern MicroBit uBit;
extern SPI spi;
extern MicroBitPin cs;
extern int sd_init;

#define SD_COMMAND_TIMEOUT 5000

#define R1_IDLE_STATE           (1 << 0)
#define R1_ERASE_RESET          (1 << 1)
#define R1_ILLEGAL_COMMAND      (1 << 2)
#define R1_COM_CRC_ERROR        (1 << 3)
#define R1_ERASE_SEQUENCE_ERROR (1 << 4)
#define R1_ADDRESS_ERROR        (1 << 5)
#define R1_PARAMETER_ERROR      (1 << 6)

// Types
//  - v1.x Standard Capacity
//  - v2.x Standard Capacity
//  - v2.x High Capacity
//  - Not recognised as an SD Card
#define SDCARD_FAIL 0
#define SDCARD_V1   1
#define SDCARD_V2   2
#define SDCARD_V2HC 3

int cmd(int cmd, int arg) {
  cs.setDigitalValue(0);

  // send a command
  spi.write(0x40 | cmd);
  spi.write(arg >> 24);
  spi.write(arg >> 16);
  spi.write(arg >> 8);
  spi.write(arg >> 0);
  spi.write(0x95);

  // wait for the response (response[7] == 0)
  int response;
  for (int i = 0; i < SD_COMMAND_TIMEOUT; i++) {
    response = spi.write(0xFF);
    if (!(response & 0x80)) {
      cs.setDigitalValue(1);
      spi.write(0xFF);
      return response;
    }
  }
  cs.setDigitalValue(1);
  spi.write(0xFF);
  uBit.serial.printf("no response\r\n");
  return -1; // timeout
}

int cmd8() {
  cs.setDigitalValue(0);

  // send a command
  spi.write(0x40 | 8); // CMD8
  spi.write(0x00);     // reserved
  spi.write(0x00);     // reserved
  spi.write(0x01);     // 3.3v
  spi.write(0xAA);     // check pattern
  spi.write(0x87);     // crc

  // wait for the response (response[7] == 0)
  for (int i = 0; i < SD_COMMAND_TIMEOUT * 1000; i++) {
    char response[5];
    response[0] = spi.write(0xFF);
    if (!(response[0] & 0x80)) {
      for (int j = 1; j < 5; j++) {
        response[i] = spi.write(0xFF);
      }
      cs.setDigitalValue(1);
      spi.write(0xFF);
      return response[0];
    }
  }
  cs.setDigitalValue(1);
  spi.write(0xFF);
  return -1; // timeout
}

int cmd58() {
  cs.setDigitalValue(0);
  int arg = 0;

  // send a command
  spi.write(0x40 | 58);
  spi.write(arg >> 24);
  spi.write(arg >> 16);
  spi.write(arg >> 8);
  spi.write(arg >> 0);
  spi.write(0x95);

  // wait for the repsonse (response[7] == 0)
  for (int i = 0; i < SD_COMMAND_TIMEOUT; i++) {
    int response = spi.write(0xFF);
    if (!(response & 0x80)) {
      int ocr = spi.write(0xFF) << 24;
      ocr |= spi.write(0xFF) << 16;
      ocr |= spi.write(0xFF) << 8;
      ocr |= spi.write(0xFF) << 0;
      cs.setDigitalValue(1);
      spi.write(0xFF);
      return response;
    }
  }
  cs.setDigitalValue(1);
  spi.write(0xFF);
  return -1; // timeout
}

int read(uint8_t *buffer, uint32_t length) {
  cs.setDigitalValue(0);

  // read until start byte (0xFF)
  while (spi.write(0xFF) != 0xFE);

  // read data
  for (uint32_t i = 0; i < length; i++) {
    buffer[i] = spi.write(0xFF);
  }
  spi.write(0xFF); // checksum
  spi.write(0xFF);

  cs.setDigitalValue(1);
  spi.write(0xFF);
  return 0;
}

int write(const uint8_t*buffer, uint32_t length) {
  cs.setDigitalValue(0);

  // indicate start of block
  spi.write(0xFE);

  // write the data
  for (uint32_t i = 0; i < length; i++) {
    spi.write(buffer[i]);
  }

  // write the checksum
  spi.write(0xFF);
  spi.write(0xFF);

  // check the response token
  if ((spi.write(0xFF) & 0x1F) != 0x05) {
    cs.setDigitalValue(1);
    spi.write(0xFF);
    return 1;
  }

  // wait for write to finish
  while (spi.write(0xFF) == 0);

  cs.setDigitalValue(1);
  spi.write(0xFF);
  return 0;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
  return sd_init;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
  spi.format(8, 0);
  spi.frequency(4000000);
  cs.setDigitalValue(1);
  for (int i = 0; i < 16; i++) {
    spi.write(0xff);
  }

  // send CMD0, should return with all zeros except IDLE STATE set (bit 0)
  if (cmd(0, 0) != R1_IDLE_STATE) {
    uBit.serial.printf("No disk, or could not put SD card in to SPI idle state\r\n");
    return STA_NOINIT;
  }

  // send CMD8 to determine whther it is ver 2.x
  int r = cmd8();
  if (r != R1_IDLE_STATE) {
    return STA_NOINIT;
  }

  for (int i = 0; i < SD_COMMAND_TIMEOUT; i++) {
    uBit.sleep(50);
    cmd58();
    cmd(55, 0);
    if (cmd(41, 0x40000000) == 0) {
      cmd58();
      sd_init = 0;
      return disk_status(pdrv);
    }
  }

  return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
  for (uint32_t b = sector; b < sector + count; b++) {
    // set read address for single block (CMD17)
    if (cmd(17, b) != 0) {
      uBit.serial.printf("CMD17 ERROR!! sector = %d\r\n",sector);
      return RES_ERROR;
    }

    // receive the data
    read(buff, 512);
    buff += 512;
  }

  return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
  for (uint32_t b = sector; b < sector + count; b++) {
    // set write address for single block (CMD24)
    if (cmd(24, b) != 0) {
      return RES_ERROR;
    }

    // send the data block
    write(buff, 512);
    buff += 512;
  }

  return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
  return RES_OK;
}

DWORD get_fattime (
    void
)
{
  return (2018-1980)<<25|12<<21|1<<16;
}
