#include "sender.h"
#include "../include/throughoutput.h"
#define MAXSIZE 10000
#define READSIZE 8000

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
    struct sockaddr_in send_addr;
    memset(&send_addr, 0, sizeof(send_addr));

    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    send_addr.sin_port = htons(10088);

    sender client(sendsocket, "127.0.0.1", 11111, MAXSIZE);

    if (bind(sendsocket, (const sockaddr *)&send_addr, sizeof(send_addr)) == SOCKET_ERROR)
    {
        std::string log = "SendSocket Failed to bind ";
        client.get_sdm()->add_log(log);
        // 在类中的析构函数处理了
        // closesocket(sendsocket);
        // WSACleanup();
        return 1;
    }

    uint8_t buff[READSIZE];

    bool flag;
    flag = client.Connect();
    // 开始连接
    while (!flag)
    {
        std::cout << "HandShake failed " << std::endl;
        // closesocket(sendsocket);
        // WSACleanup();
        return 1;
    }

    // 设定窗口大小
    int input_winsize = 0;
    std::cout << "Please set cwnd size " << std::endl;
    std::cin >> input_winsize;
    while (input_winsize <= 0)
    {
        std::cout << "Invalid input , please retry!" << std::endl;
        std::cin >> input_winsize;
    }
    client.set_cwnd(input_winsize);
    std::string win_log = "Succeed to set cwnd size = " + std::to_string(input_winsize);
    client.get_sdm()->add_log(win_log);

    std::string filename;
    throughoutputMonitor output;
    while (1)
    {
        Sleep(100);
        int file = sendfile();
        if (file == 5)
        {
            break;
        }
        std::string filepath;
        if (file <= 4)
        {
            client.Start();
        }
        switch (file)
        {
        case 1:
        {
            filepath = "./1.jpg";
            filename = "1.jpg";
            client.Sendto((uint8_t *)filename.c_str(), filename.length(), START);
            std::ifstream filein;
            filein.open(filepath, std::ios::binary);
            if (!filein.is_open())
            {
                std::cout << "Fail to open " + filepath + "\n";
            }
            output.Start();
            while (filein.read((char *)buff, READSIZE))
            {
                output.AddBytes(READSIZE);
                client.Sendto(buff, READSIZE, TRANS);
            }
            output.AddBytes(filein.gcount());
            client.Sendto(buff, filein.gcount(), TRANS);
            output.Stop();
            filein.close();

            std::string log = "File " + filepath + " succeed to trans ";
            client.get_sdm()->add_log(log);
        }
        break;
        case 2:
        {
            filepath = "./2.jpg";
            filename = "2.jpg";
            client.Sendto((uint8_t *)filename.c_str(), filename.length(), START);
            std::ifstream filein;
            filein.open(filepath, std::ios::binary);
            if (!filein.is_open())
            {
                std::cout << "Fail to open " + filepath + "\n";
            }
            output.Start();
            while (filein.read((char *)buff, READSIZE))
            {
                output.AddBytes(READSIZE);
                client.Sendto(buff, READSIZE, TRANS);
            }
            output.AddBytes(filein.gcount());
            client.Sendto(buff, filein.gcount(), TRANS);
            output.Stop();
            filein.close();

            std::string log = "File " + filepath + " succeed to trans ";
            client.get_sdm()->add_log(log);
        }
        break;
        case 3:
        {
            filepath = "./3.jpg";
            filename = "3.jpg";
            client.Sendto((uint8_t *)filename.c_str(), filename.length(), START);
            std::ifstream filein;
            filein.open(filepath, std::ios::binary);
            if (!filein.is_open())
            {
                std::cout << "Fail to open " + filepath + "\n";
            }
            output.Start();
            while (filein.read((char *)buff, READSIZE))
            {
                output.AddBytes(READSIZE);
                client.Sendto(buff, READSIZE, TRANS);
            }
            output.AddBytes(filein.gcount());
            client.Sendto(buff, filein.gcount(), TRANS);
            output.Stop();
            filein.close();

            std::string log = "File " + filepath + " succeed to trans ";
            client.get_sdm()->add_log(log);
        }
        break;
        case 4:
        {
            filepath = "./helloworld.txt";
            filename = "helloworld.txt";
            client.Sendto((uint8_t *)filename.c_str(), filename.length(), START);
            std::ifstream filein;
            filein.open(filepath, std::ios::binary);
            if (!filein.is_open())
            {
                std::cout << "Fail to open " + filepath + "\n";
            }
            output.Start();
            while (filein.read((char *)buff, READSIZE))
            {
                output.AddBytes(READSIZE);
                client.Sendto(buff, READSIZE, TRANS);
            }
            output.AddBytes(filein.gcount());
            client.Sendto(buff, filein.gcount(), TRANS);
            output.Stop();
            filein.close();

            std::string log = "File " + filepath + " succeed to trans ";
            client.get_sdm()->add_log(log);
        }
        break;
        default:
            break;
        }
        // Sleep(100);
        Sleep(100);
        if (file <= 4)
        {
            client.Stop();
        }
    }
    // 断开
    client.Disconnect();
    // 关闭socket
    // closesocket(sendsocket);

    // WSACleanup();
    return 0;
}