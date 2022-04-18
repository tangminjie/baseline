

#ifndef FRAMEWORK_INCLUDE_INFRA_THREADSPECIFIC_H
#define FRAMEWORK_INCLUDE_INFRA_THREADSPECIFIC_H

#include "Infra/Define.h"

namespace Uface {
namespace Infra {

/**
 * @brief  线程本地存储
 */
class CThreadSpecific {

public:
    /**
     * @brief 构造函数,分配本地存储索引
     */
    CThreadSpecific();
    /**
     * @brief 析构函数,释放本地存储索引
     */
    ~CThreadSpecific();
    /**
     * @brief 判断本地存储索引是否可用
     * @return
     */
    bool valid();
    /**
     * @brief 设置存储值
     * @param[in] value
     * @return
     */
    bool setValue(const void* value);
    /**
     * @brief 获取存储值
     * @param[out] value
     * @return
     */
    bool getValue(void** value);

private:
    struct CThreadSpecificInternal;
    CThreadSpecificInternal*    mInternal;
};

}
}
#endif //FRAMEWORK_INCLUDE_INFRA_THREADSPECIFIC_H
