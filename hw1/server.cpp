#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <thread>
#include <map>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <fstream>

#define PORT 2205
#define thd_num 64

// 处在连接状态的socket
std::vector<SOCKET> connected_client;

// 记录已使用的user_name
std::map<SOCKET, std::string> user_names;

// 上锁
// std::mutex client_mtx;

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

// 封装一个函数输出服务器开启页面
void print_start()
{
    std::cout << R"(

                                                                                                     
 ,---.  ,------.,------.,--.   ,--.,------.,------.      ,---. ,--------. ,---.  ,------. ,--------. 
'   .-' |  .---'|  .--. '\  `.'  / |  .---'|  .--. '    '   .-''--.  .--'/  O  \ |  .--. ''--.  .--' 
`.  `-. |  `--, |  '--'.' \     /  |  `--, |  '--'.'    `.  `-.   |  |  |  .-.  ||  '--'.'   |  |    
.-'    ||  `---.|  |\  \   \   /   |  `---.|  |\  \     .-'    |  |  |  |  | |  ||  |\  \    |  |    
`-----' `------'`--' '--'   `-'    `------'`--' '--'    `-----'   `--'  `--' `--'`--' '--'   `--'    
                                                                                                     

)" << std::endl;
}

// 判断用户是否存在
bool ifexist(char *buffer)
{
    std::string tmp;
    std::string id(buffer);
    auto it = user_names.begin();
    while (it != user_names.end())
    {
        tmp = it->second;
        if (tmp == id)
        {
            return 1;
        }
        it++;
    }
    return 0;
}

// 实现登录
int login_in(SOCKET client_socket)
{
    char ver_buffer[100];
    int n = recv(client_socket, ver_buffer, 99, 0);
    ver_buffer[n] = '\0';
    // 如果有存在的用户名，那么返回0
    if (ifexist(ver_buffer))
    {
        return 0;
    }
    else
    {
        user_names.insert(std::make_pair(client_socket, ver_buffer));
        return 1;
    }
}

// 进行类模板重载，当类型为char* 或者 const char* 时，匹配
// 这样可以更好地适应string的.c_str()函数。
template <typename T>
typename std::enable_if<
    !std::is_same<T, char *>::value &&
        !std::is_same<T, const char *>::value,
    int>::type
broadcast(T *buffer, SOCKET client)
{
    // std::cout << "here is broadcast" << std::endl;
    // std::cout << "buffer len is " << strlen(buffer) << std::endl;
    // std::cout << "buffer is " << buffer << std::endl;
    std::string bye;
    // std::unique_lock<std::mutex> lock(client_mtx);
    {
        if (strlen(buffer) == 1 && buffer[0] == 'Q')
        {
            int tmp_i = 0;
            for (int i = 0; i < connected_client.size(); i++)
            {
                if (connected_client[i] == client)
                {
                    tmp_i = i;
                }
                else
                {
                    bye = user_names[client] + "退出聊天室";
                    send(connected_client[i], bye.c_str(), strlen(bye.c_str()), 0);
                }
            }

            connected_client.erase(connected_client.begin() + tmp_i);
            return 0;
        }
        for (int i = 0; i < connected_client.size(); i++)
        {
            if (connected_client[i] != client)
            {
                send(connected_client[i], buffer, strlen(buffer), 0);
            }
        }
    }
    return 1;
}

