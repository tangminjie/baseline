

#ifndef FRAMEWORK_INCLUDE_INFRA_SEMAPHORE_H
#define FRAMEWORK_INCLUDE_INFRA_SEMAPHORE_H


#include "Infra/Define.h"

namespace Uface{
namespace Infra{

/**
 * @brief 信号量类
 */
class CSemaphore {

    CSemaphore(CSemaphore const&);
    CSemaphore& operator=(CSemaphore const&);

public:
    /**
     * @brief 构造函数
     * @param initialCount 信号量初始计数
     */
    explicit CSemaphore(int32_t initialCount = 0);
    /**
     * @brief 析构函数
     */
    virtual ~CSemaphore();
    /**
     * @brief  减少信号量计数,如果减少到0,会阻塞调用线程
     * @return 当前信号量计数
     */
    int32_t pend();
    /**
     * @brief  增加信号量计数,如果是从0累加,会唤醒等待队列的第一个线程
     * @return 当前信号量计数
     */
    int32_t post();
    /**
     * @brief  减少信号量计数,如果已经减少到0,会阻塞调用的线程直到超时
     * @param[in] timeout  超时时间,单位ms
     * @return 当前信号量计数,返回-1 表示超时
     */
    int32_t pend(uint32_t timeout);
    /**
     * @brief 尝试减少信号量;如果信号量已经为0,则马上返回
     * @note  可以通过getLastError() 查看错误号
     * @return 0表示信号量减少成功,-1表示信号量减少失败
     */
    int32_t tryPend();

private:
    struct SemaphoreInternal;
    SemaphoreInternal* mInternal;
};

} // namespace Infra
} // namespace Uface

#endif //FRAMEWORK_INCLUDE_INFRA_SEMAPHORE_H
