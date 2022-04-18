

#ifndef FRAMEWORK_INCLUDE_INFRA_THREAD_THREADMANAGER_H
#define FRAMEWORK_INCLUDE_INFRA_THREAD_THREADMANAGER_H

#include <string>
#include "Infra/Thread/Thread.h"

namespace Uface {
namespace Infra {

struct ThreadManagerInternal;

/**
 * @brief  线程管理类
 */
class CThreadManager {

    CThreadManager();
    CThreadManager(const CThreadManager&);
    CThreadManager& operator=(const CThreadManager&);

public:
    /**
     * @brief  线程管理类单例
     * @return
     */
    static CThreadManager* instance();
    /**
     * 线程管理类析构
     */
    ~CThreadManager();
    /**
     * @brief 注册主线程，只能在主线程中调用;显示所有线程的同时可以看到主线程的信息
     */
    void registerMainThread();
    /**
     * @brief 打印所有线程
     */
    void printThreads();
    /**
     * 通过Id 获取线程名
     * @param[in]   id      线程Id
     * @param[out]  name    线程名
     * @return
     */
    bool getThreadNameById(int32_t id, std::string &name);
    /**
     * @brief 获取超时线程的名
     * @param[out] name     线程名
     * @return
     */
    bool getTimeoutThreadName(std::string &name);
    /**
     * @brief 判断是否存在线程
     * @param[in] id        线程Id
     * @return
     */
    bool hasThread(int32_t id);

private:
    ThreadManagerInternal*     mInternal;
};
}
}
#endif //FRAMEWORK_INCLUDE_INFRA_THREAD_THREADMANAGER_H
