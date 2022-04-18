/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   RegisterPicTable.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-06 16:43
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_INCLUDE_STORAGE_REGISTERPICTABLE_H
#define APPLICATION_INCLUDE_STORAGE_REGISTERPICTABLE_H

#include "Storage/ITable.h"
#include "Storage/Define.h"

namespace Uface {
namespace Application {

class TableInternal;
class RegisterPicTable: public ITable {

public:
    /**
     * @brief 单例
     * @return
     */
    static RegisterPicTable* instance();
public:
    /**
     * @brief 构造函数
     */
    RegisterPicTable();
    /**
     * @brief 析构函数
     */
    virtual ~RegisterPicTable();
    /**
     * @brief 初始化
     * @return
     */
    virtual bool initial();
    /**
     * @brief 创建记录
     * @param[in] id        id信息
     * @param[in] info      记录信息,可以为null
     * @return
     */
    virtual int32_t createRecord(const std::string& id,const Json::Value& info);
    /**
     * @brief 更新记录
     * @param[in] id        id信息
     * @param[in] info      记录信息,非null
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
     * @brief 通过relation Id 删除人员
     * @param type
     * @param relationId
     * @return
     */
    virtual int32_t deleteByRelationId(int32_t type,const std::string& relationId);
    /**
     * @brief 获取最大位点
     * @param localPosition
     * @param position
     * @return
     */
    int32_t getMaxPosition(bool localPosition, Json::Value &position);
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
     * @param[in,out] infos     注册照信息
     * @return
     */
    virtual int32_t query(const Json::Value& condition, RegisterPicInfoVec& picInfos);
    /**
     * @brief 查询registerid 对应的信息
     * @param registerId
     * @param picInfo
     * @return
     */
    int32_t query(const std::string& registerId,RegisterPicInfo& picInfo);

    /**
     * @brief 分页查找接口
     * @param page 第几页，从1开始
     * @param pageSize 页大小
     * @param picInfos 图片信息
     * @return
     */
    int32_t queryRecordByPage(int32_t page, int32_t pageSize, RegisterPicInfoVec &picInfos);
private:
    /**
     * @brief 记录是否存在
     * @param registerId
     * @return
     */
    int32_t recordExist(const std::string &registerId);
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
     * @brief 根据info设置record参数
     * @param record
     * @param info
     * @return
     */
    bool setRecordValue(sql::Record* record, const Json::Value& info, bool setDefault = false);

    /**
     * @brief
     * @param[in] record
     * @param[in] element
     */
    void exchangeStr2Json(sql::Record* record,Json::Value& element);
    /**
     * @brief 转换为结构体
     * @param record
     * @param picInfo
     */
    void exchangeStr2Struct(sql::Record *record, RegisterPicInfo& picInfo);

private:
    TableInternal*       mInternal;
};
}
}
#endif //APPLICATION_INCLUDE_STORAGE_REGISTERPICTABLE_H
