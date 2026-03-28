#ifndef PROCESS_H
#define PROCESS_H
#include "io.h"

#define MAX_PROCS    16
#define KSTACK_SIZE  8192
#define PROC_NAME_LEN 32

typedef enum {
    PROC_DEAD = 0,
    PROC_READY,
    PROC_RUNNING,
    PROC_SLEEPING,
    PROC_ZOMBIE
} proc_state_t;

/* Context guardado en el stack al cambio de contexto */
typedef struct {
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pusha */
    u32 eip;                                        /* return address */
} cpu_context_t;

typedef struct process {
    u32           pid;
    char          name[PROC_NAME_LEN];
    proc_state_t  state;
    u32           esp;          /* stack pointer guardado */
    u32           stack_base;   /* base del stack */
    u32           sleep_until;  /* ticks hasta despertar */
    u32           cpu_time;     /* ticks de CPU usados */
    u32           priority;     /* 1=alta, 5=baja */
    struct process* next;
} process_t;

void     sched_init(void);
int      proc_create(const char* name, void (*entry)(void), u32 priority);
void     proc_sleep(u32 ms);
void     proc_exit(void);
void     proc_yield(void);
void     schedule(void);         /* llamado por el timer IRQ */
process_t* proc_current(void);
int      proc_count(void);
void     proc_list(void);
u32      proc_total_cpu(void);

#endif
