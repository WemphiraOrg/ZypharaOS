#ifndef CPUINFO_H
#define CPUINFO_H
#include "io.h"
typedef struct {
    char vendor[13], brand[49];
    u32  family, model, stepping, cores, mhz;
    u32  l1d_kb, l1i_kb, l2_kb;
    int  has_fpu, has_sse, has_sse2, has_apic, has_tsc, has_nx;
} cpu_info_t;
void cpu_detect(cpu_info_t* info);
void cpu_print(cpu_info_t* info);
#endif
