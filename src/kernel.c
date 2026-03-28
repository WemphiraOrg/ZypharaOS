#include "multiboot.h"
#include "io.h"
#include "vbe.h"
#include "idt.h"
#include "pmm.h"
#include "paging.h"
#include "heap.h"
#include "process.h"
#include "cpuinfo.h"

extern void term_init(void);
extern void term_print(const char*);
extern void term_putchar(char);
extern void term_print_int(int);
extern void term_print_hex(u32);
extern void term_setcolor(int,int);
extern void term_clear(void);
extern void shell_run(void);
extern void pit_init(u32);
extern void keyboard_init(void);
extern int  ata_init(void);
extern int  fat16_mount(void);
extern void vbe_detect_from_multiboot(void*);
extern int  vbe_map_framebuffer(void);

unsigned int grub_mem_lower = 0;
unsigned int grub_mem_upper = 0;

static void panic(const char* msg) {
    term_setcolor(15, 4); /* blanco sobre rojo */
    term_print("\n*** ZYPHRAOS KERNEL PANIC ***\n");
    term_print(msg);
    term_print("\nSistema detenido.\n");
    asm volatile("cli");
    while(1) asm volatile("hlt");
}

static void banner(void) {
    term_setcolor(11,0);
    term_print(" ____              _               ___  ____\n");
    term_print("|_  /  _  _  _ __| |_  _ _ __    / _ \\/ ___|\n");
    term_print(" / / || || || '_ \\ ' \\| '_/ _\\  | | | \\___ \\\n");
    term_print("/___\\_,|_||_|| .__/_||_||_|\\___| | |_| |___) |\n");
    term_print("             |_|                  \\___/|____/\n");
    term_setcolor(14,0);
    term_print("  ZyphraOS v1.0  |  x86 Bare Metal  |  FAT16 | Paging | Scheduler\n");
    term_setcolor(8,0);
    term_print("  Built from scratch. No Linux. No libc. No limits.\n");
    term_setcolor(7,0);
    term_print("\n");
}

/* Proceso de fondo: actualiza uptime cada segundo en la esquina */
static void clock_process(void) {
    extern u32 pit_ticks(void);
    extern void pit_sleep(u32);
    u16* vga = (u16*)0xB8000;
    while(1) {
        u32 s = pit_ticks() / 1000;
        u32 h=s/3600, m=(s%3600)/60, sc=s%60;
        /* Dibujar en esquina superior derecha si VGA */
        if (!vbe_current.active) {
            char buf[9];
            buf[0]='0'+h/10; buf[1]='0'+h%10; buf[2]=':';
            buf[3]='0'+m/10; buf[4]='0'+m%10; buf[5]=':';
            buf[6]='0'+sc/10; buf[7]='0'+sc%10; buf[8]=0;
            for(int i=0;i<8;i++)
                vga[72+i]=(u16)(u8)buf[i]|(u16)(0x18<<8);
        }
        pit_sleep(1000);
    }
}

void kernel_main(unsigned int magic, void* mbi_raw) {
    /* 1. Video */
    /* Intentar detectar VBE desde Multiboot si está disponible */
    if (mbi_raw) {
        vbe_detect_from_multiboot(mbi_raw);
    } else {
        vbe_current.active = 0;
    }
    term_init();

    /* Si no fue cargado por Multiboot, usar valores por defecto */
    if (magic != 0x2BADB002) {
        /* Valores por defecto: 64MB de RAM */
        grub_mem_lower = 640;      /* 640KB convencional */
        grub_mem_upper = 65536;    /* 64MB - 1MB = 63MB en KB */
    } else {
        /* Fue cargado por Multiboot, usar información de GRUB */
        multiboot_info_t* mbi = (multiboot_info_t*)mbi_raw;
        if (mbi && (mbi->flags & 1)) {
            grub_mem_lower = mbi->mem_lower;
            grub_mem_upper = mbi->mem_upper;
        }
    }

    /* 2. IDT + PIC */
    idt_init();

    /* 3. PIT 1000Hz → scheduler activado */
    pit_init(1000);

    /* 4. Teclado IRQ1 */
    keyboard_init();

    /* 5. Gestor de memoria física */
    pmm_init(grub_mem_upper);

    /* 6. Paginación */
    paging_init(grub_mem_upper);
    paging_enable();

    /* Mapear framebuffer VBE si está activo */
    if (vbe_current.active) {
        if (vbe_map_framebuffer() != 0) {
            term_print("Advertencia: No se pudo mapear framebuffer VBE\n");
        }
    }

    /* 7. Heap del kernel */
    heap_init();

    /* 8. Scheduler */
    sched_init();

    /* Mostrar banner */
    banner();

    /* 9. Info del sistema */
    cpu_info_t cpu; cpu_detect(&cpu);
    term_setcolor(8,0);

    /* CPU */
    term_print("  CPU : ");
    term_setcolor(7,0);
    term_print(cpu.brand[0] ? cpu.brand : cpu.vendor);
    term_print("  ("); term_print_int((int)cpu.cores);
    term_print(" cores, ~"); term_print_int((int)cpu.mhz);
    term_print(" MHz)\n");

    /* RAM */
    term_setcolor(8,0); term_print("  RAM : "); term_setcolor(7,0);
    term_print_int((int)(grub_mem_upper/1024)); term_print(" MB total  ");
    term_print_int((int)(pmm_free_pages()*4/1024)); term_print(" MB libre\n");

    /* Video */
    term_setcolor(8,0); term_print("  VID : "); term_setcolor(7,0);
    if (vbe_current.active) {
        term_print_int(vbe_current.width); term_putchar('x');
        term_print_int(vbe_current.height); term_putchar('x');
        term_print_int(vbe_current.bpp); term_print("bpp VBE\n");
    } else {
        term_print("80x25 VGA texto\n");
    }

    /* Disco */
    term_setcolor(8,0); term_print("  DSK : "); term_setcolor(7,0);
    if (ata_init() > 0) {
        term_print_int((int)(ata_disk_size()/2048)); term_print(" MB ATA");
        if (fat16_mount()==0) term_print("  FAT16 OK\n");
        else                  term_print("  (sin formato, usa 'install')\n");
    } else {
        term_print("no detectado\n");
    }

    /* Paginación */
    term_setcolor(8,0); term_print("  PAG : "); term_setcolor(7,0);
    term_print("Activa  CR3="); term_print_hex(paging_get_cr3());
    term_print("  Heap=2MB@6MB\n");

    term_setcolor(7,0); term_putchar('\n');

    /* Crear proceso de reloj en background */
    proc_create("clock", clock_process, 5);

    /* Shell principal */
    shell_run();

    panic("shell_run() returned");
}
