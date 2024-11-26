#include "sender.h"
#include <cassert>

sender::sender(SOCKET sendsocket, std::string recvaddr, int port, int buffsize)
{
    __sendsocket = sendsocket;
    struct timeval timeout;
    timeout.tv_sec = 50;
    timeout.tv_usec = 0;
    if (setsockopt(__sendsocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        perror("setsockopt failed");
        closesocket(sendsocket);
        WSACleanup();
    }

    Sleep(100);

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
    closesocket(__sendsocket);
    WSACleanup();
    delete[] buff;
}

bool sender::Connect()
{
    // 初始化序列号
    __sdm.init_ISN();
    socklen_t addr_len = sizeof(__recv_addr);
    std::string _connect_ = "";
    // 第一次握手
    uint8_t *first = __sdm.get_package(SYNC, (uint8_t *)_connect_.c_str(), __windowsize, (uint16_t)1);
    sendto(__sendsocket, (char *)first, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);

    int cnt = 0;
    // 发出请求后，接收
    // 第二次握手是接收端发送的
    auto starttime = std::chrono::steady_clock::now();
    auto nowtime = std::chrono::steady_clock::now();
    while (recvfrom(__sendsocket, (char *)buff, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, &addr_len) == -1)
    {
        if (cnt == 5)
        {
            std::string log = "Failed to connect in First handshake , please retry ";
            __sdm.add_log(log);
            return false;
        }
        // 表示需要重传，即出现了超时
        // 这里尝试重传3次
        nowtime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(nowtime - starttime).count() >= 1)
        {
            cnt++;
            std::string log = "First handshake timeout try " + std::to_string(cnt) + " time " + " Times passed " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(nowtime - starttime).count()) + "ms";
            __sdm.add_log(log);
            sendto(__sendsocket, (char *)first, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);

            // 更新开始时间重新计时
            starttime = std::chrono::steady_clock::now();
        }
    }
    std::string log = "First handshake Succeed! ";
    __sdm.add_log(log);
    cnt = 0;
    // 对接收到的数据包进行处理，这里应该判断是不是ACK+SYN
    if (__sdm.solve_package(buff, 1))
    {
        // std::cout << "Second handshake Succeed! " << std::endl;
    }
    else
    {
        std::string log = "Second handshake Failed ";
        __sdm.add_log(log);
        return false;
    }

    // 第三次握手

    // 释放第一次数据包的空间
    delete first;
    // 这里将数据包的flag位     ACK置位
    uint8_t *third = __sdm.get_package(ACK, (uint8_t *)_connect_.c_str(), __windowsize, (uint16_t)1);
    sendto(__sendsocket, (char *)third, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);
    log = "Third handshake Succeed! ";
    __sdm.add_log(log);
    // 当超时未收到数据时，应该断开连接
    Sleep(300);
    delete third;
    return true;
}

void sender::Disconnect()
{
    socklen_t addr_len = sizeof(__recv_addr);
    std::string _connect_ = "";
    // 第一次挥手
    uint8_t *first = __sdm.get_package(FIN, (uint8_t *)_connect_.c_str(), __windowsize, (uint16_t)1);
    sendto(__sendsocket, (char *)first, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);

    int cnt = 0;
    // 发出请求后，接收
    // 第二次挥手是接收端发送的
    auto starttime = std::chrono::steady_clock::now();
    auto nowtime = std::chrono::steady_clock::now();
    while (recvfrom(__sendsocket, (char *)buff, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, &addr_len) == -1)
    {
        if (cnt == 5)
        {
            std::string log = "Failed to disconnect in First wave , please retry ";
            __sdm.add_log(log);
            return;
        }
        // 表示需要重传，即出现了超时
        // 这里尝试重传5次
        nowtime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(nowtime - starttime).count() >= 1)
        {
            cnt++;
            std::string log = "First wave timeout try " + std::to_string(cnt) + " times " + " Times passed " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(nowtime - starttime).count()) + "ms";
            __sdm.add_log(log);
            sendto(__sendsocket, (char *)first, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);
            starttime = std::chrono::steady_clock::now();
        }
    }
    __sdm.add_log("First wave Succeed! ");
    cnt = 0;
    // 对接收到的数据包进行处理，这里应该判断是不是FIN+ACK
    if (__sdm.solve_package(buff, 2))
    {
        // std::cout << "Second wave Succeed! " << std::endl;
    }
    else
    {
        std::string log = "Second wave Failed ";
        __sdm.add_log(log);
    }

    // 第三次挥手

    // 释放第一次数据包的空间
    delete first;

    uint8_t *third = __sdm.get_package(ACK, (uint8_t *)_connect_.c_str(), __windowsize, (uint16_t)1);

    // 这里将数据包的flag位     ACK置位
    sendto(__sendsocket, (char *)first, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);

    starttime = std::chrono::steady_clock::now();
    nowtime = std::chrono::steady_clock::now();
    while (recvfrom(__sendsocket, (char *)buff, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, &addr_len) == -1)
    {
        if (cnt == 5)
        {
            std::string log = "Failed to disconnect in Third wave , please retry ";
            __sdm.add_log(log);
            return;
        }
        // 表示需要重传，即出现了超时
        // 这里尝试重传5次
        nowtime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(nowtime - starttime).count() >= 1)
        {
            cnt++;
            std::string log = "Third wave timeout try " + std::to_string(cnt) + " times " + " Times passed " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(nowtime - starttime).count()) + "ms";
            __sdm.add_log(log);
            sendto(__sendsocket, (char *)third, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);
            starttime = std::chrono::steady_clock::now();
        }
    }
    __sdm.add_log("Third wave Succeed");
    cnt = 0;
    // 对接收到的数据包进行处理，这里应该判断是不是FIN+ACK
    if (__sdm.solve_package(buff, 3))
    {
        // std::cout << "Fourth wave Succeed! " << std::endl;
    }
    else
    {
        __sdm.add_log("Fourth wave Failed");
    }
}

void sender::Sendto(uint8_t *d, uint16_t dlen, uint8_t flag)
{
    socklen_t addr_len = sizeof(__recv_addr);
    // 确保flag是TRANS或者START
    assert(flag == START || flag == TRANS);
    uint8_t *Data = __sdm.get_package(flag, d, __windowsize, dlen);
    std::cout << "len is " << dlen << std::endl;
    sendto(__sendsocket, (char *)Data, dlen + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);

    int cnt = 0;
    auto starttime = std::chrono::steady_clock::now();
    auto nowtime = std::chrono::steady_clock::now();
    while (recvfrom(__sendsocket, (char *)buff, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, &addr_len) == -1)
    {
        if (cnt == 5)
        {
            std::cout << "Failed to recv ACK , please retry " << std::endl;
            return;
        }
        // 表示需要重传，即出现了超时
        // 这里尝试重传5次
        nowtime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(nowtime - starttime).count() >= 1)
        {
            cnt++;
            std::cout << std::endl;
            std::cout << "Timeout , retry " << cnt << " time " << std::endl;
            std::cout << std::endl;
            sendto(__sendsocket, (char *)Data, dlen + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);
            starttime = std::chrono::steady_clock::now();
        }
    }

    cnt = 0;
    // 对接收到的数据包进行处理，这里应该判断是不是ACK
    if (__sdm.solve_package(buff, 0))
    {
        std::cout << "Transmit Succeed! " << std::endl;
    }
    else
    {
        std::cout << "Transmit Failed " << std::endl;
    }
}