
#ifndef INC_5C_APP_BASE_HTTPCLIENT_HTTPCLIENT_H
#define INC_5C_APP_BASE_HTTPCLIENT_HTTPCLIENT_H

#include <string>
#include "Infra/Define.h"

namespace ArcFace {
namespace Application {

typedef void (*OnDownloadCb)(int64_t dataLength);

class HttpClient {

public:
    /**
     * @brief 获取文件长度
     * @param url
     * @return
     */
    int64_t getFileLength(const std::string& url);
    /**
     * @brief 下载文件
     * @param[in] url       目标url
     * @param[in] file      下载的文件名
     * @param[in] cb        下载回调
     * @param[in] timeout   超时时间,单位s
     * @return
     */
    int32_t downloadFile(const std::string& url, const std::string& file,OnDownloadCb cb,int32_t timeout = 5);
    /**
     * @brief 向目标url 推送数据，并接受回复数据，表单
     * @param[in]  url      目标url
     * @param[in]  param    请求参数
     * @param[out] response 回复内容
     * @param[in]  timeout  超时时间,单位s,默认60s
     * @return
     */
    int32_t postData(const std::string &url, const std::string &body, std::string &response, int32_t timeout = 60);
    /**
     * @brief 向目标url 推送数据，并接受回复数据,body 格式为json
     * @param url
     * @param std
     * @param body
     * @param response
     * @param timeout
     * @return
     */
    int32_t postUpload(const std::string& url,const std::string& body,std::string& response,int32_t timeout=60);
    /**
     * @brief 从目标url 获取数据
     * @param[in]  url      目标url
     * @param[in]  params   post fields参数
     * @param[out] response 回复内容
     * @param[in]  timeout  超时时间,单位s,默认3s
     * @return
     */
    int32_t getData(const std::string &url, std::string &response, int32_t timeout);
    /**
     * @brief 向目标url指定方法 推送数据，并接受回复数据,body 格式为json
     * @param url
     * @param body
     * @param method Http方法(GET，POST， PUT)
     * @param response
     * @param timeout
     * @return
     */
    int32_t reqWithMethod(const std::string &url, const std::string &body, const std::string &method, std::string &response, int32_t timeout=60);
};

}
}
#endif //INC_5C_APP_BASE_HTTPCLIENT_HTTPCLIENT_H
