/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#include "misc_config.h"

#include "massstorage.h"
#include "SPI_Flash.h"

#include "diskio.h"
#include "ff.h"
/*_____ D E C L A R A T I O N S ____________________________________________*/

struct flag_32bit flag_PROJ_CTL;
#define FLAG_PROJ_TIMER_PERIOD_1000MS                 	(flag_PROJ_CTL.bit0)
#define FLAG_PROJ_REVERSE1                   			(flag_PROJ_CTL.bit1)
#define FLAG_PROJ_REVERSE2                 				(flag_PROJ_CTL.bit2)
#define FLAG_PROJ_REVERSE3                              (flag_PROJ_CTL.bit3)
#define FLAG_PROJ_REVERSE4                              (flag_PROJ_CTL.bit4)
#define FLAG_PROJ_REVERSE5                              (flag_PROJ_CTL.bit5)
#define FLAG_PROJ_REVERSE6                              (flag_PROJ_CTL.bit6)
#define FLAG_PROJ_REVERSE7                              (flag_PROJ_CTL.bit7)

struct flag_32bit flag_Fatfs_CTL;
#define FLAG_FATFS_TEST1                     	        (flag_Fatfs_CTL.bit0)
#define FLAG_FATFS_TEST2                   			    (flag_Fatfs_CTL.bit1)
#define FLAG_FATFS_TEST3                 		        (flag_Fatfs_CTL.bit2)
#define FLAG_FATFS_TEST4                                (flag_Fatfs_CTL.bit3)
#define FLAG_FATFS_TEST5                                (flag_Fatfs_CTL.bit4)
#define FLAG_FATFS_TEST6                                (flag_Fatfs_CTL.bit5)
#define FLAG_FATFS_REVERSE6                             (flag_Fatfs_CTL.bit6)
#define FLAG_FATFS_REVERSE7                             (flag_Fatfs_CTL.bit7)

/*_____ D E F I N I T I O N S ______________________________________________*/

volatile unsigned int counter_systick = 0;
volatile uint32_t counter_tick = 0;

#define CRYSTAL_LESS                                    1    /* CRYSTAL_LESS must be 1 if USB clock source is HIRC */
#define TRIM_INIT                                       (SYS_BASE+0x118)    
uint32_t u32TrimInit;

FATFS g_sFatFs;
FATFS *fs;	
FIL f1;
DWORD fre_clust;
DWORD acc_size;                         /* Work register for fs command */
WORD acc_files, acc_dirs;
FILINFO Finfo;
TCHAR _Path[3] = { '0', ':', 0 };

// char Line[256];                         /* Console input buffer */
#if FF_USE_LFN >= 1
char Lfname[512];
#endif

// uint16_t counter = 0;
UINT uiWriteLen;
UINT uiReadLen;

uint8_t Buffer_Block_Rx[512] = {0};
const uint8_t Power_Buffer[] = "Added selection of character encoding on the file. (_STRF_ENCODE)Added f_closedir().\
Added forced full FAT scan for f_getfree(). (_FS_NOFSINFO)Added forced mount feature with changes of f_mount().\
Improved behavior of volume auto detection.Improved write throughput of f_puts() and f_printf().\
Changed argument of f_chdrive(), f_mkfs(), disk_read() and disk_write().Fixed f_write() can be truncated when the file size is close to 4GB.\
Fixed f_open, f_mkdir and f_setlabel can return incorrect error code.The STM32F427xx and STM32F429xx devices are based on the \
high-performance ARMRCortexR-M4 32-bit RISC core operating at a frequency of up to 180 MHz. The Cortex-M4 core features a Floating \
point unit (FPU) single precision which supports all ARMR single-precision data-processing instructions and data types. It also implements \
a full set of DSP instructions and a memory protection unit (MPU) which enhances application security.\
The STM32F427xx and STM32F429xx devices incorporate high-speed embedded memories (Flash memory up to 2 Mbyte, up to 256 kbytes of \
SRAM), up to 4 Kbytes of backup SRAM, and an extensive range of enhanced I/Os and peripherals connected to two APB buses, two AHB buses \
and a 32-bit multi-AHB bus matrix.All devices offer three 12-bit ADCs, two DACs, a low-power RTC, twelve general-purpose 16-bit timers \
including two PWM timers for motor control, two general-purpose 32-bit timers.";

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

