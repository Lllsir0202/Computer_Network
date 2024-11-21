#include "data.h"
#include <string>

void data::init()
{
    this->__datalen = 0;
}

// 下面处理我们的辅助函数
// 计算校验和，需要将每个部分分为8位，然后计算出来
void data::__gen_checksum()
{
    int flag = static_cast<int>(__flag);
    int ack = std::stoi(__ACK);
    int datalen = __datalen;

    // 由于我们的限定，每个数据包的大小不可能超过2^32-1，所以我们可以直接得到
    int data = std::stoi(__d);
    // 这里需要处理对数据包的校验

    int versum = flag + ack + datalen + data;

    // 由于是在用户层的实现，所以我们不好使用位来处理
    // 这里考虑使用-1 * 计算结果，传输到后若出现求和不为0，那么就是出现了传输错误
    __checksum = -1 * versum;
}

// 下面来处理其他生成函数
std::string data::gen_data(std::string raw, bool flag, std::string ack)
{
    std::string tmp = raw;
    __flag = flag;
    __ACK = ack;
    __datalen = raw.length();
    __d = raw;

    // 然后生成校验和
    __gen_checksum();
    tmp = __checksum + std::to_string(__flag) + __ACK + std::to_string(__datalen) + __d;
    return tmp;
}

std::string data::regen_data(char *d)
{
}