#ifndef ATA_H
#define ATA_H

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

/* Puertos ATA primario */
#define ATA_PRIMARY_DATA    0x1F0
#define ATA_PRIMARY_ERROR   0x1F1
#define ATA_PRIMARY_SECTORS 0x1F2
#define ATA_PRIMARY_LBA0    0x1F3
#define ATA_PRIMARY_LBA1    0x1F4
#define ATA_PRIMARY_LBA2    0x1F5
#define ATA_PRIMARY_DRIVE   0x1F6
#define ATA_PRIMARY_CMD     0x1F7
#define ATA_PRIMARY_STATUS  0x1F7

/* Comandos ATA */
#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY      0xEC

/* Status bits */
#define ATA_SR_BSY  0x80  /* busy */
#define ATA_SR_DRDY 0x40  /* drive ready */
#define ATA_SR_DRQ  0x08  /* data request */
#define ATA_SR_ERR  0x01  /* error */

int  ata_init(void);
int  ata_read_sector(u32 lba, void* buf);
int  ata_write_sector(u32 lba, const void* buf);
u32  ata_disk_size(void);   /* sectores totales */

#endif
