#include "io.h"
#include "fat16.h"
#include "pmm.h"
#include "heap.h"
#include "process.h"
#include "cpuinfo.h"
#include "paging.h"
#include "vbe.h"

extern void  term_print(const char*);
extern void  term_putchar(char);
extern void  term_print_int(int);
extern void  term_print_uint(u32);
extern void  term_print_hex(u32);
extern void  term_clear(void);
extern void  term_setcolor(int,int);
extern void  term_gotoxy(int,int);
extern void  term_puts_at(int,int,const char*,int,int);
extern char  keyboard_getchar(void);
extern int   keyboard_poll(void);
extern void  snake_run(void);
extern void  cmd_install(void);
extern void  sysmon_run(void);
extern u32   pit_ticks(void);
extern void  pit_sleep(u32);
extern u32   ata_disk_size(void);
extern unsigned int grub_mem_lower, grub_mem_upper;

#define CMD_MAX  512
#define HIST_MAX 32
static char  cmd_buf[CMD_MAX];
static char  history[HIST_MAX][CMD_MAX];
static int   hist_n=0, hist_i=0;

/* ── String utils ── */
static int  slen(const char*s){int n=0;while(s[n])n++;return n;}
static int  scmp(const char*a,const char*b){while(*a&&*a==*b){a++;b++;}return(unsigned char)*a-(unsigned char)*b;}
static void scpy(char*d,const char*s){while((*d++=*s++));}
static void sncpy(char*d,const char*s,int n){int i=0;while(i<n-1&&s[i]){d[i]=s[i];i++;}d[i]=0;}
static int  sncmp(const char*a,const char*b,int n){while(n--&&*a&&*a==*b){a++;b++;}return n<0?0:(unsigned char)*a-(unsigned char)*b;}

static int readline(char* buf, int max) {
    int pos=0; hist_i=hist_n;
    while(1){
        char c=keyboard_getchar();
        if(c=='\n'||c=='\r'){term_putchar('\n');buf[pos]=0;return pos;}
        if(c=='\b'){if(pos>0){pos--;buf[pos]=0;term_putchar('\b');}continue;}
        if(c=='\x80'&&hist_i>0){  /* UP */
            while(pos-->0) term_putchar('\b');
            hist_i--; scpy(buf,history[hist_i%HIST_MAX]);
            pos=slen(buf); term_print(buf); continue;
        }
        if(c=='\x81'&&hist_i<hist_n){ /* DOWN */
            while(pos-->0) term_putchar('\b');
            hist_i++;
            if(hist_i<hist_n){scpy(buf,history[hist_i%HIST_MAX]);pos=slen(buf);}
            else{buf[0]=0;pos=0;}
            term_print(buf); continue;
        }
        if(c=='\x03'){term_print("^C\n");buf[0]=0;return 0;} /* Ctrl+C */
        if(pos<max-1){buf[pos++]=c;term_putchar(c);}
    }
}

#define MAX_ARGS 16
static char* argv_v[MAX_ARGS];
static char  argv_buf[CMD_MAX];
static int parse_args(const char* line){
    int argc=0; scpy(argv_buf,line); char*p=argv_buf;
    while(*p&&argc<MAX_ARGS){
        while(*p==' ')p++; if(!*p)break;
        argv_v[argc++]=p;
        while(*p&&*p!=' ')p++;
        if(*p)*p++=0;
    }
    argv_v[argc]=0; return argc;
}

/* ═══════════════ COMANDOS ═══════════════ */

static void cmd_credits(void) {
    term_setcolor(11,0);
    term_print("╔══════════════════════════════════════════════════════════════╗\n");
    term_print("║                    ZYPHRAOS CREDITS                         ║\n");
    term_print("╠══════════════════════════════════════════════════════════════╣\n");
    term_print("║  Project Creator:                                          ║\n");
    term_print("║    - ZyphraOS Team                                         ║\n");
    term_print("║                                                            ║\n");
    term_print("║  Core Contributors:                                        ║\n");
    term_print("║    - (Tu nombre podría estar aquí!)                        ║\n");
    term_print("║                                                            ║\n");
    term_print("║  Contributors:                                             ║\n");
    term_print("║    - (Tu nombre podría estar aquí!)                        ║\n");
    term_print("║                                                            ║\n");
    term_print("║  Bug Hunters:                                              ║\n");
    term_print("║    - (Tu nombre podría estar aquí!)                        ║\n");
    term_print("║                                                            ║\n");
    term_print("║  Únete: https://discord.gg/KVFBpEN3Gs                      ║\n");
    term_print("╚══════════════════════════════════════════════════════════════╝\n");
    term_setcolor(7,0);
}

