#include "bsp_esp8266.h"

uint8_t g_rx_dma_buf[USART2_DMA_RX_BUFFER_SIZE] = {0};//DMA接收数据缓冲区
volatile uint32_t g_rx_dma_cnt = 0;// 当前接收的字节数

// DMA配置函数
void USART2_DMA_Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    // 开启DMA1时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    // 配置DMA发送通道
    DMA_DeInit(USART2_TX_DMA_CHANNEL);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = 0;  // 后续发送时设置
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0;      // 后续发送时设置
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(USART2_TX_DMA_CHANNEL, &DMA_InitStructure);
	
    // 配置DMA接收通道
    DMA_DeInit(USART2_RX_DMA_CHANNEL);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_rx_dma_buf; // 设置DMA接收内存地址
    DMA_InitStructure.DMA_BufferSize = USART2_DMA_RX_BUFFER_SIZE; // 设置DMA接收缓冲区大小
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  // 普通模式
    DMA_Init(USART2_RX_DMA_CHANNEL, &DMA_InitStructure);
    
    // 使能USART2的DMA发送和接收请求
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);

	DMA_Cmd(USART2_RX_DMA_CHANNEL, ENABLE); // 使能DMA接收通道
	    
    //DMA_Cmd(USART2_TX_DMA_CHANNEL, ENABLE);
}

// 使用DMA发送数据
void USART2_DMA_SendData(uint8_t *pData, uint16_t Size)
{   
	// 清除标志
    DMA_ClearFlag(USART2_TX_DMA_FLAG_GL);
	DMA_Cmd(USART2_TX_DMA_CHANNEL, DISABLE);
	USART2_TX_DMA_CHANNEL->CMAR = (uint32_t)pData;
	USART2_TX_DMA_CHANNEL->CNDTR = Size;//重新写入需要传输数据的数量
    //DMA_SetCurrDataCounter(USART2_TX_DMA_CHANNEL,Size);//重新写入需要传输数据的数量
    // 启动传输
    DMA_Cmd(USART2_TX_DMA_CHANNEL, ENABLE);
	
    while(DMA_GetFlagStatus(USART2_TX_DMA_FLAG_TC) == RESET);
}

// 使用DMA接收数据
void USART2_DMA_ReceiveData(uint8_t *pData, uint16_t Size)
{
    // 清除标志
    DMA_ClearFlag(USART2_RX_DMA_FLAG_GL);
    
    // 设置数据地址和长度
    USART2_RX_DMA_CHANNEL->CMAR = (uint32_t)pData;
    USART2_RX_DMA_CHANNEL->CNDTR = Size;
    
    // 启动接收
    DMA_Cmd(USART2_RX_DMA_CHANNEL, ENABLE);
}

// 串口2初始化函数
void USART2_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能GPIOA和USART2时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // 配置USART2的TX和RX引脚
    GPIO_InitStructure.GPIO_Pin = USART2_TX_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = USART2_RX_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置USART2
    USART_InitStructure.USART_BaudRate = USART2_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

	USART_Cmd(USART2, ENABLE);

	// 初始化DMA
	USART2_DMA_Init();
	
	// 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	// 使能空闲中断，用于DMA接收
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE); 
	//USART_ITConfig(USART2, USART_IT_TC, ENABLE);//发送数据完成触发	
}

// 发送字符串到串口
void USART2_SendString(char* str) {
    // while (*str) {
    //     USART_SendData(USART2, *str++);
	// 	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    // }
	USART2_DMA_SendData((uint8_t*)str, strlen(str));//使用DMA方式发送
}


