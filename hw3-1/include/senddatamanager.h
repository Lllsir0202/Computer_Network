#pragma once
#include "data.h"
#include <map>

/*
首先对于数据包我们主要需要考虑两种情况：
一种是作为发送端：需要的API有根据字节流生成数据包、开辟buffer储存发送且未确认的数据、
对于发送端，需要记录目前ACK的num，以及下一个发送的数据包的序列号
另一种是作为接收端，需要的API有解包、开辟buffer存储接收到的数据、生成ACK确认包、
对于接收端，需要记录当前的接收到的最大ACKnum，同时记录期待收到的下一个序列号
*/

// 在这里考虑后还是将发送管理和接收管理进行分开封装

// 这个类处理的是发送的数据包管理，主要需要：
// 根据给定的数据流封装数据包       ->      gen_package()
// 根据随机种子生成ISN(just called once)    ->      init_ISN() 
// 对发送的数据包进行存储       ->      seq2data(TYPE: std::map)
// 对给定的序列号进行确认       ->      verify()
// 更新新的序列号和确认号       ->
// 对ACK包进行解包，并进行确认      ->      solve_package()
class senddatamanager
{
public:
    // 构建析构函数，释放内存
    ~senddatamanager();
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
    uint8_t *get_package(uint8_t flag, uint8_t *raw, uint32_t windowsize, uint16_t datalen);

    // 这个函数用于基于原始数据解包package
    bool solve_package(uint8_t *pack);

    // 下面的函数用于获得私有变量
    uint32_t get_ISN() { return __ISN; };
    uint32_t get_ACK() { return __Acknum; };
    uint32_t get_SEQ() { return __Seqnum; };
    data *get_data(uint32_t acknum)
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

    // 记录目前已经确认的最大序列号
    uint32_t __Acknum;

    // 记录目前期待接收到的序列号
    uint32_t __Seqnum;

    // 记录数据buffer，这里记录序列号和对应的数据包 -> 已经解包后的
    // refer to unack package
    std::map<uint32_t, data *> seq2data;

    // 用于验证data内的数据求和是否为0，作为校验辅助函数    ->  only used in resolve ack package
    bool __verify_data(data *d);
};