static void cmd_help(void) {
    term_setcolor(11,0);
    term_print("+=================================================+\n");
    term_print("|           ZyphraOS v1.0  -  Bare Metal          |\n");
    term_print("|  IDT | IRQ | PIT | Paging | Heap | Scheduler    |\n");
    term_print("+=================================================+\n");

    term_setcolor(14,0); term_print("Sistema:\n"); term_setcolor(7,0);
    term_print("  help              esta ayuda\n");
    term_print("  clear             limpiar pantalla\n");
    term_print("  sysinfo           info completa del sistema\n");
    term_print("  sysmon            monitor en tiempo real (htop)\n");
    term_print("  credits           ver colaboradores del proyecto\n");
    term_print("  mem               mapa y estadisticas de memoria\n");
    term_print("  uptime            tiempo de actividad\n");
    term_print("  ps                tabla de procesos\n");
    term_print("  cpuinfo           info detallada del CPU\n");
    term_print("  echo [texto]      imprimir texto\n");
    term_print("  color <fg> <bg>   cambiar colores (0-15)\n");
    term_print("  history           historial (flechas arr/abj)\n");
    term_print("  reboot            reiniciar\n");
    term_print("  halt              apagar\n");

    term_setcolor(14,0); term_print("Memoria:\n"); term_setcolor(7,0);
    term_print("  memtest           test de lectura/escritura RAM\n");
    term_print("  heapinfo          estado del heap del kernel\n");
    term_print("  pageinfo          estado de la paginacion\n");

    term_setcolor(14,0); term_print("Disco FAT16:\n"); term_setcolor(7,0);
    term_print("  ls                listar archivos\n");
    term_print("  cat [archivo]     ver archivo\n");
    term_print("  write [archivo]   escribir archivo\n");
    term_print("  rm [archivo]      borrar archivo\n");
    term_print("  diskinfo          info del disco\n");
    term_print("  install           instalar ZyphraOS en disco\n");

    term_setcolor(14,0); term_print("Juegos:\n");
    term_setcolor(10,0);
    term_print("  snake             Snake (WASD, ESC=salir)\n");
    term_setcolor(7,0);
}

static void cmd_sysinfo(void) {
    cpu_info_t cpu; cpu_detect(&cpu);
    u32 ticks = pit_ticks();
    u32 up_s = ticks/1000;

    term_setcolor(11,0); term_print("=== ZyphraOS System Info ===\n\n"); term_setcolor(7,0);

    cpu_print(&cpu);

    term_setcolor(11,0); term_print("\nMemoria:\n"); term_setcolor(7,0);
    term_print("  RAM total : "); term_print_int((int)((grub_mem_lower+grub_mem_upper)/1024)); term_print(" MB\n");
    term_print("  RAM alta  : "); term_print_int((int)(grub_mem_upper/1024)); term_print(" MB\n");
    term_print("  PMM libre : "); term_print_int((int)(pmm_free_pages()*4/1024)); term_print(" MB  (");
    term_print_int((int)pmm_free_pages()); term_print(" paginas x 4KB)\n");
    term_print("  Heap usado: "); term_print_int((int)(heap_used()/1024)); term_print(" KB\n");
    term_print("  Heap libre: "); term_print_int((int)(heap_free()/1024)); term_print(" KB\n");
    term_print("  Paginacion: CR3="); term_print_hex(paging_get_cr3()); term_putchar('\n');

    term_setcolor(11,0); term_print("\nVideo:\n"); term_setcolor(7,0);
    if (vbe_current.active) {
        term_print("  VBE "); term_print_int(vbe_current.width);
        term_putchar('x'); term_print_int(vbe_current.height);
        term_putchar('x'); term_print_int(vbe_current.bpp);
        term_print("bpp  fb:"); term_print_hex(vbe_current.framebuffer); term_putchar('\n');
    } else {
        term_print("  VGA texto 80x25 @ 0xB8000\n");
    }

    term_setcolor(11,0); term_print("\nDisco:\n"); term_setcolor(7,0);
    u32 dsecs = ata_disk_size();
    if (dsecs) { term_print("  ATA0: "); term_print_int((int)(dsecs/2048)); term_print(" MB\n"); }
    else        { term_print("  Sin disco\n"); }

    term_setcolor(11,0); term_print("\nProcesos:\n"); term_setcolor(7,0);
    term_print("  Activos: "); term_print_int(proc_count()); term_print("\n");
    term_print("  CPU total ticks: "); term_print_uint(proc_total_cpu()); term_putchar('\n');

    term_setcolor(11,0); term_print("\nUptime:\n"); term_setcolor(7,0);
    term_print("  "); term_print_int((int)(up_s/3600)); term_print("h ");
    term_print_int((int)((up_s%3600)/60)); term_print("m ");
    term_print_int((int)(up_s%60)); term_print("s  (");
    term_print_uint(ticks); term_print(" ticks)\n");
}

