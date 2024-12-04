#pragma once
#include "../include/datamanager/senddatamanager.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <fstream>
#include <thread>
#include <shared_mutex>

// enum class State {CLOSED,SYN_SENT, ESTABLISHED, };

// 这个类封装的是处理发送端的各种情况，希望提供连接的接口(三次握手，重传等)，以及断开连接(四次挥手等)
class sender
{
public:
    // 可以在初始化时就得到socket，然后后续调用不需要传递socket了
    sender(SOCKET sendsocket, std::string recvaddr, int port, int buffsize);

    // 析构函数释放内存
    ~sender();

    // 在外层处理好socket创建，传递给sender，就可以连接了
    // 这里肯定是发送端发起连接，所以这里应该先发送SYNC数据包
    bool Connect();

    // 四次挥手
    void Disconnect();

    // 发送数据
    void Sendto(uint8_t *d, uint16_t dlen, uint8_t flag);

    // Add in 3-2
    // 用于处理接收ack
    void Recv();

    // 这是修改目标地址和端口的函数
    // 注意修改后可能需要重新连接，而且这里暂时设为虚函数避免不必要的问题
    // virtual bool reset_recv(std::string recvaddr, int port) = 0;

    // 这是获得senddatamanaer的函数
    senddatamanager *get_sdm() { return &__sdm; };

    // 获得目标地址
    std::string get_recvaddr() { return __recvaddr; };

    // 获得目标端口
    int get_port() { return __port; };

    // Added in 3-2
    // 启动接收线程
    void Start()
    {
        is_Running = true;
        __recv_thread = std::thread(&sender::Recv, this);
        std::cout << "Thread started." << std::endl;
    }
    // 停止接收线程
    void Stop()
    {
        if (is_Running)
        {
            is_Running = false;
            if (__recv_thread.joinable())
            {
                __recv_thread.join();
            }
        }
    }

    // 创建一个新的线程用来重传
    // 这是线程函数
    void Resendfunc(data *d, uint16_t dlen, uint8_t *Data, socklen_t addr_len)
    {
        Lock();
        auto it = __sdm.get_seq2data_iter();
        auto end = __sdm.get_seq2data_end();
        while (it != end)
        {
            d = it->second;
            std::cout << "d seqnum is " << d->get_seq() << std::endl;
            dlen = d->get_datalen();
            Data = d->gen_data(d->get_data());
            sendto(__sendsocket, (char *)Data, dlen + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);
            it++;
        }
        Unlock();
    }

    void Resend(data *d, uint16_t dlen, uint8_t *Data, socklen_t addr_len)
    {
        __resend_thread = std::thread(std::bind(&sender::Resendfunc, this, d, dlen, Data, addr_len));
        if (__resend_thread.joinable())
        {
            __resend_thread.join();
        }
    }

    void Lock() { mtx.lock(); }
    void Unlock() { mtx.unlock(); }

    void set_size(int win_size)
    {
        __size = win_size;
    }

private:
    senddatamanager __sdm;

    // 记录发送数据socket(指用来发送数据的)
    SOCKET __sendsocket;

    // 在这里我们规定一个sender只能处理一个初始地址到目标地址的发送
    std::string __recvaddr;
    int __port;
    struct sockaddr_in __recv_addr;

    // 窗体大小
    uint32_t __windowsize = 0;

    // 缓冲区
    uint8_t *buff;

    // 用于存储接收到的ack
    uint8_t *recvbuff;

    // followings are added in 3-2
    // 这里的思路主要是：创建一个新的线程用来接收数据，同时对于seq2data进行确认，当seq2data.size() == SetWindowsize时
    // 我们把传输Sendto阻塞，同时在Recv函数中处理重传的情况
    std::thread __recv_thread;
    std::thread __resend_thread;
    bool is_Running;
    std::mutex mtx;

    // 用于设定窗口大小
    int __size;
};