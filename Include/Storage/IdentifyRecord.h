/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   IdentifyRecordTabel.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-06 16:14
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_INCLUDE_STORAGE_IDENTIFYRECORD_H
#define APPLICATION_INCLUDE_STORAGE_IDENTIFYRECORD_H

#include <vector>
#include <string>
#include "Storage/ITable.h"
#include "Storage/Define.h"
#include "easySqllite/SqlRecord.h"

namespace Uface {
namespace Application {

class TableInternal;
class IdentifyRecord {

public:
    /**
     * @brief 单例
     * @return
     */
    static IdentifyRecord* instance();
public:
    /**
     * @brief 构造函数
     */
    IdentifyRecord();
    /**
     * @brief 析构函数
     */
    virtual ~IdentifyRecord();
    /**
     * @brief 初始化
     * @return
     */
    virtual bool initial();
    /**
     * @brief 创建识别记录
     * @param[in] recordId  记录id
     * @param[in] info      人员信息,可以为null
     * @return
     */
    virtual int32_t createRecord(const Json::Value& info);
    /**
     * @brief 更新识别记录
     * @param[in] recordId  记录Id
     * @param[in] info      人员信息,非null
     * @return
     */
    virtual int32_t updateRecord(const std::string& recordId,const Json::Value& info);
    /**
     * @brief 删除识别记录
     * @param[in] recordId  记录Id
     * @return
     */
    virtual int32_t deleteRecord(const std::string& recordId);
    /**
     * @brief 删除表所有记录
     * @return
     */
    virtual int32_t deleteAllRecord();

    /**
     * @brief 删除指定条件的识别记录
     * @param[in] condition  条件
     * @return
     */
    virtual int32_t deleteRecordByCondition(const Json::Value& condition);

    /**
     * @brief 查看记录是否存在
     * @param[in] recordId  记录Id
     * @return
     */
    virtual int32_t isExist(const std::string& recordId);
    /**
     * @brief 查看满足条件的数量
     * @param[in] condition 检索条件
     * @return
     */
    virtual int32_t count(const Json::Value& condition);
    /**
     * @brief 查看
     * @param[in]     condition 检索条件
     * @param[in,out] infos     记录信息
     * @return
     */
    virtual int32_t query(const Json::Value& condition, IdentifyRecordInfoVec &recordInfoVec);

private:
    /**
     * @brief 查看记录是否存在
     * @param recordId
     * @return
     */
    int32_t recordIsExist(const std::string& recordId);
    /**
     * @brief 创建表索引
     * @return
     */
    bool createTableIndex();
    /**
     * @brief 创建检索条件
     * @param condition
     * @return
     */
    std::string createQueryCondition(const Json::Value& condition,bool onlyCount, bool& existCondition);
    /**
     * @brief
     * @param[in] record
     * @param[in] element
     */
    void exchangeStr2Struct(sql::Record *record, IdentifyRecordInfo &recordInfo);

private:
    TableInternal*     mInternal;
};

}
}

#endif //APPLICATION_INCLUDE_STORAGE_IDENTIFYRECORD_H
