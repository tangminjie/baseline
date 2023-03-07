#ifndef APPLICATION_INCLUDE_FACERECOGNATIONPROCESS_H
#define APPLICATION_INCLUDE_FACERECOGNATIONPROCESS_H

#include <mutex>
#include <list>
#include "AlgArcSoft/IAlgArcSoft.h"
#include "MediaFrame/IMediaFrame.h"

namespace Uface {
namespace Application {

/**
 * @brief 人脸识别流程类
 */
class CFaceRecognitionProcess {

public:
    /**
     * @brief 单例
     * @return
     */
    static CFaceRecognitionProcess* instance();

public:
    /**
     * @brief 析构函数
     */
    virtual ~CFaceRecognitionProcess();
    /**
     * @brief 启动识别流程
     * @return
     */
    virtual bool start();

    /**
     * @brief 获取数据
    */
    bool getFrameTotalPackets(FrameTotalPackets &frameTotalPackets);
private:
    CFaceRecognitionProcess();
    /**
     * @brief 读取码流线程
    */
    void readFrameThreadFunction();
    /**
     * @brief 算法处理线程
    */
   void AlgProcessThreadFunction();
   /**
    * @brief
   */
  void rectangleFrameThreadFunction();

private:
    // 创建一个mutex对象
	std::mutex frameDataMutex;
	std::mutex frameTotalPacketsMutex;

	//帧数据从mediaFrame模块获取
	std::list<FrameData> frameDataList;
	//储存一帧码流识别的视频数据
	std::list<FrameTotalPackets> frameTotalPacketsList;

    //两个线程一个读帧数据 一个送给算法处理
    std::thread readFrameThread;
    std::thread AlgProcessThread;
    std::thread rectangleFrameThread;

};

}
}

#endif //APPLICATION_INCLUDE_FACERECOGNATIONPROCESS_H
