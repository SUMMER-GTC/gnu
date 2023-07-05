#include "bsp_timer.h"
//定时器2初始化设置，1MS基准定时
void TIM2_InitConfiguration(void)
{	
/* Time base configuration */  
     NVIC_InitTypeDef NVIC_InitStructure;
 	RCC_APB1PeriphClockCmd(  RCC_APB1Periph_TIM2  ,ENABLE); 
     TIM_InternalClockConfig(TIM2); //TIM2使用内部时钟；
//TIM_SetAutoreload(TIM2,99);  //设置自动重装载寄存器，定时1ms；
  TIM_SetAutoreload(TIM2,198);  //设置自动重装载寄存器，定时1ms；

 
// TIM_SetAutoreload(TIM2,19);  //设置自动重装载寄存器，定时1ms；
     TIM_CounterModeConfig(TIM2,TIM_CounterMode_Up); //TIM2向上计数,即从0到TIM2_ARR；
     TIM_PrescalerConfig(TIM2,719,TIM_PSCReloadMode_Immediate);  //f[TIM2]=f[CLK_APB1]/(99+1)，即100分频，并立即产生更新；
     TIM_ClearITPendingBit(TIM2,TIM_IT_Update); //此次更新不产生中断；
     TIM_Cmd(TIM2,ENABLE);  //开启计数器；
     TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);  //开启TIM2的更新中断；
     
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
      //开TIM2全局中断；
     NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; //选择TIM2通道；
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级；  
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //响应优先级；
     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
     NVIC_Init(&NVIC_InitStructure);

}


static u8  fac_us=0;							//us延时倍乘数			   
static u16 fac_ms=0;							//ms延时倍乘数,在ucos下,代表每个节拍的ms数
	

//初始化延迟函数
//当使用OS的时候,此函数会初始化OS的时钟节拍
//SYSTICK的时钟固定为HCLK时钟的1/8
//SYSCLK:系统时钟
void delay_init(void )
{

	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//选择外部时钟  HCLK/8
	fac_us=SystemCoreClock/8000000;	//为系统时钟的1/8  
	fac_ms=(u16)fac_us*1000;//非ucos下,代表每个ms需要的systick时钟数   

}	



//延时nus
//nus为要延时的us数.		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 				//时间加载	  		 
	SysTick->VAL=0x00;        				//清空计数器
	SysTick->CTRL=0x01 ;      				//开始倒数 	 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达   
	SysTick->CTRL=0x00;      	 			//关闭计数器
	SysTick->VAL =0X00;       				//清空计数器	 
}
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对72M条件下,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;			//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;           			//清空计数器
	SysTick->CTRL=0x01 ;          			//开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达   
	SysTick->CTRL=0x00;      	 			//关闭计数器
	SysTick->VAL =0X00;       				//清空计数器	  	    
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




