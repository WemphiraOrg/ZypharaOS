#ifndef FAT16_H
#define FAT16_H

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

#define FAT16_SECTOR_SIZE   512
#define FAT16_MAX_FILENAME  12   /* 8.3 + null */
#define FAT16_MAX_FILES     128

/* Atributos de entrada de directorio */
#define FAT_ATTR_READ_ONLY  0x01
#define FAT_ATTR_HIDDEN     0x02
#define FAT_ATTR_SYSTEM     0x04
#define FAT_ATTR_VOLUME_ID  0x08
#define FAT_ATTR_DIRECTORY  0x10
#define FAT_ATTR_ARCHIVE    0x20

/* Boot sector FAT16 (BPB) */
typedef struct __attribute__((packed)) {
    u8  jump[3];
    u8  oem[8];
    u16 bytes_per_sector;       /* 512 */
    u8  sectors_per_cluster;    /* 1,2,4,8... */
    u16 reserved_sectors;       /* sectores antes del FAT (incluye boot) */
    u8  num_fats;               /* normalmente 2 */
    u16 root_entries;           /* max entradas en root dir */
    u16 total_sectors_16;       /* total sectores (0 si >65535) */
    u8  media_type;             /* 0xF8 = disco fijo */
    u16 sectors_per_fat;        /* sectores por tabla FAT */
    u16 sectors_per_track;
    u16 num_heads;
    u32 hidden_sectors;
    u32 total_sectors_32;
    /* Extended BPB */
    u8  drive_number;
    u8  reserved1;
    u8  boot_signature;         /* 0x29 */
    u32 volume_id;
    u8  volume_label[11];
    u8  fs_type[8];             /* "FAT16   " */
} fat16_bpb_t;

/* Entrada de directorio FAT (32 bytes) */
typedef struct __attribute__((packed)) {
    u8  name[8];        /* nombre, padded con espacios */
    u8  ext[3];         /* extension */
    u8  attr;
    u8  reserved[10];
    u16 time;
    u16 date;
    u16 first_cluster;
    u32 file_size;
} fat16_dirent_t;

/* Info del filesystem montado */
typedef struct {
    u32 fat_start;       /* LBA del primer FAT */
    u32 root_start;      /* LBA del root directory */
    u32 data_start;      /* LBA del area de datos */
    u16 sectors_per_fat;
    u16 root_entries;
    u8  sectors_per_cluster;
    u16 reserved_sectors;
    u8  num_fats;
} fat16_fs_t;

int  fat16_mount(void);
int  fat16_format(u32 total_sectors);
int  fat16_ls(void);
int  fat16_read(const char* name, char* buf, u32 max, u32* out_size);
int  fat16_write(const char* name, const char* buf, u32 size);
int  fat16_delete(const char* name);
int  fat16_exists(const char* name);

#endif
