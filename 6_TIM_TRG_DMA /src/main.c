#include "stm32f4xx.h"

#define SCLK 16000000U
#define BAUD 9600U

#define ADC_BUF_LEN 64U

volatile uint16_t adc_buf[ADC_BUF_LEN];
volatile uint32_t dma_half_flag = 0;
volatile uint32_t dma_full_flag = 0;

   /*--------------------------------------------------
    * USART2 send helpers (blocking)
    *-------------------------------------------------*/
static void usart2_send_char(char c)
{
    while (!((USART2->SR >> 7U) & 1U))          /* USART_SR_TXE bit 7 */
    {
        /* wait */
    }
    USART2->DR = (uint16_t)c;
}

static void usart2_send_string(const char *s)
{
    while (*s)
    {
        usart2_send_char(*s++);
    }
}

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
    * DMA2 Stream0 interrupt handler
    * Sets half or full buffer flags
    *-------------------------------------------------*/
void DMA2_Stream0_IRQHandler(void)
{
    /* DMA2 low interrupt status register flags for Stream0 */
    /* HTIF0 bit 4, TCIF0 bit 5 in DMA2->LISR */
    if ((DMA2->LISR >> 4U) & 1U)                /* DMA_LISR_HTIF0 */
    {
        DMA2->LIFCR = (1U << 4U);               /* DMA_LIFCR_CHTIF0 clear */
        dma_half_flag = 1U;
    }

    if ((DMA2->LISR >> 5U) & 1U)                /* DMA_LISR_TCIF0 */
    {
        DMA2->LIFCR = (1U << 5U);               /* DMA_LIFCR_CTCIF0 clear */
        dma_full_flag = 1U;
    }

    /* optional clear TEIF0, DMEIF0, FEIF0 if needed */
    if ((DMA2->LISR >> 3U) & 1U)                /* DMA_LISR_TEIF0 */
    {
        DMA2->LIFCR = (1U << 3U);               /* DMA_LIFCR_CTEIF0 */
    }
    if ((DMA2->LISR >> 2U) & 1U)                /* DMA_LISR_DMEIF0 */
    {
        DMA2->LIFCR = (1U << 2U);               /* DMA_LIFCR_CDMEIF0 */
    }
    if ((DMA2->LISR >> 0U) & 1U)                /* DMA_LISR_FEIF0 */
    {
        DMA2->LIFCR = (1U << 0U);               /* DMA_LIFCR_CFEIF0 */
    }
}

