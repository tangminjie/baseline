
#ifndef INC_5C_APP_CONFIGURE_CONFIGMANAGERIMPL_H
#define INC_5C_APP_CONFIGURE_CONFIGMANAGERIMPL_H

#include "MediaFrame/IMediaFrame.h"
#include "Infra/Concurrence/Mutex.h"

namespace ArcFace {
namespace Application {

/**
 * @brief 视频流实现类
 */
class CMediaFrameImpl:public IMediaFrame {

public:
    /**
     * @brief 获取视频流实例
     * @return
     */
    static CMediaFrameImpl* getInstance();

public:
    /**
    * @brief 读取一帧视频流文件
    */
    virtual bool readMediaFrame(FrameData &frameData);
    /**
    * 读取视频分辨率
    */
    virtual bool getFrameResolution(int &width,int &height);
    /**
     * 读取视频流fps
    */
    virtual bool getFrameFps(double &fps);

    /**
     * 播放视频流
    */
    virtual bool playMediaFrame();

public:
    /**
     * @brief 构造函数
     */
    CMediaFrameImpl();
    /**
     * @brief 析构函数
     */
    virtual ~CMediaFrameImpl();
    /**
     * @brief 初始化配置
     * @return
     */
    virtual bool initial();

private:
    /**
     * @brief 打开视频流
    */
    bool openMediaFrame(std::string);

    /**
     * 关闭视频流
    */
    bool closeMediaFrame(int fd);
    /**
     * 颜色空间转换
    */
   int ColorSpaceConversion(MInt32 width, MInt32 height, MInt32 format, MUInt8* imgData, ASVLOFFSCREEN& offscreen);
private:
    std::string videoPath;
    cv::VideoCapture cap;
    int frame_count;

};

}
}
#endif //INC_5C_APP_CONFIGURE_CONFIGMANAGERIMPL_H
