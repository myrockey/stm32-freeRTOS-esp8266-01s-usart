#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h" //信号量
#include "queue.h" //队列
#include "event_groups.h" //事件标志组

/* 开发版硬件bsp头文件 */
#include "globals.h" //全局变量头文件
//#include "ring_buff.h"//环形缓冲区
#include "timer3.h"
//#include "timer4.h"

#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_key.h"
#include "bsp_esp8266.h"
#include "bsp_temperature.h"

/* 任务句柄 */
/* 任务句柄是1个指针，用于指向1个任务，当任务创建好之后，它就具有了1个任务句柄
以后如果我们想要操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么这个句柄
可以为NULL */

/* 创建任务句柄 */
TaskHandle_t AppTaskCreate_Handle = NULL;
//LED任务 
TaskHandle_t LED_Task_Handle;

TaskHandle_t WIFI_Task_Handle = NULL;
/* 发送命令任务句柄 */
TaskHandle_t Send_Task_Handle = NULL;
/* 热敏传感器任务句柄 */
TaskHandle_t Temperature_Task_Handle = NULL;

TaskHandle_t Receive_Task_Handle = NULL;

/* 内核对象句柄 */
/* 信号量、消息队列、事件标志组、软件定时器这些都属于内核的对象，要想使用这些内核对象
必须先创建，创建成功之后会返回1个相应的句柄。实际上是1个指针，后续我们就可以通过这个
句柄操作这些内核对象

内核对象说白了就是一种全局的数据结构，通过这些数据结构我们可以实现任务间的通信，任务间
的事件同步等各种功能。至于这些功能的实现我们是通过调用这些内核对象的函数来完成的 */

/*	二值信号量句柄                         
 *	作用：用于控制MQTT命令缓冲处理任务，在MQTT数据接收发送缓冲处理任务中发出
 *		  当命令缓冲区收到命令数据时，发出信号量		 
 */
//SemaphoreHandle_t BinarySemaphore;
/*
事件标志组
作用：标志wifi连接，ping心跳包发送模式控制wifi是否重连，是否发送数据，传感器是否运行。
具体：
1.事件标志组位1为0，位0为1，即0x01(0000 0001),wifi连接至服务器时位0置1，此时connect报文未发送。
2.事件标志组位1为1，位0为1，即0x03(0000 0011),connect报文发送，返回连接成功报文时，位1置1
ping心跳包开启30秒发送模式，传感器任务开启，数据开始上传，设备远程控制LED功能开启。
*/
EventGroupHandle_t Event_Handle = NULL;//事件标志组(位0：wifi连接状态，位1：ping心跳包2s快速发送模式)
const int WIFI_CONNECT = (0x01 << 0);//设置事件掩码的位0，服务器连接模式，1-表示已连接，0表示未连接
const int PING_MODE = (0x01 << 1);//设置事件掩码的位1，ping心跳包发送模式，1表示开启30s发送模式，0表示未开启发送或开启2s快速发送模式。
						 
/*传感器发送消息队列*/
// 定义队列用于任务间通信
QueueHandle_t Message_Queue = NULL;//消息队列句柄
const UBaseType_t MESSAGE_DATA_TX_NUM = 5;//消息队列最大消息数
const UBaseType_t MESSAGE_DATA_TX_LEN = 100;//每条消息大小，单位字节


/* 全局变量声明 */
/* 当我们再写应用程序的时候，可能需要用到一些全局变量。 */

/****** 宏定义 ******/
/* 当我们在写应用程序的时候，可能会用到一些宏定义 */


/* 
***********************************************************
						函数声明 
***********************************************************
*/
void AppTaskCreate(void);/* 用于创建任务 */

void LED_Task(void *pvParameters);

void WIFI_Task(void * pvParameters);/* WIFI_Task任务实现 */

void Send_Task(void * pvParameters);/* Send_Task任务实现 */

void Temperature_Task(void *pvParameters);/* Temperature_Task热敏传感器任务实现 */

void Receive_Task(void * pvParameters);/* Receive_Task任务实现 */

void BSP_Init(void); /* 用于初始化板子相关资源 */

