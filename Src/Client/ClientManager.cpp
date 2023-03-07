/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   ClientManager.cpp
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-03-31 15:06
 * Description: 
 * Others:
 *************************************************/

#include "Logger/Define.h"
#include "ClientInternal.h"
#include "Infra/Thread/Thread.h"
#include "Client/ClientManager.h"
#include "Infra/Concurrence/Guard.h"

namespace ArcFace {
namespace Application {


ClientManager* ClientManager::instance() {
    static ClientManager clientManager;
    return &clientManager;
}

ClientManager::ClientManager() {
    mInternal = new ClientManagerInternal();
}

bool ClientManager::registerClient(IClient *client) {
    Infra::CGuard guard(mInternal->mMutex);
    ClientManagerInternal::IClientIterator iter = mInternal->mClientMap.find(client->getClientId());
    if (iter != mInternal->mClientMap.end()) {
        return false;
    }

    mInternal->mClientMap.insert(ClientManagerInternal::IClientMap::value_type(client->getClientId(),client));
    return true;
}

bool ClientManager::unregisterClient(const ArcFace::Application::IClient *client) {
    Infra::CGuard guard(mInternal->mMutex);
    ClientManagerInternal::IClientIterator iter = mInternal->mClientMap.find(client->getClientId());
    if (iter == mInternal->mClientMap.end()) {
        return false;
    }

    mInternal->mClientMap.erase(iter);
    return true;
}

bool ClientManager::hasClient(const char *clientId) {
    Infra::CGuard guard(mInternal->mMutex);
    ClientManagerInternal::IClientIterator iter = mInternal->mClientMap.find(clientId);
    return iter != mInternal->mClientMap.end();
}

bool ClientManager::start() {
    Infra::CGuard guard(mInternal->mMutex);
    for (ClientManagerInternal::IClientIterator iter = mInternal->mClientMap.begin();
                iter != mInternal->mClientMap.end() ; iter++) {

        infof("initial module {} >>>>>>>>>>>>>>>>>>>>",iter->second->getClientId());
        iter->second->initial();

        infof("start module {} >>>>>>>>>>>>>>>>>>>>",iter->second->getClientId());
        iter->second->start();
    }

    mInternal->mStarted = true;
    return true;
}

bool ClientManager::stop() {
    Infra::CGuard guard(mInternal->mMutex);
    for (ClientManagerInternal::IClientIterator iter = mInternal->mClientMap.begin();
         iter != mInternal->mClientMap.end() ; iter++) {

        infof("stop module {} >>>>>>>>>>>>>>>>>>>>",iter->second->getClientId());
        iter->second->stop();
    }

    mInternal->mStopped = true;
    return true;
}

bool ClientManager::started() {
    return mInternal->mStarted;
}

bool ClientManager::wait() {

    Infra::CThread::setCurrentThreadName("application");
    while (!mInternal->isStopped()) {
        mInternal->mSemaphore.pend();
    }

    return true;
}

}
}
