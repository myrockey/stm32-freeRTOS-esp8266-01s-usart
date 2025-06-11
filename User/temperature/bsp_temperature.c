#include "bsp_temperature.h"

void Thermistor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 配置热敏传感器引脚为模拟输入
    GPIO_InitStructure.GPIO_Pin = THERMISTOR_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(THERMISTOR_PORT, &GPIO_InitStructure);

    // 使能ADC时钟
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // 配置ADC
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    // 使能ADC
    ADC_Cmd(ADC1, ENABLE);

    // 校准ADC
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

float Thermistor_Read_Temperature(void)
{
    uint16_t adcValue;
    float voltage, resistance, temperature;

    // 启动ADC转换
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

    // 读取ADC值
    adcValue = ADC_GetConversionValue(ADC1);

    // 计算电压
    voltage = (adcValue * 3.3) / 4096.0;

    // 计算热敏电阻的电阻值
    resistance = (THERMISTOR_REFERENCE_RESISTANCE * voltage) / (3.3 - voltage);

    // 使用Steinhart-Hart方程计算温度
    temperature = THERMISTOR_NOMINAL_RESISTANCE / resistance;
    temperature = log(temperature);
    temperature = 1.0 / (temperature / THERMISTOR_BETA + 1.0 / (25.0 + 273.15));
    temperature -= 273.15;

    return temperature;
}

