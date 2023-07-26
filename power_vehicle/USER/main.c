#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"

#include "init.h"
#include "app_task_define.h"

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);


static void GetSystemClockConfig(void)
{
  RCC_ClocksTypeDef RCC_Clocks = { 0 };
  RCC_GetClocksFreq(&RCC_Clocks);
  if (RCC_Clocks.SYSCLK_Frequency == SystemCoreClock) {
    return;
  }
  // printf frequency information
}

int main(void)
{
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
  GetSystemClockConfig();
	delay_init(168);	    				//��ʱ������ʼ��	 
	uart_init(115200);					  //��ʼ������
  Init();

	
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���

		TaskCreate();

    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}