DWORD WINAPI handle_client(LPVOID client)
{
    SOCKET client_socket = (SOCKET)(LPVOID)client;
    char rdata[1024];
    while (1)
    {
        if (login_in(client_socket))
        {
            std::cout << user_names[client_socket] << "已经登录服务器" << get_time_str() << std::endl;
            // 输出流
            std::ofstream log_file;
            // 以添加方式写
            log_file.open("./worker.log", std::ios::app);
            if (!log_file.is_open())
            {
                std::cout << "CANNOT OPEN LOG" << std::endl;
            }
            log_file << user_names[client_socket] << "已经登录服务器" << get_time_str() << std::endl;
            std::string login = user_names[client_socket] + "进入聊天室";
            broadcast(login.c_str(), client_socket);
            break;
        }
        send(client_socket, "N", 1, 0);
    }
    // 证明可以使用
    send(client_socket, "T", 1, 0);
    // 装入连接的vector
    connected_client.emplace_back(client_socket);

    while (1)
    {
        int n = recv(client_socket, rdata, 1023, 0);
        rdata[n] = '\0';
        if (n <= 0)
        {
            std::cout << user_names[client_socket] << "   意外中断连接" << std::endl;
            break;
        }
        if (!broadcast(rdata, client_socket))
        {
            std::cout << user_names[client_socket] << "退出登录" << get_time_str() << std::endl;
            user_names.erase(client_socket);
            break;
        }
        else
        {
            // 输出流
            std::ofstream log_file;
            // 以添加方式写
            log_file.open("./worker.log", std::ios::app);
            if (!log_file.is_open())
            {
                std::cout << "CANNOT OPEN LOG" << std::endl;
            }
            log_file << rdata << std::endl;
            std::cout << rdata << std::endl;
        }
    }
    closesocket(client_socket);
    user_names.erase(client_socket);
    return 0;
}
/*
// 将字符串解析为 std::tm 类型的时间结构
std::tm parseDateString(const std::string& dateStr) {
   std::tm tm = {};
   std::istringstream ss(dateStr);
   ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");  // 假设日期格式为 "YYYY-MM-DD"
   return tm;
}

// 计算日期到现在的时间差（天数）
int calculateDaysDifference(const std::string& dateStr) {
   std::tm tm = parseDateString(dateStr);

   // 将解析到的时间转换为 time_t 类型
   std::time_t inputTime = std::mktime(&tm);

   // 获取当前时间
   auto now = std::chrono::system_clock::now();
   std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

   // 计算时间差（单位：秒）
   long long secondsDifference = std::difftime(currentTime, inputTime);

   // 将秒数转换为天数
   int daysDifference = secondsDifference / (60 * 60 * 24);

   return daysDifference;
}

//对于日期过久的日志进行清理，这里的实现没有真正实现，仅作为实际应用的一个模拟
//这种清理日志的方式会很慢，感觉可以重新实现一个专门处理日志的程序效率更好。
void checklog(int setmaxday)
{
   //输入流
   std::ifstream input_file("../worker.log");
   if (!input_file) {
       std::cerr << "Error opening file for reading." << std::endl;
       return;
   }
   std::vector<std::string> lines;
   std::string line;

   // 读取文件内容到一个 vector
   while (std::getline(input_file, line)) {
        if
   }

   input_file.close();

   // 将修改后的内容写回文件
   std::ofstream output_file("../worker.log");
   if (!output_file) {
       std::cerr << "Error opening file for writing." << std::endl;
       return;
   }

   for (const auto& l : lines) {
       output_file << l << std::endl; // 写入保留的行
   }

   output_file.close();

}
*/

// 封装一个函数，输出服务器目前的所有log
void print_log()
{
    // 输入流
    std::ifstream input_file("./worker.log");
    if (!input_file)
    {
        std::cerr << "Error opening file for reading." << std::endl;
        return;
    }
    std::vector<std::string> lines;
    std::string line;

    // 读取文件内容到一个 vector
    while (std::getline(input_file, line))
    {
        std::cout << line << std::endl;
    }

    input_file.close();
}

DWORD WINAPI handle_server(LPVOID server)
{
    SOCKET server_socket = (SOCKET)(LPVOID)server;
    std::string buffer;
    while (1)
    {
        std::cin >> buffer;
        if (buffer == "Q")
        {
            break;
        }
    }
    closesocket(server_socket);
    return 0;
}

int main()
{
    print_start();
    SetConsoleCP(65001);
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return 1;
    }

    // 创建服务器socket
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 设置ip、端口等
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    int addr_len = sizeof(server_addr);

    // 绑定端口、ip
    if (bind(server_socket, (sockaddr *)&server_addr, addr_len) == SOCKET_ERROR)
    {
        perror("SERVER FAIL to bind");
        closesocket(server_socket);
        return -1;
    }

    if (listen(server_socket, thd_num) == SOCKET_ERROR)
    {
        perror("SERVER FAIL to listen");
        closesocket(server_socket);
        return -2;
    }

    // std::cout << "started" << std::endl;
    // std::cout << server_addr.sin_addr.s_addr << ":" << server_addr.sin_port << std::endl;
    DWORD ThreadID;
    HANDLE sThread = CreateThread(NULL, NULL, handle_server, LPVOID(server_socket), 0, &ThreadID);
    DWORD exitCode;

    while (1)
    {
        if (GetExitCodeThread(sThread, &exitCode))
        {
            if (!(exitCode == STILL_ACTIVE))
            {
                break;
            }
        }
        // 每次接收到一个连接，创建一个client_socket
        sockaddr_in client_addr;
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(PORT);
        client_addr.sin_addr.s_addr = INADDR_ANY;
        int client_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_len);
        std::cout << "ACCEPT A NEW CONNECTION , check " << std::endl;
        DWORD dwThreadID;
        // 创建线程
        HANDLE hThread = CreateThread(NULL, NULL, handle_client, LPVOID(client_socket), 0, &dwThreadID);
        // 关闭句柄
        CloseHandle(hThread);
    }
    closesocket(server_socket);
    WSACleanup();
    return 0;
}