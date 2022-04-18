
#ifndef FRAMEWORK_INCLUDE_INFRA_GUARD_H
#define FRAMEWORK_INCLUDE_INFRA_GUARD_H

#include "Infra/Concurrence/Mutex.h"
#include "Infra/Concurrence/ReadWriteMutex.h"
#include "Infra/Concurrence/RecursiveMutex.h"

namespace Uface {
namespace Infra {

class CGuard {

    CGuard(const CGuard&);
    CGuard& operator=(const CGuard&);

public:

    inline CGuard(CMutex& mutex):mMutex(mutex) {
        mMutex.enter();
    }

    inline ~CGuard() {
        mMutex.leave();
    }

private:
    CMutex&     mMutex;
};

class CRecursiveGuard {

    CRecursiveGuard(const CRecursiveGuard&);

    CRecursiveGuard& operator=(const CRecursiveGuard&);

public:

    inline CRecursiveGuard(CRecursiveMutex& mutex):mMutex(mutex) {
        mMutex.enter();
    }

    inline ~CRecursiveGuard() {
        mMutex.leave();
    }

private:

    CRecursiveMutex& mMutex;
};

class CGuardReading {

    CGuardReading(const CGuardReading&);
    CGuardReading& operator=(const CGuardReading&);

public:

    inline CGuardReading(CReadWriteMutex& mutex):mRWMutex(mutex) {
        mRWMutex.enterReading();
    }

    inline ~CGuardReading() {
        mRWMutex.leave();
    }

private:
    CReadWriteMutex& mRWMutex;
};

class CGuardWriting {

    CGuardWriting(const CGuardWriting&);
    CGuardWriting& operator=(const CGuardWriting&);

public:

    inline CGuardWriting(CReadWriteMutex& mutex):mRWMutex(mutex) {
            mRWMutex.enterWriting();
    }

    inline ~CGuardWriting() {
        mRWMutex.leave();
    }

private:
    CReadWriteMutex& mRWMutex;
};

}
}

#endif //FRAMEWORK_INCLUDE_INFRA_GUARD_H
