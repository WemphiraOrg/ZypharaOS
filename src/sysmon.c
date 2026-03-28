/* sysmon.c - Monitor del sistema en tiempo real */
#include "io.h"
#include "pmm.h"
#include "heap.h"
#include "process.h"
#include "cpuinfo.h"
#include "vbe.h"

extern void term_print(const char*);
extern void term_putchar(char);
extern void term_print_int(int);
extern void term_setcolor(int,int);
extern void term_gotoxy(int,int);
extern void term_clear(void);
extern u32  pit_ticks(void);
extern int  keyboard_poll(void);
extern u32  ata_disk_size(void);
extern unsigned int grub_mem_upper;

static u16* vga = (u16*)0xB8000;

static void vga_str_attr(int x, int y, const char* s, u8 attr) {
    if (vbe_current.active) return; /* solo VGA text mode */
    while(*s) { vga[y*80+x] = (u16)(u8)*s | ((u16)attr<<8); x++; s++; }
}

static void draw_bar(int x, int y, int val, int max, int width, u8 fg, u8 bg) {
    int filled = val * width / (max ? max : 1);
    for (int i = 0; i < width; i++) {
        u8 attr = (i < filled) ? (u8)((0<<4)|fg) : (u8)((0<<4)|bg);
        vga[y*80+x+i] = (u16)'|' | ((u16)attr<<8);
    }
}

static void pad_int(int x, int y, u32 n, int width, u8 attr) {
    char buf[12]; int i=10; buf[11]=0;
    if(n==0){buf[i--]='0';}
    while(n>0){buf[i--]='0'+(n%10);n/=10;}
    char* s=buf+i+1;
    int len=0; while(s[len])len++;
    int pad=width-len; while(pad-->0){vga[y*80+x]=(u16)' '|((u16)attr<<8);x++;}
    while(*s){vga[y*80+x]=(u16)(u8)*s|((u16)attr<<8);x++;s++;}
}

