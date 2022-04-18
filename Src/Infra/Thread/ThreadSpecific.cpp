#include <pthread.h>
#include "Infra/Define.h"
#include "Infra/Thread/ThreadSpecific.h"

namespace Uface {
namespace Infra {

struct CThreadSpecific::CThreadSpecificInternal {
    pthread_key_t  key;
    bool           valid;
};

CThreadSpecific::CThreadSpecific() {
    mInternal = new CThreadSpecificInternal;
    mInternal->key = 0;
    mInternal->valid = false;
    mInternal->valid = pthread_key_create(&mInternal->key, nullptr) == 0;
}

CThreadSpecific::~CThreadSpecific() {
    if (valid()) {
        pthread_key_delete(mInternal->key);
    }

    delete mInternal;
}

bool CThreadSpecific::valid() {
    return mInternal->valid;
}

bool CThreadSpecific::setValue(const void *value) {

    if (valid()) {
        return pthread_setspecific(mInternal->key,value) == 0;
    }

    return false;
}

bool CThreadSpecific::getValue(void **value) {
    if (!valid() || value == nullptr) {
        return false;
    }

    *value = pthread_getspecific(mInternal->key);
    return true;
}

}
}
