#include "stm32f10x.h"                  // Device header

void Buzzer_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//开启外设GPIOB的时钟
	//GPIO初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;// PB12号引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//设置默认初始高电平
	GPIO_SetBits(GPIOB, GPIO_Pin_12);//设置PB12引脚 高电平
}

void Buzzer_ON(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);//设置PB12引脚 低电平
}

void Buzzer_OFF(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_12);//设置PB12引脚 高电平
}

void Buzzer_Turn(void)
{
	//获取输出位的值
	if(0 == GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_12))
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_12);//设置PB12引脚 高电平	
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);//设置PB12引脚 低电平
	}
}


