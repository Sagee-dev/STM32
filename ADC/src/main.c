#include "stm32f4xx.h"

#define SCLK 16000000U
#define BAUD 9600U

   /*--------------------------------------------------
    * ADC1 single conversion example on STM32F401
    * ADC input  : PA0 (channel 0)
    * LED output : PA5 (onboard LED)
    * UART       : USART2 PA2 TX, PA3 RX
    * LED turns ON during ADC sampling
    * ADC value is printed over UART
    *-------------------------------------------------*/

   /*--------------------------------------------------
    * Send one character over USART2 (blocking)
    *-------------------------------------------------*/
static void usart2_send_char(char c)
{
    while (!(USART2->SR & USART_SR_TXE))
    {
        /* wait for TXE */
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

int main(void)
{
   /*--------------------------------------------------
    * 1) Enable GPIOA and configure PA5 as output
    *-------------------------------------------------*/

    RCC->AHB1ENR |= (1U << 0U);            /* RCC_AHB1ENR_GPIOAEN */

    GPIOA->MODER &= ~(3U << (5U * 2U));    /* ~GPIO_MODER_MODER5 */
    GPIOA->MODER |=  (1U << (5U * 2U));    /* GPIO_MODER_MODER5_0 */

    GPIOA->OTYPER &= ~(1U << 5U);          /* ~GPIO_OTYPER_OT_5 */

    GPIOA->OSPEEDR &= ~(3U << (5U * 2U));  /* clear speed bits */
    GPIOA->OSPEEDR |=  (3U << (5U * 2U));  /* GPIO_OSPEEDER_OSPEEDR5 (11) */

    GPIOA->PUPDR &= ~(3U << (5U * 2U));    /* ~GPIO_PUPDR_PUPDR5 */

   /*--------------------------------------------------
    * 2) Configure USART2 pins PA2 and PA3
    *-------------------------------------------------*/

    GPIOA->MODER &= ~(3U << (2U * 2U));    /* ~GPIO_MODER_MODER2 */
    GPIOA->MODER |=  (2U << (2U * 2U));    /* GPIO_MODER_MODER2_1 (AF mode) */

    GPIOA->MODER &= ~(3U << (3U * 2U));    /* ~GPIO_MODER_MODER3 */
    GPIOA->MODER |=  (2U << (3U * 2U));    /* GPIO_MODER_MODER3_1 (AF mode) */

    GPIOA->AFR[0] &= ~((0xFU << (2U * 4U)) | (0xFU << (3U * 4U))); /* clear AFRL PA2,PA3 */
    GPIOA->AFR[0] |=  ((7U  << (2U * 4U)) | (7U  << (3U * 4U)));   /* AF7 USART2 */

    RCC->APB1ENR |= (1U << 17U);           /* RCC_APB1ENR_USART2EN */

    USART2->BRR = (SCLK + (BAUD / 2U)) / BAUD;  /* BRR = Fclk/baud */

    USART2->CR1 |= (1U << 3U);             /* USART_CR1_TE */
    USART2->CR1 |= (1U << 2U);             /* USART_CR1_RE */
    USART2->CR1 |= (1U << 13U);            /* USART_CR1_UE */

    usart2_send_string("ADC1 PA0 demo\r\n");

   /*--------------------------------------------------
    * 3) Configure PA0 as analog input for ADC1 ch0
    *-------------------------------------------------*/

    GPIOA->MODER |= (3U << (0U * 2U));     /* GPIO_MODER_MODER0 (11 analog) */
    GPIOA->PUPDR &= ~(3U << (0U * 2U));    /* ~GPIO_PUPDR_PUPDR0 */

   /*--------------------------------------------------
    * 4) Enable and configure ADC1
    *-------------------------------------------------*/

    RCC->APB2ENR |= (1U << 8U);            /* RCC_APB2ENR_ADC1EN */

    ADC->CCR &= ~(3U << 16U);              /* clear ADCPRE */
    ADC->CCR |=  (1U << 16U);              /* ADC_CCR_ADCPRE_0 (PCLK2/4) */

    ADC1->SMPR2 &= ~(7U << 0U);            /* clear SMP0 */
    ADC1->SMPR2 |=  (3U << 0U);            /* sampling time for ch0 */

    ADC1->SQR1 &= ~(0xFU << 20U);          /* L[3:0] = 0 -> 1 conversion */
    ADC1->SQR3 &= ~(0x1FU << 0U);          /* SQ1 = 0 -> channel 0 */

    ADC1->CR2 &= ~(1U << 1U);              /* ~ADC_CR2_CONT (single) */
    ADC1->CR2 &= ~(1U << 11U);             /* ~ADC_CR2_ALIGN (right) */

    ADC1->CR2 |= (1U << 0U);               /* ADC_CR2_ADON enable ADC */

   /*--------------------------------------------------
    * 5) Main loop
    *-------------------------------------------------*/
    while (1)
    {
        uint32_t adc;

        GPIOA->ODR |= (1U << 5U);          /* GPIO_ODR_OD5 LED ON */

        ADC1->CR2 |= (1U << 30U);          /* ADC_CR2_SWSTART start conversion */

        while (!(ADC1->SR & (1U << 1U)))   /* ADC_SR_EOC */
        {
            /* wait */
        }

        adc = (uint32_t)ADC1->DR;          /* ADC result */

        GPIOA->ODR &= ~(1U << 5U);         /* LED OFF */

        usart2_send_string("ADC = ");
        usart2_send_u32(adc);
        usart2_send_string("\r\n");

        for (volatile uint32_t i = 0; i < 200000U; i++)
        {
            /* delay */
        }
    }
}

