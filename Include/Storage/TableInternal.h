/************************************************
 * Copyright(c) 2021
 * 
 * Project:    Application
 * FileName:   TableInternal.h
 * Author:     tangminjie
 * Email:      tangminjie@163.com
 * Version:    V1.0.0
 * Date:       2021-04-13 18:31
 * Description: 
 * Others:
 *************************************************/


#ifndef APPLICATION_SRC_STORAGE_TABLEINTERNAL_H
#define APPLICATION_SRC_STORAGE_TABLEINTERNAL_H

#include <string>
#include "Infra/Concurrence/Mutex.h"
#include "easySqllite/SqlTable.h"

namespace Uface {
namespace Application {

class TableInternal {

public:

    TableInternal(const std::string& name)
            :mInitialed(false),mTblName(name),mTable(nullptr) {
    }

    ~TableInternal() {
    }

    bool            mInitialed;
    std::string     mTblName;
    sql::Table*     mTable;
    Infra::CMutex   mMutex;
};

}
}
#endif //APPLICATION_SRC_STORAGE_TABLEINTERNAL_H
