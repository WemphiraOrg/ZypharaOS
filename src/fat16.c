#include "fat16.h"
#include "ata.h"

extern void term_print(const char*);
extern void term_putchar(char);
extern void term_print_int(int);
extern void term_setcolor(int fg, int bg);

static fat16_fs_t fs;
static u8 sector_buf[512];
static u8 sector_buf2[512];
static int mounted = 0;

/* ── Utilidades de string ── */
static int k_strlen(const char* s) { int n=0; while(s[n])n++; return n; }
static void k_memset(void* d, int v, u32 n) { u8* p=(u8*)d; while(n--)*p++=v; }
static void k_memcpy(void* d, const void* s, u32 n) {
    u8* dd=(u8*)d; const u8* ss=(const u8*)s; while(n--)*dd++=*ss++;
}
static int k_strcmp(const char* a, const char* b) {
    while(*a&&*a==*b){a++;b++;} return (unsigned char)*a-(unsigned char)*b;
}

/* Convertir nombre 8.3 a string normal */
static void fat_name_to_str(const u8* name, const u8* ext, char* out) {
    int i = 0, j = 0;
    /* Nombre */
    for (i = 0; i < 8 && name[i] != ' '; i++)
        out[j++] = (char)name[i];
    /* Extension */
    if (ext[0] != ' ') {
        out[j++] = '.';
        for (i = 0; i < 3 && ext[i] != ' '; i++)
            out[j++] = (char)ext[i];
    }
    out[j] = 0;
    /* Lowercase */
    for (int k = 0; out[k]; k++)
        if (out[k] >= 'A' && out[k] <= 'Z') out[k] += 32;
}