static void cmd_mem(void) {
    term_setcolor(11,0); term_print("=== Mapa de Memoria ===\n"); term_setcolor(7,0);
    term_print("  0x00000000 - 0x000FFFFF  BIOS/ROM/IVT (1MB)\n");
    term_print("  0x000A0000 - 0x000BFFFF    VGA framebuffer\n");
    term_print("  0x000B8000 - 0x000BFFFF    VGA text buffer\n");
    term_print("  0x000C0000 - 0x000FFFFF    Video BIOS\n");
    term_print("  0x00100000 - 0x001FFFFF  Kernel ZyphraOS (~1MB)\n");
    term_print("  0x00200000 - 0x003FFFFF  Stacks kernel\n");
    term_print("  0x00400000 - 0x005FFFFF  PMM bitmap\n");
    term_print("  0x00600000 - 0x007FFFFF  Heap kernel (2MB)\n");
    term_print("  0x00800000 - ");
    term_print_hex((grub_mem_upper+1024)*1024);
    term_print("  RAM libre (PMM)\n");
    if (vbe_current.active) {
        term_print("  "); term_print_hex(vbe_current.framebuffer);
        term_print("              Framebuffer VBE\n");
    }
    term_putchar('\n');
    term_setcolor(11,0); term_print("Estadisticas PMM:\n"); term_setcolor(7,0);
    term_print("  Total paginas: "); term_print_uint(pmm_total_pages()); term_putchar('\n');
    term_print("  Libres       : "); term_print_uint(pmm_free_pages()); term_putchar('\n');
    term_print("  Usadas       : "); term_print_uint(pmm_total_pages()-pmm_free_pages()); term_putchar('\n');
    term_print("  Libre total  : "); term_print_int((int)(pmm_free_pages()*4/1024)); term_print(" MB\n");
}

static void cmd_memtest(void) {
    term_setcolor(14,0); term_print("Test de memoria RAM...\n"); term_setcolor(7,0);
    /* Escribir y leer patrones en páginas libres */
    u32 ok=0, fail=0;
    for (int i=0; i<64; i++) {
        void* page = pmm_alloc();
        if (!page) break;
        u32* p = (u32*)page;
        u32 pat = 0xDEAD0000 | (u32)i;
        for (int j=0; j<1024; j++) p[j]=pat+j;
        int err=0;
        for (int j=0; j<1024; j++) if(p[j]!=pat+(u32)j) err=1;
        if(err) fail++; else ok++;
        pmm_free(page);
    }
    term_print("  Paginas OK  : "); term_print_uint(ok); term_putchar('\n');
    term_print("  Paginas FAIL: "); term_print_uint(fail); term_putchar('\n');
    if (!fail) { term_setcolor(10,0); term_print("  RAM OK!\n"); }
    else       { term_setcolor(12,0); term_print("  ERRORES DE RAM!\n"); }
    term_setcolor(7,0);
}

static void cmd_heapinfo(void) {
    term_setcolor(11,0); term_print("Heap del kernel:\n"); term_setcolor(7,0);
    term_print("  Base  : 0x600000 (6MB)\n");
    term_print("  Tamaño: 2 MB\n");
    term_print("  Usado : "); term_print_int((int)(heap_used()/1024)); term_print(" KB\n");
    term_print("  Libre : "); term_print_int((int)(heap_free()/1024)); term_print(" KB\n");
    /* Test de kmalloc */
    void* p1 = kmalloc(1024);
    void* p2 = kmalloc(512);
    term_print("  kmalloc(1024): "); term_print_hex((u32)p1); term_putchar('\n');
    term_print("  kmalloc(512) : "); term_print_hex((u32)p2); term_putchar('\n');
    kfree(p1); kfree(p2);
    term_print("  Tras kfree   : "); term_print_int((int)(heap_used()/1024)); term_print(" KB usado\n");
}

