#include "stm32f4xx.h"

#define SCLK 16000000U
#define BAUD 9600U

   /*--------------------------------------------------
    * This code explains the usage of USART2 with STM32.
    * TX on PA2 and RX on PA3 (AF7).
    * Echo back every received character.
    *-------------------------------------------------*/

   /*--------------------------------------------------
    * Send one character over USART2 (blocking)
    * waits until TXE (transmit data register empty)
    *-------------------------------------------------*/
static void usart2_send_char(char c)
{
    while (!(USART2->SR & USART_SR_TXE))
    {
        /* wait for empty buffer */
    }
    USART2->DR = (uint16_t)c;
}

   /*--------------------------------------------------
    * Receive one character from USART2 (blocking)
    * waits until RXNE (receive data register not empty)
    *-------------------------------------------------*/
static char usart2_receive_char(void)
{
    while (!(USART2->SR & USART_SR_RXNE))
    {
        /* wait for data */
    }
    return (char)(USART2->DR & 0xFF);
}

   /*--------------------------------------------------
    * Send a null-terminated string over USART2
    *-------------------------------------------------*/
static void usart2_send_string(const char *s)
{
    while (*s)
    {
        usart2_send_char(*s++);
    }
}

int main(void)
{
   /*--------------------------------------------------
    * 1) enable GPIOA clock
    *-------------------------------------------------*/
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

   /*--------------------------------------------------
    * 2) configure PA2 and PA3 as alternate function mode
    *    MODER = 10 for AF
    *-------------------------------------------------*/
    GPIOA->MODER &= ~(3U << (2U * 2U));
    GPIOA->MODER |=  (2U << (2U * 2U));

    GPIOA->MODER &= ~(3U << (3U * 2U));
    GPIOA->MODER |=  (2U << (3U * 2U));

   /*--------------------------------------------------
    * 3) select AF7 for PA2 and PA3 (USART2)
    *    AFRL uses 4 bits per pin
    *-------------------------------------------------*/
    GPIOA->AFR[0] &= ~((0xFU << (2U * 4U)) | (0xFU << (3U * 4U)));
    GPIOA->AFR[0] |=  ((7U  << (2U * 4U)) | (7U  << (3U * 4U)));

   /*--------------------------------------------------
    * 4) enable USART2 clock
    *-------------------------------------------------*/
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

   /*--------------------------------------------------
    * 5) configure baud rate
    *    BRR = Fclk / baud  (oversampling by 16)
    *-------------------------------------------------*/
    USART2->BRR = (SCLK + (BAUD / 2U)) / BAUD;

   /*--------------------------------------------------
    * 6) enable transmitter and receiver, then enable USART
    *-------------------------------------------------*/
    USART2->CR1 |= USART_CR1_TE;
    USART2->CR1 |= USART_CR1_RE;
    USART2->CR1 |= USART_CR1_UE;

   /*--------------------------------------------------
    * 7) send a startup message
    *-------------------------------------------------*/
    usart2_send_string("USART2 ready (9600 8N1)\r\n");

   /*--------------------------------------------------
    * 8) echo loop
    *-------------------------------------------------*/
    while (1)
    {
        char c = usart2_receive_char();
        usart2_send_char(c);
    }
}

