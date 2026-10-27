// Driver for the pin the TIM2 ISR toggles so the ISR time can be measured
// on the scope. Carried over from A4, only the pin number changed.
#include "gpio.h"
#include "p2_config.h"

// Set up the output pin: push-pull, no pull resistors, high speed.
void GPIO_init(void)
{
    // turn on the GPIOC clock (it lives on the AHB2 bus)
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

    // MODER = 01 for general purpose output
    OUTPUT_PORT->MODER &= ~(3UL << (OUTPUT_PIN * 2UL));
    OUTPUT_PORT->MODER |=  (1UL << (OUTPUT_PIN * 2UL));

    // OTYPER = 0 for push-pull
    OUTPUT_PORT->OTYPER &= ~OUTPUT_MASK;

    // PUPDR = 00 for no pull-up / pull-down
    OUTPUT_PORT->PUPDR &= ~(3UL << (OUTPUT_PIN * 2UL));

    // OSPEEDR = 11 for high speed so the edges stay sharp
    OUTPUT_PORT->OSPEEDR |= (3UL << (OUTPUT_PIN * 2UL));

    // start low so we have a known state
    GPIO_clear_output();
}

// Drive the pin high. BSRR sets the bit without a read-modify-write.
void GPIO_set_output(void)
{
    OUTPUT_PORT->BSRR = OUTPUT_MASK;
}

// Drive the pin low. BRR clears the bit the same atomic way.
void GPIO_clear_output(void)
{
    OUTPUT_PORT->BRR = OUTPUT_MASK;
}
