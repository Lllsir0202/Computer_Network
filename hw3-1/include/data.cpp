#include "data.h"

data::~data()
{
    if ((__flag & TRANS) == TRANS || (__flag & START) == START)
    {
        delete[] __d;
    }
}

void data::init(uint8_t flag, uint32_t ack, uint32_t seq, uint32_t windowsize, uint16_t datalen, uint8_t *d)
{
    __flag = flag;
    __ACK = ack;
    __SEQ = seq;
    __windowsize = windowsize;
    __datalen = datalen;
    __d = d;
    // 生成校验和
    __gen_checksum();
}

void data::__gen_checksum()
{
    uint16_t sum = 0;

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

    //  回卷计算
    sum = (sum & EIGHTSIZE) + (sum >> 8);

    // 取反赋值
    __checksum = ~sum;
    return;
}

uint8_t *data::gen_data(uint8_t *raw)
{
    uint8_t *d = new uint8_t[__datalen + INITSIZE];
    // 校验和
    d[0] = __checksum;
    // 状态位
    d[1] = __flag;
    // ACK号
    d[2] = __ACK & EIGHTSIZE;         // 第一个字节
    d[3] = (__ACK >> 8) & EIGHTSIZE;  // 第二个字节
    d[4] = (__ACK >> 16) & EIGHTSIZE; // 第三个字节
    d[5] = (__ACK >> 24) & EIGHTSIZE; // 第四个字节

    // SEQ号
    d[6] = __SEQ & EIGHTSIZE;         // 第一个字节
    d[7] = (__SEQ >> 8) & EIGHTSIZE;  // 第二个字节
    d[8] = (__SEQ >> 16) & EIGHTSIZE; // 第三个字节
    d[9] = (__SEQ >> 24) & EIGHTSIZE; // 第四个字节
    // 窗体大小
    d[10] = __windowsize & EIGHTSIZE;         // 第一个字节
    d[11] = (__windowsize >> 8) & EIGHTSIZE;  // 第二个字节
    d[12] = (__windowsize >> 16) & EIGHTSIZE; // 第三个字节
    d[13] = (__windowsize >> 24) & EIGHTSIZE; // 第四个字节
    // 数据长度
    d[14] = __datalen & EIGHTSIZE;        // 第一个字节
    d[15] = (__datalen >> 8) & EIGHTSIZE; // 第二个字节

    // 当处理的是数据包的时候，进行复制
    if ((__flag & TRANS) == TRANS || (__flag & START) == START)
    {
        memcpy(d + INITSIZE, raw, __datalen * sizeof(uint8_t));
    }

    return d;
}

void data::regen_data(uint8_t *d)
{
    __checksum = d[0];
    __flag = d[1];

    __ACK = d[2];
    __ACK += d[3] << 8;
    __ACK += d[4] << 16;
    __ACK += d[5] << 24;

    __SEQ = d[6];
    __SEQ += d[7] << 8;
    __SEQ += d[8] << 16;
    __SEQ += d[9] << 24;

    __windowsize = d[10];
    __windowsize += d[11] << 8;
    __windowsize += d[12] << 16;
    __windowsize += d[13] << 24;

    __datalen = d[14];
    __datalen += d[15] << 8;

    __d = new uint8_t[__datalen + 1];
    // 这里应该只有数据包的时候才进行memcpy
    if ((__flag & TRANS) == TRANS || (__flag & START) == START)
    {
        memcpy(__d, d + INITSIZE, __datalen * sizeof(uint8_t));
    }
    __d[__datalen] = '\0';
}