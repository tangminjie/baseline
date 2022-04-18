


#ifndef FRAMEWORK_INCLUDE_INFRA_MUTEX_H
#define FRAMEWORK_INCLUDE_INFRA_MUTEX_H

#include "Infra/Define.h"

namespace Uface {
namespace Infra {

class CMutex {

    CMutex(const CMutex&);
    CMutex& operator=(const CMutex&);

public:

    /**
     * @brief 构造函数,创建临界区互斥量
     */
    CMutex();
    /**
     * @brief 析构函数,销毁临界区互斥量
     */
    ~CMutex();
    /**
     * @brief 进入临界区
     * @return
     */
    bool enter();
    /**
     * @brief 尝试进入临界区
     * @return
     */
    bool tryEnter();
    /**
     * @brief 离开临界区
     * @return
     */
    bool leave();

private:
    struct MutexInternal;
    MutexInternal* mInternal;

};

}
}
#endif //FRAMEWORK_INCLUDE_INFRA_MUTEX_H
