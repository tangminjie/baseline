#include "curl/curl.h"
#include "Logger/Define.h"
#include "HttpClient/HttpClient.h"

namespace Uface {
namespace Application {

typedef struct {
    char*       memory;
    size_t      size;
} MemoryStruct;

typedef struct {
    FILE*           file;
    OnDownloadCb    downloadCb;
} DownloadStruct;

static size_t downloadFileCallback(void *ptr, size_t size, size_t nmemb, void *use);
static size_t httpCustomCallback(void *contents, size_t size, size_t nmemb, void *userp);

int64_t HttpClient::getFileLength(const std::string &url) {

    CURLcode returnCode = CURLE_OK;
    CURL* pCurl = curl_easy_init();

    ///设置连接超时时间
    returnCode = curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(pCurl, CURLOPT_HEADER, 1);    //只需要header头
    curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1);    //不需要body

    ///https 去除证书校验
    curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 0L);

    double downloadFileLength = 0;

    returnCode = curl_easy_perform(pCurl);
    if (returnCode == CURLE_OK) {
        curl_easy_getinfo(pCurl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLength);
    } else {
        downloadFileLength = -1;
    }

    curl_easy_cleanup(pCurl);

    return (int64_t)downloadFileLength;
}

int32_t HttpClient::downloadFile(const std::string &url, const std::string &file,OnDownloadCb cb, int32_t timeout) {

    FILE* fp = fopen(file.c_str(), "w");
    if (fp == nullptr) {
        return -1;
    }

    DownloadStruct download;
    memset(&download,0x00, sizeof(DownloadStruct));
    download.file = fp;
    download.downloadCb = cb;

    CURLcode returnCode = CURLE_OK;
    CURL* pCurl = curl_easy_init();

    /**增加下载中长时间没有数据的处理逻辑问题*/
    curl_easy_setopt( pCurl, CURLOPT_LOW_SPEED_LIMIT, 1 ); //bytes
    curl_easy_setopt( pCurl, CURLOPT_LOW_SPEED_TIME, 10 ); //s

    curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, timeout);

    /// enable TCP keep-alive for this transfer
    curl_easy_setopt(pCurl, CURLOPT_TCP_KEEPALIVE, 1L);

    /// keep-alive idle time to 120 seconds
    curl_easy_setopt(pCurl, CURLOPT_TCP_KEEPIDLE, 120L);

    /// interval time between keep-alive probes: 60 seconds
    curl_easy_setopt(pCurl, CURLOPT_TCP_KEEPINTVL, 60L);

    ///当HTTP返回值大于等于400的时候，请求失败, 404错误
    curl_easy_setopt(pCurl, CURLOPT_FAILONERROR, 1L);

    curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);

    ///设置连接超时时间
    returnCode = curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 10);
    ///设置请求链接
    returnCode = curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
    ///设置CURLOPT_WRITEFUNCTION的最后一个参数值
    returnCode = curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)&download);
    ///设置接收到数据之后的回调函数
    returnCode = curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, downloadFileCallback);

    ///https 去除证书校验
    curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 0L);

    ///开始传输
    int32_t responseCode = 0;
    returnCode = curl_easy_perform(pCurl);
    if (returnCode == CURLE_OK) {
        //获取返回信息
        returnCode = curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &responseCode);
    }

    curl_easy_cleanup(pCurl);
    ::fclose(fp);

    if (returnCode == CURLE_OK) {
        return 0;
    }

    errorf("download file remoteUrl[{}] localFilePath[{}] failed: {}, \n", url.c_str(), file.c_str(), curl_easy_strerror(returnCode));
    return -1;
}

int32_t HttpClient::postData(const std::string &url, const std::string &params, std::string &response, int32_t timeout) {


    MemoryStruct chunk;
    memset(&chunk,0x00, sizeof(MemoryStruct));

    chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    CURL *curl = curl_easy_init();
    if (nullptr == curl) {
        printf("get data curl_easy_init failed !!!\n");
        free(chunk.memory);
        return -1;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpCustomCallback);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

    CURLcode curlCode = curl_easy_perform(curl);
    if (curlCode != CURLE_OK) {
        errorf("post data curl_easy_perform() url:{}, failed: {}, \n", url.c_str(), curl_easy_strerror(curlCode));
        free(chunk.memory);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl); return -1;
    }
    curl_slist_free_all(headers);

    response = chunk.memory;
    curl_easy_cleanup(curl);
    free(chunk.memory);
    return 0;
}

int32_t HttpClient::postUpload(const std::string &url,const std::string &body,std::string &response, int32_t timeout) {

    MemoryStruct chunk;
    memset(&chunk,0x00, sizeof(MemoryStruct));

    chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    CURL *curl = curl_easy_init();
    if (nullptr == curl) {
        printf("get data curl_easy_init failed !!!\n");
        free(chunk.memory);
        return -1;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Expect:");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpCustomCallback);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

    CURLcode curlCode = curl_easy_perform(curl);
    if (curlCode != CURLE_OK) {
        errorf("post data curl_easy_perform() failed: {}, \n", curl_easy_strerror(curlCode));
        free(chunk.memory);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl); return -1;
    }

    curl_slist_free_all(headers);

    response = chunk.memory;
    curl_easy_cleanup(curl);
    free(chunk.memory);
    return 0;
}

int32_t HttpClient::getData(const std::string &url, std::string &response, int32_t timeout) {


    MemoryStruct chunk;
    memset(&chunk,0x00, sizeof(MemoryStruct));
    chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    CURL *curl = curl_easy_init();
    if (nullptr == curl) {
        printf("get data init curl failed !!!\n");
        free(chunk.memory);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpCustomCallback);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

    CURLcode curlCode = curl_easy_perform(curl);
    if (CURLE_OK != curlCode) {
        errorf("EasyHttpCurl::EasyHttpCurl_Heartbeat_GetRequest: curl_easy_perform() url:{}, failed: {}\n",url.c_str(), curl_easy_strerror(curlCode));
        curl_easy_cleanup(curl);
        free(chunk.memory);
        return -1;
    }

    response = chunk.memory;
    curl_easy_cleanup(curl);
    free(chunk.memory);

    return 0;
}

int32_t HttpClient::reqWithMethod(const std::string &url, const std::string &body, const std::string &method, std::string &response, int32_t timeout) {
    MemoryStruct chunk;
    memset(&chunk,0x00, sizeof(MemoryStruct));
    chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    CURL *curl = curl_easy_init();
    if (nullptr == curl) {
        printf("get data init curl failed !!!\n");
        free(chunk.memory);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpCustomCallback);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

    CURLcode curlCode = curl_easy_perform(curl);
    if (CURLE_OK != curlCode) {
        errorf("EasyHttpCurl::EasyHttpCurl_Heartbeat_GetRequest: curl_easy_perform() url:{}, failed: {}\n",url.c_str(), curl_easy_strerror(curlCode));
        curl_easy_cleanup(curl);
        free(chunk.memory);
        return -1;
    }

    response = chunk.memory;
    curl_easy_cleanup(curl);
    free(chunk.memory);

    return 0;
}

static size_t downloadFileCallback(void *ptr, size_t size, size_t nmemb, void *use) {

    DownloadStruct* download = reinterpret_cast<DownloadStruct*>(use);
    ::fwrite(ptr, size, nmemb, download->file);

    int64_t dataSize = size * nmemb;

    if (download->downloadCb) {
        download->downloadCb(dataSize);
    }

    return dataSize;
}

static size_t httpCustomCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = reinterpret_cast<MemoryStruct *>(userp);

    char *ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == nullptr) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

}
}
