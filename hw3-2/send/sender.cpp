#include "sender.h"
#include <cassert>

sender::sender(SOCKET sendsocket, std::string recvaddr, int port, int buffsize)
{
    __sendsocket = sendsocket;
    u_long mode = 1; // 非阻塞模式
    if (ioctlsocket(__sendsocket, FIONBIO, &mode) != 0)
    {
        perror("ioctlsocket failed");
        closesocket(__sendsocket);
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
    recvbuff = new uint8_t[1 + INITSIZE];

    // Add in 3-2
    is_Running = false;
}

sender::~sender()
{
    Stop();
    closesocket(__sendsocket);
    WSACleanup();
    delete[] buff;
    delete[] recvbuff;
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
    uint8_t *third = __sdm.get_package(ACK, (uint8_t *)_connect_.c_str(), __windowsize, (uint16_t)1, false);
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
    sendto(__sendsocket, (char *)third, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);

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

    __sdm.add_log("Succedd to disconnect! ");
}

// 在这里我们对于发送握手和挥手的时候，还是采用停等机制，对于传输数据时，我们重写函数Sendto，用于传输数据时的滑动窗口
void sender::Sendto(uint8_t *d, uint16_t dlen, uint8_t flag)
{
    if (__sdm.get_seq2data_size() < 33)
    {
        socklen_t addr_len = sizeof(__recv_addr);
        // 确保flag是TRANS或者START
        assert(flag == START || flag == TRANS);
        uint8_t *Data = __sdm.get_package(flag, d, __windowsize, dlen);
        std::cout << "len is " << dlen << std::endl;
        Lock();
        sendto(__sendsocket, (char *)Data, dlen + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);
        Unlock();
        Sleep(10);
    }
    // 当现在的缓冲区等于最大值时
    while (__sdm.get_seq2data_size() == 32)
    {
        // 用适度的吞吐下降换取避免忙等待
        // std::cout << "123" << std::endl;
        Sleep(1000);
    }
}

void sender::Recv()
{
    // 当正在运行时并且seq不为空
    while (1)
    {
        Lock();
        if (__sdm.if_empty() && !is_Running)
        {
            Unlock();
            break;
        }
        Unlock();
        socklen_t addr_len = sizeof(__recv_addr);
        int cnt = 0;
        auto starttime = std::chrono::steady_clock::now();
        auto nowtime = std::chrono::steady_clock::now();
        // 这里修改为当没接收到并且缓冲池不为空时，不然会出现没有发数据，但等待接收的情况
        while (recvfrom(__sendsocket, (char *)recvbuff, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, &addr_len) == -1)
        {
            if (cnt == 5)
            {
                std::cout << "Failed to recv ACK , please retry " << std::endl;
                return;
            }
            // 表示需要重传，即出现了超时
            // 这里尝试重传5次
            nowtime = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(nowtime - starttime).count() >= 500 || __sdm.get_cnt() >= 3)
            {
                cnt++;
                // 清空cnt记录的接收到ack数目
                data *d;
                uint16_t dlen;
                uint8_t *Data;
                __sdm.clear_cnt();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(nowtime - starttime).count() >= 500)
                {
                    // 当超时并且为空时，退出
                    if (__sdm.if_empty())
                    {
                        break;
                    }
                    std::cout << std::endl;
                    std::string log = "Timeout , retry " + std::to_string(cnt) + " time ";
                    __sdm.add_log(log);
                    std::cout << std::endl;
                }
                else
                {
                    // 当超时并且为空时，退出
                    if (__sdm.if_empty())
                    {
                        Unlock();
                        break;
                    }
                    std::cout << std::endl;
                    std::string log = "Acknowledge 3 times acknum of last package , retry " + std::to_string(cnt) + " time ";
                    __sdm.add_log(log);
                    std::cout << std::endl;
                }
                if (!__sdm.if_empty())
                {
                    Lock();
                    auto it = __sdm.get_seq2data_iter();
                    auto end = __sdm.get_seq2data_end();
                    while (it != end)
                    {
                        it++;
                        d = __sdm.get_first_data();
                        std::cout << "current data begin seq is " << __sdm.get_first_data()->get_seq() << std::endl;
                        std::cout << "d seqnum is " << d->get_seq() << std::endl;
                        dlen = d->get_datalen();
                        Data = d->gen_data(d->get_data());
                        sendto(__sendsocket, (char *)Data, dlen + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);
                    }
                    Unlock();
                }

                delete Data;
                starttime = std::chrono::steady_clock::now();
            }
        }
        cnt = 0;
        // std::cout << "333" << std::endl;
        // std::cout << "current first data seqnum " << __sdm.get_first_data()->get_seq() << std::endl;
        // 对接收到的数据包进行处理，这里应该判断是不是ACK
        Lock();
        if (__sdm.solve_package(recvbuff, 0))
        {
            std::cout << "Seqnum " << __sdm.get_SEQ() << " Transmit Succeed! " << std::endl;
        }
        else
        {
            std::cout << "Transmit Failed " << std::endl;
        }
        Unlock();
    }
}