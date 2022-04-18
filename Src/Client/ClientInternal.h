/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   ClientInternal.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-03-31 15:22
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_SRC_CLIENT_CLIENTINTERNAL_H
#define APPLICATION_SRC_CLIENT_CLIENTINTERNAL_H

#include <map>
#include <string>
#include "Client/Client.h"
#include "Client/ClientManager.h"
#include "Infra/Concurrence/Guard.h"
#include "Infra/Concurrence/Semaphore.h"

namespace Uface {
namespace Application {

class IClient::ClientInternal {
    friend class IClient;

    ClientInternal(const char *id,const Json::Value& info):
                        mClientId(id),mInfo(info) {
        mClientId = id;
    }

private:
    Json::Value mInfo;
    std::string mClientId;
};


class ClientManager::ClientManagerInternal {

    friend class ClientManager;

    ClientManagerInternal(): mStopped(false), mSemaphore(0) {
        mClientMap.clear();
    }

    ~ClientManagerInternal() {
        mClientMap.clear();
    }

    bool isStopped() {
        Infra::CGuard guard(mMutex);
        return mStopped;
    }

private:

    typedef std::map<std::string,IClient*>   IClientMap;
    typedef IClientMap::iterator                   IClientIterator;

    bool                           mStopped;
    bool                           mStarted;        /**所有的客户端是否都完成开启*/
    Infra::CMutex                  mMutex;
    Infra::CSemaphore              mSemaphore;
    IClientMap                     mClientMap;
};

}
}
#endif //APPLICATION_SRC_CLIENT_CLIENTINTERNAL_H
