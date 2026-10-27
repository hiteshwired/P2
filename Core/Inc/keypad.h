#ifndef KEYPAD_H
#define KEYPAD_H

#include "stm32l476xx.h"

#define KEY_NONE   0      // nothing pressed

void Keypad_init(void);   // set up row outputs and column inputs
char Keypad_read(void);   // returns '0'-'9', '*', '#', or KEY_NONE

#endif
