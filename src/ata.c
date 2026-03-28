#include "ata.h"

static u32 disk_sectors = 0;

static inline void outb(u16 p, u8 v)  { __asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p)); }
static inline void outw(u16 p, u16 v) { __asm__ volatile("outw %0,%1"::"a"(v),"Nd"(p)); }
static inline u8  inb(u16 p)  { u8  v; __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p)); return v; }
static inline u16 inw(u16 p)  { u16 v; __asm__ volatile("inw %1,%0":"=a"(v):"Nd"(p)); return v; }

static void ata_400ns_delay(void) {
    /* Leer status 4 veces = ~400ns de delay */
    inb(ATA_PRIMARY_STATUS); inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS); inb(ATA_PRIMARY_STATUS);
}

static int ata_wait_ready(void) {
    int timeout = 100000;
    while (timeout--) {
        u8 s = inb(ATA_PRIMARY_STATUS);
        if (s & ATA_SR_ERR)  return -1;
        if (!(s & ATA_SR_BSY) && (s & ATA_SR_DRDY)) return 0;
    }
    return -2;  /* timeout */
}

static int ata_wait_drq(void) {
    int timeout = 100000;
    while (timeout--) {
        u8 s = inb(ATA_PRIMARY_STATUS);
        if (s & ATA_SR_ERR)  return -1;
        if (s & ATA_SR_DRQ)  return 0;
    }
    return -2;
}

int ata_init(void) {
    /* Reset software */
    outb(0x3F6, 0x04);  /* nIEN + SRST */
    ata_400ns_delay();
    outb(0x3F6, 0x00);  /* clear reset */
    ata_400ns_delay();

    if (ata_wait_ready() < 0) return -1;

    /* IDENTIFY para obtener tamaño del disco */
    outb(ATA_PRIMARY_DRIVE, 0xA0);  /* master */
    outb(ATA_PRIMARY_SECTORS, 0);
    outb(ATA_PRIMARY_LBA0, 0);
    outb(ATA_PRIMARY_LBA1, 0);
    outb(ATA_PRIMARY_LBA2, 0);
    outb(ATA_PRIMARY_CMD, ATA_CMD_IDENTIFY);

    u8 status = inb(ATA_PRIMARY_STATUS);
    if (status == 0) return -1;  /* no hay disco */

    if (ata_wait_drq() < 0) return -1;

    /* Leer 256 words del IDENTIFY */
    u16 identify[256];
    for (int i = 0; i < 256; i++)
        identify[i] = inw(ATA_PRIMARY_DATA);

    /* Words 60-61: LBA28 sector count */
    disk_sectors = (u32)identify[60] | ((u32)identify[61] << 16);

    return (int)disk_sectors;
}

u32 ata_disk_size(void) { return disk_sectors; }

static void ata_setup_lba(u32 lba, u8 sectors) {
    outb(ATA_PRIMARY_DRIVE,   0xE0 | ((lba >> 24) & 0x0F)); /* LBA mode, master */
    outb(ATA_PRIMARY_ERROR,   0x00);
    outb(ATA_PRIMARY_SECTORS, sectors);
    outb(ATA_PRIMARY_LBA0,    (u8)(lba & 0xFF));
    outb(ATA_PRIMARY_LBA1,    (u8)((lba >> 8)  & 0xFF));
    outb(ATA_PRIMARY_LBA2,    (u8)((lba >> 16) & 0xFF));
}

int ata_read_sector(u32 lba, void* buf) {
    if (ata_wait_ready() < 0) return -1;
    ata_setup_lba(lba, 1);
    outb(ATA_PRIMARY_CMD, ATA_CMD_READ_SECTORS);
    ata_400ns_delay();
    if (ata_wait_drq() < 0) return -1;

    u16* p = (u16*)buf;
    for (int i = 0; i < 256; i++)
        p[i] = inw(ATA_PRIMARY_DATA);
    return 0;
}

int ata_write_sector(u32 lba, const void* buf) {
    if (ata_wait_ready() < 0) return -1;
    ata_setup_lba(lba, 1);
    outb(ATA_PRIMARY_CMD, ATA_CMD_WRITE_SECTORS);
    ata_400ns_delay();
    if (ata_wait_drq() < 0) return -1;

    const u16* p = (const u16*)buf;
    for (int i = 0; i < 256; i++)
        outw(ATA_PRIMARY_DATA, p[i]);

    /* Flush cache */
    outb(ATA_PRIMARY_CMD, 0xE7);
    ata_wait_ready();
    return 0;
}
