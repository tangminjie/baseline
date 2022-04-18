

#include <ctime>
#include "Infra/Time/Time.h"
#include "Infra/Concurrence/Mutex.h"
#include "Infra/Concurrence/Guard.h"

namespace {
    
static bool enableTimezone = true;

/**每月天数*/
static int mday[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/**从1月1号到本月1号的天数*/
static int monthdays[12] = {
        0,
        31, //1
        31 + 28, //2
        31 + 28 + 31, //3
        31 + 28 + 31 + 30, //4
        31 + 28 + 31 + 30 + 31, //5
        31 + 28 + 31 + 30 + 31 + 30, //6
        31 + 28 + 31 + 30 + 31 + 30 + 31, //7
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31, //8
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30, //9
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31, //10
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30, //11
};

/**是否是闰年*/
inline bool isLeapYear(int32_t year) {
    return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}

/**
 * @brief 日期规范化
 * @param year
 * @param month
 * @param day
 */
inline void normalizeDate(int32_t &year, int32_t &month, int32_t &day) {
    if (year < 1970) {
        year = 1970;
    } else if (year > 9999) {
        year = 9999;
    }

    if (month <= 0) {
        month = 1;
    } else if (month > 12) {
        month = 12;
    }

    if (day <= 0) {
        day = 1;
    } else if (day > mday[month - 1]) {
        if (!(month == 2 && day == 29 && isLeapYear(year))) {
            day = mday[month - 1];
        }
    }
}

/**
 * @brief 时间规范化
 * @param hour
 * @param minute
 * @param second
 * @param millsec
 */
inline void normalizeTime1(int32_t &hour, int32_t &minute, int32_t &second, int32_t &millsec) {
    if (hour < 0) {
        hour = 0;
    } else if (hour > 23) {
        hour = 23;
        minute = 59;
        second = 59;
    }

    if (minute < 0) {
        minute = 0;
    } else if (minute > 59) {
        minute = 59;
    }

    if (second < 0) {
        second = 0;
    } else if (second > 59) {
        second = 59;
    }

    if (millsec < 0) {
        millsec = 0;
    } else if (millsec > 999) {
        millsec = 999;
    }
}

/**
 * @brief 自1970-01-01起的总天数
 * @param year
 * @param month
 * @param day
 * @return
 */
inline int32_t dateToDays(int32_t year, int32_t month, int32_t day) {
    /**每年365天 + 闰月增加的天数 + 当年天数*/
    int32_t leapDays = (year - 1968) / 4 - (year - 1968) / 100 + (year - 1968) / 400;
    int32_t days = (year - 1970) * 365 + leapDays + monthdays[month - 1] + (day - 1);
    /**还没有到闰月*/
    if (isLeapYear(year) && month < 3) {
        --days;
    }

    return days;
}

/**
 * @brief 计算星期数
 * @param year
 * @param month
 * @param day
 * @return
 */
inline int32_t dateToWeekDay(int32_t year, int32_t month, int32_t day) {
    int32_t leap_days = (year - 1968) / 4 - (year - 1968) / 100 + (year - 1968) / 400;
    int32_t wdays = (year - 1970) + leap_days + monthdays[month - 1] + (day - 1) + 4;
    /**还没有到闰月*/
    if (isLeapYear(year) && month < 3) {
        --wdays;
    }

    return wdays % 7;
}


/**
 * @brief 自1970-01-01起的天数转换成天数
 * @param days
 * @param year
 * @param month
 * @param day
 */
inline void daysToDate(int32_t days, int32_t &year, int32_t &month, int32_t &day) {
    year = days / 365 + 1970;
    days %= 365;

    int32_t leapDays = (year - 1 - 1968) / 4 - (year - 1 - 1968) / 100 + (year - 1 - 1968) / 400;

    year -= leapDays / 365;

    if (leapDays >= 365) {
        leapDays %= 365;
        leapDays--;
    }

    bool isLeap = false;
    if (days < leapDays) {
        --year;
        isLeap = isLeapYear(year);
        days += isLeap ? 366 : 365;
    } else {
        isLeap = isLeapYear(year);
    }

    days -= leapDays;

    for (month = 1; month <= 11; ++month) {
        if (month == 3) {
            if (isLeap) {
                if (days == monthdays[2]) {
                    month = 2;
                    break;
                }
                days--;
            }
        }

        if (days < monthdays[month]) {
            break;
        }
    }

    day = days - monthdays[month - 1] + 1;
}

/**
 * @brief 返回本地时间与GMT时间相差的毫秒数; 本地时间 + 相差的秒数 = GMT时间
 * @return
 */
inline int64_t getTimezone() {
    if (!enableTimezone) {
        return 0;
    }

    /**
     * 当前的产品中改变时区并不会修改系统的时区,只会更具时区的配置相应的修改时间,所以时区不会变化
     * 考虑性能不再每次都调用tzset更新,只更新一次
     */
    static bool first = true;
    if (first) {
        /**
         * timezone 是系统提供的全局变量,但是在时区变化时不会自动更新,需要用tzset进行刷新
         * 但是tzset 不是线程安全的,在次加锁处理
         */
        static Uface::Infra::CMutex s_mtxtz;
        Uface::Infra::CGuard guard(s_mtxtz);
        tzset();
        first = false;
    }

    return timezone;
}
}

namespace Uface {
namespace Infra {
    
void timezoneDisable() {
    enableTimezone=false;
}

int32_t dateToWeekday(int year, int month, int day) {
    return dateToWeekDay(year, month, day);
}

/**
 * @brief SystemTime规范化
 * @param time
 */
void normalizeTime(SystemTime& time) {
    normalizeDate(time.year, time.month, time.day);
    time.wday = dateToWeekDay(time.year, time.month, time.day);
    normalizeTime1(time.hour, time.minute, time.second, time.millisecond);
}

void utcToTime(uint64_t utc, SystemTime& time) {

    uint64_t seconds = utc - getTimezone();

    enum { DAY_SECONDS = 24 * 60 * 60};
    int32_t days = (int32_t)(seconds / DAY_SECONDS);
    daysToDate(days, time.year, time.month, time.day);

    int32_t sec = (int32_t)(seconds % DAY_SECONDS);

    time.hour = sec / 3600;
    sec %= 3600;
    time.minute = sec / 60;
    time.second = sec % 60;

    time.wday = dateToWeekday(time.year, time.month, time.day);
}

uint64_t timeToUtc(SystemTime const& time) {
    int32_t year = time.year, month = time.month, day = time.day;
    normalizeDate(year, month, day);
    int32_t days = dateToDays(year, month, day);

    int32_t hour = time.hour, minute = time.minute, second = time.second,milliSec = time.millisecond;
    normalizeTime1(hour, minute, second,milliSec);

    enum {DAY_SECONDS = 24 * 60 * 60};
    uint64_t seconds = uint64_t(days) * DAY_SECONDS + hour * 3600 + minute * 60 + second;
    return seconds + getTimezone();
}

void milliSecondsToTime(uint64_t localMilliSeconds, SystemTime& time) {
    uint64_t milliSeconds = localMilliSeconds;
    uint64_t seconds = milliSeconds / 1000;

    enum {DAY_SECONDS = 24 * 60 * 60};
    int32_t days = (int32_t)(seconds / DAY_SECONDS);
    daysToDate(days, time.year, time.month, time.day);

    int32_t sec = (int32_t)(seconds % DAY_SECONDS);
    time.hour = sec / 3600;
    sec %= 3600;
    time.minute = sec / 60;
    time.second = sec % 60;
    time.wday = dateToWeekday(time.year, time.month, time.day);
    time.millisecond = milliSeconds % 1000;
}

uint64_t timeToMilliSeconds(SystemTime const& time) {
    int32_t year = time.year, month = time.month, day = time.day;
    normalizeDate(year, month, day);
    int32_t days = dateToDays(year, month, day);

    int32_t hour = time.hour, minute = time.minute, second = time.second,milliSec = time.millisecond;
    normalizeTime1(hour, minute, second, milliSec);

    enum {DAY_SECONDS = 24 * 60 * 60};
    uint64_t seconds = uint64_t(days) * DAY_SECONDS + hour * 3600 + minute * 60 + second;
    return seconds*1000 + milliSec;
}

}
}