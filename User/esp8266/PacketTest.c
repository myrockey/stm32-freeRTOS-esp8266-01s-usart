#include <stdio.h>
#include <stdint.h>

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

uint8_t g_rx_buffer[100];
int g_rx_buffer_cnt = 0;

void USART2_DMA_SendData(uint8_t *buffer, int len)
{
    memcpy(g_rx_buffer,buffer,len);
    g_rx_buffer_cnt = len;
    for(int i = 0; i < len; i++)
    {
        printf("buffer:%d ,rx_buffer:%d \r\n",buffer[i],g_rx_buffer[i]);
    }
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

int main() {
    // 创建一个数据包
    Packet_TypeDef packet;
    uint8_t data[] = {'h','0','e','l','l','o','1','2','3','4','5','6','\0','7','w','8'};
    ESP8266_SendPacket(PACKET_CMD_MQTT_PUBLISH, data, sizeof(data));
    
    int status = 0;
    status = ESP8266_ParsePacket(g_rx_buffer,g_rx_buffer_cnt,&packet);
    if(status != 0)
    {
        printf("error:%d\n",status);
        return status;
    }
    // 打印数据包内容
    printf("Header: 0x%X\n", packet.header);
    printf("Length: %u\n", packet.length);
    printf("Command: 0x%X\n", packet.command);
    printf("Data: 0x%X, 0x%X\n", packet.data[0], packet.data[1]);
    printf("Checksum: 0x%X\n", packet.checksum);
    printf("Tail: 0x%X\n", packet.tail);
    
    return 0;
}