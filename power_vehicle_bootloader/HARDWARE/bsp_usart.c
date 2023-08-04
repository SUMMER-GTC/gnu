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

#define USART_USE_DMA_SEND (0)

#if USART_USE_DMA_SEND
static u8 g_usartDmaSendBuff[32] = { 0 };
#endif

 /**
  * @brief  
  * @param  
  * @retval 
  */
static void USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//ʹ��USART1ʱ��

		//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9, GPIO_AF_USART1); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10, GPIO_AF_USART1); //GPIOA10����ΪUSART1
	
	//USART1�˿�����
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA9��PA10

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = 115200;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//��������ж�
  USART_Cmd(USART1, ENABLE);  //ʹ�ܴ���1 
}

/// ����USART1�����ж�
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

static void UsartDMAConfig(void)
{
#if USART_USE_DMA_SEND
  DMA_InitTypeDef DMA_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

  DMA_DeInit(DMA2_Stream7);
  while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE) {
  }

  DMA_InitStructure.DMA_Channel = DMA_Channel_4;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = ((u32)&USART1->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)g_usartDmaSendBuff;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;	
  DMA_InitStructure.DMA_BufferSize = sizeof(g_usartDmaSendBuff);
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;      
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;        
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;    
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;    
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;      
  DMA_Init(DMA2_Stream7, &DMA_InitStructure);
  
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
#else
	return;
#endif
}

void UsartInit(void)
{
	USART_Config();
	NVIC_Configuration();
  UsartDMAConfig();
}

void UsartSend(u8 *data, u16 dataLen)
{
#if USART_USE_DMA_SEND
	if (dataLen > sizeof(g_usartDmaSendBuff)) {
		dataLen = sizeof(g_usartDmaSendBuff);
	}
	memcpy(g_usartDmaSendBuff, data, dataLen);
	DMA_SetCurrDataCounter(DMA2_Stream7, dataLen);
  DMA_Cmd(DMA2_Stream7, ENABLE);
	while(DMA_GetFlagStatus(DMA2_Stream7, DMA_FLAG_TCIF7) == RESET) {
	}
	DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7);
#else
	u8 *pData = data;
	
	for (u16 i = 0; i < dataLen; i++) {
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);    
		USART_SendData(USART1, pData[i]);
	}
#endif
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
			/* ����һ���ֽ����ݵ�USART1 */
			USART_SendData(USART1, *(u16 *)ptr++);
			
			/* �ȴ�������� */
			while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		
    }
    return len;
}

/*********************************************END OF FILE**********************/
