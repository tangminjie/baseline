
#ifndef FRAMEWORK_INCLUDE_INFRA_TIME_H
#define FRAMEWORK_INCLUDE_INFRA_TIME_H


#include "Infra/Define.h"
#include "Infra/Function.h"
#include "Infra/Time/SystemTime.h"

namespace Uface {
namespace Infra {

/**
 * @brief 时间类; 支持本地时间存取,运算,格式化控制
 */
class CTime : public SystemTime {

public:

    /**
     * @brief 时间修改回调
     */
    typedef TFunction1<bool, SystemTime> ModifyProc;

    /**
     * @brief 时间修改函数
     */
    typedef TFunction2<void, const CTime&, int32_t>         SetCurrentTimeHook;

    /**
     * @brief 日期顺序格式
     */
    enum DateFormat {
        ymd,            /**年 月 日*/
        mdy,            /**月 日 年*/
        dmy             /**日 月年*/
    };

    /**
     * @brief 格式化模式
     */
    enum FormatMask {
        fmGeneral       = 0x00,		    /** ָֻ只指定是否显示以及起始值*/
        fmSeparator     = 0x01,	        /** ָ指定分割符*/
        fmDateFormat    = 0x02,	        /** ָ指定年月日顺序*/
        fmHourFormat    = 0x04,	        /** ָ指定小时制式*/

        fmAll = fmSeparator | fmDateFormat | fmHourFormat	/** ָ指定所有格式*/
    };

public:
    /**
     * @brief 构造函数
     */
    CTime();

