#include "g01_common.h"

//�궨��-���ͽ���״̬
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

//�궨��-ͬʱ�����ά����������
#define _MAXC           200


//��������

//server��
//��¼��ȡ�ַ�����״̬
struct str_info{
    int Len;
    int Cur;
};

//��¼ÿ�����ӵ�socket�����ݴ���״̬
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
��������socket������������
server_socket   ���ýṹ��
blockmode   ����/������ ����2ѡ1
**********/
int S_CreateListenSock(struct sockaddr_in server_socket, int blockmode);

/**********
��������
listen_sock      ����socket
mode    �Զ�����ӵ�ά����ʽ ����3ѡ1
forkmode    fork/nofork ����2ѡ1
blockmode   ����/������ ����2ѡ1
**********/
int S_AcConncs(int listen_sock, int mode, int forkmode, int blockmode);

/**********
���������շ����ݣ�����0������-1�������1���������������
socket      socket������
mode    �Զ�����ӵ�ά����ʽ ����3ѡ1
blockmode   ����/������ ����2ѡ1
**********/
int S_Connection(int socket, int mode, int blockmode, struct client_info* c_info);

/**********
�շ����ݼ���
socket      socket������
mode        �Զ�����ӵ�ά����ʽ ����3ѡ1
blockmode   ����/������ ����2ѡ1
status      ״̬
result      �����Ľ��
**********/
int S_DataTrans(int socket, int mode, int blockmode, int status, char* result, struct str_info *number);


//main�е���S_CreateListenSock���ٵ���S_AcConncs�������е�����������

