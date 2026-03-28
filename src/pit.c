#include "pit.h"
#include "idt.h"

#define PIT_CH0 0x40
#define PIT_CMD 0x43

static volatile u32 ticks = 0;
static u32 hz_val = 1000;

static void pit_irq(void) {
    ticks++;
    /* Llamar al scheduler cada tick */
    extern void schedule(void);
    schedule();
}

void pit_init(u32 hz) {
    hz_val = hz;
    u32 div = 1193180 / hz;
    outb(PIT_CMD, 0x36);
    outb(PIT_CH0, (u8)(div & 0xFF));
    outb(PIT_CH0, (u8)(div >> 8));
    irq_set_handler(0, pit_irq);
}

u32 pit_ticks(void) { return ticks; }
u32 pit_hz(void)    { return hz_val; }

void pit_sleep(u32 ms) {
    u32 end = ticks + ms;
    while (ticks < end)
        __asm__ volatile("hlt");
}
