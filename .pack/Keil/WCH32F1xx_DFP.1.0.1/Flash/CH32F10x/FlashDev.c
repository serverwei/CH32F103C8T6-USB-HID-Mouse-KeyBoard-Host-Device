/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for CH32F10x Flash            		 */
/*                                                                     */
/***********************************************************************/

#include "FlashOS.H"        // FlashOS Structures


#ifdef CH32F1xx_64_128
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "CH32F1xx_64_128 Flash",		 // Device Name (64kB)
   ONCHIP,                     // Device Type
   0x08000000,                 // Device Start Address
   0x00010000,                 // Device Size in Bytes (64kB)
   128,                       	// Programming Page Size 1K
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   5000,                        // Program Page Timeout 100 mSec
   10000,                        // Erase Sector Timeout 500 mSec

// Specify Size and Address of Sectors
   0x0080, 0x000000,           // Sector Size 128byte 512setors
   SECTOR_END
};
#endif


#ifdef CH32F1xx_64_1024
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "CH32F1xx Flash", 					// Device Name (64kB)
   ONCHIP,                     // Device Type
   0x08000000,                 // Device Start Address
   0x00010000,                 // Device Size in Bytes (64kB)
   1024,                       // Programming Page Size 1K
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   5000,                       // Program Page Timeout 100 mSec
   10000,                      // Erase Sector Timeout 500 mSec

// Specify Size and Address of Sectors
   0x0400, 0x000000,           // Sector Size 1024byte 64setors
   SECTOR_END
};
#endif


#ifdef FLASH_OPT
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "CH32F103 Flash Options",   // Device Name
   ONCHIP,                     // Device Type
   0x1FFFF800,                 // Device Start Address
   0x00000010,                 // Device Size in Bytes (16)
   16,                         // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   3000,                       // Program Page Timeout 3 Sec
   3000,                       // Erase Sector Timeout 3 Sec

// Specify Size and Address of Sectors
   0x0010, 0x000000,           // Sector Size 16B
   SECTOR_END
};
#endif