unsigned int get_systick(void)
{
	return (counter_systick);
}

void set_systick(unsigned int t)
{
	counter_systick = t;
}

void systick_counter(void)
{
	counter_systick++;
}

void SysTick_Handler(void)
{

    systick_counter();

    if (get_systick() >= 0xFFFFFFFF)
    {
        set_systick(0);      
    }

    // if ((get_systick() % 1000) == 0)
    // {
       
    // }

    #if defined (ENABLE_TICK_EVENT)
    TickCheckTickEvent();
    #endif    
}

void SysTick_delay(unsigned int delay)
{  
    
    unsigned int tickstart = get_systick(); 
    unsigned int wait = delay; 

    while((get_systick() - tickstart) < wait) 
    { 
    } 

}

void SysTick_enable(unsigned int ticks_per_second)
{
    set_systick(0);
    if (SysTick_Config(SystemCoreClock / ticks_per_second))
    {
        /* Setup SysTick Timer for 1 second interrupts  */
        dbg_printf("Set system tick error!!\n");
        while (1);
    }

    #if defined (ENABLE_TICK_EVENT)
    TickInitTickEvent();
    #endif
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void tick_counter(void)
{
	counter_tick++;
    if (get_tick() >= 60000)
    {
        set_tick(0);
    }
}

// void delay_ms(uint16_t ms)
// {
// 	TIMER_Delay(TIMER0, 1000*ms);
// }


void put_rc(FRESULT rc)
{
    uint32_t i;
    const TCHAR *p =
        _T("OK\0DISK_ERR\0INT_ERR\0NOT_READY\0NO_FILE\0NO_PATH\0INVALID_NAME\0")
        _T("DENIED\0EXIST\0INVALID_OBJECT\0WRITE_PROTECTED\0INVALID_DRIVE\0")
        _T("NOT_ENABLED\0NO_FILE_SYSTEM\0MKFS_ABORTED\0TIMEOUT\0LOCKED\0")
        _T("NOT_ENOUGH_CORE\0TOO_MANY_OPEN_FILES\0");

    for (i = 0; (i != (UINT)rc) && *p; i++)
    {
        while (*p++) ;
    }

    printf(_T("rc=%u FR_%s\n"), (UINT)rc, p);
}

FRESULT get_free (char* path)	//Show logical drive status
{
	// DWORD fre_sect, tot_sect;
	const BYTE ft[] = {0, 12, 16, 32};

	FRESULT res = FR_INVALID_PARAMETER;	
	res = f_getfree(path, (DWORD*)&fre_clust, &fs);
	put_rc(res);

    // tot_sect = (fs->n_fatent - 2) * fs->csize;
    // fre_sect = fre_clust * fs->csize;
	// printf("%10lu KiB total drive space.\r\n%10lu KiB available.\r\n\r\n", tot_sect / 2, fre_sect / 2);

	printf("FAT type = FAT%d\nBytes/Cluster = %d\nNumber of FATs = %d\n"
			"Root DIR entries = %d\nSectors/FAT = %d\nNumber of clusters = %d\n"
			"FAT start (lba) = %d\nDIR start (lba,clustor) = %d\nData start (lba) = %d\n\n...",
			ft[fs->fs_type & 3], fs->csize * 512UL, fs->n_fats,
			fs->n_rootdir, fs->fsize, fs->n_fatent - 2,
			fs->fatbase, fs->dirbase, fs->database
			);
	acc_size = acc_files = acc_dirs = 0;
// #if FF_USE_LFN >= 1
// 	Finfo.lfname = Lfname;
// 	Finfo.lfsize = sizeof(Lfname);
// #endif

    return res;
}


int do_dir(void)
{
    long    p1;                             /* total file size counter                    */
    FRESULT res;                            /* FATFS operation return code                */

    DIR dir;                                /* FATFS directory object                     */
    UINT s1, s2;                            /* file and directory counter                 */

    if (f_opendir(&dir, _Path))         /* try to open USB drive root directory       */
        return -1;                          /* open failed                                */

    p1 = s1 = s2 = 0;                       /* initialize counters                        */
    for (; ;)                               /* loop until reached end of root directory   */
    {
        res = f_readdir(&dir, &Finfo);      /* read directory entry                       */
        if ((res != FR_OK) || !Finfo.fname[0]) break;  /* no more entries                 */
        if (Finfo.fattrib & AM_DIR)         /* is a directory?                            */
        {
            s2++;                           /* increase directory counter                 */
        }
        else                                /* should be a file                           */
        {
            s1++;                           /* increase file counter                      */
            p1 += Finfo.fsize;              /* increase total file size counter           */
        }
        /* print file entry information               */
        printf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s",
               (Finfo.fattrib & AM_DIR) ? 'D' : '-',    /* is a directory?                */
               (Finfo.fattrib & AM_RDO) ? 'R' : '-',    /* is read-only?                  */
               (Finfo.fattrib & AM_HID) ? 'H' : '-',    /* is hidden?                     */
               (Finfo.fattrib & AM_SYS) ? 'S' : '-',    /* is system file/directory?      */
               (Finfo.fattrib & AM_ARC) ? 'A' : '-',    /* is an archive?                 */
               (Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,       /* date */
               (Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63, Finfo.fsize, Finfo.fname);    /* time */
#if _USE_LFN
        for (p2 = strlen(Finfo.fname); p2 < 14; p2++)   /* print the long file name       */
            putchar(' ');
        printf("%s\n", Lfname);
#else
        putchar('\n');
#endif
    }
    /* print the statistic information            */
    printf("%4u File(s),%10lu bytes total\n%4u Dir(s)", s1, p1, s2);
    return 0;
}


FRESULT scan_files (char* path)
{
    DIR dirs;
    FRESULT res;
    BYTE i;
    char *fn;

    if ((res = f_opendir(&dirs, path)) == FR_OK)
    {
        i = strlen(path);
        while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0])
        {
            if (FF_FS_RPATH && Finfo.fname[0] == '.') continue;
// #if FF_USE_LFN >= 1
            // fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname;
// #else
            fn = Finfo.fname;
// #endif
            if (Finfo.fattrib & AM_DIR)
            {
                acc_dirs++;
                *(path+i) = '/';
                strcpy(path+i+1, fn);
                res = scan_files(path);
                *(path+i) = '\0';
                if (res != FR_OK) break;
            }
            else
            {
                printf("%s/%s\r\n", path, fn);
                acc_files++;
                acc_size += Finfo.fsize;
            }
        }
    }

    return res;
}


