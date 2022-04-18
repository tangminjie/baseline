/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   LoggerImol.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-05 00:22
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_SRC_LOGGER_LOGGERIMOL_H
#define APPLICATION_SRC_LOGGER_LOGGERIMOL_H

#include <string>
#include "Logger/Logger.h"
#include "Infra/Time/Timer.h"
//#include "Oss/IStorageCloud.h"
#include "Infra/Concurrence/Mutex.h"


class LoggerImpl : public ILogger {

public:
    /**
     * @brief 构造函数
     */
    LoggerImpl();

    /**
     * @brief 析构函数
     */
    virtual ~LoggerImpl();

    /**
     * @brief 初始化
     * @param[in] app 模块名
     * @param[in] size   日志缓冲区大小
     * @return
     */
    virtual bool initial(const char *app, int32_t size);

    /**
     * @brief 绑定backtrace 回调
     * @param[in] proc 回调
     * @return
     */
    virtual bool attachBtProc(BacktraceProc proc);

    /**
     * @brief 解绑backtrace 回调
     * @param[in] proc 回调
     * @return
     */
    virtual bool detachBtProc(BacktraceProc proc);

    /**
     * @brief 设置日志输出等级
     * @param[in] level 日志级别
     * @return
     */
    virtual bool setLoggerLevel(LoggerLevel level);

    /**
     * @brief 日志输出
     * @param[in] level         等级
     * @param[in] module        模块名
     * @param[in] fmt           格式化
     * @param[in] ...           可变参数
     * @return
     */
    virtual int32_t print(LoggerLevel level, const char *module, const char *fmt, ...);

public:
    /**
     * @brief backtrace 回调
     * @param[in] type  backtrace 类型
     */
    void backtraceCb(BtExitType type);
    /**
     * @brief 日志上传检测定时器
     * @param param
     */
    void onTimer(uint64_t param);
private:
    bool                               mInited;
    char*                              mBuffer;
    Uface::Infra::CMutex               mMutex;
    LoggerLevel                        mLevel;
    int32_t                            mSize;
    BacktraceSignal                    mBtSignal;
    Uface::Infra::CTimer               mTimer;
};

#endif //APPLICATION_SRC_LOGGER_LOGGERIMOL_H
