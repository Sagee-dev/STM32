STM32 Bare-Metal Firmware Development
Learning Journey on the STM32 NUCLEO-F401RE (No HAL, No CubeMX)

This repository documents my complete learning journey into embedded firmware development using the STM32 NUCLEO-F401RE microcontroller board.
The focus of this project is to understand the STM32 platform at a low level using pure C, CMSIS headers, and direct register programming.

The purpose of this repo is educational: to develop a deep, practical understanding of ARM Cortex-M microcontrollers without relying on HAL, CubeMX, or vendor abstraction layers.

Goals of This Repository

Build firmware projects fully from scratch.

Understand how STM32 peripherals work at the register level.

Learn the ARM Cortex-M architecture, SysTick, NVIC, and startup process.

Develop drivers for common peripherals without vendor libraries.

Practice clean project structure, Makefiles, linker scripts, and startup code.

Document each step as a series of self-contained learning tasks.

Hardware Used

Development Board: STM32 NUCLEO-F401RE

Microcontroller: STM32F401RE (ARM Cortex-M4, 84 MHz, ARMv7E-M architecture)

Project Approach

This repository follows a structured learning workflow:

Each firmware task is implemented in its own project folder or branch.

Every task builds from scratch without copying HAL-generated code.

Only CMSIS headers are used to access hardware registers.

Startup assembly, vector table, linker script, and system initialization are manually included.

All code is written to be readable, minimal, and reusable.

Repository Structure

/
├── GPIO_Blink/            Basic GPIO output and LED toggle
|
└── README.md                   Main documentation


(This structure is flexible; we can update it as your learning progresses.)

What This Repository Covers
Core Cortex-M Concepts

Vector tables

Stack initialization

Reset and exception handlers

SysTick timer

NVIC configuration

CMSIS core registers

STM32 Peripherals

RCC and clock configuration

GPIO input/output

Timers (TIM2, TIM3, etc.)

UART

SPI

I2C

ADC

DMA

EXTI interrupts

PWM generation

Power modes (later in the series)

Firmware Engineering Essentials

Linker scripts (.ld)

Startup assembly (.s)

Makefiles

Flasher integration (st-flash, openocd)

Directory organization

Version control with Git

Documentation discipline

How to Build/Flash

Build using GCC and Make:

make


Flash via ST-LINK:

make flash


(Each task folder includes its own Makefile and build instructions.)

Long-Term Objective

By the end of this journey, the goal is to be able to:

Write complete firmware drivers without relying on HAL

Understand every register that configures the hardware

Design and debug low-level embedded software confidently

Build real-world embedded projects

Showcase strong embedded fundamentals to recruiters

Contact

If you are interested in embedded systems or want to collaborate, feel free to reach out or connect on LinkedIn.
