
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "Logger/Define.h"
#include "Infra/Concurrence/ReadWriteMutex.h"

#define RWMUXTEX_MAXENTER	0xffff



namespace Uface {
namespace Infra {

struct CReadWriteMutex::RWMutexInternal {
    CReadWriteMutex::RW_PRIORITY markRWPriority;
    RWMutexInternal(CReadWriteMutex::RW_PRIORITY priority);
    pthread_rwlock_t*            mLock;
};

CReadWriteMutex::RWMutexInternal::RWMutexInternal(CReadWriteMutex::RW_PRIORITY priority) {
    if (priority != CReadWriteMutex::PRIORITY_DEFAULT && priority != CReadWriteMutex::PRIORITY_WRITE) {
        warnf("Input rwpriority level error, priorityDefault used!");
        priority = CReadWriteMutex::PRIORITY_DEFAULT;
    }

    int32_t ret = -1;
	mLock = (pthread_rwlock_t*)malloc(sizeof(pthread_rwlock_t));
	UFACE_ASSERT(mLock,"out of memory");

	if (CReadWriteMutex::PRIORITY_WRITE == priority ) {
        pthread_rwlockattr_t attr;
        pthread_rwlockattr_setkind_np(&attr,PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
        ret = pthread_rwlock_init(mLock,&attr);
	} else {
		ret = pthread_rwlock_init(mLock,nullptr);
	}

	UFACE_ASSERT(ret == 0,"initial rwlock error");
	(void)ret;
    markRWPriority = priority;
}

CReadWriteMutex::CReadWriteMutex() {
    mInternal = new RWMutexInternal(PRIORITY_DEFAULT);
    UFACE_ASSERT(mInternal != nullptr,"out of memory");
}

CReadWriteMutex::CReadWriteMutex(CReadWriteMutex::RW_PRIORITY priority) {
    mInternal = new RWMutexInternal(priority);
    UFACE_ASSERT(mInternal != nullptr,"out of memory");
}

CReadWriteMutex::~CReadWriteMutex() {
    int32_t ret = pthread_rwlock_destroy(mInternal->mLock);
	UFACE_ASSERT(ret == 0,"destroy rwlock failed");
	free(mInternal->mLock);
    delete mInternal;
}

bool CReadWriteMutex::enterReading() {
    return pthread_rwlock_rdlock(mInternal->mLock) == 0;
}

bool CReadWriteMutex::enterWriting() {
    int32_t ret = pthread_rwlock_wrlock(mInternal->mLock);
	if (ret != 0) {
		warnf("pthread_rwlock_wrlock called failed for :{}", strerror(errno));
		return false;
	}
	
	return true;
}

bool CReadWriteMutex::leave() {
    int32_t ret = pthread_rwlock_unlock(mInternal->mLock);
	if (ret != 0){
		warnf("pthread_rwlock_unlock called failed for :{}", strerror(errno));
		return false;	
	}

	return true;
}

}
}