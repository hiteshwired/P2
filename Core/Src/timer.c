// TIM2 fires an update interrupt once per sample (48 kHz). The ISR pulls
// the next point from the waveform table and writes it to the DAC.
// Based on the A4 timer code, minus the CCR1 compare since the output
// level now comes from the table instead of the timer.
#include "timer.h"
#include "gpio.h"
#include "DAC.h"
#include "waveform.h"
#include "p2_config.h"

void TIM2_init(void)
{
    // turn on the TIM2 clock (APB1)
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    // make sure the timer is off while we set it up
    TIM2->CR1 &= ~TIM_CR1_CEN;

    // up-counting, edge-aligned
    TIM2->CR1 &= ~TIM_CR1_DIR;
    TIM2->CR1 &= ~TIM_CR1_CMS;

    // PSC = 0 so the counter runs at the full 48 MHz clock
    TIM2->PSC = TIM2_PSC_VAL;

    // ARR sets the sample period (plus the trim from measuring on the scope)
    TIM2->ARR = (uint32_t)((int32_t)TIM2_ARR_VAL + TIM2_ARR_TRIM);

    // only the update interrupt is needed now
    TIM2->DIER |= TIM_DIER_UIE;

    // UG loads PSC/ARR right away, but it also sets UIF
    TIM2->EGR |= TIM_EGR_UG;

    // clear the flag left over from the UG
    TIM2->SR &= ~TIM_SR_UIF;

    // enable TIM2 in the NVIC, then enable interrupts globally
    NVIC->ISER[TIM2_IRQn >> 5] = (1UL << (TIM2_IRQn & 0x1FU));
    __enable_irq();
}

void TIM2_start(void)
{
    TIM2->CNT = 0U;        // start from a known count
    TIM2->CR1 |= TIM_CR1_CEN;
}

void TIM2_stop(void)
{
    TIM2->CR1 &= ~TIM_CR1_CEN;
}

// One sample per interrupt: clear the flag, write the next point.
// The timing pin brackets the whole thing for the scope measurement.
void TIM2_IRQHandler(void)
{
    GPIO_set_output();

    if ((TIM2->SR & TIM_SR_UIF) != 0U)
    {
        TIM2->SR &= ~TIM_SR_UIF;
        DAC_write(Waveform_next_sample());
    }

    GPIO_clear_output();
}
