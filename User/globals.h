#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

#define STACK_SIZE 128
#define RX_BUFFER_SIZE 1024
#define BUFFER_SIZE 1024        /* 环形缓冲区的大小 */

extern volatile char pingFlag;       //ping报文状态       0：正常状态，等待计时时间到，发送Ping报文

extern uint8_t g_rx_esp8266_buf[RX_BUFFER_SIZE];//接收缓冲区
extern volatile uint32_t g_rx_esp8266_cnt;// 当前接收的字节数

extern void Delay_ms(uint32_t ms);
extern void Filter_memcpy(uint8_t *dst, uint8_t *src, int size);

#endif /*_GLOBALS_H*/
