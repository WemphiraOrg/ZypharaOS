#ifndef PMM_H
#define PMM_H
#include "io.h"
/* Physical Memory Manager - bitmap allocator */
void pmm_init(u32 mem_upper_kb);
void* pmm_alloc(void);          /* aloca 1 página (4KB) */
void  pmm_free(void* page);
u32   pmm_free_pages(void);
u32   pmm_total_pages(void);
#define PAGE_SIZE 4096
#endif
