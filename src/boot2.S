/* boot2.S - Bootloader secundario para ZyphraOS */
/* Carga el kernel desde FAT16, activa modo protegido y salta a él */
/* Compilar con: gcc -m32 -c boot2.S -o boot2.o */
/* Enlazar como binario plano en 0x7C00 */

.code16
.section .text
.org 0x7C00
.global _start2
_start2:
    /* Configurar segmentos y stack */
    cli
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov $0x7C00, %sp
    sti

    /* Guardar número de disco */
    mov %dl, (disk_number)

    /* Activar A20 */
    inb $0x92, %al
    orb $0x02, %al
    outb %al, $0x92

    /* Leer BPB del sector de arranque de la partición */
    mov $0x02, %ah        /* INT 13h AH=02: Leer sectores */
    mov $0x01, %al        /* 1 sector */
    mov $0x00, %ch        /* Cilindro 0 */
    mov $0x02, %cl        /* Sector 2 (después del MBR) */
    mov $0x00, %dh        /* Cabeza 0 */
    mov (disk_number), %dl
    mov $bpb_buffer, %bx  /* Buffer para BPB */
    int $0x13
    jc disk_error

    /* Verificar firma de arranque */
    cmpw $0xAA55, bpb_buffer + 510
    jne disk_error

    /* Calcular ubicación de FAT, root y datos */
    /* reserved_sectors está en offset 14 del BPB */
    movw bpb_buffer + 14, %ax
    movw %ax, (fat_start)
    
    /* num_fats está en offset 16 */
    movb bpb_buffer + 16, %al
    movb %al, (num_fats)
    
    /* sectors_per_fat está en offset 22 */
    movw bpb_buffer + 22, %ax
    movw %ax, (sectors_per_fat)
    
    /* root_entries está en offset 17 */
    movw bpb_buffer + 17, %ax
    movw %ax, (root_entries)
    
    /* sectors_per_cluster está en offset 13 */
    movb bpb_buffer + 13, %al
    movb %al, (sectors_per_cluster)

    /* Calcular root_start = fat_start + num_fats * sectors_per_fat */
    movw (fat_start), %ax
    movw %ax, (root_start)  /* root_start empieza en fat_start */
    movb (num_fats), %cl
    xor %ch, %ch
    movw (sectors_per_fat), %dx
    mul %dx
    addw %ax, (root_start)  /* root_start = fat_start + num_fats * sectors_per_fat */

    /* Calcular data_start = root_start + (root_entries * 32 / 512) */
    movw (root_start), %ax
    movw %ax, (data_start)  /* data_start empieza en root_start */
    movw (root_entries), %ax
    movw $32, %dx
    mul %dx
    movw $512, %cx
    div %cx
    addw %ax, (data_start)  /* data_start = root_start + root_dir_sectors */

    /* Buscar archivo "ZYPHRAOS ELF" en directorio raíz */
    movw $0, (root_sector_counter)
    
search_root:
    /* Leer sector del directorio raíz */
    movw (root_sector_counter), %ax
    addw (root_start), %ax  /* Sumar offset del directorio raíz */
    movw %ax, (current_sector)
    
    call read_sector
    
    /* Buscar entrada "ZYPHRAOS ELF" */
    movw $sector_buffer, %si
    movw $16, %cx          /* 16 entradas por sector */
    
check_entry:
    /* Verificar si la entrada está vacía */
    cmpb $0x00, (%si)
    je not_found
    cmpb $0xE5, (%si)
    je next_entry
    
    /* Verificar nombre "ZYPHRAOS" */
    movw $name_zyphraos, %di
    movw $8, %cx
    repe cmpsb
    jne next_entry
    
    /* Verificar extensión "ELF" */
    movw $ext_elf, %di
    movw $3, %cx
    repe cmpsb
    jne next_entry
    
    /* ¡Encontrado! Obtener primer cluster */
    movw 26(%si), %ax      /* first_cluster está en offset 26 */
    movw %ax, (first_cluster)
    jmp load_kernel
    
next_entry:
    addw $32, %si          /* Siguiente entrada */
    jmp check_entry
    
not_found:
    /* Buscar en siguiente sector del directorio raíz */
    incw (root_sector_counter)
    movw (root_entries), %ax
    shr $4, %ax            /* Dividir entre 16 */
    cmpw (root_sector_counter), %ax
    jg search_root
    
    /* Archivo no encontrado */
    jmp file_error

load_kernel:
    /* Cargar kernel en buffer temporal bajo 1MB (0x10000) */
    movw $0x1000, %ax
    movw %ax, %es          /* Segmento 0x1000 = 0x10000 */
    xor %bx, %bx           /* Offset 0 */
    
    movw (first_cluster), %ax
    movw %ax, (current_cluster)
    