void Fatfs_scan_files(void)
{

	FRESULT res = FR_INVALID_PARAMETER;

	get_free(_Path);
	put_rc(res);

	res = scan_files(_Path);
	put_rc(res);

	printf("\r%d files, %d bytes.\n%d folders.\n"
			"%d KB total disk space.\n%d KB available.\n",
			acc_files, acc_size, acc_dirs,
			(fs->n_fatent - 2) * (fs->csize / 2), fre_clust * (fs->csize / 2)
			);
}

void Fatfs_ReadWriteTest(char* File , uint8_t* Buffin , uint8_t* Buffout , uint32_t Len)
{
	uint32_t i = 0 ;
	FRESULT res = FR_INVALID_PARAMETER;

	printf(">>>>>Fatfs_ReadWriteTest  START\r\n\r\n");
	res = f_open(&f1, File, FA_CREATE_ALWAYS | FA_WRITE  | FA_READ);
	put_rc(res);
	// printf("<Fatfs_Write>	f_open res : %d \r\n" , res);
	
	if(res !=FR_OK)
	{
		printf("<Fatfs_Write>	f_open NG  \r\n");
	}	
	else
	{
		printf("<Fatfs_Write>	f_open OK  \r\n");
		printf("<Fatfs_Write>	Write Data to File!!!\r\n");
		res = f_write(&f1, Buffin, Len, &uiWriteLen);
		put_rc(res);
		if(res !=FR_OK)
		{
			printf("<Fatfs_Write>	Write [%s] NG\r\n" , File);
		}		
		else
		{
			printf("<Fatfs_Write>	Write [%s] OK\r\n" , File);
		}

		res = f_close(&f1);
		put_rc(res);
		if(res != FR_OK)
		{
			printf("<Fatfs_Write>	f_close NG\r\n\r\n");
		}
		else
		{
			printf("<Fatfs_Write>	f_close OK\r\n\r\n");
		}
	}	

	res = f_open(&f1, File, FA_READ);
	put_rc(res);
	if(res != FR_OK)
	{
		printf("<Fatfs_Read>	f_open NG  \r\n");
	}	
	else
	{
		printf("\r\n<Fatfs_Read>	f_open OK  \r\n");
		printf("<Fatfs_Read>	Read Data to Buffout!!!\r\n");

		res = f_read(&f1, Buffout, Len, &uiReadLen);
		put_rc(res);
		if(res != FR_OK)
		{
			printf("<Fatfs_Read>	Read [%s] NG\r\n",File);
		}		
		else
		{
			printf("<Fatfs_Read>	Read [%s] OK\r\n",File);
		}

		res = f_close(&f1);
		put_rc(res);
		if(res != FR_OK)
		{
			printf("<Fatfs_Read>	f_close NG\r\n\r\n");
		}
		else
		{
			printf("<Fatfs_Read>	f_close OK\r\n\r\n");
		}
	}	

	printf("uiReadLen = %d \r\n",Len);
	printf("Data = ");
	printf("\r\n");
	for(i=0;i<Len;i++)
	{
		printf("%c",Buffout[i]);
	}
	printf("\r\n\r\n");

	printf("<<<<<Fatfs_ReadWriteTest  FINISH\r\n");	
}

