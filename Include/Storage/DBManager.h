
#ifndef INC_5C_APP_STORAGE_SQLLITEMANAGER_H
#define INC_5C_APP_STORAGE_SQLLITEMANAGER_H

#include <string>
#include "Infra/Define.h"

namespace Uface {
namespace Application {

class DBHandle;
/**
 * @brief sql lite 管理
 */
class IDBManager {

public:
    /**
     * @brief 获取SQL db管理实例
     * @param[in] db    db名字
     * @return
     */
    static IDBManager* instance(const std::string& db);

public:
    /**
     * @brief 析构函数
     */
    virtual ~IDBManager() {}
    /**
     * @brief 初始化db库
     * @return
     */
    virtual bool initial() = 0;
    /**
     * @brief 事务开始
     * @return
     */
    virtual bool begin() = 0;
    /**
     * @brief 事务提交
     * @return
     */
    virtual bool commit() = 0;
    /**
     * @brief 获取sql句柄
     * @return
     */
    virtual DBHandle* getDbHandle() = 0;
    /**
     * @brief 释放db handle
     * @param dbHandle
     */
    virtual void releaseHandle(DBHandle* dbHandle) = 0;
};

}
}
#endif //INC_5C_APP_STORAGE_SQLLITEMANAGER_H
