

#ifndef FRAMEWORK_INCLUDE_INFRA_TIMECOST_H
#define FRAMEWORK_INCLUDE_INFRA_TIMECOST_H

#include "Infra/Define.h"

namespace Uface {
namespace Infra {

class CTimeCost {

public:

    CTimeCost(const char* title,const uint64_t threshold = 0);

    virtual ~CTimeCost();

    void record();

    void record(const char* info);

private:
    struct CTimeCostInternal;
    CTimeCostInternal* mInternal;
};

}
}

#endif //FRAMEWORK_INCLUDE_INFRA_TIMECOST_H
