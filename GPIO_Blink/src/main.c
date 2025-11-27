#include "stm32f4xx.h"

volatile uint32_t ms = 0;

// SysTick interrupt handler: runs every 1 ms
void SysTick_Handler(void)
{
    ms++;
}

int main(void)
{
    /*--------------------------------------------------
     * 1) Enable GPIOA clock (RCC AHB1ENR)
     *-------------------------------------------------*/
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   // GPIOAEN bit = 1 → enable GPIOA

    /*--------------------------------------------------
     * 2) Configure PA5 as push-pull output
     *    - MODER5 = 01 (output)
     *    - OTYPER5 = 0 (push-pull)
     *    - OSPEEDR5 = 11 (high speed, optional)
     *    - PUPDR5 = 00 (no pull-up/pull-down)
     *-------------------------------------------------*/

    // MODER: clear both bits for pin 5, then set to 01 (output)
    GPIOA->MODER &= ~GPIO_MODER_MODER5;    // clear mode bits for PA5
    GPIOA->MODER |=  GPIO_MODER_MODER5_0;  // 01: general-purpose output

    // OTYPER: 0 = push-pull
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT_5;    // clear bit 5 → push-pull

    // OSPEEDR: set high speed for PA5 (optional)
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5;  // 11: very high speed

    // PUPDR: no pull-up, no pull-down → 00
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR5;

    /*--------------------------------------------------
     * 3) Configure SysTick for 1 ms tick
     *    Assuming SystemCoreClock = 16 MHz (HSI)
     *-------------------------------------------------*/

    SysTick->LOAD = 16000U - 1U;   // 16000 cycles @ 16 MHz = 1 ms
    SysTick->VAL  = 0U;            // clear current value

    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |  // use processor clock
                    SysTick_CTRL_TICKINT_Msk   |  // enable SysTick interrupt
                    SysTick_CTRL_ENABLE_Msk;      // enable SysTick

    /*--------------------------------------------------
     * 4) Main loop: toggle LED every 100 ms
     *-------------------------------------------------*/
    uint32_t last = 0;

    while (1)
    {
        if ((ms - last) >= 100U)   // 100 ms elapsed?
        {
            GPIOA->ODR ^= GPIO_ODR_OD5;   // toggle PA5
            last += 100U;
        }
    }
}

