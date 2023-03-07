#include "MediaFrame/MediaFrameImpl.h"

namespace ArcFace {
namespace Application {

IMediaFrame* IMediaFrame::instance() {
    return CMediaFrameImpl::getInstance();
}

CMediaFrameImpl* CMediaFrameImpl::getInstance() {
    static CMediaFrameImpl mediaFrame;
    return &mediaFrame;
}

CMediaFrameImpl::CMediaFrameImpl():cap(VIDEOPATH){
  frame_count = 0;
}

CMediaFrameImpl::~CMediaFrameImpl() {
	cap.release();
}

/**
 * @brief 初始化配置
 * @return
 */
bool CMediaFrameImpl::initial(){

    // 检查是否成功打开了视频文件
    if (!cap.isOpened()) {
        errorf("Failed to open video file\n");
        return false;
    }
    return false;
}

/**
* @brief 读取一帧视频流文件
*/
bool CMediaFrameImpl::readMediaFrame(FrameData &frameData){
    if(!cap.isOpened()){
        errorf("video not open,cap is NULL!\n");
        return false;
    }

    // 从视频文件中读取一帧
    cv::Mat frame,yuv_frame;
    cap >> frame;

    // 检查是否成功读取视频帧
    if (frame.empty())
    {
        errorf("cannot read frame!\n");
        return false;
    }

    // 转换为yuv格式
    //cvtColor(frame, yuv_frame, COLOR_BGR2YUV);
    // Convert the frame to NV21 format
    cv::cvtColor(frame, yuv_frame, cv::COLOR_BGR2YUV_I420);
    // cv::cvtColor(yuv_frame, frame_nv21, cv::COLOR_YUV2RGB_NV21);	
    frame_count++;
    int width = 0;
    int height = 0;
    double fps = 0;
    getFrameResolution(width,height);
    getFrameFps(fps);
    //色彩阈转换
    ASVLOFFSCREEN offscreen = { 0 };
    ColorSpaceConversion(width, height, ASVL_PAF_NV21, yuv_frame.data, offscreen);
    //构造数据体
    frameData.frameID =  frame_count;
    frameData.height = height;
    frameData.width =  width;
    frameData.fps = fps;
    frameData.frame_nv21 = yuv_frame;
    frameData.frame_rgb = frame;
    frameData.offscreen = offscreen;

    return false;
}

/**
* 读取视频分辨率
*/
bool CMediaFrameImpl::getFrameResolution(int &width,int &height){
    width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    return true;
}

/**
 * 读取视频流fps
*/
bool CMediaFrameImpl::getFrameFps(double &fps){
    fps = cap.get(cv::CAP_PROP_FPS);
    return true;
}

/**
 * 播放视频流
*/
bool CMediaFrameImpl::playMediaFrame(){

}

//图像颜色格式转换
int CMediaFrameImpl::ColorSpaceConversion(MInt32 width, MInt32 height, MInt32 format, MUInt8* imgData, ASVLOFFSCREEN& offscreen)
{
	offscreen.u32PixelArrayFormat = (unsigned int)format;
	offscreen.i32Width = width;
	offscreen.i32Height = height;
	
	switch (offscreen.u32PixelArrayFormat)
	{
	case ASVL_PAF_RGB24_B8G8R8:
		offscreen.pi32Pitch[0] = offscreen.i32Width * 3;
		offscreen.ppu8Plane[0] = imgData;
		break;
	case ASVL_PAF_I420:
		offscreen.pi32Pitch[0] = width;
		offscreen.pi32Pitch[1] = width >> 1;
		offscreen.pi32Pitch[2] = width >> 1;
		offscreen.ppu8Plane[0] = imgData;
		offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width;
		offscreen.ppu8Plane[2] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width * 5 / 4;
		break;
	case ASVL_PAF_NV12:
	case ASVL_PAF_NV21:
		offscreen.pi32Pitch[0] = offscreen.i32Width;
		offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0];
		offscreen.ppu8Plane[0] = imgData;
		offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.pi32Pitch[0] * offscreen.i32Height;
		break;
	case ASVL_PAF_YUYV:
	case ASVL_PAF_DEPTH_U16:
		offscreen.pi32Pitch[0] = offscreen.i32Width * 2;
		offscreen.ppu8Plane[0] = imgData;
		break;
	case ASVL_PAF_GRAY:
		offscreen.pi32Pitch[0] = offscreen.i32Width;
		offscreen.ppu8Plane[0] = imgData;
		break;
	default:
		return 0;
	}
	return 1;
}

}
}