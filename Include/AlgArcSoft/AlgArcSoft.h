#ifndef INC_5C_APP_CONFIGURE_CONFIGMANAGERIMPL_H
#define INC_5C_APP_CONFIGURE_CONFIGMANAGERIMPL_H

#include <vector>
#include <iostream> 
#include <stdlib.h>
#include "AlgArcSoft/IAlgArcSoft.h"
#include "Infra/Concurrence/Mutex.h"

namespace ArcFace {
namespace Application {

/**
 * @brief 视频流实现类
 */
class CAlgArcSoftImpl:public IAlgArcSoft {

public:
    /**
     * @brief 获取视频流实例
     * @return
     */
    static CAlgArcSoftImpl* getInstance();

public:
    /**
     * @brief 构造函数
     */
    CAlgArcSoftImpl();
    /**
     * @brief 析构函数
     */
    virtual ~CAlgArcSoftImpl();
    /**
     * @brief 初始化配置
     * @return
     */
    virtual bool initial();
    /**
     * 获取一帧中的人脸数据
    */
    virtual bool getDetectSingleFrameFaceData(ASVLOFFSCREEN &offscreen,DetectSingleFrameFaceData &detectSingleFrameFaceData);
    /**
     * @brief 反初始化
    */
    virtual bool Revinitial();


// public:
//    /**
//      * @brief 获取激活文件信息
//      * @return
//      */
//     virtual bool getActiveFileInfo(LPASF_ActiveFileInfo &activeFileInfo);
//     /**
//      * @brief 设置在线激活sdk
//      * @return
//      */
//     virtual bool setOnlineActivation(char AppId,char SDKKey);
//     /***
//      * @brief  设置算法模型参数
//      * @return
//      */
//     virtual bool setAlgEngineInfo();
//     /**
//      * @brief 初始化算法模型
//      * @return
//      */
//     virtual bool algInitEngine();
//     /**
//      * @brief 获取跟踪到的人脸
//      * @return 
//      */
//     virtual bool getDetectFaces();
//     /**
//      * @brief 人脸特征提取
//      * @return
//      */
//     virtual bool getFaceFeatureExtract();
//     /**
//      * @brief 人脸特征比对
//      * @return 
//     */
//     virtual bool getFaceFeatureCompare();
//     /**
//      * @brief 设置RGB/IR活体阈值
//      * @return
//     */
//     virtual bool setLivenessParam();
//     /**
//      * @brief   人脸属性检测 包含年龄性别 3DAngle
//      *
//      */
//     virtual bool getFaceGender();

private:
    void timestampToTime(char* timeStamp, char* dateTime, int dateTimeSize);
    int detectDataProcess(ASVLOFFSCREEN &offscreen,
    ASF_MultiFaceInfo &detectedFaces,std::vector<DetectSingleFaceData> &DetectSingleFaceDataVector
    );

private:
    MHandle handle;
};

}
}
#endif //INC_5C_APP_CONFIGURE_CONFIGMANAGERIMPL_H
