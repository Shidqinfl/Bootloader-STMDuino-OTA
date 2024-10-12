#ifndef NANDFLASH_H
#define NANDFLASH_H
#include <SPI.h>
// #include "SDCard/FatSF/FatSF.h" 
class nandFlash
{
private:
    /* data */
    // FatFs fs;
    // File file;
public:
    nandFlash(/* args */);
    int nand_flash_read(const struct fatfs_config *c, uint32_t block, uint32_t offset, void *buffer, uint32_t size);
    int nand_flash_write(const struct fatfs_config *c, uint32_t block, uint32_t offset, const void *buffer, uint32_t size);
    int nand_flash_erase(const struct fatfs_config *c, uint32_t block);
    int nand_flash_sync(const struct fatfs_config *c);
};

#endif