load_cluster:
    /* Calcular LBA del cluster */
    /* LBA = data_start + (cluster - 2) * sectors_per_cluster */
    movw (current_cluster), %ax
    subw $2, %ax
    movb (sectors_per_cluster), %cl
    xor %ch, %ch
    mul %cx
    addw (data_start), %ax
    movw %ax, (current_cluster_lba)
    
    /* Leer sectores del cluster */
    movb (sectors_per_cluster), %cl
    movb %cl, (sectors_to_read)
    
read_cluster_loop:
    movw (current_cluster_lba), %ax
    call read_sector_lba
    
    /* Avanzar buffer */
    addw $512, %bx
    incw (current_cluster_lba)
    decb (sectors_to_read)
    jnz read_cluster_loop
    
    /* Leer siguiente cluster de la FAT */
    movw (current_cluster), %ax
    call get_fat_entry
    movw %ax, (current_cluster)
    
    /* Verificar si es el último cluster */
    cmpw $0xFFF8, %ax
    jb load_cluster
    
    /* Kernel cargado en 0x10000, ahora configurar modo protegido */
    
    /* Leer entry point del ELF header */
    movw $0x1000, %ax
    movw %ax, %es
    movl %es:0x18, %eax    /* e_entry está en offset 24 (32 bits) */
    movl %eax, (entry_point)
    
    /* Leer offset de programa del ELF */
    movl %es:0x1C, %eax    /* e_phoff está en offset 28 */
    movl %eax, (ph_offset)
    
    /* Leer tamaño de entrada de programa */
    movw %es:0x2A, %ax     /* e_phentsize está en offset 42 */
    movw %ax, (ph_entry_size)
    
    /* Leer número de entradas de programa */
    movw %es:0x2C, %ax     /* e_phnum está en offset 44 */
    movw %ax, (ph_num)
    
    /* Copiar segmentos ELF a 0x100000 */
    call copy_elf_segments
    
    /* Deshabilitar interrupciones */
    cli
    
    /* Cargar GDT para modo protegido */
    lgdt gdt_ptr
    
    /* Cargar IDT vacía pero válida antes de entrar a modo protegido */
    lidt idt_ptr
    
    /* Activar modo protegido */
    movl %cr0, %eax
    orl $0x01, %eax        /* Set PE bit */
    movl %eax, %cr0
    
    /* Saltar para limpiar prefetch y cargar CS */
    ljmp $0x08, $pm_entry

/* Función: Copiar segmentos ELF a 0x100000 */
copy_elf_segments:
    pusha
    
    /* En modo real, solo podemos copiar a memoria baja */
    /* La copia a 0x100000 se hará en modo protegido */
    /* Esta función prepara los datos necesarios */
    
    popa
    ret

/* Manejador de excepciones simple */
.code32
exception_handler:
    cli
    hlt
    jmp exception_handler

/* Modo protegido - 32 bits */
.code32
pm_entry:
    /* Configurar segmentos de datos */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov $0x10, %ax
    mov %ax, %ss
    
    /* Configurar stack */
    mov $0x90000, %esp
    
    /* Llenar IDT con dirección del manejador de excepciones */
    call fill_idt
    
    /* Copiar kernel de 0x10000 a 0x100000 */
    call copy_kernel_to_high
    
    /* Saltar al entry point del kernel */
    movl (entry_point), %eax
    jmp *%eax

/* Función: Llenar IDT con dirección del manejador de excepciones */
fill_idt:
    pusha
    mov $idt_table, %edi
    mov $256, %ecx
.fill_loop:
    movl $exception_handler, (%edi)
    add $8, %edi
    dec %ecx
    jnz .fill_loop
    popa
    ret

/* Función: Copiar segmentos ELF a 0x100000 (modo protegido) */
copy_kernel_to_high:
    pusha
    
    /* Iterar a través de los program headers */
    xor %ebp, %ebp         /* ebp = índice actual */
    movl (ph_offset), %esi /* offset de la tabla de programa */
    addl $0x10000, %esi    /* ajustar a dirección física */
    
.phdr_loop:
    cmpw (ph_num), %bp
    jge .phdr_done
    
    /* Verificar si es PT_LOAD (p_type == 1) */
    cmpl $1, (%esi)
    jne .next_phdr
    
    /* Obtener p_offset (offset 4) y calcular fuente */
    movl 4(%esi), %ebx
    addl $0x10000, %ebx    /* ebx = fuente (p_offset + 0x10000) */
    
    /* Obtener p_vaddr (offset 8) */
    movl 8(%esi), %edi     /* edi = destino (p_vaddr) */
    
    /* Obtener p_filesz (offset 16) */
    movl 16(%esi), %ecx    /* ecx = tamaño a copiar */
    
    /* Copiar segmento - guardar %esi antes de copiar */
    pushl %esi
    movl %ebx, %esi        /* esi = fuente */
    rep movsb
    popl %esi              /* restaurar %esi al phdr */
    
    /* Obtener p_memsz (offset 20) */
    movl 20(%esi), %ecx
    subl 16(%esi), %ecx    /* ecx = p_memsz - p_filesz */
    jz .next_phdr
    
    /* Zero out .bss (p_memsz - p_filesz) */
    xor %al, %al
    rep stosb
    
