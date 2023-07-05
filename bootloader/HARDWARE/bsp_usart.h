#ifndef __BSP_USART_H__
#define	__BSP_USART_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "stm32f10x.h"
#include <stdio.h>

#define COMM_RCC_UART				RCC_APB1Periph_USART2
#define COMM_RCC_UART_GPIO	RCC_APB2Periph_GPIOA
#define COMM_UART_GPIO			GPIOA
#define COMM_UART_TX_GPIO		GPIO_Pin_2
#define COMM_UART_RX_GPIO		GPIO_Pin_3
#define COMM_UART						USART2
#define COMM_UART_IRQ				USART2_IRQn

void UsartInit(void);
void UsartSend(u8 *data, u16 dataLen);

int fputc(int ch, FILE *f);
int fgetc(FILE *f);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_USART_H__ */
