#include "sender.h"

sender::sender(SOCKET sendsocket, std::string recvaddr, int port)
{
    __sendsocket = sendsocket;
    __recvaddr = recvaddr;
    __port = port;
    memset(&__recv_addr, 0, sizeof(__recv_addr));
    __recv_addr.sin_family = AF_INET;
    __recv_addr.sin_port = htons(port);
    __recv_addr.sin_addr.s_addr = inet_addr(recvaddr.c_str());
}

void sender::Connect()
{
    socklen_t addr_len = sizeof(__recv_addr);
    std::string _connect_ = "";
    // 第一次握手
    uint8_t *first = __sdm.get_package(SYNC, (uint8_t *)_connect_.c_str(), __windowsize, 1);
    sendto(__sendsocket, (char *)first, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, addr_len);

    // 发出请求后，接收
    recvfrom(__sendsocket, (char *)buff, 1 + INITSIZE, 0, (struct sockaddr *)&__recv_addr, &addr_len);
}