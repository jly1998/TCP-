#include "g01_server.h"

//��ϰsockaddr_in�ṹ��
// struct sockaddr_in
// {
// short sin_family;
// /*Address familyһ����˵AF_INET����ַ�壩PF_INET��Э���壩*/

// unsigned short sin_port;
// /*Port number(����Ҫ�����������ݸ�ʽ,��ͨ���ֿ�����htons()����ת�����������ݸ�ʽ������)*/

// struct in_addr sin_addr;
// /*IP address in network byte order��Internet address��*/

// unsigned char sin_zero[8];
// /*Same size as struct sockaddrû��ʵ������,ֻ��Ϊ�ˡ���SOCKADDR�ṹ���ڴ��ж���*/
// };

//�궨��
//����һ����������
struct server_conf
{
    struct sockaddr_in server_addr;
    int block;
    int fork;
    int select;
};

void initial_para(struct server_conf *server)
{
    server->server_addr.sin_family = AF_INET;                //����ΪIPͨ��
    server->server_addr.sin_addr.s_addr = htons(INADDR_ANY); //������IP��ַ--�������ӵ����б��ص�ַ��
    server->block = _NONBLOCK;                               //Ĭ��ֵΪ������
    server->fork = _NOFORK;                                  //Ĭ��ֵΪ�������̷�ʽ
    server->select = _SELECT;                                //Ĭ��ֵΪSELECT��ʽ

    return;
}

int para_deal(int argc, char *argv[], struct server_conf *server)
{
    int if_ip = 0;
    int if_port = 0;
    int if_block = 0;
    int if_fork = 0;
    int if_num = 0;
    int if_select = 0;
    //�����жϻ���ı�ʶ��������Ѿ����ù���1���ٴα����þͻᱨ��

    int i;
    if (argc < 2)
    {
        printf("����������٣���������TCP�˿ں�!\n");
        return -1;
    }
    for (i = 1; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "--ip"))
        {
            int temp = inet_addr(argv[++i]);
            if (1 == if_ip)
            {
                if (temp != server->server_addr.sin_addr.s_addr)
                {
                    printf("ip��ַ�ظ����ã��������������!\n");
                    return -1;
                }
            }
            else
            {
                if (temp == -1)
                {
                    printf("IP��ַ��ʽ�����������������!\n");
                    return -1;
                }
                server->server_addr.sin_addr.s_addr = temp; //������IP��ַ
                if_ip = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--port"))
        {
            int temp = atoi(argv[++i]);
            if (1 == if_port)
            {
                if (htons(temp) != server->server_addr.sin_port)
                {
                    printf("port��ַ�ظ����ã��������������!\n");
                    return -1;
                }
            }
            else
            {
                server->server_addr.sin_port = htons(temp); //�������˿ں�
                if_port = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--block"))
        {
            if (1 == if_block && _BLOCK != server->block)
            {
                printf("������ʽ�������ò�һ�£��������������!\n");
                return -1;
            }
            else
            {
                server->block = _BLOCK;
                if_block = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--nonblock"))
        {
            if (1 == if_block && (_NONBLOCK != server->block))
            {
                printf("������ʽ�������ò�һ�£��������������!\n");
                return -1;
            }
            else
            {
                server->block = _NONBLOCK;
                if_block = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--fork"))
        {
            if (1 == if_fork && _FORK != server->fork)
            {
                printf("���̴���ʽ�������ò�һ�£��������������!\n");
                return -1;
            }
            else
            {
                server->fork = _FORK;
                if_fork = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--nofork"))
        {
            if (1 == if_fork && (_NOFORK != server->fork))
            {
                printf("���̴���ʽ�������ò�һ�£��������������!\n");
                return -1;
            }
            else
            {
                server->fork = _NOFORK;
                if_fork = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--select"))
        {
            if (1 == if_select && _SELECT != server->select)
            {
                printf("���̹���ʽ�ظ����ã��������������!\n");
                return -1;
            }
            else
            {
                server->select = _SELECT;
                if_select = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--poll"))
        {
            if (1 == if_select && _POLL != server->select)
            {
                printf("���̹���ʽ�ظ����ã��������������!\n");
                return -1;
            }
            else
            {
                server->select = _POLL;
                if_select = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--epoll"))
        {
            if (1 == if_select && _EPOLL != server->select)
            {
                printf("���̹���ʽ�ظ����ã��������������!\n");
                return -1;
            }
            else
            {
                server->select = _EPOLL;
                if_select = 1;
            }
        }
        else
        {
            printf("�����޲���������������!\n");
            return -1;
        }
        //NOFORK��BLOCKͬʱ���֣�CLOCK��Ч
        if (server->fork == _NOFORK)
            server->block = _NONBLOCK;
        //�����ӽ���fork��ʽ��������select
        else
            server->select=-1;
    }

    return 0;
}

void print_server_conf(struct server_conf server)
{
    printf("------------------------------\n");
    printf("server��������Ϣ��\n");
    printf("IP:%s\nPORT:%d\n", inet_ntoa(server.server_addr.sin_addr), ntohs(server.server_addr.sin_port));
    printf(server.block ? "block\n" : "nonblock\n");
    printf(server.fork ? "fork\n" : "nofork\n");
    if (_SELECT == server.select)
        printf("select\n");
    else if (_POLL == server.select)
        printf("poll\n");
    else if (_EPOLL == server.select)
        printf("epoll\n");
}

int main(int argc, char *argv[])
{
    system("rm -rf txt");    
    struct server_conf server;                 //server���ݽṹ
    initial_para(&server);                     //��ʼ��server��������Ϣ
    int flag = para_deal(argc, argv, &server); //����main������������server
    if (flag == 0)
        print_server_conf(server); //��ʾserver������Ϣ

    int listen_sock;
    listen_sock = S_CreateListenSock(server.server_addr, server.block);
    S_AcConncs(listen_sock, server.select, server.fork, server.block);

    return 0;
}