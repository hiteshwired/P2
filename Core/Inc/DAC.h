#ifndef DAC_H
#define DAC_H

#include "stm32l476xx.h"
#include <stdint.h>

// ---- pins (all GPIOA) ----
#define DAC_CS_PIN        4U      // PA4 = CS (software chip select, active low)
#define DAC_SCLK_PIN      5U      // PA5 = SPI1_SCK  (AF5)
#define DAC_SDI_PIN       7U      // PA7 = SPI1_MOSI (AF5)
#define DAC_AF5           5U      // alternate function number for SPI1

// ---- MCP4921 command word ----
// bit15 = 0 channel A, bit14 = 0 unbuffered, bit13 = 1 gain 1x,
// bit12 = 1 output active. Low 12 bits are the data.
#define DAC_CONFIG_BITS   0x3000U
#define DAC_DATA_MASK     0x0FFFU
#define DAC_MAX_CODE      4095U

// ---- reference / conversion ----
#define DAC_VREF_MV       3300U   // 3.3 V reference in mV
#define DAC_FULL_SCALE    4096U   // 2^12

// ---- SPI1 baud ----
// BR = 001 -> PCLK2 / 4 = 12 MHz with a 48 MHz core. MCP4921 max is 20 MHz.
#define DAC_SPI_BR        0x1U

void     DAC_init(void);                  // GPIOA + SPI1 setup
void     DAC_write(uint16_t code);        // send one 12-bit code to the DAC
uint16_t DAC_volt_conv(uint32_t mv);      // millivolts -> 12-bit code
uint16_t DAC_build_word(uint16_t code);   // config bits + data -> 16-bit word

#endif
