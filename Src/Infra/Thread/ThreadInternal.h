
#ifndef FRAMEWORK_SRC_INFRA_THREADINTERNAL_H
#define FRAMEWORK_SRC_INFRA_THREADINTERNAL_H

#include <map>
#include <string>
#include "Infra/Thread/ThreadLite.h"
#include "Infra/Concurrence/Guard.h"
#include "Infra/Concurrence/Mutex.h"
#include "Infra/Thread/ThreadManager.h"
#include "Infra/Concurrence/Semaphore.h"

namespace Uface {
namespace Infra {


typedef pthread_t ThreadHandle;
struct ThreadInternal {

    ThreadHandle                            mHandle;                /**thread句柄*/

    bool		                            mRunning;               /**线程running标记*/
    int32_t		                            mPolicy;                /**线程执行策略*/
    int32_t		                            mPriority;              /**线程优先级*/
    int32_t		                            mStackSize;             /**线程堆栈大小*/
    int32_t		                            mId;                    /**线程Id*/
    char	                                mName[32];              /**线程名*/
    ThreadInternal*                         mPrev;			        /**上一个线程*/
    ThreadInternal*                         mNext;			        /**下一个线程*/
    CSemaphore	                            mSemaphore;	            /**该信号量用来防止同一个对象的线程同时创建多次*/
    uint64_t	                            mExpectedTime;		    /**预计执行时间,0表示不预计*/
    bool	                                mLoop;                  /**线程执行体循环标记*/
    bool	                                mDestroyed;             /**销毁是否标记*/
    bool	                                mDestroyBlock;          /**是否等待线程阻塞退出*/
    ThreadManagerInternal*                  mManager;               /**绑定的线程管理者*/
    CThread*		                        mOwner;                 /**所属CThread*/
    CMutex                                  mMutex;				    /**互斥量*/
};


struct CThreadLite::ThreadLiteInternal {
    CThreadLite::ThreadProc mProc;
};

/**
 * @brief 线程管理类
 */
struct ThreadManagerInternal {

    friend struct ThreadInternal;
    friend class  CThread;

    ThreadManagerInternal();

public:
    /**
     * @brief 单例
     * @return
     */
    static ThreadManagerInternal* instance();

public:
    /**
     * @brief 注册主线程，只要在主线程中调用,这样在显示所有线程的信息时可以看到主线程的信息
     */
    void registerMainThread();
    /**
     * @brief 打印线程信息
     */
    void dumpThreads();
    /**
     * @brief 通过线程Id 获取线程名
     * @param[in]  id   线程Id
     * @param[out] name 线程名
     * @return
     */
    bool getThreadNameById(int32_t id, std::string &name);
    /**
     * @brief 获取超时线程名字
     * @param[out] names
     * @return
     */
    bool getTimeOutThreads(std::string &names);
    /**
     * @brief   将线程添加进链表
     * @param[in] internal  线程对象指针
     * @return
     */
    bool addThread(ThreadInternal *internal);
    /**
     * @brief  将线程从链表移除
     * @param[in] internal  线程对象指针
     * @return
     */
    bool removeThread(ThreadInternal *internal);
    /**
     * @brief  获取线程Id对应的线程对象
     * @param[in] tid   线程Id
     * @return
     */
    ThreadInternal * getThread(int tid);

private:
    ThreadInternal*                 mHead;		    /**线程链表头指针*/
    CMutex                          mMutex;		    /**互斥量*/
    int                             mMainThreadId;  /**主线程ID*/
};

class CThreadLoadingController : public CThread {

    CThreadLoadingController();

public:
    /**
     * @brief 单例
     * @return
     */
    static CThreadLoadingController* instance();

    /**
     * @brief 将负载调节线程加入到链表
     * @param pid
     * @param onCycle
     * @param offCycle
     */
    void addThread(ThreadHandle pid, int onCycle, int offCycle);

    /**
     * @brief 将负载调节线程从链表删除
     * @param pid
     */
    void removeThread(ThreadHandle pid);

    /**
     * @brief 线程调节线程回调
     */
    void threadProc( );

private:
    /**负载相关的变量*/
    struct ThreadLoadingControl {
        int32_t onCycle;
        int32_t offCycle;
        int32_t onElapsed;
        int32_t offElapsed;
    };

    typedef std::map<ThreadHandle, ThreadLoadingControl> LCTS;
    LCTS        mThreads;
    CMutex      mMutex;
};

}
}

#endif //FRAMEWORK_SRC_INFRA_THREADINTERNAL_H
