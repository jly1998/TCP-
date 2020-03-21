#include "g01_client.h"

int C_CreateSock(int blockmode)
{
    // struct hostent *server_host_name;
    int sockfd;
    // if ((server_host_name = gethostbyname(ip)) == 0)
    // {
    //     perror("Error resolving local host ");
    //     return _ERROR;
    // }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("create socket ");
        //    printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return _ERROR;
    }

    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) == -1)
    {
        perror("reuse ");
        //printf("reuse error: %s(errno: %d)\n", strerror(errno), errno);
        return _ERROR;
    }

    if (blockmode == _NONBLOCK)
        SetNonBlock(sockfd);

    return sockfd;
}

int C_Connection(struct sockaddr_in servaddr, int sockfd, int mode, int forkmode, int blockmode)
{
    static int Maxfd = 0;
    int ret = -1;
    if (forkmode == _FORK || mode == _EPOLL)
    {
        if (blockmode == _BLOCK)
        {
            if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
            {
                printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
                return _ERROR;
            }
        }
        else
            while (1)
                if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != -1)
                    break;
        printf("Connection Success!\n");
    }
    else
    { //mode 1of3 switch
        if (mode == _SELECT || mode == _POLL || mode == _EPOLL)
        {
            int error;
            socklen_t optlen = sizeof(error);

            if (connect(sockfd, (void *)&servaddr, sizeof(servaddr)) != -1)
            {
                //直接成功
                ;
            }
            else
            {
                if (!Readable(sockfd, &Maxfd) && !Writeable(sockfd, &Maxfd))
                {
                    printf("&connect error: %s(errno: %d)\n", strerror(errno), errno);
                    return _ERROR;
                }
                else if (Readable(sockfd, &Maxfd) && Writeable(sockfd, &Maxfd))
                {
                    int flag = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &optlen);
                    if (flag == 0 && error == 0)
                    {
                        printf("Connection Success!\n");
                    }
                    else
                    {
                        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
                        return _ERROR;
                    }
                }
                else if (!Readable(sockfd, &Maxfd) && Writeable(sockfd, &Maxfd))
                {
                    printf("Connection Success!\n");
                }
                else
                {
                    printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
                    return _ERROR;
                }
            }
        } //select
    }     //nofork
    fflush(stdout);
    return _SUCCESS;
}

int C_ReadMsg(int sockfd, int step, int blockmode)
{
    char readline[BUFSIZ];
    char checkread[BUFSIZ];
    int flag = 0;
    int n;
    switch (step)
    {
    case _R_STUNO:
        strcpy(checkread, "StuNo");
        break;
    case _R_PID:
        strcpy(checkread, "pid");
        break;
    case _R_TIME:
        strcpy(checkread, "TIME");
        break;
    case _R_STR:
        strcpy(checkread, "str");
        flag = 1;
        break;
    case _R_END:
        strcpy(checkread, "end");
        break;
    default:
        break;
    }

    n = read(sockfd, readline, BUFSIZ);
    if (n == 0)
        return _ERROR;

    else if (n < 0)
    {
        if (_BLOCK == blockmode)
            return _ERROR;
        else
        {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
                return _WAITING;
        }
    }
    else //n>0
        ;

    readline[n] = '\0';
    if (flag == 0)
    {
        if (strcmp(readline, checkread) == 0)
        {
            printf("recv msg from server : %s\n", readline);
            fflush(stdout);
        }
        else
        {
            return _WRONGDATA;
        }
    }
    else
    {
        if (strncmp(readline, checkread, 3) == 0)
        {
            printf("recv msg from server : %s\n", readline);
            fflush(stdout);
        }
        else
        {
            return _WRONGDATA;
        }
    }

    if (flag == 1)
        return atoi(readline + 3);
    else
        return _SUCCESS;
}