/**
*
*@brief 主函数
*@param 无
*@retval 无
*@note 第一步：开发板硬件初始化
*	   第2步：创建App应用任务
*	   第3步：启东FreeRTOS，开启多任务调度
*/
int main(void)
{
	BaseType_t xReturn = pdPASS;/* 定义1个创建信息返回值，默认为pdPASS */

	/* 开发板硬件初始化 */
	BSP_Init();

	printf("这是1个STM32F103C8T6开发板-FreeCTOS-智能环境监测项目实验！\r\n");
	printf("打开WiFi，自动连接WiFi，按下KEY1发送温度数据到ThingsCloud物联网平台\r\n");
	
	/* 创建 AppTaskCreate 任务 */
	xReturn = xTaskCreate((TaskFunction_t)AppTaskCreate,/* 任务函数 */
	(const char*)"AppTaskCreate",/* 任务名称 */
	(uint16_t)512,/* 任务堆栈大小 */
	(void*)NULL,/* 传递给任务函数的参数 */
	(UBaseType_t)1,/* 任务优先级 */
	(TaskHandle_t*)&AppTaskCreate_Handle);/* 任务控制块指针 */

	if(pdPASS == xReturn)/* 创建成功 */
	{
		vTaskStartScheduler();/* 启动任务，开启调度 */
	}
	else
	{
		printf("Task creation failed!\r\n");
		return -1;
	}

	while(1);/* 正常不会执行到这里 */
}


