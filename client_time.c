#include "g01_client.h"

/***
struct timeval 变量名
gettimeofday( &变量名,NULL);
获取当前时间

connect之前：
获取一下tcur 调用下看看能不能连

每次connect成功：
tlast = tcur

第一次运行之前（while外面）tlast初值搞成0应该就行？？
***/

/**********
隔一段时间连一个
tcur    当前时间
tlast   上次连接成功的时间
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


