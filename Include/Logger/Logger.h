

#ifndef APPLICATION_INCLUDE_LOGGER_LOGGER_H
#define APPLICATION_INCLUDE_LOGGER_LOGGER_H


#include "Infra/Signal.h"


/**
* @brief 日志操作接口
*/
class ILogger {

public:
    /**
     * @brief 日志单例
     * @return
     */
    static ILogger *instance();

    /**
     * 日志级别
     */
    typedef enum {
        loggerDebug,
        loggerTrace,
        loggerInfo,
        loggerWarn,
        loggerError
    } LoggerLevel;

    /**
     * @brief backtrace 程序退出的类型
     */
    typedef enum {
        exitReboot,
        exitTerminate
    } BtExitType;

    /**
     * @brief backtrace 回调
     */
    typedef ArcFace::Infra::TSignal1 <BtExitType> BacktraceSignal;
    typedef BacktraceSignal::Proc BacktraceProc;

public:
    /**
     * @brief 析构函数
     */
    virtual ~ILogger() {}

    /**
     * @brief 初始化
     * @param[in] app    应用名
     * @param[in] size   日志缓冲区大小
     * @return
     */
    virtual bool initial(const char *app, int32_t size) = 0;
    /**
     * @brief 绑定backtrace 回调
     * @param proc
     * @return
     */
    virtual bool attachBtProc(BacktraceProc proc) = 0;

    /**
     * @brief 解绑backtrace 回调
     * @param proc
     * @return
     */
    virtual bool detachBtProc(BacktraceProc proc) = 0;

    /**
     * @brief 设置日志输出等级
     * @param[in]   level 日志级别
     * @return
     */
    virtual bool setLoggerLevel(LoggerLevel level) = 0;

    /**
     * @brief 日志输出
     * @param[in] level         等级
     * @param[in] module        模块名
     * @param[in] fmt           格式化
     * @param[in] ...           可变参数
     * @return
     */
    virtual int32_t print(LoggerLevel level, const char *module, const char *fmt, ...) = 0;
};




#endif //APPLICATION_INCLUDE_LOGGER_LOGGER_H
