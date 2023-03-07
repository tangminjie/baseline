
#ifndef INC_5C_APP_UTILS_AESCBC_H
#define INC_5C_APP_UTILS_AESCBC_H

#include <cstdint>

namespace ArcFace {
namespace Application {


/**
 * @brief cbc加密
 * @param key
 * @param keyLen
 * @param input
 * @param inputLen
 * @param output
 * @return
*/
int32_t cbcEncrypt(uint8_t* key,int32_t keyLen,uint8_t* input,int32_t inputLen,uint8_t* output);
/**
 * @brief cbc 解密
 * @param key
 * @param keyLen
 * @param input
 * @param inputLen
 * @param output
 * @return
*/
int32_t cbcDecrypt(uint8_t* key,int32_t keyLen,uint8_t* input,int32_t inputLen,uint8_t* output);


}
}
#endif //INC_5C_APP_UTILS_AESCBC_H
