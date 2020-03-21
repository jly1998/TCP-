#include "g01_client.h"
//复习sockaddr_in结构体
// struct sockaddr_in
// {
// short sin_family;
// /*Address family一般来说AF_INET（地址族）PF_INET（协议族）*/

// unsigned short sin_port;
// /*Port number(必须要采用网络数据格式,普通数字可以用htons()函数转换成网络数据格式的数字)*/

// struct in_addr sin_addr;
// /*IP address in network byte order（Internet address）*/

// unsigned char sin_zero[8];
// /*Same size as struct sockaddr没有实际意义,只是为了　跟SOCKADDR结构在内存中对齐*/
// };

//宏定义

#define _NUM 100


void initial_para(struct client_conf *client)
{
    client->server_addr.sin_family = AF_INET; //设置为IP通信
                                              // client->server_addr.sin_addr.s_addr = htons(INADDR_ANY); //服务器IP地址--允许连接到所有本地地址上
    client->block = _NONBLOCK;                //默认值为非阻塞
    client->fork = _NOFORK;                   //默认值为单个进程方式
    client->select = _SELECT;                 //默认值为SELECT方式
    client->num = _NUM;                       //默认值为连接数为_NUM

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
    //用来判断互斥的标识符，如果已经设置过置1，再次被设置 且 与初次设置的值不同 就会报错；

    int i;
    if (argc < 4)
    {
        printf("参数输入过少，至少输入TCP端口号和服务端IP地址!\n");
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
                    printf("ip地址重复设置，请重新输入参数!\n");
                    return -1;
                }
            }
            else
            {
                if (temp == -1)
                {
                    printf("IP地址格式输入错误，请重新输入!\n");
                    return -1;
                }
                client->server_addr.sin_addr.s_addr = temp; //服务器IP地址
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
                    printf("port地址重复设置，请重新输入参数!\n");
                    return -1;
                }
            }
            else
            {
                client->server_addr.sin_port = htons(temp); //服务器端口号
                if_port = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--block"))
        {
            if (1 == if_block && _BLOCK != client->block)
            {
                printf("阻塞方式两次设置不一致，请重新输入参数!\n");
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
                printf("阻塞方式两次设置不一致，请重新输入参数!\n");
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
                printf("进程处理方式两次设置不一致，请重新输入参数!\n");
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
                printf("进程处理方式两次设置不一致，请重新输入参数!\n");
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
                printf("进程管理方式重复设置，请重新输入参数!\n");
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
                printf("进程管理方式重复设置，请重新输入参数!\n");
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
                printf("进程管理方式重复设置，请重新输入参数!\n");
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
                    printf("连接数重复设置，请重新输入参数!\n");
                    return -1;
                }
            }
            else
            {
                client->num = temp; //服务器端口号
                if_num = 1;
            }
        }
        else
        {
            printf("输入无参数，请重新输入!\n");
            return -1;
        }
        //NOFORK和BLOCK同时出现，CLOCK无效
        if (client->fork == _NOFORK)
            client->block = _NONBLOCK;
        //分裂子进程，不区分select
        else
            client->select=-1;
    }

    return 0;
}

void print_client_conf(struct client_conf client)
{
    printf("------------------------------\n");
    printf("client端配置信息：\n");
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
    struct client_conf client;                 //client数据结构
    initial_para(&client);                     //初始化client端配置信息
    int flag = para_deal(argc, argv, &client); //根据main函数参数配置client
    if (flag == 0)
    print_client_conf(client); //显示client配置信息
    C_CreateConncs(client);
    return 0;
}