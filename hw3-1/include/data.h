#include <iostream>

/*
    这里考虑设计如下：
    |   校验号  |   状态位(1位)  |   ACK号   |   数据长度(12位)    |   数据(UDP接受和发出的原始数据包)    |

    状态位用来标识： ————> 这个包是发送还是确认接受的
    ACK号保留一位用于计算我们的校验和，便于处理
*/

class data
{
public:
    // 初始化一个data主要在于对于初始化一些私有变量
    void init();

    // 通过给定的string得到对应的封装后的数据包
    std::string gen_data(std::string raw, bool flag, std::string ack);

    // 解包string
    std::string regen_data(char* d);

    // 得到校验是否正确
    bool get_verfy() { return __verify; };
    // 得到校验和
    std::string get_checksum() { return __checksum; };
    // 得到状态位
    bool get_flag() { return __flag; };
    // 得到ACK号
    std::string get_ack() { return __ACK; };
    // 得到数据长度
    int get_datalen() { return __datalen; };
    // 得到数据
    std::string get_data() { return __d; };

    // 设定初始序列号
    void set_ISN(std::string ISN) { __ISN = ISN; };

private:
    // 标识是否校验正确
    bool __verify = false;

    // 记录校验号
    std::string __checksum;
    // 记录状态位，这里其实应该在生成数据包的时候传入
    // 对于flag ，我们设定当true标识是发出的包，false为传入的包
    bool __flag; // -> 这里先设定成bool，后续可以进行修改
    // 记录ACK号
    std::string __ACK;
    // 记录数据长度
    int __datalen;
    // 记录得到的数据
    std::string __d;

    // 记录初始序列号
    std::string __ISN;

    // 下面是一些生成数据包时候的辅助函数

    // 根据数据得到校验和
    // 由于在用户层计算二进制数据会很麻烦，这里对于每个部分求和，然后我们使用32位二进制数的最大值减去得到的校验和作为校验和
    // 在接收端处理则是对于得到的各部分数据求和，在和校验和相加，从而保证数据等于32位二进制数的最大值。
    void __gen_checksum();
};