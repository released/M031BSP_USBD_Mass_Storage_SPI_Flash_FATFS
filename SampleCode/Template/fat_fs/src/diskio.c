/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "SPI_Flash.h"
#include "misc_config.h"

extern uint32_t SpiReadMidDid(void);
extern void SpiInit(void);	
extern void SpiRead(uint32_t addr, uint32_t size, uint32_t buffer);	
extern void SpiWrite(uint32_t addr, uint32_t size, uint32_t buffer);

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	// int result;
 	uint32_t u32SpiMidDid;
 	
 	#if 1
	SpiInit();
	/* read MID & DID */
	u32SpiMidDid = SpiReadMidDid();
	if ((u32SpiMidDid & 0xFFFF) == 0xEF15)
	{	
		stat = 	RES_OK;
		// printf("%s : SPI FLASH Open success\n" , __FUNCTION__);
		return stat;
	}
	else
	{
		stat = STA_NOINIT;
		printf("%s : SPI FLASH Open failed\n" , __FUNCTION__);
		return stat;
	}	
 	#else
	switch (pdrv) {
	case DEV_RAM :
		result = RAM_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		result = MMC_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_USB :
		result = USB_disk_status();

		// translate the reslut code here

		return stat;
	}
	#endif
	// return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	// int result;
 	uint32_t u32SpiMidDid;
 	
 	#if 1
	SpiInit();
	/* read MID & DID */
	u32SpiMidDid = SpiReadMidDid();
	if ((u32SpiMidDid & 0xFFFF) == 0xEF15)
	{	
		stat = 	RES_OK;
		// printf("%s : SPI FLASH Open success\n" , __FUNCTION__);
		return stat;
	}
	else
	{
		stat = STA_NOINIT;
		printf("%s : SPI FLASH Open failed\n" , __FUNCTION__);
		return stat;
	}
	#else	
	switch (pdrv) {
	case DEV_RAM :
		result = RAM_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		result = MMC_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_USB :
		result = USB_disk_initialize();

		// translate the reslut code here

		return stat;
	}
	#endif
	// return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	// int result;
	uint32_t size,address;	

	#if 1
	if (pdrv) {
		res = (DRESULT)STA_NOINIT;	
		return res;
	}	
		
	if(count==0||count>=2)
	{	 
		res =   (DRESULT)STA_NOINIT;
		return res;
	}
	address= sector*512;
	size=count*512;
	SpiRead(address, size, (uint32_t)buff);			
	
	res =RES_OK;	/* Clear STA_NOINIT */;
	return res;
			
	#else
	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		result = RAM_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		result = MMC_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		result = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
	}
	#endif

	// return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	// int result;
	uint32_t size;
	uint32_t address;

	#if 1
 	if (pdrv) {
		res = (DRESULT)STA_NOINIT;	
		return res;
	}	     
    if(count==0||count>=2)
	{	 
		res = (DRESULT)  STA_NOINIT;
		return res;
	}
	    size=count*512;

		address= sector*512;
		SpiWrite(address, size,(uint32_t)buff);
		//SD_WriteDisk((unsigned char *)buff,sector,count);
	    res = RES_OK;

	return res;	
	#else
	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		result = RAM_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	}
	#endif

	// return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	// int result;

	#if 1
	if (pdrv) return RES_PARERR;

	switch (cmd) {
		case CTRL_SYNC :		/* Make sure that no pending write process */
			res = RES_OK;
			break;
	
		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			//DrvSDCARD_GetCardSize(buff);
			// *(DWORD*)buff = 4096;//4096*2;
			*(DWORD*)buff = 4096*2;		
			res = RES_OK;
			break;
	
		case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
			*(DWORD*)buff = 512;	//512;
			res = RES_OK;
			break;
	
		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			*(DWORD*)buff = 1;
			res = RES_OK;
			break;
	
	
		default:
			res = RES_PARERR;
	}


	res = RES_OK;


	return res;	
	#else
	switch (pdrv) {
	case DEV_RAM :

		// Process of the command for the RAM drive

		return res;

	case DEV_MMC :

		// Process of the command for the MMC/SD card

		return res;

	case DEV_USB :

		// Process of the command the USB drive

		return res;
	}
	#endif

	// return RES_PARERR;
}

unsigned long get_fattime (void)
{
	unsigned long tmr;

    tmr=0x00000;

	return tmr;
}

