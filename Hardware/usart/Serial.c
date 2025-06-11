#include "Serial.h"                  // Device header

void Serial_Init(void)
{
	//RCC开�?时钟 USART GPIO
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	//GPIO初�?�化
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出，作为串口发送时
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//PA9引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	//USART初�?�化
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;//波特�?
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不需要硬件流控制
	USART_InitStructure.USART_Mode = USART_Mode_Tx;//发送模�?
	USART_InitStructure.USART_Parity = USART_Parity_No;//奇偶校验，不需�?
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//停�??位，1�?
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//传输数据长度�?8�?
	USART_Init(USART1,&USART_InitStructure);
	//USART使能
	USART_Cmd(USART1,ENABLE);
}

void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1,(uint16_t)Byte);//写入数据寄存�?，写�?USART后会�?动生成时序波�?
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);//等待发送完�?
	//下�?�写入数�?寄存器会�?动清除发送完成标志位，所以这里不需要清除标志位
}

void Serial_SendArray(uint8_t *Array,uint16_t Length)
{
	uint16_t i;
	for(i = 0; i< Length;i++)
	{
		Serial_SendByte(Array[i]);//依�?�调用发送每�?字节
	}
}

void Serial_SendString(char *String)
{
	uint16_t i;
	for(i=0;String[i] != '\0';i++)
	{
		Serial_SendByte(String[i]);
	}
}

uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Res = 1;
	while(Y--)
	{
		Res *= X;
	}
	return Res;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint16_t i;
	for(i=0;i<Length;i++)
	{
		Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');	//依�?�调用Serial_SendByte发送每位数�?
	}
}

/**
  * �?    数：使用printf需要重定向的底层函�?
  * �?    数：保持原�?�格式即�?，无需变动
  * �? �? 值：保持原�?�格式即�?，无需变动
  */
int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);//将printf的底层重定向到自己的发送fputc函数
	return ch;
}

/**
  * �?    数：�?己封装的prinf函数
  * �?    数：format 格式化字符串
  * �?    数：... �?变的参数列表
  * �? �? 值：�?
  */
void Serial_Printf(char *format,...)
{
	char String[100];//定义字�?�数�?
	va_list arg;//定义�?变参数列表数�?类型的变量arg
	va_start(arg,format);//从format开始，接受�?变参数列表到arg
	vsprintf(String,format,arg);//使用vsprintf打印格式化字符串和参数列表到字�?�数组中
	va_end(arg);//结束变量arg
	Serial_SendString(String);//串口发送字符串
}

