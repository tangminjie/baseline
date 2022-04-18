/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   AppClient.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-12 02:52
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_INCLUDE_CLIENT_APPCLIENT_H
#define APPLICATION_INCLUDE_CLIENT_APPCLIENT_H

#include "Client/Client.h"

namespace Uface {
namespace Application {

class AppClient: public IClient {

public:
    /**
     * @brief 单例
     * @return
     */
    static AppClient* instance();

public:
    /**
     * @brief 构造函数
     */
    AppClient();
    /**
     * @brief 析构函数
     */
    virtual ~AppClient();
    /**
     * @brief client初始化
     * <p> 由内部触发调用
     */
    virtual void initial() {}
    /**
     * @brief start client
     * <p> 由内部触发调用
     */
    virtual void start() {}
    /**
     * @brief stop client
     * <p> 由内部触发模块调用
     */
    virtual void stop() {}
};
}
}
#endif //APPLICATION_INCLUDE_CLIENT_APPCLIENT_H