/*函数名：初始化WiFi的复位IO                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void ESP8266_Reset_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;                    //定义一个设置IO端口参数的结构体
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE); //使能PA端口时钟
	
	GPIO_InitStructure.GPIO_Pin = WIFI_RESET_IO_PIN;               //准备设置PA4
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       //速率50Mhz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   	    //推免输出方式
	GPIO_Init(GPIOA, &GPIO_InitStructure);            	    //设置PA4
	RESET_IO(1);                                            //复位IO拉高电平
}

//清空接收缓存区
void ESP8266_Buf_Clear(void)
{
	//WiFi接收数据量变量清零                        
	//清空WiFi接收缓冲区 	
	g_rx_esp8266_cnt = 0;
	memset(g_rx_esp8266_buf,'\0',sizeof(g_rx_esp8266_buf));
}

/**
  * @brief  发送命令
  * @param  cmd 命令字符串
  * @param  res 响应关键词字符串
  * @param  timeOut 超时时间（100ms的倍数）
  * @retval 0-表示响应成功，1-表示响应失败
  */
char ESP8266_WiFi_SendCmd(char *cmd, char *res, uint8_t timeout)
{
	ESP8266_Buf_Clear();
	USART2_SendString(cmd);
	while(timeout--)
	{
		Delay_ms(100);
		//printf("cmd rx: size:%d %s\r\n", g_rx_esp8266_cnt,g_rx_esp8266_buf);
		if(strstr((const char *)g_rx_esp8266_buf, res) != NULL)		//如果检索到关键词
		{
			return 0;
		}
		printf("timeout:%d ", timeout);		//输出串口的超时时间
	}
	
	return 1; 
}

/*-------------------------------------------------*/
/*函数名：WiFi复位                                 */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char ESP8266_WiFi_Reset(int timeout)
{
	RESET_IO(0);                                    //复位IO拉低电平
	Delay_ms(500);                                  //延时500ms
	RESET_IO(1);                                   	//复位IO拉高电平	
	while(timeout--)								//等待超时时间到0 
	{                              		  
		Delay_ms(100);                              //延时100ms
		if(strstr((const char*)g_rx_esp8266_buf, "ready") != NULL)            //如果接收到ready表示复位成功
		{
			return 0;		         				   	//反之，表示正确，说明收到ready，通过break主动跳出while
		}
		//printf("reset rx: size:%d %s\r\n", g_rx_esp8266_cnt,g_rx_esp8266_buf);
		printf("reset timeout:%d", timeout);                     //串口输出现在的超时时间
	}
	return 1;                      //如果timeout<=0，说明超时时间到了，也没能收到ready，返回1
}

/*-------------------------------------------------*/
/*函数名：WiFi加入路由器指令                       */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char ESP8266_WiFi_JoinAP(int timeout)
{		
	ESP8266_Buf_Clear();
	
	char cmd_buffer[CMD_BUFFER_SIZE];
	// 连接到 WiFi
	snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
	USART2_SendString(cmd_buffer);
	while(timeout--)									   //等待超时时间到0
	{                                   
		Delay_ms(1000);                             	   //延时1s
		if(strstr((const char*)g_rx_esp8266_buf, "WIFI GOT IP\r\n\r\nOK") != NULL)   //如果接收到WIFI GOT IP表示成功
		{
			return 0;		
		}
		printf("joinAp timeout:%d \r\n", timeout);                            //串口输出现在的超时时间
	}
	return 1;                              //如果timeout<=0，说明超时时间到了，也没能收到WIFI GOT IP，返回1                                              //正确，返回0
}

