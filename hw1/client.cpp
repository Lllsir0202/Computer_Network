#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <thread>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <mutex>

#define PORT 2205
#define num 10

std::string user_name;
std::string wbuffer;
char rbuffer[1024];
std::mutex mtx;

// 实现一个函数来实现时间
std::string get_time_str()
{
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 转换为 time_t 类型，方便格式化
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // 格式化并输出
    std::tm *local_time = std::localtime(&now_time);
    // 创建字符串流用于格式化
    std::ostringstream time_stream;

    // 使用 std::put_time 来格式化时间为字符串
    time_stream << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");

    // 将流中的内容转换为字符串
    std::string time_str = time_stream.str();
    time_str = "     <" + time_str + ">";
    return time_str;
}

// 封装输出进入艺术字函数
void print_art_hello()
{
    std::cout << R"(
        ,----,                                                                                  
      ,/   .`|                                         ,----..       ,----..             ____   
    ,`   .'  :           ,--,         ,-.,-.----.     /   /   \     /   /   \          ,'  , `. 
  ;    ;     /         ,--.'|     ,--/ /|\    /  \   /   .     :   /   .     :      ,-+-,.' _ | 
.'___,/    ,'          |  | :   ,--. :/ |;   :    \ .   /   ;.  \ .   /   ;.  \  ,-+-. ;   , || 
|    :     |           :  : '   :  : ' / |   | .\ :.   ;   /  ` ;.   ;   /  ` ; ,--.'|'   |  ;| 
;    |.';  ;  ,--.--.  |  ' |   |  '  /  .   : |: |;   |  ; \ ; |;   |  ; \ ; ||   |  ,', |  ': 
`----'  |  | /       \ '  | |   '  |  :  |   |  \ :|   :  | ; | '|   :  | ; | '|   | /  | |  || 
    '   :  ;.--.  .-. ||  | :   |  |   \ |   : .  /.   |  ' ' ' :.   |  ' ' ' :'   | :  | :  |, 
    |   |  ' \__\/: . .'  : |__ '  : |. \;   | |  \'   ;  \; /  |'   ;  \; /  |;   . |  ; |--'  
    '   :  | ," .--.; ||  | '.'||  | ' \ \   | ;\  \\   \  ',  /  \   \  ',  / |   : |  | ,     
    ;   |.' /  /  ,.  |;  :    ;'  : |--':   ' | \.' ;   :    /    ;   :    /  |   : '  |/      
    '---'  ;  :   .'   \  ,   / ;  |,'   :   : :-'    \   \ .'      \   \ .'   ;   | |`-'       
           |  ,     .-./---`-'  '--'     |   |.'       `---`         `---`     |   ;/           
            `--`---'                     `---'                                 '---'            
    )" << std::endl;
    std::cout << get_time_str() << std::endl;
    std::cout << std::endl;
}

void print(std::string str)
{
    // 打印顶边框
    int length = str.length();     // 获取消息长度
    int borderLength = length + 4; // 边框长度比消息多4个字符 (左右各一个空格和一个 *)
    for (int i = 0; i < borderLength; ++i)
    {
        std::cout << "*";
    }
    std::cout << std::endl;

    // 打印包含消息的行
    std::cout << "* " << "\e[1;31m" << str << "\e[0m" << " *" << std::endl;

    // 打印底边框
    for (int i = 0; i < borderLength; ++i)
    {
        std::cout << "*";
    }
    std::cout << std::endl;
}

// 封装提示信息函数
void print_info()
{
    std::string str1 = "输入\\quit|\\QUIT表示退出，退出前需要二次验证";
    std::string str2 = "在服务器上的用户名不允许同一时间出现重复";
    print(str1);
    print(str2);
}

