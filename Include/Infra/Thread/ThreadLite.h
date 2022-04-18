

#ifndef FRAMEWORK_INCLUDE_INFRA_THREAD_THREADLITE_H
#define FRAMEWORK_INCLUDE_INFRA_THREAD_THREADLITE_H

#include "Infra/Thread/Thread.h"

namespace Uface {
namespace Infra {
/**
 * @brief  线程类，提供组合方式创建线程
 */
class CThreadLite:public CThread {

    CThreadLite(const CThreadLite&);
    CThreadLite& operator=(const CThreadLite&);

public:

    typedef TFunction1<void,CThreadLite&>   ThreadProc;

public:
    /**
     * @brief 构造函数
     * @param[in] proc      线程执行体
     * @param[in] name      线程名称; 不一定会传给系统,但CThreadManager 管理线程会用到
     * @param[in] priority  线程优先级
     * @param[in] policy    线程调度策略
     * @param[in] stackSize 指定线程推展大小;若 <= 平台要求的值,则使用平台默认值
     */
    CThreadLite(ThreadProc proc, const char* name,int32_t priority = priorDefault,
                int32_t policy = policyNormal,int32_t stackSize = 0);
    /**
     * @brief 析构函数
     * @note 如果线程还在执行，会销毁线程
     */
    virtual ~CThreadLite();

private:
    /**
     * @brief 线程执行体
     */
    virtual void threadProc();

private:
    struct ThreadLiteInternal;
    ThreadLiteInternal*    mInternal;
};

}
}
#endif //FRAMEWORK_INCLUDE_INFRA_THREAD_THREADLITE_H
