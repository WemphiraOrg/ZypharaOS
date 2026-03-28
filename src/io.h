#ifndef IO_H
#define IO_H
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long long u64;

static inline void outb(u16 p, u8  v) { __asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p)); }
static inline void outw(u16 p, u16 v) { __asm__ volatile("outw %0,%1"::"a"(v),"Nd"(p)); }
static inline void outl(u16 p, u32 v) { __asm__ volatile("outl %0,%1"::"a"(v),"Nd"(p)); }
static inline u8   inb(u16 p) { u8  v; __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p)); return v; }
static inline u16  inw(u16 p) { u16 v; __asm__ volatile("inw %1,%0":"=a"(v):"Nd"(p)); return v; }
static inline u32  inl(u16 p) { u32 v; __asm__ volatile("inl %1,%0":"=a"(v):"Nd"(p)); return v; }
static inline void io_wait(void) { outb(0x80, 0); }

static inline u64 rdtsc(void) {
    u32 lo, hi;
    __asm__ volatile("rdtsc":"=a"(lo),"=d"(hi));
    return ((u64)hi << 32) | lo;
}
#endif