// 封装输出退出艺术字函数
void print_art_bye()
{
    std::cout << R"(

                                                                   
                                                                   
    ,---,.                 ,---,.    ,---,.                 ,---,. 
  ,'  .'  \       ,---,  ,'  .' |  ,'  .'  \       ,---,  ,'  .' | 
,---.' .' |      /_ ./|,---.'   |,---.' .' |      /_ ./|,---.'   | 
|   |  |: |,---, |  ' :|   |   .'|   |  |: |,---, |  ' :|   |   .' 
:   :  :  /___/ \.  : |:   :  |-,:   :  :  /___/ \.  : |:   :  |-, 
:   |    ; .  \  \ ,' ':   |  ;/|:   |    ; .  \  \ ,' ':   |  ;/| 
|   :     \ \  ;  `  ,'|   :   .'|   :     \ \  ;  `  ,'|   :   .' 
|   |   . |  \  \    ' |   |  |-,|   |   . |  \  \    ' |   |  |-, 
'   :  '; |   '  \   | '   :  ;/|'   :  '; |   '  \   | '   :  ;/| 
|   |  | ;     \  ;  ; |   |    \|   |  | ;     \  ;  ; |   |    \ 
|   :   /       :  \  \|   :   .'|   :   /       :  \  \|   :   .' 
|   | ,'         \  ' ;|   | ,'  |   | ,'         \  ' ;|   | ,'   
`----'            `--` `----'    `----'            `--` `----'     
                                                                   

)" << std::endl;
    std::cout << get_time_str() << std::endl;
    std::cout << std::endl;
}

// 封装一个函数实现string类型是否全为空格的判断
bool is_all_space(std::string str)
{
    return std::all_of(str.begin(), str.end(), [](char c)
                       { return c == ' '; });
}

// 编写验证函数，证明用户名未被使用
int verify(std::string name, SOCKET server_socket)
{
    send(server_socket, name.c_str(), strlen(name.c_str()), 0);
    std::cout << "Verifying if username has been used" << std::endl;
    std::cout << "please wait" << std::endl;
    char ver[2];
    int n = recv(server_socket, ver, 1, 0);
    if (n <= 0)
    {
        return -1;
    }
    if (ver[0] == 'T' && n == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// 编写登录函数
void login_in(SOCKET server_socket)
{
    std::cout << "请输入你的用户名: " << std::endl;
    std::cin >> user_name;
    while (1)
    {
        if (verify(user_name, server_socket))
        {
            break;
        }
        else
        {
            std::cout << "用户名正在被使用，请重新输入" << std::endl;
            std::cin >> user_name;
        }
    }
    std::cout << "你好，" << user_name << std::endl;
}

// 编写退出函数
int login_out()
{
    std::cout << "再见，" << user_name << std::endl;
    return 1;
}

// 开两个线程，一个发送，一个接收，这样实现更好。
// 由于主线程可以执行我们的发送进程，所以只需要创建一个子线程就可以了。

DWORD WINAPI recv_socket(LPVOID client)
{
    SOCKET server_socket = (SOCKET)(LPVOID)client;
    std::cout << "已开始接受信息" << std::endl;

    while (1)
    {
        int n = recv(server_socket, rbuffer, 1023, 0);
        if (n == SOCKET_ERROR)
        {
            break;
        }
        if (n == 0)
        {
            // std::cout << "socket closed" << std::endl;
            break;
        }
        rbuffer[n] = '\0';
        for (int i = 0; i < user_name.length() + 2; i++)
        {
            // 回退username: ，使得输出正常
            std::cout << "\b";
        }
        std::cout << rbuffer << std::endl;
        std::cout << "\033[32m" << user_name << "\033[0m" << ": ";
    }
    return 0;
}

int main()
{
    print_art_hello();
    print_info();
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return 1;
    }
    // 创建连接
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    // 绑定协议，类型，ip
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        perror("client FAILED TO CONNECT");
        std::cerr << "Failed to connect: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        system("pause");
        return -1;
    }

    // 如果连接上了，那么可以login in了
    login_in(client_socket);

    // 进入循环，发送信息，当不为'q'或者'Q'时，传到服务器
    // 创建一个子进程，用来接受信息
    DWORD dwThreadID;
    // 创建线程
    HANDLE hThread = CreateThread(NULL, NULL, recv_socket, LPVOID(client_socket), 0, &dwThreadID);
    // 关闭句柄
    CloseHandle(hThread);
    Sleep(20);
    bool flag = 0;
    while (1)
    {
        std::unique_lock<std::mutex> lock(mtx);
        {
            if (flag)
            {
                std::cout << "\033[32m" << user_name << "\033[0m" << ": ";
            }
        }
        // 输入内容
        std::getline(std::cin, wbuffer, '\n');
        // 这里不需要再次输出输入信息了。
        // std::cout << wbuffer << std::endl;
        if (wbuffer == "\\quit" || wbuffer == "\\QUIT")
        {

            std::cout << "是否确定退出？(Q|q表示退出)" << std::endl;
            std::getline(std::cin, wbuffer, '\n');
            std::cout << user_name << ": " << std::endl;
            if (wbuffer == "Q" || wbuffer == "q")
            {
                send(client_socket, "Q", 1, 0);
                login_out();
                break;
            }
            else
            {
                // 如果连接上了，那么可以login in了
                login_in(client_socket);
                continue;
            }
        }
        if (!is_all_space(wbuffer))
        {
            wbuffer = user_name + ": " + wbuffer + get_time_str();
            if (send(client_socket, wbuffer.c_str(), strlen(wbuffer.c_str()), 0) == SOCKET_ERROR)
            {
                bool f = 1;
                std::cout << "服务器断开连接" << std::endl;
                for (int i = 0; i < num; i++)
                {
                    std::cout << "尝试重连" << i + 1 << "/" << num << std::endl;
                    // 每一秒重连一次
                    Sleep(2000);
                    client_socket = socket(AF_INET, SOCK_STREAM, 0);
                    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
                    {
                        int error_code = WSAGetLastError();
                        std::cerr << "Connect failed with error: " << error_code << std::endl;
                        std::cout << "重连失败" << std::endl;
                    }
                    else
                    {
                        f = 0;
                        std::cout << "重连成功" << std::endl;
                        // 重连后，重新登录
                        login_in(client_socket);
                        Sleep(2);
                        DWORD dwThreadID;
                        // 创建线程
                        HANDLE hThread = CreateThread(NULL, NULL, recv_socket, LPVOID(client_socket), 0, &dwThreadID);
                        flag = 0;
                        // 关闭句柄
                        CloseHandle(hThread);
                        break;
                    }
                }
                if (f)
                {
                    break;
                }
            }
        }
        else if (flag)
        {
            std::cout << "不允许发送空内容(null is not promitted)" << std::endl;
        }
        flag = 1;
        wbuffer = "";
    }

    // 关闭socket
    closesocket(client_socket);

    // 这里的输出应该在这里写
    print_art_bye();

    WSACleanup();
    // 在关闭后3s关闭窗口
    Sleep(3000);
    return 0;
}
