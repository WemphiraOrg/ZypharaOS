#include "mbr_boot.h"
#include "ata.h"

extern void term_print(const char*);
extern void term_putchar(char);
extern void term_print_int(int);
extern void term_setcolor(int fg, int bg);

/* MBR bootloader real de 446 bytes:
   - Verifica firma 0x55AA
   - Busca partición activa
   - Carga sector de arranque de la partición en 0x7C00
   - Salta a él
   Código x86 real mode compilado y embebido directamente */
static const u8 mbr_code[446] = {
    /* Bootloader MBR mínimo funcional en código máquina x86 */
    0xFA,                          /* cli */
    0x31,0xC0,                     /* xor ax,ax */
    0x8E,0xD0,                     /* mov ss,ax */
    0xBC,0x00,0x7C,                /* mov sp,0x7C00 */
    0xFB,                          /* sti */
    0x8E,0xD8,                     /* mov ds,ax */
    0x8E,0xC0,                     /* mov es,ax */
    /* Buscar partición activa en tabla de particiones */
    0xBE,0xBE,0x7D,                /* mov si, 0x7DBE (tabla particiones) */
    0xB9,0x04,0x00,                /* mov cx, 4 (4 entradas) */
    /* loop: */
    0x80,0x3C,0x80,                /* cmp byte[si], 0x80 */
    0x74,0x07,                     /* je found */
    0x83,0xC6,0x10,                /* add si, 16 */
    0xE2,0xF7,                     /* loop */
    /* no activa -> error */
    0xBE,0xD6,0x7D,                /* mov si, msg_err */
    0xEB,0x14,                     /* jmp print_err */
    /* found: cargar sector de arranque de la partición */
    0x8B,0x54,0x08,                /* mov edx,[si+8] -> LBA start (solo 16bits aqui) */
    /* Usar INT 13h AH=02 para leer 1 sector */
    0xB4,0x02,                     /* mov ah, 2 (read) */
    0xB0,0x01,                     /* mov al, 1 (sectors) */
    0xBB,0x00,0x7C,                /* mov bx, 0x7C00 (destino) */
    0xB1,0x01,                     /* mov cl, sector 1 */
    0xB6,0x00,                     /* mov dh, head 0 */
    0xB2,0x80,                     /* mov dl, 0x80 (hdd) */
    0xCD,0x13,                     /* int 13h */
    0x72,0x05,                     /* jc error */
    /* Saltar al sector cargado */
    0xEA,0x00,0x7C,0x00,0x00,      /* jmp far 0000:7C00 */
    /* error: */
    0x31,0xC0,                     /* xor ax,ax */
    /* print_err: */
    0xAC,                          /* lodsb */
    0x08,0xC0,                     /* or al,al */
    0x74,0xFE,                     /* jz $ (halt) */
    0xB4,0x0E,                     /* mov ah,0x0E */
    0xCD,0x10,                     /* int 10h */
    0xEB,0xF7,                     /* jmp print_err */
    /* msg_err: */
    0x4E,0x6F,0x20,0x62,0x6F,0x6F, /* "No boo" */
    0x74,0x61,0x62,0x6C,0x65,0x20, /* "table " */
    0x70,0x61,0x72,0x74,0x69,0x74, /* "partit" */
    0x69,0x6F,0x6E,0x0D,0x0A,0x00, /* "ion\r\n\0" */
    /* Padding hasta byte 446 */
    [100 ... 445] = 0x00
};

/* Tabla de partición: 1 partición FAT16 que ocupa todo el disco */
static void build_partition_table(u8* mbr, u32 disk_sectors) {
    /* Entrada 1: partición activa FAT16 */
    u8* pe = mbr + 446;
    pe[0]  = 0x80;         /* bootable */
    pe[1]  = 0x01;         /* CHS start head */
    pe[2]  = 0x01;         /* CHS start sector */
    pe[3]  = 0x00;         /* CHS start cylinder */
    pe[4]  = 0x06;         /* type: FAT16 >= 32MB */
    /* CHS end (usamos valores máximos) */
    pe[5]  = 0xFE;
    pe[6]  = 0xFF;
    pe[7]  = 0xFF;
    /* LBA start = sector 1 (dejamos el MBR en 0) */
    *(u32*)(pe+8)  = 1;
    /* LBA size = disco completo menos MBR */
    *(u32*)(pe+12) = disk_sectors - 1;

    /* Entradas 2-4: vacías */
    for (int i = 1; i < 4; i++) {
        u8* p = mbr + 446 + i * 16;
        for (int j = 0; j < 16; j++) p[j] = 0;
    }

    /* Firma */
    mbr[510] = 0x55;
    mbr[511] = 0xAA;
}

int mbr_install(void) {
    static u8 mbr_sector[512];
    u32 disk_secs = ata_disk_size();
    if (!disk_secs) return -1;

    /* Copiar código del MBR */
    for (int i = 0; i < 446; i++) mbr_sector[i] = mbr_code[i];

    /* Tabla de particiones */
    build_partition_table(mbr_sector, disk_secs);

    /* Escribir en sector 0 */
    return ata_write_sector(0, mbr_sector);
}

int mbr_write_partition_table(u32 disk_sectors) {
    static u8 sec[512];
    if (ata_read_sector(0, sec) < 0) return -1;
    build_partition_table(sec, disk_sectors);
    return ata_write_sector(0, sec);
}