.next_phdr:
    /* Avanzar al siguiente program header */
    movzwl (ph_entry_size), %eax
    addl %eax, %esi
    incw %bp
    jmp .phdr_loop
    
.phdr_done:
    popa
    ret

/* Función: Leer sector usando INT 13h (modo real) */
.code16
read_sector:
    pusha
    movw (current_sector), %ax
    /* Convertir LBA a CHS */
    /* Asumiendo geometría: 63 sectores/pista, 255 cabezas */
    movw $63, %cx
    xor %dx, %dx
    div %cx
    inc %dx                /* Sector = (LBA % 63) + 1 */
    movb %dl, %cl
    xor %dx, %dx
    movw $255, %cx
    div %cx
    movb %dl, %dh          /* Cabeza = (LBA / 63) % 255 */
    movb %al, %ch          /* Cilindro = (LBA / 63) / 255 */
    movb $0x01, %al        /* 1 sector */
    movb (disk_number), %dl
    movw $sector_buffer, %bx
    movb $0x02, %ah
    int $0x13
    popa
    ret

/* Función: Leer sector a buffer ES:BX usando LBA */
read_sector_lba:
    pusha
    movw %ax, (temp_lba)
    /* Convertir LBA a CHS */
    movw $63, %cx
    xor %dx, %dx
    div %cx
    inc %dx
    movb %dl, %cl
    xor %dx, %dx
    movw $255, %cx
    div %cx
    movb %dl, %dh
    movb %al, %ch
    movb $0x01, %al
    movb (disk_number), %dl
    movb $0x02, %ah
    int $0x13
    popa
    ret

/* Función: Obtener entrada de la FAT */
get_fat_entry:
    push %bx
    push %cx
    push %dx
    
    /* Calcular offset en FAT */
    movw %ax, %cx
    shr $8, %cx            /* Dividir entre 256 */
    addw (fat_start), %cx
    movw %cx, (current_sector)
    
    call read_sector
    
    /* Obtener entrada */
    movw %ax, %cx
    andw $0xFF, %cx
    shl $1, %cx
    movw sector_buffer, %bx
    addw %cx, %bx
    movw (%bx), %ax
    
    pop %dx
    pop %cx
    pop %bx
    ret

/* Rutinas de error */
disk_error:
    movw $msg_disk_error, %si
    call print_msg
    jmp halt

file_error:
    movw $msg_file_error, %si
    call print_msg
    jmp halt

halt:
    cli
    hlt
    jmp halt

/* Función: Imprimir mensaje (modo real) */
print_msg:
    pusha
    movb $0x0E, %ah
.loop:
    lodsb
    test %al, %al
    jz .done
    int $0x10
    jmp .loop
.done:
    popa
    ret

/* IDT mínima para manejar excepciones básicas */
.align 16
idt_table:
    .rept 256
    .word 0x0000              /* offset_lo - se llenará después */
    .word 0x0008              /* selector: código ring0 */
    .byte 0x00                /* zero */
    .byte 0x8E                /* type_attr: presente, ring0, interrupt gate */
    .word 0x0000              /* offset_hi - se llenará después */
    .endr
idt_table_end:

/* GDT para modo protegido */
.align 16
gdt_table:
    .quad 0x0000000000000000   /* 0x00: null */
    .quad 0x00CF9A000000FFFF   /* 0x08: code32 ring0 */
    .quad 0x00CF92000000FFFF   /* 0x10: data32 ring0 */
gdt_end:

gdt_ptr:
    .word gdt_end - gdt_table - 1
    .long gdt_table

/* IDT vacía pero válida para modo protegido */
.align 16
idt_ptr:
    .word idt_table_end - idt_table - 1  /* Límite: tamaño IDT - 1 */
    .long idt_table                       /* Base: dirección de la IDT */

/* Datos */
disk_number:        .byte 0
fat_start:          .word 0
root_start:         .word 0
data_start:         .word 0
num_fats:           .byte 0
sectors_per_fat:    .word 0
root_entries:       .word 0
sectors_per_cluster: .byte 0
root_sector_counter: .word 0
current_sector:     .word 0
first_cluster:      .word 0
current_cluster:    .word 0
current_cluster_lba: .word 0
sectors_to_read:    .byte 0
entry_point:        .long 0
ph_offset:          .long 0
ph_entry_size:      .word 0
ph_num:             .word 0
temp_lba:           .word 0
elf_size:           .long 0

/* Nombres de archivo */
name_zyphraos:      .ascii "ZYPHRAOS"
ext_elf:            .ascii "ELF"

/* Mensajes de error */
msg_disk_error:     .asciz "Error de disco\r\n"
msg_file_error:     .asciz "Kernel no encontrado\r\n"

/* Buffers */
bpb_buffer:         .skip 512
sector_buffer:      .skip 512

/* Firma de arranque al final del sector */
.word 0xAA55
