#include "bsp_timer.h"
//��ʱ��2��ʼ�����ã�1MS��׼��ʱ
void TIM2_InitConfiguration(void)
{	
/* Time base configuration */  
     NVIC_InitTypeDef NVIC_InitStructure;
 	RCC_APB1PeriphClockCmd(  RCC_APB1Periph_TIM2  ,ENABLE); 
     TIM_InternalClockConfig(TIM2); //TIM2ʹ���ڲ�ʱ�ӣ�
//TIM_SetAutoreload(TIM2,99);  //�����Զ���װ�ؼĴ�������ʱ1ms��
  TIM_SetAutoreload(TIM2,198);  //�����Զ���װ�ؼĴ�������ʱ1ms��

 
// TIM_SetAutoreload(TIM2,19);  //�����Զ���װ�ؼĴ�������ʱ1ms��
     TIM_CounterModeConfig(TIM2,TIM_CounterMode_Up); //TIM2���ϼ���,����0��TIM2_ARR��
     TIM_PrescalerConfig(TIM2,719,TIM_PSCReloadMode_Immediate);  //f[TIM2]=f[CLK_APB1]/(99+1)����100��Ƶ���������������£�
     TIM_ClearITPendingBit(TIM2,TIM_IT_Update); //�˴θ��²������жϣ�
     TIM_Cmd(TIM2,ENABLE);  //������������
     TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);  //����TIM2�ĸ����жϣ�
     
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
      //��TIM2ȫ���жϣ�
     NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; //ѡ��TIM2ͨ����
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ���  
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //��Ӧ���ȼ���
     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
     NVIC_Init(&NVIC_InitStructure);

}


static u8  fac_us=0;							//us��ʱ������			   
static u16 fac_ms=0;							//ms��ʱ������,��ucos��,����ÿ�����ĵ�ms��
	

//��ʼ���ӳٺ���
//��ʹ��OS��ʱ��,�˺������ʼ��OS��ʱ�ӽ���
//SYSTICK��ʱ�ӹ̶�ΪHCLKʱ�ӵ�1/8
//SYSCLK:ϵͳʱ��
void delay_init(void )
{

	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//ѡ���ⲿʱ��  HCLK/8
	fac_us=SystemCoreClock/8000000;	//Ϊϵͳʱ�ӵ�1/8  
	fac_ms=(u16)fac_us*1000;//��ucos��,����ÿ��ms��Ҫ��systickʱ����   

}	



//��ʱnus
//nusΪҪ��ʱ��us��.		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 				//ʱ�����	  		 
	SysTick->VAL=0x00;        				//��ռ�����
	SysTick->CTRL=0x01 ;      				//��ʼ���� 	 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//�ȴ�ʱ�䵽��   
	SysTick->CTRL=0x00;      	 			//�رռ�����
	SysTick->VAL =0X00;       				//��ռ�����	 
}
//��ʱnms
//ע��nms�ķ�Χ
//SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK��λΪHz,nms��λΪms
//��72M������,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;			//ʱ�����(SysTick->LOADΪ24bit)
	SysTick->VAL =0x00;           			//��ռ�����
	SysTick->CTRL=0x01 ;          			//��ʼ����  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//�ȴ�ʱ�䵽��   
	SysTick->CTRL=0x00;      	 			//�رռ�����
	SysTick->VAL =0X00;       				//��ռ�����	  	    
} 

uint16_t delay_time=0;
uint16_t set_delay_time=0;

void hct_time_handler(void)
{
	 delay_time++;
   if(delay_time>set_delay_time)
	 {
	    delay_time=set_delay_time+1;
	 }
}

void hct_set_delay_time(uint16_t time)
{
  set_delay_time=time;
  delay_time=0;
}

uint8_t hct_delay_time_ok(void)
{
	if(delay_time>=set_delay_time)
	{
	 return 1;
	}
	else
	return 0;

}




