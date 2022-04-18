/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   ClientManager.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-03-31 14:49
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_INCLUDE_CLIENT_CLIENTMANAGER_H
#define APPLICATION_INCLUDE_CLIENT_CLIENTMANAGER_H

#include "Client/Client.h"

namespace Uface {
namespace Application {

class ClientManager {

    ClientManager();
    virtual ~ClientManager(){};

public:
    /**
     * @brief Client Manager
     * @return
     */
    static ClientManager* instance();

public:
    /**
     * @brief 注册client
     * @param[in] client  客户端
     * @return true: 注册成功,false:注册失败
     */
    bool registerClient(IClient *client);
    /**
     * @brief 注销client
     * @param[in] client  客户端
     * @return
     */
    bool unregisterClient(const IClient* client);
    /**
     * @brief 是否存在Client
     * @param[in] clientId 客户端Id
     * @return
     */
    bool hasClient(const char* clientId);
    /**
     * @brief 开始操作
     * @return
     */
    bool start();
    /**
     * @brief 停止操作
     * @return
     */
    bool stop();
    /**
     * @brief 是否完成初始化
     * @return
     */
    bool started();
    /**
     * @brief 停止并等待任务结束退出
     * @return
     */
    bool wait();

private:
    class ClientManagerInternal;
    ClientManagerInternal*    mInternal;
};


}
}
#endif //APPLICATION_INCLUDE_CLIENT_CLIENTMANAGER_H
