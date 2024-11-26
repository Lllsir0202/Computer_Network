#include "recver.h"
#include <chrono>

recver::recver(SOCKET recvsocket, std::string sendaddr, int port, int buffsize)
{
    __recvsocket = recvsocket;
    struct timeval timeout;
    timeout.tv_sec = 50;
    timeout.tv_usec = 0;
    if (setsockopt(__recvsocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        perror("setsockopt failed");
        closesocket(__recvsocket);
        WSACleanup();
    }

    Sleep(100);
    __sendaddr = sendaddr;
    __port = port;
    memset(&__send_addr, 0, sizeof(__send_addr));
    __send_addr.sin_family = AF_INET;
    __send_addr.sin_port = htons(port);
    __send_addr.sin_addr.s_addr = inet_addr(sendaddr.c_str());
    buff = new uint8_t[buffsize];
    __buffsize = buffsize;
}

recver::~recver()
{
    closesocket(__recvsocket);
    WSACleanup();
    delete[] buff;
}

bool recver::Connect()
{
    __rdm.init_ISN();
    socklen_t addr_len = sizeof(__send_addr);
    // 对于接收端，首先应该一直接收
    std::cout << "Waiting client connect... " << std::endl;
    auto starttime = std::chrono::steady_clock::now();
    auto nowtime = std::chrono::steady_clock::now();
    while (recvfrom(__recvsocket, (char *)buff, 1 + INITSIZE, 0, (struct sockaddr *)&__send_addr, &addr_len) == -1)
    {
        nowtime = std::chrono::steady_clock::now();
        auto elpasedtime = std::chrono::duration_cast<std::chrono::seconds>(nowtime - starttime);
        // std::cout << "Times Timeout " << test++ << std::endl;
        //  如果60秒没有发起连接，那么我们就
        if (elpasedtime.count() >= 60)
        {
            std::string log = "No client connect ";
            __rdm.add_log(log);
            return false;
        }
    }
    // std::cout << "here" << std::endl;
    // 这是接收到第一次握手的信息
    if (__rdm.solve_package(buff, 1))
    {
    }
    else
    {
        std::string log = "First handshake Failed ";
        __rdm.add_log(log);
    }
    // std::cout << "here" << std::endl;
    //  然后发送第二次握手，再接收第三次握手信息
    std::string _connect_ = "";
    uint8_t *second = __rdm.get_package((SYNC | ACK), (uint8_t *)_connect_.c_str(), __windowsize, (uint16_t)1);
    sendto(__recvsocket, (char *)second, 1 + INITSIZE, 0, (struct sockaddr *)&__send_addr, addr_len);

    int cnt = 0;
    // 接收第三次握手
    starttime = std::chrono::steady_clock::now();
    while (recvfrom(__recvsocket, (char *)buff, 1 + INITSIZE, 0, (struct sockaddr *)&__send_addr, &addr_len) == -1)
    {
        if (cnt == 10)
        {
            std::string log = "Failed to connect in Second handshake , please retry ";
            __rdm.add_log(log);
            return false;
        }
        // 表示需要重传，即出现了超时
        // 这里尝试重传5次
        nowtime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(nowtime - starttime).count() >= 1)
        {
            cnt++;
            std::string log = "Second handshake timeout try " + std::to_string(cnt) + " time " + " Times passed " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(nowtime - starttime).count()) + "ms";
            __rdm.add_log(log);
            sendto(__recvsocket, (char *)second, 1 + INITSIZE, 0, (struct sockaddr *)&__send_addr, addr_len);
            starttime = std::chrono::steady_clock::now();
        }
    }

    std::string log = "Second handshake Succeed! ";
    __rdm.add_log(log);
    cnt = 0;
    // 对接收到的数据包进行处理，这里应该判断是不是ACK+SYN
    if (__rdm.solve_package(buff, 2))
    {
        // std::cout << "Second handshake Succeed! " << std::endl;
    }
    else
    {
        std::string log = "Third handshake Failed ";
        __rdm.add_log(log);
    }

    delete second;
    return true;
}

void recver::Disconnect()
{
    // 这里假定还是发送端发给接收端的情况
    socklen_t addr_len = sizeof(__send_addr);
    // 对于接收端，首先应该一直接收
    std::cout << "Start to Disconnect... " << std::endl;

    // 然后发送第二次挥手，再接收第三次挥手信息
    std::string _connect_ = "";
    uint8_t *second = __rdm.get_package(FIN | ACK, (uint8_t *)_connect_.c_str(), __windowsize, (uint16_t)1);
    sendto(__recvsocket, (char *)second, 1 + INITSIZE, 0, (struct sockaddr *)&__send_addr, addr_len);
    std::string log = "Second Wave Succeed! ";
    __rdm.add_log(log);

    int cnt = 0;
    // 接收第三次挥手
    auto starttime = std::chrono::steady_clock::now();
    auto nowtime = std::chrono::steady_clock::now();
    while (recvfrom(__recvsocket, (char *)buff, 1 + INITSIZE, 0, (struct sockaddr *)&__send_addr, &addr_len) == -1)
    {
        if (cnt == 5)
        {
            std::string log = "Failed to Disconnect in Third Wave , please retry ";
            __rdm.add_log(log);
            return;
        }
        // 表示需要重传，即出现了超时
        // 这里尝试重传3次
        nowtime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(nowtime - starttime).count() >= 1)
        {
            cnt++;
            std::string log = "Third Wave timeout try " + std::to_string(cnt) + " time " + " Times passed " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(nowtime - starttime).count()) + "ms";
            __rdm.add_log(log);
            sendto(__recvsocket, (char *)second, 1 + INITSIZE, 0, (struct sockaddr *)&__send_addr, addr_len);
            starttime = std::chrono::steady_clock::now();
        }
    }

    log = "Third Wave Succeed! ";
    __rdm.add_log(log);
    cnt = 0;
    // 对接收到的数据包进行处理，这里应该判断是不是ACK+SYN
    if (__rdm.solve_package(buff, 4))
    {
        // std::cout << "Second handshake Succeed! " << std::endl;
    }
    else
    {
        std::string log = "Third handshake Failed ";
        __rdm.add_log(log);
    }

    delete second;
    uint8_t *fourth = __rdm.get_package(ACK, (uint8_t *)_connect_.c_str(), __windowsize, (uint16_t)1);
    // 发出第四次挥手
    sendto(__recvsocket, (char *)fourth, 1 + INITSIZE, 0, (struct sockaddr *)&__send_addr, addr_len);
    __rdm.add_log("Fourth wave Succeed!");
    delete fourth;
}

void recver::Recvfrom()
{
    socklen_t addr_len = sizeof(__send_addr);
    // 对于接收端，需要处理接收数据，并且当结束时需要调用Disconnect
    while (1)
    {
        int d = recvfrom(__recvsocket, (char *)buff, __buffsize, 0, (struct sockaddr *)&__send_addr, &addr_len);
        if (d != -1)
        {
            if (!__rdm.solve_package(buff, 0))
            {
                break;
            }
            std::string raw = "";
            uint8_t *ack = __rdm.get_package(ACK, (uint8_t *)raw.c_str(), __windowsize, (uint16_t)1);
            sendto(__recvsocket, (char *)ack, 1 + INITSIZE, 0, (struct sockaddr *)&__send_addr, addr_len);
            std::string log = "Send ACK of " + std::to_string(__rdm.get_ACK());
            __rdm.add_log(log);
        }
    }
    Disconnect();
}