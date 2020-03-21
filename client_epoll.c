#include "g01_client.h"

int C_ReadMsg_EPOLL(int sockfd, int step, int blockmode)
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

int C_ExcgMsg_EPOLL(int sockfd, int blockmode, int forkmode, int mode, int state, int *size, char **p, int sockfdtocreatefile, struct epoll_event events[], int num)
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
        if (mode == _EPOLL && !(Epoll_Readable(sockfd, events,num)))
        {
         printf("不可读%d\n",mode);
            return _WAITING;
        }
        printf("我有错\n");
        ret = C_ReadMsg_EPOLL(sockfd, _R_STUNO, blockmode);
        printf("*%d\n", ret);
        if (ret != _SUCCESS)
            return ret;
        break;

    case _W_STUNO:
        if (mode == _EPOLL && !(Epoll_Writeable(sockfd, events, num)))
            return _WAITING;
        n = write(sockfd, &StuNo, 4);
        if (n == 0)
            return _ERROR;

        printf("send msg to server : 1750762\n");
        fflush(stdout);
        strcpy(*p, "1750762");
        break;

    case _R_PID:
        if (mode == _EPOLL && !(Epoll_Readable(sockfd, events, num)))
            return _WAITING;
        ret = C_ReadMsg_EPOLL(sockfd, _R_PID, blockmode);
        if (ret != _SUCCESS)
            return ret;
        break;

    case _W_PID:
        if (mode == _EPOLL && !(Epoll_Writeable(sockfd, events, num)))
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
        if (mode == _EPOLL && !(Epoll_Readable(sockfd, events, num)))
            return _WAITING;
        ret = C_ReadMsg_EPOLL(sockfd, _R_TIME, blockmode);
        if (ret != _SUCCESS)
            return ret;
        break;

    case _W_TIME:
        if (mode == _EPOLL && !(Epoll_Writeable(sockfd, events, num)))
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
        if (mode == _EPOLL && !(Epoll_Readable(sockfd, events, num)))
            return _WAITING;
        ret = C_ReadMsg_EPOLL(sockfd, _R_STR, blockmode);
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
            if (mode == _EPOLL && !(Epoll_Writeable(sockfd, events, num)))
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
        if (mode == _EPOLL && !(Epoll_Readable(sockfd, events, num)))
            return _WAITING;
        ret = C_ReadMsg_EPOLL(sockfd, _R_END, blockmode);
        if (ret != _SUCCESS)
            return ret;
        break;

    default:
        break;
    }

    return _SUCCESS;
}

int C_ChangeState_EPOLL(struct sockaddr_in servaddr, int mode, int forkmode, int blockmode, int num)
{
    int sockfd;
    int i, k;
    int status[1024];
    int available[1024];
    int size[1024] = {0};
    struct to_make_file forkthing[1024];
    char **p = (char **)malloc(sizeof(char *));
    int number[1024];
    struct epoll_event epoll_fds[1000]; //epoll结构体数组
    int efd;
    // memset(status, _R_STUNO, 1024 * sizeof(int));
    // memset(available, 1, 1024 * sizeof(int));
    struct timeval tcur;
    struct timeval tlast;
    efd = epoll_create1(0);
    if (efd == -1)
    {
        perror("epoll_create");
        exit(0);
    }

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
                //printf("%d\n", sockfd);
                    number[sockfd] = estb + 3;
                estb++;
                tlast = tcur;

                struct epoll_event epoll_temp;
                epoll_temp.data.fd = sockfd;
                epoll_temp.events = EPOLLIN | EPOLLOUT;
                int epoll_ret = epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &epoll_temp);
                if (epoll_ret == -1)
                {

                    perror("epoll_ctl");
                    return -1;
                }
            }
        }

        int epoll_return = epoll_wait(efd, epoll_fds, 1000, 0);
        //printf("%d\n", epoll_return);
        //printf("$$%d\n", available[3]);

        for (i = 4; i < estb + 4; i++)
        {

            //printf("&&%d\n", available[3]);
            if (available[i])
            {
                //printf("%d\n", available[3]);
                if (status[i] == _W_STUNO || status[i] == _W_PID || status[i] == _W_TIME)
                    *p = (char *)malloc(20 * sizeof(char));
                //printf("99999\n");
                ret = C_ExcgMsg_EPOLL(i, blockmode, forkmode, mode, status[i], &size[i], p, number[i], epoll_fds, epoll_return);
                if (ret == _WRONGDATA || ret == _ERROR)
                {
                    int epoll_ret = epoll_ctl(efd, EPOLL_CTL_DEL, i, NULL);
                    if (epoll_ret == -1)
                    {
                        perror("epoll_ctl");
                        return -1;
                    }
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
                    struct epoll_event epoll_temp;
                    epoll_temp.data.fd = sockfd;
                    epoll_temp.events = EPOLLIN | EPOLLOUT;
                    epoll_ret = epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &epoll_temp);
                    if (epoll_ret == -1)
                    {

                        perror("epoll_ctl");
                        return -1;
                    }

                    continue;
                    //remain--;
                }
                else if (ret == _WAITING)
                    continue;
                else
                {
                    if (status[i] == _R_END)
                    {
                        int epoll_ret = epoll_ctl(efd, EPOLL_CTL_DEL, i, NULL);
                        if (epoll_ret == -1)
                        {
                            perror("epoll_ctl");
                            return -1;
                        }
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
                        //free(*p);
                        break;
                    case _W_TIME:
                        strcpy(forkthing[i].time, *p);
                        //free(*p);
                        break;
                    case _W_PID:
                        strcpy(forkthing[i].pid, *p);
                        //free(*p);
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
                        // free(*p);
                        break;
                    default:
                        break;
                    }

                    status[i]++;
                }
            }
        }
        //free(p);
    }

    return _SUCCESS;
}
