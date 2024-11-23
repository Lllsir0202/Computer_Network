#include <iostream>
#include <stdint.h>
#include <defs.h>
/*
    这里考虑设计如下：
    |   校验和(8位)  |   状态位(8位)  |   ACK号(32位)   |    Seq号(32位)    |       窗体大小(32位)     |   数据长度(16位)    |   数据(UDP接受和发出的原始数据包)    |

    对于数据段
    分为：

    |   传输文件名  |   数据    |

    状态位用来标识： ————> 这个包是发送还是确认接受的
                ————> 这个包是初始还是什么情况
    这里设计如下：
    |   RESERVED   |    Start   |   Trans   |   SYNC    |    ACK    |    FIN    |   RST   |
    |      2       |      1     |     1     |     1     |     1     |     1     |    1    |

*/

class data
{
public:
    // 添加析构函数清理内存
    ~data();

    // 初始化一个data主要在于初始化一些私有变量
    void init(uint8_t flag, uint32_t ack, uint32_t seq, uint32_t windowsize, uint16_t datalen, uint8_t *d);

    // 通过传入的字节流得到对应的封装后的数据包 -> 同样的也是字节流
    uint8_t *gen_data(uint8_t *raw);

    // 解包得到的字节流
    void regen_data(uint8_t *d);

    // 下面这些函数是用来得到私有变量的
    uint8_t get_checksum() { return __checksum; };
    uint8_t get_flag() { return __flag; };
    uint32_t get_ack() { return __ACK; };
    uint32_t get_seq() { return __SEQ; };
    uint32_t get_windowsize() { return __windowsize; };
    uint16_t get_datalen() { return __datalen; };
    uint8_t *get_data() { return __d; };

private:
    // 记录8位校验和
    uint8_t __checksum;

    // 记录8位状态位
    uint8_t __flag;

    // 记录ACK号
    uint32_t __ACK;

    // 记录序列号
    uint32_t __SEQ;

    // 记录窗体大小
    uint32_t __windowsize;

    // 记录数据长度
    uint16_t __datalen;

    // 记录数据段
    uint8_t *__d;

    // 用于处理生成校验和的私有成员变量
    void __gen_checksum();
};