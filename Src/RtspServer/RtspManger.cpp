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

#include <string.h>
#include "rtspServer/RtspManger.h"

namespace Uface {
namespace RtspServer {

RtspManger::RtspManger(/* args */)
{
}

RtspManger::~RtspManger()
{
    
}

//初始化RTSP资源
bool RtspManger::init(){
    
    return true;
}
//启动RTSP服务
bool RtspManger::start(){
    
    return true;
}
 //停止RTSP服务
bool RtspManger::stop(){
   
    return true;
}


}
}