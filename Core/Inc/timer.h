#ifndef TIMER_H
#define TIMER_H

#include "stm32l476xx.h"

void TIM2_init(void);   // set the sample period and enable the update interrupt
void TIM2_start(void);  // start the counter
void TIM2_stop(void);   // stop the counter

#endif
