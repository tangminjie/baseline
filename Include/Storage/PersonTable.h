/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   PersonTable.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-06 15:53
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_INCLUDE_STORAGE_PERSONTABLE_H
#define APPLICATION_INCLUDE_STORAGE_PERSONTABLE_H

#include <vector>
#include <string>
#include "Storage/ITable.h"
#include "Storage/Define.h"
#include "easySqllite/SqlRecord.h"

namespace ArcFace {
namespace Application {

class TableInternal;
class PersonTable: public ITable {

public:
    /**
     * @brief 单例
     * @return
     */
    static PersonTable* instance();
public:
    /**
     * @brief 构造函数
     */
    PersonTable();
    /**
     * @brief 析构函数
     */
    virtual ~PersonTable();
    /**
     * @brief 初始化
     * @return
     */
    virtual bool initial();
    /**
     * @brief 创建人员记录
     * @param[in] personId  人员id
     * @param[in] info      人员信息,可以为null
     * @return
     */
    virtual int32_t createRecord(const std::string& personId,const Json::Value& info);
    /**
     * @brief 更新人员记录
     * @param[in] personId  人员Id
     * @param[in] info      人员信息,非null
     * @return
     */
    virtual int32_t updateRecord(const std::string& personId,const Json::Value& info);
    /**
     * @brief 更具云端Id 更新记录
     * @param remoteId
     * @param info
     * @return
     */
    virtual int32_t updateRecordByRemoteId(const std::string& remoteId, const Json::Value& info);
    /**
     * @brief 更新人员记录
     * @param[in] personId  人员Id
     * @param[in] info      人员信息,非null
     * @return
     */
    virtual int32_t updateAllRecord(const Json::Value& info);
    /**
     * @brief 删除人员记录
     * @param[in] personId  人员Id
     * @return
     */
    virtual int32_t deleteRecord(const std::string& personId);
    /**
     * @brief 删除人员
     * @param condition
     * @return
     */
    virtual int32_t deleteRecord(const Json::Value& condition);
    /**
     * @brief 通过relation Id 删除人员
     * @param type
     * @param relationId
     * @return
     */
    virtual int32_t deleteByRelationId(int32_t type,const std::string& relationId);
    /**
     * @brief 删除表所有记录
     * @return
     */
    virtual int32_t deleteAllRecord();
    /**
     * @brief 查看人员是否存在
     * @param[in] personId  人员Id
     * @return
     */
    virtual int32_t isExist(const std::string& personId);
    /**
     * @brief 查看人员是否存在
     * @param condition
     * @return
     */
    virtual int32_t isExist(const Json::Value& condition);
    /**
     * @brief 查看满足条件的数量
     * @param[in] condition 检索条件
     * @return
     */
    virtual int32_t count(const Json::Value& condition);
    /**
     * @brief 查看
     * @param[in]     condition 检索条件
     * @param[in,out] infos     人员信息
     * @return
     */
    virtual int32_t query(const Json::Value& condition,PersonInfoVec& personInfo);
    /**
     * @brief 查找文件
     * @param condition
     * @param personInfo
     * @return
     */
    virtual int32_t query(const Json::Value& condition,Json::Value& personInfo);
    /**
     * @brief 获取最大位点
     * @param[out] position
     * @param[in]  localPosition
     * @return
     */
    virtual int32_t getMaxPosition(bool localPosition, Json::Value &position);
private:
    /**
     * @brief 记录是否存在
     * @param recordId
     * @return
     */
    int32_t recordExist(const std::string& recordId);
    /**
     * @brief 记录是否存在
     * @param condition
     * @return
     */
    int32_t recordExist(const Json::Value& condition);
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
     * @brief 创建更新条件
     * @param condition
     * @return
     */
    std::string createUpdateCondition(const Json::Value& condition);
    /**
     * @brief 设置record值
     * @param record
     * @param info
     * @return
     */
    bool setRecordValue(sql::Record *record, const Json::Value &info, bool setDefault);
    /**
     * @brief
     * @param[in] record
     * @param[in] element
     */
    void exchangeStr2Json(sql::Record* record,Json::Value& element);
    /**
     * @brief 转换为结构体
     * @param record
     * @param personInfo
     */
    void exchangeStr2Struct(sql::Record *record, PersonInfo &personInfo);

private:
    TableInternal*        mInternal;
};

}
}
#endif //APPLICATION_INCLUDE_STORAGE_PERSONTABLE_H
