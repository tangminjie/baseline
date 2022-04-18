

#ifndef FRAMEWORK_SRC_INFRA_TIME_UTC_H
#define FRAMEWORK_SRC_INFRA_TIME_UTC_H

#include "Infra/Define.h"
#include "Infra/Time/SystemTime.h"

namespace Uface {
namespace Infra {
/**
 * @brief disable timezone
 */
void timezoneDisable();
/**
 * @brief utc(秒) 转换为SystemTime
 * @param utc
 * @param time
 */
void utcToTime(uint64_t utc, SystemTime& time);
/**
 * @brief SystemTime 转换为 utc(秒)
 * @param time
 * @return
 */
uint64_t timeToUtc(SystemTime const& time);
/**
 * @brief SystemTime规范化
 * @param time
 */
void normalizeTime(SystemTime& time);
/**
 * @brief 日期转换为星期
 * @param year
 * @param month
 * @param day
 * @return
 */
int32_t dateToWeekday(int32_t year, int32_t month, int32_t day);
/**
 * @brief SystemTime 转换为本地时间秒数
 * @param time
 * @return
 */
uint64_t timeToMilliSeconds(SystemTime const& time);
/**
 * @brief 本地时间秒数 转换为 SystemTime
 * @param localSeconds
 * @param time
 */
void milliSecondsToTime(uint64_t localMilliSeconds, SystemTime& time);

}
}
#endif //FRAMEWORK_SRC_INFRA_TIME_UTC_H
