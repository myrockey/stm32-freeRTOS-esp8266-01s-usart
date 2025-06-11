#ifndef __TEMPERATURE_H
#define	__TEMPERATURE_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f10x.h"
#include <stdio.h>
#include <math.h>


// 定义热敏传感器引脚
#define THERMISTOR_PIN GPIO_Pin_6
#define THERMISTOR_PORT GPIOA

// 热敏传感器参数
#define THERMISTOR_NOMINAL_RESISTANCE 10000 // 热敏电阻的标称电阻（欧姆）
#define THERMISTOR_BETA 3435 // 热敏电阻的B值
#define THERMISTOR_REFERENCE_RESISTANCE 10000 // 分压电阻（欧姆）



void Thermistor_Init(void);
float Thermistor_Read_Temperature(void);

#endif /* __TEMPERATURE_H */


