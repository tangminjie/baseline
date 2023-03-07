
#ifndef FRAMEWORK_INCLUDE_INFRA_TIME_SYSTEMTIME_H
#define FRAMEWORK_INCLUDE_INFRA_TIME_SYSTEMTIME_H

#include "Infra/Define.h"

namespace ArcFace {
namespace Infra {

/**
 * @brief 本地时间,可以精确到毫秒
 */
struct SystemTime {
    int32_t  year;		        /** 年*/
    int32_t  month;		        /** 月 January = 1, February = 2, 等等*/
    int32_t  day;		        /** 日 */
    int32_t  wday;		        /** 星期, Sunday = 0,Monday = 1, 等等 **/
    int32_t  hour;		        /** 时*/
    int32_t  minute;	        /** 分*/
    int32_t  second;	        /** 秒*/
    int32_t  millisecond;       /** 毫秒*/
    int32_t  isdst;		        /** 夏令时标记，< 0 未知，0 不在夏令时, > 0 在夏令时 */
    int32_t  reserved1;
    int32_t  reserved2;
    int32_t  reserved3;
};

}
}
#endif //FRAMEWORK_INCLUDE_INFRA_TIME_SYSTEMTIME_H
