#include "senddatamanager.h"
#include <random>
#include <cassert>
#include <chrono>
#include <thread>

// 由于这里我们记录的序列号其实是收到的确认号-1，
// 所以这里我们对于传入的acknum，可以这样处理
// 我们记录的映射是原序列号+datalen

senddatamanager::senddatamanager()
{
    logout.open("./send.log", std::ios::app);
    if (!logout.is_open())
    {
        std::cout << "CANNOT OPEN LOG " << std::endl;
    }
}

senddatamanager::~senddatamanager()
{
    for (auto it = seq2data.begin(); it != seq2data.end(); it++)
    {
        delete it->second;
    }
}

void senddatamanager::init_ISN()
{
    // 这里使用C++11引入的新库进行获取
    // 创建随机数引擎，使用std::random_device进行种子初始化
    std::random_device rd;
    std::mt19937 gen(rd());

    // 生成一个在1-65535范围内的随机起始序列号
    uint32_t min = 1, max = 65535;
    std::uniform_int_distribution<> dis(min, max);

    // 生成一个随机数
    uint32_t random_number = dis(gen);

    // 这其实就是起始的序列号
    __ISN = random_number;
    __Seqnum = __ISN;
}

bool senddatamanager::verify(uint32_t acknum)
{
    if (seq2data.find(acknum) == seq2data.end())
    {
        // 这里采取三次快速重传，添加变量记录
        __cnt++;
        return true;
    }
    else
    {
        auto d = seq2data[acknum];
        return __verify_data(d);
    }
}

bool senddatamanager::acknowledged(uint32_t acknum)
{
    if (seq2data.find(acknum) == seq2data.end())
    {
        // 如果确认号是当前包的上一个数据包，
        // std::cout << "here" << std::endl;
        // std::cout << acknum << std::endl;
        // if (!seq2data.empty())
        // {
        //     std::cout << seq2data.begin()->first << std::endl;
        // }
        while (seq2data.begin()->first < acknum)
        {
            seq2data.erase(seq2data.begin()->first);
        }
    }
    else
    {
        auto d = seq2data[acknum];
        while (seq2data.begin()->first < acknum)
        {
            seq2data.erase(seq2data.begin()->first);
        }
        seq2data.erase(acknum);
        std::cout << "current size is " << seq2data.size() << std::endl;
        // 更新下一个确认号为对方发送的渴望得到的
        // 更新下一个序列号为ack+datalen
        //__Seqnum = d->get_ack() + d->get_datalen();
        // delete d;
    }
    return true;
}

bool senddatamanager::__verify_data(data *d)
{
    uint16_t sum = 0;

    uint8_t __flag = d->get_flag();
    uint32_t __ACK = d->get_ack();
    uint32_t __SEQ = d->get_seq();
    uint32_t __windowsize = d->get_windowsize();
    uint16_t __datalen = d->get_datalen();
    uint8_t *__d = d->get_data();
    uint8_t __checksum = d->get_checksum();

    // 计算状态位
    sum += __flag;

    // 计算ACK
    sum += __ACK & EIGHTSIZE;         // 第一个字节
    sum += (__ACK >> 8) & EIGHTSIZE;  // 第二个字节
    sum += (__ACK >> 16) & EIGHTSIZE; // 第三个字节
    sum += (__ACK >> 24) & EIGHTSIZE; // 第四个字节

    // 计算SEQ
    sum += __SEQ & EIGHTSIZE;         // 第一个字节
    sum += (__SEQ >> 8) & EIGHTSIZE;  // 第二个字节
    sum += (__SEQ >> 16) & EIGHTSIZE; // 第三个字节
    sum += (__SEQ >> 24) & EIGHTSIZE; // 第四个字节

    // 计算窗体大小
    sum += __windowsize & EIGHTSIZE;         // 第一个字节
    sum += (__windowsize >> 8) & EIGHTSIZE;  // 第二个字节
    sum += (__windowsize >> 16) & EIGHTSIZE; // 第三个字节
    sum += (__windowsize >> 24) & EIGHTSIZE; // 第四个字节

    // 计算数据长度
    sum += __datalen & EIGHTSIZE;        // 第一个字节
    sum += (__datalen >> 8) & EIGHTSIZE; // 第二个字节

    // 计算数据部分的校验和
    // 仅在数据包时处理
    if ((__flag & TRANS) == TRANS || (__flag & START) == START)
    {
        for (int i = 0; i < __datalen; i++)
        {
            sum += __d[i];
        }
    }

    // 对其他部分计算然后得到八位数值，再加上校验和，应该是等于0
    while (sum > EIGHTSIZE)
    {
        sum = (sum & EIGHTSIZE) + (sum >> 8);
    }
    sum += __checksum;
    return ((sum & EIGHTSIZE) == EIGHTSIZE);
}

