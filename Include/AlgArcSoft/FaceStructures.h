#ifndef FACESTRUCT_H
#define FACESTRUCT_H
#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include "Logger/Define.h"

using namespace cv;

#define VIDEOPATH "../test/testVideo.mp4"

//从开发者中心获取APPID/SDKKEY(以下均为假数据，请替换)
#define APPID "8gibs7tvKpvGGc9W2EfZq5NmxqcmymUwwQ8sLQbP1G5h"
#define SDKKEY "DhHmgvFdKeh7FumTySxkByxy1WYXfto2PdM1NDhSYNCD"

#define NSCALE 16 
#define FACENUM	50

//帧数据
typedef struct{
	int frameID;
	int height;
	int width;
	double fps;
	cv::Mat frame_nv21; //
	cv::Mat frame_rgb;  //原始数据
    ASVLOFFSCREEN offscreen;
} FrameData, *pFrameData;

//单个人脸数据
typedef struct {
	int faceID;
	int age;
	int gender;
	ASF_SingleFaceInfo singleDetectedFace;
	ASF_FaceFeature feature;
} DetectSingleFaceData,*pDetectSingleFaceData;

//一帧数据包含人脸信息
typedef struct{
	int faceNum;
	std::vector<DetectSingleFaceData> DetectSingleFaceDataVector;
}DetectSingleFrameFaceData,*pDetectSingleFrameFaceData;

//一帧数据总的数据包
typedef struct{
	FrameData frameData;
	DetectSingleFrameFaceData detectSingleFrameFaceData;
}FrameTotalPackets,*pFrameTotalPackets;

#endif