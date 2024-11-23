#include "sender.h"

sender::sender(SOCKET sendsocket, std::string recvaddr, int port, int buffsize)
{
    __sendsocket = sendsocket;
    __recvaddr = recvaddr;
    __port = port;
    memset(&__recv_addr, 0, sizeof(__recv_addr));
    __recv_addr.sin_family = AF_INET;
    __recv_addr.sin_port = htons(port);
    __recv_addr.sin_addr.s_addr = inet_addr(recvaddr.c_str());
    buff = new uint8_t[buffsize];
}

sender::~sender()
{
    delete[] buff;
}

void sender::Connect()
{
    socklen_t addr_len = sizeof(__recv_addr);
    std::string _connect_ = "";
    // 第一次握手
    uint8_t *first = __sdm.get_package(SYNC, (uint8_t *)_connect_.c_str(), __windowsize, 1);
    sendto(__sendsocket, (char *)first, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);

    int cnt = 0;
    // 发出请求后，接收
    // 第二次握手是接收端发送的
    while (recvfrom(__sendsocket, (char *)buff, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, &addr_len) == -1)
    {
        if (cnt == 3)
        {
            std::cout << "Failed to connect in First handshake , please retry " << std::endl;
            break;
        }
        // 表示需要重传，即出现了超时
        // 这里尝试重传3次
        std::cout << "First handshake timeout try " << cnt << " time " << std::endl;
        sendto(__sendsocket, (char *)first, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);
        cnt++;
    }
    std::cout << "First handshake Succeed! " << std::endl;
    cnt = 0;
    // 对接收到的数据包进行处理，这里应该判断是不是ACK+SYN
    if (__sdm.solve_package(buff, 1))
    {
        std::cout << "Second handshake Succeed! " << std::endl;
    }
    else
    {
        std::cout << "Second handshake Failed " << std::endl;
    }

    // 第三次握手

    // 释放第一次数据包的空间
    delete first;
    // 这里将数据包的flag位     ACK置位
    first = __sdm.get_package(ACK, (uint8_t *)_connect_.c_str(), __windowsize, 1);
    sendto(__sendsocket, (char *)first, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);
    std::cout << "Third handshake Succeed! " << std::endl;
    // 当超时未收到数据时，应该断开连接
}

void sender::Disconnect()
{
    socklen_t addr_len = sizeof(__recv_addr);
    std::string _connect_ = "";
    // 第一次挥手
    uint8_t *first = __sdm.get_package(FIN, (uint8_t *)_connect_.c_str(), __windowsize, 1);
    sendto(__sendsocket, (char *)first, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);

    int cnt = 0;
    // 发出请求后，接收
    // 第二次挥手是接收端发送的
    while (recvfrom(__sendsocket, (char *)buff, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, &addr_len) == -1)
    {
        if (cnt == 3)
        {
            std::cout << "Failed to disconnect in First wave , please retry " << std::endl;
            break;
        }
        // 表示需要重传，即出现了超时
        // 这里尝试重传3次
        std::cout << "First wave timeout try " << cnt << " times " << std::endl;
        sendto(__sendsocket, (char *)first, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);
        cnt++;
    }
    std::cout << "First wave Succeed! " << std::endl;
    cnt = 0;
    // 对接收到的数据包进行处理，这里应该判断是不是FIN+ACK
    if (__sdm.solve_package(buff, 2))
    {
        std::cout << "Second wave Succeed! " << std::endl;
    }
    else
    {
        std::cout << "Second wave Failed " << std::endl;
    }

    // 第三次挥手

    // 释放第一次数据包的空间
    delete first;

    uint8_t *third = __sdm.get_package(ACK, (uint8_t *)_connect_.c_str(), __windowsize, 1);

    // 这里将数据包的flag位     ACK置位
    sendto(__sendsocket, (char *)first, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);

    while (recvfrom(__sendsocket, (char *)buff, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, &addr_len) == -1)
    {
        if (cnt == 3)
        {
            std::cout << "Failed to disconnect in Third wave , please retry " << std::endl;
            break;
        }
        // 表示需要重传，即出现了超时
        // 这里尝试重传3次
        std::cout << "Third wave timeout try " << cnt << " times " << std::endl;
        sendto(__sendsocket, (char *)third, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);
        cnt++;
    }
    std::cout << "Third wave Succeed! " << std::endl;
    cnt = 0;
    // 对接收到的数据包进行处理，这里应该判断是不是FIN+ACK
    if (__sdm.solve_package(buff, 3))
    {
        std::cout << "Fourth wave Succeed! " << std::endl;
    }
    else
    {
        std::cout << "Fourth wave Failed " << std::endl;
    }
}

void sender::Sendto(uint8_t *d, uint16_t dlen)
{
    socklen_t addr_len = sizeof(__recv_addr);
    uint8_t *Data = __sdm.get_package(TRANS, d, __windowsize, dlen);
    sendto(__sendsocket, (char *)Data, dlen + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);

    int cnt = 0;
    while (recvfrom(__sendsocket, (char *)buff, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, &addr_len) == -1)
    {
        if (cnt == 3)
        {
            std::cout << "Failed to connect in First handshake , please retry " << std::endl;
            break;
        }
        // 表示需要重传，即出现了超时
        // 这里尝试重传3次
        std::cout << "Timeout , retry " << cnt << " time " << std::endl;
        sendto(__sendsocket, (char *)Data, dlen + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);
        cnt++;
    }

    cnt = 0;
    // 对接收到的数据包进行处理，这里应该判断是不是ACK+SYN
    if (__sdm.solve_package(buff, 0))
    {
        std::cout << "Transmit Succeed! " << std::endl;
    }
    else
    {
        std::cout << "Transmit Failed " << std::endl;
    }
}