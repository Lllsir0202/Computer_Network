#pragma once
#include <windows.h>
#include <atomic>
#include <thread>
#include <iostream>
#include <fstream>

static std::string get_time_str()
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

class throughoutputMonitor
{
public:
    throughoutputMonitor() : totalBytes(0), stopEvent(CreateEvent(NULL, TRUE, FALSE, NULL)), isRunning(false)
    {
        out.open("./output.out", std::ios::out | std::ios::app);
    }

    ~throughoutputMonitor()
    {
        Stop();
        CloseHandle(stopEvent);
        out.close();
    }

    // 启动吞吐量监控线程
    void Start()
    {
        isRunning = true;
        monitorThread = std::thread(&throughoutputMonitor::MonitorThroughput, this);
    }

    // 停止吞吐量监控线程
    void Stop()
    {
        if (isRunning)
        {
            SetEvent(stopEvent);
            if (monitorThread.joinable())
            {
                monitorThread.join();
            }
            isRunning = false;
        }
    }

    // 更新传输的字节数
    void AddBytes(long long bytes)
    {
        totalBytes += bytes;
    }

    // 获取当前传输字节数
    long long GetTotalBytes() const
    {
        return totalBytes;
    }

private:
    // 吞吐量统计线程
    void MonitorThroughput()
    {
        long long previousBytes = 0;
        while (WaitForSingleObject(stopEvent, 1000) == WAIT_TIMEOUT)
        {
            long long currentBytes = totalBytes;
            long long throughput = currentBytes - previousBytes; // 计算每5秒吞吐量
            previousBytes = currentBytes;

            long long output = throughput;
            std::string log = "Throughoutput in " + get_time_str() + " is " + std::to_string(output) + " per 1 second ";
            out << log << std::endl;
        }
    }

    std::atomic<long long> totalBytes; // 使用原子操作确保线程安全
    HANDLE stopEvent;                  // 停止监控线程的事件
    std::thread monitorThread;         // 吞吐量监控线程
    bool isRunning;                    // 标记监控线程是否正在运行
    std::ofstream out;                 // 用于输出吞吐量
};