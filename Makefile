CC      = gcc
AS      = gcc
LD      = ld
CFLAGS  = -m32 -ffreestanding -nostdlib -nostdinc -fno-stack-protector \
          -fno-pie -no-pie -O2 -Wno-unused-function -Wno-unused-variable -Isrc
ASFLAGS = -m32
LDFLAGS = -m elf_i386 -T src/linker.ld --oformat=elf32-i386

OBJS = boot.o irq_stubs.o kernel.o terminal.o vbe.o \
       keyboard.o pit.o idt.o pmm.o paging.o heap.o \
       process.o cpuinfo.o ata.o fat16.o mbr_boot.o \
       install.o sysmon.o snake.o shell.o boot2_data.o

KERNEL = iso/boot/zyphraos.elf
ISO    = zyphraos.iso
BOOT2  = boot2.bin

all: $(ISO) $(BOOT2)

%.o: src/%.S ; $(AS) $(ASFLAGS) -c $< -o $@
%.o: src/%.c ; $(CC) $(CFLAGS) -c $< -o $@
boot2_data.o: boot2_data.c ; $(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo ""
	@echo "  +---------------------------------+"
	@echo "  |  ZyphraOS kernel compilado!     |"
	@size $@ | tail -1 | awk '{printf "  |  text=%-6s bss=%-8s      |\n",$$1,$$3}'
	@echo "  +---------------------------------+"

$(ISO): $(KERNEL)
	@grub-mkrescue -o $@ iso 2>/dev/null && echo "  ISO: $@ OK" || \
	 grub2-mkrescue -o $@ iso 2>/dev/null && echo "  ISO: $@ OK"

$(BOOT2): src/boot2.S
	$(AS) $(ASFLAGS) -c $< -o boot2.o
	$(LD) -m elf_i386 -T /dev/null --oformat=binary -o $@ boot2.o
	@echo "  BOOT2: $@ OK"

boot2_data.c: $(BOOT2)
	@echo "  Generando boot2_data.c..."
	@xxd -i $< > $@
	@echo "  boot2_data.c OK"

clean:
	rm -f *.o $(KERNEL) $(ISO) $(BOOT2) boot2_data.c

# Crear disco virtual 256MB
disco:
	qemu-img create disco.img 256M
	@echo "  disco.img 256MB creado"

# Boot desde ISO + disco
run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -hda disco.img -m 64M -vga std

# Boot VBE 800x600
run-vbe: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -hda disco.img -m 64M \
	  -vga std -device VGA,vgamem_mb=16

# Boot desde disco (tras instalar)
run-disk:
	qemu-system-i386 -hda disco.img -m 64M -vga std

# Boot con más RAM para pruebas
run-big: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -hda disco.img -m 256M -vga std

.PHONY: all clean run run-vbe run-disk run-big disco
