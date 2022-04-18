/************************************************
 * Copyright(c) 2019 uni-ubi
 * 
 * Project:  Application
 * FileName: System.h
 * Author:   tangminjie
 * Email:    tangminjie@163.com
 * Version:  V1.0.0
 * Date:     2021-01-02 19:47
 * Description: 
 * Others:
 *************************************************/


#ifndef FRAMEWORK_INCLUDE_INFRA_SYSTEM_H
#define FRAMEWORK_INCLUDE_INFRA_SYSTEM_H

#include "Infra/Define.h"

namespace Uface {
namespace Infra {
/**
 * @brief 系统调用操作命令
 * @param command
 * @return
 */
int32_t systemCall(const char* command);

}
}

#endif //FRAMEWORK_INCLUDE_INFRA_SYSTEM_H
