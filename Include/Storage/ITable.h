/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   ITable.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-06 16:08
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_INCLUDE_STORAGE_ITABLE_H
#define APPLICATION_INCLUDE_STORAGE_ITABLE_H

#include <string>
#include "Infra/Define.h"
#include "jsoncpp/value.h"

namespace Uface {
namespace Application {

class ITable {

public:
    /**
     * @brief 析构函数
     */
    virtual ~ITable() {}
    /**
     * @brief 初始化
     * @return
     */
    virtual bool initial() = 0;
    /**
     * @brief 创建记录
     * @param[in] id        id信息
     * @param[in] info      信息,可以为null
     * @return
     */
    virtual int32_t createRecord(const std::string& id,const Json::Value& info) = 0;
    /**
     * @brief 更新记录
     * @param[in] id        id信息
     * @param[in] info      信息,非null
     * @return
     */
    virtual int32_t updateRecord(const std::string& id,const Json::Value& info) = 0;
    /**
     * @brief 删除记录
     * @param[in] id        id信息
     * @return
     */
    virtual int32_t deleteRecord(const std::string& personId) = 0;
    /**
     * @brief 删除表所有记录
     * @return
     */
    virtual int32_t deleteAllRecord() = 0;
    /**
     * @brief 查看记录是否存在
     * @param[in] id        id信息
     * @return
     */
    virtual int32_t isExist(const std::string& id) = 0;
    /**
     * @brief 查看满足条件的数量
     * @param[in] condition 检索条件
     * @return
     */
    virtual int32_t count(const Json::Value& condition) = 0;
};
}
}

#endif //APPLICATION_INCLUDE_STORAGE_ITABLE_H
