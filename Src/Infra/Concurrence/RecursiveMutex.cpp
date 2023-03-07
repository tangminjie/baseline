

#include <pthread.h>
#include "Logger/Define.h"
#include "Infra/Concurrence/RecursiveMutex.h"



/** pthread_mutexattr_settype bad*/
extern "C"	int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind);

namespace ArcFace {
namespace Infra {

struct CRecursiveMutex::RecursiveMutexInternal {
	pthread_mutex_t	mtx;
};

CRecursiveMutex::CRecursiveMutex() {
	mInternal = new RecursiveMutexInternal;

	pthread_mutexattr_t attr;
	int32_t ret = pthread_mutexattr_init(&attr);
	ARCFACE_ASSERT(ret == 0,"initial pthread mutex attribute failed");
	ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
	ARCFACE_ASSERT(ret == 0,"set pthread mutex attribure type failed");

	ret = pthread_mutex_init(&mInternal->mtx, &attr);
	ARCFACE_ASSERT(ret == 0,"initial pthread mutex failed");
}

CRecursiveMutex::~CRecursiveMutex() {
	int ret = pthread_mutex_destroy(&mInternal->mtx);
	ARCFACE_ASSERT(ret == 0,"destroy pthread mutex failed");
	(void)ret;

	delete mInternal;
}

bool CRecursiveMutex::enter() {
	return (pthread_mutex_lock(&mInternal->mtx) == 0);
}

bool CRecursiveMutex::leave() {
	return (pthread_mutex_unlock(&mInternal->mtx) == 0);
}

}
}
