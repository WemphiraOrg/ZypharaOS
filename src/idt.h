#ifndef IDT_H
#define IDT_H
#include "io.h"
void idt_init(void);
void idt_set_handler(u8 n, void* handler);
/* Registrar handler de IRQ (0-15) */
void irq_set_handler(u8 irq, void (*handler)(void));
void irq_eoi(u8 irq);
#endif
