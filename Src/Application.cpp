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
#include "AlgArcSoft/IAlgArcSoft.h"
#include "MediaFrame/IMediaFrame.h"
#include "AlgArcSoft/FaceRecognitionProcess.h"

int32_t main(int32_t argc,char** argv) {
using namespace Uface::Application;

/**模块初始化、加载逻辑*/
ILogger::instance()->initial("application",4098);
infof("client manager initial>>>>>>>>>>>>>>>>>");
ClientManager::instance()->start();
// infof("wait app stop and exit process>>>>>>>>>>>>>>>>>");
// ClientManager::instance()->wait();
// infof("app client initial>>>>>>>>>>>>>>>>>>>>>>>>>");
// AppClient::instance()->initial();

//初始化媒体模块
IMediaFrame::instance()->initial();
//启动人脸识别流程
CFaceRecognitionProcess::instance()->start();

//显示人脸识别视频
while(true){
    FrameTotalPackets frameTotalPackets ={0};
    bool res = CFaceRecognitionProcess::instance()->getFrameTotalPackets(frameTotalPackets);
    if(!res){
        continue;
    }
    cv::Mat rgb_image = frameTotalPackets.frameData.frame_rgb;
    int type = rgb_image.type();
    int depth = type & CV_MAT_DEPTH_MASK;
    int channels = 1 + (type >> CV_CN_SHIFT);
    infof("#####type is %d\n",type);

    std::vector<DetectSingleFaceData> DVector = frameTotalPackets.detectSingleFrameFaceData.DetectSingleFaceDataVector;
    for(int i = 0;i<frameTotalPackets.detectSingleFrameFaceData.faceNum;i++){
        int left = DVector[i].singleDetectedFace.faceRect.left;
        int top = DVector[i].singleDetectedFace.faceRect.top;
        int right = DVector[i].singleDetectedFace.faceRect.right;
        int bottom = DVector[i].singleDetectedFace.faceRect.bottom;
        rectangle(rgb_image, Point(left , top), Point(right, bottom), Scalar(0, 0, 255), 2);
        std::string faceIDtext = "faceID: " + std::to_string(DVector[i].faceID);
        std::string ageText = "age: " + std::to_string(DVector[i].age);
        std::string genderText = "gender: " + std::to_string(DVector[i].gender);
        
        // 设置文本参数
        Scalar color = Scalar(0, 0, 255);
        int fontFace = FONT_HERSHEY_SIMPLEX;
        double fontScale = 1.0;
        int thickness = 2;
        int baseline = 0;

        // 在帧中添加文本
        putText(rgb_image, faceIDtext, Point(right, top), fontFace, fontScale, color, thickness);
        Size faceIDtextSize = getTextSize(faceIDtext, fontFace, fontScale, thickness, &baseline);	
        putText(rgb_image, ageText, Point(right, top+faceIDtextSize.height+2), fontFace, fontScale, color, thickness);		
        Size ageTextSize = getTextSize(ageText, fontFace, fontScale, thickness, &baseline);
        putText(rgb_image, genderText, Point(right, top+faceIDtextSize.height+ageTextSize.height+2), fontFace, fontScale, color, thickness);		

    }
    // 显示当前帧
    imshow("Video", rgb_image);
	// 按下ESC键退出循环
    if (waitKey(30) == 27)
    {
        std::cout << "播放结束" << std::endl;
        break;
    }
}


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
