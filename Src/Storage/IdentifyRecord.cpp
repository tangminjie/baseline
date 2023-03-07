/************************************************
* Copyright(c) 2021
*
* Project:    Application
* FileName:   IdentifyRecordTable.cpp
* Author:     tangminjie
* Email:      tangminjie@163.com
* Version:    V1.0.0
* Date:       2021-04-20 20:12
* Description:
* Others:
*************************************************/


#include "Utils/Utils.h"
#include "Storage/StorageErr.h" 
#include "Logger/Define.h"
#include "Storage/Common.h"
#include "Storage/DBHandle.h"
#include "Storage/DBManager.h"
#include "Storage/TableInternal.h"
#include "Storage/IdentifyRecord.h"
#include "easySqllite/SqlField.h"
#include "easySqllite/SqlCommon.h"


namespace ArcFace {
namespace Application {

static sql::Field fieldIdentifyRecordTable[] = {

    sql::Field(sql::FIELD_KEY),
    sql::Field("personId", sql::type_text),
    sql::Field("name", sql::type_text),
    sql::Field("score", sql::type_int),
    sql::Field("type", sql::type_int),
    sql::Field("mode", sql::type_int),
    sql::Field("result", sql::type_int),
    sql::Field("failType", sql::type_int),
    sql::Field("spotType", sql::type_int),
    sql::Field("spotContent", sql::type_text),
    sql::Field("uploadStatus", sql::type_int),
    sql::Field("elemType", sql::type_int),
    sql::Field("remoteType",sql::type_text),
    sql::Field("remoteId",sql::type_text),
    sql::Field("reserve1", sql::type_text),
    sql::Field("reserve2", sql::type_text),
    sql::Field("reserve3", sql::type_text),
    sql::Field("reserve4", sql::type_text),
    sql::Field("reserve5", sql::type_text),
    sql::Field("reserve6", sql::type_text),
    sql::Field("reserve7", sql::type_text),
    sql::Field("reserve8", sql::type_text),
    sql::Field("reserve9", sql::type_text),
    sql::Field("reserve10", sql::type_text),
    sql::Field("createdAt", sql::type_int),
    sql::Field(sql::DEFINITION_END)
};

IdentifyRecord* IdentifyRecord::instance() {
    static IdentifyRecord identifyRecord;
    return &identifyRecord;
}

IdentifyRecord::IdentifyRecord() {
    mInternal = new TableInternal("identifyRecordTable");
}

IdentifyRecord::~IdentifyRecord() {
    delete mInternal->mTable;
    delete mInternal;
}

bool IdentifyRecord::initial() {

    Infra::CGuard guard(mInternal->mMutex);
    if (mInternal->mInitialed) {
        return true;
    }

    const std::string dbName(IDENTIFY_RECORD_DB);
    IDBManager* manager = IDBManager::instance(dbName);
    DBHandle* handle = manager->getDbHandle();

    try {

        mInternal->mTable = new sql::Table(handle->getSqlLite(),mInternal->mTblName,fieldIdentifyRecordTable);
        if (!mInternal->mTable->exists()) {
            mInternal->mTable->create();
        }

        createTableIndex();
        mInternal->mInitialed = true;
        infof("initial identify record table success");
        return true;
    } catch (sql::Exception e) {
        errorf("initial identity record table failed,message:%s",e.msg().c_str());
    }

    return false;
}

int32_t IdentifyRecord::createRecord(const Json::Value &info) {

    try {

        sql::Record record(mInternal->mTable->fields());
        record.setInteger("createdAt",Infra::CTime::getCurrentUTCtime());

        if (info.isMember("uploadStatus")) {
            record.setInteger("uploadStatus", info["uploadStatus"].asInt());
        } else {
            record.setInteger("uploadStatus", 0);
        }

        if (info.isMember("personId")) {
            record.setString("personId",info["personId"].asString());
        } else {
            record.setString("personId", "");
        }

        if (info.isMember("name")) {
            record.setString("name",info["name"].asString());
        } else {
            record.setString("name", "");
        }

        if (info.isMember("score")) {
            record.setInteger("score",info["score"].asInt());
        } else {
            record.setInteger("score", 0);
        }

        if (info.isMember("type")) {
            record.setInteger("type",info["type"].asInt());
        } else {
            record.setInteger("type", 0);
        }

        if (info.isMember("mode")) {
            record.setInteger("mode",info["mode"].asInt());
        } else {
            record.setInteger("mode",localIdentifyMode);
        }

        if (info.isMember("result")) {
            record.setInteger("result",info["result"].asInt());
        } else {
            record.setInteger("result", 0);
        }

        if (info.isMember("failType")) {
            record.setInteger("failType",info["failType"].asInt());
        } else {
            record.setInteger("failType",identifySuccess);
        }

        if (info.isMember("elemType")) {
            record.setInteger("elemType",info["elemType"].asInt64());
        } else {
            record.setInteger("elemType",0);
        }

        if (info.isMember("remoteId")) {
            record.setString("remoteId",info["remoteId"].asString());
        } else {
            record.setString("remoteId","0");
        }

        record.setInteger("remoteType",0);

        if (info.isMember("spotType")) {
            record.setInteger("spotType",info["spotType"].asInt());
        } else {
            record.setInteger("spotType", 0);
        }

        if (info.isMember("spotContent")) {
            record.setString("spotContent",jsonToString(info["spotContent"]));
        } else {
            record.setString("spotContent", "");
        }

        //预留字段赋空
        record.setString("reserve1", "");
        record.setString("reserve2", "");
        record.setString("reserve3", "");
        record.setString("reserve4", "");
        record.setString("reserve5", "");
        record.setString("reserve6", "");
        record.setString("reserve7", "");
        record.setString("reserve8", "");
        record.setString("reserve9", "");
        record.setString("reserve10", "");

        Infra::CGuard guard(mInternal->mMutex);
        if (!mInternal->mTable->addRecord(&record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("create person register picture record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t IdentifyRecord::deleteRecord(const std::string &recordId) {

    if (recordId.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        Infra::CGuard guard(mInternal->mMutex);
        int32_t iRet = recordIsExist(recordId);
        if (iRet != StorageIdentifyNotExistCode) {
            return iRet;
        }

        const std::string condition = std::string("_ID") + "=" + "\'" + recordId + "\'";
        if (!mInternal->mTable->deleteRecords(condition)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("delete identify record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t IdentifyRecord::deleteAllRecord() {

    std::string condition;
    condition.clear();

    try {

        Infra::CGuard guard(mInternal->mMutex);
        if (!mInternal->mTable->deleteRecords(condition)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("delete identify record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t IdentifyRecord::deleteRecordByCondition(const Json::Value& condition) {

    if (condition.empty()) {
        return StorageInvalidParamCode;
    }

    bool existCondition = false;
    std::string condString = createQueryCondition(condition, false, existCondition);

    try {
        
        if (condString.empty()) {
            return StorageInvalidParamCode;
        }
        
        if (condition.isMember("page")) {
            char szSqlBuf[1024] = {0};
            
            if (existCondition) {
                snprintf(szSqlBuf, sizeof(szSqlBuf), "%s in (select %s from %s where %s )", "_ID", "_ID", mInternal->mTblName.c_str(), condString.c_str());
            } else {
                snprintf(szSqlBuf, sizeof(szSqlBuf), "%s in (select %s from %s %s )", "_ID", "_ID", mInternal->mTblName.c_str(), condString.c_str());
            }
            condString = std::string(szSqlBuf);
        }


        Infra::CGuard guard(mInternal->mMutex);
        if (!mInternal->mTable->deleteRecords(condString)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("delete identify record failed,message:[%s] condString=[%s]",e.msg().c_str(), condString.c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t IdentifyRecord::updateRecord(const std::string &recordId, const Json::Value &info) {

    if (recordId.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        const std::string condition = std::string("_ID") + "=" + "\'" + recordId + "\'";
        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->open(condition);
        int32_t recordNum = mInternal->mTable->recordCount();
        if (0 == recordNum) {
            return StorageIdentifyNotExistCode;
        }

        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            errorf("select person num error");
            return StorageIdentifyNotExistCode;
        }

        if (info.isMember("uploadStatus") || !info["uploadStatus"].empty()) {
            record->setInteger("uploadStatus",info["uploadStatus"].asInt());
        }

        if (!mInternal->mTable->updateRecord(record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update identify record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t IdentifyRecord::count(const Json::Value &condition) {

    bool existCondition = false;
    std::string condString = createQueryCondition(condition,true, existCondition);

    try {

        Infra::CGuard guard(mInternal->mMutex);
        return mInternal->mTable->totalRecordCount(condString);

    } catch (sql::Exception e) {
        errorf("select person record count failed,message:%s",e.msg().c_str());
        return -StorageException;
    }
}

int32_t IdentifyRecord::isExist(const std::string &recordId) {
    Infra::CGuard guard(mInternal->mMutex);
    return recordIsExist(recordId);
}

int32_t IdentifyRecord::query(const Json::Value& condition, IdentifyRecordInfoVec &recordInfoVec) {

    if (condition.empty()) {
        return StorageInvalidParamCode;
    }

    int32_t recordNum = 0;
    bool existCondition = false;
    std::string condString = createQueryCondition(condition,false, existCondition);

    try {

        Infra::CGuard guard(mInternal->mMutex);
        if (existCondition) {
            mInternal->mTable->open(condString);
        } else {

            if (condString.empty()) {
                return StorageInvalidParamCode;
            }

            std::string sqlCmd = std::string("select * from ") + mInternal->mTblName + " " + condString;
            mInternal->mTable->query(sqlCmd);
        }

        recordNum = mInternal->mTable->recordCount();
        sql::Record* record = nullptr;
        for (int32_t index = 0; index < recordNum; ++index) {
            record = mInternal->mTable->getRecord(index);
            if (record == nullptr) {
                continue;
            }

            IdentifyRecordInfo recordInfo;
            exchangeStr2Struct(record,recordInfo);
            recordInfoVec.push_back(recordInfo);
        }
    } catch (sql::Exception e) {
        errorf("query person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    if (recordInfoVec.empty()) {
        return StorageIdentifyNotExistCode;
    }

    return StorageSuccess;
}

bool IdentifyRecord::createTableIndex() {

    char command[256] = {0};
    snprintf(command, sizeof(command) - 1, "CREATE INDEX IF NOT EXISTS identify_guid_index ON %s(%s);",
         mInternal->mTblName.c_str(),"personId");

    mInternal->mTable->query(command);

    //识别模式
    snprintf(command, sizeof(command) - 1, "CREATE INDEX IF NOT EXISTS identify_type_index ON %s(%s);",
         mInternal->mTblName.c_str(), "type");
    mInternal->mTable->query(command);

    //上传标志
    snprintf(command, sizeof(command) - 1, "CREATE INDEX IF NOT EXISTS identify_uploadStatus_index ON %s(%s);",
         mInternal->mTblName.c_str(), "uploadStatus");
    mInternal->mTable->query(command);

    //createdAt
    snprintf(command, sizeof(command) - 1, "CREATE INDEX IF NOT EXISTS identify_createdAt_index ON %s(%s);",
         mInternal->mTblName.c_str(), "createdAt");
    mInternal->mTable->query(command);

    return true;
}

int32_t IdentifyRecord::recordIsExist(const std::string &recordId) {

    const std::string condition = std::string("_ID") + "=" + "\'" + recordId + "\'";
    try {

        mInternal->mTable->open(condition);
        int32_t recordNum = mInternal->mTable->recordCount();
        if (0 == recordNum) {
            return StorageIdentifyNotExistCode;
        }

        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            errorf("select person num error");
            return StorageIdentifyNotExistCode;
        }

        return StorageSuccess;

    } catch (sql::Exception e) {
        errorf("person record isExist failed,message:%s",e.msg().c_str());
        return StorageException;
    }
}

std::string IdentifyRecord::createQueryCondition(const Json::Value &condition, bool onlyCount, bool &existCondition) {

    std::string condString;
    existCondition = false;

    if (condition == Json::nullValue) {
        return condString;
    }

    std::string connector;
    bool hasRegisterId = condition.isMember("recordId");
    if (hasRegisterId) {
        condString.append("_ID=\'").append(condition["recordId"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("personId")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("personId=\'").append(condition["personId"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("name")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("name=\'").append(condition["name"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("uploadStatus")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        char uploadCondition[64] = {0};
        if (condition.isMember("uploadStatusNotEqual")) {
            snprintf(uploadCondition, sizeof(uploadCondition) - 1,"uploadStatus!=%d",condition["uploadStatus"].asInt());
        } else {
            snprintf(uploadCondition, sizeof(uploadCondition) - 1,"uploadStatus=%d",condition["uploadStatus"].asInt());
        }

        condString.append(uploadCondition);
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("customSqlCmd")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append(condition["customSqlCmd"].asString());
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("relationId")) {

        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("relationId=\'").append(condition["relationId"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("type")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("type=").append(condition["type"].asString());
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("mode")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("mode=").append(condition["mode"].asString());
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("result")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("result=").append(condition["result"].asString());
        connector = " and ";
    }
    
    if (!hasRegisterId && condition.isMember("failType")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("failType=").append(condition["failType"].asString());
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("startAt")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        uint64_t endAt = condition.isMember("endAt")? condition["endAt"].asUInt64():Infra::CTime::getCurrentUTCtime();
        char createAtRange[128] = {0};
        snprintf(createAtRange, sizeof(createAtRange) - 1,"createdAt between %llu and %llu",condition["startAt"].asUInt64(),endAt);
        condString.append(createAtRange);
        connector = " and ";
    }

    existCondition = !condString.empty();

    if (onlyCount) {
        return condString;
    }

    if (!hasRegisterId && condition.isMember("sort")) {
        std::string order = condition["sort"].asBool()? "ASC":"DESC";
        condString.append(" order by _ID ").append(order);
    }

    if (!hasRegisterId && condition.isMember("page")) {
        char pageCommand[128] = {0};
        int32_t limit = condition["page"]["limit"].asInt();
        int32_t pageNum = condition["page"]["pageNum"].asInt();
        int32_t offset = (pageNum - 1) * limit;
        snprintf(pageCommand, sizeof(pageCommand) - 1," LIMIT %d OFFSET %d ",limit,offset);
        condString.append(pageCommand);
    }

    return condString;
}

void IdentifyRecord::exchangeStr2Struct(sql::Record *record, IdentifyRecordInfo &recordInfo) {
    recordInfo.recordId     = record->getValue("_ID")->asString();
    recordInfo.personId     = record->getValue("personId")->asString();
    recordInfo.name         = record->getValue("name")->asString();
    recordInfo.score        = record->getValue("score")->asInteger();
    recordInfo.type         = record->getValue("type")->asInteger();
    recordInfo.mode         = record->getValue("mode")->asInteger();
    recordInfo.result       = record->getValue("result")->asInteger();
    recordInfo.failType     = record->getValue("failType")->asInteger();
    recordInfo.spotType     = record->getValue("spotType")->asInteger();
    recordInfo.spotContent  = record->getValue("spotContent")->asString();
    recordInfo.uploadStatus = record->getValue("uploadStatus")->asInteger();
    recordInfo.elemType     = record->getValue("elemType")->asInteger();
    recordInfo.remoteId     = record->getValue("remoteId")->asString();
    recordInfo.createdAt    = record->getValue("createdAt")->asInteger();

}


}
}
