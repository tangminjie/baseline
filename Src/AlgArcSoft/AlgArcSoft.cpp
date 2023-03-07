#include "AlgArcSoft/AlgArcSoft.h"
#include "Logger/Define.h"

namespace Uface {
namespace Application {

IAlgArcSoft* IAlgArcSoft::instance() {
    return CAlgArcSoftImpl::getInstance();
}

CAlgArcSoftImpl* CAlgArcSoftImpl::getInstance() {
    static CAlgArcSoftImpl algArcSoft;
    return &algArcSoft;
}

CAlgArcSoftImpl::CAlgArcSoftImpl() {
}

CAlgArcSoftImpl::~CAlgArcSoftImpl() {
    this->Revinitial();
}

bool CAlgArcSoftImpl::initial(){

      //初始化算法文件
    infof("\n************* ArcFace SDK Info *****************\n");
	MRESULT res = MOK;
	ASF_ActiveFileInfo activeFileInfo = { 0 };
	res = ASFGetActiveFileInfo(&activeFileInfo);
	if (res != MOK)
	{
		infof("ASFGetActiveFileInfo fail:{}\n", res);
		return false;
	}
	else
	{
		//这里仅获取了有效期时间，还需要其他信息直接打印即可
		char startDateTime[32];
		timestampToTime(activeFileInfo.startTime, startDateTime, 32);
		infof("startTime: %s\n", startDateTime);
		char endDateTime[32];
		timestampToTime(activeFileInfo.endTime, endDateTime, 32);
		infof("endTime: %s\n", endDateTime);
	}

	//SDK版本信息
	const ASF_VERSION version = ASFGetVersion();
	infof("\nVersion:%s\n", version.Version);
	infof("BuildDate:%s\n", version.BuildDate);
	infof("CopyRight:%s\n", version.CopyRight);

	infof("\n************* Face Recognition *****************\n");

	//在线激活激活后无需重复调用
	// res = ASFOnlineActivation(APPID, SDKKEY);
	// if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res){
	// 	infof("ASFOnlineActivation fail: {}\n", res);
	// 	return false;
	// }
	// else
	// 	infof("ASFOnlineActivation sucess: {}\n", res);

	//初始化引擎
	MInt32 mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS | ASF_IR_LIVENESS;
	res = ASFInitEngine(ASF_DETECT_MODE_VIDEO, ASF_OP_ALL_OUT, NSCALE, FACENUM, mask, &handle);
	if (res != MOK){
		infof("ASFInitEngine fail: {}\n", res);
		return false;
	}
	else
		infof("ASFInitEngine sucess: {}\n", res);

	return true;
}

bool CAlgArcSoftImpl::Revinitial(){
    //反初始化
	bool res = ASFUninitEngine(handle);
	if (res != MOK)
		errorf("ASFUninitEngine fail: {}\n", res);
}

bool CAlgArcSoftImpl::getDetectSingleFrameFaceData(ASVLOFFSCREEN &offscreen,DetectSingleFrameFaceData &detectSingleFrameFaceData){

    if(handle == NULL){
        errorf("alg handle not init!\n");
        return false;
    }
	//测试算法模块时间
    auto start_time = std::chrono::high_resolution_clock::now();
    ASF_MultiFaceInfo detectedFaces = { 0 };
    //该接口依赖初始化的模式选择，VIDEO模式下调用的人脸追踪功能，IMAGE模式下调用的人脸检测功能。
    MRESULT res = ASFDetectFacesEx(handle, &offscreen, &detectedFaces);
    if (res != MOK && detectedFaces.faceNum <= 0)
    {
        errorf("ASFDetectFaces 1 fail: %d , faceNum == 0\n", res);
        return false;
    }
	infof("image1 face num is {}\n",detectedFaces.faceNum);
    //打印一下算法耗时
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << "face detect time: " << duration.count() << "ms" << std::endl;
    //将每一帧的人脸数据存起来
    detectSingleFrameFaceData.faceNum = detectedFaces.faceNum;
	if(detectedFaces.faceNum > 0){
		//处理检测到的人脸数组并且保存到vector里
		int nums =  detectDataProcess(offscreen,detectedFaces,detectSingleFrameFaceData.DetectSingleFaceDataVector);
		if(nums < 0){
			// printf("detectDataProcess 1 fail: %d , faceNum == 0\n", nums);
			errorf("detectDataProcess error nums is {}\n",nums);
		}
	}

    return true;
}

//时间戳转换为日期格式
void CAlgArcSoftImpl::timestampToTime(char* timeStamp, char* dateTime, int dateTimeSize)
{
	time_t tTimeStamp = atoll(timeStamp);
	struct tm* pTm = gmtime(&tTimeStamp);
	strftime(dateTime, dateTimeSize, "%Y-%m-%d %H:%M:%S", pTm);
}

int CAlgArcSoftImpl::detectDataProcess(ASVLOFFSCREEN &offscreen,
ASF_MultiFaceInfo &detectedFaces,std::vector<DetectSingleFaceData> &DetectSingleFaceDataVector
) {
	int faceNum = detectedFaces.faceNum;
	auto start_time = std::chrono::high_resolution_clock::now();
	// 人脸信息检测
	MInt32 processMask = ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS;
	bool res = ASFProcessEx(handle, &offscreen, &detectedFaces, processMask);
	if (res != MOK)
		printf("ASFProcessEx fail: %d\n", res);

	// 获取年龄
	ASF_AgeInfo ageInfo = { 0 };
	res = ASFGetAge(handle, &ageInfo);

	// 获取性别
	ASF_GenderInfo genderInfo = { 0 };
	res = ASFGetGender(handle, &genderInfo);
	//打印一下算法耗时
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << "face info time: " << duration.count() << "ms" << std::endl;

	// // 获取3D角度
	// ASF_Face3DAngle angleInfo = { 0 };
	// res = ASFGetFace3DAngle(handle, &angleInfo);

	for(uint i=0;i < faceNum; i++){
		DetectSingleFaceData detectSingleFaceData = {0};
		ASF_SingleFaceInfo SingleDetectedFace = { 0 };
		ASF_FaceFeature feature = { 0 };

		detectSingleFaceData.faceID =  detectedFaces.faceID[i];
		SingleDetectedFace.faceRect.left = detectedFaces.faceRect[i].left;
		SingleDetectedFace.faceRect.top = detectedFaces.faceRect[i].top;
		SingleDetectedFace.faceRect.right = detectedFaces.faceRect[i].right;
		SingleDetectedFace.faceRect.bottom = detectedFaces.faceRect[i].bottom;
		SingleDetectedFace.faceOrient = detectedFaces.faceOrient[i];
		detectSingleFaceData.singleDetectedFace = SingleDetectedFace;
		//判断一下要不要给人脸
		if(ageInfo.num == faceNum) {
			detectSingleFaceData.age =  ageInfo.ageArray[i];
		}

		if(genderInfo.num == faceNum){
			detectSingleFaceData.gender =  genderInfo.genderArray[i];
		}
		auto start_time_extract = std::chrono::high_resolution_clock::now();
		// 单人脸特征提取
		int res = ASFFaceFeatureExtractEx(handle, &offscreen, &SingleDetectedFace, &feature);
		if (res != MOK)
		{
			printf("ASFFaceFeatureExtractEx 1 fail: %d\n", res);
		}
		else{
			detectSingleFaceData.feature = feature;
		}
		//打印一下算法耗时
		auto end_time_extract = std::chrono::high_resolution_clock::now();
		auto duration_extract = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_extract - start_time_extract);
		std::cout << "face extract time: " << duration_extract.count() << "ms" << std::endl;

		//给到vector里储存
		DetectSingleFaceDataVector.push_back(detectSingleFaceData);
	}

	return faceNum;
}



}
}