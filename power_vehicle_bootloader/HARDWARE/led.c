#include "led.h"

void LED_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);	 
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 		     
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 
 GPIO_SetBits(GPIOB, GPIO_Pin_9);						 

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	    		 
 GPIO_Init(GPIOE, &GPIO_InitStructure);	  				 
 GPIO_SetBits(GPIOE, GPIO_Pin_9); 						 
}
 