int C_ExcgMsg(int sockfd, int blockmode, int forkmode, int mode, int state, int *size, char **p, int sockfdtocreatefile)
{
    int n;
    char writeline[BUFSIZ];
    int StuNo = htonl(1750762);
    int pid;
    time_t now_time;
    struct tm *ptminfo;
    char time_buf[20];
    int oncesent = 0;

    char *longchar = NULL;
    int i = 0;
    int ret;
    srand((unsigned int)time(NULL));

    if (forkmode == _FORK)
        pid = htonl(getpid());
    else
        pid = htonl((getpid() << 16) + sockfdtocreatefile);

    static int Maxfd = 0;

    switch (state)
    {
    case _R_STUNO:
        if (mode == _SELECT && Readable(sockfd, &Maxfd) == 0)
        {
            return _WAITING;
        }
        ret = C_ReadMsg(sockfd, _R_STUNO, blockmode);
        if (ret != _SUCCESS)
            return ret;
        break;

    case _W_STUNO:
        if (mode == _SELECT && Writeable(sockfd, &Maxfd) == 0)
            return _WAITING;
        n = write(sockfd, &StuNo, 4);
        if (n == 0)
            return _ERROR;

        printf("send msg to server : 1750762\n");
        fflush(stdout);
        strcpy(*p, "1750762");
        break;

    case _R_PID:
        if (mode == _SELECT && Readable(sockfd, &Maxfd) == 0)
            return _WAITING;
        ret = C_ReadMsg(sockfd, _R_PID, blockmode);
        if (ret != _SUCCESS)
            return ret;
        break;

    case _W_PID:
        if (mode == _SELECT && Writeable(sockfd, &Maxfd) == 0)
            return _WAITING;
        n = write(sockfd, &pid, 4);
        if (n == 0)
            return _ERROR;

        if (forkmode == _FORK)
        {
            printf("send msg to server : %d\n", getpid());
            sprintf(*p, "%d", getpid());
        }
        else
        {
            printf("send msg to server : %d\n", (getpid() << 16) + sockfdtocreatefile);
            sprintf(*p, "%d", (getpid() << 16) + sockfdtocreatefile);
        }
        // printf("pid : %d\n", getpid());
        // printf("unsigned : %u\n", (getpid() << 16) + sockfd);
        fflush(stdout);

        break;

    case _R_TIME:
        if (mode == _SELECT && Readable(sockfd, &Maxfd) == 0)
            return _WAITING;
        ret = C_ReadMsg(sockfd, _R_TIME, blockmode);
        if (ret != _SUCCESS)
            return ret;
        break;

    case _W_TIME:
        if (mode == _SELECT && Writeable(sockfd, &Maxfd) == 0)
            return _WAITING;
        time(&now_time);
        ptminfo = localtime(&now_time);
        sprintf(time_buf, "%02d-%02d-%02d %02d:%02d:%02d",
                ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, ptminfo->tm_mday,
                ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec);
        n = write(sockfd, time_buf, 19);
        if (n == 0)
            return _ERROR;
        printf("send msg to server : %s\n", time_buf);
        fflush(stdout);
        strcpy(*p, time_buf);
        break;

    case _R_STR:
        if (mode == _SELECT && Readable(sockfd, &Maxfd) == 0)
            return _WAITING;
        ret = C_ReadMsg(sockfd, _R_STR, blockmode);
        if (ret == _ERROR || ret == _WRONGDATA || ret == _WAITING)
            return ret;
        *size = ret;
        break;

    case _W_STR:
        longchar = (char *)malloc(*size);
        for (i = 0; i < *size; i++)
            longchar[i] = rand() % 256;
        n = 0; //already sent 0B
        while (1)
        {
            //printf("&");
            if (mode == _SELECT && Writeable(sockfd, &Maxfd) == 0)
            {
                //free(longchar);
                //return _WAITING;
                // printf("*");
                continue;
            }
            oncesent = write(sockfd, longchar + n, *size - n);
            //if(oncesent==0)
            //perror("write : ");
            if (oncesent > 0)
                n += oncesent;
            else if (oncesent == 0)
                return _ERROR;
            else if (oncesent < 0)
            {
                if (blockmode == _NONBLOCK && ((errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)))
                    continue;
                else
                {
                    return _ERROR;
                }

            } //printf("send %dB,%d in total\n", oncesent, n);

            if (n == *size)
                break;
        }
        printf("should send %dB to server\n", *size);
        fflush(stdout);
        printf("send %dB to server\n", n);
        fflush(stdout);
        *p = NULL;
        *p = (char *)malloc((*size) * sizeof(char));
        if (*p == NULL)
            return _ERROR;
        memcpy(*p, longchar, (*size) * sizeof(char));
        //strcpy(*p, longchar);
        free(longchar);
        break;

    case _R_END:
        if (mode == _SELECT && Readable(sockfd, &Maxfd) == 0)
            return _WAITING;
        ret = C_ReadMsg(sockfd, _R_END, blockmode);
        if (ret != _SUCCESS)
            return ret;
        break;

    default:
        break;
    }

    return _SUCCESS;
}

