
#include "Infra/Time/Time.h"
#include "Infra/Time/TimeRecord.h"
#include <cstdio>
#include <string>

namespace Uface {
namespace Infra {

struct CTimeRecord::TimeRecordInternal {

    TimeRecordInternal(const char* name,int32_t size):
                        mPoints(nullptr),mName(name),mSize(size) {
        mPoints = new TimePoint[size];
    }

    ~TimeRecordInternal() {
        delete []mPoints;
    }

    TimePoint*      mPoints;
    std::string     mName;
    int32_t         mPosition;
    int32_t         mSize;
};

CTimeRecord::CTimeRecord(const char *name, int32_t size) {
    mInternal = new TimeRecordInternal(name,size);
}

CTimeRecord::~CTimeRecord() {
    delete mInternal;
}

void CTimeRecord::reset() {
    mInternal->mPosition = 0;
}

void CTimeRecord::sample(const char *name) {
    mInternal->mPoints[mInternal->mPosition].name = name;
    mInternal->mPoints[mInternal->mPosition].uSecond = CTime::getCurrentMicroSecond();
    mInternal->mPosition++;
}

void CTimeRecord::stat(uint32_t timeout) {
    sample(nullptr);

    TimePoint* point = mInternal->mPoints;
    if (timeout == 0 ||
            point[mInternal->mPosition - 1].uSecond - point[0].uSecond >= timeout) {
        for (int32_t index = 1; index < mInternal->mPosition; ++index) {
            printf("%s-Per-%s-%s : %d us\n",mInternal->mName.c_str(),point[index -1].name,point[index].name,
                   (index == 0)? 0:(int32_t)(point[index].uSecond - point[index - 1].uSecond));
        }
    }
}

}
}