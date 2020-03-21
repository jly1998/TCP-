#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/epoll.h>

//宏定义-参数
#define _NONBLOCK       0
#define _BLOCK          1

#define _NOFORK         0
#define _FORK           1

#define _SELECT         0
#define _POLL           1
#define _EPOLL          2

//宏定义-发送接受返回值
#define _SUCCESS        0
#define _ERROR          -1
#define _WRONGDATA      -2
#define _WAITING        -3

#ifndef TRUE
  #define TRUE 1
  #endif

#ifndef FALSE
  #define FALSE 0
  #endif
//函数定义


//select
/**********
监测某文件描述符是否可读，可读返回1，不可读返回0
fd      文件描述符
Maxfd   指向当前最大文件描述符的值，由于该文件描述符可能从未被监测，
        因此运行后Maxfd的值可能会改变。
        首次调用时Maxfd变量指向的值应为0
**********/
int Readable(int fd, int* Maxfd);

/**********
监测某文件描述符是否可写，可写返回1，不可写返回0
fd      文件描述符
Maxfd   指向当前最大文件描述符的值，由于该文件描述符可能从未被监测，
        因此运行后Maxfd的值可能会改变
        首次调用时Maxfd变量指向的值应为0
**********/
int Writeable(int fd, int* Maxfd);

/**********
监测某文件描述符是否可读-epoll方式，可读返回1，不可读返回0
fd      文件描述符
events  epoll_wait返回的events数组
num     event数组内容的长度（epoll_wait返回值）
**********/
int Epoll_Readable(int fd, struct epoll_event events[], int num);

/**********
监测某文件描述符是否可写-epoll方式，可写返回1，不可写返回0
fd      文件描述符
events  epoll_wait返回的events数组
num     event数组内容的长度（epoll_wait返回值）
**********/
int Epoll_Writeable(int fd, struct epoll_event events[], int num);


//其他
/**********
监测socket连接情况，返回1为连接，0断开
socket  套接字描述符
**********/
int Connected(int socket);

/**********
将socket设置为非阻塞的
iSock   套接字描述符
**********/
int SetNonBlock(int iSock);


/**********
信号处理函数
signo       待处理的函数
**********/
void sig_handler(int signo);

/**********
建立文件夹
**********/
int CreateFolder();

/**********
写入文件
stuno       收到的学号
pid         收到的pid
time        收到的time
str         收到的长长字符串
len         长长字符串的长度
**********/
int CreateFile(const char* stuno, const char* pid, const char* time, const char* str, int len);