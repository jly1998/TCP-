#include "g01_client.h"

/***
struct timeval ������
gettimeofday( &������,NULL);
��ȡ��ǰʱ��

connect֮ǰ��
��ȡһ��tcur �����¿����ܲ�����

ÿ��connect�ɹ���
tlast = tcur

��һ������֮ǰ��while���棩tlast��ֵ���0Ӧ�þ��У���
***/

/**********
��һ��ʱ����һ��
tcur    ��ǰʱ��
tlast   �ϴ����ӳɹ���ʱ��
**********/

int CanIConnect(struct timeval tcur, struct timeval tlast)
{
    int timegap;
    timegap =1000000 * ( tcur.tv_sec -tlast.tv_sec) + tcur.tv_usec -tlast.tv_usec;
    if(timegap > rand()%15+1)
        return 1;
    else
    {
        return 0;
    }
    
}


