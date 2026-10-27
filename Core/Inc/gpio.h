#ifndef GPIO_H
#define GPIO_H

#include "stm32l476xx.h"

void GPIO_init(void);          // set up the ISR timing pin
void GPIO_set_output(void);    // drive the pin high
void GPIO_clear_output(void);  // drive the pin low

#endif
