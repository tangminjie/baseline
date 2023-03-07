
#include "AlgArcSoft/FaceRecognitionProcess.h"

namespace ArcFace {
namespace Application {


CFaceRecognitionProcess* CFaceRecognitionProcess::instance() {
    static CFaceRecognitionProcess faceRecognitionProcess;
    return &faceRecognitionProcess;
}

CFaceRecognitionProcess::CFaceRecognitionProcess() {
}

CFaceRecognitionProcess::~CFaceRecognitionProcess() {
    readFrameThread.join();
    AlgProcessThread.join();
}

bool CFaceRecognitionProcess::start(){
  //启动码流读取线程  
  readFrameThread = std::thread(&CFaceRecognitionProcess::readFrameThreadFunction, this);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  AlgProcessThread = std::thread(&CFaceRecognitionProcess::AlgProcessThreadFunction,this);
  //rectangleFrameThread = std::thread(&CFaceRecognitionProcess::rectangleFrameThreadFunction,this);
}

void CFaceRecognitionProcess::readFrameThreadFunction(){

    while(true){
        FrameData frameData={0};
        bool res = IMediaFrame::instance()->readMediaFrame(frameData);
        // infof("frame count: {} ,frame height:{}, width: {}, fps;{}\n",\
        // frameData.frameID,frameData.height,frameData.width,frameData.fps);
        frameDataMutex.lock();
        //放入map储存
        frameDataList.push_back(frameData);
        frameDataMutex.unlock();
        // 等待一段时间再读取
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void CFaceRecognitionProcess::AlgProcessThreadFunction(){
    //初始化算法模块
    bool res = IAlgArcSoft::instance()->initial();
    if(!res){
        errorf("IAlgArcSoft initial error!\n");
        return;
    }

    while(true){
        if(frameDataList.size() <= 0){	
            //printf("##frameDataList size 0\n");
            // 等待一段时间再读取
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }else{
            //测试算法模块时间
            auto start_time = std::chrono::high_resolution_clock::now();
            infof("frameDataList num:{}\n",frameDataList.size());
            infof("alg times start\n");
            frameDataMutex.lock();
            FrameData frameData = frameDataList.front();
            frameDataList.erase(frameDataList.begin()); 
            frameDataMutex.unlock();
            // test image
            // char* picPath1 = "../test/images/640x480_1.NV21";
            // int Width1 = 640;
            // int Height1 = 480;
            // int Format1 = ASVL_PAF_NV21;
            // MUInt8* imageData1 = (MUInt8*)malloc(Height1*Width1*3/2);
            // FILE* fp1 = fopen(picPath1, "rb");
            // fread(imageData1, 1, Height1*Width1*3/2, fp1);	//读NV21裸数据
		    // fclose(fp1);
            DetectSingleFrameFaceData detectSingleFrameFaceData = {0};
            bool res = IAlgArcSoft::instance()->getDetectSingleFrameFaceData(frameData.offscreen, detectSingleFrameFaceData);
        
            std::lock_guard<std::mutex> lock(frameTotalPacketsMutex);
            FrameTotalPackets frameTotalPackets = {0};
            frameTotalPackets.frameData = frameData;
            frameTotalPackets.detectSingleFrameFaceData = detectSingleFrameFaceData;
            //放入map储存
            frameTotalPacketsList.push_back(frameTotalPackets);
            // 等待一段时间再读取
            //std::this_thread::sleep_for(std::chrono::seconds(1));
            //打印一下算法耗时
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            std::cout << "Thread execution time: " << duration.count() << "ms" << std::endl;
    }
    }

}

void CFaceRecognitionProcess::rectangleFrameThreadFunction(){
    if(frameTotalPacketsList.size() <= 0){
        // 暂停线程100毫秒
        //printf("1111111111111111111111111");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return;
	}else{
        infof("@@@detectSingleFrameFaceDataList size is %d\n",frameTotalPacketsList.size());
        frameTotalPacketsMutex.lock();
        // 在图像上画一个红色正方形
        FrameTotalPackets frameTotalPackets = frameTotalPacketsList.front();
        //显示完后将map删除
        frameTotalPacketsList.erase(frameTotalPacketsList.begin());
        frameTotalPacketsMutex.unlock();

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

    }
}

 bool CFaceRecognitionProcess::getFrameTotalPackets(FrameTotalPackets &frameTotalPackets){
    if(frameTotalPacketsList.size() <= 0){
        // 暂停线程100毫秒
        //printf("1111111111111111111111111");
      //  std::this_thread::sleep_for(std::chrono::seconds(1));
        return false;
	}else{
        infof("@@@detectSingleFrameFaceDataList size is %d\n",frameTotalPacketsList.size());
        frameTotalPacketsMutex.lock();
        // 在图像上画一个红色正方形
        frameTotalPackets = frameTotalPacketsList.front();
        //显示完后将map删除
        frameTotalPacketsList.erase(frameTotalPacketsList.begin());
        frameTotalPacketsMutex.unlock();
    }
    return true;
}

}
}
