#include "delay.h"
#include "sys.h"
static uint32_t g_fac_us = 0;      

#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"
#include "task.h"

extern void xPortSysTickHandler(void);

/**
 * @brief     
 * @param     
 * @retval    
 */  
void SysTick_Handler(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) 
    {
        xPortSysTickHandler();
    }
}
#endif

/**
 * @brief     
 * @param     
 * @retval    
 */  
void delay_init(uint16_t sysclk)
{
#if SYSTEM_SUPPORT_OS                                      
    uint32_t reload;
#endif
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    g_fac_us = sysclk;                                  
#if SYSTEM_SUPPORT_OS                                      
    reload = sysclk;                                    
    reload *= 1000000 / configTICK_RATE_HZ;             
                                                           
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;          
    SysTick->LOAD = reload;                   
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;           
#endif 
}
 
#if SYSTEM_SUPPORT_OS                                      

/**
 * @brief     
 * @param     
 * @note      
 * @retval    
 */ 
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;        
    
    ticks = nus * g_fac_us;                 
    told = SysTick->VAL;                    
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;        
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks) 
            {
                break;                      
            }
        }
    }
} 

/**
 * @brief    
 * @param     
 * @retval    
 */
void delay_ms(uint16_t nms)
{
    uint32_t i;

    for (i=0; i<nms; i++)
    {
        delay_us(1000);
    }
}

#else  

/**
 * @brief      
 * @param       
 * @note        
 * @retval      
 */
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;        
    ticks = nus * g_fac_us;                
    told = SysTick->VAL;                    
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;        
            }
            else 
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;                      
            }
        }
    }
}

/**
 * @brief       
 * @param       
 * @retval      
 */
void delay_ms(uint16_t nms)
{
    uint32_t repeat = nms / 30;     
    uint32_t remain = nms % 30;

    while (repeat)
    {
        delay_us(30 * 1000);        
        repeat--;
    }

    if (remain)
    {
        delay_us(remain * 1000);   
    }
}

/**
  * @brief 
           
  * @param 
  * @retval None
  */
void HAL_Delay(uint32_t Delay)
{
     delay_ms(Delay);
}
#endif
			 



































