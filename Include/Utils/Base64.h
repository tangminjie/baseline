

#ifndef INC_5C_APP_UTILS_BASE64_H
#define INC_5C_APP_UTILS_BASE64_H


#include <string>
#include "Infra/Define.h"

namespace ArcFace {
namespace Application {
/**
 * @brief base64 编码
 * @param[in]   src 输入源
 * @param[in]   len 输入源长度
 * @return
 */
std::string base64Encode(uint8_t const* src, uint32_t len);
/**
 * @brief base64解码
 * @param[in] src   输入源
 * @return
 */
std::string base64Decode(std::string const& src);

}
}

#endif //INC_5C_APP_UTILS_BASE64_H
