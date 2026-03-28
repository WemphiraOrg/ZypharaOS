#ifndef VBE_H
#define VBE_H
#include "io.h"

typedef struct __attribute__((packed)) {
    u16 width, height, bpp;
    u32 framebuffer;
    u32 pitch;        /* bytes por línea */
    int active;       /* 1 si VBE está activo */
} vbe_info_t;

/* Resoluciones disponibles */
typedef struct {
    u16 w, h, bpp;
    u16 vbe_mode;
    const char* name;
} vbe_mode_t;

extern vbe_info_t vbe_current;

/* Llamado antes de entrar a modo protegido (desde GRUB con info multiboot) */
void vbe_detect_from_multiboot(void* mbi);
int  vbe_map_framebuffer(void);
void vbe_set_pixel(int x, int y, u32 color);
void vbe_fill(u32 color);
u32  vbe_rgb(u8 r, u8 g, u8 b);
void vbe_rect(int x, int y, int w, int h, u32 color);
void vbe_char(int x, int y, char c, u32 fg, u32 bg);
void vbe_print(int x, int y, const char* s, u32 fg, u32 bg);
void vbe_print_int(int x, int y, int n, u32 fg, u32 bg);
int  vbe_cols(void);
int  vbe_rows(void);

#endif
