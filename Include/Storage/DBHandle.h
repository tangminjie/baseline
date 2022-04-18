/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   CDbHandle.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-13 17:18
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_SRC_STORAGE_DBHANDLE_H
#define APPLICATION_SRC_STORAGE_DBHANDLE_H

#include "easySqllite/sqlite3.h"
#include "easySqllite/SqlTable.h"

namespace Uface {
namespace Application {

/**
 * @brief 构造函数
 */
class DBHandle {

public:
    /**
     * @brief 构造函数
     * @param sqlite
     */
    DBHandle(sqlite3* sqlite): mSqlite(sqlite) {
    }
    /**
     * @brief 获取sqlLite3
     * @return
     */
    sqlite3* getSqlLite() {return mSqlite;}

private:
    sqlite3*        mSqlite;
};

}
}
#endif //APPLICATION_SRC_STORAGE_DBHANDLE_H
