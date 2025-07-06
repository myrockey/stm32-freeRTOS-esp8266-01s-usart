#ifndef __ESP8266_H
#define	__ESP8266_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "globals.h" //全局变量头文件

#define USART2_TX_Pin GPIO_Pin_2
#define USART2_RX_Pin GPIO_Pin_3
#define WIFI_RESET_IO_PIN GPIO_Pin_4
#define RESET_IO(x)    GPIO_WriteBit(GPIOA, WIFI_RESET_IO_PIN, (BitAction)x)  //PA4控制WiFi的复位
#define USART2_BAUDRATE 115200

#define WIFI_SSID "test"
#define WIFI_PASSWORD "12345678"
#define MQTT_CLIENT_ID "12"
#define MQTT_CLIENT_USER "ij6kv7tstqsrpzlx"
#define MQTT_CLIENT_PASSWORD "qDsJd0CqfV"
#define MQTT_HOST "gz-3-mqtt.iot-api.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "attributes"
#define MQTT_COMMAND_SUB "command/send/1000" //监听thingsCloud下发的命令
#define MQTT_QoS 0
#define MQTT_RETAIN 0 //0-不保留信息
#define CMD_BUFFER_SIZE 256

//tcp连接服务器
//#define TCP_Server_IP "gz-3-device.iot-api.com"
//#define TCP_Server_Port 28801
//#define TCP_Server_Password "qDsJd0CqfV&ij6kv7tstqsrpzlx"
//tcp 本地测试地址
#define TCP_Server_IP "12.tcp.vip.cpolar.cn"
#define TCP_Server_Port 11114


// USART2 DMA配置
#define USART2_TX_DMA_CHANNEL           DMA1_Channel7
#define USART2_RX_DMA_CHANNEL           DMA1_Channel6
#define USART2_TX_DMA_FLAG_TC           DMA1_FLAG_TC7
#define USART2_RX_DMA_FLAG_TC           DMA1_FLAG_TC6
#define USART2_TX_DMA_FLAG_GL           DMA1_FLAG_GL7
#define USART2_RX_DMA_FLAG_GL           DMA1_FLAG_GL6

// DMA缓冲区大小
#define USART2_DMA_RX_BUFFER_SIZE       1024
#define USART2_DMA_TX_BUFFER_SIZE       1024

extern uint8_t g_rx_dma_buf[USART2_DMA_RX_BUFFER_SIZE];//DMA接收缓冲区
extern volatile uint32_t g_rx_dma_cnt;// 当前接收的字节数

// 定义数据包结构体
#define PACKET_HEADER 0xAA    // 包头
#define PACKET_TAIL   0x55    // 包尾
#define PACKET_HEADER_LEN 1
#define PACKET_LENGTH_LEN 2 // 长度字段现在是 2 字节
#define PACKET_CMD_LEN 1
#define PACKET_CHECKSUM_LEN 4 // 校验和现在是 4 字节
#define PACKET_TAIL_LEN 1
#define PACKET_MIN_LEN (PACKET_HEADER_LEN + PACKET_LENGTH_LEN + PACKET_CMD_LEN + PACKET_CHECKSUM_LEN + PACKET_TAIL_LEN) // 最小包长度
#define PACKET_CMD_MQTT_PUBLISH 0x01 // For publishing MQTT 
#define PACKET_CMD_GET_VALUE 0x02 // For getting value
typedef struct {
    uint8_t header;         // 包头 0xAA
    uint16_t length;         // 数据长度（不包含包头、长度、校验和包尾）
    uint8_t command;        // 命令字
    uint8_t *data;          // 数据部分
    uint32_t checksum;       // 校验和
    uint8_t tail;           // 包尾 0x55
} Packet_TypeDef;

void Delay_ms(uint32_t ms);
// 串口2初始化函数
void USART2_Init(void);
// 发送字符串到串口
void USART2_SendString(char* str);
// 接收串口数据
char USART2_Receive(void);
// ESP8266相关函数
void ESP8266_Reset_IO_Init(void);
//清空接收缓存区
void ESP8266_Buf_Clear(void);
//发送命令
char ESP8266_WiFi_SendCmd(char *cmd, char *res, uint8_t timeout);
/*函数名：WiFi复位                                 */
char ESP8266_WiFi_Reset(int timeout);
/*函数名：WiFi加入路由器指令                       */
char ESP8266_WiFi_JoinAP(int timeout);
/*函数名：连接TCP服务器，并进入透传模式            */
char ESP8266_WiFi_Connect_TCP_Server(int timeout);
/*函数名：WiFi_Smartconfig                         */
char ESP8266_WiFi_Smartconfig(int timeout);
/*函数名：等待加入路由器                           */
char ESP8266_WiFi_WaitAP(int timeout);
/*函数名：WiFi连接IOT服务器                           */
char ESP8266_WiFi_Connect_IoTServer(void);
//连接到MQTT服务器
char ESP8266_Connect_MQTT_Server(void);
//MQTT连接IOT服务器
char ESP8266_WiFi_MQTT_Connect_IoTServer(void);
// 查询当前WIFI连接状态 返回： +CWJAP_DEF: 且 OK
void ESP8266_CheckWiFiStatus(void);
// 查询当前MQTT连接状态 返回：+MQTTCONN:<LinkID>,<state>,<scheme><"host">,<port>,<"path">,<reconnect> 且 OK
void ESP8266_CheckMQTTStatus(void);
void ESP8266_CheckTCPStatus(void);
void ESP8266_MQTT_Publish(char* message);
void ESP8266_TCP_Publish(char* message);
char ESP8266_MQTT_Subscribe(void);

// 添加DMA相关函数声明
void USART2_DMA_Init(void);
void USART2_DMA_SendData(uint8_t *pData, uint16_t Size);
void USART2_DMA_ReceiveData(uint8_t *pData, uint16_t Size);

// 新增的包处理函数声明
void ESP8266_SendPacket(uint8_t command, uint8_t *data, uint16_t length);
int ESP8266_ParsePacket(uint8_t *rx_buffer, uint32_t rx_len, Packet_TypeDef *packet);

#endif /* __ESP8266_H */

