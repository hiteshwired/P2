#ifndef WAVEFORM_H
#define WAVEFORM_H

#include "stm32l476xx.h"
#include <stdint.h>

// the four output shapes (keys 6-9)
typedef enum
{
    WAVE_SINE = 0,
    WAVE_TRIANGLE,
    WAVE_SAWTOOTH,
    WAVE_SQUARE
} wave_type_t;

void     Waveform_init(void);                    // build all four tables
void     Waveform_set_type(wave_type_t type);    // pick which table plays
void     Waveform_set_freq(uint32_t mult);       // 1..5 -> 100..500 Hz
void     Waveform_set_duty(uint32_t percent);    // rebuild the square table
uint16_t Waveform_next_sample(void);             // called by the TIM2 ISR

#endif
