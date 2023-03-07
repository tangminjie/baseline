/************************************************
 * Copyright(c) 2021
 *
 * Project:    Application
 * FileName:   PersonTable.cpp
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-06 16:27
 * Description:
 * Others:
 *************************************************/

#include <memory>
#include <sstream>
//#include "DBHandle.h"
#include "Utils/Utils.h"
#include "Storage/StorageErr.h" 
#include "Logger/Define.h"
#include "Storage/Common.h"
#include "Storage/DBHandle.h"
#include "Storage/DBManager.h"
#include "Storage/PersonTable.h"
#include "Storage/TableInternal.h"
#include "Infra/Concurrence/Guard.h"
#include "easySqllite/SqlTable.h"
#include "easySqllite/SqlRecord.h"
#include "easySqllite/SqlField.h"
#include "easySqllite/SqlCommon.h"

namespace ArcFace {
namespace Application {

static sql::Field fieldPersonTable[] = {

    sql::Field(sql::FIELD_KEY),
    sql::Field("personId", sql::type_text),
    sql::Field("role", sql::type_int),
    sql::Field("name", sql::type_text,sql::flag_not_null),
    sql::Field("phone", sql::type_text),
    sql::Field("password", sql::type_text),
    sql::Field("cardNo", sql::type_text),
    sql::Field("personIdNo", sql::type_text),
    sql::Field("remark", sql::type_text),
    sql::Field("status", sql::type_int),
    sql::Field("relationType", sql::type_int),
    sql::Field("relationId", sql::type_text),
    sql::Field("batchVersion", sql::type_int),

    sql::Field("startAt", sql::type_int),
    sql::Field("endAt", sql::type_int),
    sql::Field("accessTime", sql::type_text),
    sql::Field("facePermit", sql::type_int),
    sql::Field("cardPermit", sql::type_int),
    sql::Field("faceCardPermit", sql::type_int),
    sql::Field("faceIdCardPermit", sql::type_int),
    sql::Field("passwordPermit", sql::type_int),
    sql::Field("facePwdPermit", sql::type_int),
    sql::Field("qrCodePermit", sql::type_int),
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

PersonTable* PersonTable::instance() {
    static PersonTable personTable;
    return &personTable;
}

PersonTable::PersonTable() {
    mInternal = new TableInternal("personTable");
}

PersonTable::~PersonTable() {
    delete mInternal->mTable;
    delete mInternal;
}

bool PersonTable::initial() {

    Infra::CGuard guard(mInternal->mMutex);
    if (mInternal->mInitialed) {
        return true;
    }

    const std::string dbName(PERSON_MANAGER_DB);
    IDBManager* manager = IDBManager::instance(dbName);
    DBHandle* handle = manager->getDbHandle();

    try {

        mInternal->mTable = new sql::Table(handle->getSqlLite(),mInternal->mTblName,fieldPersonTable);
        if (!mInternal->mTable->exists()) {
            mInternal->mTable->create();
        }

        createTableIndex();
        mInternal->mInitialed = true;
        infof("initial person table success");
        return true;
    } catch (sql::Exception e) {
        errorf("initial person table failed,message:%s",e.msg().c_str());
    }

    return false;
}

int32_t PersonTable::createRecord(const std::string &personId, const Json::Value &info) {

    if (personId.empty()) {
        return StorageInvalidParamCode;
    }

    if (info.empty() || info["name"].empty()) {
        return StorageInvalidParamCode;
    }

    try {

        Infra::CGuard guard(mInternal->mMutex);
        int32_t iRet = recordExist(personId);
        if (iRet == StorageSuccess) {
            return StoragePersonGuidRepeatCode;
        }

        if (iRet != StoragePersonNotExistCode) {
            return iRet;
        }

        sql::Record record(mInternal->mTable->fields());

        record.setString("personId",personId);
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
        setRecordValue(&record, info, true);

        if (!mInternal->mTable->addRecord(&record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("create person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t PersonTable::updateRecord(const std::string &personId, const Json::Value &info) {

    if (personId.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        const std::string condition = std::string("personId") + "=" + "\'" + personId + "\'";
        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->open(condition);
        int32_t recordNum = mInternal->mTable->recordCount();
        if (0 == recordNum) {
            errorf(">>>>>>>>>>>> condition = %s >>>>>>>>>>", condition.c_str());
            return StoragePersonNotExistCode;
        }

        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            errorf("select person num error");
            return StoragePersonNotExistCode;
        }

        setRecordValue(record, info, false);

        if (!mInternal->mTable->updateRecord(record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t PersonTable::updateRecordByRemoteId(const std::string &remoteId, const Json::Value &info) {

    if (remoteId.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        const std::string condition = std::string("reserve1") + "=" + "\'" + remoteId + "\'";
        Infra::CGuard guard(mInternal->mMutex);
        mInternal->mTable->open(condition);
        int32_t recordNum = mInternal->mTable->recordCount();
        if (0 == recordNum) {
            errorf(">>>>>>>>>>>> condition = %s >>>>>>>>>>", condition.c_str());
            return StoragePersonNotExistCode;
        }

        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            errorf("select person num error");
            return StoragePersonNotExistCode;
        }

        setRecordValue(record, info, false);

        if (!mInternal->mTable->updateRecord(record)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t PersonTable::updateAllRecord(const Json::Value& info) {

    if (info.empty()) {
        return StorageInvalidParamCode;
    }

    try {
        std::string updateCond = createUpdateCondition(info);
        if (updateCond.empty()) {
            return StorageInvalidParamCode;
        }

        std::string sqlCmd = std::string("UPDATE ") + mInternal->mTblName + " SET " + updateCond;
        Infra::CGuard guard(mInternal->mMutex);
        if (!mInternal->mTable->query(sqlCmd)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update all record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t PersonTable::deleteRecord(const std::string &personId) {

    if (personId.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        Infra::CGuard guard(mInternal->mMutex);
        int32_t iRet = recordExist(personId);
        if (iRet != StorageSuccess) {
            return iRet;
        }

        const std::string condition = std::string("personId") + "=" + "\'" + personId + "\'";
        if (!mInternal->mTable->deleteRecords(condition)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t PersonTable::deleteRecord(const Json::Value &condition) {

    std::string conditionStr;
    conditionStr.clear();
    if (!condition.empty()) {
        bool existCondition = true;
        conditionStr = createQueryCondition(condition,true,existCondition);
    }

    try {

        Infra::CGuard guard(mInternal->mMutex);
        if (!mInternal->mTable->deleteRecords(conditionStr)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("delete person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t PersonTable::deleteAllRecord() {
    return deleteRecord(Json::nullValue);
}

int32_t PersonTable::deleteByRelationId(int32_t type, const std::string &relationId) {

    if (relationId.empty()) {
        return StorageInvalidParamCode;
    }

    try {

        char typeStr[64] = {0};
        snprintf(typeStr, sizeof(typeStr) - 1,"relationType=%d",type);
        const std::string condition = typeStr + std::string(" and relationId=") + "\'" + relationId + "\'";

        Infra::CGuard guard(mInternal->mMutex);
        int32_t recordNum = mInternal->mTable->totalRecordCount(condition);
        if (recordNum == 0) {
            return StoragePersonNotExistCode;
        }

        if (!mInternal->mTable->deleteRecords(condition)) {
            return StorageDbOperateFailedCode;
        }

    } catch (sql::Exception e) {
        errorf("update person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

int32_t PersonTable::count(const Json::Value &condition) {

    bool existCondition = false;
    std::string condString = createQueryCondition(condition,true, existCondition);

    try {

        Infra::CGuard guard(mInternal->mMutex);
        int32_t recordNum = mInternal->mTable->totalRecordCount(condString);
        return recordNum;
    } catch (sql::Exception e) {
        errorf("select person record count failed,message:%s",e.msg().c_str());
        return -StorageException;
    }
}

int32_t PersonTable::isExist(const std::string &id) {
    Infra::CGuard guard(mInternal->mMutex);
    return recordExist(id);
}

int32_t PersonTable::isExist(const Json::Value &condition) {
    Infra::CGuard guard(mInternal->mMutex);
    return recordExist(condition);
}

int32_t PersonTable::query(const Json::Value &condition,PersonInfoVec& personInfos) {

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

            PersonInfo personInfo;
            exchangeStr2Struct(record, personInfo);
            personInfos.push_back(personInfo);
        }

    } catch (sql::Exception e) {
        errorf("query person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return personInfos.empty()? StoragePersonNotExistCode:StorageSuccess;
}

int32_t PersonTable::query(const Json::Value &condition, Json::Value &personInfo) {

    if (condition.empty()) {
        return StorageInvalidParamCode;
    }

    int32_t recordNum = 0;
    personInfo = Json::nullValue;
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

            Json::Value person = Json::nullValue;
            exchangeStr2Json(record,person);
            personInfo.append(person);
        }
    } catch (sql::Exception e) {
        errorf("query person record failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return personInfo.empty()? StoragePersonNotExistCode:StorageSuccess;
}

int32_t PersonTable::getMaxPosition(bool localPosition, Json::Value &position) {

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
        errorf("get person record max failed failed,message:%s",e.msg().c_str());
        return StorageException;
    }

    return StorageSuccess;
}

bool PersonTable::createTableIndex() {

    char command[256] = {0};

    try {
        snprintf(command, sizeof(command) - 1, "CREATE UNIQUE INDEX IF NOT EXISTS persontable_personId_index ON %s (%s);",
             mInternal->mTblName.c_str(), "personId");
        mInternal->mTable->query(command);

        snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS persontable_relation_index ON %s (%s);",
                mInternal->mTblName.c_str(),"relationId");
        mInternal->mTable->query(command);

        snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS persontable_remote_index ON %s(%s);",
                 mInternal->mTblName.c_str(),"reserve1");
        mInternal->mTable->query(command);

        snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS persontable_person_card_index ON %s(%s);",
                mInternal->mTblName.c_str(),"cardNo");
        mInternal->mTable->query(command);

        snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS persontable_person_id_no_index ON %s (%s);",
                mInternal->mTblName.c_str(),"personIdNo");
        mInternal->mTable->query(command);

        /// 权限结束时间索引
        snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS persontable_endAt_index ON %s (%s);",
                mInternal->mTblName.c_str(), "endAt");
        mInternal->mTable->query(command);

        /// 角色索引
        snprintf(command, sizeof(command) - 1,"CREATE INDEX IF NOT EXISTS persontable_role_index ON %s (%s);",
                mInternal->mTblName.c_str(), "role");
        mInternal->mTable->query(command);

    } catch(sql::Exception e)    {
        errorf("query person record failed,message:%s",e.msg().c_str());
        return false;;
    }

    return true;
}

int32_t PersonTable::recordExist(const std::string &recordId) {

    const std::string condition = std::string("personId") + "=" + "\'" + recordId + "\'";

    try {

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

        return StorageSuccess;
    } catch (sql::Exception e) {
        errorf("person record isExist failed,message:%s",e.msg().c_str());
        return StorageException;
    }
}

int32_t PersonTable::recordExist(const Json::Value &condition) {

    bool existCondition = false;
    std::string condString = createQueryCondition(condition, true,existCondition);

    try {

        mInternal->mTable->open(condString);
        int32_t recordNum = mInternal->mTable->recordCount();
        if (0 == recordNum) {
            return StoragePersonNotExistCode;
        }

        sql::Record* record = mInternal->mTable->getRecord(0);
        if (record == nullptr) {
            errorf("select person num error");
            return StoragePersonNotExistCode;
        }

        return StorageSuccess;
    } catch (sql::Exception e) {
        errorf("person record isExist failed,message:%s",e.msg().c_str());
        return StorageException;
    }
}

std::string PersonTable::createQueryCondition(const Json::Value &condition, bool onlyCount, bool& existCondition) {

    std::string condString;
    std::string connector;
    existCondition = false;

    if (condition.empty()) {
        return condString;
    }

    bool hasPersonId = condition.isMember("personId");
    if (hasPersonId) {
        condString.append("personId=\'").append(condition["personId"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasPersonId && condition.isMember("remoteId")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("reserve1=\'").append(condition["remoteId"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasPersonId && condition.isMember("relationType")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("relationType=").append(condition["relationType"].asString());
        connector = " and ";
    }

    if (!hasPersonId && condition.isMember("relationId")) {
        if (!connector.empty()) {
            condString.append(connector);
        }


        condString.append("relationId=\'").append(condition["relationId"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasPersonId && condition.isMember("name")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("name=\'").append(condition["name"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasPersonId && condition.isMember("password")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("password=\'").append(condition["password"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasPersonId && condition.isMember("cardNo")) {
        std::string cardNo = condition["cardNo"].asString();
        size_t pos = cardNo.find_first_not_of('0');
        if (pos != std::string::npos) {
            cardNo = cardNo.substr(pos);
        }

        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("strimz(cardNo)=\'").append(cardNo).append("\'");
        connector = " and ";
    }

    if (!hasPersonId && condition.isMember("personIdNo")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("personIdNo=\'").append(condition["personIdNo"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasPersonId && condition.isMember("status")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("status=\'").append(condition["status"].asString()).append("\'");
        connector = " and ";
    }

    if (!hasPersonId && condition.isMember("customSqlCmd")) {
        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append(condition["customSqlCmd"].asString());
        connector = " and ";
    }

    if (!hasPersonId && condition.isMember("startBatchVersion")) {

        if (!connector.empty()) {
            condString.append(connector);
        }

        condString.append("batchVersion > ").append(condition["startBatchVersion"].asString());
        connector = " and ";
    }

    if (!hasPersonId && condition.isMember("limit")) {
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

    if (!hasPersonId && condition.isMember("sort")) {
        std::string order = condition["sort"].asBool()? "ASC":"DESC";
        condString.append(" order by _ID ").append(order);
    }

    if (!hasPersonId && condition.isMember("page")) {
        char pageCommand[128] = {0};
        int32_t limit = condition["page"]["limit"].asInt();
        int32_t pageNum = condition["page"]["pageNum"].asInt();
        int32_t offset = (pageNum - 1) * limit;
        snprintf(pageCommand, sizeof(pageCommand) - 1," LIMIT %d OFFSET %d ",limit,offset);
        condString.append(pageCommand);
    }

    return condString;
}

std::string PersonTable::createUpdateCondition(const Json::Value& condition) {
    std::string condString;
    std::string connector;

    if (condition.empty()) {
        return condString;
    }

    if (condition.isMember("accessTime") && condition["accessTime"].isArray()) {
        condString.append("accessTime=\'").append(jsonToString(condition["accessTime"])).append("\'");
        connector = " , ";
    }

    if (condition.isMember("role")) {
        condString.append("role=").append(std::to_string(condition["role"].asUInt()));
    }

    if (condition.isMember("startAt")) {
        if (!connector.empty()) {
            condString.append(connector);
        }
        condString.append("startAt=\'").append(condition["startAt"].asString()).append("\'");
        connector = " , ";
    }

    if (condition.isMember("batchVersion")) {
        if (!connector.empty()) {
            condString.append(connector);
        }
        condString.append("batchVersion=").append(condition["batchVersion"].asString());
        connector = " , ";
    }

    if (condition.isMember("endAt")) {
        if (!connector.empty()) {
            condString.append(connector);
        }
        condString.append("endAt=\'").append(condition["endAt"].asString()).append("\'");
        connector = " , ";
    }

    return condString;
}

bool PersonTable::setRecordValue(sql::Record *record, const Json::Value &info, bool setDefault) {

    if (record == nullptr || info.empty()) {
        return false;
    }

    if (info.isMember("name")) {
        record->setString("name",info["name"].asString());
    }

    if (info.isMember("createdAt")) {
        record->setInteger("createdAt",info["createdAt"].asUInt64());
    } else if (setDefault) {
        record->setInteger("createdAt",Infra::CTime::getCurrentUTCtime());
    }

    if (info.isMember("startAt")) {
        record->setInteger("startAt",info["startAt"].asUInt64());
    } else if (setDefault) {
        record->setInteger("startAt",personPermitStartAt);
    }

    if (info.isMember("endAt")) {
        record->setInteger("endAt",info["endAt"].asUInt64());
    } else if (setDefault) {
        record->setInteger("endAt",personPermitStopAt);
    }

    if (info.isMember("batchVersion")) {
        record->setInteger("batchVersion",info["batchVersion"].asInt64());
    } else if (setDefault) {
        record->setInteger("batchVersion", 0);
    }

    if (info.isMember("phone")) {
        record->setString("phone",info["phone"].asString());
    } else if (setDefault) {
        record->setString("phone", "");
    }

    if (info.isMember("cardNo")) {
        record->setString("cardNo",info["cardNo"].asString());
    } else if (setDefault) {
        record->setString("cardNo", "");
    }

    if (info.isMember("password")) {
        record->setString("password",info["password"].asString());
    } else if (setDefault) {
        record->setString("password","");
    }

    if (info.isMember("role")) {
        record->setInteger("role",info["role"].asInt());
    } else if (setDefault) {
        record->setInteger("role", 0);
    }

    if (info.isMember("personIdNo")) {
        record->setString("personIdNo",info["personIdNo"].asString());
    } else if (setDefault) {
        record->setString("personIdNo", "");
    }

    if (info.isMember("remark")) {
        record->setString("remark",info["remark"].asString());
    } else if (setDefault) {
        record->setString("remark", "");
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

    if (info.isMember("remoteId")) {
        record->setString("reserve1",info["remoteId"].asString());
    } else if (setDefault) {
        record->setString("reserve1","0");
    }

    if (info.isMember("status")) {
        record->setInteger("status",info["status"].asInt());
    } else if (setDefault) {
        record->setInteger("status", 0);
    }

    if (info.isMember("version")) {
        record->setString("reserve2",info["version"].asString());
    } else if (setDefault) {
        record->setString("reserve2","0");
    }

    if (info.isMember("facePermit")) {
        record->setInteger("facePermit",info["facePermit"].asInt());
    } else if (setDefault) {
        record->setInteger("facePermit",personPermitOn);
    }

    if (info.isMember("cardPermit")) {
        record->setInteger("cardPermit",info["cardPermit"].asInt());
    } else if (setDefault) {
        record->setInteger("cardPermit",personPermitOn);
    }

    if (info.isMember("faceCardPermit")) {
        record->setInteger("faceCardPermit",info["faceCardPermit"].asInt());
    } else if (setDefault) {
        record->setInteger("faceCardPermit",personPermitOff);
    }

    if (info.isMember("faceIdCardPermit")) {
        record->setInteger("faceIdCardPermit",info["faceIdCardPermit"].asInt());
    } else if (setDefault) {
        record->setInteger("faceIdCardPermit",personPermitOff);
    }

    if (info.isMember("passwordPermit")) {
        record->setInteger("passwordPermit",info["passwordPermit"].asInt());
    } else if (setDefault) {
        record->setInteger("passwordPermit",personPermitOff);
    }

    if (info.isMember("facePwdPermit")) {
        record->setInteger("facePwdPermit",info["facePwdPermit"].asInt());
    } else if (setDefault) {
        record->setInteger("facePwdPermit",personPermitOff);
    }

    if (info.isMember("qrCodePermit")) {
        record->setInteger("qrCodePermit",info["qrCodePermit"].asInt());
    } else if (setDefault) {
        record->setInteger("qrCodePermit",personPermitOff);
    }

    if (info.isMember("accessTime")) {
        record->setString("accessTime", jsonToString(info["accessTime"]));
    } else if (setDefault) {
        record->setString("accessTime", "");
    }

    return true;
}

void PersonTable::exchangeStr2Json(sql::Record *record, Json::Value &element) {

    element = Json::nullValue;

    element["personId"] = record->getValue("personId")->asString();
    element["name"]     = record->getValue("name")->asString();
    element["role"]     = (uint64_t)record->getValue("role")->asInteger();
    element["phone"] = record->getValue("phone")->asString();
    element["password"] = record->getValue("password")->asString();
    element["cardNo"]   = record->getValue("cardNo")->asString();
    element["personIdNo"]   = record->getValue("personIdNo")->asString();
    element["remark"]    = record->getValue("remark")->asString();
    element["relationId"]    = record->getValue("relationId")->asString();
    element["status"]    = (uint64_t)record->getValue("status")->asInteger();
    element["version"]    = record->getValue("reserve2")->asString();

    element["relationType"] = (uint64_t)record->getValue("relationType")->asInteger();
    element["remoteId"] = record->getValue("reserve1")->asString();
    element["batchVersion"] = (uint64_t)record->getValue("batchVersion")->asInteger();
    element["createdAt"]    = (uint64_t)record->getValue("createdAt")->asInteger();

    element["facePermit"] = (uint64_t)record->getValue("facePermit")->asInteger();
    element["cardPermit"] = (uint64_t)record->getValue("cardPermit")->asInteger();
    element["faceCardPermit"] = (uint64_t)record->getValue("faceCardPermit")->asInteger();
    element["faceIdCardPermit"] = (uint64_t)record->getValue("faceIdCardPermit")->asInteger();
    element["passwordPermit"] = (uint64_t)record->getValue("passwordPermit")->asInteger();
    element["facePwdPermit"] = (uint64_t)record->getValue("facePwdPermit")->asInteger();
    element["qrCodePermit"] = (uint64_t)record->getValue("qrCodePermit")->asInteger();

    element["startAt"]  = (uint64_t)record->getValue("startAt")->asInteger();
    element["endAt"]    = (uint64_t)record->getValue("endAt")->asInteger();

}

void PersonTable::exchangeStr2Struct(sql::Record *record, PersonInfo &personInfo)
{
    personInfo.personId = record->getValue("personId")->asString();
    personInfo.name     = record->getValue("name")->asString();
    personInfo.role     = record->getValue("role")->asInteger();
    personInfo.phone = record->getValue("phone")->asString();
    personInfo.password = record->getValue("password")->asString();
    personInfo.cardNo   = record->getValue("cardNo")->asString();
    personInfo.personIdNo   = record->getValue("personIdNo")->asString();
    personInfo.remark    = record->getValue("remark")->asString();
    personInfo.relationId    = record->getValue("relationId")->asString();
    personInfo.remoteId = record->getValue("reserve1")->asString();
    personInfo.status    = record->getValue("status")->asInteger();
    personInfo.version = record->getValue("reserve2")->asString();

    personInfo.relationType = record->getValue("relationType")->asInteger();
    personInfo.batchVersion = record->getValue("batchVersion")->asInteger();
    personInfo.createdAt    = record->getValue("createdAt")->asInteger();

    personInfo.facePermit = record->getValue("facePermit")->asInteger();
    personInfo.cardPermit = record->getValue("cardPermit")->asInteger();
    personInfo.faceCardPermit = record->getValue("faceCardPermit")->asInteger();
    personInfo.faceIdCardPermit = record->getValue("faceIdCardPermit")->asInteger();
    personInfo.passwordPermit = record->getValue("passwordPermit")->asInteger();
    personInfo.facePwdPermit = record->getValue("facePwdPermit")->asInteger();
    personInfo.qrCodePermit = record->getValue("qrCodePermit")->asInteger();

    personInfo.startAt  = record->getValue("startAt")->asInteger();
    personInfo.endAt    = record->getValue("endAt")->asInteger();

    personInfo.accessTime.clear();
    std::string accessTime = record->getValue("accessTime")->asString();
    if (accessTime.empty()) {
        return;
    }

    Json::Value value = Json::nullValue;
    stringToJson(accessTime,value);
    int32_t passTimeSize = value.size();
    for (int32_t iIndex = 0u; iIndex < passTimeSize; iIndex++) {
        personInfo.accessTime.push_back(value[iIndex].asString());
    }
}

}
}
