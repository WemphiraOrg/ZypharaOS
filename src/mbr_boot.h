#ifndef MBR_BOOT_H
#define MBR_BOOT_H
#include "io.h"
/* Escribe un MBR arrancable con GRUB stage1 embebido en el disco */
int  mbr_install(void);
int  mbr_write_partition_table(u32 disk_sectors);
/* Instala bootloader secundario en sector 1 */
int  boot2_install(void);
#endif
