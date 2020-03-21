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

//�궨��-����
#define _NONBLOCK       0
#define _BLOCK          1

#define _NOFORK         0
#define _FORK           1

#define _SELECT         0
#define _POLL           1
#define _EPOLL          2

//�궨��-���ͽ��ܷ���ֵ
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
//��������


//select
/**********
���ĳ�ļ��������Ƿ�ɶ����ɶ�����1�����ɶ�����0
fd      �ļ�������
Maxfd   ָ��ǰ����ļ���������ֵ�����ڸ��ļ����������ܴ�δ����⣬
        ������к�Maxfd��ֵ���ܻ�ı䡣
        �״ε���ʱMaxfd����ָ���ֵӦΪ0
**********/
int Readable(int fd, int* Maxfd);

/**********
���ĳ�ļ��������Ƿ��д����д����1������д����0
fd      �ļ�������
Maxfd   ָ��ǰ����ļ���������ֵ�����ڸ��ļ����������ܴ�δ����⣬
        ������к�Maxfd��ֵ���ܻ�ı�
        �״ε���ʱMaxfd����ָ���ֵӦΪ0
**********/
int Writeable(int fd, int* Maxfd);

/**********
���ĳ�ļ��������Ƿ�ɶ�-epoll��ʽ���ɶ�����1�����ɶ�����0
fd      �ļ�������
events  epoll_wait���ص�events����
num     event�������ݵĳ��ȣ�epoll_wait����ֵ��
**********/
int Epoll_Readable(int fd, struct epoll_event events[], int num);

/**********
���ĳ�ļ��������Ƿ��д-epoll��ʽ����д����1������д����0
fd      �ļ�������
events  epoll_wait���ص�events����
num     event�������ݵĳ��ȣ�epoll_wait����ֵ��
**********/
int Epoll_Writeable(int fd, struct epoll_event events[], int num);


//����
/**********
���socket�������������1Ϊ���ӣ�0�Ͽ�
socket  �׽���������
**********/
int Connected(int socket);

/**********
��socket����Ϊ��������
iSock   �׽���������
**********/
int SetNonBlock(int iSock);


/**********
�źŴ�����
signo       ������ĺ���
**********/
void sig_handler(int signo);

/**********
�����ļ���
**********/
int CreateFolder();

/**********
д���ļ�
stuno       �յ���ѧ��
pid         �յ���pid
time        �յ���time
str         �յ��ĳ����ַ���
len         �����ַ����ĳ���
**********/
int CreateFile(const char* stuno, const char* pid, const char* time, const char* str, int len);