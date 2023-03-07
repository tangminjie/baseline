# baseline
## this is a face detect mvp
This is a tangminjie provaction projiect 
to 3-1-2023
for tangminjie 



### 开始

X86：
1.创建build目录 
`mkdir build`

2.执行编译
`cd build` and `cmake ..` and `make -j`
编译可执行文件

可能遇到的编译问题：
1. X86/lib 系统库和工程链接的lib库不匹配：
    解决方案：使用/usr/lib/x86_64-linux-gnu下的库替换链接库，或者重新编译对应的版本
2. opecv 版本不匹配
    查看工程库版本和系统安装的opencv版本，理论上工程库集成了opencv不会冲突
3. ldd查看动态库链接not fund
    解决方案：使用export LD_LIBRARY_PATH= 指定库的路径，只在当前终端有效，如需要永久有效请修改系统path

#### 工程目录说明：

|--- build 编译路径
|--- Include 头文件路径
|--- Lib 动态库路径 如opencv等
|--- Src 源文件路径，入口函数在/src/applocation
|--- CMakeList.txt cmake 描述文件

#### 算法库相关说明：
|---include/algArcSoft
| |---amcomdef.h 平台文件
| |---asvloffscreen.h 平台文件
| |---arcsoft_face_sdk.h 接口文件
| |---merror.h 错误码文件

|---lib/algArc
|---|---linux_x64
| |---|---libarcsoft_face.so 算法库
| |---|---libarcsoft_face_engine.so 引擎库

### 算法性能测试：
处理器：Intel® Core™ i7- 
安装内存(RAM)：16.0GB  
系统类型：ubuntu 
分辨率：1080p 

算法 性能(ms)

| 算子模块 | 耗时 | 说明 |
| --- | --- | --- |
| FaceDetect | 10-40 | 人脸数量直接影响 |
| FeatureExtract | 70-120 | 单次特征提取 |
| faceInfo | 5-20 | 提取信息人脸数量影响 |
| RGB Liveness | < 150 | RGB活体检测 |
| IR Liveness  | < 30 | 官方数据 |

第一次测试发现 人脸特征提取耗时严重平均单个人脸在100ms左右，码流FPS为 25帧/s 不能满足场景需求。所以我们不能全帧率进行特征提取，需要对特征提取做一个筛选逻辑。

##### 特征提取方案：
对于实际应用场景并不需要对每一帧的人脸进行特征提取，算法性能也达不到。所以在这个mvp中我们使用以下方案：
1. 对trackid进行跟踪，记录trackid产生的时间戳并且持续更新，当一定时间之外trackid没有更新表示人离开了，或者id丢失，将数据丢弃。
2. 对人脸图像进行筛选，因为ARCAlg没有提供图片质量或者人脸质量的接口，在这里使用3D DANGLE作为人脸图片质量判定的标准。
3. 对于人脸特征提取的逻辑，在间隔一秒中对人脸图片进行筛选进行人脸特征提取进行比对，当比对成功后，不在进行对比，当比对失败后继续间隔1s进行对比，直到失败3次。
4. 当trackid判断丢失后，这个人脸没有提取过特征，在现有的数据中筛选最优的图片进行特征提取。

性能优化点：
测试发现算法耗时主要集中在特征提取中，考虑优化这一点。

问题：因为算法库的局限性detect算子只能在单线程中跑，算法handle并不支持多线程。

解决方案：人脸特征提取，只需要人脸坐标 不依赖detect算子，可以创建多个算法handle 批量进行人脸特征提取。

