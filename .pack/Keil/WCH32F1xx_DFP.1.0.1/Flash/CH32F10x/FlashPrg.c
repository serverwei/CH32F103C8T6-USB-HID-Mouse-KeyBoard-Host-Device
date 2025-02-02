/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for CH32F10x Flash               										 */
/*                                                                     */
/***********************************************************************/

#include "FlashOS.H"        // FlashOS Structures

typedef volatile unsigned char  vu8;
typedef volatile unsigned long  vu32;
typedef volatile unsigned short vu16;

#define M8(adr)  (*((vu8  *) (adr)))
#define M16(adr) (*((vu16 *) (adr)))
#define M32(adr) (*((vu32 *) (adr)))
#ifdef Debug_Flash
#define	   Rest() {*(uint32_t*)(0x40000000 + 0x10000 + 0x0800 + 20) = ((uint16_t)0x0001);}//Rest
#define		 Set()  {*(uint32_t *)(0x40000000 + 0x10000 + 0x0800 + 16) = ((uint16_t)0x0001);}//Set
#define     __IO      volatile 
typedef unsigned           int uint32_t;
typedef unsigned short     int uint16_t;
#endif

// Peripheral Memory Map
#define IWDG_BASE       0x40003000
#define FLASH_BASE      0x40022000

#define IWDG            ((IWDG_TypeDef *) IWDG_BASE)
#define FLASH           ((FLASH_TypeDef*) FLASH_BASE)

// FLASH BANK size
#define BANK1_SIZE      0x00080000      // Bank1 Size = 512kB

// Independent WATCHDOG
typedef struct {
  vu32 KR;
  vu32 PR;
  vu32 RLR;
  vu32 SR;
} IWDG_TypeDef;

// Flash Registers
typedef struct {
  vu32 ACR;                                     // offset  0x000
  vu32 KEYR;                                    // offset  0x004
  vu32 OPTKEYR;                                 // offset  0x008
  vu32 SR;                                      // offset  0x00C
  vu32 CR;                                      // offset  0x010
  vu32 AR;                                      // offset  0x014
  vu32 RESERVED0[1];
  vu32 OBR;                                     // offset  0x01C
  vu32 WRPR;                                    // offset  0x020
	vu32 MODEKEYP;																// offset  0x024 (CH692新增寄存器)
} FLASH_TypeDef;


// Flash Keys
#define RDPRT_KEY       0x5AA5
#define FLASH_KEY1      0x45670123
#define FLASH_KEY2      0xCDEF89AB

// Flash Control Register definitions
#define FLASH_PG        0x00000001
#define FLASH_PER       0x00000002
#define FLASH_MER       0x00000004
#define FLASH_OPTPG     0x00000010
#define FLASH_OPTER     0x00000020
#define FLASH_STRT      0x00000040
#define FLASH_LOCK      0x00000080
#define FLASH_OPTWRE    0x00000200
/* CH692的Flash_CR寄存器的新增位 */
#define FLASH_PAGE_PROGRAM	  0x00010000	//页编程（128Byte一页）
#define FLASH_PAGE_ERASE		  0x00020000	//页擦除（128Byte一页）
#define FLASH_STD_PAGE_ERASE  0x00000002  //标准页擦除（1024Byte 一页）
#define FLASH_STD_PAGE_PRG    0x00000001  //标准页编程（1024Byte 一页）
#define FLASH_BUF_LOAD			  0x00040000	//加载数据到FLASH内部缓冲区
#define FLASH_BUF_RTS				  0x00080000	//FLASH内部缓冲器复位

// Flash Status Register definitions
#define FLASH_BSY       0x00000001
#define FLASH_PGERR     0x00000004
#define FLASH_WRPRTERR  0x00000010
#define FLASH_EOP       0x00000020