/* Convertir string normal a nombre 8.3 */
static void str_to_fat_name(const char* str, u8* name, u8* ext) {
    k_memset(name, ' ', 8);
    k_memset(ext,  ' ', 3);
    int i = 0, j = 0;
    /* Uppercase */
    char upper[13];
    for (i = 0; str[i] && i < 12; i++) {
        char c = str[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        upper[i] = c;
    }
    upper[i] = 0;
    /* Separar nombre y extension */
    for (i = 0; upper[i] && upper[i] != '.' && j < 8; i++, j++)
        name[j] = (u8)upper[i];
    if (upper[i] == '.') {
        i++;
        for (j = 0; upper[i] && j < 3; i++, j++)
            ext[j] = (u8)upper[i];
    }
}

/* ── FAT16 Format ── */
int fat16_format(u32 total_sectors) {
    /* Calcular geometría FAT16 */
    u16 reserved   = 4;
    u8  num_fats   = 2;
    u16 root_ents  = 512;
    u8  spc        = 1;   /* sectors per cluster */
    if (total_sectors > 8192)  spc = 2;
    if (total_sectors > 32768) spc = 4;

    /* Sectores de FAT: cada entrada = 2 bytes, 256 entradas por sector */
    u32 data_sectors = total_sectors - reserved - (root_ents * 32 / 512);
    u32 clusters     = data_sectors / spc;
    u16 spf          = (u16)((clusters * 2 + 511) / 512 + 1);

    /* Construir BPB */
    k_memset(sector_buf, 0, 512);
    fat16_bpb_t* bpb = (fat16_bpb_t*)sector_buf;
    bpb->jump[0] = 0xEB; bpb->jump[1] = 0x58; bpb->jump[2] = 0x90;
    k_memcpy(bpb->oem, "TOS v0.2", 8);
    bpb->bytes_per_sector    = 512;
    bpb->sectors_per_cluster = spc;
    bpb->reserved_sectors    = reserved;
    bpb->num_fats            = num_fats;
    bpb->root_entries        = root_ents;
    bpb->total_sectors_16    = (total_sectors <= 65535) ? (u16)total_sectors : 0;
    bpb->total_sectors_32    = (total_sectors > 65535)  ? total_sectors : 0;
    bpb->media_type          = 0xF8;
    bpb->sectors_per_fat     = spf;
    bpb->sectors_per_track   = 63;
    bpb->num_heads           = 255;
    bpb->hidden_sectors      = 0;
    bpb->drive_number        = 0x80;
    bpb->boot_signature      = 0x29;
    bpb->volume_id           = 0x12345678;
    k_memcpy(bpb->volume_label, "TOS DISK   ", 11);
    k_memcpy(bpb->fs_type,      "FAT16   ",   8);
    /* Firma de arranque */
    sector_buf[510] = 0x55; sector_buf[511] = 0xAA;

    /* Escribir boot sector */
    if (ata_write_sector(0, sector_buf) < 0) return -1;

    /* Limpiar sectores reservados restantes */
    k_memset(sector_buf, 0, 512);
    for (u32 s = 1; s < reserved; s++)
        ata_write_sector(s, sector_buf);

    /* Escribir las 2 FATs vacías (primer entry = 0xFFF8, segundo = 0xFFFF) */
    k_memset(sector_buf, 0, 512);
    sector_buf[0] = 0xF8; sector_buf[1] = 0xFF;  /* media byte */
    sector_buf[2] = 0xFF; sector_buf[3] = 0xFF;  /* end of chain */
    for (u8 f = 0; f < num_fats; f++) {
        u32 fat_lba = reserved + f * spf;
        ata_write_sector(fat_lba, sector_buf);
        k_memset(sector_buf, 0, 512);
        for (u16 i = 1; i < spf; i++)
            ata_write_sector(fat_lba + i, sector_buf);
    }

    /* Limpiar root directory */
    u32 root_lba = reserved + num_fats * spf;
    u32 root_secs = root_ents * 32 / 512;
    for (u32 s = 0; s < root_secs; s++)
        ata_write_sector(root_lba + s, sector_buf);

    return 0;
}

/* ── FAT16 Mount ── */
int fat16_mount(void) {
    if (ata_read_sector(0, sector_buf) < 0) return -1;

    fat16_bpb_t* bpb = (fat16_bpb_t*)sector_buf;

    /* Verificar firma */
    if (sector_buf[510] != 0x55 || sector_buf[511] != 0xAA) return -1;
    /* Verificar que es FAT16 */
    if (bpb->bytes_per_sector != 512) return -1;
    if (bpb->boot_signature != 0x29)  return -1;

    fs.reserved_sectors    = bpb->reserved_sectors;
    fs.num_fats            = bpb->num_fats;
    fs.sectors_per_fat     = bpb->sectors_per_fat;
    fs.sectors_per_cluster = bpb->sectors_per_cluster;
    fs.root_entries        = bpb->root_entries;
    fs.fat_start           = bpb->reserved_sectors;
    fs.root_start          = bpb->reserved_sectors + bpb->num_fats * bpb->sectors_per_fat;
    fs.data_start          = fs.root_start + (bpb->root_entries * 32 + 511) / 512;

    mounted = 1;
    return 0;
}

/* Leer entrada FAT */
static u16 fat_get(u16 cluster) {
    u32 offset  = cluster * 2;
    u32 lba     = fs.fat_start + offset / 512;
    u32 off_in  = offset % 512;
    ata_read_sector(lba, sector_buf2);
    return *(u16*)(sector_buf2 + off_in);
}

/* Escribir entrada FAT */
static void fat_set(u16 cluster, u16 value) {
    u32 offset = cluster * 2;
    u32 lba    = fs.fat_start + offset / 512;
    u32 off_in = offset % 512;
    ata_read_sector(lba, sector_buf2);
    *(u16*)(sector_buf2 + off_in) = value;
    ata_write_sector(lba, sector_buf2);
    /* Actualizar segunda copia FAT */
    u32 lba2 = lba + fs.sectors_per_fat;
    ata_write_sector(lba2, sector_buf2);
}

/* Buscar cluster libre */
static u16 fat_alloc(void) {
    for (u16 c = 2; c < 0xFFF0; c++) {
        if (fat_get(c) == 0x0000) {
            fat_set(c, 0xFFFF);
            return c;
        }
    }
    return 0;  /* disco lleno */
}

/* Buscar entrada en root dir por nombre */
static int find_dirent(const char* name, fat16_dirent_t* out, u32* out_lba, u32* out_idx) {
    u8 fname[8], fext[3];
    str_to_fat_name(name, fname, fext);

    u32 entries_per_sector = 512 / 32;
    u32 root_sectors = (fs.root_entries * 32 + 511) / 512;

    for (u32 s = 0; s < root_sectors; s++) {
        ata_read_sector(fs.root_start + s, sector_buf);
        fat16_dirent_t* dir = (fat16_dirent_t*)sector_buf;
        for (u32 i = 0; i < entries_per_sector; i++) {
            if (dir[i].name[0] == 0x00) return -1;  /* fin */
            if (dir[i].name[0] == 0xE5) continue;   /* borrado */
            if (dir[i].attr & (FAT_ATTR_VOLUME_ID | FAT_ATTR_DIRECTORY)) continue;

            int match = 1;
            for (int j = 0; j < 8; j++)
                if (dir[i].name[j] != fname[j]) { match = 0; break; }
            if (match)
                for (int j = 0; j < 3; j++)
                    if (dir[i].ext[j] != fext[j]) { match = 0; break; }

            if (match) {
                if (out)     *out     = dir[i];
                if (out_lba) *out_lba = fs.root_start + s;
                if (out_idx) *out_idx = i;
                return 0;
            }
        }
    }
    return -1;
}

int fat16_exists(const char* name) {
    return find_dirent(name, 0, 0, 0) == 0;
}

/* ── ls ── */
int fat16_ls(void) {
    if (!mounted) return -1;
    u32 root_sectors = (fs.root_entries * 32 + 511) / 512;
    int count = 0;

    for (u32 s = 0; s < root_sectors; s++) {
        ata_read_sector(fs.root_start + s, sector_buf);
        fat16_dirent_t* dir = (fat16_dirent_t*)sector_buf;
        u32 eps = 512 / 32;
        for (u32 i = 0; i < eps; i++) {
            if (dir[i].name[0] == 0x00) goto done;
            if (dir[i].name[0] == 0xE5) continue;
            if (dir[i].attr & (FAT_ATTR_VOLUME_ID | FAT_ATTR_DIRECTORY)) continue;

            char fname[13];
            fat_name_to_str(dir[i].name, dir[i].ext, fname);
            term_setcolor(10, 0);  /* LGREEN */
            term_print(fname);
            term_setcolor(7, 0);   /* WHITE */
            int nl = k_strlen(fname);
            for (int p = nl; p < 16; p++) term_putchar(' ');
            term_print_int((int)dir[i].file_size);
            term_print(" bytes\n");
            count++;
        }
    }
done:
    if (count == 0) {
        term_setcolor(8, 0);
        term_print("(vacio)\n");
        term_setcolor(7, 0);
    }
    return count;
}

/* ── read ── */
int fat16_read(const char* name, char* buf, u32 max, u32* out_size) {
    if (!mounted) return -1;
    fat16_dirent_t de;
    if (find_dirent(name, &de, 0, 0) < 0) return -1;

    u32 size = de.file_size;
    if (out_size) *out_size = size;
    if (size > max - 1) size = max - 1;

    u32 read = 0;
    u16 cluster = de.first_cluster;

    while (cluster >= 2 && cluster < 0xFFF8 && read < size) {
        u32 lba = fs.data_start + (cluster - 2) * fs.sectors_per_cluster;
        for (u8 s = 0; s < fs.sectors_per_cluster && read < size; s++) {
            ata_read_sector(lba + s, sector_buf);
            u32 to_copy = 512;
            if (read + to_copy > size) to_copy = size - read;
            k_memcpy(buf + read, sector_buf, to_copy);
            read += to_copy;
        }
        cluster = fat_get(cluster);
    }
    buf[read] = 0;
    return (int)read;
}

/* ── write ── */
int fat16_write(const char* name, const char* buf, u32 size) {
    if (!mounted) return -1;

    /* Borrar si existe */
    fat16_delete(name);

    /* Buscar entrada libre en root dir */
    u32 root_sectors = (fs.root_entries * 32 + 511) / 512;
    u32 free_lba = 0, free_idx = 0;
    int found = 0;

    for (u32 s = 0; s < root_sectors && !found; s++) {
        ata_read_sector(fs.root_start + s, sector_buf);
        fat16_dirent_t* dir = (fat16_dirent_t*)sector_buf;
        u32 eps = 512 / 32;
        for (u32 i = 0; i < eps; i++) {
            if (dir[i].name[0] == 0x00 || dir[i].name[0] == 0xE5) {
                free_lba = fs.root_start + s;
                free_idx = i;
                found = 1;
                break;
            }
        }
    }
    if (!found) return -2;  /* root dir lleno */

    /* Alocar clusters */
    u16 first_cluster = 0, prev_cluster = 0;
    u32 written = 0;

    while (written < size) {
        u16 cluster = fat_alloc();
        if (!cluster) return -3;  /* disco lleno */

        if (!first_cluster) first_cluster = cluster;
        if (prev_cluster)   fat_set(prev_cluster, cluster);
        fat_set(cluster, 0xFFFF);

        u32 lba = fs.data_start + (cluster - 2) * fs.sectors_per_cluster;
        for (u8 s = 0; s < fs.sectors_per_cluster; s++) {
            k_memset(sector_buf, 0, 512);
            u32 to_write = 512;
            if (written + to_write > size) to_write = size - written;
            if (to_write > 0) k_memcpy(sector_buf, buf + written, to_write);
            ata_write_sector(lba + s, sector_buf);
            written += to_write;
            if (written >= size) break;
        }
        prev_cluster = cluster;
    }

    /* Escribir entrada de directorio */
    ata_read_sector(free_lba, sector_buf);
    fat16_dirent_t* dir = (fat16_dirent_t*)sector_buf;
    fat16_dirent_t* de = &dir[free_idx];
    k_memset(de, 0, 32);
    str_to_fat_name(name, de->name, de->ext);
    de->attr          = FAT_ATTR_ARCHIVE;
    de->first_cluster = first_cluster;
    de->file_size     = size;
    de->date          = 0x5860;  /* 2023-03-00 */
    de->time          = 0x0000;
    ata_write_sector(free_lba, sector_buf);

    return 0;
}

/* ── delete ── */
int fat16_delete(const char* name) {
    if (!mounted) return -1;
    fat16_dirent_t de;
    u32 lba, idx;
    if (find_dirent(name, &de, &lba, &idx) < 0) return -1;

    /* Liberar cadena de clusters */
    u16 cluster = de.first_cluster;
    while (cluster >= 2 && cluster < 0xFFF8) {
        u16 next = fat_get(cluster);
        fat_set(cluster, 0x0000);
        cluster = next;
    }

    /* Marcar entrada como borrada */
    ata_read_sector(lba, sector_buf);
    fat16_dirent_t* dir = (fat16_dirent_t*)sector_buf;
    dir[idx].name[0] = 0xE5;
    ata_write_sector(lba, sector_buf);
    return 0;
}
