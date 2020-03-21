#include "g01_common.h"

//step用
#define _R_STUNO 1 
#define _W_STUNO 2
#define _R_PID 3
#define _W_PID 4 
#define _R_TIME 5
#define _W_TIME 6
#define _R_STR 7
#define _W_STR 8
#define _R_END 9

#define _IMKILLED 35

//定义一个参数配置
struct client_conf
{
    struct sockaddr_in server_addr;
    int block;
    int fork;
    int select;
    int num;
};

struct to_make_file
{
    char stuno[8];
    char pid[20];
    char time[20];
    char* str;
    int len;
};


//函数定义

//client端
/**********
建立n个链接并进行数据收发
ip      目标ip地址
port    目标端口地址
n       建立的连接数
mode    对多个连接的维护方式 参数3选1
forkmode    fork/nofork 参数2选1
blockmode   阻塞/非阻塞 参数2选1
**********/
int C_CreateConncs(struct client_conf client);

/**********
建立1个sock，返回描述符
ip      目标ip地址
port    目标端口地址
blockmode   阻塞/非阻塞 参数2选1
**********/
int C_CreateSock(int blockmode);

//main中直接调用C_CreateConncs，该函数中调用其他函数

/**********
server端与client端建立连接
socket      描述符
mode        select/poll/epoll
forkmode    fork/nonfork
blockmode   阻塞/非阻塞 参数2选1
**********/
int C_Connection(struct sockaddr_in servaddr, int sockfd, int mode, int forkmode, int blockmode);

/**********
读一条消息 根据step判断是否合法 str状态下返回字符串长度
socket      描述符
step        步骤
**********/
int C_ReadMsg(int sockfd, int step, int blockmode);

/**********
server端与client端交换数据
socket      描述符
mode        select/poll/epoll
forkmode    fork/nonfork
blockmode   阻塞/非阻塞 参数2选1
**********/

int C_ExcgMsg(int sockfd, int blockmode, int forkmode, int mode, int state, int *size, char **p,int sockfdtocreatefile);

    int C_ChangeState(int sockfd, int mode, int forkmode, int blockmode, int state);

    int C_ChangeState_Select(struct sockaddr_in servaddr, int mode, int forkmode, int blockmode, int num);

    int CanIConnect(struct timeval tcur, struct timeval tlast);

    int C_ReadMsg_Poll(int sockfd, int step, int blockmode);

    int C_ExcgMsg_Poll(int sockfd, int blockmode, int forkmode, int mode, int state, int *size, char **p, int sockfdtocreatefile, struct pollfd poll_fds);

    int C_ChangeState_Poll(struct sockaddr_in servaddr, int mode, int forkmode, int blockmode, int num);

    int C_ChangeState_EPOLL(struct sockaddr_in servaddr, int mode, int forkmode, int blockmode, int num);
    int C_ExcgMsg_EPOLL(int sockfd, int blockmode, int forkmode, int mode, int state, int *size, char **p, int sockfdtocreatefile, struct epoll_event events[], int num);
    int C_ReadMsg_EPOLL(int sockfd, int step, int blockmode);