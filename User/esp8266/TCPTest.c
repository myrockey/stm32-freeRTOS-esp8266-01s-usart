#include <stdio.h>
#include <stdint.h>
#include <string.h>
/*1 / 8 Espressif Systems June 27, 2014
Espressif AT 指令示例
1、 前言
本文介绍如何使用 Espressif AT 指令，指令请参考文档“Espressif AT指令
集”。

2、 使用指南
1） 设备烧录 blank.bin 初始化 wifi 配置，再烧录支持 AT 指令的 sdk 软件。
2） 设备上电。PC 打开串口工具，波特率设置为57600，输入 AT 指令。
2.1. 单连接 client
1） 设置 wifi 模式：
AT+CWMODE=3 //设置为 softAP+station 共存模式
响应：OK
2） 重启生效
AT+RST
响应：OK
3） 连接路由
AT+CWJAP="ssid","password" // 传入路由的ssid 和 password
响应：OK
4） 查询设备IP
AT+CIFSR
6 / 8 Espressif Systems June 27, 2014
响应：192.168.3.106 //返回设备的 IP 地址
5） 在 PC 上使用网络调试助手，创建一个服务器。
6） 设备连接服务器
AT+CIPSTART="TCP","192.168.3.116",8080 //传入协议、服务器 IP、端口号
响应：OK
7） 发送数据
AT+CIPSEND=4 // 发送四个字节，字节数可按需任定
>DGFY // 输入要发送的四个字节内容，无需回车。
响应：SEND OK
注意，若发送的字节数目超过了指令设定的长度n，则会响应busy，并发
送数据的前 n 个字节，完成后响应SEND OK。
8） 接收数据
+IPD,n:xxxxxxxxxx //接收到的数据长度为 n 个字节，xxxxx为数据内容
7 / 8 Espressif Systems June 27, 2014
2.2. 单连接 server
1） 设置 wifi 模式：
AT+CWMODE=3 //设置为 softAP+station 共存模式
响应：OK
2） 重启生效
AT+RST
响应：OK
3） 建立server
AT+CIPSERVER=1 //默认端口 333
响应：OK
4） PC 连入设备 softAP，PC 作 client 连接设备。
注意，ESP8266 作为server 有超时机制，如果连接建立后，一段时间内无
数据来往，server 会将client 踢掉。请在 PC 工具连上 ESP8266 后建立一个
2s 的循环数据发送，用于保持连接。
8 / 8 Espressif Systems June 27, 2014
5） 发送数据
AT+CIPSEND=4 // 发送四个字节，字节数可按需任定
>iopd // 输入要发送的四个字节内容，无需回车。
响应：SEND OK
注意，若发送的字节数目超过了指令设定的长度n，则会响应busy，并发
送数据的前 n 个字节，完成后响应SEND OK。
6） 接收数据
+IPD,n:xxxxxxxxxx //接收到的数据长度为 n 个字节，xxxxx为数据内容
注，本文以单连接为例，多连接请参考本文档与“Espressif AT 指令集”类似
使用。*/
int main() {
    return 0;
}