/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   rtspServer.h
 * Author:     tangminjie
 * Email:      tangminjie_130@163.com
 * Version:    V1.0.0
 * Date:       2021-04-02 10:39
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_INCLUDE_PRIVATECHANNEL_PRIVATECHANNEL_H
#define APPLICATION_INCLUDE_PRIVATECHANNEL_PRIVATECHANNEL_H

#include "Client/Client.h"

namespace Uface {
namespace RtspServer {

class RtspServer: public Application::IClient {

public:
    /**
     * @brief 获取私有通道Client
     * @return
     */
    static RtspServer* instance();

public:
    /**
     * @brief 构造函数
     */
    RtspServer();
    /**
     * @brief 析构函数
     */
    virtual ~RtspServer();
    /**
     * @brief 初始化函数
     */
    virtual void initial();
    /**
     * @brief 开启私有通道
     */
    virtual void start();
    /**
     * @brief 停止私有通道
     */
    virtual void stop();
};

}
}
#endif //APPLICATION_INCLUDE_PRIVATECHANNEL_PRIVATECHANNEL_H
