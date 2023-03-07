/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   RegisterPicTable.cpp
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-06 16:55
 * Description: 
 * Others:
 *************************************************/

#include "Utils/Utils.h"
#include "Storage/StorageErr.h" 
#include "Logger/Define.h"
#include "Storage/Define.h"
#include "Storage/Common.h"
#include "Storage/DBHandle.h"
#include "Storage/DBManager.h"
#include "Storage/TableInternal.h"
#include "Infra/Concurrence/Guard.h"
#include "Storage/RegisterPicTable.h"
#include "easySqllite/SqlField.h"
#include "easySqllite/SqlCommon.h"

namespace ArcFace {
namespace Application {

static sql::Field fieldRegisterPicTable[] = {

    sql::Field(sql::FIELD_KEY),
    sql::Field("registerId", sql::type_text),
    sql::Field("personId", sql::type_text,sql::flag_not_null),
    sql::Field("format", sql::type_int),
    sql::Field("content", sql::type_text),
    sql::Field("remark", sql::type_text),
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
    sql::Field("reserve7", sql::type_text),
    sql::Field("reserve8", sql::type_text),
    sql::Field("createdAt", sql::type_int),
    sql::Field(sql::DEFINITION_END)
};

RegisterPicTable* RegisterPicTable::instance() {
    static RegisterPicTable registerPicTable;
    return &registerPicTable;
}

RegisterPicTable::RegisterPicTable() {
    mInternal = new TableInternal("personRegisterPicTable");
}

RegisterPicTable::~RegisterPicTable() {
    delete mInternal->mTable;
    delete mInternal;
}

bool RegisterPicTable::initial() {

    Infra::CGuard guard(mInternal->mMutex);
    if (mInternal->mInitialed) {
        return true;
    }

    const std::string dbName(PERSON_MANAGER_DB);
    IDBManager* manager = IDBManager::instance(dbName);
    DBHandle* handle = manager->getDbHandle();

    try {

        mInternal->mTable = new sql::Table(handle->getSqlLite(),mInternal->mTblName,fieldRegisterPicTable);
        if (!mInternal->mTable->exists()) {
            mInternal->mTable->create();
        }

        createTableIndex();
        mInternal->mInitialed = true;
        infof("initial register picture table success");
        return true;
    } catch (sql::Exception e) {
        errorf("initial register picture table failed,message:%s",e.msg().c_str());
    }

    return false;
}

int32_t RegisterPicTable::createRecord(const std::string &registerId, const Json::Value &info) {

    if (registerId.empty()) {
        return StorageInvalidParamCode;
    }

    if (!info.isMember("personId") || info["personId"].empty()) {
        return StorageInvalidParamCode;
    }

    try {

        Infra::CGuard guard(mInternal->mMutex);
        int32_t iRet = recordExist(registerId);
        if (iRet == StorageSuccess) {
            return StorageRegisterPicGuidRepeatCode;
        }
        if (iRet != StorageRegisterPicNotExistCode) {
            return iRet;
        }

        sql::Record record(mInternal->mTable->fields());

        if (info.isMember("createdAt")) {
            record.setInteger("createdAt",info["createdAt"].asUInt64());
        } else {
            record.setInteger("createdAt",Infra::CTime::getCurrentUTCtime());
        }

        record.setString("registerId",registerId);
        record.setString("personId",info["personId"].asString());
        //预留字段赋空
        record.setString("reserve1", "");
        record.setString("reserve2", "");
        record.setString("reserve3", "");
        record.setString("reserve4", "");
        record.setString("reserve5", "");
        record.setString("reserve6", "");
        record.setString("reserve7", "");
        record.setString("reserve8", "");

        setRecordValue(&record,info, true);

        if (!mInternal->mTable->addRecord(&record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("create person register picture record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t RegisterPicTable::deleteRecord(const std::string &registerId) {

    if (registerId.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        Infra::CGuard guard(mInternal->mMutex);
        int32_t iRet = recordExist(registerId);
        if (iRet != StorageSuccess) {
            return iRet;
        }

        const std::string condition = std::string("registerId=\'") + registerId + "\'";
        if (!mInternal->mTable->deleteRecords(condition)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update person register picture record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t RegisterPicTable::deleteByRelationId(int32_t type, const std::string &relationId) {

    if (relationId.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        char typeStr[64] = {0};
        snprintf(typeStr, sizeof(typeStr) - 1,"relationType=%d",type);
        const std::string condition = typeStr + std::string(" and relationId=\'") + relationId + "\'";
        Infra::CGuard guard(mInternal->mMutex);
        int32_t recordNum = mInternal->mTable->deleteRecords(condition);
        if (recordNum == 0) {
            return StorageRegisterPicNotExistCode;
        }

        if (!mInternal->mTable->deleteRecords(condition)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update person register picture record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t RegisterPicTable::getMaxPosition(bool localPosition, Json::Value &position) {

    char condition[512] = {0};
    const char* field = localPosition? "batchVersion":"relationId";
    snprintf(condition, sizeof(condition) - 1,"select MAX(%s) from %s limit 1",field,mInternal->mTblName.c_str());

    position = Json::nullValue;

    try {

        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->query(condition);
        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            return StoragePersonNotExistCode;
        }

        if (localPosition) {
            position["position"] = record->getValue(field) == nullptr ? 0 : (uint64_t)record->getValue(field)->asInteger();
        } else {
            position["position"] = record->getValue(field) == nullptr ? 0 : stringToNum<int64_t >(record->getValue(field)->asString());
        }

    } catch (sql::Exception e) {
        errorf("get register pic record max failed failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t RegisterPicTable::deleteAllRecord() {

    std::string condition;
    condition.clear();

    try {

        Infra::CGuard guard(mInternal->mMutex);
        if (!mInternal->mTable->deleteRecords(condition)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("delete person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t RegisterPicTable::query(const Json::Value& condition,RegisterPicInfoVec& picInfos) {

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

            RegisterPicInfo picInfo;
            exchangeStr2Struct(record, picInfo);
            picInfos.push_back(picInfo);
        }
    } catch (sql::Exception e) {
        errorf("query person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return picInfos.empty()? StorageRegisterPicNotExistCode:StorageSuccess;
}

int32_t RegisterPicTable::query(const std::string &registerId, ArcFace::Application::RegisterPicInfo &picInfo) {

    std::string condString;
    condString.append("registerId=\'").append(registerId).append("\'");

    try {

        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->open(condString);
        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            return StoragePersonNotExistCode;
        }

        exchangeStr2Struct(record, picInfo);

    } catch (sql::Exception e) {
        errorf("query person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t RegisterPicTable::queryRecordByPage(int32_t page, int32_t pageSize, RegisterPicInfoVec &picInfos) {

    char pageCommand[256] = {0};
    snprintf(pageCommand, sizeof(pageCommand) - 1,"select * from %s INDEXED BY pictable_Id_index order by _ID ASC LIMIT %d OFFSET %d",
            mInternal->mTblName.c_str(), pageSize, (page - 1) * pageSize);

    picInfos.clear();
    int32_t recordNum = 0;
    try {
        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->query(pageCommand);
        recordNum = mInternal->mTable->recordCount();
        sql::Record* record = nullptr;
        for (int32_t index = 0; index < recordNum; ++index) {
            record = mInternal->mTable->getRecord(index);
            if (record == nullptr) {
                continue;
            }

            RegisterPicInfo picInfo;
            exchangeStr2Struct(record, picInfo);
            picInfos.push_back(picInfo);
        }
    } catch (sql::Exception e) {
        errorf("query feature record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return picInfos.empty()? StorageRegisterPicNotExistCode:StorageSuccess;
}

int32_t RegisterPicTable::count(const Json::Value &condition) {

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

int32_t RegisterPicTable::updateRecord(const std::string &id, const Json::Value &info) {

    if (id.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        const std::string condition = std::string("registerId") + "=" + "\'" + id + "\'";
        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->open(condition);
        int32_t recordNum = mInternal->mTable->recordCount();
        if (0 == recordNum) {
            return StoragePersonNotExistCode;
        }

        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            errorf("select person num error");
            return StoragePersonNotExistCode;
        }

        setRecordValue(record,info, false);
        if (!mInternal->mTable->updateRecord(record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t RegisterPicTable::isExist(const std::string &id) {
    Infra::CGuard guard(mInternal->mMutex);
    return recordExist(id);
}

bool RegisterPicTable::createTableIndex() {

    char command[256] = {0};
    snprintf(command, sizeof(command) - 1,"CREATE UNIQUE INDEX IF NOT EXISTS pictable_registerId_index ON %s (%s);",
             mInternal->mTblName.c_str(),"registerId");
    mInternal->mTable->query(command);

    snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS pictable_personId_index ON %s (%s);",
             mInternal->mTblName.c_str(),"personId");
    mInternal->mTable->query(command);

    snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS pictable_remote_index ON %s(%s);",
             mInternal->mTblName.c_str(),"reserve1");
    mInternal->mTable->query(command);

    snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS pictable_relation_index ON %s(%s,%s);",
             mInternal->mTblName.c_str(),"relationType","relationId");
    mInternal->mTable->query(command);

    snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS pictable_Id_index ON %s (%s);",
             mInternal->mTblName.c_str(),"_ID");
    mInternal->mTable->query(command);
    return true;
}

int32_t RegisterPicTable::recordExist(const std::string &registerId) {

    const std::string condition = std::string("registerId") + "=" + "\'" + registerId + "\'";

    try {

        mInternal->mTable->open(condition);
        int32_t recordNum = mInternal->mTable->recordCount();
        if (0 == recordNum) {
            return StorageRegisterPicNotExistCode;
        }

        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            errorf("select person register picture num error");
            return StorageRegisterPicNotExistCode;
        }

        return StorageSuccess;
    } catch (sql::Exception e) {
        errorf("person register picture record isExist failed,message:%s",e.msg().c_str());
        return StorageException;
    }
}

std::string RegisterPicTable::createQueryCondition(const Json::Value &condition, bool onlyCount, bool& existCondition) {

    std::string condString;
    existCondition = false;
    if (condition.empty()) {
        return condString;
    }

    std::string connector;
    bool hasRegisterId = condition.isMember("registerId");
    if (hasRegisterId) {
        condString.append("registerId=\'").append(condition["registerId"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("personId")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("personId=\'").append(condition["personId"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("relationType")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("relationType=").append(condition["relationType"].asString());
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("relationId")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("relationId=\'").append(condition["relationId"].asString().append("\'"));
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("remoteRegisterId")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("reserve3=\'").append(condition["remoteRegisterId"].asString().append("\'"));
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("remotePersonId")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("reserve1=\'").append(condition["remotePersonId"].asString().append("\'"));
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("status")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("status=").append(condition["status"].asString());
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("startBatchVersion")) {

        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("batchVersion > ").append(condition["startBatchVersion"].asString());
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("limit")) {
        char pageCommand[128] = {0};
        snprintf(pageCommand, sizeof(pageCommand) - 1," LIMIT %d",condition["limit"].asInt());
        condString.append(pageCommand);
    }

    if (!condString.empty()) {
        existCondition = true;
    }

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

bool RegisterPicTable::setRecordValue(sql::Record* record, const Json::Value& info, bool setDefault) {

    if (record == nullptr || info.empty()) {
        return false;
    }

    if (info.isMember("content")) {
        record->setString("content",jsonToString(info["content"]));
    } else if (setDefault) {
        record->setString("content", "");
    }

    if (info.isMember("remark")) {
        record->setString("remark",info["remark"].asString());
    } else if (setDefault) {
        record->setString("remark", "");
    }

    if (info.isMember("status")) {
        record->setInteger("status",info["status"].asInt());
    } else if (setDefault) {
        record->setInteger("status", 0);
    }

    if (info.isMember("format")) {
        record->setInteger("format",info["format"].asInt());
    } else if (setDefault) {
        record->setInteger("format", 0);
    }

    if (info.isMember("relationType")) {
        record->setInteger("relationType",info["relationType"].asInt());
    } else if (setDefault) {
        record->setInteger("relationType", 0);
    }

    if (info.isMember("relationId")) {
        record->setString("relationId",info["relationId"].asString());
    } else if (setDefault) {
        record->setString("relationId", "0");
    }

    if (info.isMember("remoteRegisterId")) {
        record->setString("reserve3",info["remoteRegisterId"].asString());
    } else if (setDefault) {
        record->setString("reserve3","0");
    }

    if (info.isMember("remotePersonId")) {
        record->setString("reserve1",info["remotePersonId"].asString());
    } else if (setDefault) {
        record->setString("reserve1","0");
    }

    if (info.isMember("version")) {
        record->setString("reserve2",info["version"].asString());
    } else if (setDefault) {
        record->setString("reserve2","0");
    }

    if (info.isMember("batchVersion")) {
        record->setInteger("batchVersion",info["batchVersion"].asInt64());
    } else if (setDefault) {
        record->setInteger("batchVersion", 0);
    }

    return true;
}

static void exchangeStr2Json(sql::Record *record, Json::Value &element) {
    element = Json::nullValue;
    element["registerId"]   = record->getValue("registerId")->asString();
    element["personId"]     = record->getValue("personId")->asString();
    stringToJson(record->getValue("content")->asString(),element["content"]);

    element["format"]       = (uint64_t)record->getValue("format")->asInteger();
    element["remark"]       = record->getValue("remark")->asString();
    element["status"]       = (uint64_t)record->getValue("status")->asInteger();
    element["relationType"] = (uint64_t)record->getValue("relationType")->asInteger();
    element["remoteRegisterId"]     = record->getValue("reserve3")->asString();
    element["remotePersonId"]     = record->getValue("reserve1")->asString();
    element["relationId"]   = record->getValue("relationId")->asString();
    element["batchVersion"] = (uint64_t)record->getValue("batchVersion")->asInteger();
    element["version"]      = record->getValue("reserve2")->asString();
    element["createdAt"]    = (uint64_t)record->getValue("createdAt")->asInteger();
}

void RegisterPicTable::exchangeStr2Struct(sql::Record *record, RegisterPicInfo& picInfo) {
    picInfo.registerId      = record->getValue("registerId")->asString();
    picInfo.personId        = record->getValue("personId")->asString();
    picInfo.content         = record->getValue("content")->asString();
    picInfo.format          = record->getValue("format")->asInteger();
    picInfo.remark          = record->getValue("remark")->asString();
    picInfo.status          = record->getValue("status")->asInteger();
    picInfo.relationId      = record->getValue("relationId")->asString();
    picInfo.remoteRegisterId        = record->getValue("reserve3")->asString();
    picInfo.remotePersonId  = record->getValue("reserve1")->asString();
    picInfo.version         = record->getValue("reserve2")->asString();
    picInfo.createdAt       = record->getValue("createdAt")->asInteger();
    picInfo.batchVersion    = record->getValue("batchVersion")->asInteger();
    picInfo.relationType    = record->getValue("relationType")->asInteger();
}

}
}