/*
@函数名：AppTaskCreate
@功能说明：为了方便管理，所有的任务创建函数都放在这个函数里面
@参数：无
@返回值：无
*/
void AppTaskCreate(void)
{
	BaseType_t xReturn = pdPASS;/* 定义1个创建信息返回值，默认为pdPASS */
	taskENTER_CRITICAL();//进入临界区
	
	//创建二值信号量
	//BinarySemaphore = xSemaphoreCreateBinary();
	//事件标志组，用于wifi连接状态及ping发送状态
	Event_Handle = xEventGroupCreate();
	if (Event_Handle == NULL) {
        // 处理创建失败的情况
        printf("Failed to create event group.\n");
    }
	// 创建传感器消息体消息队列
    Message_Queue = xQueueCreate(MESSAGE_DATA_TX_NUM, sizeof(float));
	if(NULL != Message_Queue)
	{
		printf("创建Message_Queue消息队列成功！\r\n");
	}
	else
	{
		printf("创建Message_Queue消息队列失败！\r\n");
	}

	/* 创建LED_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t)LED_Task,//任务函数
	(const char*)"LED_Task",//任务名称
	(uint16_t)STACK_SIZE,//任务堆栈大小
	(void*)NULL,//传递给任务函数的参数
	(UBaseType_t)3,//任务优先级
	(TaskHandle_t*)&LED_Task_Handle);//任务控制块指针

	if(pdPASS == xReturn)
	{
		printf("LED_Task任务创建成功！\r\n");
	}
	else
	{
		printf("LED_Task任务创建失败！\r\n");
	}

	/* 创建WIFI_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t)WIFI_Task,//任务函数
	(const char*)"WIFI_Task",//任务名称
	(uint16_t)STACK_SIZE,//任务堆栈大小
	(void*)NULL,//传递给任务函数的参数
	(UBaseType_t)2,//任务优先级
	(TaskHandle_t*)&WIFI_Task_Handle);//任务控制块指针

	if(pdPASS == xReturn)
	{
		printf("WIFI_Task任务创建成功！\r\n");
	}
	else
	{
		printf("WIFI_Task任务创建失败！\r\n");
	}

	/* 创建KEY_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t)Send_Task,//任务函数
	(const char*)"Send_Task",//任务名称
	512,//任务堆栈大小
	(void*)NULL,//传递给任务函数的参数
	(UBaseType_t)3,//任务优先级
	(TaskHandle_t*)&Send_Task_Handle);//任务控制块指针

	if(pdPASS == xReturn)
	{
		printf("Send_Task任务创建成功！\r\n");
	}
	else
	{
		printf("Send_Task任务创建失败！\r\n");
	}

	/* 创建Temperature_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t)Temperature_Task,//任务函数
	(const char*)"Temperature_Task",//任务名称
	(uint16_t)STACK_SIZE,//任务堆栈大小
	(void*)NULL,//传递给任务函数的参数
	(UBaseType_t)4,//任务优先级
	(TaskHandle_t*)&Temperature_Task_Handle);//任务控制块指针

	if(pdPASS == xReturn)
	{
		printf("Temperature_Task任务创建成功！\r\n");
	}
	else
	{
		printf("Temperature_Task任务创建失败！\r\n");
	}
	
	
	/* 创建Receive_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t)Receive_Task,//任务函数
	(const char*)"Receive_Task",//任务名称
	512,//任务堆栈大小
	(void*)NULL,//传递给任务函数的参数
	(UBaseType_t)5,//任务优先级
	(TaskHandle_t*)&Receive_Task_Handle);//任务控制块指针

	if(pdPASS == xReturn)
	{
		printf("Receive_Task任务创建成功！\r\n");
	}
	else
	{
		printf("Receive_Task任务创建失败！\r\n");
	}

	vTaskDelete(AppTaskCreate_Handle);//删除AppTaskCreate任务

	taskEXIT_CRITICAL();//推出临界区

}

/*---------------------------------------------------------------*/
/*函数名：void LED_Task(void *pvParameters)                  */
/*功  能：LED任务（配置）									     */
/*		  1.LED2任务执行       							         */
/*参  数：无                          			   				 */
/*返回值：无                                       			     */
/*其  他：服务器连接以及ping心跳包30S发送模式事件发生时执行此任务，*/
/*		  否则挂起任务   									     */
/*---------------------------------------------------------------*/
void LED_Task(void *pvParameters)
{
	while(1)
	{
		//服务器连接以及ping心跳包30S发送模式事件发生时执行此任务，否则挂起任务
		xEventGroupWaitBits((EventGroupHandle_t	)Event_Handle,		
							(EventBits_t		)PING_MODE,
							(BaseType_t			)pdFALSE,				
							(BaseType_t			)pdTRUE,
							(TickType_t			)portMAX_DELAY);
		LED2_ON;
		Delay_ms(500);	//延时500ms
		LED2_OFF;
		Delay_ms(500);	//延时500ms
	}
}

/*
@函数名：WIFI_Task
@功能说明：WIFI_Task任务主体
@参数：
@返回值：无
*/
void WIFI_Task(void * pvParameters)
{
	while(1)
	{
		printf("需要连接服务器\r\n");                 
		//TIM_Cmd(TIM4, DISABLE);                       //关闭TIM4 
		TIM_Cmd(TIM3, DISABLE);                       //关闭TIM3
		xEventGroupClearBits(Event_Handle, PING_MODE);//关闭发送PING包的定时器3，清除事件标志位
		ESP8266_Buf_Clear();//清空接收缓存区
		if(ESP8266_WiFi_MQTT_Connect_IoTServer() == 0)			  //如果WiFi连接云服务器函数返回0，表示正确，进入if
		{   			     
			printf("WIFI及MQTT服务器连接并订阅成功\r\n");            
			ESP8266_Buf_Clear();//清空接收缓存区
			//MQTT_Buff_Init();                         //初始化发送缓冲区
			
			xEventGroupSetBits(Event_Handle, WIFI_CONNECT);  //服务器已连接，抛出事件标志 
			
			//启动定时器30s模式
			TIM3_ENABLE_30S();
			pingFlag = 0;
			xEventGroupSetBits(Event_Handle, PING_MODE); //30s的PING定时器，设置事件标志位
			
			vTaskSuspend(NULL);	    						//服务器已连接，挂起自己，进入挂起态（任务由挂起转为就绪态时在这继续执行下去）
			xEventGroupClearBits(Event_Handle, WIFI_CONNECT);//服务器或者wifi已断开，清除事件标志，继续执行本任务，重新连接 
			xEventGroupClearBits(Event_Handle, PING_MODE);  //关闭发送PING包的定时器3，清除事件标志位
		}
		Delay_ms(20);	    //延时10
	}
}

/*
@函数名：Send_Task
@功能说明：Send_Task任务主体
@参数：
@返回值：无
*/
void Send_Task(void *pvParameters)
{
	BaseType_t xReturn = pdTRUE;/* 定义1个创建信息返回值，默认为pdTRUE */
	float r_queue;/* 定义1个接收消息的变量 */
	char message[128] = {0};
	//服务器连接以及ping心跳包30S发送模式事件发生时执行此任务，否则挂起任务
	xEventGroupWaitBits((EventGroupHandle_t	)Event_Handle,		
						(EventBits_t		)PING_MODE,
						(BaseType_t			)pdFALSE,				
						(BaseType_t			)pdTRUE,
						(TickType_t			)portMAX_DELAY);
		
    while (1)
    {
		//xSemaphoreTake(BinarySemaphore, portMAX_DELAY);	//获取信号量，获取到信号量，继续执行，否则进入阻塞态，等待执行
		
		xReturn = xQueueReceive(Message_Queue,/* 消息队列的句柄 */
		&r_queue,/* 接收消息的内容 */
		portMAX_DELAY);/* 等待时间*/
		if(pdTRUE == xReturn)
		{
			snprintf(message,sizeof(message),"{\\\"temperature\\\": %.2f}",r_queue);	
			taskENTER_CRITICAL(); //进入临界区，防止中断打断
			ESP8266_MQTT_Publish(message);//添加数据，发布给服务器
			taskEXIT_CRITICAL();  //退出临界区
			printf("send Data to ThingsCloud:本次接收到的数据是%.2f\r\n",r_queue);		
		}
		else
		{
			printf("数据接收出错，错误代码0x%lx\r\n",xReturn);
		}
        vTaskDelay(pdMS_TO_TICKS(20)); 
    }
}



/*
@函数名：Temperature_Task
@功能说明：Temperature_Task任务主体
@参数：
@返回值：无
*/
void Temperature_Task(void *pvParameters)
{
    float temperature;	   //定义一个变量，保存温度值	
		
    while (1)
    {
		//服务器连接以及PING心跳包30S发送模式事件发生时执行此任务，否则挂起任务
		xEventGroupWaitBits((EventGroupHandle_t	)Event_Handle,		
							(EventBits_t		)PING_MODE,
							(BaseType_t			)pdFALSE,				
							(BaseType_t			)pdTRUE,
							(TickType_t			)portMAX_DELAY);
	
		
		if(Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON)
		{
			/* K1 被按下 */
			temperature = Thermistor_Read_Temperature();
			// 将温度数据发送到队列
			if (xQueueSend(Message_Queue, &temperature, 0) != pdPASS)
			{
				printf("Data Queue Full, Data Dropped\r\n");
			}
			printf("getTemperature: %.2f C\r\n", temperature);
		}
		
        vTaskDelay(pdMS_TO_TICKS(20)); // 每2秒读取一次数据
    }
}

/*
@函数名：Receive_Task
@功能说明：Receive_Task任务主体
@参数：
@返回值：无
*/
void Receive_Task(void * pvParameters)
{
	float temperature;
	uint16_t len  = 0;
	uint8_t flg = 0;
	uint8_t received_str[USART2_DMA_RX_BUFFER_SIZE];
	//Packet_TypeDef packet;//数据包
	while(1)
	{
		//服务器连接事件发生执行此任务，否则挂起
		xEventGroupWaitBits((EventGroupHandle_t	)Event_Handle,		
							(EventBits_t		)WIFI_CONNECT,
							(BaseType_t			)pdFALSE,				
							(BaseType_t			)pdTRUE,
							(TickType_t			)portMAX_DELAY);
		
		//printf("KEY_Task Running\r\n");
		// 等待IDLE中断的通知
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);//等待通知

		flg = GetAFra(received_str,&len);//读取接收的数据（由于读取 和 写入是 异步的，读写速度可能不一致，所以不能放中断中处理）
		if(flg)
		{
		//len = RingBuff_GetLen(&encoeanBuff);
        //if (len) {
			//RingBuff_ReadNByte(&encoeanBuff,received_str,len);
            received_str[len] = '\0';
            // 输出接收到的字符串
            printf("Received: %s\n", received_str);

            // ping状态，mqtt连接成功
			//+MQTTCONN:0,6,1,"gz-3-mqtt.iot-api.com","1883","",1\r\n\r\nOK
            if (strstr((const char*)received_str, "+MQTTCONN:0,6") != NULL && strstr((const char*)received_str, "OK") != NULL) {
                printf("PING报文回复\r\n");                       
				if(pingFlag == 1)
				{                   						     //如果pingFlag=1，表示第一次发送
					pingFlag = 0;    				       		 //要清除pingFlag标志
				}
				else if(pingFlag > 1)	
				{ 				 								 //如果pingFlag>1，表示是多次发送了，而且是2s间隔的快速发送
					pingFlag = 0;     				      		 //要清除pingFlag标志
					TIM3_ENABLE_30S(); 				      		 //PING定时器重回30s的时间
					xEventGroupSetBits(Event_Handle, PING_MODE); //30s的PING定时器，设置事件标志位
				}
            }
			
			// 获取远程命令
			if(strstr((const char*)received_str, "getValue") != NULL && strstr((const char*)received_str, "state") != NULL){
				printf("服务器等级0推送\r\n"); 		   	 //串口输出信息 
				temperature = Thermistor_Read_Temperature();
				// 将温度数据发送到队列
				if (xQueueSend(Message_Queue, &temperature, 0) != pdPASS)
				{
					printf("Data Queue Full, Data Dropped\r\n");
				}
				printf("getValue getTemperature: %.2f C\r\n", temperature);
				//xSemaphoreGive(BinarySemaphore);	     //给出二值信号量，控制MQTT命令缓冲处理任务执行
			}
			
        }
		vTaskDelay(20);//延时20个tick
	}
}

/* 所有板子上的初始化均可放在这个函数里 */
void BSP_Init(void)
{
	/* 中断优先级分组为4，即4bit都用来表示抢占优先级别，范围为：0-15
	优先级分组只需要分组1次即可，以后如果有其他的任务都需要用到中断，都统一用这个优先级分组，
	切忌，千万不要再分组 */
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	//RingBuff_Init(&encoeanBuff);//环形缓冲区初始化
	
	//Delay_init();//延时函数初始化
	//Tim4_Init(500,7200);//TIM4初始化，定时时间500*7200*1000/7200000 = 50ms
	LED_GPIO_Config();
	
	
	/* 串口初始化	*/
	USART_Config();
	
	/* 按键初始化 */
	Key_GPIO_Config();
	
	/*初始化 */
	USART2_Init();
	ESP8266_Reset_IO_Init();
	
	/*初始化热敏传感器*/
	Thermistor_Init();
}
