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


#ifndef APPLICATION_INCLUDE_PRIVATECHANNEL_RTSPSERVER_H
#define APPLICATION_INCLUDE_PRIVATECHANNEL_RTSPSERVER_H

#include <string>
#include <memory>

namespace ArcFace {
namespace RtspServer {

class RtspManger
{
private:
    /* data */
public:
    RtspManger(/* args */);
    ~RtspManger();

    bool init();
    bool start();
    bool stop();
};


}
}
#endif