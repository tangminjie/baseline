#define  _SIGNAL_H
#include <list>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <algorithm>
#include <poll.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#define TSK_DEF_STACK_SIZE		16384
#include <bits/signum.h>
#include <pthread.h>
#include "Logger/Define.h"
#include "ThreadInternal.h"
#include "Infra/Time/Time.h"

namespace Uface {
namespace Infra {


ThreadManagerInternal* ThreadManagerInternal::instance() {
    static ThreadManagerInternal managerInternal;
    return &managerInternal;
}

ThreadManagerInternal::ThreadManagerInternal() :
        mHead(nullptr), mMainThreadId(-1) {
    tracef("CThreadManager::CThreadManager()>>>>>>>>>");
}

void ThreadManagerInternal::registerMainThread() {
    mMainThreadId = CThread::getCurrentThreadId();
}

bool ThreadManagerInternal::addThread(ThreadInternal *internal) {
    CGuard guard(mMutex);

    internal->mPrev = nullptr;
    internal->mNext = mHead;

    if(mHead) {
        mHead->mPrev = internal;
    }

    mHead = internal;
    return true;
}

bool ThreadManagerInternal::removeThread(ThreadInternal *internal) {
    CGuard guard(mMutex);
    if(internal->mPrev == nullptr) {
        mHead = internal->mNext;
        if(mHead) {
            mHead->mPrev = nullptr;
        }

        return true;
    }

    internal->mPrev->mNext = internal->mNext;
    if(internal->mNext != nullptr) {
        internal->mNext->mPrev = internal->mPrev;
    }

    return true;
}

void ThreadManagerInternal::dumpThreads() {
    std::list<std::string> strList;
    do {
        CGuard guard(mMutex);
        ThreadInternal * p = mHead;
        char buffer[128] = {0};
        buffer[127] = 0;
        while (p) {
            snprintf(buffer, sizeof(buffer) - 1, "%24s   %8d  %3d  %s\n", p->mName, p->mId, p->mPriority, (p->mExpectedTime != 0 && p->mExpectedTime < CTime::getCurrentMilliSecond()) ? "Timeout" : "Normal");
            p = p->mNext;
            strList.push_back(buffer);
        }
    } while(false);

    infof("Threads:\n");
    infof("               Name            TID  Prior State\n");
    infof("_______________________________________________________\n");
    infof("%24s   %8d  %3d  %s\n", "Main", mMainThreadId, 64, "Normal");
    for (std::list<std::string>::iterator iter = strList.begin(); iter != strList.end(); ++iter) {
        infof("%s", (*iter).c_str());
    }

    infof("\n");
}

bool ThreadManagerInternal::getThreadNameById(int32_t id, std::string& name) {
    CGuard guard(mMutex);
    ThreadInternal * p = mHead;
    while (p) {
        if (p->mId == id) {
            name = p->mName;
            return true;
        } else {
            p = p->mNext;
        }
    }

    return false;
}

bool ThreadManagerInternal::getTimeOutThreads(std::string& threadName) {
    CGuard guard(mMutex);
    ThreadInternal *p = mHead;
    bool ret = false;
    threadName.resize(0);
    uint64_t currentTime = CTime::getCurrentMilliSecond();
    while(p) {
        p->mMutex.enter();
        if (p->mExpectedTime != 0 && p->mExpectedTime < currentTime) {
            threadName += p->mName;
            char id[8] = {0};

            snprintf(id, sizeof(id) - 1, "(%d)", p->mId);
            threadName += id;
            ret = true;
        }

        p->mMutex.leave();
        p = p->mNext;
    }

    return ret;
}

ThreadInternal* ThreadManagerInternal::getThread(int tid) {
    CGuard guard(mMutex);
    for (ThreadInternal* p = mHead; p; p = p->mNext) {
        if (p->mId == tid) {
            return p;
        }
    }

    return nullptr;
}

static void onSignal(int signal) {
    CThread::sleep(1);
}

CThreadLoadingController::CThreadLoadingController():
        CThread("ThreadLoadingControl", CThread::priorTop, CThread::policyRealtime) {
    // struct sigaction act,oldact;

    // memset(&act, 0, sizeof(act));
    // act.sa_handler = onSignal;
    // act.sa_flags = SA_NODEFER | SA_RESTART;

    // int ret = sigaction(SIGUSR2,&act,&oldact);
    // UFACE_ASSERT(ret == 0,"sigaction return error");
    // /**保证没有使用*/
    // UFACE_ASSERT(oldact.sa_handler == 0 && oldact.sa_sigaction == 0,"has used");
}


CThreadLoadingController* CThreadLoadingController::instance() {
    static CThreadLoadingController controller;
    return &controller;
}

void CThreadLoadingController::addThread(ThreadHandle pid, int onCycle, int offCycle) {
    {
        CGuard guard(mMutex);
        ThreadLoadingControl control = {onCycle, offCycle, 0, 0};
        mThreads[pid] = control;
    }

    if (isThreadOver()) {
        createThread();
    }
}

void CThreadLoadingController::removeThread(ThreadHandle pid) {
    bool empty = false; {
        CGuard guard(mMutex);
        mThreads.erase(pid);
        empty = mThreads.empty();
    }

    if (empty && !isThreadOver()) {
        destroyThread();
    }
}


void CThreadLoadingController::threadProc() {
    while (looping()) {
        {
            CGuard guard(mMutex);

            for (LCTS::iterator pi = mThreads.begin(); pi != mThreads.end(); ++pi) {
                ThreadLoadingControl& tlc = pi->second;
                if (tlc.onCycle * tlc.offElapsed > tlc.offCycle * tlc.onElapsed) {
                    tlc.onElapsed++;
                } else {
                    //pthread_kill(pi->first, SIGUSR2);
                    tlc.offElapsed++;
                }

                if (tlc.onCycle == tlc.onElapsed && tlc.offCycle == tlc.offElapsed) {
                    tlc.onElapsed = 0;
                    tlc.offElapsed = 0;
                }
            }
        }

        /**在循环体之后睡眠,避免本线程未被及时唤醒*/
        CThread::sleep(1);
    }
}

}
}