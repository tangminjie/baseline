/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   DBManagerImpl.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-13 16:04
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_SRC_STORAGE_DBMANAGERIMPL_H
#define APPLICATION_SRC_STORAGE_DBMANAGERIMPL_H

#include "Storage/DBManager.h"

namespace Uface {
namespace Application {

class DBManagerImpl: public IDBManager {

public:
    /**
     * @brief 构造函数
     * @param[in] db db库名字
     */
    DBManagerImpl(const std::string& db);
    /**
     * @brief 析构函数
     */
    virtual ~DBManagerImpl();
    /**
     * @brief 初始化DB
     * @return
     */
    virtual bool initial();
    /**
     * @brief 事务开始
     * @return
     */
    virtual bool begin();
    /**
     * @brief 事务提交
     * @return
     */
    virtual bool commit();
    /**
     * @brief 获取db handle
     * @return
     */
    virtual DBHandle* getDbHandle();
    /**
     * @brief 释放db handle
     * @param[in] dbHandle
     */
    virtual void releaseHandle(DBHandle* dbHandle);

private:
    class DBInternal;
    DBInternal*     mInternal;

};

}
}
#endif //APPLICATION_SRC_STORAGE_DBMANAGERIMPL_H
