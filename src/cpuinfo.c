#include "cpuinfo.h"

extern void term_print(const char*);
extern void term_print_int(int);
extern void term_putchar(char);
extern void term_setcolor(int,int);

static void cpuid(u32 leaf, u32* a, u32* b, u32* c, u32* d) {
    __asm__ volatile("cpuid":"=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d):"a"(leaf):"memory");
}

static u32 measure_mhz(void) {
    /* Medición con RDTSC: esperar ~10ms via PIT polling */
    /* Leer contador del PIT canal 0 antes y después */
    u64 t1 = rdtsc();
    /* Busy-wait ~10ms basado en PIT a 1000Hz */
    u32 volatile count = 0;
    while (count < 2000000) count++;
    u64 t2 = rdtsc();
    u32 delta = (u32)(t2 - t1);
    /* Estimación: ~200MHz loop de calibración */
    return delta / 2000;
}

void cpu_detect(cpu_info_t* c) {
    u32 a,b,cc,d;
    cpuid(0,&a,&b,&cc,&d);
    u32* v=(u32*)c->vendor; v[0]=b; v[1]=d; v[2]=cc; c->vendor[12]=0;

    cpuid(1,&a,&b,&cc,&d);
    c->stepping = a&0xF; c->model=(a>>4)&0xF; c->family=(a>>8)&0xF;
    /* Extended family/model */
    if (c->family==0xF) c->family += (a>>20)&0xFF;
    if (c->family>=6)   c->model  += ((a>>16)&0xF)<<4;
    c->has_fpu =(d>>0)&1; c->has_tsc=(d>>4)&1; c->has_apic=(d>>9)&1;
    c->has_sse =(d>>25)&1; c->has_sse2=(d>>26)&1;
    c->has_nx  =(cc>>20)&1;
    c->cores   =(b>>16)&0xFF; if(!c->cores) c->cores=1;

    cpuid(0x80000000,&a,&b,&cc,&d);
    if (a >= 0x80000004) {
        u32* br=(u32*)c->brand;
        cpuid(0x80000002,&br[0],&br[1],&br[2],&br[3]);
        cpuid(0x80000003,&br[4],&br[5],&br[6],&br[7]);
        cpuid(0x80000004,&br[8],&br[9],&br[10],&br[11]);
        c->brand[48]=0;
        /* Trim leading spaces */
        char* p=c->brand; while(*p==' ')p++;
        if (p!=c->brand) {
            int i=0; while(p[i]){c->brand[i]=p[i];i++;} c->brand[i]=0;
        }
    } else c->brand[0]=0;

    /* Cache info */
    cpuid(0x80000005,&a,&b,&cc,&d);
    c->l1d_kb=(cc>>24)&0xFF;
    c->l1i_kb=(d>>24)&0xFF;
    cpuid(0x80000006,&a,&b,&cc,&d);
    c->l2_kb=(cc>>16)&0xFFFF;

    c->mhz = measure_mhz();
}

void cpu_print(cpu_info_t* c) {
    term_setcolor(11,0); term_print("CPU:\n"); term_setcolor(7,0);
    if (c->brand[0]) { term_print("  "); term_print(c->brand); term_putchar('\n'); }
    term_print("  Vendor  : "); term_print(c->vendor); term_putchar('\n');
    term_print("  Family  : "); term_print_int((int)c->family);
    term_print("  Model: "); term_print_int((int)c->model);
    term_print("  Step: "); term_print_int((int)c->stepping); term_putchar('\n');
    term_print("  Nucleos  : "); term_print_int((int)c->cores); term_putchar('\n');
    term_print("  Frec est.: ~"); term_print_int((int)c->mhz); term_print(" MHz\n");
    if (c->l2_kb) { term_print("  L2 Cache : "); term_print_int((int)c->l2_kb); term_print(" KB\n"); }
    term_print("  Features : ");
    if(c->has_fpu)  term_print("FPU ");
    if(c->has_tsc)  term_print("TSC ");
    if(c->has_apic) term_print("APIC ");
    if(c->has_sse)  term_print("SSE ");
    if(c->has_sse2) term_print("SSE2 ");
    if(c->has_nx)   term_print("NX ");
    term_putchar('\n');
}
