#include "g01_common.h"

/**********
监测socket连接情况，返回1为连接，0断开
socket  套接字描述符
**********/
int Connected(int socket)
{
    struct tcp_info info; 
        int info_len=sizeof(info); 
        getsockopt(socket, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&info_len); 
        if((info.tcpi_state == TCP_ESTABLISHED)) { 
            return 1;
        }
        else
        {
            char rbuf[2];
            if(read(socket,rbuf,2) == 0)
                return 1;

            return 0;
        }
}

/**********
将socket设置为非阻塞的
iSock   套接字描述符
**********/
int SetNonBlock(int iSock)
{
    int iFlags;

    iFlags = fcntl(iSock, F_GETFL, 0);
    iFlags |= O_NONBLOCK;
    iFlags |= O_NDELAY;
    int ret = fcntl(iSock, F_SETFL, iFlags);
    return ret;
}


/**********
监测某文件描述符是否可读-select方式，可读返回1，不可读返回0
fd      文件描述符
Maxfd   指向当前最大文件描述符的值，由于该文件描述符可能从未被监测，
        因此运行后Maxfd的值可能会改变。
        首次调用时Maxfd变量指向的值应为0
**********/
int Readable(int fd, int* Maxfd)
{
    fd_set rfds;
	FD_ZERO(&rfds);		//清空
	FD_SET(fd,&rfds);	//添加描述符	

    if(fd > *Maxfd){
        *Maxfd = fd;
    }

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	int slct_ans;
	if((slct_ans = select(*Maxfd+1, &rfds, NULL, NULL, &timeout))<0){
		perror("select error");
		exit(-1);
	}
	else if (slct_ans > 0){
		if (FD_ISSET(fd, &rfds))
            return 1;
        else
            return 0;
	}

	return 0;
}


/**********
监测某文件描述符是否可写-select方式，可写返回1，不可写返回0
fd      文件描述符
Maxfd   指向当前最大文件描述符的值，由于该文件描述符可能从未被监测，
        因此运行后Maxfd的值可能会改变
        首次调用时Maxfd变量指向的值应为0
**********/
int Writeable(int fd, int* Maxfd)
{
    fd_set wfds;
	FD_ZERO(&wfds);		//清空
	FD_SET(fd,&wfds);	//添加描述符	

    if(fd > *Maxfd){
        *Maxfd = fd;
    }

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	int slct_ans;
	if((slct_ans = select(*Maxfd+1, NULL, &wfds, NULL, &timeout))<0){
		perror("select error");
		exit(-1);
	}
	else if (slct_ans > 0){
		if (FD_ISSET(fd, &wfds))
            return 1;
        else
            return 0;
	}

	return 0;
}


/**********
监测某文件描述符是否可读-epoll方式，可读返回1，不可读返回0
fd      文件描述符
events  epoll_wait返回的events数组
num     event数组内容的长度（epoll_wait返回值）
**********/
int Epoll_Readable(int fd, struct epoll_event events[], int num)
{
    int i;
    for (i = 0; i < num; i++)
    {
        if ((fd == events[i].data.fd) && (events[i].events & EPOLLIN))
            return 1;
    }
    return 0;
}

/**********
监测某文件描述符是否可写-epoll方式，可写返回1，不可写返回0
fd      文件描述符
events  epoll_wait返回的events数组
num     event数组内容的长度（epoll_wait返回值）
**********/
int Epoll_Writeable(int fd, struct epoll_event events[], int num)
{
    int i;
    for (i = 0; i < num; i++)
    {
        if ((fd == events[i].data.fd) && (events[i].events & EPOLLOUT))
            return 1;
    }
    return 0;
}



/**********
信号处理函数
signo       待处理的函数
**********/
void sig_handler(int signo)
{
    if(signo == SIGCHLD){
	    while (waitpid(-1, NULL, WNOHANG) > 0)
            ;
    }
}


/**********
写入文件
stuno       收到的学号
pid         收到的pid
time        收到的time
str         收到的长长字符串
len         长长字符串的长度
**********/
int CreateFile(const char* stuno, const char* pid, const char* time, const char* str, int len)
{
    char filename[32];
    if(CreateFolder() == _ERROR)
        exit(-1); 
    sprintf(filename, "./txt/%s.%s.pid.txt", stuno, pid);
    /***
    FILE *fp=fopen("test.txt","a");
    fprintf(fp, "%s\n", filename);
    fclose(fp);
    ***/
    int fd;
    ssize_t wset;
    fd = open(filename, O_WRONLY|O_CREAT);
    if (-1 == fd)     //创建文件失败
    {   
        perror("create file failed\n");
        exit(-1); 
    }   

    wset = write(fd, stuno, strlen(stuno));
    if(wset<=0){
        perror("write to file");
        exit(-1);
    }

    wset = write(fd, "\n", 1);
    if(wset<=0){
        perror("write to file");
        exit(-1);
    }

    wset = write(fd, pid, strlen(pid));
    if(wset<=0){
        perror("write to file");
        exit(-1);
    }

    wset = write(fd, "\n", 1);
    if(wset<=0){
        perror("write to file");
        exit(-1);
    }

    wset = write(fd, time, strlen(time));
    if(wset<=0){
        perror("write to file");
        exit(-1);
    }

    wset = write(fd, "\n", 1);
    if(wset<=0){
        perror("write to file");
        exit(-1);
    }

    wset = write(fd, str, len);
    if(wset<=0){
        perror("write to file");
        exit(-1);
    }
    close(fd);
    return _SUCCESS;
    
}


/**********
建立文件夹
**********/
int CreateFolder()
{
    //文件夹名称
    char folderName[] = "txt";

    // 文件夹不存在则创建文件夹
    if(access(folderName,0)==-1)//access函数是查看文件是不是存在
    {
        if (mkdir(folderName,0777))//如果不存在就用mkdir函数来创建
        {
            printf("creat folder failed!!!");
            return _ERROR;
        }
    }
    return _SUCCESS;
}