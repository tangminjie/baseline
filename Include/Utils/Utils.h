
#ifndef INC_5C_APP_UTILS_UTILS_H
#define INC_5C_APP_UTILS_UTILS_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "Infra/Define.h"
#include "jsoncpp/json.h"

namespace Uface {
namespace Application {

/**
 * @brief 生成uuid
 * @param[out] out           结果输出
 * @param[in]  uuIdLength    指定生成的uuid长度
 * @return
 */
int32_t generateUUID(std::string& out,int32_t uuIdLength = 32);
/**
 * @brief base64 string 转换为 float数组
 * @param[in]  base64String  base64编码的数据
 * @param[in]  maxLength     float数组最大空间
 * @param[out] floatVec      float数组
 * @return 返回数组的实际长度；-1 表示失败
 */
int32_t base64String2Float(const std::string& base64String,int32_t maxLength,float* floatVec);
/**
 * @brief float数组转换为base编码数据
 * @param[in] featureFloat
 * @param[in] length
 * @param[out] base64Str
 * @return
 */
int32_t float2Base64String(const float *floatVec,int32_t length,std::string &base64Str);
/**
 * @brief url 编码
 * @param src
 * @return
 */
std::string urlEncode(const std::string& src);
/**
 * @brief url 解码
 * @param src
 * @param str_decode
 * @return
 */
std::string urlDecode(const std::string& src);
/**
 * @brief 创建GUID
 * @return
 */
bool createGUID(std::string& uuid);
/**
 * @brief 创建GUID
 * @return
 */
std::string createGUID();
/**
 * @brief string 转json
 * @param content
 * @param root
 * @return
 */
bool stringToJson(const std::string& content,Json::Value& root);
/**
 * @brief string 转json group
 * @param content
 * @param root
 * @return
 */
bool stringToJsonGroup(const std::string& content,Json::Value& root);
/**
 *
 * @param feature
 * @param root
 * @return
 */
bool floatVecToString(const std::vector<float> &feature, std::string& value);
/**
 * @brief json转string
 * @param value
 * @param value
 * @return
 */
std::string jsonToString(const Json::Value &value);
/**
 * @brief json string 转换为 float 数组
 * @param content
 * @param feature
 * @return
 */
bool jsonStringToFloatVec(const std::string& content,std::vector<float>& feature);
bool floatVecToJsonString(const std::vector<float> &feature, std::string& value);
/**
 * @brief jpg 转换为 png
 * @param srcImage
 * @param dstImage
 * @return
 */
bool jpgToPng(const std::string &srcImage, const std::string &dstImage, bool isRename);
/**
 * @brief 创建随机数
 * @param[in] min     最小值
 * @param[in] range   范围
 * @return
 */
int32_t createRandNum(int32_t min,int32_t range);
/**
 * @brief 获取随机数
 * @param offset
 * @return
 */
int32_t createRandNum(int32_t offset);
/**
 * @brief 华氏摄氏度转摄氏度
 * @param fahrenheit
 * @return
 */
float fahrenheit2Celsius(float fahrenheit);
/**
 * @brief 摄氏度转华氏摄氏度
 * @param celsius
 * @return
 */
float celsius2Fahrenheit(float celsius);
/**
 * @brief 替换指定字符
 * @param base
 * @param strSrc
 * @param des
 */
void replaceAllString(std::string& base,const std::string& strSrc,const std::string& des);
/**
 * @brief float 转成string
 * @param value 浮点数值
 * @param percision 浮点数小数点精度
 */
std::string toString(float value, int precision);
/**
 * @brief 设置浮点数精度
 * @param value         输入值
 * @param precision     精度(>=0)
 * @return
 */
float round(float value,int32_t precision);
/**
 * @brief 字符串转数字
 * @tparam Type
 * @param str
 * @return
 */
template<class Type>
Type stringToNum(const std::string &str) {
    std::istringstream iss(str);
    Type num;iss >> num;
    return num;
}
/**
 * @brief 字符串分割
 * @param str
 * @param pattern
 * @return
 */
std::vector<std::string> splitString(std::string str, std::string pattern);

}
}

#endif //INC_5C_APP_UTILS_UTILS_H
