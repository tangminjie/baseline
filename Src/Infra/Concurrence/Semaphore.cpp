/************************************************
 * Copyright(c) 2019 Sang Yang
 * 
 * Project:  Framework
 * FileName: Semaphore.cpp
 * Author: xieshren
 * Email: xieshren@gmail.com
 * Version: V1.0.0
 * Date: 2021-01-02 20:25
 * Description: 
 * Others:
 *************************************************/


#include <ctime>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include "Logger/Define.h"
#include "Infra/Concurrence/Semaphore.h"

namespace Uface {
namespace Infra {

struct CSemaphore::SemaphoreInternal {
    pthread_condattr_t		mCondAttr;
    pthread_mutex_t			mtx;
    pthread_cond_t			mCv;
    int32_t					mCount;
};


CSemaphore::CSemaphore(int32_t initialCount) {
    mInternal = new SemaphoreInternal;

    int32_t ret = pthread_condattr_init(&mInternal->mCondAttr);
    UFACE_ASSERT(ret == 0,"pthread cond attribute initial failed");
    ret = pthread_condattr_setclock(&mInternal->mCondAttr, CLOCK_MONOTONIC);
    UFACE_ASSERT(ret == 0,"pthread cond set clock attribute failed");
    /* initialize a condition variable to its default value */
    ret = pthread_cond_init(&mInternal->mCv, &mInternal->mCondAttr);
    UFACE_ASSERT(ret == 0,"pthread cond initial failed");
    /* initialize a condition variable */
    ret = pthread_mutex_init(&mInternal->mtx, nullptr);
    UFACE_ASSERT(ret == 0,"pthread cond initial mutex failed");
    /**消除变量未使用编译告警*/
    (void)ret;
    assert(initialCount >= 0);
    mInternal->mCount = initialCount;
}

CSemaphore::~CSemaphore() {
    int32_t ret = pthread_mutex_destroy(&mInternal->mtx);
    UFACE_ASSERT(ret == 0,"pthread destroy cond mutex failed");
    (void)ret;
    ret = pthread_cond_destroy(&mInternal->mCv);
    UFACE_ASSERT(ret == 0,"pthread destroy cond failed");
    delete mInternal;
    mInternal = nullptr;
}

int32_t CSemaphore::pend() {

    int32_t ret = pthread_mutex_lock(&mInternal->mtx);
    while (mInternal->mCount == 0 && 0 == ret) {
        ret = pthread_cond_wait(&mInternal->mCv, &mInternal->mtx);
    }

    if(ret!=0){
        pthread_mutex_unlock(&mInternal->mtx);
        return -1;
    }

    mInternal->mCount = mInternal->mCount -1;
    int finalcnt=mInternal->mCount;
    ret=pthread_mutex_unlock(&mInternal->mtx);
    return (ret == 0)?finalcnt:-1;
}

int32_t CSemaphore::post() {

    pthread_mutex_lock(&mInternal->mtx);
    /**
     * 即使使用多signal,如果没有wait等待,也只是无效浪费
     * mCount永远可以正确的计算signal也即post的次数
     * signal可能会有多次造成浪费,即没有wait来等待就会浪费
     */
    int32_t ret = pthread_cond_signal(&mInternal->mCv);
    mInternal->mCount = mInternal->mCount +1;
    int32_t finalCnt = mInternal->mCount;
    ret=pthread_mutex_unlock(&mInternal->mtx);
    return (ret == 0) ? finalCnt : -1;
}

int32_t CSemaphore::pend(uint32_t timeout) {

    int32_t err = 0;
    struct timespec to={0},now={0};
    if(clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
        errorf("clock_gettime failed,errno=%d",errno);
        return -1;
    }

    to.tv_sec=now.tv_sec + (now.tv_nsec +(timeout%1000L)*1000000L)/1000000000L + timeout/1000L ;
    to.tv_nsec=(now.tv_nsec +(timeout%1000L)*1000000L)%1000000000L;

    pthread_mutex_lock(&mInternal->mtx);
    while (mInternal->mCount == 0 && 0 == err){
        /**
         * 超时时err!=0,此时可以自动在条件判断时退出循环
         */
        err = pthread_cond_timedwait(&mInternal->mCv, &mInternal->mtx,&to);
    }

    if(0 == err) {
        --mInternal->mCount;
    }

    int32_t finalCnt = mInternal->mCount;
    pthread_mutex_unlock(&mInternal->mtx);
    return (err == 0)? finalCnt : -1;
}

int32_t CSemaphore::tryPend() {

    int32_t ret = pthread_mutex_trylock(&mInternal->mtx);
    /**可以加解锁,但资源为0*/
    bool bzero=false;
    if(0 == ret) {
        if(mInternal->mCount > 0){
            mInternal->mCount--;
            bzero=false;
        } else{
            bzero=true;
        }

        ret = pthread_mutex_unlock(&mInternal->mtx);
    }

    /**
     * 资源为0,即使解锁成功也返回 -1
     */
    ret=bzero?-1:ret;
    return (ret == 0)?0:-1;
}

}
}
