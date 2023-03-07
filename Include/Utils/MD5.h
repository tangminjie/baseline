
#ifndef INC_5C_APP_UTILS_MD5_H
#define INC_5C_APP_UTILS_MD5_H

#include <string>
#include "Infra/Define.h"

namespace ArcFace {
namespace Application {

/**
 * 对文件md5加密
 * @param[in] pathname  文件名
 * @param[in] md5data   md5结果
 * @return
 */
int32_t md5File(const char *pathname, char *md5data);

/**
 * 对字符串进行md5编码
 * @param[in] sourcestr 源字符串
 * @param[in] length 源字符串长度
 * @param[out] deststr MD5编码结果
 * @param[in] lower 是否输出小写md5值
 * @param[in] longer 是否输出32位md5值
 * @return
 */
void md5String(const std::string &sourcestr, int32_t length, std::string &deststr, bool lower, bool longer);

}
}
#endif //INC_5C_APP_UTILS_MD5_H
