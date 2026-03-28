#ifndef VGA_H
#define VGA_H

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

/* Colores VGA */
typedef enum {
    VGA_BLACK=0, VGA_BLUE, VGA_GREEN, VGA_CYAN,
    VGA_RED, VGA_MAGENTA, VGA_BROWN, VGA_LGRAY,
    VGA_DGRAY, VGA_LBLUE, VGA_LGREEN, VGA_LCYAN,
    VGA_LRED, VGA_LMAGENTA, VGA_YELLOW, VGA_WHITE
} vga_color;

static inline u8 make_vga_attr(vga_color fg, vga_color bg) {
    return (u8)((bg << 4) | fg);
}
static inline u16 vga_entry(u8 ch, u8 attr) {
    return (u16)ch | ((u16)attr << 8);
}

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEM    ((u16*)0xB8000)

#endif
