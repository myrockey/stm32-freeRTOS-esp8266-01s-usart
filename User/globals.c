#include "globals.h"

volatile char pingFlag = 0;       //ping报文状态       0：正常状态，等待计时时间到，发送Ping报文
                         //ping报文状态       1：Ping报文已发送，当收到 服务器回复报文的后 将1置为0

uint8_t g_rx_esp8266_buf[RX_BUFFER_SIZE] = {0};
volatile uint32_t g_rx_esp8266_cnt = 0;// 当前接收的字节数

// FreeRTOS 的延时函数
void Delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));  // 将毫秒转换为 tick 数
}
