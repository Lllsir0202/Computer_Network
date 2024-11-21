#pragma once
#include "data.h"
#include <map>

// 这个类封装的是对于数据包的处理，这里需要记录初始序列号，已经ACK的序列号，期待接收到的下一个序列号，
// 以及开辟缓冲区记录所有未ACK的数据包，并对接收到的数据包做校验
class datamanager
{
public:
    // 使用随机数生成ISN
    void init_ISN();

    // 对上层传入的字节流数据包进行校验
    // 用于差错检验
    // 对给定的acknum进行一个检验
    bool verify(uint32_t acknum);

    // 对某一个数据进行确认
    void acquire(uint32_t acknum);

    // 还需要封装函数对ACK数据包进行处理，以及计算出确认序列号

    // 这个函数用于基于原始数据生成package
    uint8_t *get_package(uint8_t *raw);

    // 这个函数用于基于原始数据解包package
    void solve_package(uint8_t *pack);

    // 下面的函数用于获得私有变量
    uint32_t get_ISN() { return __ISN; };
    uint32_t get_ACK() { return __Acknum; };
    uint32_t get_SRQ() { return __Seqnum; };
    data* get_data(uint32_t acknum)
    {
        if (seq2data.find(acknum) != seq2data.end())
        {
            return seq2data[acknum];
        }
        else
        {
            std::string str = "ERROR: Invalid acknum " + std::to_string(acknum);
            perror(str.c_str());
        }
    }

private:
    // 记录初始序列号
    uint32_t __ISN;

    // 记录目前确认的序列号
    uint32_t __Acknum;

    // 记录目前期待接收到的序列号
    uint32_t __Seqnum;

    // 记录数据buffer，这里记录序列号和对应的数据包 -> 已经解包后的
    // refer to unack package
    std::map<uint32_t, data*> seq2data;

    // 用于验证data内的数据求和是否为0，作为校验辅助函数
    bool __verify_data(data* d);
};