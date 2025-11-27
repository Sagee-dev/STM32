#include "stm32f4xx.h"

uint32_t SystemCoreClock = 16000000U;

void SystemInit(void) {
    // Enable FPU
    SCB->CPACR |= (0xF << 20);
    // Vector table at flash base
    SCB->VTOR = FLASH_BASE;
}

void SystemCoreClockUpdate(void) {
    SystemCoreClock = 16000000U;
}

