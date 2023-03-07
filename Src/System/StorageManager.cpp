

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <dirent.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include "Logger/Define.h"
#include "Infra/Time/Time.h"
#include "System/StorageManager.h"

namespace ArcFace {
namespace Application {

StorageManager* StorageManager::instance() {
    static StorageManager storageManager;
    return &storageManager;
}

bool StorageManager::getDataSpace(const char *dir, int32_t &totoal, int32_t &free) {
    struct statfs diskInfo;
    statfs(dir, &diskInfo);
    unsigned long long totalBlocks = diskInfo.f_bsize;
    unsigned long long totalSize = totalBlocks * diskInfo.f_blocks;
    totoal = totalSize >> 20;
    unsigned long long freeDisk = diskInfo.f_bfree*totalBlocks;
    free = freeDisk >> 20;

    return true;
}

bool StorageManager::getFileNode(const char *dir, int64_t &total, int64_t &free) {

    struct statfs diskInfo;
    statfs(dir, &diskInfo);

    total = diskInfo.f_files;
    free   = diskInfo.f_ffree;
    return true;
}

int64_t StorageManager::getFileSpace(const char *dir) {

    DIR *dp;
    struct dirent *entry = nullptr;
    struct stat statbuf;
    long long int totalSize = 0;

    if ((dp = ::opendir(dir)) == nullptr) {
        fprintf(stderr, "open dir: %s faild\n", dir);
        return -1;
    }

    //先加上自身目录的大小
    lstat(dir, &statbuf);
    totalSize += statbuf.st_size;

    while ((entry = readdir(dp)) != nullptr) {

        char subdir[256] = {0};
        sprintf(subdir, "%s/%s", dir, entry->d_name);
        lstat(subdir, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) {
            if (strncmp(".", entry->d_name,1) == 0 ||
                strncmp("..", entry->d_name,2) == 0) {
                continue;
            }

            totalSize += getFileSpace(subdir);
        } else {
            totalSize += statbuf.st_size;
        }
    }

    ::closedir(dp);
    return totalSize;
}

bool StorageManager::makeDir(const char *dir) {

    char dirName[256] = {0};
    int32_t length = strlen(dir);
    strncpy(dirName,dir,length);
    mode_t mode = umask(0);/*设备默认屏蔽位077，此处将屏蔽位置0，避免权限不足*/
    for (int32_t index = 0; index < length; ++index) {
        if (index != 0 && dirName[index] == '/') {
            dirName[index] = 0;
            if (::access(dirName, F_OK) != 0) {
                if (mkdir(dirName,0777) != 0) {
                    umask(mode);
                    return false;
                }
            }

            dirName[index] = '/';
        }
    }

    if (length > 0 && ::access(dir, F_OK) != 0) {
        mkdir(dir,0777);
    }
    umask(mode);
    chmod(dirName, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);
    return true;
}

bool StorageManager::fileIsExist(const char *file) {
    return access(file, F_OK) == 0;
}

bool StorageManager::fileCopy(const char *srcFile, const char *dstFile) {

    FILE* src = fopen(srcFile,"r+");
    FILE* dst = fopen(dstFile,"w+");

    if (src == nullptr || dst == nullptr) {
        if (src != nullptr) fclose(src);
        if (dst != nullptr) fclose(dst);

        return false;
    }

    char buffer[1024] = {0};
    int32_t readLength = 0;
    readLength = fread(buffer, 1, sizeof(buffer), src);
    while (readLength > 0) {

        fwrite(buffer,1,readLength,dst);
        readLength = fread(buffer, 1, sizeof(buffer), src);
    }

    fclose(src);
    fclose(dst);

    return true;
}

bool StorageManager::fileRead(const char *file, char *buffer, int32_t *length) {

    memset(buffer,0x00,*length);
    FILE* fp = fopen(file,"r+");
    if (fp == nullptr) {
        return false;
    }

    int32_t readN = ::fread(buffer,1,*length,fp);
    *length = readN;
    fclose(fp);
    return true;
}

std::string StorageManager::getFileContent(const char *file) {

    std::string audioResource;
    std::ifstream in;
    in.open(file,std::ifstream::in);
    if (!in.is_open()) {
        return audioResource;
    }

    in.seekg(0,std::ios_base::end);
    if (in.tellg() < (2 << 20)) {
        in.seekg(0,std::ios_base::beg);
        std::ostringstream tempStream;
        tempStream << in.rdbuf();
        audioResource = tempStream.str();
    }

    in.close();
    return audioResource;
}

bool StorageManager::fileIsEmpty(const char *file) {
    FILE* fp = fopen(file,"rb");
    if (fp == nullptr) {
        return false;
    } else {
        fseek(fp,0,SEEK_END);
        int64_t size = ftell(fp);
        fclose(fp);
        return size <= 0;
    }
}

std::vector<std::string> StorageManager::splitData(const std::string &content, char split) {

    std::vector<std::string> result;
    std::stringstream ss(content);
    std::string item;
    while (getline(ss,item,split)) {
        result.push_back(item);
    }

    return result;
}

bool StorageManager::saveFile(const char *file, const std::string &content, bool sync) {

    std::ofstream streamFile(file,std::ios::binary);
    streamFile << content;
    streamFile.close();

    if (sync) {
        StorageManager::instance()->syncFile();
    }

    return true;
}

bool StorageManager::saveImage(const std::string &file, const uint8_t *content, int32_t length) {
    std::fstream fileStream;
    fileStream.open(file.c_str(),std::ios_base::out|std::ios_base::trunc);
    if (!fileStream.is_open()) {
        errorf("save image file(%s) failed",file.c_str());
        return false;
    }

    fileStream.write((char*)content,length);
    fileStream.close();
    mode_t mode = S_IRUSR | S_IRGRP | S_IROTH;
    chmod(file.c_str(), mode);

    return true;
}

std::string StorageManager::createFileName(const char *path, const char *name, const char *postFix) {

    char fileDir[512] = {0};
    Infra::CTime time = Infra::CTime::getCurrentTime();

    snprintf(fileDir, sizeof(fileDir) - 1,"%s/%04d-%02d-%02d/%02d",path,time.year,time.month,time.day,time.hour);
    if (!makeDir(fileDir)) {
        errorf("create dir(%s) failed......",fileDir);
        return "";
    }

    char fileName[1024] = {0};
    if (name == nullptr) {
        if (postFix == nullptr) {
            snprintf(fileName, sizeof(fileName) - 1,"%s/%02d%02d%02d_%03d",
                     fileDir,time.hour,time.minute,time.second,time.millisecond);
        } else {
            snprintf(fileName, sizeof(fileName) - 1,"%s/%02d%02d%02d_%03d.%s",
                     fileDir,time.hour,time.minute,time.second,time.millisecond,postFix);
        }

    } else {
        if (postFix != nullptr) {
            snprintf(fileName, sizeof(fileName) - 1,"%s/%02d%02d%02d_%03d_%s.%s",
                     fileDir,time.hour,time.minute,time.second,time.millisecond,name,postFix);
        } else {
            snprintf(fileName, sizeof(fileName) - 1,"%s/%02d%02d%02d_%03d_%s",
                     fileDir,time.hour,time.minute,time.second,time.millisecond,name);
        }
    }

    return fileName;
}

void StorageManager::removeFile(const char *file) {
    tracef("remove file %s",file);
    if (file == nullptr || !fileIsExist(file)) {
        return;
    }
    
    remove(file);
}

void StorageManager::unlinkFile(const std::string &file) {

    if (file.empty() || !fileIsExist(file.c_str())) {
        return;
    }

    unlink(file.c_str());
}

int StorageManager::removeDir(const char *dir) {

    char curDir[] = ".";
    char upDir[] = "..";
    char dirName[1024] = {0};
    
    DIR *dirp = nullptr;
    struct dirent *dp = nullptr;
    struct stat dir_stat;

    /**参数传递进来的目录不存在，直接返回*/
    if (dir == nullptr ||  0 != access(dir, F_OK) ) {
        return 0;
    }

    /**获取目录属性失败，返回错误*/
    if ( 0 > stat(dir, &dir_stat) ) {
        errorf("get directory stat error");
        return -1;
    }

    if (S_ISREG(dir_stat.st_mode) ) {
        /**普通文件直接删除*/
        remove(dir);
    } else if ( S_ISDIR(dir_stat.st_mode) ) {
        /**目录文件，递归删除目录中内容*/
        dirp = opendir(dir);
         while ( (dp = readdir(dirp)) != nullptr ) {
            /** 忽略.和..*/
            if ((0 == strcmp(curDir, dp->d_name)) || (0 == strcmp(upDir, dp->d_name)) ) {
                continue;
            }

            snprintf(dirName, sizeof(dirName), "%s/%s", dir, dp->d_name);
            /**递归调用*/
            removeDir(dirName);
        }

        closedir(dirp);
        rmdir(dir);

    } else {
        errorf("unknown file type!");
    }

    return 0;
}

std::vector<std::string> StorageManager::getSubDirent(const std::string &dir) {

    std::vector<std::string> subDir;
    if (dir.empty()) {
        return subDir;
    }

    DIR* dp = nullptr;
    if ((dp = opendir(dir.c_str())) == nullptr) {
        return subDir;
    }

    struct dirent* result = nullptr;

    while (result = readdir(dp)) {
        if (result == nullptr) {
            break;
        }

        if ((result->d_type & DT_DIR) == 0) {
            continue;
        }

        if (strncmp(result->d_name,".",1) == 0 || strncmp(result->d_name,"..",2) == 0) {
            continue;
        }

        subDir.emplace_back(result->d_name);
    }

    closedir(dp);
    return subDir;
}

std::vector<std::string> StorageManager::getSubFile(const std::string &dir) {

    std::vector<std::string> subFile;
    if (dir.empty()) {
        return subFile;
    }

    DIR* dp = nullptr;
    if ((dp = opendir(dir.c_str())) == nullptr) {
        return subFile;
    }

 
    struct dirent* result = nullptr;

    while (result = readdir(dp)) {
        if (result == nullptr) {
            break;
        }

        if ((result->d_type & DT_REG) == 0) {
            continue;
        }

        if (strncmp(result->d_name,".",1) == 0 || strncmp(result->d_name,"..",2) == 0) {
            continue;
        }

        subFile.emplace_back(result->d_name);
    }

    closedir(dp);
    return subFile;
}

void StorageManager::syncFile() {
    sync();
}

}
}
