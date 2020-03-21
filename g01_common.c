#include "g01_common.h"

/**********
���socket�������������1Ϊ���ӣ�0�Ͽ�
socket  �׽���������
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
��socket����Ϊ��������
iSock   �׽���������
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
���ĳ�ļ��������Ƿ�ɶ�-select��ʽ���ɶ�����1�����ɶ�����0
fd      �ļ�������
Maxfd   ָ��ǰ����ļ���������ֵ�����ڸ��ļ����������ܴ�δ����⣬
        ������к�Maxfd��ֵ���ܻ�ı䡣
        �״ε���ʱMaxfd����ָ���ֵӦΪ0
**********/
int Readable(int fd, int* Maxfd)
{
    fd_set rfds;
	FD_ZERO(&rfds);		//���
	FD_SET(fd,&rfds);	//���������	

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
���ĳ�ļ��������Ƿ��д-select��ʽ����д����1������д����0
fd      �ļ�������
Maxfd   ָ��ǰ����ļ���������ֵ�����ڸ��ļ����������ܴ�δ����⣬
        ������к�Maxfd��ֵ���ܻ�ı�
        �״ε���ʱMaxfd����ָ���ֵӦΪ0
**********/
int Writeable(int fd, int* Maxfd)
{
    fd_set wfds;
	FD_ZERO(&wfds);		//���
	FD_SET(fd,&wfds);	//���������	

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
���ĳ�ļ��������Ƿ�ɶ�-epoll��ʽ���ɶ�����1�����ɶ�����0
fd      �ļ�������
events  epoll_wait���ص�events����
num     event�������ݵĳ��ȣ�epoll_wait����ֵ��
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
���ĳ�ļ��������Ƿ��д-epoll��ʽ����д����1������д����0
fd      �ļ�������
events  epoll_wait���ص�events����
num     event�������ݵĳ��ȣ�epoll_wait����ֵ��
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
�źŴ�����
signo       ������ĺ���
**********/
void sig_handler(int signo)
{
    if(signo == SIGCHLD){
	    while (waitpid(-1, NULL, WNOHANG) > 0)
            ;
    }
}


/**********
д���ļ�
stuno       �յ���ѧ��
pid         �յ���pid
time        �յ���time
str         �յ��ĳ����ַ���
len         �����ַ����ĳ���
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
    if (-1 == fd)     //�����ļ�ʧ��
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
�����ļ���
**********/
int CreateFolder()
{
    //�ļ�������
    char folderName[] = "txt";

    // �ļ��в������򴴽��ļ���
    if(access(folderName,0)==-1)//access�����ǲ鿴�ļ��ǲ��Ǵ���
    {
        if (mkdir(folderName,0777))//��������ھ���mkdir����������
        {
            printf("creat folder failed!!!");
            return _ERROR;
        }
    }
    return _SUCCESS;
}