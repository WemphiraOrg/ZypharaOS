/* boot.S - ZyphraOS entry point */
/* Punto de entrada del kernel, compatible con bootloader secundario */

/* Header Multiboot para GRUB */
.section .multiboot
.align 4
multiboot_header:
    .long 0x1BADB002        /* magic */
    .long 0x00000003        /* flags: align + meminfo */
    .long -(0x1BADB002 + 0x00000003)  /* checksum */

.section .text
.global _start
.type _start, @function
_start:
    cli
    
    /* El bootloader secundario ya activó modo protegido y configuró GDT */
    /* Solo necesitamos configurar segmentos y stack */
    
    /* Configurar segmentos de datos */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    /* Stack alineado a 16 bytes */
    mov $stack_top, %esp
    and $0xFFFFFFF0, %esp
    
    /* FPU init */
    fninit
    
    /* Llamar kernel */
    /* No pasamos parámetros Multiboot, el kernel debe funcionar sin ellos */
    xor %eax, %eax         /* magic = 0 */
    xor %ebx, %ebx         /* mbi = 0 */
    push %ebx
    push %eax
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang
.size _start, . - _start

/* Stack del kernel: 64KB */
.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
    .skip 65536
stack_top:

/* GDT: null, code32, data32, code16(unused), TSS placeholder */
.section .data
.align 16
gdt_table:
    .quad 0x0000000000000000   /* 0x00: null */
    .quad 0x00CF9A000000FFFF   /* 0x08: code32 ring0 */
    .quad 0x00CF92000000FFFF   /* 0x10: data32 ring0 */
    .quad 0x00CFFA000000FFFF   /* 0x18: code32 ring3 (user, futuro) */
    .quad 0x00CFF2000000FFFF   /* 0x20: data32 ring3 (user, futuro) */
gdt_end:

gdt_ptr:
    .word gdt_end - gdt_table - 1
    .long gdt_table
