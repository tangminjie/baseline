/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   Client.cpp
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-03-31 12:46
 * Description: 
 * Others:
 *************************************************/

#include <string>
#include "Client/Client.h"
#include "ClientInternal.h"
#include "Client/AppClient.h"
#include "Client/ClientManager.h"


namespace ArcFace {
namespace Application {


IClient::IClient(const char *clientId,const Json::Value& info) : mInternal(nullptr) {
    mInternal = new ClientInternal(clientId,info);
    ClientManager::instance()->registerClient(this);
}

IClient::~IClient() {
    ClientManager::instance()->unregisterClient(this);
    delete mInternal;
}

const char* IClient::getClientId() const {
    return mInternal->mClientId.c_str();
}

Json::Value IClient::getClientInfo() const {
    return mInternal->mInfo;
}

static AppClient appClient;
AppClient* AppClient::instance() {
    return &appClient;
}

AppClient::AppClient(): IClient("innerClient") {
}

AppClient::~AppClient() {
}

}
}