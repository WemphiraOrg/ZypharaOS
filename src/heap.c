#include "heap.h"
#include "pmm.h"

/* Heap del kernel: pool de 2MB a partir de 6MB físico */
#define HEAP_START   (6 * 1024 * 1024)
#define HEAP_SIZE    (2 * 1024 * 1024)
#define HEAP_MAGIC   0xABCD1234

typedef struct block_hdr {
    u32              magic;
    u32              size;      /* tamaño del dato (sin header) */
    u8               free;
    struct block_hdr* next;
    struct block_hdr* prev;
} block_hdr_t;

#define HDR_SIZE sizeof(block_hdr_t)

static block_hdr_t* heap_start_ptr = 0;
static u32 used_bytes = 0;

void heap_init(void) {
    heap_start_ptr = (block_hdr_t*)HEAP_START;
    heap_start_ptr->magic = HEAP_MAGIC;
    heap_start_ptr->size  = HEAP_SIZE - HDR_SIZE;
    heap_start_ptr->free  = 1;
    heap_start_ptr->next  = 0;
    heap_start_ptr->prev  = 0;
    used_bytes = 0;
}

static void merge_free_blocks(void) {
    block_hdr_t* b = heap_start_ptr;
    while (b && b->next) {
        if (b->free && b->next->free) {
            b->size += HDR_SIZE + b->next->size;
            b->next  = b->next->next;
            if (b->next) b->next->prev = b;
        } else {
            b = b->next;
        }
    }
}

void* kmalloc(u32 size) {
    if (!size) return 0;
    /* Alinear a 8 bytes */
    size = (size + 7) & ~7u;

    block_hdr_t* b = heap_start_ptr;
    while (b) {
        if (b->magic != HEAP_MAGIC) return 0; /* heap corrupto */
        if (b->free && b->size >= size) {
            /* Split si sobra espacio suficiente */
            if (b->size >= size + HDR_SIZE + 16) {
                block_hdr_t* newb = (block_hdr_t*)((u8*)b + HDR_SIZE + size);
                newb->magic = HEAP_MAGIC;
                newb->size  = b->size - size - HDR_SIZE;
                newb->free  = 1;
                newb->next  = b->next;
                newb->prev  = b;
                if (b->next) b->next->prev = newb;
                b->next = newb;
                b->size = size;
            }
            b->free = 0;
            used_bytes += b->size + HDR_SIZE;
            return (void*)((u8*)b + HDR_SIZE);
        }
        b = b->next;
    }
    return 0; /* sin memoria */
}

void* kmalloc_aligned(u32 size, u32 align) {
    /* Simple: alocar size+align y ajustar */
    void* raw = kmalloc(size + align);
    if (!raw) return 0;
    u32 addr = (u32)raw;
    u32 aligned = (addr + align - 1) & ~(align - 1);
    return (void*)aligned;
}

void kfree(void* ptr) {
    if (!ptr) return;
    block_hdr_t* b = (block_hdr_t*)((u8*)ptr - HDR_SIZE);
    if (b->magic != HEAP_MAGIC || b->free) return;
    used_bytes -= b->size + HDR_SIZE;
    b->free = 1;
    merge_free_blocks();
}

u32 heap_used(void) { return used_bytes; }
u32 heap_free(void) { return HEAP_SIZE - used_bytes; }