void Fatfs_Test(void)
{
	reset_buffer(Buffer_Block_Rx , 0x00 , 512);
	Fatfs_ReadWriteTest("1218DATA.TXT", (uint8_t*) Power_Buffer,Buffer_Block_Rx,512);

	Fatfs_ReadWriteTest("0311DATA.TXT", (uint8_t*) Power_Buffer,Buffer_Block_Rx,512);

}

void Fatfs_Init(void)
{
	BYTE work[FF_MAX_SS];
	FRESULT res = FR_INVALID_PARAMETER;	

	#if 0
	printf("erase flash\r\n");
	SpiChipErase();	
	#endif

	res = f_mount(&g_sFatFs, _Path ,1 );
	put_rc(res);
	if(res != FR_OK)	//Mount Logical Drive 
	{
		printf("f_mount NG , start erase flash\r\n");

		//if mount , erase flash and reset fatfs
		SpiChipErase();	

		res = f_mkfs(_Path , 0 , work , sizeof(work));
		printf("f_mkfs process\r\n\r\n");

	}
	else
	{
		printf("f_mount OK\r\n\r\n");
	}
}

/*
	WP : PA4 (D9)
	HOLD : PA5 (D8)
*/
void SPIFlash_WP_HOLD_Init (void)
{
    SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA4MFP_Msk)) | (SYS_GPA_MFPL_PA4MFP_GPIO);
    SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA5MFP_Msk)) | (SYS_GPA_MFPL_PA5MFP_GPIO);
	
    GPIO_SetMode(PA, BIT4, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PA, BIT5, GPIO_MODE_OUTPUT);	

	PA4 = 1;
	PA5 = 1;		
}

