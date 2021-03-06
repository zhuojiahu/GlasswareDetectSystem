/************************************************************************/
/* Copyright (c) 2006, 中国大恒集团北京图像视觉技术分公司研发二部     
/* All rights reserved.													
/*																		
/* 文件名称： DHGrabberForGM.h												
/* 文件标识： 见配置管理计划书
/*
/* 摘要： 导出采集卡类
/* 当前版本： 3.0.0.2 beta 2
/* 修改者：郭文佳
/* 修改内容摘要：将头文件中DC参数命名改为GM。
/* 完成日期： 2010年06月10日
/*
/* 当前版本： 3.0.0.2 beta 1
/* 修改者：郭文佳
/* 修改内容摘要：增加相机闪光灯控制。
/* 完成日期： 2010年06月10日
/*
/* 当前版本： 3.0.0.1 beta 3
/* 修改者：郭文佳
/* 修改内容摘要：修改DHGrabberForGM.h中宏定义，改为DHGrabber_h_。
/* 完成日期： 2010年01月11日
/*
/* 当前版本： 3.0.0.1 beta 2
/* 修改者：郭文佳
/* 修改内容摘要：重新编译采集卡动态库。
/* 完成日期： 2010年01月05日
/*
/* 当前版本： 3.0.0.1 beta 1
/* 修改者：郭文佳
/* 修改内容摘要：初始化结构体及采集回调结构体加入“采集卡类型编号”。
/* 完成日期： 2009年12月25日
/*
/* 当前版本： 1.0
/* 作者： 郭文佳
/* 完成日期： 2009年12月03日
/************************************************************************/

#if !defined(DHGrabber_h_)
#define DHGrabber_h_

const int GBMaxFileNameLen = 255;
const int GBMaxTextLen = 255;

/*
采集卡类型编号定义如下
0:模拟采集卡
1:DALSA X64采集卡
2:AVT相机
3:千兆网相机
4:公司CG300模拟采集卡
5:公司1394相机
6:EURESYS 采集卡*/

typedef struct _GBSignalInfoStruct//回调信息结构
{
	PVOID Context;								//存放拥有回调函数的对象的this指针
	int iGrabberTypeSN;							// 采集卡类型编号
	int nGrabberSN;								//采集卡序号
	int nErrorCode;								//错误代码（0：正常）
	int nFrameCount;							//已采集的帧记数
	char strDescription[GBMaxTextLen];			//描述信息

	int iReserve1;								// 备用字段1
	int iReserve2;								// 备用字段2
	
}s_GBSIGNALINFO;//回调信息结构

//回调函数指针声明
typedef void (WINAPI *PGBCALLBACK)(const s_GBSIGNALINFO* SignalInfo);

typedef struct _GBInitStruct//采集卡初始化结构
{
	int iGrabberTypeSN;						// 采集卡类型编号
	int nGrabberSN;							//采集卡序号
    char strDeviceName[GBMaxTextLen];		//设备名称
	char strDeviceMark[GBMaxTextLen];		// 设备标识
	char strGrabberFile[GBMaxFileNameLen];	//用于初始化的文件
	PGBCALLBACK CallBackFunc;				//回调函数指针
	PVOID Context;							//存放拥有回调函数的对象的this指针

	int iReserve1;							// 备用字段1
	int iReserve2;							// 备用字段2

}s_GBINITSTRUCT;//采集卡初始化结构

typedef struct _GBErrorInfoStruct//错误信息结构
{
   	int  nErrorCode;						//错误代码（0：正常）
	char strErrorDescription[GBMaxTextLen];	//错误描述
	char strErrorRemark[GBMaxTextLen];		//附加信息

}s_GBERRORINFO;//错误信息结构

//公共参数
enum GBParamID
{
	GBImageWidth = 0,		//图像宽度 (单位:字节)
	GBImageHeight,			//图像高度 (单位:字节)
	GBImagePixelSize,		//图像象素大小 (单位:字节)
	GBImageBufferSize,		//图像缓冲区大小 (单位:字节)
	GBImageBufferAddr,		//图像缓冲区地址
	GBGrabberTypeSN			//采集卡类型编号
};

class CGrabber
{
//操作
public:
	CGrabber()
	{
	}
	virtual ~CGrabber()
	{
	}
	
