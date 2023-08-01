/**
  ******************************************************************************
  * @file    bsp_usart.c
  * @author  
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  */
  
#include "bsp_usart.h"
#include "bsp_comm_protocol.h"

 /**
  * @brief  
  * @param  
  * @retval 
  */
static void USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//使能USART1时钟

		//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9, GPIO_AF_USART1); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10, GPIO_AF_USART1); //GPIOA10复用为USART1
	
	//USART1端口配置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = 115200;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断
  USART_Cmd(USART1, ENABLE);  //使能串口1 
}

/// 配置USART1接收中断
static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	/* Configure the NVIC Preemption Priority Bits */  
	
	/* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void UsartInit(void)
{
	USART_Config();
	NVIC_Configuration();
}

void UsartSend(u8 *data, u16 dataLen)
{
	u8 *pData = data;
	
	for (u16 i = 0; i < dataLen; i++) {
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);    
		USART_SendData(USART1, pData[i]);
	}
}

void USART1_IRQHandler(void)
{
	u8 ch;
	
 	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
			ch = USART_ReceiveData(USART1);
			CommOtaIrqHandler(ch);
	} 
	 
}

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

__attribute__((weak)) int _isatty(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 1;
 
    errno = EBADF;
    return 0;
}
 
__attribute__((weak)) int _close(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 0;
 
    errno = EBADF;
    return -1;
}
 
__attribute__((weak)) int _lseek(int fd, int ptr, int dir)
{
    (void)fd;
    (void)ptr;
    (void)dir;
 
    errno = EBADF;
    return -1;
}
 
__attribute__((weak)) int _fstat(int fd, struct stat *st)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    {
        st->st_mode = S_IFCHR;
        return 0;
    }
 
    errno = EBADF;
    return 0;
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
    (void)file;
    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {

    }
    return len;
}
 
__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    (void)file;
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
			/* 发送一个字节数据到USART1 */
			USART_SendData(USART1, *(u16 *)ptr++);
			
			/* 等待发送完毕 */
			while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		
    }
    return len;
}

/*********************************************END OF FILE**********************/
