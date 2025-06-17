/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
//FreeRTOS使用
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "globals.h" //全局变量头文件

#include "event_groups.h"
#include "timer3.h"        //包含需要的头文件
#include "bsp_esp8266.h"
#include "ring_buff.h"//环形缓冲区

extern TaskHandle_t WIFI_Task_Handle;
extern EventGroupHandle_t Event_Handle;
extern const int PING_MODE;
extern TaskHandle_t Receive_Task_Handle;

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)
//{
//}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
extern void xPortSysTickHandler(void);
//systick中断服务函数
void SysTick_Handler(void)
{	
    #if (INCLUDE_xTaskGetSchedulerState  == 1 )
      if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
      {
    #endif  /* INCLUDE_xTaskGetSchedulerState */  
        xPortSysTickHandler();
    #if (INCLUDE_xTaskGetSchedulerState  == 1 )
      }
    #endif  /* INCLUDE_xTaskGetSchedulerState */
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
void USART2_IRQHandler(void)
{
    uint32_t ulReturn;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    /* 进入临界段 */
    ulReturn = taskENTER_CRITICAL_FROM_ISR();
//	if(USART_GetITStatus(USART2, USART_IT_TC) != RESET)
//	{
//		USART_ClearITPendingBit(USART2, USART_IT_TC);         //清除中断标志	
//		DMA_ClearFlag(DMA1_FLAG_TC7);
//		DMA_SetCurrDataCounter(USART2_TX_DMA_CHANNEL,0);//重新写入需要传输数据的数量
//	}
    // 处理空闲中断
     if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        // 读取SR和DR寄存器以清除IDLE标志
        USART2->SR;
        USART2->DR;
        
        // 停止DMA传输
        DMA_Cmd(USART2_RX_DMA_CHANNEL, DISABLE);
        
        // 获取接收到的数据长度
        g_rx_dma_cnt = USART2_DMA_RX_BUFFER_SIZE - DMA_GetCurrDataCounter(USART2_RX_DMA_CHANNEL);
        
        if(g_rx_dma_cnt > 0)
        {
            if ((xEventGroupGetBitsFromISR(Event_Handle) & 0x01) == 0)
            {
                // 未连接服务器时的数据处理
                if(g_rx_dma_cnt < USART2_DMA_RX_BUFFER_SIZE)
                {
                    Filter_memcpy(g_rx_esp8266_buf, g_rx_dma_buf, g_rx_dma_cnt);
                    g_rx_esp8266_cnt = g_rx_dma_cnt;
                }
                else
                {
                  Filter_memcpy(g_rx_esp8266_buf, g_rx_dma_buf, USART2_DMA_RX_BUFFER_SIZE);
                  g_rx_esp8266_cnt = USART2_DMA_RX_BUFFER_SIZE;
                  // 可以添加一个标志位表示数据溢出
                  // uint8_t overflow_flag = 1;

                  // 可以通过LED或其他方式提示用户数据溢出
                  // LED_RED_ON();
                }
            }
            else
            {
                // 已连接服务器时的数据处理
                RingBuff_WriteNByte(&encoeanBuff, g_rx_dma_buf, g_rx_dma_cnt);
                
                // 重置定时器3计数器（ping包计时器）
                TIM_SetCounter(TIM3, 0);

                // 通知接收任务处理数据
                if(Receive_Task_Handle != NULL)
                {
                    vTaskNotifyGiveFromISR(Receive_Task_Handle, &xHigherPriorityTaskWoken);
                }
            }
        }
        
        // 重新设置DMA传输数量并启动DMA
        USART2_RX_DMA_CHANNEL->CNDTR = USART2_DMA_RX_BUFFER_SIZE;
        DMA_Cmd(USART2_RX_DMA_CHANNEL, ENABLE);
    }
    
    /* 退出临界段 */
    taskEXIT_CRITICAL_FROM_ISR(ulReturn);
    
    if(xHigherPriorityTaskWoken){
      // 如果需要进行任务切换，在中断退出时进行切换
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/*---------------------------------------------------------------*/
/*函数名：void TIM4_IRQHandler(void)				      			 */
/*功  能：定时器4中断处理函数									 */
/*		  1.处理串口2接收到的MQTT数据，将串口接收缓冲复制到MQTT接收*/
/*			缓冲        										 */
/*参  数：无                                       				 */
/*返回值：无                                     				 */
/*---------------------------------------------------------------*/
// 删除以下函数
/*void TIM4_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {   
        RingBuff_WriteNByte(&encoeanBuff,g_rx_esp8266_buf,g_rx_esp8266_cnt);
        g_rx_esp8266_cnt = 0;                                        
        TIM_SetCounter(TIM3, 0);                                     
        TIM_Cmd(TIM4, DISABLE);                                      
        TIM_SetCounter(TIM4, 0);                                     
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);                 
    }
}*/

/*---------------------------------------------------------------*/
/*函数名：void TIM3_IRQHandler(void)				      			 */
/*功  能：定时器3中断处理函数									 */
/*		  1.控制ping心跳包的发送									 */
/*参  数：无                                       				 */
/*返回值：无                                     				 */
/*其  他：多次快速发送（2s，5次）没有反应，wifi任务由挂起态->就绪态*/
/*---------------------------------------------------------------*/
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)//如果TIM_IT_Update置位，表示TIM3溢出中断，进入if	
	{  
		printf("pingFlag=%d\r\n",pingFlag);
		switch(pingFlag) 					//判断pingFlag的状态
		{                               
			case 0:							//如果pingFlag等于0，表示正常状态，发送Ping报文  
					ESP8266_CheckMQTTStatus(); 		//添加Ping报文到发送缓冲区  
					break;
			case 1:							//如果pingFlag等于1，说明上一次发送到的ping报文，没有收到服务器回复，所以1没有被清除为0，可能是连接异常，我们要启动快速ping模式
					TIM3_ENABLE_2S(); 	    //我们将定时器6设置为2s定时,快速发送Ping报文
					xEventGroupClearBitsFromISR(Event_Handle, PING_MODE);//关闭发送PING包的定时器3，设置事件标志位
					ESP8266_CheckMQTTStatus();			//添加Ping报文到发送缓冲区  
					break;
			case 2:							//如果pingFlag等于2，说明还没有收到服务器回复
			case 3:				            //如果pingFlag等于3，说明还没有收到服务器回复
			case 4:				            //如果pingFlag等于4，说明还没有收到服务器回复	
					ESP8266_CheckMQTTStatus();  		//添加Ping报文到发送缓冲区 
					break;
			case 5:							//如果pingFlag等于5，说明我们发送了多次ping，均无回复，应该是连接有问题，我们重启连接
					xTaskResumeFromISR(WIFI_Task_Handle);        //连接状态置0，表示断开，没连上服务器
					TIM_Cmd(TIM3, DISABLE); //关TIM3 				
					break;			
		}
		pingFlag++;           		   		//pingFlag自增1，表示又发送了一次ping，期待服务器的回复
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //清除TIM3溢出中断标志 	
	}
}
