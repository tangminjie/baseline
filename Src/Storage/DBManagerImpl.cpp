/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   DBManagerImpl.cpp
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-13 16:07
 * Description: 
 * Others:
 *************************************************/

#include <map>
#include "DBManagerImpl.h"
#include "Logger/Define.h"
#include "Storage/DBHandle.h"
#include "easySqllite/SqlDatabase.h"
#include "Infra/Concurrence/Guard.h"
#include "Infra/Concurrence/Mutex.h"

namespace ArcFace {
namespace Application {

typedef std::map<std::string,IDBManager*> SQLLiteMap;

static Infra::CMutex sMutex;
static SQLLiteMap    sDBSet;

class DBManagerImpl::DBInternal {

    friend class DBManagerImpl;

public:

    DBInternal(const std::string& dbName):mDbName(dbName) {
    }

private:
    std::string     mDbName;
    sql::Database   mMainDb;
};

IDBManager* IDBManager::instance(const std::string &db) {
    Infra::CGuard guard(sMutex);
    IDBManager* sqlLiteManager = nullptr;
    SQLLiteMap::iterator iter = sDBSet.find(db);
    if (iter != sDBSet.end()) {
        sqlLiteManager = iter->second;
    } else {
        sqlLiteManager = new DBManagerImpl(db);
        sqlLiteManager->initial();
        sDBSet.insert(SQLLiteMap::value_type(db,sqlLiteManager));
    }

    return sqlLiteManager;
}

DBManagerImpl::DBManagerImpl(const std::string& db){
    mInternal = new DBInternal(db);
}

DBManagerImpl::~DBManagerImpl() {
    mInternal->mMainDb.close();
    delete mInternal;
}

bool DBManagerImpl::initial() {

    try {
        if(!mInternal->mMainDb.isOpen()) {
            mInternal->mMainDb.open(mInternal->mDbName);
        }

        return true;
    } catch(sql::Exception e) {
        errorf("initial db(%s) exception,message:%s",mInternal->mDbName.c_str(),e.msg().c_str());
        return false;
    }
}

/**
* @brief 事务开始
* @return
*/
bool DBManagerImpl::begin() {

    try {
        mInternal->mMainDb.transactionBegin();
        return true;
    } catch (sql::Exception e) {
        errorf("transaction begin exception,message:%s",e.msg().c_str());
    }

    return false;
}
/**
* @brief 事务提交
* @return
*/
bool DBManagerImpl::commit() {

    try {
        mInternal->mMainDb.transactionCommit();
        return true;
    } catch (sql::Exception e) {
        errorf("transaction commit exception,message:%s",e.msg().c_str());
    }

    return false;
}

DBHandle* DBManagerImpl::getDbHandle() {

    if (mInternal->mMainDb.isOpen()) {
        sqlite3* sqlite = mInternal->mMainDb.getHandle();
        return new DBHandle(sqlite);
    }

    return nullptr;
}

void DBManagerImpl::releaseHandle(ArcFace::Application::DBHandle *dbHandle) {
    delete dbHandle;
}

}
}