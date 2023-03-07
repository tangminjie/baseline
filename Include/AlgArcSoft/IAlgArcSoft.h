#ifndef APPLICATION_INCLUDE_ALGSRCSOFT_H
#define APPLICATION_INCLUDE_ALGSRCSOFT_H

#include "AlgArcSoft/FaceStructures.h"

namespace Uface {
namespace Application {

/**
 * @brief 算法类接口
 */
class IAlgArcSoft{

public:
    /**
     * @brief 单例
     * @return
     */
    static IAlgArcSoft* instance();

public:
    /**
     * @brief 析构函数
     */
    virtual ~IAlgArcSoft(){};
    /**
     * @brief 初始化
     * @return
     */
    virtual bool initial() = 0;
    /**
     * 获取一帧中的人脸数据
    */
    virtual bool getDetectSingleFrameFaceData(ASVLOFFSCREEN &offscreen,DetectSingleFrameFaceData &detectSingleFrameFaceData) = 0;
    /**
     * 
    */
};

}
}
#endif //APPLICATION_INCLUDE_ALGSRCSOFT_H
