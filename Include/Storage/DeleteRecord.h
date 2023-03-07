/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   DeleteRecord.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-06 16:51
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_INCLUDE_STORAGE_DELETERECORD_H
#define APPLICATION_INCLUDE_STORAGE_DELETERECORD_H

#include "Storage/ITable.h"
#include "Storage/Define.h"

namespace ArcFace {
namespace Application {

class TableInternal;
class DeleteRecord: public ITable {

public:
    /**
     * @brief 单例
     * @return
     */
    static DeleteRecord* instance();
public:
    /**
     * @brief 构造函数
     */
    DeleteRecord();
    /**
     * @brief 析构函数
     */
    virtual ~DeleteRecord();
    /**
     * @brief 初始化
     * @return
     */
    virtual bool initial();
    /**
     * @brief 创建记录
     * @param[in] id        id信息
     * @param[in] info      信息,可以为null
     * @return
     */
    virtual int32_t createRecord(const std::string& id,const Json::Value& info);
    /**
     * @brief 更新记录
     * @param[in] id        id信息
     * @param[in] info      信息,非null
     * @return
     */
    virtual int32_t updateRecord(const std::string& id,const Json::Value& info);
    /**
     * @brief 删除记录
     * @param[in] id        id信息
     * @return
     */
    virtual int32_t deleteRecord(const std::string& personId);
    /**
     * @brief 删除表所有记录
     * @return
     */
    virtual int32_t deleteAllRecord();
    /**
     * @brief 查看记录是否存在
     * @param[in] id        id信息
     * @return
     */
    virtual int32_t isExist(const std::string &id);
    /**
     * @brief 查看满足条件的数量
     * @param[in] condition 检索条件
     * @return
     */
    virtual int32_t count(const Json::Value& condition);
    /**
     * @brief 查看
     * @param[in]     condition 检索条件
     * @param[in,out] infos     信息
     * @return
     */
    virtual int32_t query(const Json::Value& condition, Json::Value &infos);
    /**
     * @brief 查看
     * @param[in]     condition 检索条件
     * @param[in,out] deleteInfo删除信息
     * @return
     */
    virtual int32_t query(const Json::Value& condition,DeleteRecordInfoVec& deleteInfos);
private:
    /**
     * @brief 创建表索引
     * @return
     */
    bool createTableIndex();
    /**
     * @brief 是否存在记录
     */
    int32_t isRecordExist(const std::string &recordId);
    /**
     * @brief 创建检索条件
     * @param condition
     * @return
     */
    std::string createQueryCondition(const Json::Value& condition,bool onlyCount);

private:
    TableInternal*   mInternal;
};
}
}
#endif //APPLICATION_INCLUDE_STORAGE_DELETERECORD_H
