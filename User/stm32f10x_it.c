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
extern  EventGroupHandle_t Event_Handle;
extern const int PING_MODE;


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
//extern volatile uint8_t esp8266_buf[256];
//extern volatile uint16_t esp8266_cnt;

//void USART2_IRQHandler(void)
//{
//  uint32_t ulReturn;
//  /* 进入临界段，临界段可以嵌套 */
//  ulReturn = taskENTER_CRITICAL_FROM_ISR();

//	if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET) {
//    uint8_t data = USART_ReceiveData(USART2);
//    esp8266_buf[esp8266_cnt++] = data;
//    if (esp8266_cnt >= sizeof(esp8266_buf)) {
//        esp8266_cnt = 0;
//    }
//    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
//  }
//	
//  /* 退出临界段 */
//  taskEXIT_CRITICAL_FROM_ISR( ulReturn );
//}

void USART2_IRQHandler(void)
{
    uint32_t ulReturn;
    /* 进入临界段 */
    ulReturn = taskENTER_CRITICAL_FROM_ISR();

    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        uint8_t data = USART_ReceiveData(USART2);
        
		if ((xEventGroupGetBitsFromISR(Event_Handle) & 0x01) == 0)//获取事件标志组数据，等于0说明未连接服务器，不开启定时器4（MQTT接收数据处理）定时器
		{
//			if(USART2->DR)                                        //处于指令配置状态时，非零值才保存到缓冲区	
//			{                                     			 
//				Usart2_RxBuff[Usart2_RxCounter] = USART2->DR;	  //保存到缓冲区	
//				Usart2_RxCounter++; 						      //每接收1个字节的数据，Usart2_RxCounter加1，表示接收的数据总量+1 
//			}
			//必须过滤空字符，否则会导致 strstr((const char*)g_rx_esp8266_buf, "ready") != NULL 匹配不到
			/*如果 g_rx_esp8266_buf 中包含空字符（'\0'），strstr 在遇到第一个空字符时就会停止搜索，从而无法找到后续的 "ready" 字符串。
例如，如果缓冲区内容为 "abc\0ready"，strstr 只会搜索到 "abc" 部分，而不会继续搜索到 "ready"*/
			if(data != '\0')
			{
				g_rx_esp8266_buf[g_rx_esp8266_cnt++] = data;
			}
		}
		else
		{
//			Usart2_RxBuff[Usart2_RxCounter] = USART2->DR;//把接收到的数据保存到Usart2_RxBuff中
//				
//			if(Usart2_RxCounter == 0)				     //如果Usart2_RxCounter等于0，表示是接收的第1个数据，进入if分支	
//			{    								
//				TIM_Cmd(TIM4, ENABLE); 					 //使能定时器4
//			}
//			else										 //else分支，表示果Usart2_RxCounter不等于0，不是接收的第一个数据
//			{                        									    
//				TIM_SetCounter(TIM4, 0);  				 //置位定时器4
//			}	
//			Usart2_RxCounter++;         				 //每接收1个字节的数据，Usart2_RxCounter加1，表示接收的数据总量+1 				
			if(g_rx_esp8266_cnt == 0)
			{
				TIM_Cmd(TIM4,ENABLE);//使能定时器
			}
			else 
			{
				TIM_SetCounter(TIM4, 0);//置为定时器
			}
			g_rx_esp8266_buf[g_rx_esp8266_cnt++] = data;
		}
				
		if (g_rx_esp8266_cnt >= RX_BUFFER_SIZE) {
			g_rx_esp8266_cnt = 0;
			//printf("接收数据溢出\r\n");
		}
		
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }

    /* 退出临界段 */
    taskEXIT_CRITICAL_FROM_ISR(ulReturn);
}

/*---------------------------------------------------------------*/
/*函数名：void TIM4_IRQHandler(void)				      			 */
/*功  能：定时器4中断处理函数									 */
/*		  1.处理串口2接收到的MQTT数据，将串口接收缓冲复制到MQTT接收*/
/*			缓冲        										 */
/*参  数：无                                       				 */
/*返回值：无                                     				 */
/*---------------------------------------------------------------*/
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)//如果TIM_IT_Update置位，表示TIM4溢出中断，进入if	
	{   
		RingBuff_WriteNByte(&encoeanBuff,g_rx_esp8266_buf,g_rx_esp8266_cnt);
		g_rx_esp8266_cnt = 0;                                        	//串口2接收数据量变量清零
		TIM_SetCounter(TIM3, 0);                                     	//清零定时器3计数器，重新计时ping包发送时间
		TIM_Cmd(TIM4, DISABLE);                        				 	//关闭TIM4定时器
		TIM_SetCounter(TIM4, 0);                        			 	//清零定时器4计数器
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);     			 	//清除TIM4溢出中断标志 	
	}
}

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
