# baseline
## this is a face detect mvp
This is a tangminjie provaction projiect 
to 3-1-2023
for tangminjie 



### 开始

X86：
`cd build` and `cmake ..` and `make -j`
编译可执行文件

可能遇到的编译问题：
1. X86/lib 系统库和工程链接的lib库不匹配：
    解决方案：使用/usr/lib/x86_64-linux-gnu下的库替换链接库，或者重新编译对应的版本
2. opecv 版本不匹配
    查看工程库版本和系统安装的opencv版本，理论上工程库集成了opencv不会冲突
3. ldd查看动态库链接not fund
    解决方案：使用export LD_LIBRARY_PATH= 指定库的路径，只在当前终端有效，如需要永久有效请修改系统path

### 算法性能测试：
处理器：Intel® Core™ i7-
安装内存(RAM)：16.0GB 
系统类型：ubuntu
分辨率：1080p

算法 性能(ms)
FaceDetect 10-40 (人脸数量直接影响)
FeatureExtract 70-120（单次特征提取）
faceInfo 5-20 （人脸数量影响）
FeatureCompare < 0.061
RGB Liveness < 150
IR Liveness < 30

第一次测试发现 人脸特征提取耗时严重平均单个人脸在100ms左右，不能满足场景需求。

问题：因为算法库的局限性detect算子只能在单线程中跑，算法handle并不支持多线程。

解决方案：人脸特征提取，只需要人脸坐标 不依赖detect算子，可以创建多个算法handle 批量进行人脸特征提取。

