
#ifndef FRAMEWORK_INCLUDE_INFRA_ERROR_H
#define FRAMEWORK_INCLUDE_INFRA_ERROR_H

#include "Infra/Define.h"

namespace Uface {
namespace Infra {

/**
 * @brief 设置错误号
 * @param[in] error     错误号
 */
void setLastErrno(int32_t error);
/**
 * @brief 获取错误号
 * @return
 */
int32_t getLastErrno();
/**
 * @brief 设置错误信息
 * @param[in] error
 * @param message
 */
void setErrorMsg(int32_t error, const char* message);
/**
 * @brief 获取错误信息
 * @param[in] error
 * @return
 */
const char* getErrorMsg(int32_t error);

}
}

#endif //FRAMEWORK_INCLUDE_INFRA_ERROR_H