int main(void)
{
   /*--------------------------------------------------
    * 1) Enable GPIOA clock
    *-------------------------------------------------*/
    RCC->AHB1ENR |= (1U << 0U);                 /* RCC_AHB1ENR_GPIOAEN */

   /*--------------------------------------------------
    * 2) Optional LED PA5 output (debug)
    *-------------------------------------------------*/
    GPIOA->MODER &= ~(3U << (5U * 2U));         /* ~GPIO_MODER_MODER5 */
    GPIOA->MODER |=  (1U << (5U * 2U));         /* GPIO_MODER_MODER5_0 */

    GPIOA->OTYPER &= ~(1U << 5U);               /* ~GPIO_OTYPER_OT_5 */
    GPIOA->PUPDR  &= ~(3U << (5U * 2U));        /* ~GPIO_PUPDR_PUPDR5 */

   /*--------------------------------------------------
    * 3) Configure USART2 pins PA2 TX and PA3 RX
    * MODER2 = 10, MODER3 = 10, AF7 for both
    *-------------------------------------------------*/
    GPIOA->MODER &= ~(3U << (2U * 2U));         /* clear PA2 */
    GPIOA->MODER |=  (2U << (2U * 2U));         /* PA2 AF mode */

    GPIOA->MODER &= ~(3U << (3U * 2U));         /* clear PA3 */
    GPIOA->MODER |=  (2U << (3U * 2U));         /* PA3 AF mode */

    GPIOA->AFR[0] &= ~((0xFU << (2U * 4U)) | (0xFU << (3U * 4U)));
    GPIOA->AFR[0] |=  ((7U  << (2U * 4U)) | (7U  << (3U * 4U)));   /* AF7 USART2 */

    RCC->APB1ENR |= (1U << 17U);                /* RCC_APB1ENR_USART2EN */

    USART2->BRR = (SCLK + (BAUD / 2U)) / BAUD;  /* BRR = Fclk/baud */

    USART2->CR1 |= (1U << 3U);                  /* USART_CR1_TE */
    USART2->CR1 |= (1U << 2U);                  /* USART_CR1_RE */
    USART2->CR1 |= (1U << 13U);                 /* USART_CR1_UE */

    usart2_send_string("Day6 ADC DMA circular\r\n");

   /*--------------------------------------------------
    * 4) Configure PA0 as analog input (ADC1 channel 0)
    *-------------------------------------------------*/
    GPIOA->MODER |= (3U << (0U * 2U));          /* GPIO_MODER_MODER0 (analog) */
    GPIOA->PUPDR &= ~(3U << (0U * 2U));         /* ~GPIO_PUPDR_PUPDR0 */

   /*--------------------------------------------------
    * 5) Enable DMA2 clock
    *-------------------------------------------------*/
    RCC->AHB1ENR |= (1U << 22U);                /* RCC_AHB1ENR_DMA2EN */

   /*--------------------------------------------------
    * 6) Configure TIM2 for periodic update and TRGO
    * PSC = 16000-1 -> 1 kHz
    * ARR = 10-1    -> 10 ms sample period (100 Hz)
    * MMS = 010 update event as TRGO
    *-------------------------------------------------*/
    RCC->APB1ENR |= (1U << 0U);                 /* RCC_APB1ENR_TIM2EN */

    TIM2->PSC = 16000U - 1U;
    TIM2->ARR = 10U - 1U;

    TIM2->CR2 &= ~(7U << 4U);                   /* clear TIM_CR2_MMS */
    TIM2->CR2 |=  (2U << 4U);                   /* MMS = 010 update as TRGO */

    TIM2->EGR = (1U << 0U);                     /* TIM_EGR_UG */

   /*--------------------------------------------------
    * 7) Configure ADC1
    * external trigger TIM2 TRGO rising edge
    * enable DMA in ADC
    *-------------------------------------------------*/
    RCC->APB2ENR |= (1U << 8U);                 /* RCC_APB2ENR_ADC1EN */

    ADC->CCR &= ~(3U << 16U);                   /* clear ADCPRE */
    ADC->CCR |=  (1U << 16U);                   /* ADCPRE = 01 (PCLK2/4) */

    ADC1->SMPR2 &= ~(7U << 0U);                 /* SMP0 */
    ADC1->SMPR2 |=  (3U << 0U);                 /* sampling time */

    ADC1->SQR1 &= ~(0xFU << 20U);               /* L = 0 (1 conversion) */
    ADC1->SQR3 &= ~(0x1FU << 0U);               /* SQ1 = 0 (channel 0) */

    ADC1->CR2 &= ~(1U << 1U);                   /* ~CONT single */
    ADC1->CR2 &= ~(1U << 11U);                  /* ~ALIGN right */

    /* disable ADC EOC interrupt for DMA design */
    ADC1->CR1 &= ~(1U << 5U);                   /* ~EOCIE */

    /* EXTSEL bits [27:24] = 6 (TIM2 TRGO), EXTEN bits [29:28] = 01 rising */
    ADC1->CR2 &= ~(0xFU << 24U);                /* clear EXTSEL */
    ADC1->CR2 |=  (6U   << 24U);                /* EXTSEL = 6 TIM2 TRGO */

    ADC1->CR2 &= ~(3U << 28U);                  /* clear EXTEN */
    ADC1->CR2 |=  (1U << 28U);                  /* EXTEN = 01 rising */

    /* enable DMA in ADC, and continuous DMA requests */
    ADC1->CR2 |= (1U << 8U);                    /* ADC_CR2_DMA */
    ADC1->CR2 |= (1U << 9U);                    /* ADC_CR2_DDS */

   /*--------------------------------------------------
    * 8) Configure DMA2 Stream0 for ADC1->DR to adc_buf
    * CHSEL = 0 (channel 0)
    * DIR = 00 (peripheral to memory)
    * CIRC = 1
    * MINC = 1
    * PSIZE = 01 (16-bit)
    * MSIZE = 01 (16-bit)
    * HTIE = 1, TCIE = 1
    *-------------------------------------------------*/

    /* disable stream before config */
    DMA2_Stream0->CR &= ~(1U << 0U);            /* DMA_SxCR_EN */
    while ((DMA2_Stream0->CR >> 0U) & 1U)
    {
        /* wait until disabled */
    }

    /* clear pending flags for stream0 */
    DMA2->LIFCR = (1U << 0U) | (1U << 2U) | (1U << 3U) | (1U << 4U) | (1U << 5U);

    DMA2_Stream0->PAR  = (uint32_t)&ADC1->DR;   /* peripheral address */
    DMA2_Stream0->M0AR = (uint32_t)adc_buf;     /* memory address */
    DMA2_Stream0->NDTR = (uint32_t)ADC_BUF_LEN; /* number of transfers */

    DMA2_Stream0->CR = 0;

    DMA2_Stream0->CR &= ~(7U << 25U);           /* CHSEL */
    DMA2_Stream0->CR |=  (0U << 25U);           /* channel 0 */

    DMA2_Stream0->CR &= ~(3U << 6U);            /* DIR */
    DMA2_Stream0->CR |=  (0U << 6U);            /* peripheral to memory */

    DMA2_Stream0->CR |= (1U << 8U);             /* CIRC */
    DMA2_Stream0->CR |= (1U << 10U);            /* MINC */

    DMA2_Stream0->CR &= ~(3U << 11U);           /* PSIZE */
    DMA2_Stream0->CR |=  (1U << 11U);           /* PSIZE = 01 16-bit */

    DMA2_Stream0->CR &= ~(3U << 13U);           /* MSIZE */
    DMA2_Stream0->CR |=  (1U << 13U);           /* MSIZE = 01 16-bit */

    DMA2_Stream0->CR |= (1U << 3U);             /* HTIE */
    DMA2_Stream0->CR |= (1U << 4U);             /* TCIE */

    /* enable DMA2 Stream0 IRQ in NVIC
       DMA2_Stream0_IRQn is usually 56 on STM32F4
       56 -> ISER[1] bit 24 (because 32..63 in ISER[1])
    */
    NVIC->ISER[1] |= (1U << 24U);

    /* enable stream */
    DMA2_Stream0->CR |= (1U << 0U);             /* EN */

   /*--------------------------------------------------
    * 9) Enable ADC and start TIM2
    *-------------------------------------------------*/
    ADC1->CR2 |= (1U << 0U);                    /* ADC_CR2_ADON */

    TIM2->CR1 |= (1U << 0U);                    /* TIM_CR1_CEN */

   /*--------------------------------------------------
    * 10) Main loop
    * Print average of half buffer or full buffer
    *-------------------------------------------------*/
    while (1)
    {
        if (dma_half_flag)
        {
            uint32_t sum = 0;
            uint32_t i;

            dma_half_flag = 0;

            for (i = 0; i < (ADC_BUF_LEN / 2U); i++)
            {
                sum += (uint32_t)adc_buf[i];
            }

            usart2_send_string("AVG0 = ");
            usart2_send_u32(sum / (ADC_BUF_LEN / 2U));
            usart2_send_string("\r\n");
        }

        if (dma_full_flag)
        {
            uint32_t sum = 0;
            uint32_t i;

            dma_full_flag = 0;

            for (i = (ADC_BUF_LEN / 2U); i < ADC_BUF_LEN; i++)
            {
                sum += (uint32_t)adc_buf[i];
            }

            usart2_send_string("AVG1 = ");
            usart2_send_u32(sum / (ADC_BUF_LEN / 2U));
            usart2_send_string("\r\n");
        }

        /* optional low power */
        /* __WFI(); */
    }
}