/*-------------------------------------------------*/
/*函数名：连接TCP服务器，并进入透传模式            */
/*参  数：timeout： 超时时间（100ms的倍数）        */
/*返回值：0：正确  其他：错误                      */
/*-------------------------------------------------*/
char ESP8266_WiFi_Connect_TCP_Server(int timeout)
{	ESP8266_Buf_Clear();
	
	char cmd_buffer[CMD_BUFFER_SIZE];
	// 连接到 WiFi
	snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", TCP_Server_IP, TCP_Server_Port);
	USART2_SendString(cmd_buffer);//发送连接服务器指令
	while(timeout--)								  //等待超时与否
	{                           
		Delay_ms(100);                             	  //延时100ms	
		if(strstr((const char*)g_rx_esp8266_buf, "CONNECT"))            //如果接受到CONNECT表示连接成功
		{
			break;                                    //跳出while循环
		}
		if(strstr((const char*)g_rx_esp8266_buf, "CLOSED"))             //如果接受到CLOSED表示服务器未开启
		{
			return 1;                                 //服务器未开启返回1
		}
		if(strstr((const char*)g_rx_esp8266_buf, "ALREADY CONNECTED"))  //如果接受到ALREADY CONNECTED已经建立连接
		{
			return 2;                                 //已经建立连接返回2
		}
		printf("connect tcp server timeout:%d \r\n", timeout);                       //串口输出现在的超时时间  
	}
	printf("\r\n");                                   
	if(timeout <= 0)
	{
		return 3;                         //超时错误，返回3
	}
	else                                              //连接成功，准备进入透传
	{
		printf("连接服务器成功，准备进入透传\r\n");   

		ESP8266_Buf_Clear();
		USART2_SendString("AT+CIPSEND\r\n");                //发送进入透传指令
		while(timeout--)							  //等待超时与否
		{                            
			Delay_ms(100);                            //延时100ms	
			if(strstr((const char*)g_rx_esp8266_buf, "\r\nOK\r\n\r\n>"))//如果成立表示进入透传成功
			{
				break;                          	  //跳出while循环
			}
			printf("cipsend timeout:%d \r\n", timeout);                   //串口输出现在的超时时间  
		}
		if(timeout <= 0)return 4;                     //透传超时错误，返回4	
	}
	return 0;	                                      //成功返回0	
}

/*-------------------------------------------------*/
/*函数名：WiFi_Smartconfig                         */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char ESP8266_WiFi_Smartconfig(int timeout)
{
	ESP8266_Buf_Clear();
	while(timeout--)									//等待超时时间到0
	{                           		
		Delay_ms(1000);                         		//延时1s
		if(strstr((const char*)g_rx_esp8266_buf, "connected"))    	 	//如果串口接受到connected表示成功
		{
			break;                                  	//跳出while循环  
		}
		printf("Smartconfig timeout:%d \r\n", timeout);                 		//串口输出现在的超时时间  
	}	  			
	if(timeout <= 0)return 1;                     		//超时错误，返回1
	return 0;                                   		//正确返回0
}

/*-------------------------------------------------*/
/*函数名：等待加入路由器                           */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char ESP8266_WiFi_WaitAP(int timeout)
{		
	while(timeout--){                               //等待超时时间到0
		Delay_ms(1000);                             //延时1s
		if(strstr((const char*)g_rx_esp8266_buf, "WIFI GOT IP"))      //如果接收到WIFI GOT IP表示成功
		{
			break;       						 
		}
		printf("waitAp timeout:%d \r\n", timeout);                     //串口输出现在的超时时间
	}
	printf("\r\n");                             	//串口输出信息
	if(timeout <= 0)return 1;                       //如果timeout<=0，说明超时时间到了，也没能收到WIFI GOT IP，返回1
	return 0;                                       //正确，返回0
}

