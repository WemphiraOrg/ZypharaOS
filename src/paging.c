#include "paging.h"
#include "pmm.h"

/* Page Directory y primera Page Table estáticos (en BSS) */
/* 4KB alineados - compilador los alinea con __attribute__ */
static u32 kernel_pd[1024]  __attribute__((aligned(4096)));
static u32 kernel_pt0[1024] __attribute__((aligned(4096)));  /* 0-4MB */
static u32 kernel_pt1[1024] __attribute__((aligned(4096)));  /* 4-8MB */
static int paging_active = 0;

/* Identity map: virtual == physical para el kernel */
void paging_init(u32 mem_upper_kb) {
    (void)mem_upper_kb;

    /* Limpiar page directory */
    for (int i = 0; i < 1024; i++) kernel_pd[i] = 0;

    /* Identity map 0-4MB (kernel + BIOS + VGA) */
    for (int i = 0; i < 1024; i++)
        kernel_pt0[i] = (u32)(i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;

    /* Identity map 4-8MB (stack + heap kernel) */
    for (int i = 0; i < 1024; i++)
        kernel_pt1[i] = (u32)(4*1024*1024 + i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;

    /* Registrar en Page Directory */
    kernel_pd[0] = (u32)kernel_pt0 | PAGE_PRESENT | PAGE_WRITABLE;
    kernel_pd[1] = (u32)kernel_pt1 | PAGE_PRESENT | PAGE_WRITABLE;

    /* Map VGA framebuffer region (0xA0000-0xBFFFF) - ya en pt0 por identity map */
    /* Map framebuffer VBE si está por encima de 8MB - lo haremos on-demand */
}

void paging_enable(void) {
    __asm__ volatile(
        "mov %0, %%cr3\n"          /* cargar page directory */
        "mov %%cr0, %%eax\n"
        "or  $0x80000001, %%eax\n" /* PE + PG */
        "mov %%eax, %%cr0\n"
        :: "r"(kernel_pd) : "eax"
    );
    paging_active = 1;
}

u32 paging_get_cr3(void) {
    u32 cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

int map_page(u32 vaddr, u32 paddr, u32 flags) {
    u32 pdi = PD_INDEX(vaddr);
    u32 pti = PT_INDEX(vaddr);

    if (!(kernel_pd[pdi] & PAGE_PRESENT)) {
        /* Alocar nueva page table */
        u32* pt = (u32*)pmm_alloc();
        if (!pt) return -1;
        for (int i = 0; i < 1024; i++) pt[i] = 0;
        kernel_pd[pdi] = (u32)pt | PAGE_PRESENT | PAGE_WRITABLE;
    }

    u32* pt = (u32*)(kernel_pd[pdi] & 0xFFFFF000);
    pt[pti] = (paddr & 0xFFFFF000) | (flags & 0xFFF) | PAGE_PRESENT;

    /* Invalidar TLB para esta dirección */
    __asm__ volatile("invlpg (%0)" :: "r"(vaddr) : "memory");
    return 0;
}

int unmap_page(u32 vaddr) {
    u32 pdi = PD_INDEX(vaddr);
    u32 pti = PT_INDEX(vaddr);
    if (!(kernel_pd[pdi] & PAGE_PRESENT)) return -1;
    u32* pt = (u32*)(kernel_pd[pdi] & 0xFFFFF000);
    pt[pti] = 0;
    __asm__ volatile("invlpg (%0)" :: "r"(vaddr) : "memory");
    return 0;
}

void* virt_to_phys(void* vaddr) {
    u32 va = (u32)vaddr;
    u32 pdi = PD_INDEX(va);
    u32 pti = PT_INDEX(va);
    if (!(kernel_pd[pdi] & PAGE_PRESENT)) return 0;
    u32* pt = (u32*)(kernel_pd[pdi] & 0xFFFFF000);
    if (!(pt[pti] & PAGE_PRESENT)) return 0;
    return (void*)((pt[pti] & 0xFFFFF000) | (va & 0xFFF));
}
