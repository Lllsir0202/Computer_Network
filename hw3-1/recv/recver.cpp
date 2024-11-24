#include "recver.h"

recver::recver(SOCKET recvsocket, std::string sendaddr, int port, int buffsize)
{
    __recvsocket = recvsocket;
    __sendaddr = sendaddr;
    __port = port;
    memset(&__send_addr, 0, sizeof(__send_addr));
    __send_addr.sin_family = AF_INET;
    __send_addr.sin_port = htons(port);
    __send_addr.sin_addr.s_addr = inet_addr(sendaddr.c_str());
    buff = new uint8_t[buffsize];
}

recver::~recver()
{
    delete[] buff;
}