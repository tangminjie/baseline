
#include <string>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <algorithm>
#include "Logger/Define.h"
#include "TimerInternal.h"
#include "Infra/Time/Time.h"
#include "Infra/Time/Timer.h"


namespace ArcFace {
namespace Infra {

static TimerManagerInternal* gTimerManager = TimerManagerInternal::instance();

CTimer::CTimer(const char * pName) {

    mInternal = new TimerInternal;
    mInternal->mRunningTime = 0;
    mInternal->mCallTime = 0;
    mInternal->mCheckTime = 0;
    mInternal->mPeriod = 0;
    mInternal->mTimeout = 0;
    mInternal->mParam = 0;
    mInternal->mThread = nullptr;
    setName(pName);
    mInternal->mStarted = false;
    mInternal->mCalled = false;

    mInternal->mPrev = nullptr;
    mInternal->mNext = nullptr;
}

CTimer::~CTimer() {

    if (nullptr != gTimerManager) {
        CRecursiveGuard guard(gTimerManager->smMutex);
        /**解锁到底,保证StopAndWait内部解锁有效*/
        gTimerManager->smMutex.leave();
        stopAndWait();
        gTimerManager->smMutex.enter();
    }

    delete mInternal;
}

bool CTimer::start(Proc fun, uint32_t delay, uint32_t period, uint64_t param, uint32_t timeout) {

    CRecursiveGuard guard(gTimerManager->smMutex);
    mInternal->mFunc = fun;
    mInternal->mCallTime = gTimerManager->mCurrentTime;
    mInternal->mCallTime += (uint64_t)delay;
    mInternal->mPeriod = period;
    mInternal->mTimeout = timeout;
    mInternal->mParam = param;
    mInternal->mCalled = false;
    mInternal->mStatistic = false;

    if(mInternal->mStarted) {
        gTimerManager->removeTimer(mInternal);
    }

    gTimerManager->addTimer(mInternal);
    mInternal->mStarted = true;

    /**需要立即调用的情况下直接调用,不需要等系统定时器信号触发*/
    if(delay == 0) {
        return mInternal->run();
    }

    return true;
}

bool CTimer::stop(bool bCallNow) {
    CRecursiveGuard guard(gTimerManager->smMutex);
    if(!mInternal->mStarted) {
        return false;
    }

    gTimerManager->removeTimer(mInternal);
    mInternal->mStarted = false;

    /**带延时的非周期定时器*/
    if(!mInternal->mPeriod && bCallNow) {
        return mInternal->run();
    }

    return true;
}

bool CTimer::stopAndWait() {
    /**解决非周期定时器不去等待执行体结束的问题*/
    stop();

    CRecursiveGuard guard(gTimerManager->smMutex);
    if(mInternal->mThread && mInternal->mThread->getThreadId() != CThread::getCurrentThreadId()) {
        int32_t n = 0;
        while(mInternal->mThread) {
            gTimerManager->smMutex.leave();
            CThread::sleep(10);
            gTimerManager->smMutex.enter();

            /**降低打印频率到1s 2 次*/
            if((n % 500) == 0) {
                infof("CTimer::Stop '{}' wait callback exit!", mInternal->mName);
            }

            n++;
        }
    }

    return true;
}

void CTimer::setPeriodTime(unsigned int pertime) {
    CRecursiveGuard guard(gTimerManager->smMutex);
    if(mInternal->mStarted) {
        /**先删除链表中的定时器*/
        gTimerManager->removeTimer(mInternal);
    }

    mInternal->mCallTime -= mInternal->mPeriod;
    mInternal->mCallTime += pertime;
    mInternal->mPeriod = pertime;

    if(mInternal->mStarted) {
        /**重新设置周期后再加入定时器*/
        gTimerManager->addTimer(mInternal);
    }
}

void CTimer::enableStat(bool bStat) {
    mInternal->mStatistic = bStat;
}

const char * CTimer::getName() {
    return mInternal->mName;
}

void CTimer::setName(const char * pszName) {
    using namespace std;
    int len = min((int)strlen(pszName), (int)31);

    memcpy(mInternal->mName, pszName, len);
    mInternal->mName[len]='\0';
}

bool CTimer::isStarted() {
    return mInternal->mStarted;
}

bool CTimer::isCalled() {
    return mInternal->mCalled;
}

bool CTimer::isRunning() {
    return (mInternal->mThread != nullptr);
}

/**********************************************************************************
 * CTimerThread
**********************************************************************************/

CTimerThread::CTimerThread():CThread("[Pooled]", priorDefault)
        , mParam(0), mNextPooled(nullptr), mCaller(nullptr), mCancel(false){
}

CTimerThread::~CTimerThread() {
}

void CTimerThread::threadProc() {
    while(looping()) {
        mSemaphore.pend();

        if (mCancel) {
            break;
        }

        uint64_t us = 0;
        if(mCaller && mCaller->mStatistic) {
            us = CTime::getCurrentMicroSecond();
        }

        mAsyncFunc(mParam);
        setTimeout(0);
        setThreadName("[Pooled]");

        /**CTimer 和 CTimerThread 的关联关系*/
        gTimerManager->smMutex.enter();
        if(mCaller ) {
            if(mCaller->mStatistic) {
                mCaller->mRunningTime += CTime::getCurrentMicroSecond() - us;
            }

            mCaller->mThread = nullptr;
            mCaller = nullptr;
        }

        gTimerManager->smMutex.leave();
        gTimerManager->putTimerThread(this);
    }
}

void CTimerThread::cancel() {
    mCancel = true;
    mSemaphore.post();
}

/**********************************************************************************
 * CTimerManager
 **********************************************************************************/

CTimerManager* CTimerManager::instance() {
    static CTimerManager timerManager;
    return &timerManager;
}

CTimerManager::CTimerManager() {
    tracef("CTimerManager::CTimerManager()>>>>>>>>>");
    mInternal = TimerManagerInternal::instance();
}

CTimerManager::~CTimerManager() {
}

void CTimerManager::printTimers() {
    mInternal->dumpTimers();
}

bool CTimerManager::limitIdlePooled(int32_t count) {
    return mInternal->limitIdlePooled(count);
}

TimerManagerInternal* TimerManagerInternal::instance() {
    static TimerManagerInternal managerInternal;
    return &managerInternal;
}

TimerManagerInternal::TimerManagerInternal()
                                     :CThread("TimerManager", CThread::priorTop),
                                     mHeadPooled(nullptr), mPHead(nullptr),
                                     mLimitThreadCount(-1), mIdleThreadCount(0) {

    mPHead = nullptr;
    mCurrentTime = CTime::getCurrentMilliSecond();
    createThread();
}

TimerManagerInternal::~TimerManagerInternal() {
    destroyThread();

    {
        CRecursiveGuard guard(TimerManagerInternal::smMutex);

        CTimerThread *p = mHeadPooled;
        while (p) {
            CTimerThread *pDel = p;
            p = p->mNextPooled;
            pDel->cancel();
            pDel->destroyThread();
            delete pDel;
            pDel = nullptr;
        }

        mHeadPooled = nullptr;
    }
}

bool TimerManagerInternal::addTimer(TimerInternal * pTimer) {

    TimerInternal *pp = nullptr, *pn = mPHead;
    while(pn && (pTimer->mCallTime > pn->mCallTime)) {
        pp = pn;
        pn = pn->mNext;
    }

    if(pp) {
        pp->mNext = pTimer;
    } else {
        mPHead = pTimer;
    }

    pTimer->mPrev = pp;

    if(pn) {
        pn->mPrev = pTimer;
    }

    pTimer->mNext = pn;
    return true;
}

bool TimerManagerInternal::removeTimer(TimerInternal * pTimer) {

    if(pTimer->mPrev == nullptr) {
        mPHead = pTimer->mNext;
        if(mPHead) {
            mPHead->mPrev = nullptr;
        }

        return true;
    }

    pTimer->mPrev->mNext = pTimer->mNext;
    if(pTimer->mNext != nullptr) {
        pTimer->mNext->mPrev = pTimer->mPrev;
    }

    return true;
}

CTimerThread* TimerManagerInternal::getTimerThread() {
    CRecursiveGuard guard(TimerManagerInternal::smMutex);
    CTimerThread * p = mHeadPooled;

    if(p != nullptr) {
        mIdleThreadCount--;
        mHeadPooled = p->mNextPooled;
    } else {
        p = new CTimerThread();
        p->createThread();
    }

    return p;
}

void TimerManagerInternal::putTimerThread(CTimerThread* pThread) {
    CTimerThread *pDelHead = nullptr;

    {
        CRecursiveGuard guard(TimerManagerInternal::smMutex);
        ARCFACE_ASSERT(pThread != nullptr,"thread is null");

        pThread->mNextPooled = mHeadPooled;
        mHeadPooled = pThread;
        mIdleThreadCount++;

        /**未设置限制数量或者没有达到限制的数量直接返回*/
        if (mLimitThreadCount <= 0 || mIdleThreadCount <= mLimitThreadCount) {
            return;
        }

        CTimerThread *pHead = mHeadPooled;

        /**遍历空闲线程数计数用*/
        int32_t num = 0;
        while (pHead) {
            num++;
            if (num >= mLimitThreadCount) {
                pDelHead = pHead->mNextPooled;
                pHead->mNextPooled = nullptr;
                mIdleThreadCount = num;
                break;
            }

            pHead = pHead->mNextPooled;
        }
    }

    /**删除多余的空闲线程*/
    while (pDelHead) {
        CTimerThread *pDel = pDelHead;
        pDelHead = pDelHead->mNextPooled;
        pDel->cancel();
        pDel->destroyThread();
        delete pDel;
        pDel = nullptr;
    }
}

void TimerManagerInternal::dumpTimers() {

    TimerInternal* p = mPHead;
    CRecursiveGuard guard(TimerManagerInternal::smMutex);
    infof("Timers: ( %11lu Milli-Seconds Elapsed )\n", mCurrentTime);
    infof("            Name            NextTime RunningTime Period   Used/Timeout State\n");
    infof("____________________________________________________________________________\n");
    while (p) {
        infof("%24s %11lu %11lu %6lu %6d/%6lu {}\n",p->mName,p->mCallTime,p->mRunningTime / 1000,
              p->mPeriod,p->mThread ? (int)(mCurrentTime - p->mCheckTime) : 0,p->mTimeout,p->mThread ? "Running" : "Idle");

        p = p->mNext;
    }

    infof("\n");
}

bool TimerManagerInternal::limitIdlePooled(int count) {
    mLimitThreadCount = count;
    return true;
}

void TimerManagerInternal::threadProc() {

    do {
        CThread::sleep(10);

        CRecursiveGuard guard(TimerManagerInternal::smMutex);

        uint64_t OldTime = mCurrentTime;
        mCurrentTime = CTime::getCurrentMilliSecond();

        /**设置超时时间为10s,超时看门狗会重启*/
        setTimeout(10000,mCurrentTime);

        /**计时没有改变，可能因为计时器精度不高*/
        if(mCurrentTime == OldTime) {
            continue;
        }

        ARCFACE_ASSERT(mCurrentTime >= OldTime,"old time > current time");
        while(mPHead && mPHead->mCallTime < mCurrentTime) {
            mPHead->run();
        }

    } while(looping());
}

bool TimerInternal::run() {

    gTimerManager->removeTimer(this);
    uint64_t curTime = gTimerManager->mCurrentTime;
    if(mPeriod) {
        /**避免多次连续执行*/
        while(mCallTime <= curTime) {
            /**加入=判断，避免出现没有delay的定时器开启时调用两次*/
            uint64_t temp = (curTime - mCallTime) / (uint64_t)mPeriod;
            mCallTime += (uint64_t)mPeriod * (temp + 1);
        }

        gTimerManager->addTimer(this);
    } else {
        mStarted = false;
        mCalled = true;
    }

    /**正在运行*/
    if(mThread) {
        return false;
    }

    mCheckTime = curTime;
    mThread = gTimerManager->getTimerThread();
    mThread->setThreadName(mName);
    mThread->setTimeout(mTimeout);
    mThread->mAsyncFunc = mFunc;
    mThread->mParam = mParam;
    mThread->mCaller = this;
    mThread->mSemaphore.post();

    return true;
}

/**ֻ只为构造函数能够调用*/
void TimerManagerInternal::doNothing() {
}

}
}