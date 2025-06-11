#include "stm32f10x.h"                  // Device header

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//开启外设GPIOA的时钟
	//GPIO初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//通用推挽输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;//配置 PA1 PA2引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//默认置为高电平，LED灭状态
	GPIO_SetBits(GPIOA,GPIO_Pin_1);
	GPIO_SetBits(GPIOA,GPIO_Pin_2);
}

void LED1_ON(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);//设置PA1引脚 低电平
}

void LED1_OFF(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_1);//设置PA1引脚 高电平
}

void LED1_Turn(void)
{
	//获取输出位的值
	if(0 == GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_1))
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_1);
	}
	else
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
	}
}

void LED2_ON(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_2);
}

void LED2_OFF(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_2);
}

void LED2_Turn(void)
{
	//获取输出位的值
	if(0 == GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2))
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_2);
	}
	else
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_2);
	}
}