unsigned long base_adr;


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */
 
 #ifdef Debug_Flash
 void GPIO_Toggle_INIT_reg2(void)
{
  uint32_t currentmode = 0x00, currentpin = 0x00, pinpos = 0x00, pos = 0x00;
  uint32_t tmpreg = 0x00, pinmask = 0x00;

  currentmode = 0x10 & ((uint32_t)0x0F);
	
	*(uint32_t *)(0x40000000 + 0x20000 + 0x1000+24) |= ((uint32_t)0x00000004);  
	
  if ((((uint32_t)0x10) & ((uint32_t)0x10)) != 0x00)
  { 
    currentmode |= (uint32_t)0x3;
  }

  if (((uint32_t)((uint16_t)0x0001) & ((uint32_t)0x00FF)) != 0x00)
  {
    tmpreg = *(uint32_t *)(0x40000000 + 0x10000 + 0x0800);
		
    for (pinpos = 0x00; pinpos < 0x08; pinpos++)
    {
      pos = ((uint32_t)0x01) << pinpos;
      currentpin = (((uint16_t)0x0001)) & pos;
			
      if (currentpin == pos)
      {
        pos = pinpos << 2;
        pinmask = ((uint32_t)0x0F) << pos;
        tmpreg &= ~pinmask;
        tmpreg |= (currentmode << pos);
    
      }
    }
    *(uint32_t*)(0x40000000 + 0x10000 + 0x0800) = tmpreg;
  }

  if (((uint16_t)0x0001) > 0x00FF)
  {
    tmpreg = *(uint32_t *)(0x40000000 + 0x10000 + 0x0800+4);
		
    for (pinpos = 0x00; pinpos < 0x08; pinpos++)
    {
      pos = (((uint32_t)0x01) << (pinpos + 0x08));
      currentpin = ((((uint16_t)0x0001)) & pos);
			
      if (currentpin == pos)
      {
        pos = pinpos << 2;
        pinmask = ((uint32_t)0x0F) << pos;
        tmpreg &= ~pinmask;
        tmpreg |= (currentmode << pos);

      }
    }
    *(uint32_t *)(0x40000000 + 0x10000 + 0x0800 + 4) = tmpreg;
  }
}
#endif

#ifdef FLASH_MEM
int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {
	
  base_adr = adr & ~(BANK1_SIZE - 1);          // Align to Size Boundary

  // Zero Wait State
  FLASH->ACR  = 0x00000000;

  // Unlock Flash    
  FLASH->KEYR  = FLASH_KEY1;
  FLASH->KEYR  = FLASH_KEY2;
	// 解锁快速模式（CH692中Flash新增寄存器）
	FLASH->MODEKEYP = FLASH_KEY1;
	FLASH->MODEKEYP = FLASH_KEY2;

  // Test if IWDG is running (IWDG in HW mode)
  if ((FLASH->OBR & 0x04) == 0x00) {
    // Set IWDG time out to ~32.768 second
    IWDG->KR  = 0x5555;                         // Enable write access to IWDG_PR and IWDG_RLR     
    IWDG->PR  = 0x06;                           // Set prescaler to 256  
    IWDG->RLR = 4095;                           // Set reload value to 4095
  }
 #ifdef Debug_Flash
  GPIO_Toggle_INIT_reg2();
#endif
	
  return (0);
} 
#endif

#ifdef FLASH_OPT
int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  // Zero Wait State
  FLASH->ACR  = 0x00000000;

  // Unlock Flash    
  FLASH->KEYR = FLASH_KEY1;
  FLASH->KEYR = FLASH_KEY2;

  // Unlock Option Bytes
  FLASH->OPTKEYR = FLASH_KEY1;
  FLASH->OPTKEYR = FLASH_KEY2;

  // Test if IWDG is running (IWDG in HW mode)
  if ((FLASH->OBR & 0x04) == 0x00) {
    // Set IWDG time out to ~32.768 second
    IWDG->KR  = 0x5555;                         // Enable write access to IWDG_PR and IWDG_RLR     
    IWDG->PR  = 0x06;                           // Set prescaler to 256  
    IWDG->RLR = 4095;                           // Set reload value to 4095
  }

  return (0);
}
#endif


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

#ifdef FLASH_MEM
int UnInit (unsigned long fnc) {

  // Lock Flash
  FLASH->CR  |=  FLASH_LOCK;

  return (0);
}
#endif

#ifdef FLASH_OPT
int UnInit (unsigned long fnc) {

  // Lock Flash & Option Bytes
  FLASH->CR &= ~FLASH_OPTWRE;
  FLASH->CR |=  FLASH_LOCK;

  return (0);
}
#endif


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

