#include "stm32f4xx.h"

   /*--------------------------------------------------
    * This code explains the usage of timer interrupts
    * with STM32.
    * Code flashes LED with 500 ms period
    * based on timer interrupts, not SysTick.
    *-------------------------------------------------*/
     
   /*--------------------------------------------------
    * Timer 2 interrupt handler function.
    * Function name must exactly match the vector table.
    * Check the status register for update flag (UIF),
    * clear it, then toggle the LED pin (PA5).
    *-------------------------------------------------*/
void TIM2_IRQHandler(void)
{
    /* Check update interrupt flag */
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF;        /* clear flag */
        GPIOA->ODR ^= GPIO_ODR_OD5;     /* toggle LED on PA5 */
    }
}

int main(void)
{
   /*--------------------------------------------------
    * 1) Enable GPIOA clock (RCC AHB1ENR)
    *-------------------------------------------------*/
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
     
   /*--------------------------------------------------
    * 2) Configure PA5 as push-pull output
    *    - MODER5 = 01 (output)
    *    - OTYPER5 = 0 (push-pull)
    *    - OSPEEDR5 = 11 (high speed, optional)
    *    - PUPDR5 = 00 (no pull-up / pull-down)
    *-------------------------------------------------*/
     
    /* MODER: clear pin 5 and set it to output (01) */
    GPIOA->MODER &= ~GPIO_MODER_MODER5;
    GPIOA->MODER |=  GPIO_MODER_MODER5_0;
     
    /* OTYPER: push-pull (bit = 0) */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT_5;
     
    /* OSPEEDR: set pin 5 output to high speed (11) */
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5;
     
    /* PUPDR: no pull-up, no pull-down on PA5 */
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR5;
     
   /*--------------------------------------------------
    * 3) Configure Timer 2 interrupt
    *
    *    Timer clock assumed 16 MHz (HSI, no PLL).
    *    We want 500 ms period:
    *      - Make timer tick at 1 kHz (1 ms per tick)
    *      - PSC = 16000 - 1  -> 16 MHz / 16000 = 1000 Hz
    *      - ARR = 500  - 1  -> 500 ms between updates
    *
    *      - Enable TIM2 clock on APB1
    *      - Set PSC and ARR
    *      - Generate update event to load registers
    *      - Clear any pending UIF flag
    *      - Enable update interrupt (UIE)
    *      - Enable TIM2 interrupt in NVIC
    *      - Start timer
    *-------------------------------------------------*/
     
    /* Enable TIM2 clock (on APB1) */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
     
    /* Prescaler: 16 MHz / 16000 = 1 kHz timer tick */
    TIM2->PSC = 16000U - 1U;

    /* Auto-reload: 500 ms period */
    TIM2->ARR = 500U - 1U;

    /* Generate an update event to load PSC and ARR */
    TIM2->EGR = TIM_EGR_UG;

    /* Clear update flag just in case */
    TIM2->SR &= ~TIM_SR_UIF;

    /* Enable update interrupt (UIE) */
    TIM2->DIER |= TIM_DIER_UIE;

    /* Enable TIM2 interrupt line in NVIC */
    NVIC_EnableIRQ(TIM2_IRQn);

    /* Start the counter */
    TIM2->CR1 |= TIM_CR1_CEN;
    
    /* Main loop: all the work happens in the interrupt */
    while (1)
    {
        /* optional: low power wait for interrupt */
        /* __WFI(); */
    }
}
