#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//开启外设GPIOB的时钟
	//GPIO初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;//配置PB1 PB11号引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

uint8_t Key_GetNum(void)
{
	uint8_t KeyNum;
	//获取输入位的值
	if(0 == GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1))
	{
		Delay_ms(20);//延时消抖
		while(0 == GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1));//持续到松开按键时，跳出循环往下执行
		Delay_ms(20);//延时消抖
		
		KeyNum = 1;
	}
	
	if(0 == GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11))
	{
		Delay_ms(20);//延时消抖
		while(0 == GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11));//持续到松开按键时，跳出循环往下执行
		Delay_ms(20);//延时消抖
		
		KeyNum = 2;
	}
	
	return KeyNum;
}

