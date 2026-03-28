#ifndef PAGING_H
#define PAGING_H
#include "io.h"

#define PAGE_SIZE       4096
#define PAGE_PRESENT    0x001
#define PAGE_WRITABLE   0x002
#define PAGE_USER       0x004
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040
#define PAGE_4MB        0x080

/* Dirección virtual -> índices */
#define PD_INDEX(va)  (((u32)(va)) >> 22)
#define PT_INDEX(va)  ((((u32)(va)) >> 12) & 0x3FF)
#define PAGE_ALIGN(x) (((u32)(x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

void paging_init(u32 mem_upper_kb);
void* virt_to_phys(void* vaddr);
int   map_page(u32 vaddr, u32 paddr, u32 flags);
int   unmap_page(u32 vaddr);
void  paging_enable(void);
u32   paging_get_cr3(void);

#endif