void sysmon_run(void) {
    if (vbe_current.active) {
        term_clear();
        term_setcolor(12,0);
        term_print("sysmon: requiere modo VGA texto (80x25)\n");
        term_setcolor(7,0);
        return;
    }

    cpu_info_t cpu; cpu_detect(&cpu);

    /* Guardar pantalla */
    u16 saved[80*25];
    for(int i=0;i<80*25;i++) saved[i]=vga[i];

    /* Limpiar con color oscuro */
    for(int i=0;i<80*25;i++) vga[i]=(u16)' '|(u16)(0x18<<8);

    u32 last_cpu_time = 0;
    u32 last_ticks = pit_ticks();

    while(1) {
        int key = keyboard_poll();
        if (key=='q'||key=='\x1b') break;

        u32 now = pit_ticks();
        if (now - last_ticks < 500) { /* actualizar cada 500ms */
            __asm__ volatile("hlt"); continue;
        }
        last_ticks = now;

        u32 up_s = now / 1000;

        /* ── Header ── */
        vga_str_attr(0,  0, "  ZyphraOS System Monitor                                          [Q]=salir  ", 0x1E);

        /* ── Uptime ── */
        {
            char buf[32]; int i=0;
            buf[i++]='U'; buf[i++]='p'; buf[i++]=':'; buf[i++]=' ';
            u32 h=up_s/3600, m=(up_s%3600)/60, s=up_s%60;
            if(h){buf[i++]='0'+h/10;buf[i++]='0'+h%10;buf[i++]='h';buf[i++]=' ';}
            buf[i++]='0'+m/10; buf[i++]='0'+m%10; buf[i++]='m'; buf[i++]=' ';
            buf[i++]='0'+s/10; buf[i++]='0'+s%10; buf[i++]='s'; buf[i]=0;
            vga_str_attr(2, 1, buf, 0x1F);
        }

        /* ── CPU ── */
        vga_str_attr(0, 3, " CPU ", 0x1B);
        vga_str_attr(5, 3, cpu.brand[0]?cpu.brand:cpu.vendor, 0x17);

        /* Uso CPU estimado via cpu_time de procesos */
        u32 cur_cpu = proc_total_cpu();
        u32 delta_cpu = cur_cpu - last_cpu_time;
        u32 delta_t   = (now - last_ticks) + 1;
        u32 cpu_pct   = delta_cpu * 100 / (delta_t > 0 ? delta_t : 1);
        if (cpu_pct > 100) cpu_pct = 100;
        last_cpu_time = cur_cpu;

        vga_str_attr(2, 4, "Uso:  [                    ] ", 0x17);
        draw_bar(8, 4, (int)cpu_pct, 100, 20, 0xA, 0x8);
        pad_int(30, 4, cpu_pct, 3, 0x1E); vga[4*80+33]=(u16)'%'|(0x1E<<8);

        vga_str_attr(2, 5, "Freq: ", 0x17);
        pad_int(8, 5, cpu.mhz, 4, 0x1F);
        vga_str_attr(12,5, " MHz", 0x17);
        vga_str_attr(18,5, "Cores:", 0x17);
        pad_int(25,5,(u32)cpu.cores,2,0x1F);

        /* ── RAM ── */
        vga_str_attr(0, 7, " RAM ", 0x1B);
        u32 total_kb = grub_mem_upper;
        u32 free_pages = pmm_free_pages();
        u32 free_kb  = free_pages * 4;
        u32 used_kb  = total_kb > free_kb ? total_kb - free_kb : 0;
        u32 ram_pct  = used_kb * 100 / (total_kb ? total_kb : 1);

        vga_str_attr(2, 8, "Total:[                    ] ", 0x17);
        draw_bar(8, 8, (int)ram_pct, 100, 20, 0xB, 0x8);
        pad_int(30,8,total_kb/1024,4,0x1F); vga_str_attr(34,8," MB",0x17);

        vga_str_attr(2, 9, "Usado:", 0x17);
        pad_int(8,9,used_kb/1024,4,0x1F); vga_str_attr(12,9," MB  Libre:",0x17);
        pad_int(22,9,free_kb/1024,4,0x1F); vga_str_attr(26,9," MB",0x17);

        /* Heap */
        vga_str_attr(2,10,"Heap: ", 0x17);
        pad_int(8,10,heap_used()/1024,4,0x1F); vga_str_attr(12,10,"KB usado",0x17);

        /* ── Procesos ── */
        vga_str_attr(0,12," PROCESOS ", 0x1B);
        vga_str_attr(2,13,"PID  Nombre              Estado   CPU%  Pri",0x1E);

        u32 total_cpu_all = proc_total_cpu() + 1;
        int row = 14;

        /* Acceso directo a la tabla de procesos via proc_list extendido */
        /* Mostrar proceso actual */
        process_t* cur = proc_current();
        if (cur) {
            /* Dibujar proceso actual */
            pad_int(2,row,(u32)cur->pid,3,0x1F);
            char* name=cur->name; int nx=6;
            while(*name&&nx<22){vga[row*80+nx]=(u16)(u8)*name|(0x1F<<8);nx++;name++;}
            while(nx<22){vga[row*80+nx]=(u16)' '|(0x17<<8);nx++;}
            vga_str_attr(24,row,"RUNNING  ",0x1A);
            u32 cpu_p = cur->cpu_time*100/total_cpu_all;
            pad_int(33,row,cpu_p,3,0x1E);
            pad_int(38,row,cur->priority,1,0x17);
            row++;
        }

        /* ── Disco ── */
        vga_str_attr(0,row+1," DISCO ", 0x1B);
        u32 dsecs = ata_disk_size();
        if (dsecs) {
            vga_str_attr(2,row+2,"ATA0: ",0x17);
            pad_int(8,row+2,dsecs/2048,4,0x1F);
            vga_str_attr(12,row+2," MB",0x17);
        } else {
            vga_str_attr(2,row+2,"No detectado",0x18);
        }

        /* ── Info estática ── */
        vga_str_attr(0,23," ZyphraOS v1.0  Bare Metal OS ", 0x18);
        vga_str_attr(32,23,"[Q] Salir  Actualiza cada 500ms", 0x18);
    }

    /* Restaurar pantalla */
    for(int i=0;i<80*25;i++) vga[i]=saved[i];
}