void usb_trim_process(void)
{
    /* Start USB trim if it is not enabled. */
    if((SYS->HIRCTRIMCTL & SYS_HIRCTRIMCTL_FREQSEL_Msk) != 1)
    {
        /* Start USB trim only when SOF */
        if(USBD->INTSTS & USBD_INTSTS_SOFIF_Msk)
        {
            /* Clear SOF */
            USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;

            /* Re-enable crystal-less */
            SYS->HIRCTRIMCTL = 0x01;
            SYS->HIRCTRIMCTL |= SYS_HIRCTRIMCTL_REFCKSEL_Msk;
        }
    }

    /* Disable USB Trim when error */
    if(SYS->HIRCTRIMSTS & (SYS_HIRCTRIMSTS_CLKERIF_Msk | SYS_HIRCTRIMSTS_TFAILIF_Msk))
    {
        /* Init TRIM */
        M32(TRIM_INIT) = u32TrimInit;

        /* Disable crystal-less */
        SYS->HIRCTRIMCTL = 0;

        /* Clear error flags */
        SYS->HIRCTRIMSTS = SYS_HIRCTRIMSTS_CLKERIF_Msk | SYS_HIRCTRIMSTS_TFAILIF_Msk;

        /* Clear SOF */
        USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;
    }
}

void usb_msc_Init(void)
{
    /* initialize USBD */
    USBD_Open(&gsInfo, MSC_ClassRequest, NULL);

    USBD_SetConfigCallback(MSC_SetConfig);

    /* Endpoint configuration */
    MSC_Init();
    USBD_Start();

    SYS_UnlockReg();
    if((SYS->HIRCTRIMCTL & SYS_HIRCTRIMCTL_FREQSEL_Msk) != 1)
    {
        /* Start USB trim */
        USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;
        while((USBD->INTSTS & USBD_INTSTS_SOFIF_Msk) == 0);

        /* Re-enable crystal-less */
        SYS->HIRCTRIMCTL = 0x01;
        SYS->HIRCTRIMCTL |= SYS_HIRCTRIMCTL_REFCKSEL_Msk;
        
        /* Backup default trim */
        u32TrimInit = M32(TRIM_INIT);
    }

    NVIC_EnableIRQ(USBD_IRQn);

}

void masstorage_process(void)
{
   MSC_ProcessCmd();
}

void TMR1_IRQHandler(void)
{
	
    if(TIMER_GetIntFlag(TIMER1) == 1)
    {
        TIMER_ClearIntFlag(TIMER1);
		tick_counter();

		if ((get_tick() % 1000) == 0)
		{
            FLAG_PROJ_TIMER_PERIOD_1000MS = 1;//set_flag(flag_timer_period_1000ms ,ENABLE);
		}

		if ((get_tick() % 50) == 0)
		{

		}	
    }
}

void TIMER1_Init(void)
{
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);	
    TIMER_Start(TIMER1);
}