static void cmd_pageinfo(void) {
    term_setcolor(11,0); term_print("Paginacion x86:\n"); term_setcolor(7,0);
    term_print("  Modo    : 32-bit paging (CR0.PG=1)\n");
    term_print("  CR3     : "); term_print_hex(paging_get_cr3()); term_putchar('\n');
    term_print("  Mapping : Identity map 0-8MB\n");
    term_print("  PD      : 1024 entradas x 4MB\n");
    term_print("  PT      : 1024 entradas x 4KB\n");
    /* Verificar que un par de direcciones mapean correcto */
    void* test_virt = (void*)0x100000;
    void* test_phys = virt_to_phys(test_virt);
    term_print("  virt_to_phys(0x100000) = "); term_print_hex((u32)test_phys); term_putchar('\n');
    test_virt = (void*)0xB8000;
    test_phys = virt_to_phys(test_virt);
    term_print("  virt_to_phys(0xB8000)  = "); term_print_hex((u32)test_phys); term_putchar('\n');
}

static void cmd_cpuinfo(void) {
    cpu_info_t cpu; cpu_detect(&cpu);
    cpu_print(&cpu);
}

static void cmd_ps(void) {
    term_setcolor(11,0); term_print("Procesos:\n"); term_setcolor(7,0);
    proc_list();
}

static void cmd_uptime(void) {
    u32 s=pit_ticks()/1000;
    term_print("Uptime: ");
    term_print_int((int)(s/3600)); term_print("h ");
    term_print_int((int)((s%3600)/60)); term_print("m ");
    term_print_int((int)(s%60)); term_print("s\n");
}

static void cmd_diskinfo(void) {
    term_setcolor(11,0); term_print("Disco ATA:\n"); term_setcolor(7,0);
    u32 s=ata_disk_size();
    if(!s){term_print("  Sin disco\n");return;}
    term_print("  Sectores : "); term_print_uint(s); term_putchar('\n');
    term_print("  Capacidad: "); term_print_int((int)(s/2048)); term_print(" MB\n");
    term_print("  Sector   : 512 bytes  LBA28\n");
    term_print("  FAT16    : "); term_print_int((int)(s*512/1024/1024)); term_print(" MB disponible\n");
}

static void cmd_echo(int argc, char** args) {
    for(int i=1;i<argc;i++){if(i>1)term_putchar(' ');term_print(args[i]);}
    term_putchar('\n');
}

static void cmd_color(int argc, char** args) {
    if(argc<3){term_print("uso: color <fg 0-15> <bg 0-15>\n");return;}
    /* Parsear números de hasta 2 dígitos */
    int fg=0, bg=0;
    for(int i=0;args[1][i]>='0'&&args[1][i]<='9';i++) fg=fg*10+(args[1][i]-'0');
    for(int i=0;args[2][i]>='0'&&args[2][i]<='9';i++) bg=bg*10+(args[2][i]-'0');
    if(fg<0||fg>15||bg<0||bg>15){term_print("rango: 0-15\n");return;}
    term_setcolor(fg,bg);
    term_print("Color cambiado.\n");
}

#define RBUF (16*1024)
static char read_buf[RBUF];

