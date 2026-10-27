// Speeds the MCU up from the 4 MHz reset clock to 48 MHz so the TIM2 ISR
// and the SPI transfer finish well inside one sample period.
#include "clock.h"
#include "p2_config.h"

void Clock_init(void)
{
    // 48 MHz needs 2 flash wait states (voltage range 1 is the reset default).
    // Set this BEFORE raising the clock.
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |=  FLASH_ACR_LATENCY_2WS;

    // MSIRGSEL = 1 so the range comes from RCC_CR.MSIRANGE instead of
    // the standby register. MSI is ready after reset so the range can be
    // changed while it is running.
    RCC->CR |= RCC_CR_MSIRGSEL;

    // MSIRANGE = 1011 -> 48 MHz
    RCC->CR &= ~RCC_CR_MSIRANGE;
    RCC->CR |=  RCC_CR_MSIRANGE_11;

    // wait for MSI to settle at the new range
    while ((RCC->CR & RCC_CR_MSIRDY) == 0U)
    {
    }

    // MSI is already the system clock (SW = 00), and the AHB/APB
    // prescalers are all /1, so TIM2 and SPI1 both see 48 MHz now.
}
