#include "recvdatamanager.h"
#include <random>
#include <cassert>

recvdatamanager::recvdatamanager()
{
    logout.open("./recv.log", std::ios::app);
    if (!logout.is_open())
    {
        std::cout << "CANNOT OPEN LOG " << std::endl;
    }
    __path = "./";
}

recvdatamanager::~recvdatamanager()
{
    for (auto it = seq2data.begin(); it != seq2data.end(); it++)
    {
        delete it->second;
    }
}

void recvdatamanager::init_ISN()
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

bool recvdatamanager::verify(uint32_t acknum)
{
    if (seq2data.find(acknum - 1) == seq2data.end())
    {
        std::string error = "ERROR: Package preACKED ";
        perror(error.c_str());
    }
    else
    {
        auto d = seq2data[acknum - 1];
        return __verify_data(d);
    }
}

void recvdatamanager::acknowledge(uint32_t acknum)
{
    if (seq2data.find(acknum - 1) == seq2data.end())
    {
        std::string error = "ERROR: Cannot find data to be acquired ";
        perror(error.c_str());
    }
    else
    {
        std::string log = "Acknowledge seqnum package " + std::to_string(acknum - 1);
        add_log(log);

        auto d = seq2data[acknum - 1];
        seq2data.erase(acknum);
        // 更新下一个序列号为对方发送的渴望得到的
        __Acknum = d->get_seq() + d->get_datalen() + 1;
        // 更新下一个next为seq+datalen
        //__Seqnum = d->get_ack() + d->get_datalen();
        delete d;
    }
}

bool recvdatamanager::__verify_data(data *d)
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

uint8_t *recvdatamanager::get_package(uint8_t flag, uint8_t *raw, uint32_t windowsize, uint16_t datalen)
{
    // 首先new一个data对象
    auto d = new data();
    d->init(flag, __Acknum, __Seqnum, windowsize, datalen, raw);

    // 生成对应数据包后，将seq += datalen
    __Seqnum += datalen;
    // 保存Seqnum对应的数据包
    seq2data[__Seqnum] = d;
    return d->gen_data(raw);
}

bool recvdatamanager::solve_package(uint8_t *pack, int flag)
{
    auto d = new data;
    d->regen_data(pack);
    // 对于收到的包进行解包，然后进行差错检验
    if (verify(d->get_ack()))
    { // 标识差错检测通过
        switch (flag)
        {
        case 0:
        {
            // 这里对接收端应该将这个文件写回
            // acknowledge(d->get_ack());
            // 这种情况表示这个数据包是第一个数据包
            if (d->get_flag() & START == START)
            {
                // 这里第一个数据包仅传输文件名
                std::string filename = std::string((char *)(d->get_data()));
                __filename = filename;
                __filepath = __path + filename;
                __packagenum = 0;
                fileout.open(__filepath.c_str(), std::ios::app);
                std::string log = "Start to accept " + __filename + " in file path: " + __filepath;
                add_log(log);
            }
            else if (d->get_flag() & TRANS == TRANS)
            {
                // 这是文件传输的情况
                // 将文件写回
                __packagenum++;
                std::string log = "Filename: " + __filename + " Package number: " + std::to_string(__packagenum);
                fileout << d->get_data();
                add_log(log);
            }
            else if (d->get_flag() & FIN == FIN)
            {
                // 这是在接收到数据时已经收到FIN的情况
                delete d;
                solve_package(pack, 3);
                return false;
            }

            // 接收之后这个包就无用了
            delete d;
            return true;
        }
        break;
        // 第一次握手
        case 1:
        {
            assert(d->get_flag() & (SYNC | ACK) == (SYNC | ACK));
            acknowledge(d->get_ack());

            std::string log = "Acknowledge First Handshake";
            add_log(log);
            delete d;
            return true;
        }
        break;
        // 第三次握手
        case 2:
        {
            assert(d->get_flag() & ACK == ACK);
            acknowledge(d->get_ack());

            std::string log = "Acknowledge Third Handshake";
            add_log(log);
            delete d;
            return true;
        }
        break;
        // 第一次挥手
        case 3:
        {
            assert(d->get_flag() & FIN == FIN);
            acknowledge(d->get_ack());

            std::string log = "Acknowledge First Wave";
            add_log(log);
            delete d;
            return true;
        }
        break;
        // 第三次挥手
        case 4:
        {
            assert(d->get_flag() & ACK == ACK);
            acknowledge(d->get_ack());

            std::string log = "Acknowledge Third Wave";
            add_log(log);
            delete d;
            return true;
        }
        break;
        default:
        {
            delete d;
            perror("Invalid flag ");
        }
        break;
        }
    }
    else
    {
        // 这里应该进行重传了，但现在还没有实现
        return false;
        // 在这里返回false，在外面封装处理函数
    }
}

void recvdatamanager::add_log(std::string log)
{
    logout << log << std::endl;
    std::cout << log << std::endl;
}