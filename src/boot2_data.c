/* boot2_data.c - Datos del bootloader secundario */
/* Este archivo se genera automáticamente para incluir boot2.bin como símbolo */

#include <stdint.h>

/* Símbolos del bootloader secundario */
uint8_t boot2_bin[] = {
    /* Los datos se incluirán aquí durante la compilación */
    /* Este archivo se genera con: objcopy -O binary boot2.o boot2.bin */
    /* Luego se convierte a C array con: xxd -i boot2.bin > boot2_data.c */
    0x00  /* Placeholder - se reemplaza durante la compilación */
};

uint32_t boot2_bin_len = sizeof(boot2_bin);