/*-------------------------------------------------*/
/*函数名：WiFi连接服务器                           */
/*参  数：无                                       */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char ESP8266_WiFi_Connect_IoTServer(void)
{	
	printf("准备设置STA模式\r\n");                
	if(ESP8266_WiFi_SendCmd("AT+CWMODE=1\r\n","OK",100))			  //设置STA模式，100ms超时单位，总计5s超时时间
	{             
		printf("设置STA模式失败，准备重启\r\n");  //返回非0值，进入if
		return 2;                                 //返回2
	}
	printf("设置STA模式成功\r\n"); 

	printf("准备复位模块\r\n");//设置模式后，需重启才能生效                   
	if(ESP8266_WiFi_Reset(100))							  //复位，100ms超时单位，总计5s超时时间
	//if(ESP8266_WiFi_SendCmd("AT+RST\r\n","OK",100))							  //复位，100ms超时单位，总计5s超时时间
	{                             
		printf("复位失败，准备重启\r\n");	      //返回非0值，进入if
		return 1;                                 //返回1
	} 
	printf("复位成功\r\n");        
	                            
	printf("准备取消自动连接\r\n");            	  
	if(ESP8266_WiFi_SendCmd("AT+CWAUTOCONN=0","OK",50))		  //取消自动连接，100ms超时单位，总计5s超时时间
	{       
		printf("取消自动连接失败，准备重启\r\n"); //返回非0值，进入if
		return 3;                                 //返回3
	}
	printf("取消自动连接成功\r\n");         
			
	printf("准备连接路由器\r\n");                 	
	if(ESP8266_WiFi_JoinAP(30))							  //连接路由器,1s超时单位，总计30s超时时间
	{                          
		printf("连接路由器失败，准备重启\r\n");   //返回非0值，进入if
		return 4;                                 //返回4	
	}
	printf("连接路由器成功\r\n");       		

	printf("准备设置透传\r\n");                    
	if(ESP8266_WiFi_SendCmd("AT+CIPMODE=1","OK",50)) 		  //设置透传，100ms超时单位，总计5s超时时间
	{           
		printf("设置透传失败，准备重启\r\n");     //返回非0值，进入if
		return 8;                                 //返回8
	}
	printf("设置透传成功\r\n");              
	
	printf("准备关闭多路连接\r\n");               
	if(ESP8266_WiFi_SendCmd("AT+CIPMUX=0","OK",50)) 		      //关闭多路连接，100ms超时单位，总计5s超时时间
	{            
		printf("关闭多路连接失败，准备重启\r\n"); //返回非0值，进入if
		return 9;                                 //返回9
	}
	printf("关闭多路连接成功\r\n");         
	 
	printf("准备连接服务器\r\n");                 
	if(ESP8266_WiFi_Connect_TCP_Server(100))      			  //连接服务器，100ms超时单位，总计10s超时时间
	{            
		printf("连接服务器失败，准备重启\r\n");   //返回非0值，进入if
		return 10;                                //返回10
	}
	printf("连接服务器成功\r\n");           
	
	return 0;                                     //正确返回0
}

