/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   VerifiableTable.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-06 16:48
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_INCLUDE_STORAGE_DEFINE_H
#define APPLICATION_INCLUDE_STORAGE_DEFINE_H

#include <utility>
#include "Storage/ITable.h"

namespace Uface {
namespace Application {

/**
 *  图片特征信息 
 */
typedef struct FeatureInfo_ {

    FeatureInfo_(const std::string& feature,const std::string& person,
                 const std::string& ver,const std::vector<float>& vec):
                 featureId(feature),personId(person),version(ver),feature(vec) {
    }

    FeatureInfo_(){}

    std::string         featureId;              /**特征Id,与注册照Id相同*/
    std::string         personId;               /**关联的人员Id*/
    std::string         version;                /**算法版本*/
    std::vector<float>  feature;
} FeatureInfo;

/**
 *  注册照信息
 */
typedef struct {
    std::string     registerId;                 /**注册照Id*/
    std::string     personId;                   /**注册照关联人员Id*/
    std::string     content;                    /**注册照内容,Json格式*/
    int32_t         format;                     /**注册照片格式,0:jpeg*/
    std::string     remark;                     /**注册照备注信息*/
    int32_t         status;                     /**注册照记录状态,与平台同步时需要,@see RecordStatus*/
    int32_t         relationType;               /**关联的平台类型*/
    std::string     relationId;                 /**关联平台id*/
    std::string     remoteRegisterId;           /**平台个体id,默认为0*/
    std::string     remotePersonId;             /**平台主体id,默认为0*/
    std::string     version;                    /**平台数据版本信息，默认为0*/
    int32_t         batchVersion;               /**本地位点,平台下发数据位点为0;设备自身数据变化时从1自增*/
    int64_t         createdAt;                  /**创建时间*/
} RegisterPicInfo;
/**
 *  人员信息
 */
typedef struct PersonInfo_ {
    std::string                 personId;                   /**人员Id*/
    std::string                 name;                       /**人员名字*/
    std::string                 phone;                      /**人员电话*/
    std::string                 password;                   /**人员密码*/
    std::string                 cardNo;                     /**卡号*/
    std::string                 personIdNo;                 /**身份证号*/
    std::string                 remark;                      /**人员备注*/
    std::string                 relationId;                 /**关联平台id,默认为0*/
    std::string                 remoteId;                   /**平台id,默认为0*/
    std::string                 version;                    /**平台数据版本信息,默认为0*/
    int32_t                     role;                       /**人员角色;0:默认,1:管理员*/
    int32_t                     status;                     /**人员记录状态,与平台同步时需要，@see RecordStatus*/
    int32_t                     relationType;               /**关联的平台类型*/
    int64_t                     batchVersion;               /**本地位点,平台下发数据位点为0;设备自身数据变化时从1自增*/
    uint64_t                    startAt;                    /**准入有效开始时间*/
    uint64_t                    endAt;                      /**准入有效结束时间*/
    uint32_t                    facePermit;                 /**刷脸权限,1:没有权限，2: 有权限(默认2)*/
    uint32_t                    cardPermit;                 /**刷卡权限,1:没有权限，2: 有权限(默认2)*/
    uint32_t                    faceCardPermit;             /**人卡合一权限,1:没有权限，2: 有权限(默认1)*/
    uint32_t                    faceIdCardPermit;           /**人证比对权限,1:没有权限，2: 有权限(默认1)*/
    uint32_t                    passwordPermit;             /**密码权限,1:没有权限，2: 有权限(默认1)*/
    uint32_t                    facePwdPermit;              /**刷脸+密码权限,1:没有权限，2: 有权限(默认1)*/
    uint32_t                    qrCodePermit;               /**二维码权限,1:没有权限，2: 有权限(默认1)*/
    uint64_t                    createdAt;                  /**创建时间*/
    std::vector<std::string>    accessTime;

    PersonInfo_() {
        role = status = 0;
        relationType = batchVersion = 0;
        startAt = endAt = 0;
        facePermit = cardPermit = 2;
        faceCardPermit = faceIdCardPermit = passwordPermit = facePwdPermit = 1;
        qrCodePermit = 1;
        createdAt = 0;
        accessTime.clear();
    }

} PersonInfo;

/**
 * @brief 记录与平台同步的状态
 */
typedef enum {
    defaultStatus       = 0,            /**默认状态,数据来源于设备本地*/
    syncedStatus        = 1,            /**数据已经平台*/
} DataSyncStatus;
/**
 * @brief 人员权限开关定义
 */
typedef enum {
    personPermitOff = 1,                 /**人员权限关*/
    personPermitOn  = 2,                 /**人员权限开*/
} PersonPermitSwitch;
/**
 *
 */
typedef enum {
    personRecModeOff = 1,               /**识别模式关*/
    personRecModeOn  = 2,               /**识别模式开*/
} PersonRecModeSwitch;

typedef enum {
    personPermitStartAt = 0,                /**人员开始准入开始时间*/
    personPermitStopAt  = 253370764800000,  /**人员开始准入结束时间*/
} PersonPermitTime;

/**
 *  识别记录信息
 */
typedef struct {
    std::string     recordId;       /**识别记录Id*/
    std::string     personId;       /**人员Id*/
    std::string     name;           /**人员名*/
    int32_t         score;          /**识别分数*/
    int32_t         type;           /**识别方式,@see IdentifySpotType*/
    int32_t         mode;           /**识别模式:0:本地识别，1: 云端识别*/
    int32_t         result;         /**识别结果:0:成功，1: 失败*/
    int32_t         failType;       /**识别失败类型,@see IdentifyFailType */
    int32_t         spotType;       /**识别现场内容,@see IdentifySpotType*/
    std::string     spotContent;    /**识别现场内容;内容为json*/
    std::string     remoteId;       /**平台Id*/
    int32_t         uploadStatus;   /**是否上传平台状态,0:未上传,1:等待上传,20:完成上传*/
    int64_t         elemType;       /**识别元标记,包括活体/比对/人员验证/识别模式/有效时间/准入时间*/
    uint64_t        createdAt;      /**创建时间*/
} IdentifyRecordInfo;

/**
 *   删除信息
 */
typedef struct {
    std::string     deleteId;       /**删除Id*/
    int32_t         type;
    int32_t         status;         /**人员记录状态,与平台同步时需要，@see RecordStatus*/
    int32_t         relationType;   /**关联的平台类型*/
    std::string     relationId;     /**关联平台id,默认为0*/
    int32_t         batchVersion;   /**本地位点,平台下发数据位点为0;设备自身数据变化时从1自增*/
    std::string     version;        /**平台数据版本信息,默认为0*/
    int64_t         createdAt;      /**创建时间*/
    std::string     remoteRegisterId;       /**平台个体id,默认为0*/
    std::string     remotePersonId; /**平台主体id,默认为0*/

} DeleteRecordInfo;

typedef std::vector<FeatureInfo>            FeatureInfoVec;
typedef std::vector<PersonInfo>             PersonInfoVec;
typedef std::vector<RegisterPicInfo>        RegisterPicInfoVec;
typedef std::vector<IdentifyRecordInfo>     IdentifyRecordInfoVec;
typedef std::vector<DeleteRecordInfo>       DeleteRecordInfoVec;

typedef enum {
    identifySuccess         = 0,          /**成功*/
    aliveFailed             = 1,          /**活体失败*/
    compareFailed           = 2,          /**比对失败*/
    noPersonPermit          = 3,          /**人员验证未开启*/
    modeDisable             = 4,          /**识别模式未开启(设备级别)*/
    disableAccess           = 5,          /**有效时间不准入*/
    noPermission            = 6           /**有效时间段不准入*/
} IdentifyFailType;

typedef enum {
    identifyRetTypeSuccess  = 1,          /**成功*/
    identifyRetTypeFailed   = 2,          /**失败*/
    identifyRetTypeUnCheck  = 3           /**未检测*/
} IdentifyRetType;

typedef enum {
    faceSpotType                ,          /**刷脸认证*/
    faceCardSpotType            ,          /**人卡合一认证*/
    facePersonIdSpotType        ,          /**人证比对*/
    cardAlgSpotType             ,          /**刷卡认证*/
    openButtonSpotType          ,          /**按钮开门*/
    openRemoteSpotType          ,          /**远程开门*/
    openPasswordSpotType        ,          /**密码开门*/
    openFacePasswordSpotType    ,          /**人+密码开门*/
} IdentifySpotType;

typedef enum {
    localIdentifyMode           ,          /**本地*/
    remoteIdentifyMode          ,          /**远程操作*/
} IdentifyMode;

}
}
#endif //APPLICATION_INCLUDE_STORAGE_VERIFIABLETABLE_H
