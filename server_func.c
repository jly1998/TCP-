
#include "g01_server.h"

#define _BACKLOG_ 1024

extern int exit_sig;
/**********
��������socket������������
server_socket   ���ýṹ��
blockmode   ����/������ ����2ѡ1
**********/
int S_CreateListenSock(struct sockaddr_in server_socket, int blockmode)
{
    int flag = 1;
	//�����׽���
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("create socket failed");
		exit(-1);
	}
    //	printf("creat socket success\n");

    if(_NONBLOCK == blockmode){
        //���÷�����
        SetNonBlock(sock);
        //"listen socket"
    }


	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0) 
    { 
        printf("set socket option error");
        exit(-1);
    } 
	
	//��
	int len = sizeof(server_socket);
	if(bind(sock, (struct sockaddr*)&server_socket, (socklen_t)len) < 0)
	{
		perror("bind error!");
		exit(-1);
	}
    //	printf("bind success\n");
 
	//����
	if(listen(sock, _BACKLOG_) < 0)
	{
		perror("listen");
		exit(-1);
	}
	printf("Listen status, waiting for connection request...\n");
	return sock;
}


/**********
��������
listen_sock      ����socket
mode    �Զ�����ӵ�ά����ʽ ����5ѡ1
blockmode   ����/������ ����2ѡ1
**********/
int S_AcConncs(int listen_sock, int mode, int forkmode, int blockmode)
{
    srand((unsigned)time(NULL));
    int peer_sock[1024], i;
	struct sockaddr_in connc;
	int sock_len;
    int CurNum = 0;     //��ǰ�ɹ����µ�������

    if(_FORK == forkmode){
        signal(SIGCHLD, sig_handler);
        while(1){
            peer_sock[i] = accept(listen_sock, (struct sockaddr *)&connc, (socklen_t*)&sock_len);

            if (peer_sock[i] > 0){
                //��ȡ�����ip���˿ں���Ϣ
                struct sockaddr_in peer;
                socklen_t len = sizeof(peer);
                if(!getpeername(peer_sock[i], (struct sockaddr *)&peer, &len))
                {
                    printf("Accept connection%d from %s:%d\n", i, inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
                    fflush(stdout);
                }
                else{
                    perror("getpeername failed");
                    printf("accept again\n");
                    continue;
                }   

                //����
                if(_NONBLOCK == blockmode){
                   // char temp[32];
                   // sprintf(temp, "accept_socket%d", i);
                    SetNonBlock(peer_sock[i]);
                }

                pid_t pid;
                pid = fork();       //fork�ӽ��������������շ�����
                if(pid<0){
                    perror("fork failed");
                    continue;
                }
                else if(pid == 0){
	                prctl(PR_SET_PDEATHSIG,SIGKILL);
                    struct client_info sub_info;
                    sub_info.str = NULL;
                    int ret = S_Connection(peer_sock[i], mode, blockmode, &sub_info);
                    
                    close(peer_sock[i]);
                    if(_SUCCESS == ret){
                        CreateFile(sub_info.stuno, sub_info.pid, sub_info.time, sub_info.str, sub_info.Num.Len);
                            
                        if(sub_info.str){
                            free(sub_info.str);
                            sub_info.str = NULL;
                        }
                    }
                    else
                    {
                        printf("connection(sock:%d) close unnormally.\n", i);
                        fflush(stdout);
                    }
                    
                    exit(0);
                    break;          //�ӽ����뿪ѭ��
                }
                i++;
            }//if >0
            else if(_BLOCK == blockmode){
                perror("accept error");
                exit(-1);
            }
        }//while
        
    }//if _FORK
    else {
        if(_SELECT == mode)
        {
            int i, j;
            int Maxfd = listen_sock;
            //�½���������  
            int CurSock = 0;
            fd_set rfds, wfds;
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            struct client_info c_info[1027];
            int CurConnection = 0;

            for(j=0; j<1027; j++)
            {
                c_info[j].used = FALSE;
            }

            while (1)
            {
                FD_ZERO(&rfds);
                FD_ZERO(&wfds);
                    
                FD_SET(listen_sock, &rfds);
                FD_SET(listen_sock, &wfds);

                for(j=4; j<1027; j++)
                {
                    if(c_info[j].used){
                        FD_SET(j, &rfds);
                        FD_SET(j, &wfds);
                    }
                }

                int flag = select(Maxfd + 1, &rfds, &wfds, NULL, &timeout); 
                if (flag < 0)
                {
                    perror("select error");
                    continue;
                }
                else
                {
                    for (i = 4; i < 1027; i++)
                    {
                        //����������
                        if(c_info[i].used){
                            //һ�ζ���д
                            int ret;
                            char rbuf[32], *result;
                            int Status = c_info[i].status;
                            
                            if(_RSTR == Status){
                                result = c_info[i].str;
                            }
                            else{
                                result = rbuf;
                            }

                            //              ������        �ɶ�                    д����          ��д
                            if (((Status % 2 == 1) && FD_ISSET(i, &rfds)) || ((Status % 2 == 0) && FD_ISSET(i, &wfds))){
                                ret = S_DataTrans(i, mode, blockmode, Status, result, &(c_info[i].Num));
                                if(_ERROR == ret || _WRONGDATA == ret){
                                    printf("connection(sock:%d) close unnormally.\n", i);
                                    fflush(stdout);
                                    if(c_info[i].str){
                                        free(c_info[i].str);
                                        c_info[i].str = NULL;
                                    }
                                    
                                    c_info[i].used = FALSE;
                                    CurConnection--;
                                    
                                    close(i);
                                    continue;
                                }

                                if(_WAITING == ret){
                                    if(_WAITCLOSE == Status){
                                        printf("waiting sock:%d to close\n",i);
                                        fflush(stdout);
                                    }
                                    continue;
                                    
                                }

                                if(_WEND == Status){
                                    printf("Send end\n");
                                    fflush(stdout);
                                }
                                if(_WAITCLOSE == Status){
                                    printf("communication success, connection(sock:%d) close.\n", i);
                                    fflush(stdout);
                                    //���ˣ�
                                    c_info[i].used = FALSE;
                                    
                                    close(i);
                                    CreateFile(c_info[i].stuno, c_info[i].pid, c_info[i].time, c_info[i].str, c_info[i].Num.Len);
                                    
                                    if(c_info[i].str){
                                        free(c_info[i].str);    
                                        c_info[i].str = NULL;
                                    }
                                    
                                    CurNum++;
                                    CurConnection--;
                                    printf("Current: %d Success\n", CurNum);
                                    fflush(stdout);
                                }

                                if(_WSTR == Status){
                                    printf("Ask for %dB str\n", c_info[i].Num.Len);
                                    fflush(stdout);
                                    while(1){
                                        c_info[i].str = (char *)malloc(sizeof(char)*(c_info[i].Num.Len+1));
                                        if(!c_info[i].str)
                                            perror("malloc");
                                        else
                                            break;
                                        
                                    }
                                } 

                                //��ȡ��ɺ��ݴ�
                                if(_RSTUNO == Status){
                                    strcpy(c_info[i].stuno, result);
                                } 
                                if(_RPID == Status){
                                    strcpy(c_info[i].pid, result);
                                } 
                                if(_RTIME== Status){
                                    strcpy(c_info[i].time, result);
                                } 


                                c_info[i].status++;
                            }//if able
                        }//if user
                    }//for

                    if (FD_ISSET(listen_sock, &rfds) && CurConnection < _MAXC)
                    {
                        printf("found new request from client\n");
                        fflush(stdout);
                        struct sockaddr_in connc;
                        int connc_len;
                        CurSock = accept(listen_sock, (struct sockaddr *)&connc, (socklen_t*)&connc_len);

                        if (CurSock == -1)
                        {
                            perror("accept error");
                            continue;
                        }

                        //����maxfd
                        if(CurSock > Maxfd){
                            Maxfd = CurSock;
                        }
                        c_info[CurSock].status = 0;
                        //�����ˣ�
                        c_info[CurSock].used = TRUE;
                        c_info[CurSock].str = NULL;
                        //����
                        CurConnection++;

                        struct sockaddr_in peer;
                        socklen_t len = sizeof(peer);
                        if(!getpeername(CurSock, (struct sockaddr *)&peer, &len))
                        {
                            printf("Accept connection from %s:%d, this is No.%d\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port), CurSock);
                            fflush(stdout);
                            SetNonBlock(CurSock);
                        }
                    }
                }
            }//while
        }//if _SELECT
        else if(_POLL == mode)
        {
            int i, j;
            int Maxfd = listen_sock;
            //�½���������  
            int CurSock = 0;
            fd_set rfds, wfds;
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            struct client_info c_info[1027];
            struct pollfd poll_fds[1024];
            int pollnum[1027];

            int CurConnection = 0;

            for(j=0; j<1027; j++)
            {
                c_info[j].used = FALSE;
            }

            while (1)
            {
                memset(poll_fds, 0, sizeof(struct pollfd)*1024); 
                poll_fds[0].fd = listen_sock;           //�Ѽ����׽��ַ�������
                poll_fds[0].events = POLLIN;            //���ù�ע���¼�
                
                int use_now = 1;
                pollnum[3]=0;

                for (j = 0; j < 1027; j++) 
                {
                    if(c_info[j].used){
                        poll_fds[use_now].fd = j;             //�����׽��ּ�������
                        poll_fds[use_now].events = POLLIN | POLLOUT;   //���ù�ע��/д�¼�
                        pollnum[j]=use_now;
                        use_now++;
                    }
                }
                
                
                int flag = poll(poll_fds, CurConnection + 1, 0);
                if (flag < 0)
                {
                    perror("poll error");
                    continue;
                }
                else
                {
                    for (i = 4; i < 1027; i++)
                    {
                        //����������
                        if(c_info[i].used){
                            //һ�ζ���д
                            int ret;
                            char rbuf[32], *result;
                            int Status = c_info[i].status;
                            
                            if(_RSTR == Status){
                                result = c_info[i].str;
                            }
                            else{
                                result = rbuf;
                            }

                            //              ������        �ɶ�                              д����          ��д
                            if (((Status % 2 == 1) && (poll_fds[pollnum[i]].revents & POLLIN)) || ((Status % 2 == 0) && (poll_fds[pollnum[i]].revents & POLLOUT))){
                                ret = S_DataTrans(i, mode, blockmode, Status, result, &(c_info[i].Num));
                                if(_ERROR == ret || _WRONGDATA == ret){
                                    printf("connection(sock:%d) close unnormally.\n", i);
                                    fflush(stdout);
                                    if(c_info[i].str){
                                        free(c_info[i].str);
                                        c_info[i].str = NULL;
                                    }
                                    
                                    c_info[i].used = FALSE;
                                    CurConnection--;
                                    
                                    close(i);
                                    continue;
                                }

                                if(_WAITING == ret){
                                    if(_WAITCLOSE == Status){
                                        printf("waiting sock:%d to close\n",i);
                                        fflush(stdout);
                                    }
                                    continue;
                                    
                                }

                                if(_WEND == Status){
                                    printf("Send end\n");
                                    fflush(stdout);
                                }
                                if(_WAITCLOSE == Status){
                                    printf("communication success, connection(sock:%d) close.\n", i);
                                    fflush(stdout);
                                    //���ˣ�
                                    c_info[i].used = FALSE;
                                    
                                    close(i);
                                    CreateFile(c_info[i].stuno, c_info[i].pid, c_info[i].time, c_info[i].str, c_info[i].Num.Len);
                                    
                                    if(c_info[i].str){
                                        free(c_info[i].str);    
                                        c_info[i].str = NULL;
                                    }
                                    
                                    CurNum++;
                                    CurConnection--;
                                    printf("Current: %d Success\n", CurNum);
                                    fflush(stdout);
                                }

                                if(_WSTR == Status){
                                    printf("Ask for %dB str\n", c_info[i].Num.Len);
                                    fflush(stdout);
                                    while(1){
                                        c_info[i].str = (char *)malloc(sizeof(char)*(c_info[i].Num.Len+1));
                                        if(!c_info[i].str)
                                            perror("malloc");
                                        else
                                            break;
                                        
                                    }
                                } 

                                //��ȡ��ɺ��ݴ�
                                if(_RSTUNO == Status){
                                    strcpy(c_info[i].stuno, result);
                                } 
                                if(_RPID == Status){
                                    strcpy(c_info[i].pid, result);
                                } 
                                if(_RTIME== Status){
                                    strcpy(c_info[i].time, result);
                                } 


                                c_info[i].status++;
                            }//if able
                        }//if user
                    }//for

                    if ((poll_fds[0].revents & POLLIN) && CurConnection < _MAXC)
                    {
                        printf("found new request from client\n");
                        fflush(stdout);
                        struct sockaddr_in connc;
                        int connc_len;
                        CurSock = accept(listen_sock, (struct sockaddr *)&connc, (socklen_t*)&connc_len);

                        if (CurSock == -1)
                        {
                            perror("accept error");
                            continue;
                        }

                        c_info[CurSock].status = 0;
                        //�����ˣ�
                        c_info[CurSock].used = TRUE;
                        c_info[CurSock].str = NULL;
                        //����
                        CurConnection++;

                        struct sockaddr_in peer;
                        socklen_t len = sizeof(peer);
                        if(!getpeername(CurSock, (struct sockaddr *)&peer, &len))
                        {
                            printf("Accept connection from %s:%d, this is No.%d\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port), CurSock);
                            fflush(stdout);
                            SetNonBlock(CurSock);
                        }
                    }
                }
            }//while
        }//if _POLL
        else if(_EPOLL == mode)
        {
            int i, j;
            int Maxfd = listen_sock;
            //�½���������  
            int CurSock = 0;
            fd_set rfds, wfds;
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            struct client_info c_info[1027];

            struct epoll_event epoll_fds[1024];
            
            int efd;
            efd = epoll_create1(0);

            int CurConnection = 0;

            for(j=0; j<1027; j++)
            {
                c_info[j].used = FALSE;
            }

            //�������socket
            struct epoll_event epoll_temp;
            epoll_temp.data.fd = listen_sock; 
            epoll_temp.events = EPOLLIN;
                
            int epoll_ret = epoll_ctl(efd, EPOLL_CTL_ADD, listen_sock, &epoll_temp);
            if (epoll_ret == -1)
            {
                perror("epoll_ctl");
                exit(-1);
            }

            while (1)
            {
                int epoll_num = epoll_wait(efd, epoll_fds, 1024, 0);

                if (Epoll_Readable(listen_sock, epoll_fds, epoll_num) && CurConnection < _MAXC)
                {
                    printf("found new request from client\n");
                    fflush(stdout);
                    struct sockaddr_in connc;
                    int connc_len;
                    CurSock = accept(listen_sock, (struct sockaddr *)&connc, (socklen_t*)&connc_len);

                    if (CurSock == -1)
                    {
                        perror("accept error");
                        continue;
                    }

                    //�������
                    struct epoll_event epoll_temp;
                    epoll_temp.data.fd = CurSock;
                    epoll_temp.events = EPOLLIN | EPOLLOUT;
                    int epoll_ret = epoll_ctl(efd, EPOLL_CTL_ADD, CurSock, &epoll_temp);
                    if (epoll_ret == -1)
                    {
                        perror("epoll_ctl");
                        close(CurSock);
                        continue;
                    }

                    c_info[CurSock].status = 0;
                    //�����ˣ�
                    c_info[CurSock].used = TRUE;
                    c_info[CurSock].str = NULL;
                    //����
                    CurConnection++;

                    struct sockaddr_in peer;
                    socklen_t len = sizeof(peer);
                    if(!getpeername(CurSock, (struct sockaddr *)&peer, &len))
                    {
                        printf("Accept connection from %s:%d, this is No.%d\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port), CurSock);
                        fflush(stdout);
                        SetNonBlock(CurSock);
                    }

                    
                }//������

                for (i = 4; i < 1027; i++)
                {
                    //����������
                    if(c_info[i].used){
                        //һ�ζ���д
                        int ret;
                        char rbuf[32], *result;
                        int Status = c_info[i].status;
                            
                        if(_RSTR == Status){
                            result = c_info[i].str;
                        }
                        else{
                            result = rbuf;
                        }

                        //              ������        �ɶ�                              д����          ��д
                        if (((Status % 2 == 1) && Epoll_Readable(i, epoll_fds, epoll_num)) || ((Status % 2 == 0) && Epoll_Writeable(i, epoll_fds, epoll_num))){
                            ret = S_DataTrans(i, mode, blockmode, Status, result, &(c_info[i].Num));
                            if(_ERROR == ret || _WRONGDATA == ret){
                                printf("connection(sock:%d) close unnormally.\n", i);
                                fflush(stdout);
                                if(c_info[i].str){
                                    free(c_info[i].str);
                                    c_info[i].str = NULL;
                                }
                                    
                                c_info[i].used = FALSE;
                                CurConnection--;
                                //ɾ��
                                int epoll_ret = epoll_ctl(efd, EPOLL_CTL_DEL, i, NULL);
                                if (epoll_ret == -1)
                                {
                                    perror("epoll_ctl");
                                    exit(-1);
                                }
                                close(i);
                                continue;
                            }

                            if(_WAITING == ret){
                                if(_WAITCLOSE == Status){
                                    printf("waiting sock:%d to close\n",i);
                                    fflush(stdout);
                                }
                                continue;
                                    
                            }

                            if(_WEND == Status){
                                printf("Send end\n");
                                fflush(stdout);
                            }
                            if(_WAITCLOSE == Status){
                                printf("communication success, connection(sock:%d) close.\n", i);
                                fflush(stdout);
                                //���ˣ�
                                c_info[i].used = FALSE;
                                //ɾ��
                                int epoll_ret = epoll_ctl(efd, EPOLL_CTL_DEL, i, NULL);
                                if (epoll_ret == -1)
                                {
                                    perror("epoll_ctl");
                                    exit(-1);
                                }

                                close(i);
                                CreateFile(c_info[i].stuno, c_info[i].pid, c_info[i].time, c_info[i].str, c_info[i].Num.Len);
                                
                                if(c_info[i].str){
                                    free(c_info[i].str);    
                                    c_info[i].str = NULL;
                                }
                                    
                                CurNum++;
                                CurConnection--;
                                printf("Current: %d Success\n", CurNum);
                               fflush(stdout);
                            }

                            if(_WSTR == Status){
                                printf("Ask for %dB str\n", c_info[i].Num.Len);
                                fflush(stdout);
                                while(1){
                                    c_info[i].str = (char *)malloc(sizeof(char)*(c_info[i].Num.Len+1));
                                    if(!c_info[i].str)
                                        perror("malloc");
                                    else
                                        break;
                                        
                                }
                            } 

                            //��ȡ��ɺ��ݴ�
                            if(_RSTUNO == Status){
                                strcpy(c_info[i].stuno, result);
                            } 
                            if(_RPID == Status){
                                strcpy(c_info[i].pid, result);
                            } 
                            if(_RTIME== Status){
                                strcpy(c_info[i].time, result);
                            } 


                            c_info[i].status++;
                        }//if able
                    }//if user
                }//for

            }//while
        }//if _EPOLL
    }
}//S_AcConncs

/**********
��ȡһ��32768-99999֮��������
**********/
int GetRand()
{
    //srand((unsigned)time(NULL));
    int a;
    a = rand() % 67232 + 32768;  
    return a;
}

/**********
�����ַ���������0������-1�������
socket      socket������
blockmode   ����ģʽ
str     �ַ���
tzero   0����β�㣬1��β��
**********/
int SendStr(int socket, int blockmode, const char* str, int tzero)
{
    ssize_t wset;
    char wbuf[16];
    strcpy(wbuf, str);

    
    wset = write(socket, wbuf, strlen(wbuf)+tzero);
    if(wset > 0){
        //printf("Write %s, return %d\n", wbuf, wset);
        return _SUCCESS;
    }
    if(wset == 0){
        printf("Write 0B, error\n");
        fflush(stdout);
        return _ERROR;
    }
    if(_NONBLOCK == blockmode && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
    {
        return _WAITING;
    }
    else
    {
        perror("Write failed");
        return _ERROR;
    }
    
}

/**********
���������շ����ݣ�����0������-1�������1���������������
socket      socket������
mode    �Զ�����ӵ�ά����ʽ ����3ѡ1
blockmode   ����/������ ����2ѡ1
**********/
int S_Connection(int socket, int mode, int blockmode, struct client_info* c_info)
{
    int status = _WSTUNO;
    char *result, rbuf[32];
    int ret;
    while(1){

        if(_RSTR == status){
            result = c_info->str;
        }
        else{
            result = rbuf;
        }
        
        ret = S_DataTrans(socket, mode, blockmode, status, result, &(c_info->Num));
        if(_ERROR == ret || _WRONGDATA == ret){
            printf("connection(sock:%d) close unnormally.\n", socket);
            fflush(stdout);
            if(c_info->str){
                free(c_info->str);
                c_info->str = NULL;
            }
            return ret;
        }

        if(_WAITING == ret)
            continue;

        //����Ϊ�ɹ�
        if(_WAITCLOSE == status){
            printf("communication success, connection(sock:%d) close.\n", socket);
            fflush(stdout);
            return ret;
        }

        if(_WSTR == status){
            printf("Ask for %dB str\n", c_info->Num.Len);
            fflush(stdout);
            while(1){
                c_info->str = (char *)malloc(sizeof(char)*(c_info->Num.Len+1));
                if(!c_info->str)
                    perror("wstr malloc");
                else
                    break;
                
            }
        }

        //��ȡ��ɺ��ݴ�
        if(_RSTUNO == status){
            strcpy(c_info->stuno, result);
        } 
        if(_RPID == status){
            strcpy(c_info->pid, result);
        } 
        if(_RTIME== status){
            strcpy(c_info->time, result);
        } 


        status++;
        
    }//switch



}//S_Connection



/**********
�շ����ݼ���
socket      socket������
mode        �Զ�����ӵ�ά����ʽ ����3ѡ1
blockmode   ����/������ ����2ѡ1
status      ״̬
result      �����Ľ��
**********/
int S_DataTrans(int socket, int mode, int blockmode, int status, char* result, struct str_info *number)
{
    int rset, wset;
    int no, no_tmp;
    char *temp;
    char wbuf[32], rbuf[32];
    switch (status)
    {
    case _WSTUNO:       //Send "StuNo"
        return SendStr(socket, blockmode, "StuNo", 0);
        break;

    case _RSTUNO:       //Recv no
        rset = read(socket, &no_tmp, 4);
        if(rset > 0){
            /***
            if(rset != 7){
                printf("Received data is no a student number, connection close.\n");
                return _WRONGDATA;
            }***/

            no = ntohl(no_tmp);
            printf("StuNo read %dB: %d\n", rset, no);
            fflush(stdout);
            //�������
            sprintf(result, "%d", no);
            fflush(stdout);
            return _SUCCESS;
        }//if
        if(rset == 0)
        {
            printf("StuNo read 0B, error\n");
            fflush(stdout);
            return _ERROR;
        }
        
        if(_NONBLOCK == blockmode && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
        {
            return _WAITING;
        }
        else
        {
            perror("StuNo read failed");
            return _ERROR;
        }
        break;

    case _WPID:         //send "pid"
        return SendStr(socket, blockmode, "pid", 0);
        break;

    case _RPID:         //recv pidɶ��
        rset = read(socket, &no_tmp, 4);
        if(rset > 0){
            /***
             if(rset != 7){
                printf("Received data is no a student number, connection close.\n");
                return _WRONGDATA;
            }***/
            no = ntohl(no_tmp);
            printf("pid read %dB: %d\n", rset, no);
            fflush(stdout);
            //�������
            sprintf(result, "%d", no);
            fflush(stdout);
            return _SUCCESS;
        }
        if(rset == 0)
        {
            printf("pid read 0B, error\n");
            fflush(stdout);
            return _ERROR;
        }
        if(_NONBLOCK == blockmode && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
        {
            return _WAITING;
        }
        else
        {
            perror("pid read failed");
            return _ERROR;
        }
        break;

    case _WTIME:        //send "TIME\0"
        return SendStr(socket, blockmode, "TIME", 1);
        break;

    case _RTIME:        //recv time
        rset = read(socket, rbuf, 19);
        if(rset > 0){
            rbuf[rset] = '\0';
            printf("TIME read %dB: %s\n", rset, rbuf);
            fflush(stdout);
            //�������
            memcpy(result, rbuf, rset + 1);
            return _SUCCESS;
        }
        if(rset == 0)
        {
            printf("TIME read 0B, error\n");
            fflush(stdout);
            return _ERROR;
        }
        if(_NONBLOCK == blockmode && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
        {
            return _WAITING;
        }
        else
        {
            perror("TIME read failed");
            return _ERROR;
        }
        break;

    case _WSTR:         //send "str*****\0"
        number->Len = GetRand();
        sprintf(wbuf, "str%5d", number->Len);
        number->Cur = 0;
        return SendStr(socket, blockmode, wbuf, 1);
        break;

    case _RSTR:         //recv *****    
        temp = (char *)malloc((number->Len - number->Cur) * sizeof(char) + 1);
        if(!temp){
            perror("rstr malloc failed");
            return _ERROR;
        }
        rset = read(socket, temp, number->Len - number->Cur);
        if(rset > 0){
            if(number->Cur + rset < number->Len){
            //    printf("sock:%d read %dB, %dB in total\n", socket, rset, number->Cur + rset);
                //�������
                memcpy(result + number->Cur, temp, rset);
                number->Cur += rset;
                if(temp){
                    free(temp);
                    temp = NULL;
                }
                return _WAITING;
            }
            if(number->Cur + rset > number->Len){
                printf("Received %dB in total, number should be %dB, connection close.\n", number->Cur + rset, number->Len);
                fflush(stdout);
                if(temp){
                    free(temp);
                    temp = NULL;
                }
                return _WRONGDATA;
            }
            

            temp[rset] = '\0';
            printf("sock:%d read %dB, %dB in total, success\n", socket, rset, number->Cur + rset);
            fflush(stdout);
            //�������
            memcpy(result + number->Cur, temp, rset);
            number->Cur += rset;
            if(temp){
                free(temp);
                temp = NULL;
            }
            return _SUCCESS;
        }
        if(rset == 0){
            printf("read 0B, error\n");
            fflush(stdout);
            return _ERROR;
        }
        if(_NONBLOCK == blockmode && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
        {
            return _WAITING;
        }
        else
        {
            perror("read failed");
            return _ERROR;
        }
        break;

    case _WEND:         //send "end"
        return SendStr(socket, blockmode, "end", 0);
        break;

    case _WAITCLOSE:    //close?
        //if(!Connected(socket)){
        if(read(socket, rbuf, sizeof(rbuf)) == 0){
            //printf("client closed the connection\n");
            return _SUCCESS;
        }
        else{
            return _WAITING;
        }
        break;

    default:
        return _ERROR;
        break;
    }//switch
    return _ERROR;
}//S_DataTrans