    /**
     * @brief 相对时间构造函数
     * @param time      utc时间
     */
    CTime(uint64_t time);
    /**
     * @brief 普通时间构造函数
     * @param[in] vyear     年
     * @param[in] vmonth    月
     * @param[in] vday      日
     * @param[in] vhour     时
     * @param[in] vmin      分
     * @param[in] vsec      秒
     * @param[in] msec      毫秒
     */
    CTime(int32_t vyear, int32_t vmonth, int32_t vday, int32_t vhour, int32_t vmin, int32_t vsec, int32_t vmillisec);
    /**
     * @brief  得到相对时间(从GMT 1970-01-01 00:00:00到某个时刻经历的毫秒数)
     * @return
     */
    uint64_t makeTime() const;
    /**
     * @brief 分解相对时间
     * @param[in] time 相对时间(毫秒) (GMT 1970-01-01 00:00:00开始)
     */
    void breakTime(uint64_t time);
    /**
     * @brief 时间调节
     * @param[in] millSeconds  调节的毫秒数
     * @return
     */
    CTime operator +( int64_t millSeconds ) const;
    /**
     * @brief 时间调节
     * @param[in] millSeconds  调节的毫秒数
     * @return
     */
    CTime operator -( int64_t millSeconds ) const;
    /**
     * @brief 时间差运算
     * @param[in] time    相比较的结果
     * @return 相差的秒数
     */
    int64_t operator -( const CTime& time) const;
    /**
     * @brief 时间调节
     * @param[in] millSeconds  调节的毫秒数
     * @return
     */
    CTime& operator +=( int64_t millSeconds );
    /**
     * @brief 时间调节
     * @param[in] millSeconds  调节的毫秒数
     * @return
     */
    CTime& operator -=( int64_t millSeconds );
    /**
     * @brief 时间比较
     * @param[in] time  相比较的时间
     * @return
     */
    bool operator == (const CTime& time) const;
    /**
     * @brief 时间比较
     * @param[in] time 相比较的时间
     * @return
     */
    bool operator != (const CTime& time) const;
    /**
     * @brief 时间比较
     * @param[in] time  相比较的时间
     * @return
     */
    bool operator < (const CTime& time) const;
    /**
     * @brief 时间比较
     * @param[in] time  相比较的时间
     * @return
     */
    bool operator <= (const CTime& time) const;
    /**
     * @brief 时间比较
     * @param[in] time 相比较的时间
     * @return
     */
    bool operator > (const CTime& time) const;
    /**
     * @brief 时间比较
     * @param[in] time  相比较的时间
     * @return
     */
    bool operator >= (const CTime& time) const;
    /**
     * @brief 时间格式化
     * @param[in,out] buf    字符串缓冲,要足够大
     *
     * @param[in]     format 格式化字符串 如"yyyy-MM-dd HH:mm:ss tt"
     *                yy = 年，不带世纪; yyyy = 年，带世纪
     *                M = 非0起始月; MM = 0起始月; MMMM = 月名称
     *                d = 非0起始日; dd = 0起始日
     *                H = 非0起始24小时; HH = 0起始24小时; h = 非0起始12小时; hh = 0起始12小时
     *                tt = 显示上午或者下午
     *
     * @param mask    格式选项，指定日期分割符，年月日序号,小时制式是否由统一的格式决定.
     *                0 表示使用统一格式,1 使用format 指定的格式
     */
    void format(char* buf, const char *format = "yyyy-MM-dd HH:mm:ss", int32_t mask = fmGeneral) const;
    /**
     * @brief 时间字符串解析
     * @param[in] buf    输入的字符串缓冲
     * @param[in] format 格式化字符串，当前只支持默认的方式"yyyy-MM-dd HH:mm:ss"
     * @param[in] mask   格式选项，指定日期分割符，年月日序号,小时制式是否由统一的格式决定.
     *                   0 表示使用统一格式,1 使用format 指定的格式
     * @return
     */
    bool parse(const char* buf, const char *format = "yyyy-MM-dd HH:mm:ss", int32_t mask = fmGeneral);

public:
    /**
     * @brief  获取系统时间
     * @note   该接口频繁调用会造成较高的CPU,适用于调用不频繁的场景
     * @return
     */
    static CTime getCurrentTime();
    /**
     * @brief  从时间缓存中获取系统时间
     * @note   该接口提供的时间与实际时间存在1s的偏差,但cpu占用率低
     * @return
     */
    static CTime getCurTimeForPrint();
    /**
     * @brief 获取未经本地化处理的时间(毫秒)，执行速度比getCurrentTime快
     * @note  由于嵌入式OS没有时区文件,代码中将系统时间作为本地时间,该接口返回的不一定就是0 时区的时间
     * @return
     */
    static uint64_t getCurrentUTCtime();
    /**
     * @brief   设置本地当前系统时间
     * @param[in]  time 新的时间
     * @param[in] toleranceMilliSec  表示容许设置时间和当前差多少毫秒内不做修改
     */
    static bool setCurrentTime(const CTime& time, int32_t toleranceMilliSec = 0);
    /**
     * @brief   获取从系统启动到目前的毫秒数
     * @return
     */
    static uint64_t getCurrentMilliSecond();
    /**
     * @brief   获取从系统启动到目前的微秒数
     * @return
     */
    static uint64_t getCurrentMicroSecond();
    /**
     * @brief 设置时间格式,会影响Format 输出的字符串格式,如"yyyy-MM-dd HH:mm:ss"
     * @param format
     */
    static void setFormatString(const char* format);
    /**
     * @brief 获取日期格式
     * @return
     */
    static DateFormat getDateFormat();
    /**
     * @brief 是否是12小时制
     * @return
     */
    static bool get12Hour();

    /**
     * @brief 获取日期分隔符
     * @return
     */
    static char getSeparator();
    /**
     * @brief   注册修改时间的回调函数
     * @param[in] proc  时间修改回调函数
     */
    static bool attachModifyProc(ModifyProc proc);
    /**
     * @brief  注册自定义修改时间的函数
     * @param[in] proc 自定义修改时间函数
     */
    static SetCurrentTimeHook attachSetCurrentTimeHook(SetCurrentTimeHook proc);
    /**
     * @brief 禁止使用时区;CTime中不进行时区转换,业务获取时区
     * @return
     */
    static int disableTimezone();
    /**
     * @brief 设置夏令时标记
     * @param[in] flag true夏令时 false 不在夏令时
     * @return
     */
    static bool setDst(bool flag);

public:
    static const CTime minTime;     /** 有效最小时间*/
    static const CTime maxTime;     /** 有效最大时间*/

};

} // namespace Infra
} // namespace Uface

#endif //FRAMEWORK_INCLUDE_INFRA_TIME_H
