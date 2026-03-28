#include "pmm.h"

/* Bitmap: 1 bit por página de 4KB
   Para 256MB necesitamos 64K páginas = 8KB de bitmap */
#define MAX_PAGES (256 * 1024 * 1024 / PAGE_SIZE)  /* 65536 páginas */
static u32 bitmap[MAX_PAGES / 32];
static u32 total_pages = 0;
static u32 free_count  = 0;

/* Base de memoria disponible: empezamos en 2MB (después del kernel) */
#define MEM_BASE (2 * 1024 * 1024)

static void bitmap_set(u32 page)   { bitmap[page/32] |=  (1U << (page%32)); }
static void bitmap_clear(u32 page) { bitmap[page/32] &= ~(1U << (page%32)); }
static int  bitmap_test(u32 page)  { return (bitmap[page/32] >> (page%32)) & 1; }

void pmm_init(u32 mem_upper_kb) {
    u32 mem_bytes = mem_upper_kb * 1024 + (1024 * 1024); /* +1MB baja */
    total_pages = mem_bytes / PAGE_SIZE;
    if (total_pages > MAX_PAGES) total_pages = MAX_PAGES;

    /* Marcar todo como usado inicialmente */
    for (u32 i = 0; i < MAX_PAGES/32; i++) bitmap[i] = 0xFFFFFFFF;

    /* Liberar desde 2MB hasta fin de RAM */
    u32 first_free = MEM_BASE / PAGE_SIZE;
    for (u32 i = first_free; i < total_pages; i++) {
        bitmap_clear(i);
        free_count++;
    }
}

void* pmm_alloc(void) {
    for (u32 i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            free_count--;
            return (void*)(i * PAGE_SIZE);
        }
    }
    return 0;  /* sin memoria */
}

void pmm_free(void* page) {
    u32 i = (u32)page / PAGE_SIZE;
    if (i < total_pages && bitmap_test(i)) {
        bitmap_clear(i);
        free_count++;
    }
}

u32 pmm_free_pages(void)  { return free_count; }
u32 pmm_total_pages(void) { return total_pages; }
