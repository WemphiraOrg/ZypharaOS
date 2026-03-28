#include "process.h"
#include "heap.h"
#include "pmm.h"

extern u32 pit_ticks(void);
extern void term_print(const char*);
extern void term_print_int(int);
extern void term_putchar(char);
extern void term_setcolor(int,int);

static process_t procs[MAX_PROCS];
static process_t* current_proc = 0;
static u32 next_pid = 1;
static int proc_count_n = 0;

/* Cambio de contexto: guarda ESP del proceso actual, restaura el nuevo */
static void context_switch(u32* old_esp, u32 new_esp) {
    __asm__ volatile(
        "pusha\n"
        "mov %%esp, (%0)\n"   /* guardar ESP actual */
        "mov %1, %%esp\n"     /* cargar nuevo ESP */
        "popa\n"
        :: "r"(old_esp), "r"(new_esp) : "memory"
    );
}

/* Stack inicial de un proceso:
   Al hacer popa + ret, el proceso arranca en entry() */
static void setup_stack(process_t* p, void (*entry)(void)) {
    u32* stack = (u32*)(p->stack_base + KSTACK_SIZE);

    /* Simular un pusha + dirección de retorno */
    *(--stack) = (u32)entry;  /* EIP = entry point */
    *(--stack) = 0;           /* EAX */
    *(--stack) = 0;           /* ECX */
    *(--stack) = 0;           /* EDX */
    *(--stack) = 0;           /* EBX */
    *(--stack) = 0;           /* ESP (ignorado por popa) */
    *(--stack) = p->stack_base + KSTACK_SIZE; /* EBP */
    *(--stack) = 0;           /* ESI */
    *(--stack) = 0;           /* EDI */
    p->esp = (u32)stack;
}

void sched_init(void) {
    for (int i = 0; i < MAX_PROCS; i++) {
        procs[i].state = PROC_DEAD;
        procs[i].pid   = 0;
    }
    proc_count_n = 0;
}

int proc_create(const char* name, void (*entry)(void), u32 priority) {
    /* Buscar slot libre */
    process_t* p = 0;
    for (int i = 0; i < MAX_PROCS; i++) {
        if (procs[i].state == PROC_DEAD) { p = &procs[i]; break; }
    }
    if (!p) return -1;

    /* Alocar stack del proceso */
    void* stack = kmalloc(KSTACK_SIZE);
    if (!stack) return -1;

    /* Inicializar proceso */
    p->pid        = next_pid++;
    p->state      = PROC_READY;
    p->stack_base = (u32)stack;
    p->cpu_time   = 0;
    p->sleep_until= 0;
    p->priority   = priority ? priority : 3;

    /* Copiar nombre */
    int i = 0;
    while (name[i] && i < PROC_NAME_LEN-1) { p->name[i]=name[i]; i++; }
    p->name[i] = 0;

    setup_stack(p, entry);
    proc_count_n++;
    return (int)p->pid;
}

process_t* proc_current(void) { return current_proc; }
int proc_count(void) { return proc_count_n; }

void proc_sleep(u32 ms) {
    if (!current_proc) return;
    current_proc->state       = PROC_SLEEPING;
    current_proc->sleep_until = pit_ticks() + ms;
    proc_yield();
}

void proc_exit(void) {
    if (!current_proc) return;
    current_proc->state = PROC_ZOMBIE;
    proc_count_n--;
    proc_yield();
}

/* Round-robin scheduler */
void schedule(void) {
    u32 now = pit_ticks();

    /* Despertar procesos dormidos */
    for (int i = 0; i < MAX_PROCS; i++) {
        if (procs[i].state == PROC_SLEEPING && now >= procs[i].sleep_until)
            procs[i].state = PROC_READY;
    }

    if (!current_proc) {
        /* Primera vez: buscar cualquier proceso READY */
        for (int i = 0; i < MAX_PROCS; i++) {
            if (procs[i].state == PROC_READY) {
                current_proc = &procs[i];
                current_proc->state = PROC_RUNNING;
                return;
            }
        }
        return;
    }

    /* Contabilizar CPU time */
    current_proc->cpu_time++;

    /* Buscar siguiente proceso READY (round-robin) */
    int cur_idx = (int)(current_proc - procs);
    process_t* old = current_proc;

    for (int step = 1; step <= MAX_PROCS; step++) {
        int next = (cur_idx + step) % MAX_PROCS;
        if (procs[next].state == PROC_READY) {
            /* Cambio de contexto */
            old->state = (old->state == PROC_RUNNING) ? PROC_READY : old->state;
            current_proc = &procs[next];
            current_proc->state = PROC_RUNNING;
            context_switch(&old->esp, current_proc->esp);
            return;
        }
    }
    /* Ningún otro proceso, continuar con el actual */
}

void proc_yield(void) {
    /* Forzar schedule via software */
    __asm__ volatile("int $0x20");  /* simular IRQ0 */
}

void proc_list(void) {
    term_setcolor(14, 0);
    term_print("PID  Estado     CPU    Pri  Nombre\n");
    term_print("---  ---------  -----  ---  --------\n");
    term_setcolor(7, 0);
    for (int i = 0; i < MAX_PROCS; i++) {
        if (procs[i].state == PROC_DEAD) continue;
        term_print_int((int)procs[i].pid);
        term_print("    ");
        const char* states[] = {"DEAD","READY","RUN  ","SLEEP","ZOMBI"};
        term_print(states[procs[i].state]);
        term_print("  ");
        term_print_int((int)procs[i].cpu_time);
        term_print("    ");
        term_print_int((int)procs[i].priority);
        term_print("    ");
        term_print(procs[i].name);
        term_putchar('\n');
    }
}

u32 proc_total_cpu(void) {
    u32 total = 0;
    for (int i = 0; i < MAX_PROCS; i++)
        if (procs[i].state != PROC_DEAD) total += procs[i].cpu_time;
    return total;
}