int C_ChangeState(int sockfd, int mode, int forkmode, int blockmode, int state)
{
    int ret;
    int size = 0;
    struct to_make_file forkthing;
    char **p = (char **)malloc(sizeof(char *));

    while (state < _R_END + 1)
    {
        if (state == _W_STUNO || state == _W_PID || state == _W_TIME)
            *p = (char *)malloc(20 * sizeof(char));

        ret = C_ExcgMsg(sockfd, blockmode, forkmode, mode, state, &size, p, 0);
        if (ret == _WRONGDATA || ret == _ERROR)
            return ret;
        else if (ret == _WAITING)
            continue;
        else
        {
            switch (state)
            {
            case _W_STUNO:
                strcpy(forkthing.stuno, *p);
                free(*p);
                break;
            case _W_TIME:
                strcpy(forkthing.time, *p);
                free(*p);
                break;
            case _W_PID:
                strcpy(forkthing.pid, *p);
                free(*p);
                break;
            case _W_STR:
                forkthing.str = NULL;
                forkthing.str = (char *)malloc(size * sizeof(char));
                if (forkthing.str == NULL)
                    return _ERROR;
                //strcpy(forkthing.str, *p);
                memcpy(forkthing.str, *p, size * sizeof(char));
                free(*p);
                break;
            default:
                break;
            }
            state++;
        }
    }
    //printf("%s\n%s\n%s\n", forkthing.stuno, forkthing.pid, forkthing.time);
    CreateFile(forkthing.stuno, forkthing.pid, forkthing.time,
               forkthing.str, size);
    free(forkthing.str);
    free(p);
    return _SUCCESS;
}

int C_ChangeState_Select(struct sockaddr_in servaddr, int mode, int forkmode, int blockmode, int num)
{
    int sockfd;
    int i, k;
    int status[1024];
    int available[1024];
    int size[1024] = {0};
    struct to_make_file forkthing[1024];
    char **p = (char **)malloc(sizeof(char *));
    int number[1024];
    // memset(status, _R_STUNO, 1024 * sizeof(int));
    // memset(available, 1, 1024 * sizeof(int));
    struct timeval tcur;
    struct timeval tlast;

    gettimeofday(&tlast, NULL);
    for (i = 0; i < 1024; i++)
    {
        status[i] = 0;
        available[i] = 0;
        number[i] = 0;
    }
    int ret;
    int okay = 0;
    int estb = 0;

    while (1)
    {
        if (okay == num)
            break;
        if (estb < num)
        {
            gettimeofday(&tcur, NULL);
            if (CanIConnect(tcur, tlast))
            {
                sockfd = C_CreateSock(blockmode);
                C_Connection(servaddr, sockfd, mode, forkmode, blockmode);
                status[sockfd] = _R_STUNO;
                available[sockfd] = 1;
                number[sockfd] = estb + 3;
                estb++;
                tlast = tcur;
            }
        }
        for (i = 3; i < estb + 3; i++)
        {

            if (available[i])
            {
                if (status[i] == _W_STUNO || status[i] == _W_PID || status[i] == _W_TIME)
                    *p = (char *)malloc(20 * sizeof(char));

                ret = C_ExcgMsg(i, blockmode, forkmode, mode, status[i], &size[i], p, number[i]);
                if (ret == _WRONGDATA || ret == _ERROR)
                {
                    close(i);
                    printf("communication fail(%d) : \n", number[i]);
                    if (ret == _WRONGDATA)
                        printf("wrongdata\n");
                    else
                        printf("error\n");

                    available[i] = 0;
                    sockfd = C_CreateSock(blockmode);
                    C_Connection(servaddr, sockfd, mode, forkmode, blockmode);
                    status[sockfd] = _R_STUNO;
                    available[sockfd] = 1;
                    number[sockfd] = number[i];
                    continue;
                    //remain--;
                }
                else if (ret == _WAITING)
                    continue;
                else
                {
                    if (status[i] == _R_END)
                    {
                        close(i);
                        printf("%d communication success\n", i);
                        available[i] = 0;
                        okay++;
                        //remain--;
                        CreateFile(forkthing[i].stuno, forkthing[i].pid, forkthing[i].time,
                                   forkthing[i].str, size[i]);
                        free(forkthing[i].str);
                    }
                    switch (status[i])
                    {
                    case _W_STUNO:
                        strcpy(forkthing[i].stuno, *p);
                        free(*p);
                        break;
                    case _W_TIME:
                        strcpy(forkthing[i].time, *p);
                        free(*p);
                        break;
                    case _W_PID:
                        strcpy(forkthing[i].pid, *p);
                        free(*p);
                        break;
                    case _W_STR:
                        forkthing[i].str = NULL;
                        forkthing[i].str = (char *)malloc(size[i] * sizeof(char));
                        if (forkthing[i].str == NULL)
                        {
                            close(i);
                            printf("communication fail(%d) : \n", number[i]);

                            available[i] = 0;
                            sockfd = C_CreateSock(blockmode);
                            C_Connection(servaddr, sockfd, mode, forkmode, blockmode);
                            status[sockfd] = _R_STUNO;
                            available[sockfd] = 1;
                            number[sockfd] = number[i];
                            continue;
                        }
                        memcpy(forkthing[i].str, *p, size[i] * sizeof(char));
                        //strcpy(forkthing[i].str, *p);
                        free(*p);
                        break;
                    default:
                        break;
                    }

                    status[i]++;
                }
            }
        }
    }
    free(p);

    return _SUCCESS;
}