void loop(void)
{
	// static uint32_t LOG1 = 0;
	// static uint32_t LOG2 = 0;

    if ((get_systick() % 1000) == 0)
    {
        // dbg_printf("%s(systick) : %4d\r\n",__FUNCTION__,LOG2++);    
    }

    if (FLAG_PROJ_TIMER_PERIOD_1000MS)//(is_flag_set(flag_timer_period_1000ms))
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 0;//set_flag(flag_timer_period_1000ms ,DISABLE);

        // dbg_printf("%s(timer) : %4d\r\n",__FUNCTION__,LOG1++);
        PB14 ^= 1;        
    }


	if (FLAG_FATFS_TEST1)
	{
		FLAG_FATFS_TEST1 = 0;
        printf("\r\n[FLAG_FATFS_TEST1]\r\n");
		reset_buffer(Buffer_Block_Rx , 0x00 , 512);
		Fatfs_ReadWriteTest("20220311.TXT", (uint8_t*) Power_Buffer,Buffer_Block_Rx,512);			
	}
	if (FLAG_FATFS_TEST2)
	{
		FLAG_FATFS_TEST2 = 0;
        printf("\r\n[FLAG_FATFS_TEST2]\r\n");
		reset_buffer(Buffer_Block_Rx , 0x00 , 512);
		Fatfs_ReadWriteTest("customfile_aaa.TXT", (uint8_t*) Power_Buffer,Buffer_Block_Rx,512);
	}
	if (FLAG_FATFS_TEST3)
	{
		FLAG_FATFS_TEST3 = 0;
        printf("\r\n[FLAG_FATFS_TEST3]\r\n");
		reset_buffer(Buffer_Block_Rx , 0x00 , 512);
		Fatfs_ReadWriteTest("test_longlonglonglong_files_name_need_to_enable_FF_USE_LFN_code_size_will_increase.TXT", (uint8_t*) Power_Buffer,Buffer_Block_Rx,512);
	}
	if (FLAG_FATFS_TEST4)
	{
		FLAG_FATFS_TEST4 = 0;
        printf("\r\n[FLAG_FATFS_TEST4]\r\n");
		Fatfs_scan_files();
	}
	if (FLAG_FATFS_TEST5)
	{
		FLAG_FATFS_TEST5 = 0;
        printf("\r\n[FLAG_FATFS_TEST5]\r\n");
        if (do_dir() < 0)  
        {
            printf("Cannot open root directory.\r\n");
        }
	}
	if (FLAG_FATFS_TEST6)
	{
		FLAG_FATFS_TEST6 = 0;
        printf("erase flash\r\n");
        SpiChipErase();	
        printf("erase flash (done)\r\n");
	}           

    usb_trim_process();
    masstorage_process();

}

void UARTx_Process(void)
{
	uint8_t res = 0;
	res = UART_READ(UART0);

	if (res > 0x7F)
	{
		dbg_printf("invalid command\r\n");
	}
	else
	{
		dbg_printf("press : %c\r\n" , res);
		switch(res)
		{
			case '1':
                FLAG_FATFS_TEST1 = 1;
				break;
			case '2':
                FLAG_FATFS_TEST2 = 1;
				break;
			case '3':
                FLAG_FATFS_TEST3 = 1;
				break;
			case '4':
                FLAG_FATFS_TEST4 = 1;
				break;
			case '5':
                FLAG_FATFS_TEST5 = 1;
				break;
			case '6':
                FLAG_FATFS_TEST6 = 1;
				break;

			case 'X':
			case 'x':
			case 'Z':
			case 'z':
                SYS_UnlockReg();
				// NVIC_SystemReset();	// Reset I/O and peripherals , only check BS(FMC_ISPCTL[1])
                // SYS_ResetCPU();     // Not reset I/O and peripherals
                SYS_ResetChip();    // Reset I/O and peripherals ,  BS(FMC_ISPCTL[1]) reload from CONFIG setting (CBS)	
				break;
		}
	}
}

void UART02_IRQHandler(void)
{
    if(UART_GET_INT_FLAG(UART0, UART_INTSTS_RDAINT_Msk | UART_INTSTS_RXTOINT_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(UART0) == 0)
        {
			UARTx_Process();
        }
    }

    if(UART0->FIFOSTS & (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk))
    {
        UART_ClearIntFlag(UART0, (UART_INTSTS_RLSINT_Msk| UART_INTSTS_BUFERRINT_Msk));
    }	
}

