
#ifndef FRAMEWORK_INCLUDE_INFRA_TIME_TIMEZONE_H
#define FRAMEWORK_INCLUDE_INFRA_TIME_TIMEZONE_H

#include "Infra/Define.h"

namespace Uface {
namespace Infra {

typedef struct {
    const char*  desc;  /**时区秒数*/
    int32_t      zone;  /**与0时区相差的秒数*/
} TimeZone;

extern "C" TimeZone getDiffWithTimezone0(int32_t zone);

}
}
#endif //FRAMEWORK_INCLUDE_INFRA_TIME_TIMEZONE_H
