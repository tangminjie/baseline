
#include "Utils/Base64.h"

namespace Uface {
namespace Application {

static const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline bool isBase64(uint8_t c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}


std::string base64Encode(uint8_t const* src, uint32_t len) {

    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char charArray3[3] = {0};
    unsigned char charArray4[4] = {0};

    while (len--) {
        charArray3[i++] = *(src++);
        if (i == 3) {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;

            for (i = 0; (i < 4); i++) {
                ret += base64Chars[charArray4[i]];
            }
                
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++) {
            charArray3[j] = '\0';    
        }

        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++) {
            ret += base64Chars[charArray4[j]];    
        }

        while ((i++ < 3)) {
            ret += '=';    
        }
    }

    return ret;
}


std::string base64Decode(std::string const& src) {
    int32_t srcPosition = src.size();
    uint8_t charArray4[4] = {0};
    uint8_t charArray3[3] = {0};
    std::string ret;

    int32_t i = 0;
    int32_t j = 0;
    int32_t srcIndex = 0;
    while (srcPosition-- && (src[srcIndex] != '=') && isBase64(src[srcIndex])) {
        charArray4[i++] = src[srcIndex]; srcIndex++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                charArray4[i] = base64Chars.find(charArray4[i]);
            }

            charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
            charArray3[1] = ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
            charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];

            for (i = 0; i < 3; i++) {
                ret += charArray3[i];
            }

            i = 0;
        }
    }

    if (i) {
        for (j = 0; j < i; j++) {
            charArray4[j] = base64Chars.find(charArray4[j]);
        }

        charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
        charArray3[1] = ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);

        for (j = 0; j < i - 1; j++) {
            ret += charArray3[j];
        }
    }

    return ret;
}

}
}