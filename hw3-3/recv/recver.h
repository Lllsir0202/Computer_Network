#pragma once
#include "../include/datamanager/recvdatamanager.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <fstream>

// 这个类应该封装对于接收端的处理
class recver
{
public:
    // 可以在初始化时就得到socket，然后后续调用不需要传递socket了
    recver(SOCKET recvsocket, std::string sendaddr, int port, int buffsize);

    // 析构函数释放内存
    ~recver();

    // 这里在连接阶段和挥手阶段应该有一些不同
    // 这里被创建后，应该不停接收信息，再接收后发送
    bool Connect();

    void Disconnect();

    void Recvfrom();

    // virtual bool reset_send(std::string sendaddr, int port) = 0;

    // 这是获取私有变量的函数
    recvdatamanager *get_rdm() { return &__rdm; };

    // 获得目标地址
    std::string get_sendaddr() { return __sendaddr; };

    // 获得目标端口
    int get_port() { return __port; };

private:
    recvdatamanager __rdm;

    // 记录接收数据socket(指用来接收数据的)
    SOCKET __recvsocket;

    // 在这里我们规定一个sender只能处理一个初始地址到目标地址的发送
    std::string __sendaddr;
    int __port;
    struct sockaddr_in __send_addr;

    // 窗体大小
    uint32_t __windowsize = 0;

    // 缓冲区
    uint8_t *buff;

    int __buffsize;
};