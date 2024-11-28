### 目录简介

目录如下：

    hw3-1/
    ├── include
        ├── data.h
        ├── data.cpp
        ├── defs.h
        ├── datamanager
            ├── recvdatamanager.h
            ├── recvdatamanager.cpp
            ├── senddatamanager.h
            ├── senddatamanager.cpp
        ├── throughoutput.h
    ├── recv
        ├── recver.h
        ├── recver.cpp
    ├── send
        ├── sender.h
        ├── sender.cpp
    ├── client
        ├── client.cpp
    ├── server
        ├── server.cpp

其中``/include/data.cpp(h)``主要封装了对于传输数据的处理和生成传输的数据包功能。

``/include/defs.h``提供了一些宏定义，主要是常数。

``/include/datamanager/``文件夹下的send(recv)datamanager类主要用来处理一些对data类的接口，用于记录未确认的数据包、产生一个新的数据包、确认一个数据包、对数据包进行校验和检验等功能。

``/include/throughoutput.h``是处理对吞吐量的一个计算，在这里创建了一个单独的线程，用来记录吞吐量并输出日志。

``/recv/recv.cpp(h)``是处理接收端的一个封装类，在这里仅处理了连接、断开连接、接收的接口。

``/send/send.cpp(h)``是处理发送端的一个封装类，在这里处理了连接、断开连接、发送的接口。

``/client/client.cpp``处理的是客户端也就是发送端的操作。

``/server/server.cpp``是服务器端也就是接收端的处理。