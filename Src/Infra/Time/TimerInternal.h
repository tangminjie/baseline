
#include "Infra/Time/Timer.h"
#include "Infra/Thread/Thread.h"
#include "Infra/Concurrence/Guard.h"
#include "Infra/Concurrence/Semaphore.h"

namespace Uface {
namespace Infra {

class CTimerThread;

struct TimerInternal {

    uint64_t            mRunningTime;     /** 定时器执行累计时间,单位微秒*/
    uint64_t            mCallTime;        /** 定时器调用时间*/
    uint64_t            mCheckTime;       /** 检测时间*/
    uint32_t            mPeriod;          /** 定期器周期*/
    uint32_t            mTimeout;         /** 定时器回调执行超时时间*/
    CTimer::Proc        mFunc;            /** 回调函数*/
    uint64_t            mParam;           /** 用户自定义参数*/
    char                mName[32];        /** 定时器名字*/
    bool                mStarted;         /** 定时器是否开始*/
    bool                mCalled;          /** 定时器是否被回调*/

    TimerInternal*      mPrev;            /**上一个定时器*/
    TimerInternal*      mNext;			  /**下一个定时器*/
    CTimerThread*       mThread;          /**执行线程*/
    bool                mStatistic;		  /**是否统计运行时间*/
    bool                run();            /**内部执行体*/
};


/**
 * @brief 定时器线程类，仅共CTimer; 使用线程池模式
 */
class CTimerThread : public CThread {

    friend class  CTimer;
    friend struct TimerInternal;
    friend struct TimerManagerInternal;

private:
    /**
     * @brief 构造函数
     */
    CTimerThread();
    /**
     * @brief 析构函数
     */
    ~CTimerThread();
    /**
     * @brief 现场执行体
     */
    void threadProc();
    /**
     * @brief 取消线程执行
     */
    void cancel();

private:
    uint64_t                        mParam;
    CTimer::Proc                    mAsyncFunc;
    CSemaphore                      mSemaphore;
    CTimerThread*                   mNextPooled;
    TimerInternal*                  mCaller;
    bool                            mCancel;
};

/**
 * @brief 定时器管理类；使用高精度系统定时器来驱动应用定时器工作
 *
 * 高精度定时器每次触发时，检查所有应用定时器的状态,决定是否调用其回调
 *    - Win32  使用多媒体定时器,周期1ms
 *    - pSOS   使用tmdlTimer组件,周期为1ms
 *    - ucLinux2.4 使用系统信号,周期为10ms
 */
struct TimerManagerInternal : public CThread {

    friend class  CTimer;
    friend class  CTimerThread;
    friend struct TimerInternal;
    /**
     * @brief 构造函数
     */
    TimerManagerInternal();

public:
    /**
     * @brief 单例
     * @return
     */
    static TimerManagerInternal* instance();

    /**
     * @brief 析构函数
     */
    ~TimerManagerInternal();

    /**
     * @brief 打印所有定时器
     */
    void dumpTimers();

    /**
     * @brief 限制线程池空闲线程数
     * @param count
     * @return
     */
    bool limitIdlePooled(int count);

    /**
     * @brief 空闲数让构造函数能够调用
     */
    void doNothing();

private:
    /**
     * @brief 增加定时器
     * @param pTimer
     * @return
     */
    bool addTimer(TimerInternal * pTimer);
    /**
     * @brief 删除定时器
     * @param pTimer
     * @return
     */
    bool removeTimer(TimerInternal * pTimer);
    /**
     * @brief 执行题
     */
    void threadProc();
    /**
     * 获取定时器绑定线程
     * @return
     */
    CTimerThread* getTimerThread();
    /**
     *
     * @param pThread
     */
    void putTimerThread(CTimerThread* pThread);

private:
    CTimerThread*                   mHeadPooled;
    CSemaphore                      mSemaphore;
    uint64_t                        mCurrentTime;	        /**毫秒计数*/
    TimerInternal*                  mPHead;
    CRecursiveMutex                 smMutex;
    int32_t                         mLimitThreadCount;     /**线程池空闲线程数的上限,负数表示不限制*/
    int32_t                         mIdleThreadCount;      /**空闲线程数量*/

    static Infra::CMutex            smInstanceMutex;
};

} // namespace Infra
} // namespace Uface
