#include "ata.h"
#include "fat16.h"
#include "mbr_boot.h"
#include "io.h"

/* Símbolos del bootloader secundario */
extern u8 boot2_bin[];
extern u32 boot2_bin_len;

extern void term_print(const char*);
extern void term_putchar(char);
extern void term_print_int(int);
extern void term_setcolor(int,int);
extern char keyboard_getchar(void);

/* Marcadores del linker - inicio y fin del kernel en memoria */
extern u8 _start[];        /* definido en boot.S */

/* Tamaño del kernel: desde 1MB hasta fin del BSS */
#define KERNEL_LOAD_ADDR 0x100000

static void bar(int done, int total, int width) {
    term_putchar('[');
    int filled = done * width / total;
    for(int i=0;i<width;i++){
        if(i<filled) { term_setcolor(10,0); term_putchar('#'); }
        else          { term_setcolor(8,0);  term_putchar('.'); }
    }
    term_setcolor(7,0); term_putchar(']');
    term_putchar(' ');
    term_print_int(done*100/total);
    term_putchar('%');
}

void cmd_install(void) {
    term_setcolor(11,0);
    term_print("\n");
    term_print(" +==============================================+\n");
    term_print(" |         ZyphraOS Installer v1.0             |\n");
    term_print(" |   Bare Metal | FAT16 | MBR | Hardware Real  |\n");
    term_print(" +==============================================+\n\n");
    term_setcolor(7,0);

    int secs = ata_init();
    if (secs <= 0) {
        term_setcolor(12,0);
        term_print(" ERROR: No se detecto disco.\n");
        term_print(" Inicia QEMU con: -hda disco.img\n");
        term_setcolor(7,0); return;
    }

    term_print(" Disco: "); term_setcolor(10,0);
    term_print_int(secs/2048); term_print(" MB");
    term_setcolor(7,0); term_print("  (");
    term_print_int(secs); term_print(" sectores)\n");

    /* Estimar tamaño del kernel leyendo ELF en memoria */
    u32 kern_size = 0x20000; /* ~128KB estimado conservador */
    term_print(" Kernel en RAM: ~");
    term_print_int((int)kern_size/1024); term_print(" KB\n");

    term_setcolor(14,0);
    term_print("\n ADVERTENCIA: Se BORRARAN todos los datos del disco.\n");
    term_setcolor(7,0);
    term_print(" Continuar? (s/n): ");
    char c = keyboard_getchar();
    term_putchar(c); term_putchar('\n');
    if (c!='s'&&c!='S') { term_print(" Cancelado.\n"); return; }
    term_putchar('\n');

    /* Paso 1: MBR */
    term_print(" [1/5] MBR bootloader...        ");
    if (mbr_install() < 0) { term_setcolor(12,0); term_print("FALLO\n"); term_setcolor(7,0); return; }
    term_setcolor(10,0); term_print("OK\n"); term_setcolor(7,0);

    /* Paso 1b: Bootloader secundario */
    term_print(" [1/5] Bootloader secundario... ");
    if (boot2_install() < 0) { term_setcolor(12,0); term_print("FALLO\n"); term_setcolor(7,0); return; }
    term_setcolor(10,0); term_print("OK\n"); term_setcolor(7,0);

    /* Paso 2: Tabla de particiones */
    term_print(" [2/5] Tabla de particiones...  ");
    if (mbr_write_partition_table((u32)secs) < 0) {
        term_setcolor(12,0); term_print("FALLO\n"); term_setcolor(7,0); return;
    }
    term_setcolor(10,0); term_print("OK\n"); term_setcolor(7,0);

    /* Paso 3: Formatear FAT16 */
    term_print(" [3/5] Formateando FAT16...     ");
    if (fat16_format((u32)secs) < 0) {
        term_setcolor(12,0); term_print("FALLO\n"); term_setcolor(7,0); return;
    }
    term_setcolor(10,0); term_print("OK\n"); term_setcolor(7,0);

    /* Paso 4: Montar */
    term_print(" [4/5] Montando filesystem...   ");
    if (fat16_mount() < 0) {
        term_setcolor(12,0); term_print("FALLO\n"); term_setcolor(7,0); return;
    }
    term_setcolor(10,0); term_print("OK\n"); term_setcolor(7,0);

    /* Paso 5: Copiar kernel byte a byte desde 0x100000 */
    term_print(" [5/5] Copiando kernel...       \n        ");
    u8* kernel_mem = (u8*)KERNEL_LOAD_ADDR;
    if (fat16_write("zyphraos.elf", (const char*)kernel_mem, kern_size) < 0) {
        term_setcolor(12,0); term_print("FALLO\n"); term_setcolor(7,0); return;
    }
    bar((int)kern_size, (int)kern_size, 30);
    term_putchar('\n');

    /* Archivos del sistema */
    fat16_write("readme.txt",
        "=== ZyphraOS v1.0 ===\n"
        "Bare Metal Operating System\n"
        "IDT | PIT | FAT16 | Paging | Scheduler\n"
        "Usa 'help' para ver comandos.\n", 0);
    fat16_write("motd.txt",
        "Bienvenido a ZyphraOS\n"
        "El sistema operativo bare metal serio.\n", 0);

    term_putchar('\n');
    term_setcolor(10,0);
    term_print(" *** Instalacion completada! ***\n\n");
    term_setcolor(7,0);
    term_print(" Para arrancar desde disco:\n");
    term_setcolor(14,0);
    term_print("   qemu-system-i386 -hda disco.img -m 64M\n");
    term_setcolor(8,0);
    term_print("   (hardware real: graba disco.img con 'dd' o Rufus)\n\n");
    term_setcolor(7,0);
}

/* Instalar bootloader secundario en sector 1 */
int boot2_install(void) {
    /* Escribir boot2.bin en sector 1 (después del MBR) */
    return ata_write_sector(1, boot2_bin);
}
