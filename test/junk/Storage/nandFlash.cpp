#include "nandFlash.h"

#include "Configurator.h"
struct fatfs_config {
  // Define your FATFS configuration here if necessary
};

// Custom NAND Flash operations (implement based on your NAND flash specifics)
int nandFlash::nand_flash_read(const struct fatfs_config *c, uint32_t block, uint32_t offset, void *buffer, uint32_t size){
    
}
int nandFlash::nand_flash_write(const struct fatfs_config *c, uint32_t block, uint32_t offset, const void *buffer, uint32_t size){

}
int nandFlash::nand_flash_erase(const struct fatfs_config *c, uint32_t block){

}
int nandFlash::nand_flash_sync(const struct fatfs_config *c){

}
