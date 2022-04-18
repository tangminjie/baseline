
#ifndef FRAMEWORK_INCLUDE_INFRA_ATOMICCOUNT_H
#define FRAMEWORK_INCLUDE_INFRA_ATOMICCOUNT_H

#include "Infra/Concurrence/Guard.h"

namespace Uface {
namespace Infra {

class CAtomicCount {

private:

    CAtomicCount(CAtomicCount const &);
    CAtomicCount & operator=(CAtomicCount const &);

public:

    explicit CAtomicCount(int64_t v): mValue(v) {
    }

    ~CAtomicCount() {
    }

    int64_t operator++() {
        CGuard guard(mMutex);
        return ++mValue;
    }

    int64_t operator--() {
        CGuard guard(mMutex);
        return --mValue;
    }

    operator int64_t () const {
        CGuard guard(mMutex);
        return mValue;
    }

private:
    mutable CMutex          mMutex;
    int64_t                 mValue;
};

}
}

#endif //FRAMEWORK_INCLUDE_INFRA_ATOMICCOUNT_H
