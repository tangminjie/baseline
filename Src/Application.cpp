/************************************************
 * Copyright(c) 2019 Sang Yang
 * 
 * Project:  applicate
 * FileName: Applicate.cpp
 * Author: xieshren
 * Email: xieshren@gmail.com
 * Version: V1.0.0
 * Date: 2021-03-29 02:22
 * Description: 
 * Others:
 *************************************************/

#include "Logger/Define.h"
#include "HttpClient/HttpClient.h"
#include "System/StorageManager.h"
#include "Client/ClientManager.h"
#include "Client/AppClient.h"

int32_t main(int32_t argc,char** argv) {
using namespace Uface::Application;

/**模块初始化、加载逻辑*/
ILogger::instance()->initial("application",4098);
infof("client manager initial>>>>>>>>>>>>>>>>>");
ClientManager::instance()->start();
infof("wait app stop and exit process>>>>>>>>>>>>>>>>>");
ClientManager::instance()->wait();
infof("app client initial>>>>>>>>>>>>>>>>>>>>>>>>>");
AppClient::instance()->initial();

//////////////////////http库测试///////////////////////////////
// std::string photoUrl = "https://i.loli.net/2021/08/19/e73jt9DKwi1IRqH.jpg";
// std::string photoPath = "/tmp/";
// std::string photoName = "photo1";
// std::string photoDir = StorageManager::instance()->createFileName(photoPath.c_str(),photoName.c_str(),".jpg");
// Uface::Application::HttpClient httpclient;
// int64_t photolen = httpclient.getFileLength(photoUrl);
// tracef("photo length {}",photolen);
// //Uface::Application::OnDownloadCb cb;
// httpclient.downloadFile(photoUrl,photoDir,nullptr);
//tracef("######################## photoUrl {}",photoUrl.c_str());    

}
