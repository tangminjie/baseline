/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   FeatureTable.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-06 16:20
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_INCLUDE_STORAGE_FEATURETABLE_H
#define APPLICATION_INCLUDE_STORAGE_FEATURETABLE_H

#include <string>
#include <vector>
#include "Storage/ITable.h"
#include "Storage/Define.h"
#include "easySqllite/SqlRecord.h"

namespace ArcFace {
namespace Application {

class TableInternal;
class FeatureTable: public ITable {

public:
    /**
     * @brief 单例
     * @return
     */
    static FeatureTable* instance();

public:
    /**
     * @brief 构造函数
     */
    FeatureTable();
    /**
     * @brief 析构函数
     */
    virtual ~FeatureTable();
    /**
     * @brief 初始化
     * @return
     */
    virtual bool initial();
    /**
     * @brief 创建记录
     * @param[in] id        id信息
     * @param[in] info      人员信息,可以为null
     * @return
     */
    virtual int32_t createRecord(const std::string& id,const Json::Value& info);
    /**
     * @brief 创建记录
     * @param[in] info      特征信息,
     * @return
     */
    virtual int32_t createRecord(const FeatureInfo& info);
    /**
     * @brief 更新记录
     * @param[in] id        id信息
     * @param[in] info      人员信息,非null
     * @return
     */
    virtual int32_t updateRecord(const std::string& id,const Json::Value& info);
    /**
     * @brief 更新记录
     * @param[in] id        id信息
     * @param[in] info      人员信息,非null
     * @return
     */
    virtual int32_t updateRecord(const FeatureInfo& info);
    /**
     * @brief 删除记录
     * @param[in] id        id信息
     * @return
     */
    virtual int32_t deleteRecord(const std::string& featureId);
    /**
     * @brief 删除表所有记录
     * @return
     */
    virtual int32_t deleteAllRecord();
    /**
     * @brief 查看人员是否存在
     * @param[in] id        id信息
     * @return
     */
    virtual int32_t isExist(const std::string& id);
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
    virtual int32_t query(const Json::Value& condition,FeatureInfoVec& featureVec);
    /**
     * @brief 是否存在与当前算法版本不匹配的特征
     * @param feature
     * @return
     */
    virtual int32_t versionMatch(const std::string &version, bool match);
    /**
     * @brief 
     * @param page
     * @param featureVec
     * @return
     */
    virtual int32_t queryRecordByPage(int32_t page, int32_t pageSize, FeatureInfoVec &featureVec);
private:
    /**
     * @brief 记录是否存在
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
    void exchangeStr2Json(sql::Record* record,Json::Value& element);

    void exchangeStr2Struct(sql::Record *record, FeatureInfo &featureInfo);
private:
    TableInternal*   mInternal;
};

}
}
#endif //APPLICATION_INCLUDE_STORAGE_FEATURETABLE_H
