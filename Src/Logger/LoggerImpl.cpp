/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   LoggerImpl.cpp
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-05 00:25
 * Description: 
 * Others:
 *************************************************/

#include <stdarg.h>
#include "LoggerImpl.h"
//#include "System/System.h"
#include "Infra/Time/Time.h"
// #include "System/StorageManager.h"
#include "Logger/Define.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

/**
* @brief core dump backtrace callback
* @param enableWD
*/
static void btCb(int32_t enableWD);

ILogger *ILogger::instance() {
    static LoggerImpl logger;
    return &logger;
}

LoggerImpl::LoggerImpl() : mInited(false),
                           mBuffer(nullptr), mLevel(loggerDebug),
                           mSize(0), mBtSignal(64), mTimer("logger") {
   // mStorageClient = ArcFace::cloudStorage::IStorageCloud::createSCClient("alioss");
}

LoggerImpl::~LoggerImpl() {
    if (mBuffer != nullptr) {
        free(mBuffer);
        mBuffer = nullptr;
    }
    spdlog::drop_all();
    //delete mStorageClient;
}

bool LoggerImpl::initial(const char *app, int32_t size) {


    if (mInited) {
        return false;
    }

    if (app == nullptr || strlen(app) <= 0) {
        fprintf(stderr,"initial logger module invalid");
        return false;
    }

    if (size <= 0) {
        fprintf(stdout,"initial logger buffer size(%d) <= 0\n",size);
        return false;
    }
    //日志文件初始化
    // 按文件大小
    //auto file_logger = spdlog::rotating_logger_mt("file_log", "log/log.log", 1024 * 1024 * 100, 3);
    // 每天2:30 am 新建一个日志文件
    auto logger = spdlog::daily_logger_mt("daily_logger", "/tmp/logs/daily.txt", 2, 30);
    // 遇到warn flush日志，防止丢失
    logger->flush_on(spdlog::level::warn);
    //每三秒刷新一次
    spdlog::flush_every(std::chrono::seconds(3));
    
    // Set the default logger to file logger
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug

    // change log pattern
    // %s：文件名
    // %#：行号
    // %!：函数名
    spdlog::set_pattern("%Y-%m-%d %H:%M:%S [%l] [%t] - <%s>|<%#>|<%!>,%v");

    tracef("LoggerImpl start");
    infof("LoggerImpl start");

    // univ_gcore_init(btCb,GCORE_EXIT_TERMINATE);

    if (mBuffer != nullptr) {
        free(mBuffer);
        mBuffer = nullptr;
    }

    mSize = size + 1;
    mBuffer = (char*)malloc(mSize);
    mLevel = loggerDebug;
    //mTimer.start(ArcFace::Infra::CTimer::Proc(&LoggerImpl::onTimer, this), 1000, 600*1000);
    mInited = true;

    return mInited;

}


bool LoggerImpl::attachBtProc(ILogger::BacktraceProc proc) {

    if (!mInited) {
        fprintf(stderr, "logger not initial,attach backtrace proc failed");
        return false;
    }

    return mBtSignal.attach(proc) > 0;
}

bool LoggerImpl::detachBtProc(ILogger::BacktraceProc proc) {
    if (!mInited) {
        fprintf(stderr, "logger not initial,detach backtrace proc failed");
        return false;
    }

    return mBtSignal.detach(proc) < 0;
}

 bool LoggerImpl::setLoggerLevel(LoggerLevel level){
    return true;
 }
//  {
//     if (!mInited) {
//         return false;
//     }

//     int32_t vlogLevel = VLOG_LEVEL_DEBUG;
//     switch (level) {
//         case loggerTrace:
//         case loggerInfo:
//             vlogLevel = VLOG_LEVEL_INFO;
//             break;
//         case loggerWarn:
//         case loggerError:
//             vlogLevel = VLOG_LEVEL_ERROR;
//             break;
//         default:
//             vlogLevel = VLOG_LEVEL_DEBUG;
//             break;
//     }

//     mLevel = level;
//     vlog_level(vlogLevel);
//     return true;
// }

int32_t LoggerImpl::print(LoggerLevel level, const char *module, const char *fmt, ...) {

    if (!mInited) {
        return 0;
    }

    va_list ap;
    va_start(ap, fmt);
    ArcFace::Infra::CGuard guard(mMutex);
    memset(mBuffer,0x00, mSize);
    int32_t logLength = vsnprintf(mBuffer, mSize - 1, fmt, ap);
    va_end(ap);

    if (logLength <= 0) {
        return -1;
    } else if (logLength >= mSize) {
        logLength = mSize - 1;
    }

    mBuffer[logLength] = '\0';

#ifdef __LOG_CONSOLE
    printf("%s\n",mBuffer);
#else
    switch (level)
    {
    case loggerTrace: /* constant-expression */
    /* code */
    case loggerInfo:
    
    break;
    case loggerWarn:
    case loggerError:
    
    break;
        break;
    
    default:
        break;
    }
#endif
    return logLength;
}

void LoggerImpl::backtraceCb(ILogger::BtExitType type) {
    mBtSignal(type);
}

void LoggerImpl::onTimer(uint64_t param) {

}
// void LoggerImpl::onTimer(uint64_t param) 
// {

//     (void)param;
//     std::string logDir = "/data/record/log/compress/";
//     int64_t space = ArcFace::Application::StorageManager::instance()->getFileSpace(logDir.c_str());
// //    tracef("compress space:%d", space);
//     if (space > 0) {
//         std::vector<std::string> files = ArcFace::Application::StorageManager::instance()->getSubFile(logDir.c_str());
//         Json::Value devInfo = Json::nullValue;
//         ArcFace::Application::ISystem::instance()->getDeviceInfo(devInfo);
//         char date[128] = {0};
//         ArcFace::Infra::CTime time = ArcFace::Infra::CTime::getCurrentTime();
//         snprintf(date, sizeof(date) - 1,"/%04d-%02d-%02d/",
//                  time.year, time.month, time.day);

//         for (int i= 0; i < files.size();i++) {
//             std::string dir = "release/hisi/log/" + devInfo["deviceNo"].asString() + date;
//             std::string objectName = dir + files[i];
//             std::string filePath = logDir + files[i];
//             if (strstr(files[i].c_str() , "log.bz2") == nullptr) {
//                 continue;
//             }
//             if (ArcFace::Application::StorageManager::instance()->fileIsEmpty(filePath.c_str())) {
//                 continue;
//             }

//             Json::Value ossInfo = ArcFace::PrivateChannel::DeviceConfig::instance()->getOssStorageInfo();
//             ossInfo["bucket"] = "wo-device";
//             mStorageClient->setStorageParam(ossInfo);
//             if (mStorageClient->uploadFile(filePath,objectName)) {
//                 std::string url;
//                 url.append("https://").append(ossInfo["bucket"].asString()).append(".").append(
//                         ossInfo["storageUrl"].asString()).append("/").append(objectName);
// //                tracef("log url : %s", url.c_str());
//                 ArcFace::Application::StorageManager::instance()->unlinkFile(filePath.c_str());
//             }
//         }
//     }
// }

static void btCb(int32_t enableWD) {

    // if (enableWD == GCORE_EXIT_TERMINATE) {
    //     LoggerImpl *logger = reinterpret_cast<LoggerImpl *>(ILogger::instance());
    //     logger->backtraceCb(ILogger::exitTerminate);
    // }
}

