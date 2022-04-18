
#include "ThreadInternal.h"
#include "Infra/Thread/ThreadLite.h"


namespace Uface {
namespace Infra {

CThreadLite::CThreadLite(ThreadProc proc, const char * name,
                    int32_t priority, int32_t policy, int32_t stackSize)
                    :CThread(name, priority, policy, stackSize) {
    mInternal = new ThreadLiteInternal;
    mInternal->mProc = proc;
}

CThreadLite::~CThreadLite() {
    /**
     * 停止线程体
     */
    if (!isThreadOver()) {
        destroyThread();
    }

    delete mInternal;
}

void CThreadLite::threadProc() {
    /**设置线程名称*/
    CThread::setCurrentThreadName(getThreadName());
    mInternal->mProc(*this);
}

}
}
