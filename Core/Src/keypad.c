// Driver for the 4x3 matrix keypad. Rows are outputs and get driven high
// one at a time. Columns are inputs with pull-downs, so a pressed key
// pulls its column high while its row is the one being driven.
#include "keypad.h"
#include "p2_config.h"

// what each row/column position means
static const char key_map[KEYPAD_ROWS][KEYPAD_COLS] =
{
    { '1', '2', '3' },
    { '4', '5', '6' },
    { '7', '8', '9' },
    { '*', '0', '#' }
};

void Keypad_init(void)
{
    // GPIOC clock (AHB2)
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

    // rows PC0-PC3: MODER = 01 output, push-pull, no pull
    for (uint32_t pin = KEYPAD_ROW0_PIN; pin <= KEYPAD_ROW3_PIN; pin++)
    {
        KEYPAD_PORT->MODER  &= ~(3UL << (pin * 2UL));
        KEYPAD_PORT->MODER  |=  (1UL << (pin * 2UL));
        KEYPAD_PORT->OTYPER &= ~(1UL << pin);
        KEYPAD_PORT->PUPDR  &= ~(3UL << (pin * 2UL));
    }

    // columns PC4-PC6: MODER = 00 input, PUPDR = 10 pull-down
    for (uint32_t pin = KEYPAD_COL0_PIN; pin <= KEYPAD_COL2_PIN; pin++)
    {
        KEYPAD_PORT->MODER &= ~(3UL << (pin * 2UL));
        KEYPAD_PORT->PUPDR &= ~(3UL << (pin * 2UL));
        KEYPAD_PORT->PUPDR |=  (2UL << (pin * 2UL));
    }

    // all rows low to start
    KEYPAD_PORT->BRR = KEYPAD_ROW_MASK;
}

// Scan the keypad once. Returns the first key found or KEY_NONE.
char Keypad_read(void)
{
    char key = KEY_NONE;

    for (uint32_t row = 0; row < KEYPAD_ROWS; row++)
    {
        // drive only this row high
        KEYPAD_PORT->BRR  = KEYPAD_ROW_MASK;
        KEYPAD_PORT->BSRR = (1UL << (KEYPAD_ROW0_PIN + row));

        // give the column lines a moment to settle
        for (volatile uint32_t i = 0; i < KEYPAD_SETTLE_LOOPS; i++)
        {
        }

        // read the three column bits
        uint32_t cols = (KEYPAD_PORT->IDR & KEYPAD_COL_MASK) >> KEYPAD_COL0_PIN;

        if (cols != 0U)
        {
            // figure out which column is high
            for (uint32_t col = 0; col < KEYPAD_COLS; col++)
            {
                if ((cols & (1UL << col)) != 0U)
                {
                    key = key_map[row][col];
                    break;
                }
            }
            break;
        }
    }

    // leave the rows low between scans
    KEYPAD_PORT->BRR = KEYPAD_ROW_MASK;

    return key;
}
