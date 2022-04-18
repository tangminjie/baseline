/************************************************
 * Copyright(c) 2021
 *
 * Project:    Application
 * FileName:   Errno.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-23 18:59
 * Description:
 * Others:
 *************************************************/


#ifndef APPLICATION_INCLUDE_STORAGE_ERRNO_H
#define APPLICATION_INCLUDE_STORAGE_ERRNO_H

namespace Uface {
namespace Application {

typedef enum {

    StorageSuccess                          = 0x10020000,        /**存储成功*/
    StorageInvalidParamCode                 = 0x10020001,        /**非法入参*/
    StorageDbOperateFailedCode              = 0x10020002,        /**数据库操作异常*/

    StorageCreatePersonFailedCode           = 0x10020003,        /**创建人员记录失败*/
    StorageUpdatePersonFailedCode           = 0x10020004,        /**更新人员记录失败*/
    StorageDeletePersonFailedCode           = 0x10020005,        /**删除人员记录失败*/
    StoragePersonNotExistCode               = 0x10020006,        /**人员不存在*/
    StoragePersonGuidRepeatCode             = 0x10020007,        /**数据异常:人员GUID重复*/
    StoragePersonEmptyCode                  = 0x10020008,        /**人员数据库为空*/

    StorageRegisterPicFormatErrorCode       = 0x10020009,        /**注册照格式错误*/
    StorageRegisterPicNotExistCode          = 0x1002000A,        /**注册照不存在*/
    StorageRegisterPicCreateFailedCode      = 0x1002000B,        /**创建注册照失败*/
    StorageRegisterPicGuidRepeatCode        = 0x1002000C,        /**数据异常:注册照guid重复*/
    StorageRegisterPicUpdateFailedCode      = 0x1002000D,        /**更新注册照信息失败*/
    StorageRegisterPicDeleteFailedCode      = 0x1002000E,        /**删除一个注册照信息失败*/
    StorageRegisterPicIsEmptyCode           = 0x1002000F,        /**人员注册照数据库为空*/
    StorageRegisterPicIsFullCode            = 0x10020010,        /**人员注册照达到最大值*/

    StorageFeatureNotExistCode              = 0x10020011,        /**特征值记录不存在*/
    StorageCreateFeatureFailedCode          = 0x10020012,        /**创建特征值记录失败*/
    StorageFeatureGuidRepeatCode            = 0x10020013,        /**数据异常:特征值记录guid重复*/
    StorageFeatureUpdateFailedCode          = 0x10020014,        /**更新特征值信息失败*/
    StorageFeatureDeleteFailedCode          = 0x10020015,        /**删除一个特征值信息失败*/
    StorageFeatureIsEmptyCode               = 0x10020016,        /**人员特征值数据库为空*/
    StorageDeleteGuidRepeatCode             = 0x10020017,        /**删除表guid重复*/
    StorageDeleteRecordNotExist             = 0x10020018,        /**删除记录不存在*/

    StorageIdentifyNotExistCode             = 0x10020019,        /**识别记录不存在*/
    StorageException                        = 0x1002001A,        /**存储异常*/
    StorageRecordNotExist                   = 0x1002001B,        /**记录不存在*/

    StorageDeviceRegisterPicIsFullCode      = 0x1002001C,        /**设备注册照达到最大值*/
    StorageDataLoadNotFinish                = 0x1002001D,        /**数据未加载完成*/


} StorageErrorCode;

}
}
#endif //APPLICATION_INCLUDE_STORAGE_ERRNO_H
