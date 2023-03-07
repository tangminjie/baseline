


#include <list>
#include <string>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <linux/unistd.h>

#include "Utc.h"
#include "Infra/System.h"
#include "Logger/Define.h"
#include "Infra/Time/Time.h"
#include "Infra/AtomicCount.h"
#include "Infra/Concurrence/Guard.h"
#include "Infra/Concurrence/Mutex.h"


#ifndef _SC_CLK_TCK
#define sysconf
#define _SC_CLK_TCK 100
#endif

#define USECLOCK_GETTIME

namespace ArcFace {
namespace Infra {

static bool setRtcTime(bool utc);

/**ͬ同步RTC 时钟*/
static bool setRtcTime(bool utc) {
	/**枚举RTC设备*/
	int32_t rtc = -1;
	if ((rtc = open("/dev/rtc", O_WRONLY)) < 0) {
		if ((rtc = open("/dev/rtc0", O_WRONLY)) < 0) {
			if ((rtc = open("/dev/misc/rtc", O_WRONLY, 0666)) < 0) {
				return false;
			}
		}
	}

	struct tm tmTime;
	memset(&tmTime,0x00, sizeof(struct tm));
	/* Try to catch the moment when whole second is close */
	do {
		enum {TWEAK_USEC = 200};
		uint32_t adj = TWEAK_USEC;
        uint32_t remUsec;
		time_t t;
		struct timeval tv;

		gettimeofday(&tv, nullptr);

		t = tv.tv_sec;
        remUsec = 1000000 - tv.tv_usec;
		if (remUsec < 16384) {
			/* Less than 1ms to next second. Good enough */
small_rem:
			t++;
		}

		/* Prepare tm */
		if (utc) {
		    gmtime_r(&t, &tmTime); /* may read /etc/xxx (it takes time) */
		} else {
		    localtime_r(&t, &tmTime); /* same */
		}

		tmTime.tm_isdst = 0;
		/* gmtime/localtime took some time, re-get cur time */
		gettimeofday(&tv, nullptr);

		/* may happen if rem_usec was < 1024 */
		if (tv.tv_sec < t || (tv.tv_sec == t && tv.tv_usec < 16384)) {
			/* We are not too far into next second. Good. */
			break;
		}

		t++;
		/* Prepare tm */
		if (utc) {
		    gmtime_r(&t, &tmTime); /* may read /etc/xxx (it takes time) */
		} else {
		    localtime_r(&t, &tmTime); /* same */
		}

		tmTime.tm_isdst = 0;
		gettimeofday(&tv, nullptr);
        remUsec = 1000000 - tv.tv_usec;

		/* Need to sleep.
		 * Note that small adj on slow processors can make us
		 * to always overshoot tv.tv_usec < 1024 check on next
		 * iteration. That's why adj is increased on each iteration.
		 * This also allows it to be reused as a loop limiter.
		 */
		usleep(remUsec);
	}while(false);

    int32_t ret = ioctl(rtc, RTC_SET_TIME, &tmTime);
    if (ret < 0) {
        errorf("write system time to rtc devic faild!");
    }

    close(rtc);
	return (ret == 0);
}



static std::string  sFormat("yyyy-MM-dd HH:mm:ss");
static ArcFace::Infra::CTime::DateFormat sDateFormat = ArcFace::Infra::CTime::ymd;
static char sDateSeparator = '-';
static bool s12hour = false;
static bool sBTimeUpdateNow = false;


static CTime  getCurrentLocalTime();
static void disableTimezone();
static void setSysCurrentTime(const CTime& time, int toleranceMilliSec);

typedef std::list<CTime::ModifyProc> ModifyProcList;
static CTime::SetCurrentTimeHook sSetTimeHook = setSysCurrentTime;

ModifyProcList& getModifyProcList() {
    static ModifyProcList sProcList;
    return sProcList;
}

Infra::CMutex& getModifyProcListMtx() {
    static Infra::CMutex sProcListMtx;
    return sProcListMtx;
}

/**标记是否是夏令时*/
static int32_t sIsDst = -1;
const CTime CTime::minTime(1970, 1, 1, 0, 0, 0, 0);
const CTime CTime::maxTime(9999, 1, 1, 0, 0, 0, 0);

static void setSysCurrentTime(const CTime& time, int32_t toleranceMilliSec) {
    /**标记是否更改过系统时间*/
    bool bNeedCallback = true;
    /**标记RTC时钟的同步结果*/
    bool bPreRtcSync = true;
    CTime temp = time;

    do {
        if(temp < CTime::minTime){
            temp = CTime::minTime;
        } else if(temp > CTime::maxTime){
            temp = CTime::maxTime;
        }

        if(toleranceMilliSec > 0){
            if(abs((int32_t)(CTime::getCurrentTime() - temp)) <= toleranceMilliSec){
                bNeedCallback=false;
                break;
            }
        }

        struct timeval tv;
		tv.tv_sec =  timeToUtc(temp);
		tv.tv_usec = temp.millisecond *1000;

		/**设置系统时间*/
		tracef("CTime::setCurrentTime to %04d-%02d-%02d %02d:%02d:%02d:%04d\n",temp.year,temp.month,temp.day,
		                                              temp.hour, temp.minute, temp.second,temp.millisecond);
		if (0 != settimeofday(&tv, nullptr)) {
			bNeedCallback = false;
			break;
		}

		/**RTC 时钟同步*/
#ifdef PLATFORM_SH4_LINUX_N6
		if (SystemSetCurrentTime(&temp) != 0) {
			warnf("CTime::setCurrentTime(): SystemSetCurrentTime failed");
			bPreRtcSync = false;
		}
#else
#if defined( __i386__ ) || defined( __x86_64__ )
		if (systemCall("hwclock -w") != 0) {
			warnf("CTime::setCurrentTime(): hwclock -w failed\n");
			bPreRtcSync = false;
		}
#else
		if (!setRtcTime(false)) {
			warnf("CTime::setCurrentTime() rtc failed\n");
			bPreRtcSync = false;
		} else {
		    infof("CTime::setCurrentTime() rtc set successful\n");
		}

#endif
#endif // PLATFORM_SH4_LINUX_N6

    }while(false);

    /**设置成功或者更改过系统时间才回调,否则不回调*/
    if(bNeedCallback) {

        getModifyProcListMtx().enter();
        ModifyProcList tmpProcList = getModifyProcList();
        getModifyProcListMtx().leave();

        size_t szList = tmpProcList.size();
        if(szList > 0){
            for(ModifyProcList::iterator it = tmpProcList.begin(); it != tmpProcList.end(); ++it){
                CTime::ModifyProc  &tmpProc = *it;
                tmpProc(temp);
            }
        } else if (!bPreRtcSync) {
            /**预设的RTC时钟同步失败也没有外部回调函数用于RTC回调,说明RTC时钟失败了*/
            errorf("CTime::setCurrentTime()  rtc time set function failed\n");
        } else {
            infof("CTime::setCurrentTime() success without callback function\n");
        }
    }
}

int CTime::disableTimezone() {
    timezoneDisable();
    return 0;
}

bool CTime::setDst(bool flag) {
    sIsDst = flag ? 1 : 0;
    return true;
}

CTime::CTime() {}

CTime::CTime(uint64_t time) {
    breakTime(time);
}

CTime::CTime(int32_t vyear, int32_t vmonth, int32_t vday,
             int32_t vhour, int32_t vmin, int32_t vsec,int32_t vmillisec) {

    this->year = vyear;
    this->month = vmonth;
    this->day = vday;
    this->hour = vhour;
    this->minute = vmin;
    this->second = vsec;
    this->millisecond = vmillisec;
    /**校验时间，规范化，生成星期*/
    normalizeTime(*this);
}

CTime CTime::getCurrentTime() {

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    struct tm t = *localtime(&tv.tv_sec);

    CTime result;
    result.year =  t.tm_year + 1900;
    result.month = t.tm_mon + 1;
    result.wday = t.tm_wday;
    result.day = t.tm_mday;
    result.hour = t.tm_hour;
    result.minute = t.tm_min;
    result.second = t.tm_sec;
    result.millisecond = tv.tv_usec/1000;

    if(sIsDst < 0) {
        result.isdst = t.tm_isdst;
    } else {
        result.isdst = sIsDst;
    }

    return result;
}

CTime CTime::getCurTimeForPrint() {

    static const int32_t CUR_TIME_CHECK_INTERVAL = 400000;
    static Infra::CTime tnow =  getCurrentLocalTime();
	static uint64_t preTime = Infra::CTime::getCurrentMicroSecond();
	uint64_t curTime = Infra::CTime::getCurrentMicroSecond();

	if (curTime >= preTime + CUR_TIME_CHECK_INTERVAL || sBTimeUpdateNow) {
		tnow = getCurrentLocalTime();preTime = curTime;sBTimeUpdateNow = false;
	}

	return tnow;
}

uint64_t CTime::getCurrentUTCtime() {

    struct timeval tv;
    gettimeofday(&tv, nullptr);

    if(sIsDst > 0) {
        /**若为夏令时则将本地时间减去3600s*/
        tv.tv_sec -= 3600;
    }

    return (tv.tv_sec*1000ULL+tv.tv_usec/1000) ;
}

extern "C" int SystemSetCurrentTime(SystemTime* pTime);

bool CTime::setCurrentTime(const CTime& time, int32_t toleranceMilliSec) {
    /**重新调整星期,否则time中的星期不对，直接调用sSetTimeHook会导致失败*/
    CTime ttime;
    ttime.year = time.year;
    ttime.month = time.month;
    ttime.day = time.day;
    ttime.hour = time.hour;
    ttime.minute = time.minute;
    ttime.second = time.second;
    ttime.wday = time.wday;
    ttime.millisecond = time.millisecond;

    sSetTimeHook(ttime, toleranceMilliSec);
    return true;
}

uint64_t CTime::getCurrentMilliSecond() {

#if defined (USECLOCK_GETTIME)
    struct timespec tp;
    long ret = clock_gettime(CLOCK_MONOTONIC, &tp);
    if(0 == ret) {
        return (uint64_t)1 * tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
    } else {
        errorf("CTime::getCurrentMilliSecond, sys_clock_gettime failed, error : %ld",ret);
    }

    return 0 ;
#else
    struct tms t;
    /**当前tick计数,初始化为一个中间值,处理第一次溢出*/
    static uint64_t curtick = 0x7fffffff;
    /**用户互斥访问*/
    static ArcFace::Infra::CAtomicCount exclusion(0);
    /**先备份一下,避免多线程访问出错*/
    uint64_t tmptick = curtick;
    /**times必须使用非空参数,以兼容更多的平台̨*/
    uint64_t tick = (uint32_t)times(&t);

    if(tick == (uint32_t)-1) {
        tick = tmptick;
    }

    /**低32位变化说明tick 变化*/
    if((uint32_t)tmptick != (uint32_t)tick) {
        /** 溢出处理*/
        while(tick < tmptick) {
            tick += 0xffffffff;
            tick += 1;
        }

        if (++exclusion == 1) {
            /**提前处理多线程引起的时间倒流问题,提高效率*/
            if(curtick < tick) {
                curtick = tick;
            }
        }

        --exclusion;
    }

    return curtick * (1000 / sysconf(_SC_CLK_TCK));

#endif
}

uint64_t CTime::getCurrentMicroSecond() {

    struct timespec tp;
	int32_t ret = clock_gettime(CLOCK_MONOTONIC, &tp);
	if(0 == ret) {
        return (uint64_t)1 * tp.tv_sec * 1000 * 1000 + tp.tv_nsec / 1000;
	} else {
		errorf("CTime::getCurrentMicroSecond, sys_clock_gettime failed, error : %ld\n",ret);
	}

	return 0 ;
}

void CTime::setFormatString(const char* format) {

    sFormat = format ? format : "";
    /**设置日期分隔符*/
    if(sFormat.find('.') != std::string::npos) {
        sDateSeparator = '.';
    } else if(sFormat.find('/') != std::string::npos) {
        sDateSeparator = '/';
    } else if(sFormat.find('-') != std::string::npos) {
        sDateSeparator = '-';
    } else {
        ARCFACE_ASSERT(0,"not support separator falg");
    }

    /**设置12小时制*/
    s12hour = (sFormat.find('h') != std::string::npos);

    /**设置日期格式*/
    size_t pos1 = sFormat.find('y');
    size_t pos2 = sFormat.find('M');
    size_t pos3 = sFormat.find('d');
    if(pos1 < pos2 && pos2 < pos3) {
        sDateFormat = ymd;
    } else if(pos2 < pos3 && pos3 < pos1) {
        sDateFormat = mdy;
    } else if(pos3 < pos2 && pos2 < pos1){
        sDateFormat = dmy;
    } else {
        ARCFACE_ASSERT(0,"not support data format");
    }
}

CTime::DateFormat CTime::getDateFormat() {
    return sDateFormat;
}

bool CTime::get12Hour() {
    return s12hour;
}

char CTime::getSeparator() {
    return sDateSeparator;
}

bool CTime::attachModifyProc(ModifyProc proc) {
    Infra::CGuard gd(getModifyProcListMtx());
    getModifyProcList().push_back(proc);
    return true;
}

CTime::SetCurrentTimeHook CTime::attachSetCurrentTimeHook(SetCurrentTimeHook proc) {
    SetCurrentTimeHook  hookTemp = sSetTimeHook;
    sSetTimeHook = proc;
    return hookTemp;
}

uint64_t CTime::makeTime() const {
    uint64_t seconds = timeToUtc(*this);
    return seconds*1000 + this->millisecond;
}

void CTime::breakTime(uint64_t time) {
    /**linux 下可能是32位,这样会溢出*/
    uint64_t seconds = time / 1000;
    time_t tt = (time_t)seconds;
    if(seconds != (uint64_t)tt) {
        errorf("CTime::breakTime overflowed!");
    }

    utcToTime(seconds, *this);
    this->millisecond = (int32_t)(time % 1000);
}

CTime CTime::operator +(int64_t millSeconds ) const {
    CTime time;
    milliSecondsToTime(timeToMilliSeconds(*this)+millSeconds,time);
    return time;
}

CTime CTime::operator -( int64_t millSeconds ) const {
    CTime time;
    milliSecondsToTime(timeToMilliSeconds(*this)-millSeconds,time);
    return time;
}

int64_t CTime::operator -(const CTime& time ) const {
    return ((int64_t)timeToMilliSeconds(*this)- (int64_t)timeToMilliSeconds(time));
}

CTime& CTime::operator += (int64_t millSeconds) {
    milliSecondsToTime(timeToMilliSeconds(*this)+millSeconds,*this);
    return *this;
}

CTime& CTime::operator -= (int64_t millSeconds) {
    milliSecondsToTime(timeToMilliSeconds(*this)-millSeconds,*this);
    return *this;
}

bool CTime::operator == (const CTime& time) const {
    return (millisecond == time.millisecond) && (second == time.second) && (minute == time.minute) &&
            (hour == time.hour) && (day == time.day) && (month == time.month) && (year == time.year) ;
}

bool CTime::operator != (const CTime& time) const {
    return !operator==(time);
}

bool CTime::operator < (const CTime& time) const {
    return year < time.year ||
           (year == time.year && month < time.month) ||
           (year == time.year && month == time.month && day < time.day) ||
           (year == time.year && month == time.month && day == time.day && hour < time.hour) ||
           (year == time.year && month == time.month && day == time.day && hour == time.hour && minute < time.minute) ||
           (year == time.year && month == time.month && day == time.day && hour == time.hour && minute == time.minute && second < time.second) ||
           (year == time.year && month == time.month && day == time.day && hour == time.hour && minute == time.minute && second < time.second && millisecond < time.millisecond);
}

bool CTime::operator > (const CTime& time) const {
    return time.operator<(*this);
}

bool CTime::operator >= (const CTime& time) const {
    return !operator<(time);
}

bool CTime::operator <= (const CTime& time) const {
    return !operator>(time);
}

/**
 * @brief 即使设置了格式,也只是按照格式format字符串中的y以及m和d等来进行数字取值,
 * 年月日顺序、分隔符、小时制都需要由mask告知是否按照format中进行调整
 */
void CTime::format(char* buf, const char *format, int mask) const {
    char temp[8] = {0};
    char tempYear[8] = {0};
    char tempMonth[8] = {0};
    char tempDay[8] = {0};
    const char* str[3] = {nullptr};
    size_t size = strlen(format);

    ARCFACE_ASSERT(buf != nullptr,"buffer is null");
    buf[0] = '\0';

    /**日期字符串格式化*/
    int32_t ny = 0, nm = 0, nd = 0;
    const char *p = format;
    while(*p) {
        if(*p == 'y') {ny++;}
        else if(*p == 'M') {nm++;}
        else if(*p == 'd') {nd++;}
        p++;
    }

    if(ny > 2) {
        tempYear[0] = year / 1000 + '0';
        tempYear[1] = year % 1000 / 100 + '0';
        tempYear[2] = year % 100 / 10 + '0';
        tempYear[3] = year % 10 + '0';
        tempYear[4] = 0;
    } else if(ny > 0) {
        tempYear[0] = year % 100 / 10 + '0';
        tempYear[1] = year % 10 + '0';
        tempYear[2] = 0;
    } else {
        tempYear[0] = '\0';
    }

    if(nm > 0) {
        int si = 0;
        if (month >= 10 || nm > 1) {
            tempMonth[si++] = month / 10 + '0';
        }

        tempMonth[si++] = month % 10 + '0';
        tempMonth[si] = 0;
    } else {
        tempMonth[0] = '\0';
    }

    if(nd > 0) {
        int si = 0;
        if (day >= 10 || nd > 1) {
            tempDay[si++] = day / 10 + '0';
        }

        tempDay[si++] = day % 10 + '0';
        tempDay[si] = 0;
    } else {
        tempDay[0] = '\0';
    }

    DateFormat df = sDateFormat;
    if (mask & fmDateFormat) {
        char const* pos1 = std::find(format, format + size, 'y');
        char const* pos2 = std::find(format, format + size, 'M');
        char const* pos3 = std::find(format, format + size, 'd');

        if(pos1 < pos2 && pos2 < pos3) {
            df = ymd;
        } else if(pos2 < pos3 && pos3 < pos1) {
            df = mdy;
        } else if(pos3 < pos2 && pos2 < pos1) {
            df = dmy;
        } else {
            ARCFACE_ASSERT(0,"not support data format");
        }
    }

    if(df == ymd) {
        str[0] = tempYear;
        str[1] = tempMonth;
        str[2] = tempDay;
    } else if(df == mdy) {
        str[0] = tempMonth;
        str[1] = tempDay;
        str[2] = tempYear;
    } else if(df == dmy) {
        str[0] = tempDay;
        str[1] = tempMonth;
        str[2] = tempYear;
    }

    /**去掉无效的日期字符串*/
    if(str[0][0] == '\0') {
        str[0] = str[1];
        str[1] = str[2];
    } else if(str[1][0] == '\0') {
        str[1] = str[2];
    }

    /** 12小时换算*/
    bool is_12hour = (mask & fmHourFormat) ? (std::find(format, format + size, 'h') != format + size) : s12hour;
    int32_t h = this->hour;
    if (is_12hour) {
        if(h > 12) {h -= 12;}
        else if(h == 0) {h = 12;}
    }

    /** 扫描格式字符串，逐步格式化*/
    int32_t istr = 0;
    for(size_t i = 0; i < size; i++) {
        size_t n = i;
        switch(format[i]) {
            case '-':
            case '.':
            case '/':
            {
                char ds = (mask & fmSeparator) ? format[i] : sDateSeparator;
                strncat(buf, &ds, 1);
            }
                break;
            case 'y':
            case 'M':
            case 'd':
                while(n < size && format[++n] == format[i]);
                strncat(buf, str[istr++], 4);
                i = n - 1;
                break;

            case 'H':
            case 'h':
            {
                while(n < size && (format[n] == 'h' || format[n] == 'H')) {
                    ++n;
                }

                int si = 0;
                if (h >= 10 || n > i + 1) {
                    temp[si++] = h / 10 + '0';
                }

                temp[si++] = h % 10 + '0';
                temp[si] = 0;
                strncat(buf, temp, 4);
                i = n - 1;
            }

                break;
            case 'm':
            {
                while(n < size && format[++n] == 'm');
                int si = 0;
                if (minute >= 10 || n > i + 1) {
                    temp[si++] = minute / 10 + '0';
                }

                temp[si++] = minute % 10 + '0';
                temp[si] = 0;
                strncat(buf, temp, 4);
                i = n - 1;
            }
                break;

            case 's':
                {
                    while(n < size && format[++n] == 's');
                    int si = 0;
                    if (second >= 10 || n > i + 1) {
                        temp[si++] = second / 10 + '0';
                    }

                    temp[si++] = second % 10 + '0';
                    temp[si] = 0;
                    strncat(buf, temp, 4);
                    i = n - 1;
                }
                    break;

            case 't':
                if (is_12hour) {
                    while(n < size && format[++n] == 't');
                    strncat(buf,hour/12?"PM":"AM", 2);
                    i = n - 1;
                } else {
                    n = strlen(buf);
                    while(n > 0 && buf[n - 1] == ' ')
                    {
                        buf[n - 1] = '\0';
                        n--;
                    }
                }

                break;

            default:
                strncat(buf, format + i, 1);
                break;
        }
    }
}

bool CTime::parse(const char* buf, const char *format, int mask) {
    /**mask无效无效，必须按照format给出的格式进行解析*/
    int32_t n[6] = {0}; /**存放解析出的数字*/
    int32_t count = 0;	/**解析出的数字个数*/
    char const* p = buf;
    size_t size = strlen(format);
    while (count < 6) {
        while (*p >= '0' && *p <= '9') {
            n[count] = n[count] * 10 + (*p - '0');
            ++p;
        }

        ++count;
        while (*p != 0 && (*p < '0' || *p > '9')) {
            ++p;
        }

        if (*p == 0) {
            break;
        }
    }

    /**获取日期格式,年月日的先后顺序*/
    std::string timeFormat(format);
    size_t pos1 = timeFormat.find('y');
    size_t pos2 = timeFormat.find('M');
    size_t pos3 = timeFormat.find('d');
    if(pos1 < pos2 && pos2 < pos3) {
        year = n[0];month = n[1];day = n[2];
    } else if(pos2 < pos3 && pos3 < pos1) {
        month = n[0];day = n[1];year = n[2];
    } else if(pos3 < pos2 && pos2 < pos1) {
        day = n[0];month = n[1];year = n[2];
    }

    /** 不支持时分秒的顺序变更*/
    hour = n[3];minute = n[4];second = n[5];

    /** 是否按照12小时换算,存储在CTime的小时都是24小时的数字,默认必须有如果hh的话，tt是必须*/
    bool is12hour =(std::find(format, format + size, 'h') != format + size) ;

    /** 判断时间是否齐备*/
    if (count != 6) {
        errorf("CTime parser input information is not enough.");
        goto parse_error;
    }

    if (is12hour && hour > 12) {
        errorf("CTime parser input hour format is not accord with real value format.");
        goto parse_error;
    } else if (is12hour) {
        if (strstr(buf, "PM") != nullptr) {
            if(hour != 12) {
                hour += 12;
            }
        } else if (strstr(buf, "AM") != nullptr) {
            if(hour == 12){
                hour = 0;
            }
        } else {
            errorf("12 hours time point is unclear");
            goto parse_error;
        }
    }

    normalizeTime(*this);
    return true;

parse_error:
    day = 0;month = 0;year =  0;
    hour =  0;minute =  0;second =  0;
    return false;
}


static CTime  getCurrentLocalTime() {
    struct timeval tv;
    struct tm t;
    gettimeofday(&tv, nullptr);
    localtime_r(&tv.tv_sec, &t);

    CTime result;
    result.year =  t.tm_year + 1900;
    result.month = t.tm_mon + 1;
    result.wday = t.tm_wday;
    result.day = t.tm_mday;
    result.hour = t.tm_hour;
    result.minute = t.tm_min;
    result.second = t.tm_sec;
    result.millisecond = tv.tv_usec /1000;
    return result;
}
} //namespace Infra
} //namespace ArcFace