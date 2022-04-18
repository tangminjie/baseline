

#ifndef FRAMEWORK_INCLUDE_INFRA_READWRITEMUTEX_H
#define FRAMEWORK_INCLUDE_INFRA_READWRITEMUTEX_H

#include "Infra/Define.h"

namespace Uface {
namespace Infra {

/**
 * @brief 读写锁
 */
class CReadWriteMutex {

    CReadWriteMutex(const CReadWriteMutex&);
    CReadWriteMutex& operator=(const CReadWriteMutex&);

public:

    typedef enum {
        PRIORITY_DEFAULT,           /**默认优先级,读优先*/
        PRIORITY_WRITE              /**写锁优先级,写优先*/
    } RW_PRIORITY;

public:
    /**
     * @brief 构造函数
     */
    CReadWriteMutex();
    /**
     * @brief 构造函数
     * @param[in] priority 优先级
     */
    CReadWriteMutex(RW_PRIORITY priority);
    /**
     * @brief 析构函数
     */
    ~CReadWriteMutex();
    /**
     * @brief 进入读临界区
     * @return
     */
    bool enterReading();
    /**
     * @brief 进入写临界区
     * @return
     */
    bool enterWriting();
    /**
     * @brief 离开临界区
     * @return
     */
    bool leave();

private:
    struct RWMutexInternal;
    RWMutexInternal*  mInternal;
};
}
}
#endif //FRAMEWORK_INCLUDE_INFRA_READWRITEMUTEX_H
