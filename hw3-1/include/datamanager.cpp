#pragma once
#include "datamanager.h"
#include <random>

void datamanager::init_ISN()
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

    __ISN = random_number;
}

bool datamanager::verify(uint32_t acknum)
{
    if (seq2data.find(acknum) == seq2data.end())
    {
        std::string error = "ERROR: Package preACKED ";
        perror(error.c_str());
    }
    else
    {
        auto d = seq2data[acknum];
        return __verify_data(d);
    }
}

void datamanager::acquire(uint32_t acknum)
{
    if(seq2data.find(acknum) == seq2data.end())
    {
        std::string error = "ERROR: Cannot find data to be acquired ";
        perror(error.c_str());
    }
    else
    {
        auto d = seq2data[acknum];
        seq2data.erase(acknum);
        delete d;
    }
}

bool datamanager::__verify_data(data* d)
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
    for (int i = 0; i < __datalen; i++)
    {
        sum += __d[i];
    }

    sum += __checksum;
    return ((sum & EIGHTSIZE) == EIGHTSIZE);
}

