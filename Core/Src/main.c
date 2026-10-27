// P2 - Function Generator
//
// TIM2 + the MCP4921 DAC make the waveform in the background (see timer.c
// and waveform.c). All main() does is run a small state machine that
// watches the keypad and pushes any changes into the waveform module.
//
//   keys 1-5   : frequency 100-500 Hz
//   keys 6-9   : sine / triangle / sawtooth / square
//   * 0 #      : square duty -10% / reset 50% / +10% (square only)
//
// Power-up default is a 100 Hz square wave at 50% duty.
#include "stm32l476xx.h"
#include "p2_config.h"
#include "clock.h"
#include "gpio.h"
#include "DAC.h"
#include "keypad.h"
#include "waveform.h"
#include "timer.h"

// ---- keypad state machine ----
typedef enum
{
    STATE_WAIT_PRESS,     // scanning, nothing pressed yet
    STATE_HANDLE_KEY,     // a debounced key came in, act on it
    STATE_WAIT_RELEASE    // key handled, wait for the finger to come off
} key_state_t;

// ---- what the generator is currently set to ----
typedef struct
{
    wave_type_t type;     // which shape
    uint32_t    freq_mult; // 1..5 -> 100..500 Hz
    uint32_t    duty;      // square duty in percent
} gen_settings_t;

static gen_settings_t gen;

// crude software delay, only used for keypad debouncing
static void delay_loops(volatile uint32_t n)
{
    while (n--)
    {
        __asm volatile ("nop");
    }
}

// Apply one key press to the generator settings.
static void handle_key(char key)
{
    switch (key)
    {
        // frequency
        case '1': case '2': case '3': case '4': case '5':
            gen.freq_mult = (uint32_t)(key - '0');
            Waveform_set_freq(gen.freq_mult);
            break;

        // waveform type
        case '6':
            gen.type = WAVE_SINE;
            Waveform_set_type(gen.type);
            break;
        case '7':
            gen.type = WAVE_TRIANGLE;
            Waveform_set_type(gen.type);
            break;
        case '8':
            gen.type = WAVE_SAWTOOTH;
            Waveform_set_type(gen.type);
            break;
        case '9':
            gen.type = WAVE_SQUARE;
            Waveform_set_type(gen.type);
            break;

        // duty cycle, only does anything while on the square wave
        case '*':
            if (gen.type == WAVE_SQUARE && gen.duty > DUTY_MIN)
            {
                gen.duty -= DUTY_STEP;
                Waveform_set_duty(gen.duty);
            }
            break;
        case '#':
            if (gen.type == WAVE_SQUARE && gen.duty < DUTY_MAX)
            {
                gen.duty += DUTY_STEP;
                Waveform_set_duty(gen.duty);
            }
            break;
        case '0':
            if (gen.type == WAVE_SQUARE)
            {
                gen.duty = DUTY_DEFAULT;
                Waveform_set_duty(gen.duty);
            }
            break;

        default:
            break;
    }
}

int main(void)
{
    key_state_t state = STATE_WAIT_PRESS;
    char        key   = KEY_NONE;

    // hardware setup
    Clock_init();      // 48 MHz
    GPIO_init();       // ISR timing pin
    DAC_init();        // SPI1 + chip select
    Keypad_init();     // rows / columns
    Waveform_init();   // build the look-up tables

    // power-up defaults (Req 5): 100 Hz square, 50% duty
    gen.type      = WAVE_SQUARE;
    gen.freq_mult = FREQ_MULT_DEFAULT;
    gen.duty      = DUTY_DEFAULT;
    Waveform_set_type(gen.type);
    Waveform_set_freq(gen.freq_mult);
    Waveform_set_duty(gen.duty);

    // start generating
    TIM2_init();
    TIM2_start();

    while (1)
    {
        switch (state)
        {
            case STATE_WAIT_PRESS:
                key = Keypad_read();
                if (key != KEY_NONE)
                {
                    // debounce: wait a bit and make sure it is still the same key
                    delay_loops(DEBOUNCE_LOOPS);
                    if (Keypad_read() == key)
                    {
                        state = STATE_HANDLE_KEY;
                    }
                }
                break;

            case STATE_HANDLE_KEY:
                handle_key(key);
                state = STATE_WAIT_RELEASE;
                break;

            case STATE_WAIT_RELEASE:
                // sit here until the key is let go, then debounce the release
                if (Keypad_read() == KEY_NONE)
                {
                    delay_loops(DEBOUNCE_LOOPS);
                    if (Keypad_read() == KEY_NONE)
                    {
                        state = STATE_WAIT_PRESS;
                    }
                }
                break;

            default:
                state = STATE_WAIT_PRESS;
                break;
        }
    }
}
