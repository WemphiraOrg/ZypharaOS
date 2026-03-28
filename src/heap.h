#ifndef HEAP_H
#define HEAP_H
#include "io.h"
void  heap_init(void);
void* kmalloc(u32 size);
void* kmalloc_aligned(u32 size, u32 align);
void  kfree(void* ptr);
u32   heap_used(void);
u32   heap_free(void);
#endif
