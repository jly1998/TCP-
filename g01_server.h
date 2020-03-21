#include "g01_common.h"

//宏定义-发送接受状态
#define _WSTUNO         0
#define _RSTUNO         1
#define _WPID           2
#define _RPID           3
#define _WTIME          4
#define _RTIME          5
#define _WSTR           6
#define _RSTR           7
#define _WEND           8
#define _WAITCLOSE      9

//宏定义-同时刻最大维护的链接数
#define _MAXC           200


//函数定义

//server端
//记录读取字符串的状态
struct str_info{
    int Len;
    int Cur;
};

//记录每个连接的socket与数据传输状态
struct client_info{
    struct str_info Num;
    //int socket;
    int status;
    int used;
    char stuno[8];
    char pid[20];
    char time[20];
    char *str;
};



/**********
建立监听socket，返回描述符
server_socket   绑定用结构体
blockmode   阻塞/非阻塞 参数2选1
**********/
int S_CreateListenSock(struct sockaddr_in server_socket, int blockmode);

/**********
接受连接
listen_sock      监听socket
mode    对多个连接的维护方式 参数3选1
forkmode    fork/nofork 参数2选1
blockmode   阻塞/非阻塞 参数2选1
**********/
int S_AcConncs(int listen_sock, int mode, int forkmode, int blockmode);

/**********
单个连接收发数据，返回0正常，-1代表出错，1代表接受数据有误
socket      socket描述符
mode    对多个连接的维护方式 参数3选1
blockmode   阻塞/非阻塞 参数2选1
**********/
int S_Connection(int socket, int mode, int blockmode, struct client_info* c_info);

/**********
收发数据集成
socket      socket描述符
mode        对多个连接的维护方式 参数3选1
blockmode   阻塞/非阻塞 参数2选1
status      状态
result      读到的结果
**********/
int S_DataTrans(int socket, int mode, int blockmode, int status, char* result, struct str_info *number);


//main中调用S_CreateListenSock，再调用S_AcConncs，后者中调用其他函数

