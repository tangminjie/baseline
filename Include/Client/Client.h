/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   IClient.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-03-31 03:18
 * Description: Client抽象
 * Others:
 *************************************************/


#ifndef APPLICATE_INCLUDE_CLIENT_H
#define APPLICATE_INCLUDE_CLIENT_H

#include "jsoncpp/value.h"

namespace Uface {
namespace Application {

/**
 * @brief 抽象Client对象
 * <p> 对于核心模块,接入模块就是Client;外部接入对象必须继承该类,
 *
 */
class IClient {

public:
    /**
     * @brief 构造函数
     * @param[in] clientId 客户端Id,由用户自定义
     * @param[in] info     客户端信息，但前用不到,后续扩展使用
     * <p> 该构造函数会校验 clientId
     */
    IClient(const char* clientId,const Json::Value& info = Json::Value::null);
    /**
     * @brief 客户端
     */
    virtual ~IClient();
    /**
     * @brief 获取client id
     * <p> 由内部模块调用
     * @return clientId
     */
    const char* getClientId() const;
    /**
     * @brief 获取client信息
     * @return 返回client info信息
     */
    Json::Value getClientInfo() const;

public:
    /**
     * @brief client初始化,子类实现
     * <p> 由内部触发调用
     */
    virtual void initial() = 0;
    /**
     * @brief start client,子类实现
     * <p> 由内部触发调用
     */
    virtual void start() = 0;
    /**
     * @brief stop client,子类实现
     * <p> 由内部触发模块调用
     */
    virtual void stop() = 0;

private:
    class ClientInternal;
    ClientInternal*     mInternal;
};

}
}
#endif //APPLICATE_INCLUDE_CLIENT_H
