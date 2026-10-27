// Look-up tables for the four waveforms plus the little bit of state the
// TIM2 ISR needs to walk through them.
//
// Every table holds one full period of the 100 Hz wave (WAVE_POINTS
// entries of 12-bit DAC codes). The ISR plays a table back at a fixed
// 48 kHz. To get 200 Hz it steps through the table 2 entries at a time,
// 300 Hz steps 3 at a time, and so on. Same table, same sample rate, just a
// bigger step. That is the "plot fewer points per period" idea from the
// project handout.
#include "waveform.h"
#include "DAC.h"
#include "p2_config.h"
#include <math.h>

// one table per shape so switching waves is just swapping a pointer
static uint16_t sine_table[WAVE_POINTS];
static uint16_t triangle_table[WAVE_POINTS];
static uint16_t sawtooth_table[WAVE_POINTS];
static uint16_t square_table[WAVE_POINTS];

// low / high DAC codes for the 0 V to 3.0 V swing
static uint16_t code_low;
static uint16_t code_high;

// what the ISR is currently playing. volatile because main writes these
// while the ISR reads them.
static volatile const uint16_t *active_table = square_table;
static volatile uint32_t        wave_step    = FREQ_MULT_DEFAULT;
static volatile uint32_t        wave_index   = 0U;

// ---- table builders (integer math except the one-time sine) ----

// sawtooth: straight ramp from low to high across the period
static void build_sawtooth(void)
{
    uint32_t range = code_high - code_low;

    for (uint32_t i = 0; i < WAVE_POINTS; i++)
    {
        sawtooth_table[i] = (uint16_t)(code_low + (i * range) / (WAVE_POINTS - 1U));
    }
}

// triangle: ramp up for the first half, ramp back down for the second
static void build_triangle(void)
{
    uint32_t range = code_high - code_low;
    uint32_t half  = WAVE_POINTS / 2U;

    for (uint32_t i = 0; i < WAVE_POINTS; i++)
    {
        if (i < half)
        {
            triangle_table[i] = (uint16_t)(code_low + (i * range) / half);
        }
        else
        {
            triangle_table[i] = (uint16_t)(code_low + ((WAVE_POINTS - i) * range) / half);
        }
    }
}

// square: high for the first duty% of the period, low for the rest
static void build_square(uint32_t duty_percent)
{
    uint32_t high_points = (WAVE_POINTS * duty_percent) / 100U;

    for (uint32_t i = 0; i < WAVE_POINTS; i++)
    {
        if (i < high_points)
        {
            square_table[i] = code_high;
        }
        else
        {
            square_table[i] = code_low;
        }
    }
}

// sine: the only place floating point is used, and it only runs once at
// start-up. After this the ISR just reads the table.
static void build_sine(void)
{
    float mid = (float)(code_high + code_low) / 2.0f;
    float amp = (float)(code_high - code_low) / 2.0f;

    for (uint32_t i = 0; i < WAVE_POINTS; i++)
    {
        float angle = (2.0f * 3.14159265f * (float)i) / (float)WAVE_POINTS;
        sine_table[i] = (uint16_t)(mid + amp * sinf(angle) + 0.5f);
    }
}

// ---- public functions ----

void Waveform_init(void)
{
    code_low  = DAC_volt_conv(WAVE_LOW_MV);
    code_high = DAC_volt_conv(WAVE_HIGH_MV);

    build_sine();
    build_triangle();
    build_sawtooth();
    build_square(DUTY_DEFAULT);

    // power-up default: 100 Hz square, 50% duty
    active_table = square_table;
    wave_step    = FREQ_MULT_DEFAULT;
    wave_index   = 0U;
}

void Waveform_set_type(wave_type_t type)
{
    switch (type)
    {
        case WAVE_SINE:     active_table = sine_table;     break;
        case WAVE_TRIANGLE: active_table = triangle_table; break;
        case WAVE_SAWTOOTH: active_table = sawtooth_table; break;
        case WAVE_SQUARE:
        default:            active_table = square_table;   break;
    }
}

// mult = 1..5 for 100..500 Hz. Anything out of range gets clamped.
void Waveform_set_freq(uint32_t mult)
{
    if (mult < FREQ_MULT_MIN)
    {
        mult = FREQ_MULT_MIN;
    }
    if (mult > FREQ_MULT_MAX)
    {
        mult = FREQ_MULT_MAX;
    }
    wave_step = mult;
}

// Rebuilds the square table in place. The ISR keeps reading it while this
// runs, so one period might look a little off right after a duty change.
void Waveform_set_duty(uint32_t percent)
{
    build_square(percent);
}

// Called from the TIM2 ISR once per sample. Returns the next code and
// moves the index along by the current step, wrapping at the end.
uint16_t Waveform_next_sample(void)
{
    uint16_t code = active_table[wave_index];

    wave_index += wave_step;
    if (wave_index >= WAVE_POINTS)
    {
        wave_index -= WAVE_POINTS;
    }

    return code;
}