void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
    UART_EnableInt(UART0, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
    NVIC_EnableIRQ(UART02_IRQn);
	
	#if (_debug_log_UART_ == 1)	//debug
	dbg_printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	dbg_printf("CLK_GetHCLKFreq : %8d\r\n",CLK_GetHCLKFreq());
	dbg_printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	dbg_printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	dbg_printf("CLK_GetPCLK0Freq : %8d\r\n",CLK_GetPCLK0Freq());
	dbg_printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK1Freq());	
	#endif	

    #if 0
    dbg_printf("FLAG_PROJ_TIMER_PERIOD_1000MS : 0x%2X\r\n",FLAG_PROJ_TIMER_PERIOD_1000MS);
    dbg_printf("FLAG_PROJ_REVERSE1 : 0x%2X\r\n",FLAG_PROJ_REVERSE1);
    dbg_printf("FLAG_PROJ_REVERSE2 : 0x%2X\r\n",FLAG_PROJ_REVERSE2);
    dbg_printf("FLAG_PROJ_REVERSE3 : 0x%2X\r\n",FLAG_PROJ_REVERSE3);
    dbg_printf("FLAG_PROJ_REVERSE4 : 0x%2X\r\n",FLAG_PROJ_REVERSE4);
    dbg_printf("FLAG_PROJ_REVERSE5 : 0x%2X\r\n",FLAG_PROJ_REVERSE5);
    dbg_printf("FLAG_PROJ_REVERSE6 : 0x%2X\r\n",FLAG_PROJ_REVERSE6);
    dbg_printf("FLAG_PROJ_REVERSE7 : 0x%2X\r\n",FLAG_PROJ_REVERSE7);
    #endif

}

void GPIO_Init (void)
{
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB14MFP_Msk)) | (SYS_GPB_MFPH_PB14MFP_GPIO);
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB15MFP_Msk)) | (SYS_GPB_MFPH_PB15MFP_GPIO);

    GPIO_SetMode(PB, BIT14, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PB, BIT15, GPIO_MODE_OUTPUT);	

}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);
    
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

//    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

//    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);	

//    CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);	

    /* Select HCLK clock source as HIRC and HCLK source divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    // CLK_EnableModuleClock(TMR0_MODULE);
  	// CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);

    CLK_EnableModuleClock(TMR1_MODULE);
  	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_HIRC, 0);

    /* Switch UART0 clock source to HIRC */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Enable UART0 clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Switch USB clock source to HIRC & USB Clock = HIRC / 1 */
    CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL0_USBDSEL_HIRC, CLK_CLKDIV0_USB(1));

    /* Enable USB clock */
    CLK_EnableModuleClock(USBD_MODULE);

    /* Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk)) |
                    (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);

   /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();
}

/*
 * This is a template project for M031 series MCU. Users could based on this project to create their
 * own application without worry about the IAR/Keil project settings.
 *
 * This template application uses external crystal as HCLK source and configures UART0 to print out
 * "Hello World", users may need to do extra system configuration based on their system design.
 */

int main()
{
	unsigned int MidDid;
    uint8_t spi_temp[4]= {0xFF};

    SYS_Init();

	GPIO_Init();
	UART0_Init();
	TIMER1_Init();

    SysTick_enable(1000);
    #if defined (ENABLE_TICK_EVENT)
    TickSetTickEvent(1000, TickCallback_processA);  // 1000 ms
    TickSetTickEvent(5000, TickCallback_processB);  // 5000 ms
    #endif


    /*
        FLASH_SS    D10     PA.3
        FLASH_CLK   D13     PA.2
        FLASH_MOSI  D11     PA.0
        FLASH_MISO  D12     PA.1
        FLASH_HOLD  D8      PA.5
        FLASH_WP    D9      PA.4

    */

	SPIFlash_WP_HOLD_Init();
    SpiInit();

    /* Read MID & DID */
    MidDid = SpiReadMidDid();
    printf("\r\nMID and DID = 0x%4X\r\n", MidDid);

    if(SPI_FlashReadID(spi_temp)) 
    {
        printf("SPI FLASH Open success.(%X-%X-%X)\n",spi_temp[0],spi_temp[1],spi_temp[2]);
    } else 
    {
        printf("SPI FLASH Open failed. (%X-%X-%X)\n",spi_temp[0],spi_temp[1],spi_temp[2]);
        //        printf("SPI FLASH Open failed %X\n",u32SpiMidDid);
    }

	Fatfs_Init();

    usb_msc_Init();

    /* Got no where to go, just loop forever */
    while(1)
    {
        loop();

    }
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
