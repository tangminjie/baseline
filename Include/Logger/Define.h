/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   Define.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-05 01:54
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_INCLUDE_LOGGER_DEFINE_H
#define APPLICATION_INCLUDE_LOGGER_DEFINE_H

#include "Logger/Logger.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#ifdef LOG_LIB_MODULE

#define loggerDebugf(format,...)          ILogger::instance()->print(ILogger::loggerDebug,LOG_LIB_MODULE,format,##__VA_ARGS__)
#define loggerTracef(format,...)          ILogger::instance()->print(ILogger::loggerTrace,LOG_LIB_MODULE,format,##__VA_ARGS__)
#define loggerInfof(format,...)           ILogger::instance()->print(ILogger::loggerInfo,LOG_LIB_MODULE,format,##__VA_ARGS__)
#define loggerWarnf(format,...)           ILogger::instance()->print(ILogger::loggerWarn,LOG_LIB_MODULE,format,##__VA_ARGS__)
#define loggerErrorf(format,...)          ILogger::instance()->print(ILogger::loggerError,LOG_LIB_MODULE,format,##__VA_ARGS__)

#else 
#define loggerDebugf(format,...)          SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(),format, ##__VA_ARGS__);
#define loggerTracef(format,...)          SPDLOG_LOGGER_TRACE(spdlog::default_logger_raw(),format, ##__VA_ARGS__);
#define loggerInfof(format,...)           SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(),format, ##__VA_ARGS__);
#define loggerWarnf(format,...)           SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(),format, ##__VA_ARGS__);
#define loggerErrorf(format,...)          SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(),format, ##__VA_ARGS__);

// #else

// #define loggerDebugf(format,...)          ILogger::instance()->print(ILogger::loggerDebug,0,format,##__VA_ARGS__)
// #define loggerTracef(format,...)          ILogger::instance()->print(ILogger::loggerTrace,0,format,##__VA_ARGS__)
// #define loggerInfof(format,...)           ILogger::instance()->print(ILogger::loggerInfo,0,format,##__VA_ARGS__)
// #define loggerWarnf(format,...)           ILogger::instance()->print(ILogger::loggerWarn,0,format,##__VA_ARGS__)
// #define loggerErrorf(format,...)          ILogger::instance()->print(ILogger::loggerError,0,format,##__VA_ARGS__)

#endif

#define tracepoint()                    loggerDebugf("trace :{}:%d",__FILE__,__LINE__)

#define debugf(format,...)               loggerDebugf(format, ##__VA_ARGS__)
#define tracef(format,...)               loggerTracef(format,##__VA_ARGS__)
#define infof(format,...)                loggerInfof(format,##__VA_ARGS__)
#define warnf(format,...)                loggerWarnf(format,##__VA_ARGS__)
#define errorf(format,...)               loggerErrorf(format,##__VA_ARGS__)

#include <cassert>
#define ARCFACE_ASSERT(condition,fmt)     do { if (!(condition)) {errorf(fmt);assert(0);}} while(0);

#endif //APPLICATION_INCLUDE_LOGGER_DEFINE_H
