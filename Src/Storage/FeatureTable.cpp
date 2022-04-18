/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   FeatureTable.cpp
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-06 16:30
 * Description: 
 * Others:
 *************************************************/



#include "Utils/Utils.h"
#include "Storage/StorageErr.h" 
#include "Logger/Define.h"
#include "Storage/Common.h"
#include "Storage/DBHandle.h"
#include "Storage/DBManager.h"
#include "Storage/FeatureTable.h"
#include "Storage/TableInternal.h"
#include "Infra/Concurrence/Guard.h"
#include "easySqllite/SqlField.h"
#include "easySqllite/SqlCommon.h"

namespace Uface {
namespace Application {

static sql::Field fieldFeatureTable[] = {

    sql::Field(sql::FIELD_KEY),
    sql::Field("featureId", sql::type_text),
    sql::Field("personId", sql::type_text),
    sql::Field("feature", sql::type_text),
    sql::Field("version", sql::type_text),
    sql::Field("reserve1", sql::type_text),
    sql::Field("reserve2", sql::type_text),
    sql::Field("reserve3", sql::type_text),
    sql::Field("reserve4", sql::type_text),
    sql::Field("reserve5", sql::type_text),
    sql::Field("createdAt", sql::type_int),
    sql::Field(sql::DEFINITION_END)
};

static void exchangeStr2Struct(sql::Record *record, FeatureInfo &featureInfo);

static FeatureTable featureTable;
FeatureTable* FeatureTable::instance() {
    return &featureTable;
}

FeatureTable::FeatureTable() {
    mInternal = new TableInternal("personFeatureTable");
}

FeatureTable::~FeatureTable() {
    delete mInternal->mTable;
    delete mInternal;
}

bool FeatureTable::initial() {

    Infra::CGuard guard(mInternal->mMutex);

    if (mInternal->mInitialed) {
        return true;
    }

    const std::string dbName(PERSON_FEATURE_DB);
    IDBManager* manager = IDBManager::instance(dbName);
    DBHandle* handle = manager->getDbHandle();

    try {

        mInternal->mTable = new sql::Table(handle->getSqlLite(),mInternal->mTblName,fieldFeatureTable);
        if (!mInternal->mTable->exists()) {
            mInternal->mTable->create();
        }

        createTableIndex();
        infof("initial person feature table success");
        return true;
    } catch (sql::Exception e) {
        errorf("initial feature table failed,message:%s",e.msg().c_str());
    }

    return false;
}

int32_t FeatureTable::createRecord(const std::string &id, const Json::Value &info) {

    if (id.empty()) {
        return StorageInvalidParamCode;
    }

    if (!info.isMember("personId") || info["personId"].empty()) {
        return StorageInvalidParamCode;
    }

    try {

        Infra::CGuard guard(mInternal->mMutex);
        int32_t iRet = recordIsExist(id);
        if (iRet != StorageFeatureNotExistCode) {
            return iRet;
        }

        sql::Record record(mInternal->mTable->fields());

        record.setString("featureId",info["featureId"].asString());
        record.setString("personId",info["personId"].asString());

        if (info.isMember("createdAt")) {
            record.setInteger("createdAt",info["createdAt"].asUInt64());
        } else {
            record.setInteger("createdAt",Infra::CTime::getCurrentUTCtime());
        }

        if (info.isMember("feature") && !info["feature"].empty()) {
            record.setString("feature",info["feature"].asString());
        } else {
            record.setString("feature", "");
        }

        if (info.isMember("version") && !info["version"].empty()) {
            record.setString("version",info["version"].asString());
        } else {
            record.setString("version", "");
        }

        //预留字段赋空
        record.setString("reserve1", "");
        record.setString("reserve2", "");
        record.setString("reserve3", "");
        record.setString("reserve4", "");
        record.setString("reserve5", "");

        if (!mInternal->mTable->addRecord(&record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("create person register feature record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t FeatureTable::createRecord(const FeatureInfo& info)
{
    if (info.featureId.empty() || info.personId.empty() || info.feature.empty() || info.version.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        Infra::CGuard guard(mInternal->mMutex);
        int32_t iRet = recordIsExist(info.featureId);
        if (iRet == StorageSuccess) {
            return StorageFeatureGuidRepeatCode;
        }
        if (iRet != StorageFeatureNotExistCode) {
            return iRet;
        }

        sql::Record record(mInternal->mTable->fields());

        record.setInteger("createdAt", Infra::CTime::getCurrentUTCtime());
        record.setString("featureId", info.featureId);
        record.setString("personId", info.personId);

        std::string feature;
        floatVecToString(info.feature,feature);
        record.setString("feature", feature);
        record.setString("version", info.version);

        //预留字段赋空
        record.setString("reserve1", "");
        record.setString("reserve2", "");
        
        if (!mInternal->mTable->addRecord(&record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("create person register feature record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t FeatureTable::query(const Json::Value &condition,FeatureInfoVec& featureVec) {

    if (condition.empty()) {
        return StorageInvalidParamCode;
    }

    featureVec.clear();
    int32_t recordNum = 0;
    bool existCondition = false;
    std::string condString = createQueryCondition(condition,false, existCondition);
    try {

        Infra::CGuard guard(mInternal->mMutex);
        if (existCondition) {
            mInternal->mTable->open(condString);
        } else {
            const std::string& expresss = condString.empty()? "" : condString;
            std::string sqlCmd = std::string("select * from ") + mInternal->mTblName + " " + expresss;
            mInternal->mTable->query(sqlCmd);
        }
        
        recordNum = mInternal->mTable->recordCount();
        sql::Record* record = nullptr;
        for (int32_t index = 0; index < recordNum; ++index) {
            record = mInternal->mTable->getRecord(index);
            if (record == nullptr) {
                continue;
            }

            FeatureInfo featureInfo;
            exchangeStr2Struct(record, featureInfo);
            featureVec.push_back(featureInfo);
        }
    } catch (sql::Exception e) {
        errorf("query feature record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return featureVec.empty()? StorageFeatureNotExistCode:StorageSuccess;
}

int32_t FeatureTable::versionMatch(const std::string &version, bool match) {

    std::string condString;
    if (!match) {
        condString.append("version!=\'").append(version).append("\'");
    } else {
        condString.append("version=\'").append(version).append("\'");
    }

    try {
        
        Infra::CGuard guard(mInternal->mMutex);
        int32_t recordNum = mInternal->mTable->totalRecordCount(condString);
        return recordNum;

    } catch (sql::Exception e) {
        errorf("person register feature record isExist failed,message:%s",e.msg().c_str());
        return -StorageException;
    }
}

int32_t FeatureTable::queryRecordByPage(int32_t page, int32_t pageSize, FeatureInfoVec &featureVec) {

    char pageCommand[256] = {0};
    snprintf(pageCommand, sizeof(pageCommand) - 1,"select * from %s INDEXED BY id_index order by _ID ASC LIMIT %d OFFSET %d",
            mInternal->mTblName.c_str(), pageSize,(page - 1) * pageSize);

    featureVec.clear();
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

            FeatureInfo featureInfo;
            exchangeStr2Struct(record, featureInfo);
            featureVec.push_back(featureInfo);
        }
    } catch (sql::Exception e) {
        errorf("query feature record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return featureVec.empty()? StorageFeatureNotExistCode:StorageSuccess;
}

int32_t FeatureTable::isExist(const std::string &id) {
    Infra::CGuard guard(mInternal->mMutex);
    return recordIsExist(id);
}

int32_t FeatureTable::updateRecord(const std::string &id, const Json::Value &info) {

    if (id.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        const std::string condition = std::string("featureId") + "=" + "\'" + id + "\'";

        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->open(condition);
        int32_t recordNum = mInternal->mTable->recordCount();
        if (0 == recordNum) {
            return StorageFeatureNotExistCode;
        }

        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            errorf("select person num error");
            return StorageFeatureNotExistCode;
        }

        if (info.isMember("feature") && !info["feature"].empty()) {
            record->setString("feature",info["feature"].asString());
        }

        if (info.isMember("version") && !info["version"].empty()) {
            record->setString("version",info["version"].asString());
        }

        if (!mInternal->mTable->updateRecord(record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t FeatureTable::updateRecord(const FeatureInfo& info) {

    if (info.featureId.empty()) {
        return StorageInvalidParamCode;
    }

    if (info.feature.empty() || info.version.empty()) {
        return StorageInvalidParamCode;
    }

    std::string feature;
    floatVecToString(info.feature,feature);

    try {

        const std::string condition = std::string("featureId") + "=" + "\'" + info.featureId + "\'";
        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->open(condition);
        int32_t recordNum = mInternal->mTable->recordCount();
        if (0 == recordNum) {
            return StorageFeatureNotExistCode;
        }

        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            errorf("select person num error");
            return StorageFeatureNotExistCode;
        }

        record->setString("feature",feature);
        record->setString("version", info.version);

        if (!mInternal->mTable->updateRecord(record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t FeatureTable::deleteRecord(const std::string &featureId) {

    if (featureId.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        Infra::CGuard guard(mInternal->mMutex);
        int32_t iRet = recordIsExist(featureId);
        if (iRet != StorageSuccess) {
            return iRet;
        }

        const std::string condition = std::string("featureId") + "=" + "\'" + featureId + "\'";
        if (!mInternal->mTable->deleteRecords(condition)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update person register feature record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t FeatureTable::deleteAllRecord() {

    Infra::CGuard guard(mInternal->mMutex);
    try {

        std::string condition;
        condition.clear();

        if (!mInternal->mTable->deleteRecords(condition)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("delete person record feature failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t FeatureTable::count(const Json::Value &condition) {
    bool existCondition = false;
    std::string condString = createQueryCondition(condition,true, existCondition);

    try {

        Infra::CGuard guard(mInternal->mMutex);
        int32_t recordNum = mInternal->mTable->totalRecordCount(condString);
        return recordNum;

    } catch (sql::Exception e) {
        errorf("select person record register feature count failed,message:%s",e.msg().c_str());
        return -StorageException;
    }
}

bool FeatureTable::createTableIndex() {

    char command[256] = {0};

    snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS id_index ON %s (%s);",
                     mInternal->mTblName.c_str(),"_ID");
    mInternal->mTable->query(command);

    snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS personId_index ON %s (%s);",
             mInternal->mTblName.c_str(),"personId");
    mInternal->mTable->query(command);

    snprintf(command, sizeof(command) - 1, "CREATE UNIQUE INDEX IF NOT EXISTS featureId_index ON %s (%s);",
             mInternal->mTblName.c_str(), "featureId");
    mInternal->mTable->query(command);

    return true;
}

int32_t FeatureTable::recordIsExist(const std::string &recordId) {

    const std::string condition = std::string("featureId") + "=" + "\'" + recordId + "\'";

    try {

        mInternal->mTable->open(condition);
        int32_t recordNum = mInternal->mTable->recordCount();
        if (0 == recordNum) {
            return StorageFeatureNotExistCode;
        }

        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            errorf("select person register feature num error");
            return StorageFeatureNotExistCode;
        }

        return StorageSuccess;
    } catch (sql::Exception e) {
        errorf("person register feature record isExist failed,message:%s",e.msg().c_str());
        return StorageException;
    }
}

std::string FeatureTable::createQueryCondition(const Json::Value &condition, bool onlyCount, bool& existCondition) {

    std::string condString;
    if (condition == Json::nullValue) {
        return condString;
    }

    std::string connector;
    bool hasRegisterId = condition.isMember("featureId");
    if (hasRegisterId) {
        condString.append("featureId=\'").append(condition["featureId"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("personId") && condition.isMember("personId")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("personId=\'").append(condition["personId"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasRegisterId && condition.isMember("version") && condition.isMember("version")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("version=\'").append(condition["version"].asString()).append("\'");
        connector = " and ";
    }

    existCondition = !condString.empty();
    if (onlyCount) {
        return condString;
    }

    if (!hasRegisterId && condition.isMember("sort")) {
        std::string order = condition["sort"].asBool()? "ASC":"DESC";
        condString.append(" order by _ID").append(order);
    }

    if (!hasRegisterId && condition.isMember("page")) {
        char pageCommand[128] = {0};
        int32_t limit = condition["page"]["limit"].asInt();
        int32_t pageNum = condition["page"]["pageNum"].asInt();
        int32_t offset = (pageNum - 1) * limit;
        snprintf(pageCommand, sizeof(pageCommand) - 1," LIMIT %d OFFSET %d",limit,offset);
        condString.append(pageCommand);
    }

    return condString;
}

void FeatureTable::exchangeStr2Json(sql::Record *record, Json::Value &element) {
    element = Json::nullValue;
    int32_t index = 0;
    element["featureId"] = record->getValue(index++)->asString();
    element["personId"] = record->getValue(index++)->asString();
    element["feature"] = record->getValue(index++)->asString();
    element["version"] = record->getValue(index++)->asString();
    element["createdAt"] = (uint64_t)record->getValue(index)->asInteger();
}

void FeatureTable::exchangeStr2Struct(sql::Record *record, FeatureInfo &featureInfo)
{
    featureInfo.featureId = record->getValue("featureId")->asString();
    featureInfo.personId  = record->getValue("personId")->asString();
    featureInfo.version   = record->getValue("version")->asString();

    std::string feature  = record->getValue("feature") ->asString();
    jsonStringToFloatVec(feature,featureInfo.feature);

//    while (feature.size() > 0) {
//
//        int p1 = feature.find("[");
//        int p2 = feature.find("]");
//        if (p1 == -1 || p2 == -1)
//            break;
//
//        std::string s = feature.substr(p1 + 1, p2 - p1 - 1);
//        char *pStr = (char *)s.c_str();
//
//        char *pStart = pStr;
//        char *pEnd = pStr + s.length();
//        char *pCurrent = pStart;
//
//        while (pStart != pEnd) {
//            while(*pCurrent != ',' && pCurrent != pEnd)
//                pCurrent++;
//            if (*pCurrent == ',') {
//                *pCurrent = '\0';
//                union intFloat val = {.iVal = (uint32_t)(strtoul(pStart, NULL, 10))};
//                featureInfo.feature.push_back(val.fVal);
//                pStart = pCurrent + 1;
//            } else {
//                union intFloat val = {.iVal = (uint32_t)(strtoul(pStart, NULL, 10))};
//                featureInfo.feature.push_back(val.fVal);
//                pStart = pCurrent;
//            }
//        }
//    }
}

}
}