	//初始化
	virtual BOOL Init(const s_GBINITSTRUCT* pInitParam) = 0;

	//关闭
	virtual BOOL Close() = 0;

	//开始采集
	virtual BOOL StartGrab() = 0;

	//停止采集
	virtual BOOL StopGrab() = 0;
	
	//单帧采集
	virtual BOOL Snapshot() = 0;	
	
	//设置参数
	virtual BOOL SetParamInt(GBParamID Param, int nInputVal) = 0;
	
	//得到参数
	virtual BOOL GetParamInt(GBParamID Param, int &nOutputVal) = 0;
	
	//调用参数对话框
	virtual void CallParamDialog() = 0;
	
	//得到出错信息
	virtual void GetLastErrorInfo(s_GBERRORINFO *pErrorInfo) = 0;
};

#endif// !defined(Grabber_h_)

#if !defined(DHGrabberGM_h_)
#define DHGrabberGM_h_

//////////////////////////////////////////////////////////////////////////
//
//公司相机专有参数
enum GMParamID
{
	GMSnapMode = 0,			// 采集模式
	GMShutterUnit,			// 曝光时间单位
	GMShutterTime,			// 曝光时间
	GMGain,					// 增益
	GMStrobeOnOff,			// 闪光灯开关
	GMStrobeTrigger,		// 闪光灯信号极性
	GMAutoStrobeOnOff		// 自动闪光灯开关
};

//错误代码定义
enum GMErrorStruct
{
	GMErrorNoFindCam = 0,		// 没有找到相机
	GMErrorGrabberSN,			// 相机序号超出能够找到的相机数
	GMErrorGetCamInfo,			// 获取相机信息失败
	GMErrorValOverFlow,			// 数据溢出
	GMErrorOpenCam,				// 打开相机失败
	GMErrorInit,				// 初始化失败
	GMErrorStartGrab,			// 开始采集失败
	GMErrorStopGrab,			// 停止采集失败
	GMErrorClose,				// 关闭失败
	GMErrorSendBufAdd,			// 获取图像内存地址失败
	GMErrorGetParam,			// 非法获取参数
	GMErrorSetParam,			// 非法设置参数
	GMErrorParamDlg,			// 调用参数设置对话框失败
	GMErrorReadIniFile,			// 读取配置文件失败
	GMErrorSaveCamSetting,		// 保存相机参数组失败
	GMErrorLoadCamSetting,		// 加载相机参数组失败
	GMErrorStartupCamSetting,	// 设置相机启动参数组失败
	GMErrorGetCurCamSetting		// 得到相机当前使用的参数组失败
};

//相机参数配置模式
enum CamMemSet
{
	Manufacturer = 0,
	UserSet1,
	UserSet2,
	UserSet3
};
//
//////////////////////////////////////////////////////////////////////////

//采集类
class _declspec(dllexport) CDHGrabberGM : public CGrabber
{
//操作
public:
	CDHGrabberGM();
	virtual ~CDHGrabberGM();
	
	//初始化
	BOOL Init(const s_GBINITSTRUCT* pInitParam);

	//关闭
	BOOL Close();

	//开始采集
	BOOL StartGrab();

	//停止采集
	BOOL StopGrab();

	//单帧采集
	BOOL Snapshot();

	//设置参数
	BOOL SetParamInt(GBParamID Param, int nInputVal);
	
	//得到参数
	BOOL GetParamInt(GBParamID Param, int &nOutputVal);

	//调用参数对话框
	void CallParamDialog();

	//得到出错信息
	void GetLastErrorInfo(s_GBERRORINFO *pErrorInfo);

	//////////////////////////////////////////////////////////////////////////
	// 千兆网相机专有接口

	//设置参数
	BOOL GMSetParamInt(GMParamID Param, int nInputVal);
	
	//得到参数
	BOOL GMGetParamInt(GMParamID Param, int &nOutputVal);

	//保存相机参数组设置
	BOOL SaveCameraSetting(CamMemSet Param);
	
	//加载相机参数组设置
	BOOL LoadCameraSetting(CamMemSet Param);

	//设置相机启动参数组
	BOOL SetStartupCameraSetting(CamMemSet Param);

	//
	//////////////////////////////////////////////////////////////////////////
	
//属性
protected:
	CGrabber *m_pGrabber;
};

#endif// !defined(DHGrabberGM_h_)
