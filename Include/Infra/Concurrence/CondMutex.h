
#ifndef FRAMEWORK_INCLUDE_INFRA_CONDMUTEX_H
#define FRAMEWORK_INCLUDE_INFRA_CONDMUTEX_H

#include "Infra/Define.h"
#include "Infra/Concurrence/Mutex.h"

namespace ArcFace {
namespace Infra {

class CCondMutex : public CMutex {

    CCondMutex(const CCondMutex&);
    CCondMutex& operator=(const CCondMutex&);

public:
    /**
     * @brief 构造函数,创建条件变量
     */
    CCondMutex();
    /**
     * @brief 析构函数,销毁条件变量
     */
    ~CCondMutex();
    /**
     * @brief 等待通知
     * @return
     */
    bool wait();
    /**
     * @brief 通知
     * @return
     */
    bool notify();
    /**
     * @brief 广播通知
     * @return
     */
    bool broadcast();

private:
    struct CCondMutexInternal;
    CCondMutexInternal*     mInternal;
};

}
}
#endif //FRAMEWORK_INCLUDE_INFRA_CONDMUTEX_H
