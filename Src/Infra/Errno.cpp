
#include <map>
#include <string>
#include "Infra/Error.h"
#include "Infra/Concurrence/Guard.h"
#include "Infra/Thread/ThreadSpecific.h"
#include "Infra/Concurrence/ReadWriteMutex.h"

namespace Uface {

static Infra::CThreadSpecific       tssLastError;
class CErrorManager {


    typedef std::map<int32_t ,std::string>  ErrorMap;
    typedef ErrorMap::const_iterator        ConstIter;

public:

    static CErrorManager* instance();

    static void setErrno(int32_t error) {
        tssLastError.setValue(reinterpret_cast<const void*>(error));
    }

    static int32_t getErrno() {
        void* value = nullptr;
        tssLastError.getValue(&value);
        setErrno(0);
        return (int32_t)reinterpret_cast<intptr_t >(value);
    }

    void setErrorMessage(int32_t error,const char* message) {
         Infra::CGuardWriting guardWriting(mRWMutex);
         mErrorMap[error] = message? message:"";
    }

    const char* getErrorMessage(int32_t error) {
        Infra::CGuardReading guardReading(mRWMutex);
        ConstIter iter = mErrorMap.find(error);
        return (iter != mErrorMap.end())? iter->second.c_str():"";
    }

private:
    ErrorMap                            mErrorMap;
    Infra::CReadWriteMutex              mRWMutex;
};



CErrorManager* CErrorManager::instance() {
    static CErrorManager errorManager;
    return &errorManager;
}

}

namespace Uface {
namespace Infra {

void setLastErrno(int32_t error) {
    CErrorManager::setErrno(error);
}

int32_t getLastErrno() {
    return CErrorManager::getErrno();
}

void setErrorMsg(int32_t error, const char* message) {
    CErrorManager::instance()->setErrorMessage(error, message);
}

const char* getErrorMsg(int32_t error) {
    return CErrorManager::instance()->getErrorMessage(error);
}

}

}