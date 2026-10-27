// Library for the Microchip MCP4921 12-bit SPI DAC. Brought over from A5.
// The only real change for P2 is a faster SPI clock, since DAC_write() now
// gets called from the TIM2 ISR 48,000 times a second.
#include "DAC.h"

// millivolts -> 12-bit DAC code, integer math only
uint16_t DAC_volt_conv(uint32_t mv)
{
    uint32_t code = (mv * DAC_FULL_SCALE) / DAC_VREF_MV;

    // clamp to full scale
    if (code > DAC_MAX_CODE)
    {
        code = DAC_MAX_CODE;
    }

    return (uint16_t)code;
}

// Put the config nibble in the top 4 bits and the data in the low 12.
uint16_t DAC_build_word(uint16_t code)
{
    return (uint16_t)(DAC_CONFIG_BITS | (code & DAC_DATA_MASK));
}

// Send one 16-bit command word, MSB first, with CS framed around it.
// DR is written as a byte so each store sends exactly one 8-bit frame.
void DAC_write(uint16_t code)
{
    uint16_t word = DAC_build_word(code);

    // CS low before any clocks
    GPIOA->BSRR = (1U << (DAC_CS_PIN + 16U));

    // high byte
    while ((SPI1->SR & SPI_SR_TXE) == 0U)
    {
    }
    *(volatile uint8_t *)&SPI1->DR = (uint8_t)(word >> 8);

    // low byte
    while ((SPI1->SR & SPI_SR_TXE) == 0U)
    {
    }
    *(volatile uint8_t *)&SPI1->DR = (uint8_t)(word & 0xFFU);

    // wait for the last bit to actually shift out before raising CS
    while ((SPI1->SR & SPI_SR_TXE) == 0U)
    {
    }
    while ((SPI1->SR & SPI_SR_BSY) != 0U)
    {
    }

    // CS high latches the new value into the DAC
    GPIOA->BSRR = (1U << DAC_CS_PIN);
}

// GPIOA + SPI1 setup. Clocks first, then pin modes, then AF, then SPI
// config, and SPE last.
void DAC_init(void)
{
    // clocks: GPIOA on AHB2, SPI1 on APB2
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // MODER: PA5/PA7 -> AF (10), PA4 -> output (01)
    GPIOA->MODER &= ~((3U << (2U * DAC_SCLK_PIN)) |
                      (3U << (2U * DAC_SDI_PIN))  |
                      (3U << (2U * DAC_CS_PIN)));
    GPIOA->MODER |=  ((2U << (2U * DAC_SCLK_PIN)) |
                      (2U << (2U * DAC_SDI_PIN))  |
                      (1U << (2U * DAC_CS_PIN)));

    // push-pull, high speed on the clock and data lines
    GPIOA->OTYPER  &= ~((1U << DAC_SCLK_PIN) | (1U << DAC_SDI_PIN) | (1U << DAC_CS_PIN));
    GPIOA->OSPEEDR |=  ((3U << (2U * DAC_SCLK_PIN)) | (3U << (2U * DAC_SDI_PIN)));

    // AFR[0] covers pins 0..7: PA5 and PA7 -> AF5
    GPIOA->AFR[0] &= ~((0xFU << (4U * DAC_SCLK_PIN)) | (0xFU << (4U * DAC_SDI_PIN)));
    GPIOA->AFR[0] |=  ((DAC_AF5 << (4U * DAC_SCLK_PIN)) | (DAC_AF5 << (4U * DAC_SDI_PIN)));

    // CS high (inactive) before SPI gets enabled
    GPIOA->BSRR = (1U << DAC_CS_PIN);

    // CR1: master, mode 0 (CPOL = 0, CPHA = 0), MSB first, software NSS
    SPI1->CR1  = 0U;
    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR1 |= (DAC_SPI_BR << SPI_CR1_BR_Pos);
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;

    // CR2: 8-bit frames, RXNE on one byte
    SPI1->CR2 &= ~SPI_CR2_DS;
    SPI1->CR2 |= (0x7U << SPI_CR2_DS_Pos);
    SPI1->CR2 |= SPI_CR2_FRXTH;

    // enable SPI1 last
    SPI1->CR1 |= SPI_CR1_SPE;
}
