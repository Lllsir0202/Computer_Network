#include "../recv/recver.h"
#define MAXSIZE 10000
#define READSIZE 8000

int main()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return 1;
    }
    // std::cout<<"Please input ip and port sendto " <<std::endl;
    SOCKET recvsocket = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in recv_addr;
    memset(&recv_addr, 0, sizeof(recv_addr));

    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    recv_addr.sin_port = htons(11111);

    recver server(recvsocket, "127.0.0.1", 10088, MAXSIZE);

    if (bind(recvsocket, (const sockaddr *)&recv_addr, sizeof(recv_addr)) == SOCKET_ERROR)
    {
        std::string log = "SendSocket Failed to bind ";
        server.get_rdm()->add_log(log);
        // closesocket(recvsocket);
        // WSACleanup();
        return 1;
    }

    uint8_t buff[READSIZE];

    std::string log = "Start to accept client ";
    server.get_rdm()->add_log(log);

    // 开始连接
    if (!server.Connect())
    {
        std::cout << "Timeout " << std::endl;
        // closesocket(recvsocket);
        return 1;
    }

    log = "Succeed to connect ";
    server.get_rdm()->add_log(log);
    server.Recvfrom();

    // 断开在接收到Recvfrom中接收到FIN时处理
    // server.Disconnect();
    // 关闭socket
    // closesocket(recvsocket);

    // WSACleanup();
    return 0;
}