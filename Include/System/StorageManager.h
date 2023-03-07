

#ifndef INC_5C_APP_SYSTEM_STORAGEMANAGER_H
#define INC_5C_APP_SYSTEM_STORAGEMANAGER_H

#include <string>
#include <vector>
#include "Infra/Define.h"

namespace ArcFace {
namespace Application {

class StorageManager {

public:

    static StorageManager* instance();

public:
    /**
     * @brief 获取空间总容量剩余容量
     * @param[in] dir
     * @param[out] totoal
     * @param[out] free
     * @return
     */
    bool getDataSpace(const char *dir, int32_t &totoal, int32_t &free);
    /**
     * @brief 获取空间下文件节点数
     * @param[in] dir
     * @param[out] total
     * @param[out] free
     * @return
     */
    bool getFileNode(const char *dir, int64_t &total, int64_t &free);
    /**
     * @brief 获取空间下文件大小
     * @param dir
     * @return
     */
    int64_t getFileSpace(const char *dir);
    /**
     * @brief 创建目录
     * @param[in] dir  目录
     * @return
     */
    bool makeDir(const char* dir);
    /**
     * @brief 文件是否存在
     * @param[in] file  文件名
     * @return
     */
    bool fileIsExist(const char* file);
    /**
     * @brief 文件拷贝
     * @param[in] srcFile   源文件
     * @param[in] dstFile   目标文件
     * @return
     */
    bool fileCopy(const char* srcFile,const char* dstFile);
    /**
     * @brief 读取文件内容
     * @param file
     * @param buffer
     * @param length
     * @return
     */
    bool fileRead(const char* file,char* buffer,int32_t* length);
    /**
     * @brief 获取文件内容,有最大字节限制
     * @param file
     * @return
     */
    std::string getFileContent(const char* file);
    /**
     * @brief 文件是否为空
     * @param file
     * @return
     */
    bool fileIsEmpty(const char* file);
    /**
     * @brief 字符串分割
     * @param content
     * @param split
     * @return
     */
    std::vector<std::string> splitData(const std::string& content,char split);
    /**
     * @brief 文件保存
     * @param file
     * @param content
     * @return
     */
    bool saveFile(const char *file, const std::string &content, bool sync);
    /**
     * @brief 图片保存
     * @param path
     * @param content
     * @param length
     * @return
     */
    bool saveImage(const std::string &file, const uint8_t *content, int32_t length);
    /**
     * @brief 创建文件名
     * @notice 接口内部根据时间戳生产文件名，精度到毫秒，存在多处调用会产生一样的文件名，若要唯一需通过name做分区
     * @param name
     * @param type
     * @return
     */
    std::string createFileName(const char *path, const char *name, const char *postFix);
    /**
     * @brief 删除文件
     * @param[in] file
     */
    void removeFile(const char* file);
    /**
     * @brief unlink file
     * @param file
     */
    void unlinkFile(const std::string& file);
    /**
     * @brief 删除目录
     * @param[in] file
     */
    int removeDir(const char *dir);
    /**
     * @brief 获取子目录
     * @param dir
     * @return
     */
    std::vector<std::string> getSubDirent(const std::string& dir);
    /**
     * @brief 获取目录下的文件
     * @param dir
     * @return
     */
    std::vector<std::string> getSubFile(const std::string &dir);
    /**
     * @brief 同步操作
     */
    void syncFile();
};

}
}
#endif //INC_5C_APP_SYSTEM_STORAGEMANAGER_H