#ifdef FLASH_MEM
int EraseChip (void) {

  FLASH->CR  |=  FLASH_MER;                     // Mass Erase Enabled
  FLASH->CR  |=  FLASH_STRT;                    // Start Erase

  while (FLASH->SR  & FLASH_BSY) {
    IWDG->KR = 0xAAAA;                          // Reload IWDG
  }

  FLASH->CR  &= ~FLASH_MER;                     // Mass Erase Disabled

  return (0);                                   // Done
}
#endif

#ifdef FLASH_OPT
int EraseChip (void) {

  FLASH->CR |=  FLASH_OPTER;                    // Option Byte Erase Enabled 
  FLASH->CR |=  FLASH_STRT;                     // Start Erase

  while (FLASH->SR & FLASH_BSY) {
    IWDG->KR = 0xAAAA;                          // Reload IWDG
  }

  FLASH->CR &= ~FLASH_OPTER;                    // Option Byte Erase Disabled 

  // Unprotect Flash

  FLASH->CR |=  FLASH_OPTPG;                    // Option Byte Programming Enabled

  M16(0x1FFFF800) = RDPRT_KEY;                  // Program Half Word: RDPRT Key
  while (FLASH->SR & FLASH_BSY) {
    IWDG->KR = 0xAAAA;                          // Reload IWDG
  }

  FLASH->CR &= ~FLASH_OPTPG;                    // Option Byte Programming Disabled

  // Check for Errors
  if (FLASH->SR & (FLASH_PGERR | FLASH_WRPRTERR)) {
    FLASH->SR |= FLASH_PGERR | FLASH_WRPRTERR;
    return (1);                                 // Failed
  }

  return (0);                                   // Done
}
#endif


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

#ifdef FLASH_MEM
int EraseSector (unsigned long adr) {
		vu8 i = 0;
	#ifdef CH32F1xx_64_128
	  FLASH->CR  |=  FLASH_PAGE_ERASE;                // Page Erase Enabled （128Byte）
	  i = 1; 
	#endif
	#ifdef CH32F1xx_64_1024 
    FLASH->CR  |=  FLASH_STD_PAGE_ERASE;            // Page Erase Enabled （1024Byte）
	  i = 1; 
  #endif 
	do
	{
	  FLASH->AR   =  adr;                         // Page Address
    FLASH->CR  |=  FLASH_STRT;                  // Start Erase
    while (FLASH->SR  & FLASH_BSY) {
      IWDG->KR = 0xAAAA;                        // Reload IWDG
    }
	}while(--i);
	#ifdef CH32F1xx_64_128 
    FLASH->CR  &= ~FLASH_PAGE_ERASE;                       // Page Erase Disabled 	
	#endif
  #ifndef CH32F1xx_64_128 
    FLASH->CR  &= ~FLASH_STD_PAGE_ERASE;                   // Page Erase Disabled 	
	#endif
  return (0);                                   // Done
}
#endif

#ifdef FLASH_OPT
int EraseSector (unsigned long adr) {

  FLASH->CR |=  FLASH_OPTER;                    // Option Byte Erase Enabled 
  FLASH->CR |=  FLASH_STRT;                     // Start Erase

  while (FLASH->SR & FLASH_BSY) {
    IWDG->KR = 0xAAAA;                          // Reload IWDG
  }

  FLASH->CR &= ~FLASH_OPTER;                    // Option Byte Erase Disabled 

  return (0);                                   // Done
}
#endif


/*  
 *  Blank Check Checks if Memory is Blank
 *    Parameter:      adr:  Block Start Address
 *                    sz:   Block Size (in bytes)
 *                    pat:  Block Pattern
 *    Return Value:   0 - OK,  1 - Failed
 */

#ifdef FLASH_OPT
int BlankCheck (unsigned long adr, unsigned long sz, unsigned char pat) {
  return (1);                                   // Always Force Erase
}
#endif

/*
 *   Program128Byte in Flash Memory 
 *    Parameter:      adr:    Sector Address
 *                    buf:    program data
 *                    cr_reg: data in flash_cr reg
 *    Return Value:   0 - OK,  1 - Failed
 */
