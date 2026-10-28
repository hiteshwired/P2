# STM32 Function Generator (DAC + Keypad)

A bare-metal function/waveform generator for the STM32L476RG (Nucleo-64). A
hardware timer and an MCP4921 SPI DAC synthesize the waveform in the background
while a matrix keypad drives a small state machine that changes the signal live.

## Features

- **Four waveform types:** sine, triangle, sawtooth, square (keys 6&ndash;9)
- **Selectable frequency:** 100&ndash;500 Hz in 100 Hz steps (keys 1&ndash;5)
- **Adjustable square-wave duty cycle:** &minus;10% / reset 50% / +10% (keys `*` `0` `#`)
- **Timer-driven generation:** TIM2 fires an ISR that pushes samples to the DAC,
  so the foreground loop only handles the keypad
- Power-up default: 100 Hz square wave at 50% duty

## Architecture

The code is split into focused, register-level modules rather than one monolithic
`main`:

| Module | Responsibility |
|--------|----------------|
| `main.c` | Keypad state machine (press / handle / release with debouncing) |
| `waveform.c/.h` | Look-up tables and the sample engine for each wave type |
| `timer.c/.h` | TIM2 setup and the sample-rate interrupt |
| `DAC.c/.h` | MCP4921 driver over SPI1 with software chip-select framing |
| `keypad.c/.h` | Row/column matrix scanning |
| `gpio.c/.h`, `clock.c/.h` | Pin and 48 MHz clock configuration |
| `p2_config.h` | Tunable constants (frequency map, duty limits, debounce) |

## Hardware

- STM32L476RG (Nucleo-L476RG), running at 48 MHz
- Microchip MCP4921 12-bit SPI DAC on SPI1
- 4&times;3 matrix keypad

## Build

Generated from STM32CubeMX with a CMake build. Build with the included CMake
presets and an `arm-none-eabi` toolchain, or open in STM32CubeIDE, then flash to
the board.

```sh
cmake --preset Debug
cmake --build --preset Debug
```

---

## Academic honesty & licensing

This is a course project from an embedded systems course at Cal Poly, San Luis
Obispo. The application and driver code under `Core/` (`main.c`, `waveform`,
`timer`, `DAC`, `keypad`, `gpio`, `clock`, `p2_config`) is my own work.

The project also bundles STMicroelectronics' STM32L4xx HAL/CMSIS drivers and
CubeMX-generated startup and configuration files, which are distributed under
ST's own license terms and are **not** mine to relicense.

This repository is shared as a learning reference only. It is intentionally not
released under an open-source license, and you should not submit it as your own
work for a graded assignment.
