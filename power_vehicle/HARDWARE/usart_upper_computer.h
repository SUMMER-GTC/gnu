#ifndef __USART_UPPER_COMPUTER_H__
#define	__USART_UPPER_COMPUTER_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "stm32f4xx.h"
#include <stdio.h>

#define COMM_RCC_UART				RCC_APB1Periph_USART2
#define COMM_RCC_UART_GPIO	RCC_APB2Periph_GPIOA
#define COMM_UART_GPIO			GPIOA
#define COMM_UART_TX_GPIO		GPIO_Pin_2
#define COMM_UART_RX_GPIO		GPIO_Pin_3
#define COMM_UART						USART2
#define COMM_UART_IRQ				USART2_IRQn


#ifdef __cplusplus
}
#endif

#endif 
