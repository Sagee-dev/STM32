#include "stm32f4xx.h"

#define SCLK 16000000U
#define BAUD 9600U

   /*--------------------------------------------------
    * TIM2 TRGO triggers ADC1 conversion
    * ADC end of conversion triggers ADC interrupt
    * Main loop prints ADC value using USART2
    *
    * LED on PA5 toggles on each ADC interrupt
    * ADC input on PA0 (ADC1 channel 0)
    * USART2 on PA2 TX and PA3 RX (AF7)
    *-------------------------------------------------*/

volatile uint32_t adc_value = 0;
volatile uint32_t adc_ready_flag = 0;

   /*--------------------------------------------------
    * Send one character over USART2 (blocking)
    *-------------------------------------------------*/
static void usart2_send_char(char c)
{
    while (!((USART2->SR >> 7U) & 1U))      /* USART_SR_TXE bit 7 */
    {
        /* wait */
    }
    USART2->DR = (uint16_t)c;
}

   /*--------------------------------------------------
    * Send a string over USART2
    *-------------------------------------------------*/
static void usart2_send_string(const char *s)
{
    while (*s)
    {
        usart2_send_char(*s++);
    }
}

   /*--------------------------------------------------
    * Send unsigned integer as decimal over USART2
    *-------------------------------------------------*/
static void usart2_send_u32(uint32_t v)
{
    char buf[11];
    int i = 0;

    if (v == 0U)
    {
        usart2_send_char('0');
        return;
    }

    while (v > 0U && i < 10)
    {
        buf[i++] = (char)('0' + (v % 10U));
        v /= 10U;
    }

    while (i > 0)
    {
        usart2_send_char(buf[--i]);
    }
}

   /*--------------------------------------------------
    * ADC interrupt handler
    * Reads ADC1->DR and sets adc_ready_flag
    *-------------------------------------------------*/
void ADC_IRQHandler(void)
{
    /* check EOC flag */
    if ((ADC1->SR >> 1U) & 1U)             /* ADC_SR_EOC bit 1 */
    {
        adc_value = (uint32_t)ADC1->DR;    /* reading DR clears EOC */
        adc_ready_flag = 1U;

        /* toggle LED PA5 */
        GPIOA->ODR ^= (1U << 5U);          /* GPIO_ODR_OD5 */
    }
}

