#include <random>
#include <sstream>
#include <regex>
#include <fstream>
#include "curl/curl.h"
#include "uuid/uuid.h"
#include "Utils/Utils.h"
#include "Utils/Base64.h"
#include "jsoncpp/json.h"

namespace Uface {
namespace Application {

typedef union {
    uint32_t    uInt32Value;
    float       floatValue;
} Uint32Float;


static bool isAlphaNumber(unsigned char c);
static unsigned char charToHex(unsigned char x);

int32_t generateUUID(std::string& out,int32_t uuIdLength) {

    static auto& chrs = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    out.reserve(uuIdLength);

    while(uuIdLength--) {
        out += chrs[pick(rg)];
    }

    return 0;
}

int32_t base64String2Float(const std::string& base64String,int32_t maxLength,float* floatVec) {

    std::string decryptString = base64Decode(base64String);
    if (decryptString.length() == 0) {
        return -1;
    }

    std::string substr;
    Uint32Float uint32Float;
    std::regex matchRegex("^\\d+$");
    std::string::size_type pos = 0, pos1 = 0;
    int32_t index = 0;
    while (true) {

        if (index >= maxLength) {
            return -1;
        }

        pos1 = decryptString.find(" ", pos);
        if (pos1 == std::string::npos) {
            pos1 = decryptString.length();
        }

        substr = decryptString.substr(pos, pos1-pos);
        if (!std::regex_match(substr, matchRegex)) {
            return -1;
        }

        uint32Float.uInt32Value = strtoul(substr.c_str(), nullptr, 10);
        floatVec[index++] = uint32Float.floatValue;
        pos = pos1 + 1;
        if (pos > decryptString.length()) {
            break;
        }
    }

    return index;
}

int32_t float2Base64String(const float *floatVec, int32_t length, std::string &floatStr) {

    if (floatVec == nullptr || length <= 0) {
        return -1;
    }


    std::stringstream ss;
    Uint32Float uint32Float;

    for (int32_t index = 0; index < length; ++index) {
        if (index != 0) {
            ss << " ";
        }

        uint32Float.floatValue = floatVec[index];
        ss << uint32Float.uInt32Value;
    }

    floatStr = base64Encode((uint8_t*)ss.str().c_str(), ss.str().size());
    return 0;
}

std::string urlEncode(const std::string& src) {
    CURL *curl = curl_easy_init();
    std::string out;

    char *outputUrl = curl_easy_escape(curl, src.c_str(),src.length());
    if (!outputUrl)
    {
       curl_easy_cleanup(curl);
    }

    out = outputUrl;
    curl_free(outputUrl);
    curl_easy_cleanup(curl);

    return out;
}


std::string urlDecode(const std::string& src) {
    std::string decodeString;
    int i;
    char *cd = (char *) src.c_str();
    char p[2];
    for (i = 0; i < strlen(cd); i++) {
        memset(p, '\0', 2);
        if (cd[i] != '%') {
            decodeString += cd[i];
            continue;
        }
        p[0] = cd[++i];
        p[1] = cd[++i];
        p[0] = p[0] - 48 - ((p[0] >= 'A') ? 7 : 0) - ((p[0] >= 'a') ? 32 : 0);
        p[1] = p[1] - 48 - ((p[1] >= 'A') ? 7 : 0) - ((p[1] >= 'a') ? 32 : 0);
        decodeString += (unsigned char) (p[0] * 16 + p[1]);
    }

    return decodeString;
}

bool createGUID(std::string& uuid) {

    char buffer[64] = {0};
    uuid_t uu;
    uuid_generate_random(uu);
    uuid_unparse(uu, buffer);
    uuid = std::string(buffer);
    return true;
}

std::string createGUID() {
    std::string guid;
    createGUID(guid);
    return guid;
}

bool stringToJson(const std::string& content,Json::Value& root) {

    if (content.empty()) {
        return false;
    }

    Json::CharReaderBuilder jsReader;
    std::unique_ptr<Json::CharReader> const reader(jsReader.newCharReader());
    std::string err;
    if (!reader->parse(content.c_str(), content.c_str() + content.size(), &root, &err)) {
        return false;
    }

    return root.isObject() || root.isArray();
}

bool stringToJsonGroup(const std::string& content,Json::Value& root) {

    std::string strContent = content;
    if (content.size() > 3 && 0xef == content[0] && 0xbb == content[1] && 0xbf == content[2]) {
        strContent = content.substr(3, content.size() - 3);
    }

    if (!stringToJson(strContent, root)) {
        return false;
    }

    return root.isObject();
}

bool floatVecToString(const std::vector<float> &feature, std::string& value) {

    Json::Value jsonFeature;
    int32_t featureSize = feature.size();
    jsonFeature.resize(0);

    for (int32_t index = 0; index < featureSize; ++index) {
        Uint32Float val = {.floatValue = feature[index]};
        jsonFeature.append(Json::Value(val.uInt32Value));
    }

    Json::StreamWriterBuilder build;
    build.settings_["indentation"] = "";
    std::unique_ptr<Json::StreamWriter> writer(build.newStreamWriter());
    std::ostringstream os;
    writer->write(jsonFeature,&os);
    value = os.str();
    return true;
}

std::string jsonToString(const Json::Value &value) {

    Json::StreamWriterBuilder build;
    build.settings_["indentation"] = "";
    std::unique_ptr<Json::StreamWriter> writer(build.newStreamWriter());
    std::ostringstream os;
    writer->write(value,&os);
    return os.str();
}

bool jsonStringToFloatVec(const std::string& content,std::vector<float>& feature) {

    Json::Value value = Json::nullValue;
    stringToJson(content,value);

    int32_t size = value.size();
    for (int32_t index = 0; index < size; ++index) {
        Uint32Float val = {.uInt32Value = value[index].asUInt()};
        feature.push_back(val.floatValue);
    }

    return true;
}

// bool jpgToPng(const std::string &srcImage, const std::string &dstImage, bool isRename) {

//     std::string photobuf;
//     std::ifstream in;
//     std::ostringstream tmp;
//     in.open(srcImage, std::ios::in | std::ios::binary | std::ios::ate);
//     if (!in.is_open()) {
//         return false;
//     }

//     in.seekg(0, std::ios_base::beg);
//     tmp << in.rdbuf();
//     photobuf = tmp.str();
//     in.close();

//     SDL_RWops * rw = SDL_RWFromMem((void *)photobuf.c_str(), photobuf.length());
//     if (IMG_isJPG(rw)) {
//         SDL_Surface * surface = IMG_LoadJPG_RW(rw);
//         IMG_SavePNG(surface, dstImage.c_str());
//         SDL_FreeSurface(surface);
//         remove(srcImage.c_str());
//     } else if (isRename) {
//         ::rename(srcImage.c_str(),dstImage.c_str());
//     }

//     SDL_FreeRW(rw);
//     return true;
// }

int32_t createRandNum(int32_t min,int32_t range) {
    if (range == 0) {
        return min;
    }

    srand(time(nullptr));
    return rand() % range + min;
}

int32_t createRandNum(int32_t offset) {
    if (offset == 0) {
        return 0;
    }

    srand(time(nullptr));
    return rand() % offset;
}

float fahrenheit2Celsius(float fahrenheit) {

    float result = (fahrenheit - 32)*5/9;

    return  result;
}

float celsius2Fahrenheit(float celsius) {

    float fahrenheit = celsius * 9 / 5 + 32;

    return fahrenheit;
}

void replaceAllString(std::string& base,const std::string& strSrc,const std::string& des) {

    if (base.empty()) {
        return;
    }

    std::string::size_type pos = 0;
    std::string::size_type srcLen = strSrc.size();
    std::string::size_type desLen = des.size();
    pos = base.find(strSrc, pos);
    while ((pos != std::string::npos)) {
        base.replace(pos, srcLen, des);
        pos = base.find(strSrc, (pos + desLen));
    }
}

std::string toString(float value, int precision) {
    char strValue[64] = {0};

    snprintf(strValue, sizeof(strValue), "%.*f", precision, value);
    return strValue;
}

float round(float value,int32_t precision) {

    if (precision < 0) {
        return 0.0;
    }

    int32_t flag = value < 0? 1:-1;
    float srcValue = flag * value;

    float preValue = pow(10.0, precision);
    int32_t tempValue = (int32_t)((srcValue + 0.5 / preValue) * preValue);
    return flag * ((float)tempValue / preValue);
}

static unsigned char charToHex(unsigned char x) {
    return (unsigned char) (x > 9 ? x + 55 : x + 48);
}

static bool isAlphaNumber(unsigned char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
        return true;
    return false;
}

std::vector<std::string> splitString(std::string str,std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;//扩展字符串以方便操作
    int size = str.size();

    for(int i = 0; i < size; i++) {
        pos=str.find(pattern,i);
        if(pos < size) {
            std::string s = str.substr(i,pos-i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}

bool floatVecToJsonString(const std::vector<float> &feature, std::string& value) {

    Json::Value jsonFeature;
    int32_t featureSize = feature.size();
    jsonFeature.resize(0);

    for (int32_t index = 0; index < featureSize; ++index) {
        Uint32Float val = {.floatValue = feature[index]};
        jsonFeature.append(Json::Value(val.uInt32Value));
    }

    value = jsonToString(jsonFeature);
    return true;
}


}
}