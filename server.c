#include "g01_server.h"

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
//定义一个参数配置
struct server_conf
{
    struct sockaddr_in server_addr;
    int block;
    int fork;
    int select;
};

void initial_para(struct server_conf *server)
{
    server->server_addr.sin_family = AF_INET;                //设置为IP通信
    server->server_addr.sin_addr.s_addr = htons(INADDR_ANY); //服务器IP地址--允许连接到所有本地地址上
    server->block = _NONBLOCK;                               //默认值为非阻塞
    server->fork = _NOFORK;                                  //默认值为单个进程方式
    server->select = _SELECT;                                //默认值为SELECT方式

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
    //用来判断互斥的标识符，如果已经设置过置1，再次被设置就会报错；

    int i;
    if (argc < 2)
    {
        printf("参数输入过少，至少输入TCP端口号!\n");
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
                server->server_addr.sin_addr.s_addr = temp; //服务器IP地址
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
                    printf("port地址重复设置，请重新输入参数!\n");
                    return -1;
                }
            }
            else
            {
                server->server_addr.sin_port = htons(temp); //服务器端口号
                if_port = 1;
            }
        }
        else if (0 == strcmp(argv[i], "--block"))
        {
            if (1 == if_block && _BLOCK != server->block)
            {
                printf("阻塞方式两次设置不一致，请重新输入参数!\n");
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
                printf("阻塞方式两次设置不一致，请重新输入参数!\n");
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
                printf("进程处理方式两次设置不一致，请重新输入参数!\n");
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
                printf("进程处理方式两次设置不一致，请重新输入参数!\n");
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
                printf("进程管理方式重复设置，请重新输入参数!\n");
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
                printf("进程管理方式重复设置，请重新输入参数!\n");
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
                printf("进程管理方式重复设置，请重新输入参数!\n");
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
            printf("输入无参数，请重新输入!\n");
            return -1;
        }
        //NOFORK和BLOCK同时出现，CLOCK无效
        if (server->fork == _NOFORK)
            server->block = _NONBLOCK;
        //分裂子进程fork方式，不区分select
        else
            server->select=-1;
    }

    return 0;
}

void print_server_conf(struct server_conf server)
{
    printf("------------------------------\n");
    printf("server端配置信息：\n");
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
    struct server_conf server;                 //server数据结构
    initial_para(&server);                     //初始化server端配置信息
    int flag = para_deal(argc, argv, &server); //根据main函数参数配置server
    if (flag == 0)
        print_server_conf(server); //显示server配置信息

    int listen_sock;
    listen_sock = S_CreateListenSock(server.server_addr, server.block);
    S_AcConncs(listen_sock, server.select, server.fork, server.block);

    return 0;
}