#ifndef APPLICATION_INCLUDE_MEDIAFRAME_H
#define APPLICATION_INCLUDE_MEDIAFRAME_H

#include "AlgArcSoft/FaceStructures.h"

namespace Uface {
namespace Application {

/**
 * @brief 算法类接口
 */
class IMediaFrame {

public:
    /**
     * @brief 单例
     * @return
     */
    static IMediaFrame* instance();

public:
    /**
     * @brief 析构函数
     */
    virtual ~IMediaFrame(){};
    /**
     * @brief 初始化
     * @return
     */
    virtual bool initial() = 0;
    /**
    * @brief 读取一帧视频流文件
    */
    virtual bool readMediaFrame(FrameData &frameData) = 0;
    /**
    * 读取视频分辨率
    */
    virtual bool getFrameResolution(int &width,int &height) = 0;
    /**
     * 读取视频流fps
    */
    virtual bool getFrameFps(double &fps) = 0;
    /**
     * 播放视频流
    */
    virtual bool playMediaFrame() = 0;

};

}
}
#endif //APPLICATION_INCLUDE_MEDIAFRAME_H
