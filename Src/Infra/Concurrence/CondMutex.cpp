

#include "Infra/Concurrence/CondMutex.h"

namespace ArcFace {
namespace Infra {

struct CCondMutex::CCondMutexInternal {

};

CCondMutex::CCondMutex():mInternal(nullptr) {
    mInternal = new CCondMutexInternal;
}

CCondMutex::~CCondMutex() {
    delete mInternal;
}

bool CCondMutex::wait() {
    return true;
}

bool CCondMutex::notify() {
    return true;
}

bool CCondMutex::broadcast() {
    return true;
}

}
}