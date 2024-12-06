#define START 0x32 // 用于表示这是该文件的第一个数据包，用于传输长度和文件名
#define TRANS 0x16 // 用于传输
#define SYNC 0x8   // 用于握手
#define ACK 0x4    // 用于确认
#define FIN 0x2    // 用于挥手
#define RST 0x1    // 用于重传

// 用于处理多字节转8位单字节情况
#define EIGHTSIZE 0XFF

// 记录除了数据段外的字节数
#define INITSIZE 16

// 定义超时时间(微秒)
#define TIMEOUT 500