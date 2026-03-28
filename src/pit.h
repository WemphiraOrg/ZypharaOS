#ifndef PIT_H
#define PIT_H
#include "io.h"
void pit_init(u32 hz);
void pit_sleep(u32 ms);
u32  pit_ticks(void);
u32  pit_hz(void);
#endif
