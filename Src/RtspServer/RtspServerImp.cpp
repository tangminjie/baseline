/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   DBManagerImpl.cpp
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-13 16:07
 * Description: 
 * Others:
 *************************************************/

#include "rtspServer/RtspServerImp.h"
#include "Logger/Define.h"

namespace ArcFace {
namespace RtspServer {

static RtspServer sRtspServer;
RtspServer* RtspServer::instance() {
    return &sRtspServer;
}

RtspServer::RtspServer():
            Application::IClient("RtspServer",Json::Value::null) {
}

RtspServer::~RtspServer() {
}

void RtspServer::initial() {

    tracef("initial private channel>>>>>>>>>>>>>>>>>>>>>>");

}

void RtspServer::start() {
  
}

void RtspServer::stop() {
  
}

}
}