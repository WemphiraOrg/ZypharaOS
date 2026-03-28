#include "idt.h"
#include "io.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

typedef struct __attribute__((packed)) {
    u16 offset_lo, selector;
    u8  zero, type_attr;
    u16 offset_hi;
} idt_entry_t;

typedef struct __attribute__((packed)) {
    u16 limit; u32 base;
} idt_ptr_t;

static idt_entry_t idt[256];
static idt_ptr_t   idtp;
static void (*irq_handlers[16])(void);

extern void irq0_stub(void);  extern void irq1_stub(void);
extern void irq2_stub(void);  extern void irq3_stub(void);
extern void irq4_stub(void);  extern void irq5_stub(void);
extern void irq6_stub(void);  extern void irq7_stub(void);
extern void irq8_stub(void);  extern void irq9_stub(void);
extern void irq10_stub(void); extern void irq11_stub(void);
extern void irq12_stub(void); extern void irq13_stub(void);
extern void irq14_stub(void); extern void irq15_stub(void);

static void idt_gate(u8 n, u32 h) {
    idt[n].offset_lo = (u16)(h & 0xFFFF);
    idt[n].selector  = 0x08;
    idt[n].zero      = 0;
    idt[n].type_attr = 0x8E;
    idt[n].offset_hi = (u16)(h >> 16);
}

void irq_dispatch(u8 irq) {
    if (irq < 16 && irq_handlers[irq]) irq_handlers[irq]();
    if (irq >= 8) outb(PIC2_CMD, 0x20);
    outb(PIC1_CMD, 0x20);
}

void irq_eoi(u8 irq) {
    if (irq >= 8) outb(PIC2_CMD, 0x20);
    outb(PIC1_CMD, 0x20);
}

void irq_set_handler(u8 irq, void (*h)(void)) {
    if (irq >= 16) return;
    irq_handlers[irq] = h;
    u16 port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    u8  bit  = irq < 8 ? irq : irq - 8;
    outb(port, inb(port) & ~(1u << bit));
}

void idt_set_handler(u8 n, void* h) { idt_gate(n, (u32)h); }

void idt_init(void) {
    for (int i = 0; i < 256; i++) {
        idt[i].offset_lo=0; idt[i].selector=0x08;
        idt[i].zero=0; idt[i].type_attr=0x8E; idt[i].offset_hi=0;
    }
    /* Remap PIC: IRQ0-7 → INT 0x20-0x27, IRQ8-15 → INT 0x28-0x2F */
    outb(PIC1_CMD,0x11); io_wait(); outb(PIC2_CMD,0x11); io_wait();
    outb(PIC1_DATA,0x20); io_wait(); outb(PIC2_DATA,0x28); io_wait();
    outb(PIC1_DATA,0x04); io_wait(); outb(PIC2_DATA,0x02); io_wait();
    outb(PIC1_DATA,0x01); io_wait(); outb(PIC2_DATA,0x01); io_wait();
    outb(PIC1_DATA,0xFF); outb(PIC2_DATA,0xFF);

    idt_gate(0x20,(u32)irq0_stub);  idt_gate(0x21,(u32)irq1_stub);
    idt_gate(0x22,(u32)irq2_stub);  idt_gate(0x23,(u32)irq3_stub);
    idt_gate(0x24,(u32)irq4_stub);  idt_gate(0x25,(u32)irq5_stub);
    idt_gate(0x26,(u32)irq6_stub);  idt_gate(0x27,(u32)irq7_stub);
    idt_gate(0x28,(u32)irq8_stub);  idt_gate(0x29,(u32)irq9_stub);
    idt_gate(0x2A,(u32)irq10_stub); idt_gate(0x2B,(u32)irq11_stub);
    idt_gate(0x2C,(u32)irq12_stub); idt_gate(0x2D,(u32)irq13_stub);
    idt_gate(0x2E,(u32)irq14_stub); idt_gate(0x2F,(u32)irq15_stub);

    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (u32)idt;
    __asm__ volatile("lidt %0" :: "m"(idtp));
    __asm__ volatile("sti");
}