//连接到MQTT服务器
char ESP8266_Connect_MQTT_Server(void)
{
	char cmd_buffer[CMD_BUFFER_SIZE];
	printf("准备清除MQTT连接\r\n");                    
	USART2_SendString("AT+MQTTCLEAN=0\r\n");
	printf("清除MQTT连接成功\r\n");

	// 配置 MQTT 用户信息
	snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n", MQTT_CLIENT_ID, MQTT_CLIENT_USER, MQTT_CLIENT_PASSWORD);
	printf("配置 MQTT 用户属性\r\n");                    
	if(ESP8266_WiFi_SendCmd(cmd_buffer,"OK",100)) 		  //配置 MQTT 用户属性，100ms超时单位，总计5s超时时间
	{           
		printf("配置 MQTT 用户属性失败，准备重启\r\n");     //返回非0值，进入if
		return 11;                                 
	}
	printf("配置 MQTT 用户属性成功\r\n");

	// 连接到 ThingsCloud MQTT 服务器
	snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+MQTTCONN=0,\"%s\",%d,0\r\n", MQTT_HOST, MQTT_PORT);//最后面的0代表不自动重连
	printf("连接指定 MQTT broker\r\n");  
	// +MQTTCONNECTED:0,1,"gz-3-mqtt.iot-api.com","1883","",0 且 OK
	if(ESP8266_WiFi_SendCmd(cmd_buffer,"OK",100)) 		  //连接指定 MQTT broker，100ms超时单位，总计5s超时时间
	{           
		printf("连接指定 MQTT broker失败，准备重启\r\n");     //返回非0值，进入if
		return 12;                                 
	}
	printf("连接指定 MQTT broker成功\r\n");
	
	return 0;                                     //正确返回0
}

 char ESP8266_WiFi_MQTT_Connect_IoTServer(void) {
//	printf("准备AT测试WIFI模块\r\n");                   
//	if(ESP8266_WiFi_SendCmd("AT\r\n","OK",20))							  //复位，100ms超时单位，总计5s超时时间
//	{                             
//		printf("AT测试WIFI模块失败，请检查硬件连接\r\n");	      //返回非0值，进入if
//		return 1;                                 //返回1
//	} 
//	printf("AT测试成功\r\n");	
	
	printf("准备设置STA模式\r\n");                
	if(ESP8266_WiFi_SendCmd("AT+CWMODE=1\r\n","OK",100))			  //设置STA模式，100ms超时单位，总计5s超时时间
	{             
		printf("设置STA模式失败，准备重启\r\n");  //返回非0值，进入if
		return 2;                                 //返回2
	}
	printf("设置STA模式成功\r\n");
	
	printf("准备复位模块\r\n");//设置模式后，需重启才能生效                   
	if(ESP8266_WiFi_Reset(100))							  //复位，100ms超时单位，总计5s超时时间
	//if(ESP8266_WiFi_SendCmd("AT+RST\r\n","OK",100))							  //复位，100ms超时单位，总计5s超时时间
	{                             
		printf("复位失败，准备重启\r\n");	      //返回非0值，进入if
		return 1;                                 //返回1
	} 
	printf("复位成功\r\n");
	       
	
	printf("准备取消自动连接\r\n");            	  
	if(ESP8266_WiFi_SendCmd("AT+CWAUTOCONN=0\r\n","OK",50))		  //取消自动连接，100ms超时单位，总计5s超时时间
	{       
		printf("取消自动连接失败，准备重启\r\n"); //返回非0值，进入if
		return 3;                                 //返回3
	}
	printf("取消自动连接成功\r\n");         
			
	printf("准备连接路由器\r\n");                 	
	if(ESP8266_WiFi_JoinAP(30))							  //连接路由器,1s超时单位，总计30s超时时间
	{                          
		printf("连接路由器失败，准备重启\r\n");   //返回非0值，进入if
		return 4;                                 //返回4	
	}
	printf("连接路由器成功\r\n");   
	
	printf("准备MQTT连接服务器\r\n");                 	
	if(ESP8266_Connect_MQTT_Server())
	{                          
		printf("连接MQTT服务器失败，准备重启\r\n");   //返回非0值，进入if
		return 5;                                 //返回5
	}
	printf("连接MQTT服务器成功\r\n");       		
	
	printf("准备MQTT订阅主题\r\n");                 	
	if(ESP8266_MQTT_Subscribe())
	{                          
		printf("MQTT订阅主题失败，准备重启\r\n");   //返回非0值，进入if
		return 6;                                 //返回5
	}
	printf("MQTT订阅主题成功\r\n");
    	
	return 0;
 }

// 查询当前WIFI连接状态 返回： +CWJAP: 且 OK
void ESP8266_CheckWiFiStatus(void)
{
	USART2_SendString("AT+CWJAP?\r\n");
	//USART2_SendString("AT+CIFSR\r\n");//连接成功，才能查到ip，所以也可以判断是否连接状态，返回数据：
	/*
	+CIFSR:STAIP,"192.168.14.220"
	+CIFSR:STAMAC,"ec:fa:bc:97:02:a1"

	OK
	*/
	Delay_ms(2000);
}

