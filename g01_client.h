#include "g01_common.h"

//step��
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

//����һ����������
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


//��������

//client��
/**********
����n�����Ӳ����������շ�
ip      Ŀ��ip��ַ
port    Ŀ��˿ڵ�ַ
n       ������������
mode    �Զ�����ӵ�ά����ʽ ����3ѡ1
forkmode    fork/nofork ����2ѡ1
blockmode   ����/������ ����2ѡ1
**********/
int C_CreateConncs(struct client_conf client);

/**********
����1��sock������������
ip      Ŀ��ip��ַ
port    Ŀ��˿ڵ�ַ
blockmode   ����/������ ����2ѡ1
**********/
int C_CreateSock(int blockmode);

//main��ֱ�ӵ���C_CreateConncs���ú����е�����������

/**********
server����client�˽�������
socket      ������
mode        select/poll/epoll
forkmode    fork/nonfork
blockmode   ����/������ ����2ѡ1
**********/
int C_Connection(struct sockaddr_in servaddr, int sockfd, int mode, int forkmode, int blockmode);

/**********
��һ����Ϣ ����step�ж��Ƿ�Ϸ� str״̬�·����ַ�������
socket      ������
step        ����
**********/
int C_ReadMsg(int sockfd, int step, int blockmode);

/**********
server����client�˽�������
socket      ������
mode        select/poll/epoll
forkmode    fork/nonfork
blockmode   ����/������ ����2ѡ1
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