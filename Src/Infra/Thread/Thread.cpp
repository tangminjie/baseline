

#include <list>
#include <cerrno>
#include <algorithm>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#define TSK_DEF_STACK_SIZE		16384
#include <poll.h>
#include <sys/param.h>
#include <signal.h>
#include <pthread.h>

#define LOG_LIB_MODULE          "Infra"

#include "Infra/Define.h"
#include "Logger/Define.h"
#include "ThreadInternal.h"
#include "Infra/Time/Time.h"
#include "Infra/Thread/Thread.h"

namespace Uface {
namespace Infra {

static void* InternalThreadBody(void *pdat);

static void* InternalThreadBody(void *pdat) {

    Uface::Infra::ThreadInternal* pInternal = reinterpret_cast<Uface::Infra::ThreadInternal*>(pdat);

    /**设置linux 非实时线程的优先级,实际上是设置nice值*/
	if(pInternal->mPolicy == Uface::Infra::CThread::policyNormal) {
		int32_t priority = -19 + pInternal->mPriority * 40 / (Uface::Infra::CThread::priorBottom + 1);
		setpriority(PRIO_PROCESS, 0, priority);
	}

    pInternal->mMutex.enter();
    pInternal->mRunning = true;
    pInternal->mId = Uface::Infra::CThread::getCurrentThreadId();
    pInternal->mMutex.leave();
    pInternal->mManager->addThread(pInternal);

    debugf("ThreadBody Enter name = {}, id = %d, prior = {}%d, stack = %p \n",
    pInternal->mName, pInternal->mId, (pInternal->mPolicy == Uface::Infra::CThread::policyRealtime) ? "R" : "N", pInternal->mPriority, &pInternal);
    pInternal->mOwner->threadProc();
    debugf("ThreadBody leave name = {}, id = %d \n", pInternal->mName, pInternal->mId);
    pInternal->mManager->removeThread(pInternal);

    pInternal->mLoop = false;
    if (pInternal->mDestroyBlock) {
        pInternal->mSemaphore.post();
    }
    /**
     * mRunning 变量基本是单线程使用,可以不用通过锁保护;通过此变量判定线程是否结束,
     * 保证此语句后此线程没有语句访问pInternal资源
     * 移到这里设置标志位保证安全释放此内存
     */
    pInternal->mRunning = false;
	return nullptr;
}



CThread::CThread(const char * name, int priority, int policy, int stackSize) {
    mInternal = new ThreadInternal();
    mInternal->mOwner = this;
    mInternal->mManager = ThreadManagerInternal::instance();
    mInternal->mPriority = priority;
    mInternal->mPolicy = policy;
    mInternal->mStackSize = stackSize;
    mInternal->mName[sizeof(mInternal->mName) - 1] = '\0';
    if(name) {
        strncpy(mInternal->mName, name, sizeof(mInternal->mName) - 1);
    } else {
        strncpy(mInternal->mName, "noNamed", sizeof(mInternal->mName) - 1);
    }

    mInternal->mId = -1;
    mInternal->mRunning = false;
    mInternal->mLoop = false;
    mInternal->mDestroyed = false;
    mInternal->mDestroyBlock = true;
    mInternal->mExpectedTime = 0;

    mInternal->mPrev = nullptr;
    mInternal->mNext = nullptr;
}

CThread::~CThread() {
    mInternal->mMutex.enter();
    if(mInternal->mLoop) {
        /**
         * @note 解锁到底，避免包含mSemaphore.pend() 引起死锁
         */
        mInternal->mMutex.leave();
        destroyThread();
    } else {
        mInternal->mMutex.leave();
    }

    uint64_t times = 0;
    /**
     * 等待，保证mInternal的delete 是安全的
     */
    while (mInternal->mRunning)
    {
        sleep(1);
        times++;
        if(times % 100 == 0) {
            debugf("Thread destructor function wait for thread {} over,times:%llu",mInternal->mName, times);
        }
    }

    delete mInternal;
}

void CThread::setThreadName(const char*pName) {
    if(pName) {
        strncpy(mInternal->mName, pName, sizeof(mInternal->mName) - 1);
    } else {
        strncpy(mInternal->mName, "noNamed", sizeof(mInternal->mName) - 1);
    }
}

char* CThread::getThreadName() {
    return mInternal->mName;
}

bool CThread::createThread() {

    mInternal->mMutex.enter();
    if(mInternal->mLoop) {
        mInternal->mMutex.leave();
        warnf("thread {} is still running",mInternal->mName);
        return false;
    }

    /**
     * 加上mRunning判断可以避免一下问题:在DestroyThread֮之后线程体还未退出RemoveThread也没有调用
     * 但mLoop为false，CreateThread再次进入，会再次创建线程体并调用AddThread;相当于AddThread
     * 调用了两次,从而引起线程管理链表死链,进而访问链表时出现死循环
     */
    while(mInternal->mRunning) {
        /** 解锁，让正在退出的线程无障碍退出**/
        mInternal->mMutex.leave();
        sleep(1);
        mInternal->mMutex.enter();
    }

    /**
     * 再次判断mLoop 是为了解决多个线程在同一个线程对象上创建线程体引起的竞争
     */
    if(mInternal->mLoop) {
        mInternal->mMutex.leave();
        warnf("thread {} is still running",mInternal->mName);
        return false;
    }

    /**获取创建权,设置loop标记*/
    mInternal->mLoop = true;

    /**需要再次初始化这些状态*/
    mInternal->mId = -1;
    mInternal->mDestroyed = false;
    mInternal->mDestroyBlock = true;
    mInternal->mExpectedTime = 0;

    /**make pretty sure that the count of mSemaphore has been set to 0*/
    while(mInternal->mSemaphore.tryPend()>=0) continue;

    int32_t stkSize = mInternal->mStackSize < TSK_DEF_STACK_SIZE? TSK_DEF_STACK_SIZE:mInternal->mStackSize;

    pthread_attr_t		attr;
    int32_t ret = pthread_attr_init(&attr);
    if(ret != 0) {
        mInternal->mMutex.leave();
        errorf("pthread attribute initial failed,errno:%d", ret);
        return false;
    }

    if (mInternal->mPolicy == policyRealtime) {
        ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        if(ret != 0) {
            mInternal->mMutex.leave();
            errorf("pthread attr setinheritsched PTHREAD_EXPLICIT_SCHED failed, errno=%d", ret);
            return false;
        }

        ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        if(ret != 0) {
            mInternal->mMutex.leave();
            errorf("pthread attr set SCHED_FIFO failed, errno=%d", ret);
            return false;
        }

        /** 设置linux实时线程优先级*/
        struct sched_param param;
        int32_t priorMin = sched_get_priority_min(SCHED_FIFO);
        int32_t priorMax = sched_get_priority_max(SCHED_FIFO);

        param.sched_priority = priorMax - (mInternal->mPriority - CThread::priorTop) * (priorMax - priorMin)/(CThread::priorBottom - CThread::priorTop);
        ret = pthread_attr_setschedparam(&attr, &param);

        if(ret != 0) {
            mInternal->mMutex.leave();
            errorf("pthread attr pthread_attr_setschedparam(sched priority=%d) failed, errno=%d", param.sched_priority,ret);
            return false;
        }
    } else {
        if(mInternal->mPolicy != policyNormal) {
            mInternal->mMutex.leave();
            warnf("CThread::CreateThread policy isn't set properly, policy = %d", mInternal->mPolicy);
        }

        ret = pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        if(ret != 0) {
            mInternal->mMutex.leave();
            warnf("pthread attr pthread_attr_setschedpolicy(SCHED_OTHER) failed, errno=%d", ret );
            return false;
        }
    }

    ret = pthread_create(&mInternal->mHandle, &attr,(void* (*)(void *))InternalThreadBody, (void *)mInternal);
    if(ret != 0) {
        mInternal->mMutex.leave();
        errorf("pthread create failed, errno=%d",ret);
        return false;
    }

    ret = pthread_detach(mInternal->mHandle);
    if(ret != 0) {
        mInternal->mMutex.leave();
        errorf("pthread detach failed, errno=%d\n",ret);
        return false;
    }

    ret = pthread_attr_destroy(&attr);
    if(ret != 0) {
        mInternal->mMutex.leave();
        errorf("pthread attr destroy failed, errno=%d",ret);
        return false;
    }

    bool succ = (ret == 0);
    mInternal->mRunning = (succ && mInternal->mLoop);
    mInternal->mMutex.leave();
    return succ;
}

bool CThread::destroyThread() {
    mInternal->mMutex.enter();
    if(mInternal->mDestroyed) {
        mInternal->mMutex.leave();
        warnf("CThread::DestroyThread() thread '{}' has been destroyed!", mInternal->mName);
        return false;
    }

    /**避免没有createThread就直接destroyThread导致的永久阻塞*/
    if(isThreadOver()) {
        mInternal->mMutex.leave();
        warnf("CThread::DestroyThread() thread '{}' has exited!", mInternal->mName);
        return false;
    }

    mInternal->mLoop = false;
    mInternal->mDestroyed = true;
    mInternal->mMutex.leave();

    /**等待线程结束,自己关闭自己的时候才采用非阻塞的方式*/
    if (mInternal->mId == CThread::getCurrentThreadId()) {
        mInternal->mDestroyBlock = false;
    } else {
        while(mInternal->mRunning){
            if(-1 != mInternal->mSemaphore.pend(50) || !mInternal->mDestroyed) break;
        }
    }

    return true;
}

bool CThread::terminateThread() {
    destroyThread();
    return (pthread_join(mInternal->mHandle, nullptr) == 0);
}

/**
 * @brief 取消线程,设置线程退出标记，非阻塞方式，不等待线程结束
 */
bool CThread::cancelThread() {
    mInternal->mMutex.enter();
    if(!mInternal->mLoop) {
        mInternal->mMutex.leave();
        warnf("CThread::cancelThread() thread '{}' not exist!", mInternal->mName);
        return false;
    }

    mInternal->mLoop = false;
    mInternal->mMutex.leave();

    /**不等待线程退出,非阻塞方式*/
    mInternal->mDestroyBlock = false;
    return true;
}

bool CThread::isThreadOver() {
    return !mInternal->mRunning && !mInternal->mLoop;
}

int32_t CThread::getThreadId() {
    return mInternal->mId;
}

void CThread::setTimeout(int milliSeconds) {
    if(milliSeconds == 0) {
        mInternal->mExpectedTime = 0;
    } else {
        mInternal->mExpectedTime = CTime::getCurrentMilliSecond() + milliSeconds;
    }
}

void CThread::setTimeout(uint64_t now,int32_t milliSeconds) {
    /**清空预期时间*/
    mInternal->mExpectedTime = (milliSeconds == 0)? 0:now + milliSeconds;
}

bool CThread::isTimeout() {
    CGuard guard(mInternal->mMutex);
    return (mInternal->mExpectedTime != 0 && mInternal->mExpectedTime < CTime::getCurrentMilliSecond());
}

bool CThread::looping() const {
    return mInternal->mLoop;
}

bool CThread::setCurrentThreadName(const char *name) {
    if (name == nullptr) {
        return false;
    }

    prctl(PR_SET_NAME,name, 0,0,0);
    return true;
}

int32_t CThread::getCurrentThreadId() {

#ifndef SYS_gettid
    #define SYS_gettid __NR_gettid
#endif
    return (int)syscall(SYS_gettid);
}

/**
 * @brief 设置当前线程超时时间
 */
bool CThread::setCurrentTimeout(int32_t milliSeconds) {
    int tid = CThread::getCurrentThreadId();
    ThreadInternal* thread = ThreadManagerInternal::instance()->getThread(tid);
    if (thread) {
        CGuard guard(thread->mMutex);
        thread->mExpectedTime = (milliSeconds == 0)? 0:CTime::getCurrentMilliSecond() + (int32_t)milliSeconds;
        return true;
    }

    return false;
}

void CThread::sleep(int32_t ms) {

    if (ms > 0) {
        int32_t ret = 0;
        int32_t pollMs = ms;
        uint64_t startTime = CTime::getCurrentMilliSecond();
        while((ret = poll(nullptr, 0, pollMs)) != 0) {
            if (-1 == ret) {
                /**捕捉信号,引起中断,返回的时间可能不会正确填写，进入无限期等待*/
                uint64_t endTime = CTime::getCurrentMilliSecond();
                int32_t sleepTime = (int32_t)(endTime - startTime);
                if(EINTR != errno) {
                    warnf("thread sleep select time = %lld, SleepTime = %d while breaking, errormsg : {}",
                    sleepTime, ms, strerror(errno));
                }

                if(sleepTime > ms) {
                    break;
                }

                pollMs = ms - sleepTime;
            }
        }
    } else {
        sched_yield();
    }
}

void CThread::load(int percent) {
    UFACE_ASSERT(percent >= 1 && percent <= 100,"percent is out range(0~100)");
    if(percent < 100) {
        CThreadLoadingController::instance()->addThread(pthread_self(), percent, 100 - percent);
    } else {
        CThreadLoadingController::instance()->removeThread(pthread_self());
    }
}



}
}