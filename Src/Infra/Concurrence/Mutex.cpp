
#include <cstdlib>
#include <pthread.h>
#include "Logger/Define.h"
#include "Infra/Concurrence/Mutex.h"

/** pthread_mutexattr_settype bad*/

extern "C"	int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind);

namespace Uface {
namespace Infra {

struct CMutex::MutexInternal {
	pthread_mutex_t	mtx;
};

CMutex::CMutex() {
	mInternal = new MutexInternal;
	int ret = pthread_mutex_init(&mInternal->mtx, nullptr);
	UFACE_ASSERT(ret == 0,"initial pthread mutex failed");
	(void)ret;
}

CMutex::~CMutex() {
    int32_t ret = pthread_mutex_destroy(&mInternal->mtx);
    UFACE_ASSERT(ret == 0,"destroy pthread mutex failed");
	(void)ret;

	delete mInternal;
	mInternal = nullptr;
}

bool CMutex::enter() {
	return (pthread_mutex_lock(&mInternal->mtx) == 0);
}

bool CMutex::tryEnter() {
	return (pthread_mutex_trylock(&mInternal->mtx) == 0);
}

bool CMutex::leave() {
	return (pthread_mutex_unlock(&mInternal->mtx) == 0);
}

} // namespace Infra
} // namespace Uface


