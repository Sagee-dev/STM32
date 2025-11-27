# STM32 Bare-Metal Firmware Development  
Learning on the STM32 NUCLEO-F401RE (No HAL, No CubeMX)

This repository documents my learning journey into embedded firmware development using the STM32 NUCLEO-F401RE development board.  
The focus of this project is to understand the STM32F401RE microcontroller at a low level using pure C, CMSIS headers, and direct register programming.  
No HAL, no CubeMX, and no vendor-generated code are used in any part of the firmware.

---

## Purpose of This Repository

The goal of this repository is to build a strong foundation in low-level embedded systems by:

- Writing firmware from scratch using only CMSIS and reference manuals.
- Understanding how STM32 peripherals work at the register level.
- Learning the ARM Cortex-M4 architecture, SysTick, NVIC, and startup process.
- Developing basic drivers for common peripherals without vendor libraries.
- Managing a clean project architecture, including Makefiles, linker scripts, and startup code.
- Creating a structured learning path with self-contained tasks.

---

## Hardware Used

- Development Board: STM32 NUCLEO-F401RE  
- Microcontroller: STM32F401RE (ARM Cortex-M4, 84 MHz, ARMv7E-M)

---

## Repository Structure

Each topic or experiment is placed in its own project folder.  
Shared code and device headers (CMSIS, startup files, linker scripts) are stored in a common directory.

Example structure:

STM32_BareMetal/
│
├── common/
│   ├── Drivers/
│   │   ├── CMSIS/Core/Include
│   │   └── CMSIS/Device/ST/STM32F4xx/Include
│   ├── startup/
│   │   └── startup_stm32f401xx.s
│   ├── linker/
│   │   └── stm32f401.ld
│   ├── system_stm32f4xx.c
│   └── stubs.c
│
├── GPIO_Blink/
    ├── src/main.c
    ├── Makefile
    └── README.md



---

## Topics Covered in This Repository

### ARM Cortex-M Core Concepts
- Vector tables and exception handling
- Reset and startup sequence
- SysTick timer configuration
- NVIC interrupt control
- Memory layout and system initialization (SystemInit)

### STM32 Peripheral Concepts
- RCC clock gating and system clock structure
- GPIO configuration using CMSIS
- Hardware timers (TIMx)
- UART transmit/receive
- SPI master mode
- I2C communication basics
- ADC single-shot conversion
- DMA transfers
- PWM signal generation
- External interrupts (EXTI)

### Firmware Engineering Concepts
- Makefile-based build systems
- Linker scripts and memory mapping
- Startup assembly and vector tables
- SWD programming and debugging
- Flashing via st-flash
- Project modularity and version control practices

---

## Building and Flashing

Each project folder contains its own Makefile.  
Typical build and flash process:

make
make flash

These use the GNU ARM toolchain and ST-LINK programmer.

---

## Long-Term Objective

The long-term purpose of this repository is to gain confidence working directly with ARM Cortex-M microcontrollers at the register level.  
By avoiding HAL and CubeMX, the focus is placed on understanding how the hardware operates internally and how firmware interacts with it at its most fundamental level.

This repository will continue expanding with new drivers, experiments, and low-level explorations as learning progresses.

---

## Notes

- This project is intended for educational purposes.
- Only CMSIS device headers are used for register definitions.
- No HAL or vendor middleware is included.
