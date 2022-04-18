
#ifndef FRAMEWORK_INCLUDE_INFRA_THREAD_H
#define FRAMEWORK_INCLUDE_INFRA_THREAD_H

#include <string.h>
#include "Infra/Define.h"
#include "Infra/Function.h"

namespace Uface {
namespace Infra {


struct ThreadInternal;
/**
 * @brief 线程类，提供继承方式创建线程
 */
class CThread {

    CThread(const CThread&);
    CThread& operator=(const CThread&);

public:

    /**
     * 优先级
     */
    typedef enum {
        priorTop = 1,
        priorBottom = 127,
        priorDefault = 64,
    } Priority;

    /**
     * 调度策略
     */
    typedef enum {
        policyNormal,               /**普通线程*/
        policyRealtime              /**实时线程*/
    } Policy;

public:
    /**
     * @brief  构造函数
     * @param name          线程名
     * @param priority      线程优先级,值越低优先级越高,取值范围 priorTop ~ priorBottom
     * @param policy        线程调度策略
     * @param stackSize     指定线程堆栈大小
     */
    CThread(const char* name,int32_t priority = priorDefault,
                        int32_t policy = policyNormal,int32_t stackSize = 0);
    /**
     * @brief 析构函数
     * @note  若线程还在执行，会销毁线程
     */
    virtual ~CThread();
    /**
     * @brief 线程执行体
     */
    virtual void threadProc() = 0;
    /**
     * @brief 创建线程
     * @return
     */
    bool createThread();
    /**
     * @brief 销毁线程,设置线程退出标志
     * @return
     */
    bool destroyThread();
    /**
     * @brief 终止线程
     * @note 和销毁线程不同在于它是由操作系统强制销毁线程，不保证用户数据安全
     * @return
     */
    bool terminateThread();
    /**
     * @brief 取消线程，不等待线程结束
     * @return
     */
    bool cancelThread();
    /**
     * @brief 判断线程是否还在运行
     * @return
     */
    bool isThreadOver();
    /**
     * @brief 获取当前线程id
     * @return
     */
    int32_t getThreadId();
    /**
     * @brief 设置线程名字
     * @param name
     */
    void setThreadName(const char* name);
    /**
     * @brief 获取线程名字
     */
    char* getThreadName();
    /**
     * @brief 设置线程超时时间
     * @param[in] milliSeconds 单位毫秒,设置为0表示清空设置
     */
    void setTimeout(int32_t milliSeconds);
    /**
     * @brief  设置超时时间，主要用于优化继承类中实现，减少重复获取当前时间的开销
     * @param[in] now  当前时间(单位毫秒)
     * @param milliSeconds 超时时间(毫秒)
     */
    void setTimeout(uint64_t now,int32_t milliSeconds);
    /**
     * @brief 判断线程是否超时
     */
    bool isTimeout();
    /**
     * @brief 线程退出标志, 线程执行体根据该标记退出
     * @return
     */
    bool looping() const;

public:
    /**
     * @brief 设置线程名
     * @param name
     */
    static bool setCurrentThreadName(const char* name);
    /**
     * @brief  获取线程Id
     * @return
     */
    static int32_t getCurrentThreadId();
    /**
     * @brief 设置当前线程超时时间
     * @param milliSeconds
     * @return
     */
    static bool setCurrentTimeout(int32_t milliSeconds);
    /**
     * @brief 让调度线程阻塞一段时间
     * @param ms 单位ms
     */
    static void sleep(int ms);
    /**
     * @brief 调节线程CPU负载，避免长时间占用CPU
     * @param percent 取之1～100, 1 表示最慢,100表示正常速度
     */
    static void load(int32_t percent);

private:
    ThreadInternal*     mInternal;
};

}
}
#endif //FRAMEWORK_INCLUDE_INFRA_THREAD_H
