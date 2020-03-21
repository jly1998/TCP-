#include "g01_client.h"
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

#define _NUM 100


void initial_para(struct client_conf *client)
{
    client->server_addr.sin_family = AF_INET; //����ΪIPͨ��
                                              // client->server_addr.sin_addr.s_addr = htons(INADDR_ANY); //������IP��ַ--�������ӵ����б��ص�ַ��
    client->block = _NONBLOCK;                //Ĭ��ֵΪ������
    client->fork = _NOFORK;                   //Ĭ��ֵΪ�������̷�ʽ
    client->select = _SELECT;                 //Ĭ��ֵΪSELECT��ʽ
    client->num = _NUM;                       //Ĭ��ֵΪ������Ϊ_NUM

    return;
}

int para_deal(int argc, char *argv[], struct client_conf *client)
{
    int if_ip = 0;
    int if_port = 0;
    int if_block = 0;
    int if_fork = 0;
    int if_num = 0;
    int if_select = 0;
    //�����жϻ���ı�ʶ��������Ѿ����ù���1���ٴα����� �� ��������õ�ֵ��ͬ �ͻᱨ��

    int i;
    if (argc < 4)
    {
        printf("����������٣���������TCP�˿ںźͷ����IP��ַ!\n");
        return -1;
    }
    for (i = 1; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "--ip"))
        {
            int temp = inet_addr(argv[++i]);
            if (1 == if_ip)
            {
                if (temp != client->server_addr.sin_addr.s_addr)
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
                client->server_addr.sin_addr.s_addr = temp; //������IP��ַ
                if_ip = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--port"))
        {
            int temp = atoi(argv[++i]);
            if (1 == if_port)
            {
                if (htons(temp) != client->server_addr.sin_port)
                {
                    printf("port��ַ�ظ����ã��������������!\n");
                    return -1;
                }
            }
            else
            {
                client->server_addr.sin_port = htons(temp); //�������˿ں�
                if_port = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--block"))
        {
            if (1 == if_block && _BLOCK != client->block)
            {
                printf("������ʽ�������ò�һ�£��������������!\n");
                return -1;
            }
            else
            {
                client->block = _BLOCK;
                if_block = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--nonblock"))
        {
            if (1 == if_block && (_NONBLOCK != client->block))
            {
                printf("������ʽ�������ò�һ�£��������������!\n");
                return -1;
            }
            else
            {
                client->block = _NONBLOCK;
                if_block = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--fork"))
        {
            if (1 == if_fork && _FORK != client->fork)
            {
                printf("���̴���ʽ�������ò�һ�£��������������!\n");
                return -1;
            }
            else
            {
                client->fork = _FORK;
                if_fork = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--nofork"))
        {
            if (1 == if_fork && (_NOFORK != client->fork))
            {
                printf("���̴���ʽ�������ò�һ�£��������������!\n");
                return -1;
            }
            else
            {
                client->fork = _NOFORK;
                if_fork = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--select"))
        {
            if (1 == if_select && _SELECT != client->select)
            {
                printf("���̹���ʽ�ظ����ã��������������!\n");
                return -1;
            }
            else
            {
                client->select = _SELECT;
                if_select = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--poll"))
        {
            if (1 == if_select && _POLL != client->select)
            {
                printf("���̹���ʽ�ظ����ã��������������!\n");
                return -1;
            }
            else
            {
                client->select = _POLL;
                if_select = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--epoll"))
        {
            if (1 == if_select && _EPOLL != client->select)
            {
                printf("���̹���ʽ�ظ����ã��������������!\n");
                return -1;
            }
            else
            {
                client->select = _EPOLL;
                if_select = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--num"))
        {
            int temp = atoi(argv[++i]);
            if (1 == if_num)
            {
                if (temp != client->num)
                {
                    printf("�������ظ����ã��������������!\n");
                    return -1;
                }
            }
            else
            {
                client->num = temp; //�������˿ں�
                if_num = 1;
            }
        }
        else
        {
            printf("�����޲���������������!\n");
            return -1;
        }
        //NOFORK��BLOCKͬʱ���֣�CLOCK��Ч
        if (client->fork == _NOFORK)
            client->block = _NONBLOCK;
        //�����ӽ��̣�������select
        else
            client->select=-1;
    }

    return 0;
}

void print_client_conf(struct client_conf client)
{
    printf("------------------------------\n");
    printf("client��������Ϣ��\n");
    printf("IP:%s\nPORT:%d\n", inet_ntoa(client.server_addr.sin_addr), ntohs(client.server_addr.sin_port));
    printf(client.block ? "block\n" : "nonblock\n");
    printf(client.fork ? "fork\n" : "nofork\n");

    if (_SELECT == client.select)
        printf("select\n");
    else if (_POLL == client.select)
        printf("poll\n");
    else if (_EPOLL == client.select)
        printf("epoll\n");

    printf("num:%d\n", client.num);
}

int main(int argc, char *argv[])
{
    system("rm -rf txt");
    struct client_conf client;                 //client���ݽṹ
    initial_para(&client);                     //��ʼ��client��������Ϣ
    int flag = para_deal(argc, argv, &client); //����main������������client
    if (flag == 0)
    print_client_conf(client); //��ʾclient������Ϣ
    C_CreateConncs(client);
    return 0;
}