int Program128Byte (unsigned long adr, unsigned char *buf, unsigned long cr_reg) 
{
	unsigned long *p32;
	unsigned char i = 8;
//	unsigned int sz = 0;
	unsigned long addr = adr;

	*p32 = *((unsigned long*)buf);
	
	cr_reg &= 0x2ff|FLASH_PAGE_PROGRAM;	
	cr_reg |= FLASH_BUF_RTS;
	FLASH->CR  =  cr_reg;					// flashbuf复位
	while (FLASH->SR & FLASH_BSY); 
	
	cr_reg  &= ~FLASH_BUF_RTS;
	
//	FLASH->CR |= FLASH_BUF_RTS;
//	while (FLASH->SR & FLASH_BSY);	
	
	do{
			*(unsigned long *)(addr + 0) = *p32++;		
			*(unsigned long *)(addr + 4) = *p32++;
			*(unsigned long *)(addr + 8) = *p32++;
			*(unsigned long *)(addr + 12) = *p32++;			
			cr_reg |= FLASH_BUF_LOAD;
			FLASH->CR = cr_reg;						//加载数据到内部FLASH缓冲区		
//		   FLASH->CR |= FLASH_BUF_LOAD;
		   addr += 16;
			 while(FLASH->SR  & FLASH_BSY);		//等待闪存操作结束	        
	}while(--i);	
	cr_reg  &= ~FLASH_BUF_LOAD;	
	
  FLASH->AR = adr &(~0x7F);;									//编程页地址
	cr_reg |= FLASH_STRT;
	FLASH->CR = cr_reg;							//开始编程
//	while (FLASH->SR & FLASH_BSY);						//等待闪存操作结束
	cr_reg  &= ~FLASH_STRT;
	
//	FLASH->AR = adr &(~0x7F);
//	FLASH->CR |= FLASH_STRT;
	while (FLASH->SR & FLASH_BSY);
	
		
	if (FLASH->SR  & (FLASH_PGERR | FLASH_WRPRTERR)) 
	{
		 FLASH->SR  |= FLASH_PGERR | FLASH_WRPRTERR;
		 cr_reg &= 0x2ff;
		 FLASH->CR  =  cr_reg;	//  关掉编程模式
//		 FLASH->CR &= ~FLASH_PAGE_PROGRAM;
		 return (1);                             // Failed
	}
	return 0;
}

/*
 *   Program128Byte in Flash Memory 
 *    Parameter:      adr:    Sector Address
 *                    buf:    program data
 *                    sz :    data size
 *    Return Value:   0 - OK,  1 - Failed
 */
#ifdef FLASH_MEM
int ProgramPage(unsigned long adr, unsigned long sz, unsigned char *buf) 
{
	vu32 PAGE_ADD;
  unsigned char i = 0;
	unsigned long crreg;
	
	PAGE_ADD = adr;  //取8—31位，作为页地址
	i = (sz+127)/128;
	
	crreg = FLASH->CR;
	crreg |= FLASH_PAGE_PROGRAM;		
	FLASH->CR  =  crreg ;					//  打开快速编程模式
//    FLASH->CR |= FLASH_PAGE_PROGRAM;
	
	do
	{
		if(Program128Byte(PAGE_ADD, buf,crreg)) return (1);                               // Failed
		else
		{
			PAGE_ADD += 128;
			buf += 128;
		}
	}while(--i);
	
	crreg &= 0x2ff;
	FLASH->CR  =  crreg;	//  关掉编程模式
	
//	FLASH->CR &= ~FLASH_PAGE_PROGRAM;
	
	return (0);                                   // Done
}
#endif

#ifdef FLASH_OPT
int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {

  sz = (sz + 3) & ~3;                           // Adjust size for Half Words
  
  while (sz) {

    FLASH->CR |=  FLASH_OPTPG;                  // Option Byte Programming Enabled

    M16(adr) = *((unsigned short *)buf);        // Program Half Word
    while (FLASH->SR & FLASH_BSY) {
      IWDG->KR = 0xAAAA;                        // Reload IWDG
    }

    FLASH->CR &= ~FLASH_OPTPG;                  // Option Byte Programming Disabled

    // Check for Errors
    if (FLASH->SR & (FLASH_PGERR | FLASH_WRPRTERR)) {
      FLASH->SR |= FLASH_PGERR | FLASH_WRPRTERR;
      return (1);                               // Failed
    }

    // Go to next Half Word
    adr += 2;
    buf += 2;
    sz  -= 2;
   
  }

  return (0);                                   // Done
}
#endif