uint8_t *senddatamanager::get_package(uint8_t flag, uint8_t *raw, uint32_t windowsize, uint16_t datalen, bool tag)
{
    // 首先new一个data对象
    auto d = new data();
    d->init(flag, __Acknum, __Seqnum, windowsize, datalen, raw);
    std::cout << "Seqnum is " << __Seqnum << std::endl;
    std::cout << "Current seq2data size is " << seq2data.size() << std::endl;
    // 在生成对应的后，我们将seq += datalen，于是接收到的确认号其实就是seq+1
    if (tag)
    {
        __Seqnum += datalen;
        seq2data[__Seqnum] = d;
    }

    // 当传输的是数据包的时候，需要记录
    // if ((flag & TRANS) == TRANS || (flag & START) == START)
    //{ // 保存Seqnum对应的数据包
    // 这里可以把握手挥手也存储了，这样便于重传确认
    //}
    return d->gen_data(raw);
}

bool senddatamanager::solve_package(uint8_t *pack, int flag)
{
    auto d = new data;
    d->regen_data(pack);
    std::cout << "accept ack is " << d->get_ack() << std::endl;
    // 对于收到的包进行解包，然后进行差错检验
    if (verify(d->get_ack()))
    { // 标识差错检测通过
        switch (flag)
        {
        case 0:
        { // 确认这是一个确认数据包
            assert((d->get_flag() & ACK) == ACK);
            // 这是对收到的接受包进行确认，从缓冲区移去
            bool flag = acknowledged(d->get_ack());
            if (!flag)
            {
                __Acknum = d->get_seq() + d->get_datalen();
                delete d;
                return false;
            }
            __Acknum = d->get_seq() + d->get_datalen();
            std::string log = "Acknowledge package seqnum " + std::to_string(d->get_ack());
            add_log(log);
            // 确认之后这个包就无用了
            delete d;
            return true;
        }
        break;
        // 第二次握手
        case 1:
        {
            // 确认收到的包SYNC和ACK被置位
            assert((d->get_flag() & (SYNC | ACK)) == (SYNC | ACK));
            // 这里其实是对第一次握手的数据包的确认
            acknowledged(d->get_ack());
            __Acknum = d->get_seq() + d->get_datalen();
            // 第二次握手应该是第一次确认号获取，但这里应该已经有了处理
            std::string log = "Acknowledge Second Handshake";
            add_log(log);
            delete d;
            return true;
        }
        break;
        // 第二次挥手
        case 2:
        {
            assert((d->get_flag() & (FIN | ACK)) == (FIN | ACK));
            acknowledged(d->get_ack());
            __Acknum = d->get_seq() + d->get_datalen();
            std::string log = "Acknowledge Second Wave";
            add_log(log);
            delete d;
            return true;
        }
        break;
        // 第四次挥手
        // 第四次挥手
        case 3:
        {
            assert((d->get_flag() & ACK) == ACK);
            acknowledged(d->get_ack());
            __Acknum = d->get_seq() + d->get_datalen();
            std::string log = "Acknowledge Fourth Wave";
            add_log(log);
            delete d;
            return true;
        }
        break;
        default:
        {
            delete d;
            std::cout << "Invalid flag " << std::endl;
            return false;
        }
        break;
        }
    }
    else
    {
        // 这里应该进行重传了，但现在还没有实现
        std::cout << "here" << std::endl;
        return false;
        // 在这里返回false，在外面封装处理函数
    }
    return true;
}

void senddatamanager::add_log(std::string log)
{
    logout << log << std::endl;
    std::cout << log << std::endl;
}