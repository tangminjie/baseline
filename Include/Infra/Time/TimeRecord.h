

#ifndef FRAMEWORK_INCLUDE_INFRA_TIME_TIMERECORD_H
#define FRAMEWORK_INCLUDE_INFRA_TIME_TIMERECORD_H

#include "Infra/Define.h"

namespace Uface {
namespace Infra {
/**
 * @brief 时间记录,用于测试统计多个样点之间的时间差
 */
class CTimeRecord {

public:

    typedef struct {
        const char* name;
        uint64_t    uSecond;
    } TimePoint;

public:
    /**
     * @brief 构造函数
     * @param[in] name  采样点名
     * @param[in] size  预计采样点个数,实际采样点个数应该小于该值
     */
    CTimeRecord(const char* name,int32_t size);
    /**
     * @brief 析构函数
     */
    ~CTimeRecord();
    /**
     * @brief 清空采样记录
     */
    void reset();
    /**
     * @brief 执行采样
     * @param[in] name 采样点名称标记
     */
    void sample(const char* name);
    /**
     * @brief 执行最后一次采样，并统计采样结果
     * @param timeout  超时后打印,单位微秒,0 表示总打印
     */
    void stat(uint32_t timeout = 0);

private:
    struct TimeRecordInternal;
    TimeRecordInternal*  mInternal;
};

}
}
#endif //FRAMEWORK_INCLUDE_INFRA_TIME_TIMERECORD_H
