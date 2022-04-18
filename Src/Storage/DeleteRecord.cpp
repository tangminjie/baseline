/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   DeleteRecord.cpp
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-06 17:03
 * Description: 
 * Others:
 *************************************************/


#include "Storage/StorageErr.h" 
#include "Logger/Define.h"
#include "Storage/Common.h"
#include "Storage/DBHandle.h"
#include "Storage/DBManager.h"
#include "Storage/DeleteRecord.h"
#include "Storage/TableInternal.h"
#include "System/StorageManager.h"
#include "easySqllite/SqlRecord.h"

namespace Uface {
namespace Application {

static sql::Field fieldDeleteTable[] = {

    sql::Field(sql::FIELD_KEY),
    sql::Field("deleteId", sql::type_text),
    sql::Field("type", sql::type_int),
    sql::Field("status", sql::type_int),
    sql::Field("relationType", sql::type_int),
    sql::Field("relationId", sql::type_text),
    sql::Field("batchVersion", sql::type_int),
    sql::Field("reserve1", sql::type_text),
    sql::Field("reserve2", sql::type_text),
    sql::Field("reserve3", sql::type_text),
    sql::Field("reserve4", sql::type_text),
    sql::Field("reserve5", sql::type_text),
    sql::Field("reserve6", sql::type_text),
    sql::Field("createdAt", sql::type_text),

    sql::Field(sql::DEFINITION_END)
};

static bool setRecordValue(sql::Record *record, const Json::Value &info, bool setDefault);
static void exchangeStr2Json(sql::Record* record,Json::Value& element);
static void exchangeStr2Struct(sql::Record *record, DeleteRecordInfo &deleteInfo);

static DeleteRecord deleteRecordImpl;
DeleteRecord* DeleteRecord::instance() {
    return &deleteRecordImpl;
}

DeleteRecord::DeleteRecord() {
    mInternal = new TableInternal("deleteRecordTable");
}

DeleteRecord::~DeleteRecord() {
    delete mInternal->mTable;
    delete mInternal;
}

bool DeleteRecord::initial() {

    Infra::CGuard guard(mInternal->mMutex);
    if (mInternal->mInitialed) {
        return true;
    }

    const std::string dbName(DELETE_PERSON_RECORD_DB);
    IDBManager* manager = IDBManager::instance(dbName);
    DBHandle* handle = manager->getDbHandle();

    try {

        mInternal->mTable = new sql::Table(handle->getSqlLite(),mInternal->mTblName,fieldDeleteTable);
        if (mInternal->mTable->exists()) {
            createTableIndex();
            return true;
        }

        mInternal->mTable->create();
        mInternal->mTable->open();
        createTableIndex();
        mInternal->mInitialed = true;
        infof("initial person delete record table success");
        return true;
    } catch (sql::Exception e) {
        errorf("initial delete record table failed,message:%s",e.msg().c_str());
    }

    return false;

}

static bool setRecordValue(sql::Record *record, const Json::Value &info, bool setDefault) {

    if (info.isMember("status")) {
        record->setInteger("status",info["status"].asInt());
    }

    if (info.isMember("type")) {
        record->setInteger("type",info["type"].asInt());
    }

    if (info.isMember("relationType")) {
        record->setInteger("relationType",info["relationType"].asInt());
    }

    if (info.isMember("relationId")) {
        record->setString("relationId",info["relationId"].asString());
    }

    if (info.isMember("batchVersion")) {
        record->setInteger("batchVersion",info["batchVersion"].asInt64());
    } else if (setDefault) {
        record->setInteger("batchVersion",0);
    }

    if (info.isMember("remoteRegisterId")) {
        record->setString("reserve2",info["remoteRegisterId"].asString());
    } else if (setDefault) {
        record->setString("reserve2","0");
    }

    if (info.isMember("remotePersonId")) {
        record->setString("reserve1",info["remotePersonId"].asString());
    } else if (setDefault) {
        record->setString("reserve1","0");
    }

    if (info.isMember("version")) {
        record->setString("reserve3",info["version"].asString());
    } else if (setDefault) {
        record->setString("reserve3","0");
    }
    return true;
}

int32_t DeleteRecord::createRecord(const std::string &id, const Json::Value &info) {

    if (id.empty()) {
        return StorageInvalidParamCode;
    }


    try {

        Infra::CGuard guard(mInternal->mMutex);
        int32_t iRet = isRecordExist(id);
        if (iRet != StorageDeleteRecordNotExist) {
            return iRet;
        }

        sql::Record record(mInternal->mTable->fields());

        record.setInteger("createdAt",Infra::CTime::getCurrentUTCtime());
        record.setString("deleteId",id);

        setRecordValue(&record, info, true);

        if (!mInternal->mTable->addRecord(&record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("create person register picture record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t DeleteRecord::query(const Json::Value &condition, Json::Value &infos) {

    if (condition.empty()) {
        return StorageInvalidParamCode;
    }

    infos = Json::nullValue;
    int32_t recordNum = 0;
    std::string condString = createQueryCondition(condition,false);
    try {
        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->open(condString);
        recordNum = mInternal->mTable->recordCount();
        sql::Record* record = nullptr;
        for (int32_t index = 0; index < recordNum; ++index) {
            record = mInternal->mTable->getRecord(index);
            if (record == nullptr) {
                continue;
            }

            Json::Value element = Json::nullValue;
            exchangeStr2Json(record,element);
            infos.append(element);
        }
    } catch (sql::Exception e) {
        errorf("query person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    if (infos.empty()) {
        return StorageDeleteRecordNotExist;
    }

    return StorageSuccess;
}

int32_t DeleteRecord::query(const Json::Value& condition,DeleteRecordInfoVec& deleteInfos) {
    if (condition.empty()) {
        return StorageInvalidParamCode;
    }

    int32_t recordNum = 0;
    std::string condString = createQueryCondition(condition,false);
    tracef("delete table query %s",condString.c_str());
    try {
        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->open(condString);
        recordNum = mInternal->mTable->recordCount();
        sql::Record* record = nullptr;
        for (int32_t index = 0; index < recordNum; ++index) {
            record = mInternal->mTable->getRecord(index);
            if (record == nullptr) {
                continue;
            }

            DeleteRecordInfo deleteInfo;
            exchangeStr2Struct(record,deleteInfo);
            deleteInfos.push_back(deleteInfo);
        }
    } catch (sql::Exception e) {
        errorf("query person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    if (deleteInfos.empty()) {
        return StorageDeleteRecordNotExist;
    }

    return StorageSuccess;
}

int32_t DeleteRecord::updateRecord(const std::string &id, const Json::Value &info) {

    if (id.empty()) {
        return StorageInvalidParamCode;
    }

    Infra::CGuard guard(mInternal->mMutex);
    try {

        const std::string condition = std::string("deleteId") + "=" + "\'" + id + "\'";
        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->open(condition);
        int32_t recordNum = mInternal->mTable->recordCount();
        if (0 == recordNum) {
            return StorageDeleteRecordNotExist;
        }

        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            errorf("select person num error");
            return StorageDeleteRecordNotExist;
        }

        setRecordValue(record, info, true);

        if (!mInternal->mTable->updateRecord(record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t DeleteRecord::deleteRecord(const std::string &deleteId) {
    if (deleteId.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        Infra::CGuard guard(mInternal->mMutex);
        int32_t iRet = isRecordExist(deleteId);
        if (iRet == StorageDeleteRecordNotExist) {
            return iRet;
        }

        const std::string condition = std::string("deleteId") + "=" + "\'" + deleteId + "\'";
        if (!mInternal->mTable->deleteRecords(condition)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update person register picture record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t DeleteRecord::deleteAllRecord() {

    std::string condition;
    condition.clear();

    try {

        Infra::CGuard guard(mInternal->mMutex);
        if (!mInternal->mTable->deleteRecords(condition)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("delete delete record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t DeleteRecord::count(const Json::Value &condition) {

    std::string condString = createQueryCondition(condition,true);

    try {

        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->open(condString);
        int32_t recordNum = mInternal->mTable->totalRecordCount(condString);
        return recordNum;
    } catch (sql::Exception e) {
        errorf("select person record count failed,message:%s",e.msg().c_str());
        return -StorageException;
    }
}

int32_t DeleteRecord::isExist(const std::string &id) {

    Infra::CGuard guard(mInternal->mMutex);
    return isRecordExist(id);
}

bool DeleteRecord::createTableIndex() {
    char command[256] = {0};

    snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS personId_index ON %s(%s,%s);",
             mInternal->mTblName.c_str(),"type","deleteId");
    mInternal->mTable->query(command);

    snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS relation_index ON %s(%s,%s);",
             mInternal->mTblName.c_str(),"relationType","relationId");
    mInternal->mTable->query(command);

    snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS persontable_remote_index ON %s(%s);",
             mInternal->mTblName.c_str(),"reserve1");
    mInternal->mTable->query(command);
}

int32_t DeleteRecord::isRecordExist(const std::string &recordId) {

    const std::string condition = std::string("deleteId") + "=" + "\'" + recordId + "\'";

    try {

        mInternal->mTable->open(condition);
        int32_t recordNum = mInternal->mTable->recordCount();
        if (0 == recordNum) {
            return StorageDeleteRecordNotExist;
        }

        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            errorf("select delete record num error");
            return StorageDeleteRecordNotExist;
        }

        return StorageSuccess;
    } catch (sql::Exception e) {
        errorf("delete record isExist failed,message:%s",e.msg().c_str());
        return StorageException;
    }
}

std::string DeleteRecord::createQueryCondition(const Json::Value &condition, bool onlyCount) {

    std::string condString;
    std::string connector;
    bool hasDeleteId = condition.isMember("deleteId");
    if (hasDeleteId) {
        condString.append("deleteId=\'").append(condition["deleteId"].asString()).append("\'");
        connector = "and ";
    }

    if (!hasDeleteId && condition.isMember("type") && condition.isMember("type")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("type=").append(condition["type"].asString());
        connector = "and ";
    }

    if (!hasDeleteId && condition.isMember("relationType") && condition.isMember("relationType")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("relationType=").append(condition["relationType"].asString());
        connector = "and ";
    }

    if (!hasDeleteId && condition.isMember("relationId") && condition.isMember("relationId")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("relationId=").append(condition["relationId"].asString());
        connector = "and ";
    }

    if (!hasDeleteId && condition.isMember("startBatchVersion")) {

        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("batchVersion > ").append(condition["startBatchVersion"].asString());
        connector = " and ";
    }

    if (!hasDeleteId && condition.isMember("limit")) {
        char pageCommand[128] = {0};
        snprintf(pageCommand, sizeof(pageCommand) - 1," LIMIT %d",condition["limit"].asInt());
        condString.append(pageCommand);
    }

    if (!hasDeleteId && condition.isMember("remoteRegisterId")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("reserve2=\'").append(condition["remoteRegisterId"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasDeleteId && condition.isMember("remotePersonId")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("reserve1=\'").append(condition["remotePersonId"].asString()).append("\'");
        connector = " and ";
    }

    if (onlyCount) {
        return condString;
    }

    if (!hasDeleteId && condition.isMember("sort")) {
        std::string order = condition["sort"].asBool()? "ASC":"DESC";
        condString.append(" order by ").append(order);
    }

    if (!hasDeleteId && condition.isMember("page")) {
        char pageCommand[128] = {0};
        int32_t limit = condition["page"]["limit"].asInt();
        int32_t pageNum = condition["page"]["pageNum"].asInt();
        int32_t offset = (pageNum - 1) * limit;
        snprintf(pageCommand, sizeof(pageCommand) - 1,"LIMIT %d OFFSET %d",limit,offset);
        condString.append(pageCommand);
    }

    return condString;
}

static void exchangeStr2Json(sql::Record *record, Json::Value &element) {
    element = Json::nullValue;
    int32_t index = 0;
    element["deleteId"] = record->getValue("deleteId")->asString();
    element["type"] = (uint64_t)record->getValue("type")->asInteger();
    element["status"] = (uint64_t)record->getValue("status")->asInteger();
    element["relationType"] = (uint64_t)record->getValue("relationType")->asInteger();
    element["relationId"] = record->getValue("relationId")->asString();
    element["batchVersion"] =(uint64_t) record->getValue("batchVersion")->asInteger();
    element["createdAt"] = (uint64_t)record->getValue("createdAt")->asInteger();
    element["remoteRegisterId"] = record->getValue("reserve2")->asString();
    element["remotePersonId"] = record->getValue("reserve1")->asString();
    element["version"]  = record->getValue("reserve3")->asString();
}

static void exchangeStr2Struct(sql::Record *record, DeleteRecordInfo &deleteInfo) {
    deleteInfo.deleteId = record->getValue("deleteId")->asString();
    deleteInfo.type     = record->getValue("type")->asInteger();
    deleteInfo.status   = record->getValue("status")->asInteger();
    deleteInfo.relationType = record->getValue("relationType")->asInteger();
    deleteInfo.relationId   = record->getValue("relationId")->asString();
    deleteInfo.batchVersion = record->getValue("batchVersion")->asInteger();
    deleteInfo.createdAt = record->getValue("createdAt")->asInteger();
    deleteInfo.remoteRegisterId  = record->getValue("reserve2")->asString();
    deleteInfo.remotePersonId  = record->getValue("reserve1")->asString();
    deleteInfo.version   = record->getValue("reserve3")->asString();
}

}
}
