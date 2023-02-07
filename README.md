# M031BSP_USBD_Mass_Storage_SPI_Flash_FATFS
 M031BSP_USBD_Mass_Storage_SPI_Flash_FATFS

update @ 2023/02/06

1. Initial SPI0 , to drive W25Q32JV ( 32MBit = 4MB ) , with USB FS + FATFS , to set MCU USB MSC with SPI flash storage 

	- FLASH_SS    D10     PA.3
		
	- FLASH_CLK   D13     PA.2
		
	- FLASH_MOSI  D11     PA.0
		
	- FLASH_MISO  D12     PA.1
		
	- FLASH_HOLD  D8      PA.5
		
	- FLASH_WP    D9      PA.4
	
2. reference sample code : http://forum.nuvoton.com/viewtopic.php?f=19&t=8477&sid=589ad6d3aac774f0541ed7ca770b2670
	
3. depend on SPI flash size , need to modify define : SPI_FLASH_STORAGE_SIZE (SPI_Flash.h)

	- ex : W25Q32JV ( 32MBit = 4MB ) , modify SPI_FLASH_STORAGE_SIZE to (4*1024*1024)

4. under KEIL > OPTION , TAB : arm

set stack size to 0x800 and heap size to 0x100

![image](https://github.com/released/M031BSP_USBD_Mass_Storage_SPI_Flash_FATFS/blob/main/keil_option.jpg)

5. use UART terminal , digit ( 1 ~ 6 ) , to test FATFS function

	- digit 1 , 2: create file and write data into file
	
	- digit 3: create file (long file name) and write data into file
	
	- digit 4 , 5: scan file function
	
	- digit 6 : erase SPI flash
	
6. below is log capture 

![image](https://github.com/released/M031BSP_USBD_Mass_Storage_SPI_Flash_FATFS/blob/main/noram_power_on.jpg)

create file and write data into file

![image](https://github.com/released/M031BSP_USBD_Mass_Storage_SPI_Flash_FATFS/blob/main/TEST1.jpg)

![image](https://github.com/released/M031BSP_USBD_Mass_Storage_SPI_Flash_FATFS/blob/main/TEST2.jpg)


create file (long file name) and write data into file

![image](https://github.com/released/M031BSP_USBD_Mass_Storage_SPI_Flash_FATFS/blob/main/TEST3.jpg)


scan file function 1 : 

![image](https://github.com/released/M031BSP_USBD_Mass_Storage_SPI_Flash_FATFS/blob/main/TEST4.jpg)


scan file function 2 :

![image](https://github.com/released/M031BSP_USBD_Mass_Storage_SPI_Flash_FATFS/blob/main/TEST5.jpg)


erase SPI flash , and press digit : z to reset MCU

![image](https://github.com/released/M031BSP_USBD_Mass_Storage_SPI_Flash_FATFS/blob/main/TEST6.jpg)


below is WINDOWS file manager display , 

![image](https://github.com/released/M031BSP_USBD_Mass_Storage_SPI_Flash_FATFS/blob/main/windows.jpg)


