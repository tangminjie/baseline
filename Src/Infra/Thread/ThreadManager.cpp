

#include "ThreadInternal.h"
#include "Infra/Thread/ThreadManager.h"

namespace Uface {
namespace Infra {

CThreadManager* CThreadManager::instance() {
    static CThreadManager threadManager;
    return &threadManager;
}

CThreadManager::CThreadManager() {
    mInternal = ThreadManagerInternal::instance();
}

CThreadManager::~CThreadManager() {
}

void CThreadManager::registerMainThread() {
    mInternal->registerMainThread();
}

void CThreadManager::printThreads() {
    mInternal->dumpThreads();
}

bool CThreadManager::getThreadNameById(int32_t id, std::string& name) {
    return mInternal->getThreadNameById(id, name);
}

bool CThreadManager::getTimeoutThreadName(std::string& names) {
    return mInternal->getTimeOutThreads(names);
}

bool CThreadManager::hasThread(int id) {
    ThreadInternal* thread = mInternal->getThread(id);
    return thread != nullptr;
}
}
}