// 查询当前MQTT连接状态 返回：+MQTTCONN:<LinkID>,<state>,<scheme><"host">,<port>,<"path">,<reconnect> 且 OK
// +MQTTCONN:0,6,1,"gz-3-mqtt.iot-api.com","1883","",1\r\n\r\nOK
void ESP8266_CheckMQTTStatus(void)
{
	USART2_SendString("AT+MQTTCONN?\r\n");
	//Delay_ms(500);
}

void ESP8266_MQTT_Publish(char* message) {
    char cmd_buffer[CMD_BUFFER_SIZE];
	//char message[] = "{\\\"temperature\\\":30}";
	snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n",MQTT_TOPIC,message);
	USART2_SendString(cmd_buffer);
	Delay_ms(2000);
}

char ESP8266_MQTT_Subscribe(void) {
    char cmd_buffer[CMD_BUFFER_SIZE];
    snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+MQTTSUB=0,\"%s\",0\r\n", MQTT_COMMAND_SUB);
    return ESP8266_WiFi_SendCmd(cmd_buffer,"OK",20);
}

// 计算校验和
static uint32_t calculate_checksum(uint8_t command, uint8_t *data, uint16_t length) {
    uint32_t sum = command;
    for (int i = 0; i < length; i++) {
        sum += data[i];
    }
    return sum;
}

// 发送固定格式数据包
void ESP8266_SendPacket(uint8_t command, uint8_t *data, uint16_t length) {
    uint32_t calculated_checksum_val = calculate_checksum(command, data, length);
    uint8_t buffer[PACKET_MIN_LEN + length];
    uint8_t index = 0;

    buffer[index++] = PACKET_HEADER;
    buffer[index++] = (uint8_t)(length >> 8); // 长度高字节
    buffer[index++] = (uint8_t)(length & 0xFF); // 长度低字节
    buffer[index++] = command;
    memcpy(&buffer[index], data, length);
    index += length;
    buffer[index++] = (uint8_t)(calculated_checksum_val >> 24); // 校验和最高字节
    buffer[index++] = (uint8_t)(calculated_checksum_val >> 16); // 校验和次高字节
    buffer[index++] = (uint8_t)(calculated_checksum_val >> 8);  // 校验和次低字节
    buffer[index++] = (uint8_t)(calculated_checksum_val & 0xFF); // 校验和最低字节
    buffer[index++] = PACKET_TAIL;

    USART2_DMA_SendData(buffer, sizeof(buffer));
}

// 解析固定格式数据包
// 返回值：0-成功解析，1-数据不完整，2-校验和错误，3-包头或包尾错误
int ESP8266_ParsePacket(uint8_t *rx_buffer, uint32_t rx_len, Packet_TypeDef *packet) {
    if (rx_len < PACKET_MIN_LEN) {
        return 1; // 数据不完整
    }

    if (rx_buffer[0] != PACKET_HEADER || rx_buffer[rx_len - 1] != PACKET_TAIL) {
        return 3; // 包头或包尾错误
    }

    packet->header = rx_buffer[0];
    packet->length = ((uint16_t)rx_buffer[1] << 8) | rx_buffer[2]; // 组合高低字节
    packet->command = rx_buffer[3];
    packet->data = &rx_buffer[4];
    packet->checksum = ((uint32_t)rx_buffer[rx_len - 5] << 24) | \
                       ((uint32_t)rx_buffer[rx_len - 4] << 16) | \
                       ((uint32_t)rx_buffer[rx_len - 3] << 8)  | \
                       rx_buffer[rx_len - 2]; // 组合 4 字节校验和
    packet->tail = rx_buffer[rx_len - 1];

    // 检查数据长度是否匹配
    if (packet->length != (rx_len - PACKET_MIN_LEN)) {
        return 1; // 数据长度不匹配
    }

    // 校验和检查
    uint32_t calculated_checksum_val = calculate_checksum(packet->command, packet->data, packet->length);
    if (calculated_checksum_val != packet->checksum) {
        return 2; // 校验和错误
    }

    return 0; // 成功解析
}
