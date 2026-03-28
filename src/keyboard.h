#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "io.h"
void keyboard_init(void);
char keyboard_getchar(void);    /* bloqueante */
int  keyboard_poll(void);       /* no bloqueante, -1 si nada */
int  keyboard_has_key(void);
#endif