int main(void)
{
   /*--------------------------------------------------
    * 1) Enable GPIOA clock
    *-------------------------------------------------*/
    RCC->AHB1ENR |= (1U << 0U);            /* RCC_AHB1ENR_GPIOAEN */

   /*--------------------------------------------------
    * 2) Configure PA5 as output (LED)
    * MODER5 = 01, OT5 = 0, OSPEED5 = 11, PUPD5 = 00
    *-------------------------------------------------*/
    GPIOA->MODER &= ~(3U << (5U * 2U));    /* ~GPIO_MODER_MODER5 */
    GPIOA->MODER |=  (1U << (5U * 2U));    /* GPIO_MODER_MODER5_0 */

    GPIOA->OTYPER &= ~(1U << 5U);          /* ~GPIO_OTYPER_OT_5 */

    GPIOA->OSPEEDR &= ~(3U << (5U * 2U));  /* clear speed bits */
    GPIOA->OSPEEDR |=  (3U << (5U * 2U));  /* OSPEED5 = 11 */

    GPIOA->PUPDR &= ~(3U << (5U * 2U));    /* ~GPIO_PUPDR_PUPDR5 */

   /*--------------------------------------------------
    * 3) Configure USART2 pins PA2 and PA3
    * MODER2 = 10, MODER3 = 10, AFRL2 = AF7, AFRL3 = AF7
    *-------------------------------------------------*/
    GPIOA->MODER &= ~(3U << (2U * 2U));    /* ~GPIO_MODER_MODER2 */
    GPIOA->MODER |=  (2U << (2U * 2U));    /* GPIO_MODER_MODER2_1 */

    GPIOA->MODER &= ~(3U << (3U * 2U));    /* ~GPIO_MODER_MODER3 */
    GPIOA->MODER |=  (2U << (3U * 2U));    /* GPIO_MODER_MODER3_1 */

    GPIOA->AFR[0] &= ~((0xFU << (2U * 4U)) | (0xFU << (3U * 4U))); /* clear AFRL */
    GPIOA->AFR[0] |=  ((7U  << (2U * 4U)) | (7U  << (3U * 4U)));   /* AF7 USART2 */

   /*--------------------------------------------------
    * 4) Enable USART2 clock and configure USART2
    * APB1ENR USART2EN bit 17
    * CR1 TE bit 3, RE bit 2, UE bit 13
    *-------------------------------------------------*/
    RCC->APB1ENR |= (1U << 17U);           /* RCC_APB1ENR_USART2EN */

    USART2->BRR = (SCLK + (BAUD / 2U)) / BAUD;

    USART2->CR1 |= (1U << 3U);             /* USART_CR1_TE */
    USART2->CR1 |= (1U << 2U);             /* USART_CR1_RE */
    USART2->CR1 |= (1U << 13U);            /* USART_CR1_UE */

    usart2_send_string("TIM2 TRGO ADC IRQ\r\n");

   /*--------------------------------------------------
    * 5) Configure PA0 as analog input (ADC1 channel 0)
    * MODER0 = 11, PUPD0 = 00
    *-------------------------------------------------*/
    GPIOA->MODER |= (3U << (0U * 2U));     /* GPIO_MODER_MODER0 */
    GPIOA->PUPDR &= ~(3U << (0U * 2U));    /* ~GPIO_PUPDR_PUPDR0 */

   /*--------------------------------------------------
    * 6) Configure TIM2 for periodic update and TRGO
    * APB1ENR TIM2EN bit 0
    * PSC = 16000-1 -> 1 kHz
    * ARR = 500-1   -> 500 ms
    * CR2 MMS bits [6:4] = 010 -> TRGO on update event
    *-------------------------------------------------*/
    RCC->APB1ENR |= (1U << 0U);            /* RCC_APB1ENR_TIM2EN */

    TIM2->PSC = 16000U - 1U;
    TIM2->ARR = 500U - 1U;

    TIM2->CR2 &= ~(7U << 4U);              /* clear TIM_CR2_MMS */
    TIM2->CR2 |=  (2U << 4U);              /* TIM_CR2_MMS = 010 (update as TRGO) */

    TIM2->EGR = (1U << 0U);                /* TIM_EGR_UG */

   /*--------------------------------------------------
    * 7) Configure ADC1 external trigger and interrupt
    * APB2ENR ADC1EN bit 8
    *
    * ADC common prescaler ADCPRE bits [17:16] set to 01 (PCLK2/4)
    * SMPR2 SMP0 bits [2:0] sampling time
    * SQR1 L bits [23:20] = 0 (1 conversion)
    * SQR3 SQ1 bits [4:0] = 0 (channel 0)
    *
    * CR1 EOCIE bit 5 enable end of conversion interrupt
    * CR2 EXTSEL bits [27:24] select trigger source
    * CR2 EXTEN  bits [29:28] select trigger edge (01 rising)
    * CR2 ADON bit 0 enable ADC
    *-------------------------------------------------*/
    RCC->APB2ENR |= (1U << 8U);            /* RCC_APB2ENR_ADC1EN */

    ADC->CCR &= ~(3U << 16U);              /* clear ADC_CCR_ADCPRE */
    ADC->CCR |=  (1U << 16U);              /* ADCPRE = 01 -> PCLK2/4 */

    ADC1->SMPR2 &= ~(7U << 0U);            /* clear SMP0 */
    ADC1->SMPR2 |=  (3U << 0U);            /* sampling time value */

    ADC1->SQR1 &= ~(0xFU << 20U);          /* L = 0 -> 1 conversion */
    ADC1->SQR3 &= ~(0x1FU << 0U);          /* SQ1 = 0 -> channel 0 */

    ADC1->CR2 &= ~(1U << 1U);              /* ~ADC_CR2_CONT */
    ADC1->CR2 &= ~(1U << 11U);             /* ~ADC_CR2_ALIGN */

    ADC1->CR1 |= (1U << 5U);               /* ADC_CR1_EOCIE */

    /* external trigger selection and edge */
    ADC1->CR2 &= ~(0xFU << 24U);           /* clear EXTSEL[27:24] */
    ADC1->CR2 |=  (6U   << 24U);           /* EXTSEL = 6 (TIM2 TRGO) */

    ADC1->CR2 &= ~(3U << 28U);             /* clear EXTEN[29:28] */
    ADC1->CR2 |=  (1U << 28U);             /* EXTEN = 01 rising edge */

    /* enable ADC interrupt in NVIC (ADC_IRQn = 18) */
    NVIC->ISER[0] |= (1U << 18U);          /* NVIC_ISER0 set bit 18 */

    /* enable ADC */
    ADC1->CR2 |= (1U << 0U);               /* ADC_CR2_ADON */

   /*--------------------------------------------------
    * 8) Start TIM2
    * ADC starts conversions automatically on TRGO
    *-------------------------------------------------*/
    TIM2->CR1 |= (1U << 0U);               /* TIM_CR1_CEN */

   /*--------------------------------------------------
    * 9) Main loop prints ADC value when ready
    *-------------------------------------------------*/
    while (1)
    {
        if (adc_ready_flag)
        {
            adc_ready_flag = 0U;

            usart2_send_string("ADC = ");
            usart2_send_u32(adc_value);
            usart2_send_string("\r\n");
        }

        /* optional low power */
        /* __WFI(); */
    }
}