static void cmd_ls(void){
    term_setcolor(14,0); term_print("Archivos en disco (FAT16):\n"); term_setcolor(7,0);
    fat16_ls();
}
static void cmd_cat(int argc, char** args) {
    if(argc<2){term_print("uso: cat [archivo]\n");return;}
    u32 sz; int n=fat16_read(args[1],read_buf,RBUF,&sz);
    if(n<0){term_setcolor(12,0);term_print("No existe: ");term_setcolor(7,0);term_print(args[1]);term_putchar('\n');return;}
    term_print(read_buf);
    if(!n||read_buf[n-1]!='\n') term_putchar('\n');
}
static char wcont[8192];
static void cmd_write(int argc, char** args) {
    if(argc<2){term_print("uso: write [archivo]\n");return;}
    term_setcolor(8,0); term_print("Escribe (linea vacia=fin, Ctrl+C=cancelar):\n"); term_setcolor(7,0);
    int total=0; char line[CMD_MAX];
    while(1){
        term_print("> "); readline(line,CMD_MAX);
        if(!line[0]) break;
        int ll=slen(line);
        if(total+ll+1<8191){scpy(wcont+total,line);total+=ll;wcont[total++]='\n';wcont[total]=0;}
    }
    if(total==0){term_print("Cancelado.\n");return;}
    if(fat16_write(args[1],wcont,(u32)total)==0){
        term_setcolor(10,0); term_print("Guardado: "); term_print(args[1]);
        term_print(" ("); term_print_int(total); term_print(" bytes)\n");
    } else { term_setcolor(12,0); term_print("ERROR al guardar\n"); }
    term_setcolor(7,0);
}
static void cmd_rm(int argc, char** args) {
    if(argc<2){term_print("uso: rm [archivo]\n");return;}
    if(fat16_delete(args[1])==0){term_setcolor(14,0);term_print("Borrado: ");term_setcolor(7,0);term_print(args[1]);term_putchar('\n');}
    else{term_setcolor(12,0);term_print("No existe: ");term_setcolor(7,0);term_print(args[1]);term_putchar('\n');}
}
static void cmd_history(void) {
    for(int i=0;i<hist_n;i++){
        term_setcolor(14,0); term_print_int(i+1);
        term_setcolor(7,0); term_print("  "); term_print(history[i%HIST_MAX]); term_putchar('\n');
    }
}
static void cmd_halt(void) {
    term_setcolor(12,0); term_print("Sistema detenido.\n");
    __asm__ volatile("cli;hlt"); while(1);
}
static void cmd_reboot(void) {
    term_setcolor(14,0); term_print("Reiniciando...\n");
    pit_sleep(500);
    /* Método 1: keyboard controller */
    __asm__ volatile("cli");
    u8 val;
    do { val=inb(0x64); } while(val&2);
    outb(0x64,0xFE);
    pit_sleep(500);
    /* Método 2: triple fault */
    __asm__ volatile("lidt 0\nint $0" ::: "memory");
    while(1);
}

static void hist_add(const char* c){
    if(!c[0]) return;
    sncpy(history[hist_n%HIST_MAX],c,CMD_MAX);
    hist_n++;
}

static void prompt(void) {
    process_t* cur = proc_current();
    term_setcolor(10,0); term_print("zyphraos");
    term_setcolor(8,0);  term_putchar('@');
    term_setcolor(11,0); term_print(cur ? cur->name : "kernel");
    term_setcolor(14,0); term_putchar(':');
    term_setcolor(7,0);  term_putchar('/');
    term_setcolor(14,0); term_putchar('$');
    term_setcolor(7,0);  term_putchar(' ');
}

void shell_run(void) {
    while(1) {
        prompt();
        readline(cmd_buf, CMD_MAX);
        if(!cmd_buf[0]) continue;
        hist_add(cmd_buf);
        int argc=parse_args(cmd_buf);
        if(!argc) continue;
        char** args=argv_v;

        if     (!scmp(args[0],"help"))     cmd_help();
        else if(!scmp(args[0],"clear"))    term_clear();
        else if(!scmp(args[0],"sysinfo"))  cmd_sysinfo();
        else if(!scmp(args[0],"sysmon"))   sysmon_run();
        else if(!scmp(args[0],"credits"))  cmd_credits();
        else if(!scmp(args[0],"mem"))      cmd_mem();
        else if(!scmp(args[0],"memtest"))  cmd_memtest();
        else if(!scmp(args[0],"heapinfo")) cmd_heapinfo();
        else if(!scmp(args[0],"pageinfo")) cmd_pageinfo();
        else if(!scmp(args[0],"cpuinfo"))  cmd_cpuinfo();
        else if(!scmp(args[0],"ps"))       cmd_ps();
        else if(!scmp(args[0],"uptime"))   cmd_uptime();
        else if(!scmp(args[0],"diskinfo")) cmd_diskinfo();
        else if(!scmp(args[0],"echo"))     cmd_echo(argc,args);
        else if(!scmp(args[0],"color"))    cmd_color(argc,args);
        else if(!scmp(args[0],"ls"))       cmd_ls();
        else if(!scmp(args[0],"cat"))      cmd_cat(argc,args);
        else if(!scmp(args[0],"write"))    cmd_write(argc,args);
        else if(!scmp(args[0],"rm"))       cmd_rm(argc,args);
        else if(!scmp(args[0],"install"))  cmd_install();
        else if(!scmp(args[0],"history"))  cmd_history();
        else if(!scmp(args[0],"snake"))    snake_run();
        else if(!scmp(args[0],"halt"))     cmd_halt();
        else if(!scmp(args[0],"reboot"))   cmd_reboot();
        else {
            term_setcolor(12,0); term_print("zyphraos: comando no encontrado: ");
            term_setcolor(7,0);  term_print(args[0]); term_putchar('\n');
        }
    }
}