void afterdead(int sig)
{
}

int C_CreateConncs(struct client_conf client)
{
    int sockfd;
    char recvline[BUFSIZ], sendline[BUFSIZ];
    pid_t pid;
    int i;
    //char ip[25];
    //int port;
    int n = client.num;
    int mode = client.select;
    int forkmode = client.fork;
    int blockmode = client.block;
    int ret;

    if (forkmode == _FORK)
    {
        i = 0;
        int status; //waitpid的状态判断

        signal(SIGCHLD, sig_handler);
        while (1)
        {
            if (i == n)
                break;
            pid = fork();
            if (pid < 0)
            {
                perror("Fork failed \n");
                exit(_ERROR);
            }
            if (pid > 0)
            {
                //parent process
                i++;
                waitpid(-1, &status, 0);

                if (WTERMSIG(status) == 9)
                {
                    i--;
                }

                continue;
            }
            if (pid == 0)
            {
                //child process
                sockfd = C_CreateSock(blockmode);
                C_Connection(client.server_addr, sockfd, mode, forkmode, blockmode);
                ret = C_ChangeState(sockfd, mode, forkmode, blockmode, _R_STUNO);
                if (ret == _WRONGDATA)
                {
                    printf("communication fail : wrongdata\n");
                    close(sockfd);
                    char s[20];
                    sprintf(s, "kill -9 %d", getpid());
                    system(s);
                }
                else if (ret == _ERROR)
                {
                    printf("communication fail : error\n");
                    close(sockfd);
                    char s[20];
                    sprintf(s, "kill -9 %d", getpid());
                    system(s);
                }
                else if (ret == _SUCCESS)
                {
                    printf("communication success\n");
                    close(sockfd);
                    char s[20];
                    sprintf(s, "kill -7 %d", getpid());
                    system(s);
                }
            }
        }
        while (1)
            ;
    }
    else
    {
        if (mode == _SELECT)
        {
            // int sockfd[1024];
            // for (i = 0; i < n; i++)
            // {
            //     sockfd[i] = C_CreateSock(blockmode);
            //     C_Connection(client.server_addr, sockfd[i], mode, forkmode, blockmode);
            // }
            C_ChangeState_Select(client.server_addr, mode, forkmode, blockmode, n);
        }
        if (mode == _POLL)
            //C_ChangeState_Select(client.server_addr, mode, forkmode, blockmode, n);

            C_ChangeState_Poll(client.server_addr, mode, forkmode, blockmode, n);
        if (mode == _EPOLL)
            C_ChangeState_EPOLL(client.server_addr, mode, forkmode, blockmode, n);
        //nofork
        //switch mode
        // sockfd = C_CreateSock(blockmode);
        // C_Connection(client.server_addr, sockfd, mode, forkmode, blockmode);
        // C_ExcgMsg(sockfd, mode, forkmode, blockmode);
    }
    struct timeval t1, t2;

    gettimeofday(&t1, NULL);
    while (1)
    {
        gettimeofday(&t2, NULL);
        if (t2.tv_sec - t1.tv_sec > 3)
            break;
    }
}
