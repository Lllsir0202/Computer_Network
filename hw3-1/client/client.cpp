#include "sender.h"
#define MAXSIZE 2000
#define READSIZE 1500

int sendfile()
{
    std::cout << "Please choose file among following files " << std::endl;

    std::cout << "1.jpg" << std::endl;
    std::cout << "2.jpg" << std::endl;
    std::cout << "3.jpg" << std::endl;
    std::cout << "helloworld.txt" << std::endl;

    std::string filename;
    while (1)
    {
        std::cin >> filename;
        if (filename == "1.jpg")
        {
            return 1;
        }
        else if (filename == "2.jpg")
        {
            return 2;
        }
        else if (filename == "3.jpg")
        {
            return 3;
        }
        else if (filename == "helloworld.txt")
        {
            return 4;
        }
        else if (filename == "quit")
        {
            return 5;
        }
        else
        {
            std::cout << "Invalid Input , please rery " << std::endl;
            continue;
        }
    }
}

int main()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return 1;
    }
    // std::cout<<"Please input ip and port sendto " <<std::endl;
    SOCKET sendsocket = socket(AF_INET, SOCK_DGRAM, 0);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500; // 500微秒超时
    if (setsockopt(sendsocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        perror("setsockopt failed");
        closesocket(sendsocket);
        return -1;
    }
    struct sockaddr_in send_addr;
    memset(&send_addr, 0, sizeof(send_addr));

    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    send_addr.sin_port = htons(10088);

    sender client(sendsocket, "127.0.0.1", 10086, MAXSIZE);

    Sleep(100);

    uint8_t buff[READSIZE];

    // 开始连接
    client.Connect();

    while (1)
    {
        int file = sendfile();
        if (file == 5)
        {
            break;
        }
        std::string filepath;

        switch (file)
        {
        case 1:
        {
            filepath = "./1.jpg";
            std::ifstream filein;
            filein.open(filepath, std::ios::binary);
            if (!filein.is_open())
            {
                std::cout << "Fail to open " + filepath + "\n";
            }
            while (filein.read((char *)buff, READSIZE))
            {
                client.Sendto(buff, READSIZE);
            }
            client.Sendto(buff, filein.gcount());
            filein.close();

            std::string log = "File " + filepath + " succeed to trans ";
            client.get_sdm()->add_log(log);
        }
        break;
        case 2:
        {
            filepath = "./2.jpg";
            std::ifstream filein;
            filein.open(filepath, std::ios::binary);
            if (!filein.is_open())
            {
                std::cout << "Fail to open " + filepath + "\n";
            }
            while (filein.read((char *)buff, READSIZE))
            {
                client.Sendto(buff, READSIZE);
            }
            client.Sendto(buff, filein.gcount());
            filein.close();

            std::string log = "File " + filepath + " succeed to trans ";
            client.get_sdm()->add_log(log);
        }
        break;
        case 3:
        {
            filepath = "./3.jpg";
            std::ifstream filein;
            filein.open(filepath, std::ios::binary);
            if (!filein.is_open())
            {
                std::cout << "Fail to open " + filepath + "\n";
            }
            while (filein.read((char *)buff, READSIZE))
            {
                client.Sendto(buff, READSIZE);
            }
            client.Sendto(buff, filein.gcount());
            filein.close();

            std::string log = "File " + filepath + " succeed to trans ";
            client.get_sdm()->add_log(log);
        }
        break;
        case 4:
        {
            filepath = "./helloworld.txt";
            std::ifstream filein;
            filein.open(filepath, std::ios::binary);
            if (!filein.is_open())
            {
                std::cout << "Fail to open " + filepath + "\n";
            }
            while (filein.read((char *)buff, READSIZE))
            {
                client.Sendto(buff, READSIZE);
            }
            client.Sendto(buff, filein.gcount());
            filein.close();

            std::string log = "File " + filepath + " succeed to trans ";
            client.get_sdm()->add_log(log);
        }
        break;
        default:
            break;
        }
    }

    // 断开
    client.Disconnect();
    // 关闭socket
    closesocket(sendsocket);

    WSACleanup();
    return 0;
}