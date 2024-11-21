#define SYNC 0x8
#define ACK 0x4
#define FIN 0x2

// 用于处理多字节转8位单字节情况
#define EIGHTSIZE 0XFF

// 记录除了数据段外的字节数
#define INITSIZE 12