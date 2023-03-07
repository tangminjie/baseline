
#ifndef FRAMEWORK_INCLUDE_INFRA_RECURSIVEMUTEX_H
#define FRAMEWORK_INCLUDE_INFRA_RECURSIVEMUTEX_H

#include "Infra/Define.h"

namespace ArcFace {
namespace Infra {

class CRecursiveMutex {

    CRecursiveMutex(const CRecursiveMutex&);
    CRecursiveMutex &operator=(const CRecursiveMutex&);

public:
    /**
     * @brief 构造函数
     */
    CRecursiveMutex();
    /**
     * @brief 析构函数
     */
    ~CRecursiveMutex();
    /**
     * @brief 进入临界区
     * @return
     */
    bool enter();
    /**
     * @brief 退出临界区
     * @return
     */
    bool leave();

private:
    struct RecursiveMutexInternal;
    RecursiveMutexInternal* mInternal;
};
}
}
#endif //FRAMEWORK_INCLUDE_INFRA_RECURSIVEMUTEX_H
