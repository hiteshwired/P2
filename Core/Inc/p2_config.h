#ifndef P2_CONFIG_H
#define P2_CONFIG_H

#include "stm32l476xx.h"

// ---------------- system clock ----------------
// Clock_init() bumps MSI from the 4 MHz reset value up to 48 MHz.
// 48 MHz divides evenly into the 48 kHz sample rate below.
#define SYSCLK_HZ          48000000UL

// ---------------- waveform sampling ----------------
// One period of the 100 Hz wave is WAVE_POINTS points. Higher frequencies
// step through the same table 2, 3, 4, or 5 points at a time, so
// WAVE_POINTS needs to divide evenly by 3, 4, and 5 (480 does).
#define WAVE_POINTS        480U
#define BASE_FREQ_HZ       100UL
#define SAMPLE_RATE_HZ     (BASE_FREQ_HZ * WAVE_POINTS)        // 48 kHz

// TIM2 runs straight off the 48 MHz clock (PSC = 0) and fires an update
// interrupt every sample. 48 MHz / 48 kHz = 1000 ticks -> ARR = 999.
#define TIM2_PSC_VAL       0U
#define TIM2_ARR_VAL       ((SYSCLK_HZ / SAMPLE_RATE_HZ) - 1UL)

// Fudge factor for the MSI clock error. Measure the 100 Hz output on the
// scope, then: trim = round(1000 * (measured / 100) - 1000). Positive if
// the output is too fast, negative if too slow.
#define TIM2_ARR_TRIM      0

// Frequency multipliers (keys 1-5 -> 100..500 Hz)
#define FREQ_MULT_MIN      1U
#define FREQ_MULT_MAX      5U
#define FREQ_MULT_DEFAULT  1U

// ---------------- amplitude ----------------
// 3.0 Vpp centered on 1.5 V -> swing from 0 V to 3.0 V
#define WAVE_LOW_MV        0U
#define WAVE_HIGH_MV       3000U

// ---------------- square wave duty cycle ----------------
#define DUTY_DEFAULT       50U
#define DUTY_MIN           10U
#define DUTY_MAX           90U
#define DUTY_STEP          10U

// ---------------- ISR timing pin (from A4) ----------------
// Goes high at the start of the TIM2 ISR and low at the end so the ISR
// time can be measured on the scope.
#define OUTPUT_PORT        GPIOC
#define OUTPUT_PIN         8UL
#define OUTPUT_MASK        (1UL << OUTPUT_PIN)

// ---------------- keypad (4 rows x 3 columns) ----------------
// Rows are outputs driven one at a time, columns are inputs with
// pull-downs. All on GPIOC so the DAC keeps GPIOA to itself.
#define KEYPAD_PORT        GPIOC
#define KEYPAD_ROW0_PIN    0UL      // PC0 row 0: 1 2 3
#define KEYPAD_ROW1_PIN    1UL      // PC1 row 1: 4 5 6
#define KEYPAD_ROW2_PIN    2UL      // PC2 row 2: 7 8 9
#define KEYPAD_ROW3_PIN    3UL      // PC3 row 3: * 0 #
#define KEYPAD_COL0_PIN    4UL      // PC4 column 0
#define KEYPAD_COL1_PIN    5UL      // PC5 column 1
#define KEYPAD_COL2_PIN    6UL      // PC6 column 2
#define KEYPAD_ROWS        4U
#define KEYPAD_COLS        3U
#define KEYPAD_ROW_MASK    (0xFUL << KEYPAD_ROW0_PIN)
#define KEYPAD_COL_MASK    (0x7UL << KEYPAD_COL0_PIN)

// crude busy-wait counts (software delay is only used for debouncing)
#define KEYPAD_SETTLE_LOOPS   20U        // let the column lines settle after a row goes high
#define DEBOUNCE_LOOPS        40000U     // roughly 5 ms at 48 MHz

#endif
