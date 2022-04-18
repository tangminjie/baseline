

#ifndef FRAMEWORK_INCLUDE_INFRA_TIMER_H
#define FRAMEWORK_INCLUDE_INFRA_TIMER_H

#include "Infra/Define.h"
#include "Infra/Function.h"

namespace Uface {
namespace Infra {

struct  TimerInternal;
struct  TimerManagerInternal;

class CTimer {

    CTimer(const CTimer&);
    CTimer& operator=(const CTimer&);

public:

    typedef TFunction1<void,uint64_t >   Proc;
    /**
     * @brief 构造函数
     * @param[in] name  定时器名
     */
    CTimer(const char* name);
    /**
     * @brief 析构函数
     */
    virtual ~CTimer();
    /**
     * @brief 开启定时器
     * @param[in] proc      定时器回调函数
     * @param[in] delay     定时器启动后延迟多长时间调用,单位毫秒; 0 表示立即开始调用
     * @param[in] period    定时器周期,单位毫秒; 0 表示非周期定时器
     * @param[in] param     回调函数参数, 在回调触发时传递给用户回调
     * @param[in] timeout   回调函数执行的超时时间,默认60s,单位毫秒
     * @return
     */
    bool start(Proc proc,uint32_t delay,uint32_t period,uint64_t param = 0,uint32_t timeout = 60*1000);
    /**
     * @brief 停止定时器
     * @param[in] callback  定时器停止时,是否同时调用一下回调函数,只对带延迟的非周期定时器有效
     * @return
     */
    bool stop(bool callback = false);
    /**
     * @brief 获取定时器名
     * @return
     */
    const char* getName();
    /**
     * @brief 设置定时器名
     * @param[in] name
     */
    void setName(const char* name);
    /**
     * @brief 定时器是否开启
     * @note  非周期定时器在调用后状态自动置为关闭
     * @return
     */
    bool isStarted();
    /**
     * @brief 非周期定时器是否已经调过
     * @return
     */
    bool isCalled();
    /**
     * @brief 回调函数是否正在执行
     * @return
     */
    bool isRunning();
    /**
     * @brief 关闭定时器,阻塞至回调函数结束才返回
     * @note  用户析构的是否调用
     * @return
     */
    bool stopAndWait();
    /**
     * @brief 设置定时器执行周期
     * @param[in] period 周期
     */
    void setPeriodTime(uint32_t period);
    /**
     * @brief  开启统计
     * @param[in]  bStat
     */
    void enableStat(bool bStat = true);

private:
    TimerInternal* mInternal;
};

/**
 * @brief 定时器管理类，使用高精度系统定时其来驱动应用定时器工作
 * 高精度定时器每次被触发时，检查所有应用定时器的状态，决定是否调用其回调函数
 * 这个定时器的周期也决定了应用定时器的精度
 *  - Win32使用多媒体定时器，周期为1ms
 *  - pSOS使用tmdlTimer组件，周期为1ms
 *  - ucLinux2.4 使用系统信号，周期为10ms
 */
class CTimerManager {

    CTimerManager();
    CTimerManager(CTimerManager const&);
    CTimerManager& operator=(CTimerManager const&);

public:
    /**
     * @brief 创建定时器管理对象
     * @return
     */
    static CTimerManager* instance();
    /**
     * @brief 析构函数
     */
    ~CTimerManager();

    /**
     * @brief 打印所有线程信息
     */
    void printTimers();

    /**
     * @brief 限制线程池空闲线程数
     * @param count
     * @return
     */
    bool limitIdlePooled(int32_t count);

private:
    TimerManagerInternal* mInternal;
};

}
}
#endif //FRAMEWORK_INCLUDE_INFRA_TIMER_H
