﻿#include "glasswaredetectsystem.h"
#include <Mmsystem.h>
#pragma comment( lib,"winmm.lib" )
#include <QTextCodec>
#include <QDateTime>
#include <QPropertyAnimation>

#include "EquipRuntime.h"
//是否加密宏开关 JIAMI_INITIA
//#define  JIAMI_INITIA
#ifdef JIAMI_INITIA
#ifdef WIN_32_1_10
#include "ProgramLicense.h"
#include "pushLicense.h"
CProgramLicense m_ProgramLicense;
#pragma comment( lib,"DHProgramLicense.lib" )
#else
#include "ProgramLicense.h"
#include "pushLicense.h"
CProgramLicense m_ProgramLicense;
#pragma comment( lib,"DHProgramLicense64.lib" )
#endif
#endif

GlasswareDetectSystem * pMainFrm;
QString appPath;  //更新路径
DWORD GlasswareDetectSystem::SendIOCard(void* param)
{
	while(!pMainFrm->m_bIsThreadDead)
	{
		if(pMainFrm->ncSocketWriteData.count()>0)
		{
			QByteArray nTest = pMainFrm->ncSocketWriteData.first();
			pMainFrm->ncSocketWriteData.removeFirst();
			pMainFrm->m_tcpSocket->write(nTest.data(),nTest.size());
		}
		Sleep(20);
	}
	return TRUE;
}
DWORD GlasswareDetectSystem::SendDetect(void* param)
{
	int lastNumber = 0;
	int nSignalNo = 0 ;
	int nlastNumber = 0;
	int nCurrentNum = 0;
	char *m_reportPtr = new char[256*sizeof(MyErrorType)+sizeof(MyStruct)];
	memset(m_reportPtr,0,256*sizeof(MyErrorType)+sizeof(MyStruct));
	MyStruct nTempStruct;
	memset(&nTempStruct,0,sizeof(MyStruct));
	char* nTPtr;
	if(pMainFrm->m_sSystemInfo.m_iSystemType == 1)
	{
		nTempStruct.nUnit = LEADING;
	}else if(pMainFrm->m_sSystemInfo.m_iSystemType == 2)
	{
		nTempStruct.nUnit = CLAMPING;
	}else if(pMainFrm->m_sSystemInfo.m_iSystemType == 3)
	{
		nTempStruct.nUnit = BACKING;
	}
	MyErrorType nCheckSendData[256]={0};
	
	while(!pMainFrm->m_bIsThreadDead)
	{
		if(pMainFrm->m_sRunningInfo.m_bCheck && pMainFrm->m_sSystemInfo.m_bIsIOCardOK)
		{
			nCurrentNum = pMainFrm->m_vIOCard[0]->ReadCounter(3);
			if(nCurrentNum != lastNumber)
			{
				nSignalNo= pMainFrm->m_vIOCard[0]->ReadCounter(26); //根据预先保存的图像号来找对应的缺陷
				lastNumber = nCurrentNum;
				if(nSignalNo != nlastNumber)//排除加减速造成的图像号一致问题
				{
					nlastNumber = nSignalNo;
					nCheckSendData[pMainFrm->nCountNumber] = pMainFrm->nSendData[nSignalNo];
					pMainFrm->nSendData[nSignalNo].id = 0;
					pMainFrm->nSendData[nSignalNo].nType = 0;
					pMainFrm->nSendData[nSignalNo].nErrorArea = 0;
					pMainFrm->nCountNumber++;
					if(pMainFrm->nCountNumber == 256)//发送256个数据到服务器进行汇总
					{
						pMainFrm->nCountNumber = 0;
						nTempStruct.nState = SENDDATA;
						nTempStruct.nCount = 256;
						memcpy(m_reportPtr,&nTempStruct,sizeof(MyStruct));
						nTPtr = m_reportPtr;
						nTPtr+=sizeof(MyStruct);
						memcpy(nTPtr,&nCheckSendData,256*sizeof(MyErrorType));
						memset(&nCheckSendData,0,256*sizeof(MyErrorType));
						QByteArray ba(m_reportPtr, 256*sizeof(MyErrorType)+sizeof(MyStruct));
						pMainFrm->nSocketMutex.lock();
						pMainFrm->ncSocketWriteData.push_back(ba);
						pMainFrm->nSocketMutex.unlock();
					}
				}
			}
		}
		Sleep(1);
	}
	return TRUE;
}
GlasswareDetectSystem::GlasswareDetectSystem(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent)
{
	pMainFrm = this;
	for (int i=0;i<CAMERA_MAX_COUNT;i++)
	{
		nQueue[i].listDetect.clear();
		pdetthread[i] = NULL;
		m_SavePicture[i].pThat=NULL;
		m_sRunningInfo.m_bIsCheck[i] = TRUE;
		pHandles[i] = CreateEvent(NULL,FALSE,NULL,NULL);
	}
	m_vcolorTable.clear();
	for (int i = 0; i < 256; i++)  
	{  
		nSendData[i].id = 0;
		nSendData[i].nErrorArea = 0;
		nSendData[i].nType = 0;
		m_vcolorTable.append(qRgb(i, i, i)); 
	}
	//m_eCurrentMainPage = CarveSettingPage;
	CherkerAry.pCheckerlist=NULL;
	surplusDays=0;

	//网络通信初始化
	n_EndTime = 0;
	nLastCheckNum = 0;
	nLastFailedNum = 0;
	nCountNumber = 0;
	nIOCard = new int[24];
	memset(nIOCard,0,24*sizeof(int));
	m_ptr = new char[24*sizeof(int)+sizeof(MyStruct)];
	memset(m_ptr ,0,24*sizeof(int)+sizeof(MyStruct));
}
QString GlasswareDetectSystem::getVersion(QString strFullName)
{
	QString SysType;
	if(m_sSystemInfo.m_iSystemType == 1)
	{
		SysType = QString(tr("UpDown"));
	}else if(m_sSystemInfo.m_iSystemType == 2)
	{
		SysType = QString(tr("ClampDown"));
	}else if(m_sSystemInfo.m_iSystemType == 3)
	{
		SysType = QString(tr("GoDown"));
	}
	return SysType + QString(tr("Version:")+"5.64.1.0");
}
GlasswareDetectSystem::~GlasswareDetectSystem()
{
	delete m_ptr;
	delete nIOCard;
}
void GlasswareDetectSystem::Init()
{
	InitParameter();
	ReadIniInformation();
	Initialize();
	initInterface();
	StartDetectThread();
	StartCamGrab();
	InitLastData();
}
//初始化
void GlasswareDetectSystem::Initialize()
{
	LoadParameterAndCam();
	InitImage();
	InitIOCard();
	InitCheckSet();
	initDetectThread();
}
//采集回调函数
void WINAPI GlobalGrabOverCallback (const s_GBSIGNALINFO* SigInfo)
{
	if (SigInfo && SigInfo->Context)
	{
		GlasswareDetectSystem* pGlasswareDetectSystem = (GlasswareDetectSystem*) SigInfo->Context;
		pGlasswareDetectSystem->GrabCallBack(SigInfo);
	}
}
//采集回调函数
void GlasswareDetectSystem::GrabCallBack(const s_GBSIGNALINFO *SigInfo)
{
	int iRealCameraSN = SigInfo->nGrabberSN;
	if (iRealCameraSN==-1 || !m_sRunningInfo.m_bCheck)
	{
		return;
	}
	if(SigInfo->nErrorCode != GBOK)
	{
		s_GBERRORINFO ErrorInfo;
		m_sRealCamInfo[iRealCameraSN].m_pGrabber->GetLastErrorInfo(&ErrorInfo);
		QString str = QString((QString("Camera:%1,")+QString("Error code:%2,")+QString("Error description:%3,")+QString("Additional information:%4")).arg(iRealCameraSN+1).arg(ErrorInfo.nErrorCode).arg(ErrorInfo.strErrorDescription).arg(ErrorInfo.strErrorRemark));		
		Logfile.write(QString("GrabCallBack:") + str ,CheckLog);
		return;
	}
	int imgNumber = 0;
	int tempI = 0;
	if (m_sSystemInfo.m_bIsIOCardOK)
	{
		//每次回调尝试读取两个工位的图像号，通过是否重复来判断是第几工位拍图，再放到对应的检测队列，就没法判断误触发的情况
		if(m_sSystemInfo.m_bIsTest)
		{
			//tempI = m_sCarvedCamInfo[widget_carveSetting->iCameraNo].m_iStress;
		}else{
			for(;tempI<2;tempI++)
			{
				if(tempI == 0)
				{
					imgNumber = ReadImageSignal(struGrabCardPara[iRealCameraSN].iReserve1,iRealCameraSN);
				}else{
					imgNumber = ReadImageSignal(struGrabCardPara[iRealCameraSN].iReserve2,iRealCameraSN);
				}
				if(m_sRealCamInfo[iRealCameraSN].m_iImageIdxLast[tempI] == imgNumber || imgNumber < 0)
				{
					continue;
				}else{
					m_sRealCamInfo[iRealCameraSN].m_iImageIdxLast[tempI] = imgNumber;
					break;
				}
			}
		}
	}else
	{
		if(m_sCarvedCamInfo[widget_carveSetting->iCameraNo].m_iStress)
		{
			tempI = 0;
		}else{
			tempI = 1;
		}
		
		m_sRealCamInfo[iRealCameraSN].m_iGrabImageCount++;
		imgNumber = m_sRealCamInfo[iRealCameraSN].m_iImageIdxLast[0];
		if(m_sRealCamInfo[iRealCameraSN].m_iGrabImageCount%2 == 0)
		{
			imgNumber = m_sRealCamInfo[iRealCameraSN].m_iImageIdxLast[0]++;
		}
 		if (imgNumber >= 256)
 		{
 			imgNumber = 0;
 		}
	}
	//******************采集:得到图像缓冲区地址****************************//
	int tempCamera = iRealCameraSN;
	uchar* pImageBuffer = NULL;
	int nAddr = 0;
	int nWidth, nHeight;
	mutexDetectElement[iRealCameraSN].lock();
	m_sRealCamInfo[iRealCameraSN].m_pGrabber->GetParamInt(GBImageBufferAddr, nAddr);
	pImageBuffer = (uchar*)nAddr;
	m_sRealCamInfo[iRealCameraSN].m_pGrabber->GetParamInt(GBImageWidth, nWidth);
	m_sRealCamInfo[iRealCameraSN].m_pGrabber->GetParamInt(GBImageHeight, nHeight);
	//int nAddr2;
	//__int64 lAddr, lAddr2, lAddr3;
	//if (m_sRealCamInfo[iRealCameraSN].m_pGrabber->GetParamInt(GBImageBufferAddr2, nAddr2)) //只有sg加了该功能
	//{
	//	lAddr  = (__int64)nAddr & 0xFFFFFFFF;
	//	lAddr2 = ((__int64)nAddr2) << 32;
	//	lAddr3 = lAddr | lAddr2;
	//	pImageBuffer = (uchar*)lAddr3;
	//}
	//else
	//{
	//	pImageBuffer = (uchar*)nAddr;
	//}
	mutexDetectElement[iRealCameraSN].unlock();

	if (m_sSystemInfo.m_bIsIOCardOK)
	{
		if(m_sSystemInfo.m_bIsTest)
		{
			if(widget_carveSetting->iCameraNo >= m_sSystemInfo.iRealCamCount)
			{
				tempCamera += m_sSystemInfo.iRealCamCount;
			}
		}else{
			if(tempI == 1 && m_sSystemInfo.m_iSystemType != 2)
			{
				tempCamera += m_sSystemInfo.iRealCamCount;
			}
		}
		/*m_sRealCamInfo[tempCamera].m_iGrabImageCount++;
		if(m_sRealCamInfo[tempCamera].m_iGrabImageCount>2)
		{
			return;
		}*/
	}else{
		if(m_sRealCamInfo[iRealCameraSN].m_iGrabImageCount%2 == 0)
		{
			if(m_sSystemInfo.m_iSystemType != 2)
			{
				tempCamera += m_sSystemInfo.iRealCamCount;
			}
		}
	}
	
	CGrabElement *pGrabElement = NULL;

	if(nQueue[tempCamera].listGrab.count()>0)
	{
		pGrabElement = (CGrabElement *) nQueue[tempCamera].listGrab.first();
		nQueue[tempCamera].listGrab.removeFirst();
		memcpy(pGrabElement->SourceImage->bits(),pImageBuffer,nWidth*nHeight);
		pGrabElement->bHaveImage=TRUE;
		pGrabElement->nCheckRet = FALSE;
		pGrabElement->cErrorParaList.clear();
		pGrabElement->nWidth = nWidth;
		pGrabElement->nHeight = nHeight;
		pGrabElement->nSignalNo = imgNumber;
		pGrabElement->nCamSN = tempCamera;

		if (nQueue[tempCamera].InitID == pGrabElement->initID)
		{
			m_detectElement[tempCamera].ImageNormal = pGrabElement;
			m_detectElement[tempCamera].iCameraNormal = tempCamera;
			if(1 == m_sCarvedCamInfo[tempCamera].m_iStress)
			{
				m_detectElement[tempCamera].iType = 1;
			}else{
				m_detectElement[tempCamera].iType = 0;
			}
			nQueue[tempCamera].mDetectLocker.lock();
			nQueue[tempCamera].listDetect.append(m_detectElement[tempCamera]);
			nQueue[tempCamera].mDetectLocker.unlock();
		}
		else
		{
			delete pGrabElement;
		}
	}
}
//配置初始化信息
void GlasswareDetectSystem::InitParameter()
{
	// 注册s_MSGBoxInfo至元对象系统,否则s_MSGBoxInfo,s_ImgWidgetShowInfo，s_StatisticsInfo无法作为参数进行传递
	qRegisterMetaType<s_MSGBoxInfo>("s_MSGBoxInfo"); 
	qRegisterMetaType<e_SaveLogType>("e_SaveLogType");  
	qRegisterMetaType<QList<QRect>>("QList<QRect>");   
	//初始化路径
	QString path = QApplication::applicationFilePath();  
	appPath = path.left(path.findRev("/")+1);
	m_sConfigInfo.m_strAppPath = appPath;
	//获取文件版本号
	sVersion = getVersion(path);
	//配置文件在run目录中位置
	m_sConfigInfo.m_strConfigPath = "Config/Config.ini";
	m_sConfigInfo.m_strDataPath = "Config/Data.ini";
	if (sLanguage == 0)
	{
		m_sConfigInfo.m_strErrorTypePath = "Config/ErrorType.ini";
	}else{
		m_sConfigInfo.m_strErrorTypePath = "Config/ErrorType-en.ini";
	}
	m_sConfigInfo.m_strPLCStatusTypePath = "Config/PLCAlertType.ini";
	m_sConfigInfo.m_sAlgFilePath = "ModelInfo";// 算法路径 [10/26/2010 GZ]	
	m_sConfigInfo.m_sRuntimePath = "Config/runtime.ini";

	//配置文件绝对路径
	m_sConfigInfo.m_strConfigPath = appPath + m_sConfigInfo.m_strConfigPath;
	m_sConfigInfo.m_strDataPath = appPath + m_sConfigInfo.m_strDataPath;
	m_sConfigInfo.m_strErrorTypePath = appPath + m_sConfigInfo.m_strErrorTypePath;
	m_sConfigInfo.m_strPLCStatusTypePath = appPath + m_sConfigInfo.m_strPLCStatusTypePath;
	m_sConfigInfo.m_sAlgFilePath = appPath + m_sConfigInfo.m_sAlgFilePath;
	m_sConfigInfo.m_sRuntimePath = appPath + m_sConfigInfo.m_sRuntimePath;

	SaveDataPath=appPath+"./LastData.ini";
	
	//初始化相机参数
	for (int i = 0;i<CAMERA_MAX_COUNT;i++)
	{
		m_sRealCamInfo[i].m_bGrabIsStart = FALSE;
	}
	m_tcpSocket = new QTcpSocket();
	m_tcpSocket->connectToHost("192.168.250.204",8088);
	//m_tcpSocket->connectToHost("127.0.0.1",8088);
	m_tcpSocket->waitForConnected(3000);
	connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(onServerDataReady()));
}
void GlasswareDetectSystem::onServerDataReady()
{
	QTcpSocket* nTcpSocket = dynamic_cast<QTcpSocket*>(sender());
	QByteArray buffer = nTcpSocket->readAll();
	char* t_ptr = buffer.data();
	if(buffer.size()==sizeof(MyStruct) && ((MyStruct*)t_ptr)->nState == CLEAR)
	{
		widget_carveSetting->errorList_widget->slots_clearTable();
		m_sRunningInfo.nGSoap_ErrorTypeCount[0]=0;
		m_sRunningInfo.nGSoap_ErrorCamCount[0]=0;
		m_sRunningInfo.nGSoap_ErrorTypeCount[2]=0;
		m_sRunningInfo.nGSoap_ErrorCamCount[2]=0;
		m_sRunningInfo.m_checkedNum = 0;
		m_sRunningInfo.m_passNum = 0;
		m_sRunningInfo.m_kickoutNumber = 0;
		m_sRunningInfo.m_failureNumFromIOcard = 0;
		test_widget->nInfo.m_checkedNum = 0;
		test_widget->nInfo.m_checkedNum2 = 0;
		test_widget->nInfo.m_passNum = 0;
		test_widget->nInfo.m_failureNum = 0;
		nLastCheckNum=0;
		nLastFailedNum = 0;
		QString nClearName = QString(((MyStruct*)t_ptr)->nTemp);
		if(nClearName == "Clear")
		{
			m_vIOCard[0]->m_Pio24b.softReset();
			if(m_sSystemInfo.m_iSystemType == 2)
			{
				test_widget->m_vIOCard->m_Pio24b.softReset();
			}
			QSettings iniDataSet(m_sConfigInfo.m_strDataPath,QSettings::IniFormat);
			iniDataSet.setIniCodec(QTextCodec::codecForName("GBK"));
			QString strSession;
			strSession=QString("/system/checkedNum");
			iniDataSet.setValue(strSession,m_sRunningInfo.m_checkedNum);

			strSession = QString("/system/failureNum");
			iniDataSet.setValue(strSession,m_sRunningInfo.m_failureNumFromIOcard);
			nCountNumber = 0;
		}
	}else if(((MyStruct*)t_ptr)->nState == IMAGE)
	{
		
	}else if(buffer.size()==sizeof(MyStruct) && ((MyStruct*)t_ptr)->nState == CONNECT)
	{

	}else if(((MyStruct*)t_ptr)->nState == SEVERS)
	{
		QString nClearName = QString(((MyStruct*)t_ptr)->nTemp);
		if(nClearName == "NULL")
		{
			nWidgetWarning->hideWarnning();
		}else{
			QString nAddModeName = QString::fromLocal8Bit(((MyStruct*)t_ptr)->nTemp);//报警的错误信息
			nWidgetWarning->showWarnning(nAddModeName);
		}
	}else if(buffer.size()==sizeof(MyStruct) && ((MyStruct*)t_ptr)->nState == SYSTEMMODEADD)
	{
		if(m_sRunningInfo.m_bCheck)//如果是开始检测和开始调试中则自动关闭
		{
			slots_OnBtnStar();
		}
		if(m_sSystemInfo.m_bIsTest)
		{
			widget_carveSetting->widgetCarveImage->slots_startTest();
		}
		//创建新的模板函数
		QString nAddModeName = QString::fromLocal8Bit(((MyStruct*)t_ptr)->nTemp);
		widget_Management->SeverAdd(nAddModeName);
	}else if(buffer.size()==sizeof(MyStruct) && ((MyStruct*)t_ptr)->nState == SYSTEMMODESELECT)
	{
		if(m_sRunningInfo.m_bCheck)//如果是开始检测和开始调试中则自动关闭
		{
			slots_OnBtnStar();
		}
		if(m_sSystemInfo.m_bIsTest)
		{
			widget_carveSetting->widgetCarveImage->slots_startTest();
		}
		//创建新的模板函数
		QString nAddModeName = QString::fromLocal8Bit(((MyStruct*)t_ptr)->nTemp);
		widget_Management->SeverSelect(nAddModeName);
	}else if(buffer.size()==sizeof(MyStruct) && ((MyStruct*)t_ptr)->nState == SYSTEMMODEDELTE)
	{
		if(m_sRunningInfo.m_bCheck)//如果是开始检测和开始调试中则自动关闭
		{
			slots_OnBtnStar();
		}
		if(m_sSystemInfo.m_bIsTest)
		{
			widget_carveSetting->widgetCarveImage->slots_startTest();
		}
		//创建新的模板函数
		QString nAddModeName = QString::fromLocal8Bit(((MyStruct*)t_ptr)->nTemp);
		widget_Management->SeverDelete(nAddModeName);
	}
}

//读取配置信息
void GlasswareDetectSystem::ReadIniInformation()
{
	QSettings iniset(m_sConfigInfo.m_strConfigPath,QSettings::IniFormat);
	iniset.setIniCodec(QTextCodec::codecForName("GBK"));
	QSettings erroriniset(m_sConfigInfo.m_strErrorTypePath,QSettings::IniFormat);
	erroriniset.setIniCodec(QTextCodec::codecForName("GBK"));
	QSettings PLCStatusiniset(m_sConfigInfo.m_strPLCStatusTypePath,QSettings::IniFormat);
	PLCStatusiniset.setIniCodec(QTextCodec::codecForName("GBK"));
	QSettings iniDataSet(m_sConfigInfo.m_strDataPath,QSettings::IniFormat);
	iniDataSet.setIniCodec(QTextCodec::codecForName("GBK"));

	QSettings runtimeCfg(pMainFrm->m_sConfigInfo.m_sRuntimePath,QSettings::IniFormat);
	runtimeCfg.setIniCodec(QTextCodec::codecForName("GBK"));

	QString strSession;
	strSession = QString("/ErrorType/total");
	m_sErrorInfo.m_iErrorTypeCount = erroriniset.value(strSession,0).toInt();

	for (int i=0;i<=m_sErrorInfo.m_iErrorTypeCount;i++)
	{
		if (0 == i)
		{
			m_sErrorInfo.m_bErrorType[i] = false;
			m_sErrorInfo.m_vstrErrorType.append(tr("Good"));//.toLatin1().data()));
		}
		else
		{
			m_sErrorInfo.m_bErrorType[i] = true;
			strSession = QString("/ErrorType/%1").arg(i);
			m_sErrorInfo.m_vstrErrorType.append(QString::fromLocal8Bit(erroriniset.value(strSession,"NULL").toString()));//.toLatin1().data()));
			m_sErrorInfo.m_cErrorReject.iErrorCountByType[i] = 0;
		}
	}
	m_sErrorInfo.m_vstrErrorType.append(tr("Unknown Defect"));//.toLatin1().data()));

	strSession = QString("/StatusType/total");
	int  StatusTypeNumber= PLCStatusiniset.value(strSession,0).toInt();
	for (int i=1;i<=StatusTypeNumber;i++)
	{
		strSession = QString("/StatusType/%1").arg(i);
		m_vstrPLCInfoType.append(QString::fromLocal8Bit(PLCStatusiniset.value(strSession,"NULL").toString()));//.toLatin1().data()));
	}
	//读取系统参数
	m_sRunningInfo.m_failureNumFromIOcard = iniDataSet.value("/system/failureNum",0).toInt();	
	m_sRunningInfo.m_checkedNum = iniDataSet.value("/system/checkedNum",0).toInt();	
	m_sSystemInfo.IsCarve = iniset.value("/system/IsCarve",false).toBool();
	m_sSystemInfo.nLoginHoldTime = iniset.value("/system/nLoginHoldTime",10).toInt();	//是否报警统计
	m_sSystemInfo.m_strWindowTitle = QObject::tr("Glass Bottle Detect System");//读取系统标题
	m_sSystemInfo.m_iTest = iniset.value("/system/Test",0).toInt();
	m_sSystemInfo.m_bDebugMode = iniset.value("/system/DebugMode",0).toInt();	//读取是否debug
	m_sSystemInfo.m_iSystemType = iniset.value("/system/systemType",0).toInt();	//读取系统类型

	m_sSystemInfo.m_bIsIOCardOK=iniset.value("/system/isUseIOCard",1).toInt();	//是否使用IO卡
	m_sSystemInfo.m_bIsStopNeedPermission=iniset.value("/system/IsStopPermission",0).toBool();	//是否使用IO卡
	m_sSystemInfo.iIOCardCount=iniset.value("/system/iIOCardCount",1).toInt();	//读取IO卡个数
	m_sSystemInfo.iRealCamCount = iniset.value("/GarbCardParameter/DeviceNum",2).toInt();	//真实相机个数
	
	m_sSystemInfo.m_bIsTest = iniset.value("/system/IsTest",0).toInt();//是否是测试模式
	m_sSystemInfo.iIsButtomStress = iniset.value("/system/IsButtomStree",0).toInt();//是否有瓶底应力
	m_sSystemInfo.iIsSaveCountInfoByTime = iniset.value("/system/IsSaveCountInfoByTime",0).toInt();//是否保存指定时间段内的统计信息
	m_sSystemInfo.iIsSample = iniset.value("/system/IsSample",0).toInt();//是否取样
	m_sSystemInfo.iIsCameraCount = iniset.value("/system/IsCameraCount",0).toInt();//是否统计各相机踢废率
	m_sSystemInfo.LastModelName = iniset.value("/system/LastModelName","default").toString();	//读取上次使用模板
	m_sSystemInfo.m_iIsTrackStatistics = iniset.value("/system/isTrackStatistics",0).toInt();	//是否报警统计
	m_sSystemInfo.m_iTrackNumber = iniset.value("/system/TrackNumber",10).toInt();	//报警统计个数
	m_sSystemInfo.m_NoKickIfNoFind = iniset.value("/system/NoKickIfNoFind",0).toInt();	//报警类型
	m_sSystemInfo.m_NoKickIfROIFail = iniset.value("/system/NoKickIfROIFail",0).toInt();	//报警类型	

	m_sSystemInfo.m_iImageStretch = iniset.value("/system/ImageStretch",1).toInt();	//图像横向排布还是上下排布
	m_sSystemInfo.m_iSaveNormalErrorImageByTime = iniset.value("/system/SaveNormalErrorImageByTime",0).toInt();	
	m_sSystemInfo.m_iSaveStressErrorImageByTime = iniset.value("/system/SaveStressErrorImageByTime",0).toInt();	
	m_sSystemInfo.m_iStopOnConveyorStoped = iniset.value("/system/stopCheckWhenConveyorStoped",0).toBool();	//输送带停止是否停止检测
	m_sSystemInfo.fPressScale = iniset.value("/system/fPressScale",1).toDouble();	//瓶身应力增强系数
	m_sSystemInfo.fBasePressScale = iniset.value("/system/fBasePressScale",1).toDouble();	//瓶底应力增强系数
	m_sSystemInfo.m_strModelName = m_sSystemInfo.LastModelName;
	m_sSystemInfo.bSaveRecord = iniset.value("/system/bSaveRecord",1).toBool();
	m_sSystemInfo.iSaveRecordInterval = iniset.value("/system/iSaveRecordInterval",60).toInt();
	m_sSystemInfo.bAutoSetZero = iniset.value("/system/bAutoSetZero",1).toBool();

	m_sSystemInfo.bCameraOffLineSurveillance = iniset.value("/system/bCameraOffLineSurveillance",0).toBool();	
	m_sSystemInfo.bCameraContinueRejectSurveillance = iniset.value("/system/bCameraContinueRejectSurveillance",0).toBool();	
	m_sSystemInfo.iCamOfflineNo = iniset.value("/system/iCamOfflineNo",10).toInt();	
	m_sSystemInfo.iCamContinueRejectNumber = iniset.value("/system/iCamContinueRejectNumber",10).toInt();	
	m_sSystemInfo.iWebServerPort = iniset.value("/system/iWebServerPort",8000).toInt();	
	m_sSystemInfo.iIfCountDetectNumberByCamera = iniset.value("/system/iIfCountDetectNumberByCamera",0).toInt();		

	for (int i=0;i<CAMERA_MAX_COUNT;i++)
	{
		strSession = QString("/NoRejectIfNoOrigin/Device_%1").arg(i+1);
		m_sSystemInfo.m_iNoRejectIfNoOrigin[i] = iniset.value(strSession,0).toInt();
		strSession = QString("/NoKickIfROIFail/Device_%1").arg(i+1);
		m_sSystemInfo.m_iNoRejectIfROIfail[i] = iniset.value(strSession,0).toInt();
		strSession = QString("/NoStaticIfNoOrigin/Device_%1").arg(i+1);
		m_sSystemInfo.m_iNoStaticIfNoOrigin[i] = iniset.value(strSession,0).toInt();
	}
	
	int iShift[3];
	iShift[0] = iniset.value("/system/shift1",000000).toInt();
	iShift[1] = iniset.value("/system/shift2",80000).toInt();
	iShift[2] = iniset.value("/system/shift3",160000).toInt();
	for (int i = 0; i<2; i++)
	{
		if (iShift[i] > iShift[i+1] )
		{
			int temp =iShift[i];
			iShift[i] = iShift[i+1];
			iShift[i+1] = temp;
		}
	}
	m_sSystemInfo.shift1.setHMS(iShift[0]/10000,(iShift[0]%10000)/100,iShift[0]%100);
	m_sSystemInfo.shift2.setHMS(iShift[1]/10000,(iShift[1]%10000)/100,iShift[1]%100);
	m_sSystemInfo.shift3.setHMS(iShift[2]/10000,(iShift[2]%10000)/100,iShift[2]%100);

	//读取报表查询历史参数
	iShift[0] = iniset.value("/system/Searchshift1",80000).toInt();
	iShift[1] = iniset.value("/system/Searchshift2",160000).toInt();
	iShift[2] = iniset.value("/system/Searchshift3",230000).toInt();
	for (int i = 0; i<2; i++)
	{
		if (iShift[i] > iShift[i+1] )
		{
			int temp =iShift[i];
			iShift[i] = iShift[i+1];
			iShift[i+1] = temp;
		}
	}
	m_sSystemInfo.SearchShift1.setHMS(iShift[0]/10000,(iShift[0]%10000)/100,iShift[0]%100);
	m_sSystemInfo.SearchShift2.setHMS(iShift[1]/10000,(iShift[1]%10000)/100,iShift[1]%100);
	m_sSystemInfo.SearchShift3.setHMS(iShift[2]/10000,(iShift[2]%10000)/100,iShift[2]%100);
	m_sSystemInfo.iSearchTimeFlag = iniset.value("/system/SearchTimeFlags",1).toInt();

	//设置剪切参数路径
	m_sConfigInfo.m_strGrabInfoPath = m_sConfigInfo.m_strAppPath + "ModelInfo/" + m_sSystemInfo.m_strModelName + "/GrabInfo.ini";

	//切割后相机个数
	m_sSystemInfo.iCamCount = iniset.value("/system/CarveDeviceCount",1).toInt();
	if(pMainFrm->m_sSystemInfo.m_iSystemType != 2)
	{
		if(m_sSystemInfo.iCamCount>m_sSystemInfo.iRealCamCount+m_sSystemInfo.iRealCamCount/2)
		{
			m_sSystemInfo.iCamCount = m_sSystemInfo.iRealCamCount+m_sSystemInfo.iRealCamCount/2;
		}
	}
	
	for (int i=0;i<m_sSystemInfo.iRealCamCount;i++)
	{
		struGrabCardPara[i].iGrabberTypeSN = 1;
		strSession = QString("/GarbCardParameter/Device%1ID").arg(i+1);
		struGrabCardPara[i].nGrabberSN = iniset.value(strSession,-1).toInt();
		strSession = QString("/GarbCardParameter/Device%1Name").arg(i+1);
		strcpy_s(struGrabCardPara[i].strDeviceName,iniset.value(strSession,"").toString().toLocal8Bit().data());
		strSession = QString("/GarbCardParameter/Device%1Mark").arg(i+1);
		strcpy_s(struGrabCardPara[i].strDeviceMark,iniset.value(strSession,"").toString().toLocal8Bit().data());
		QString strGrabInitFile;//存储相机初始化位置
		strSession = QString("/GarbCardParameter/Device%1InitFile").arg(i+1);
		strGrabInitFile = iniset.value(strSession,"").toString();

		strSession = QString("/GarbCardParameter/Device%1FristTrigger").arg(i+1);
		struGrabCardPara[i].iReserve1 = iniset.value(strSession,1).toInt();
		strSession = QString("/GarbCardParameter/Device%1SecondTrigger").arg(i+1);
		struGrabCardPara[i].iReserve2 = iniset.value(strSession,1).toInt();


		strSession = QString("/GarbCardParameter/Device%1Station").arg(i+1);
		m_sRealCamInfo[i].m_iGrabPosition = iniset.value(strSession,0).toInt();
		strSession = QString("/RoAngle/Device_%1").arg(i+1);
		m_sRealCamInfo[i].m_iImageRoAngle = iniset.value(strSession,0).toInt();
		strSession = QString("/ImageType/Device_%1").arg(i+1);
		m_sRealCamInfo[i].m_iImageType = iniset.value(strSession,0).toInt();
		strSession = QString("/IOCardSN/Device_%1").arg(i+1);
		m_sRealCamInfo[i].m_iIOCardSN = iniset.value(strSession,0).toInt();
		//采集卡文件路径与config所在文件夹相同
		strGrabInitFile = m_sConfigInfo.m_strConfigPath.left(m_sConfigInfo.m_strConfigPath.findRev("/")+1) + strGrabInitFile;
		memcpy(struGrabCardPara[i].strGrabberFile,strGrabInitFile.toLocal8Bit().data(),GBMaxTextLen);
	}

	QSettings iniCameraSet(m_sConfigInfo.m_strGrabInfoPath,QSettings::IniFormat);
	QString strShuter,strTrigger;
	for(int i = 0; i < m_sSystemInfo.iRealCamCount; i++)
	{
		strShuter = QString("/Shuter/Grab_%1").arg(i);
		strTrigger = QString("/Trigger/Grab_%1").arg(i);
		m_sRealCamInfo[i].m_iShuter=iniCameraSet.value(strShuter,20).toInt();
		m_sRealCamInfo[i].m_iTrigger=iniCameraSet.value(strTrigger,1).toInt();//默认外触发
	}
	sVersion = getVersion(NULL);
	//read Equipment maintenance Config
	m_sRuntimeInfo.isEnable = runtimeCfg.value("EquipAlarm/Enable",false).toBool();
	m_sRuntimeInfo.total = runtimeCfg.value("EquipAlarm/total",20).toInt();
	for (int i=0;i<m_sRuntimeInfo.total;i++)
	{
		m_sRuntimeInfo.AlarmsEnable << runtimeCfg.value(QString("EquipAlarm/Alarm%1_Enable").arg(i+1) , false ).toBool();
		m_sRuntimeInfo.AlarmsDays << runtimeCfg.value(QString("EquipAlarm/Alarm%1_Days").arg(i+1) , 0 ).toInt();
		m_sRuntimeInfo.AlarmsInfo << runtimeCfg.value(QString("EquipAlarm/Alarm%1_Info").arg(i+1) , "" ).toString();
	}
}
//读取切割信息
void GlasswareDetectSystem::ReadCorveConfig()
{
	QSettings iniCarveSet(m_sConfigInfo.m_strGrabInfoPath,QSettings::IniFormat);
	QString strSession;
	for(int i=0; i < pMainFrm->m_sSystemInfo.iCamCount; i++)
	{
		//加载剪切后参数
		strSession = QString("/angle/Grab_%1").arg(i);
		m_sCarvedCamInfo[i].m_iImageAngle = iniCarveSet.value(strSession,0).toInt();
		strSession = QString("/Stress/Device_%1").arg(i);
		m_sCarvedCamInfo[i].m_iStress = iniCarveSet.value(strSession,0).toInt();
		strSession = QString("/tonormal/Grab_%1").arg(i);
		m_sCarvedCamInfo[i].m_iToNormalCamera = iniCarveSet.value(strSession,i).toInt();
		strSession = QString("/pointx/Grab_%1").arg(i);
		m_sCarvedCamInfo[i].i_ImageX = iniCarveSet.value(strSession,i).toInt();
		strSession = QString("/pointy/Grab_%1").arg(i);
		m_sCarvedCamInfo[i].i_ImageY = iniCarveSet.value(strSession,i).toInt();
		strSession = QString("/width/Grab_%1").arg(i);
		m_sCarvedCamInfo[i].m_iImageWidth = iniCarveSet.value(strSession,i).toInt();
		strSession = QString("/height/Grab_%1").arg(i);
		m_sCarvedCamInfo[i].m_iImageHeight = iniCarveSet.value(strSession,i).toInt();
		strSession = QString("/convert/Grab_%1").arg(i);
		m_sCarvedCamInfo[i].m_iToRealCamera = iniCarveSet.value(strSession,i).toInt();

		if ((m_sCarvedCamInfo[i].i_ImageX + m_sCarvedCamInfo[i].m_iImageWidth) > m_sRealCamInfo[i].m_iImageWidth)
		{
			m_sCarvedCamInfo[i].i_ImageX = 0;
			m_sCarvedCamInfo[i].m_iImageWidth = m_sRealCamInfo[i].m_iImageWidth;
		}
		if ((m_sCarvedCamInfo[i].i_ImageY + m_sCarvedCamInfo[i].m_iImageHeight) > m_sRealCamInfo[i].m_iImageHeight)
		{
			m_sCarvedCamInfo[i].i_ImageY = 0;
			m_sCarvedCamInfo[i].m_iImageHeight = m_sRealCamInfo[i].m_iImageHeight;
		}
	}
}
//加载参数和相机
void GlasswareDetectSystem::LoadParameterAndCam()
{
	for (int i=0;i<m_sSystemInfo.iRealCamCount;i++)
	{
		//回调
		struGrabCardPara[i].CallBackFunc = GlobalGrabOverCallback;
		struGrabCardPara[i].Context = this;
		//初始化采集卡
		InitGrabCard(struGrabCardPara[i],i);
	}
}
//初始化采集卡（：初始化相机）
void GlasswareDetectSystem::InitGrabCard(s_GBINITSTRUCT struGrabCardPara,int index)
{
	QString strDeviceName = QString(struGrabCardPara.strDeviceName);
	if (strDeviceName=="SimulaGrab")
	{
		m_sRealCamInfo[index].m_pGrabber = new CDHGrabberSG;
		m_sRealCamInfo[index].m_bSmuGrabber = true;
		m_sRealCamInfo[index].m_iGrabType = 0;
	}
	else if (strDeviceName == "MER")
	{
		m_sRealCamInfo[index].m_pGrabber = new CDHGrabberMER;
		m_sRealCamInfo[index].m_bSmuGrabber = false;
		m_sRealCamInfo[index].m_iGrabType = 8;
	}
	BOOL bRet = FALSE;
	try
	{
		bRet = m_sRealCamInfo[index].m_pGrabber->Init(&struGrabCardPara);	
		if(bRet)
		{
			m_sRealCamInfo[index].m_bCameraInitSuccess=TRUE;
			bRet = m_sRealCamInfo[index].m_pGrabber->GetParamInt(GBImageWidth, m_sRealCamInfo[index].m_iImageWidth);
			if(bRet)
			{
				bRet = m_sRealCamInfo[index].m_pGrabber->GetParamInt(GBImageHeight, m_sRealCamInfo[index].m_iImageHeight);
				if(bRet)
				{
					bRet = m_sRealCamInfo[index].m_pGrabber->GetParamInt(GBImageBufferSize, m_sRealCamInfo[index].m_iImageSize);	
					if(bRet)
					{
						int nImagePixelSize = 0;
						bRet = m_sRealCamInfo[index].m_pGrabber->GetParamInt(GBImagePixelSize, nImagePixelSize);
						if(bRet)
						{
							int result=0;
							bRet = m_sRealCamInfo[index].m_pGrabber->GetParamInt(GBImageBufferAddr, result);
							if(bRet)
							{
								m_sRealCamInfo[index].m_iImageBitCount =8* nImagePixelSize;
								if(strDeviceName == "MER")
								{
									((CDHGrabberMER*)m_sRealCamInfo[index].m_pGrabber)->MERSetParamInt(MERSnapMode,1);
								}
							}
						}
					}
				}
			}		
		}
	}
	catch (...)
	{
		QString strError;
		strError = QString("catch camera%1 initial error").arg(index);
		m_sRealCamInfo[index].m_bCameraInitSuccess = FALSE;
	}

	if (bRet)
	{
		InitCam();
		m_sRealCamInfo[index].m_bCameraInitSuccess = TRUE;
		if(m_sSystemInfo.m_iSystemType != 2)
		{
			m_sRealCamInfo[index+m_sSystemInfo.iRealCamCount].m_bCameraInitSuccess = m_sRealCamInfo[index].m_bCameraInitSuccess;
			m_sRealCamInfo[index+m_sSystemInfo.iRealCamCount].m_iImageWidth = m_sRealCamInfo[index].m_iImageWidth;
			m_sRealCamInfo[index+m_sSystemInfo.iRealCamCount].m_iImageHeight = m_sRealCamInfo[index].m_iImageHeight;
			m_sRealCamInfo[index+m_sSystemInfo.iRealCamCount].m_iImageRoAngle = m_sRealCamInfo[index].m_iImageRoAngle;
			if (90 == m_sRealCamInfo[index].m_iImageRoAngle || 270 == m_sRealCamInfo[index].m_iImageRoAngle )
			{
				int iTemp = m_sRealCamInfo[index+m_sSystemInfo.iRealCamCount].m_iImageHeight;
				m_sRealCamInfo[index+m_sSystemInfo.iRealCamCount].m_iImageHeight = m_sRealCamInfo[index+m_sSystemInfo.iRealCamCount].m_iImageWidth;
				m_sRealCamInfo[index+m_sSystemInfo.iRealCamCount].m_iImageWidth = iTemp;
			}
		}
	}
	else
	{
		m_sRealCamInfo[index].m_bCameraInitSuccess = FALSE;
		s_GBERRORINFO ErrorInfo;
		QString str;			
		m_sRealCamInfo[index].m_pGrabber->GetLastErrorInfo(&ErrorInfo);
		str = tr("DeviceName:%1").arg(strDeviceName)+"\n"+tr("ErrorCode:%2").arg(ErrorInfo.nErrorCode)+"\n"+tr("ErrorDescription:%3").arg(ErrorInfo.strErrorDescription)+"\n"+tr("ErrorRemark:%4\n").arg(ErrorInfo.strErrorRemark);
		QMessageBox::information(this,tr("Error"),str);
		m_sRealCamInfo[index].m_strErrorInfo = str;
		if(m_sSystemInfo.m_iSystemType != 2)
		{
			m_sRealCamInfo[index+m_sSystemInfo.iRealCamCount].m_bCameraInitSuccess = m_sRealCamInfo[index].m_bCameraInitSuccess;
		}
	}
	if (90 == m_sRealCamInfo[index].m_iImageRoAngle || 270 == m_sRealCamInfo[index].m_iImageRoAngle )
	{
		int iTemp = m_sRealCamInfo[index].m_iImageHeight;
		m_sRealCamInfo[index].m_iImageHeight = m_sRealCamInfo[index].m_iImageWidth;
		m_sRealCamInfo[index].m_iImageWidth = iTemp;
	}
}
//初始化相机（设置曝光时间和触发方式）
void GlasswareDetectSystem::InitCam()
{
	for(int i = 0; i < m_sSystemInfo.iRealCamCount; i++)
	{
		if(m_sRealCamInfo[i].m_iGrabType == 8)
		{
			if(m_sRealCamInfo[i].m_iTrigger == 1)
			{
				((CDHGrabberMER*)m_sRealCamInfo[i].m_pGrabber)->MERSetParamInt(MERSnapMode,1);
				m_sRealCamInfo[i].m_bGrabIsTrigger = true;
			}
			else if(m_sRealCamInfo[i].m_iTrigger == 0)
			{
				((CDHGrabberMER*)m_sRealCamInfo[i].m_pGrabber)->MERSetParamInt(MERSnapMode,0);
				m_sRealCamInfo[i].m_bGrabIsTrigger = false;
			}
			((CDHGrabberMER*)m_sRealCamInfo[i].m_pGrabber)->MERSetParamInt(MERExposure,m_sRealCamInfo[i].m_iShuter);
		}
	}
}
//初始化图像（：读取切割信息:初始化图像队列和剪切后相机参数）
void GlasswareDetectSystem::InitImage()
{
	for (int i = 0;i<pMainFrm->m_sSystemInfo.iCamCount;i++)
	{
		int iRealCameraSN = m_sCarvedCamInfo[i].m_iToRealCamera;
		m_sCarvedCamInfo[i].m_iResImageWidth = m_sRealCamInfo[iRealCameraSN].m_iImageWidth;
		m_sCarvedCamInfo[i].m_iResImageHeight = m_sRealCamInfo[iRealCameraSN].m_iImageHeight;
		m_sCarvedCamInfo[i].m_iImageType = m_sRealCamInfo[iRealCameraSN].m_iImageType;
		m_sCarvedCamInfo[i].m_iIOCardSN =  m_sRealCamInfo[iRealCameraSN].m_iIOCardSN;
	}

	//获取最大图像信息
	for (int i=0;i<m_sSystemInfo.iRealCamCount;i++)
	{
		if (i==0)
		{
			m_sSystemInfo.m_iMaxCameraImageWidth     = m_sRealCamInfo[i].m_iImageWidth;
			m_sSystemInfo.m_iMaxCameraImageHeight    = m_sRealCamInfo[i].m_iImageHeight;
			m_sSystemInfo.m_iMaxCameraImageSize      = m_sRealCamInfo[i].m_iImageSize;
			m_sSystemInfo.m_iMaxCameraImagePixelSize = (m_sRealCamInfo[i].m_iImageBitCount+7)/8;
		}
		else
		{
			if (m_sRealCamInfo[i].m_iImageWidth > m_sSystemInfo.m_iMaxCameraImageWidth)
			{
				m_sSystemInfo.m_iMaxCameraImageWidth = m_sRealCamInfo[i].m_iImageWidth;
			}				
			if (m_sRealCamInfo[i].m_iImageHeight > m_sSystemInfo.m_iMaxCameraImageHeight)
			{
				m_sSystemInfo.m_iMaxCameraImageHeight = m_sRealCamInfo[i].m_iImageHeight;
			}				
			if (((m_sRealCamInfo[i].m_iImageBitCount+7)/8) > m_sSystemInfo.m_iMaxCameraImagePixelSize)
			{
				m_sSystemInfo.m_iMaxCameraImagePixelSize = ((m_sRealCamInfo[i].m_iImageBitCount+7)/8);
			}			
		}
		m_sSystemInfo.m_iMaxCameraImageSize = m_sSystemInfo.m_iMaxCameraImageWidth*m_sSystemInfo.m_iMaxCameraImageHeight;
	}
	ReadCorveConfig();
	QString strSession;
	for(int i = 0 ; i < m_sSystemInfo.iCamCount; i++)
	{
		//分配原始图像空间：每真实相机1个，剪切图像使用
		if (m_sRealCamInfo[i].m_pRealImage!=NULL)
		{
			delete m_sRealCamInfo[i].m_pRealImage;
			m_sRealCamInfo[i].m_pRealImage = NULL;
		}
		
		m_sRealCamInfo[i].m_pRealImage=new QImage(m_sRealCamInfo[i].m_iImageWidth,m_sRealCamInfo[i].m_iImageHeight,m_sRealCamInfo[i].m_iImageBitCount);// 用于实时显示

		if (8 == m_sRealCamInfo[i].m_iImageBitCount)
		{
			m_sRealCamInfo[i].m_pRealImage->setColorTable(m_vcolorTable);
		}

		memset(m_sRealCamInfo[i].m_pRealImage->bits(),0, m_sRealCamInfo[i].m_pRealImage->byteCount());

		m_sCarvedCamInfo[i].m_iImageBitCount = m_sRealCamInfo[i].m_iImageBitCount;   //图像位数从相机处继承[8/7/2013 nanjc]
		m_sCarvedCamInfo[i].m_iImageRoAngle = m_sRealCamInfo[i].m_iImageRoAngle;
		// 错误统计用类
		m_sRunningInfo.m_cErrorTypeInfo[i].m_iErrorTypeCount = m_sErrorInfo.m_iErrorTypeCount;

		//实时显示用, 预分配QImage空间，每切出相机一个
		if (m_sCarvedCamInfo[i].m_pActiveImage!=NULL)
		{
			delete m_sCarvedCamInfo[i].m_pActiveImage;
			m_sCarvedCamInfo[i].m_pActiveImage = NULL;
		}
		
		m_sCarvedCamInfo[i].m_pActiveImage=new QImage(m_sCarvedCamInfo[i].m_iImageWidth,m_sCarvedCamInfo[i].m_iImageHeight,m_sCarvedCamInfo[i].m_iImageBitCount);// 用于实时显示
		
		m_sCarvedCamInfo[i].m_pActiveImage->setColorTable(m_vcolorTable);
		//开始采集前补一张黑图
		BYTE* pByte = m_sCarvedCamInfo[i].m_pActiveImage->bits();
		int iLength = m_sCarvedCamInfo[i].m_pActiveImage->byteCount();
		memset((pByte),0,(iLength));
		//分配图像剪切内存区域,大小等于真实相机大小
		if (m_sCarvedCamInfo[i].m_pGrabTemp!=NULL)
		{
			delete m_sCarvedCamInfo[i].m_pGrabTemp; 
			m_sCarvedCamInfo[i].m_pGrabTemp = NULL;
		}
		m_sCarvedCamInfo[i].m_pGrabTemp = new BYTE[m_sRealCamInfo[i].m_iImageWidth*m_sRealCamInfo[i].m_iImageHeight];
		//分配元素链表中图像的内存，每剪切出来的相机10个。
		nQueue[i].InitCarveQueue(m_sCarvedCamInfo[i].m_iImageWidth, m_sCarvedCamInfo[i].m_iImageHeight,m_sRealCamInfo[i].m_iImageWidth,m_sRealCamInfo[i].m_iImageHeight,m_sCarvedCamInfo[i].m_iImageBitCount, 10, true);
		for (int k = 0; k < 256;k++)
		{
			delete []m_sCarvedCamInfo[i].sImageLocInfo[k].m_AlgImageLocInfos.sXldPoint.nColsAry;
			delete []m_sCarvedCamInfo[i].sImageLocInfo[k].m_AlgImageLocInfos.sXldPoint.nRowsAry;

			m_sCarvedCamInfo[i].sImageLocInfo[k].m_iHaveInfo = 0;
			m_sCarvedCamInfo[i].sImageLocInfo[k].m_AlgImageLocInfos.sXldPoint.nCount = 0;
			m_sCarvedCamInfo[i].sImageLocInfo[k].m_AlgImageLocInfos.sXldPoint.nRowsAry = new int[BOTTLEXLD_POINTNUM];
			m_sCarvedCamInfo[i].sImageLocInfo[k].m_AlgImageLocInfos.sXldPoint.nColsAry = new int[BOTTLEXLD_POINTNUM];
			memset(m_sCarvedCamInfo[i].sImageLocInfo[k].m_AlgImageLocInfos.sXldPoint.nRowsAry,0, 4*BOTTLEXLD_POINTNUM);
			memset(m_sCarvedCamInfo[i].sImageLocInfo[k].m_AlgImageLocInfos.sXldPoint.nColsAry,0, 4*BOTTLEXLD_POINTNUM);
			// 				memset
		}
	}
	SetCarvedCamInfo();
	//初始化缺陷图像列表
	m_ErrorList.initErrorList(m_sSystemInfo.m_iMaxCameraImageWidth,m_sSystemInfo.m_iMaxCameraImageHeight,m_sSystemInfo.m_iMaxCameraImagePixelSize*8,ERROR_IMAGE_COUNT,true);
}
//设置剪切后相机的参数
void GlasswareDetectSystem::SetCarvedCamInfo()
{
	for (int i = 0;i<m_sSystemInfo.iCamCount;i++)
	{
		int iRealCameraSN = m_sCarvedCamInfo[i].m_iToRealCamera;
		m_sCarvedCamInfo[i].m_iResImageWidth = m_sRealCamInfo[iRealCameraSN].m_iImageWidth;
		m_sCarvedCamInfo[i].m_iResImageHeight = m_sRealCamInfo[iRealCameraSN].m_iImageHeight;
		m_sCarvedCamInfo[i].m_iImageType = m_sRealCamInfo[iRealCameraSN].m_iImageType;
		m_sCarvedCamInfo[i].m_iIOCardSN =  m_sRealCamInfo[iRealCameraSN].m_iIOCardSN;
		m_sCarvedCamInfo[i].m_iShuter = m_sRealCamInfo[iRealCameraSN].m_iShuter;
		m_sCarvedCamInfo[i].m_iTrigger = m_sRealCamInfo[iRealCameraSN].m_iTrigger;
		m_sCarvedCamInfo[i].m_iGrabPosition = m_sRealCamInfo[iRealCameraSN].m_iGrabPosition;
	}
	SetCombineInfo();
}
//设置图像综合参数
void GlasswareDetectSystem::SetCombineInfo()
{
	//初始化结果综合参数
	for (int i = 0;i<m_sSystemInfo.iCamCount;i++)
	{
		m_cCombine.SetCombineCamera(i,true);
		m_sSystemInfo.IOCardiCamCount[0]++;
	}
	m_cCombine.Inital(m_sSystemInfo.IOCardiCamCount[0]);
}
//初始化IO卡
void GlasswareDetectSystem::InitIOCard()
{
	if (m_sSystemInfo.m_bIsIOCardOK)
	{
		m_sSystemInfo.m_sConfigIOCardInfo[0].iCardID = 0;
		m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile = QString("./PIO24B_reg_init.txt");
		m_sSystemInfo.m_sConfigIOCardInfo[0].strCardName = QString("PIO24B");
		m_vIOCard[0] = new CIOCard(m_sSystemInfo.m_sConfigIOCardInfo[0],0);
		connect(m_vIOCard[0],SIGNAL(emitMessageBoxMainThread(s_MSGBoxInfo)),this,SLOT(slots_MessageBoxMainThread(s_MSGBoxInfo)));
		s_IOCardErrorInfo sIOCardErrorInfo = m_vIOCard[0]->InitIOCard();
		if (!sIOCardErrorInfo.bResult)
		{
			m_sSystemInfo.m_bIsIOCardOK = false;
			pMainFrm->Logfile.write(tr("Error in init IOCard"),AbnormityLog);
		}
		pMainFrm->Logfile.write(tr("IOCard success"),AbnormityLog);
	}
}
//初始化算法
void GlasswareDetectSystem::InitCheckSet()
{
	//算法初始化，模板调入等 [8/4/2010 GZ]
	s_Status  sReturnStatus;
	s_AlgInitParam   sAlgInitParam;	
	//	QSettings iniAlgSet(m_sConfigInfo.m_strConfigPath,QSettings::IniFormat);
	if(m_sSystemInfo.m_iMaxCameraImageWidth>m_sSystemInfo.m_iMaxCameraImageHeight)
	{
		sReturnStatus = init_bottle_module(m_sSystemInfo.m_iMaxCameraImageWidth,m_sSystemInfo.m_iMaxCameraImageWidth,1);
	}
	else
	{
		sReturnStatus = init_bottle_module(m_sSystemInfo.m_iMaxCameraImageHeight,m_sSystemInfo.m_iMaxCameraImageHeight,1);
	}
	if (sReturnStatus.nErrorID != RETURN_OK)
	{
		pMainFrm->Logfile.write(tr("----load model error----"),AbnormityLog);
		return;
	}	
	for (int i=0;i<m_sSystemInfo.iCamCount;i++)
	{
		sAlgInitParam.nCamIndex=i;
		sAlgInitParam.nModelType = m_sRealCamInfo[i].m_iImageType;  //检测类型
		sAlgInitParam.nWidth = m_sRealCamInfo[i].m_iImageWidth; 
		sAlgInitParam.nHeight =  m_sRealCamInfo[i].m_iImageHeight;
		memset(sAlgInitParam.chCurrentPath,0,MAX_PATH);

		strcpy_s(sAlgInitParam.chCurrentPath,m_sConfigInfo.m_sAlgFilePath.toLocal8Bit()); 
		memset(sAlgInitParam.chModelName,0,MAX_PATH); //模板名称
		strcpy_s(sAlgInitParam.chModelName,m_sSystemInfo.m_strModelName.toLocal8Bit()); 
		sReturnStatus = m_cBottleCheck[i].init(sAlgInitParam);

		if (sReturnStatus.nErrorID != RETURN_OK && sReturnStatus.nErrorID != 1)
		{
			pMainFrm->Logfile.write(tr("----camera%1 load model error----").arg(i),AbnormityLog);
			return;
		}
		if (sReturnStatus.nErrorID == 1) //模板为空
		{
			//模板为空
			m_sSystemInfo.m_bLoadModel =  FALSE;  //如果模板为空，则不能检测 
		}
		else
		{
			m_sSystemInfo.m_bLoadModel =  TRUE;  //成功载入上一次的模板
		}
		// 旋转类 [12/10/2010]
		sAlgInitParam.nModelType = 99;  //检测类型
		memset(sAlgInitParam.chModelName,0,MAX_PATH); //模板名称
		m_cBottleRotate[i].init(sAlgInitParam);
		sAlgInitParam.nModelType = 98;  //检测类型
		m_cBottleStress[i].init(sAlgInitParam);
	}
	// 算法初始化，模板调入等 [8/4/2010 GZ]
	//////////////////////////////////////////////////////////////////////////
	if (CherkerAry.pCheckerlist != NULL)
	{
		delete[] CherkerAry.pCheckerlist;
	}
	CherkerAry.iValidNum = m_sSystemInfo.iCamCount;
	CherkerAry.pCheckerlist = new s_CheckerList[CherkerAry.iValidNum];


	//////////////////////////////////////////////////////////////////////////

}
//开启相机采集
void GlasswareDetectSystem::StartCamGrab()
{ 
	for (int i = 0;i<m_sSystemInfo.iRealCamCount;i++)
	{
		m_sRealCamInfo[i].m_pGrabber->StartGrab();
		m_sRealCamInfo[i].m_bGrabIsStart=TRUE;
	}
}
//开启检测线程
void GlasswareDetectSystem::StartDetectThread()
{
	m_bIsThreadDead = FALSE;
	for (int i=0;i<m_sSystemInfo.iCamCount;i++)
	{
		pdetthread[i]->start();
	}
}
void GlasswareDetectSystem::initDetectThread()
{
	m_bIsThreadDead = FALSE;
	CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)SendDetect, this, 0, NULL );
	CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)SendIOCard, this, 0, NULL );
	for (int i=0;i<m_sSystemInfo.iCamCount;i++)
	{
		pdetthread[i] = new DetectThread(this,i);
	}
}
//初始化界面
void GlasswareDetectSystem::initInterface()
{
	setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog  | Qt::WindowSystemMenuHint);//去掉标题栏
	QDesktopWidget* desktopWidget = QApplication::desktop();
	QRect screenRect = desktopWidget->screenGeometry();
	setMinimumSize(screenRect.width(),screenRect.height());

	QIcon icon;
	icon.addFile(QString::fromUtf8(":/sys/icon"), QSize(), QIcon::Normal, QIcon::Off);
	setWindowIcon(icon);

	statked_widget = new QStackedWidget();
	statked_widget->setObjectName("mainStacked");
	title_widget = new WidgetTitle(this);
	widget_carveSetting = new WidgetCarveSetting;
	widget_Management = new WidgetManagement;
	test_widget = new WidgetTest(this);
	test_widget->slots_intoWidget();
	widget_alg = new QWidget(this);
	widget_alg->setObjectName("widget_alg");

	QPalette palette;
	palette.setBrush(QPalette::Window, QBrush(Qt::white));
	statked_widget->setPalette(palette);
	statked_widget->setAutoFillBackground(true);
	statked_widget->addWidget(widget_carveSetting);
	statked_widget->addWidget(widget_Management);
	statked_widget->addWidget(test_widget);
	statked_widget->addWidget(widget_alg);
	
	//状态栏
	stateBar = new QWidget(this);
	stateBar->setFixedHeight(40);
	QGridLayout* gridLayoutStatusLight = new QGridLayout;
	for (int i = 0;i<pMainFrm->m_sSystemInfo.iCamCount;i++)
	{
		CameraStatusLabel *cameraStatus = new CameraStatusLabel(stateBar);
		cameraStatus->setObjectName("toolButtonCamera");
		cameraStatus->setAlignment(Qt::AlignCenter);
		cameraStatus->setText(QString::number(i+1));
		cameraStatus_list.append(cameraStatus);
		gridLayoutStatusLight->addWidget(cameraStatus,i%2,i/2);
	}
	//检查相机状态
	for (int i = 0; i < pMainFrm->m_sSystemInfo.iCamCount; i++)
	{
		int iRealCameraSN = pMainFrm->m_sCarvedCamInfo[i].m_iToRealCamera;
		CameraStatusLabel *cameraStatus = pMainFrm->cameraStatus_list.at(i);
		if (!pMainFrm->m_sRealCamInfo[iRealCameraSN].m_bCameraInitSuccess)
		{
			cameraStatus->SetCameraStatus(1);
		}
		else
		{
			cameraStatus->SetCameraStatus(0);
		}
	}
	QFont fontCoder;
	fontCoder.setPixelSize(28);
	labelCoder = new QLabel(stateBar);
	labelCoder->setFont(fontCoder);
	timerUpdateCoder = new QTimer(this);
	timerUpdateCoder->setInterval(1000);
	timerUpdateCoder->start();
	nSockScreen = new QTimer(this);
	nSockScreen->setInterval(1000*60);
	nSockScreen->start();

	QHBoxLayout* hLayoutStateBar = new QHBoxLayout(stateBar);
	hLayoutStateBar->addLayout(gridLayoutStatusLight);
	hLayoutStateBar->addStretch();
	hLayoutStateBar->addWidget(labelCoder);
	hLayoutStateBar->setSpacing(3);
	hLayoutStateBar->setContentsMargins(10, 0, 10, 0);

	QHBoxLayout *center_layout = new QHBoxLayout();
	center_layout->addWidget(statked_widget);
	center_layout->setSpacing(0);
	center_layout->setContentsMargins(5,5,5,5);

	QVBoxLayout *main_layout = new QVBoxLayout();
	main_layout->addWidget(title_widget);
	main_layout->addLayout(center_layout);
	main_layout->addWidget(stateBar);
	main_layout->setSpacing(0);
	main_layout->setContentsMargins(0, 0, 0, 0);

	setLayout(main_layout);
	connect(title_widget, SIGNAL(showMin()), this, SLOT(showMinimized()));
	connect(title_widget, SIGNAL(closeWidget()), this, SLOT(slots_OnExit()));
	connect(title_widget, SIGNAL(turnPage(int)), this, SLOT(slots_turnPage(int)));
	connect(this,SIGNAL(signals_intoManagementWidget()),widget_Management,SLOT(slots_intoWidget()));	
	connect(this,SIGNAL(signals_intoTestWidget()),test_widget,SLOT(slots_intoWidget()));	
	connect(timerUpdateCoder, SIGNAL(timeout()), this, SLOT(slots_UpdateCoderNumber()));    
	connect(nSockScreen, SIGNAL(timeout()), this, SLOT(slot_SockScreen()));    
 	for (int i = 0;i < pMainFrm->m_sSystemInfo.iCamCount;i++)
	{
		connect(pMainFrm->pdetthread[i], SIGNAL(signals_upDateCamera(int,int)), this, SLOT(slots_updateCameraState(int,int)));
	}
	connect(pMainFrm->widget_carveSetting->image_widget, SIGNAL(signals_SetCameraStatus(int,int)), this, SLOT(slots_SetCameraStatus(int,int)));
	m_eLastMainPage = CarveSettingPage;
	iLastPage = 0;
	skin.fill(QColor(90,90,90,120));
	nLightSource = new LightSource();
	nWidgetWarning = new Widget_Warning();
}
void GlasswareDetectSystem::SendDataToSever(int nSendCount,StateEnum nState)
{
	MyStruct nTempStruct;
	nTempStruct.nState = nState;
	QByteArray ba((char*)&nTempStruct, sizeof(MyStruct));
	nSocketMutex.lock();
	ncSocketWriteData.push_back(ba);
	nSocketMutex.unlock();
}
void GlasswareDetectSystem::slots_UpdateCoderNumber()
{
	int nCodeNum=0,nCheckNum=0,nSignNum=0;
	
	if (m_sRunningInfo.m_bCheck && m_sSystemInfo.m_bIsIOCardOK)
	{
		m_vIOCard[0]->m_mutexmIOCard.lock();
		nCheckNum = m_vIOCard[0]->ReadCounter(0);
		nSignNum = m_vIOCard[0]->ReadCounter(4);
		nCodeNum = m_vIOCard[0]->ReadCounter(13);
		m_vIOCard[0]->m_mutexmIOCard.unlock();
		//药玻瓶口瓶底剔废
		if((nCheckNum - m_sRunningInfo.m_passNum>0)&&(nCheckNum - m_sRunningInfo.m_passNum<50))
		{
			m_sRunningInfo.m_checkedNum = m_sRunningInfo.m_checkedNum + nCheckNum - m_sRunningInfo.m_passNum;
		}
		m_sRunningInfo.m_passNum = nCheckNum;
		if ((nSignNum - m_sRunningInfo.m_kickoutNumber > 0) && (nSignNum - m_sRunningInfo.m_kickoutNumber < 50))
		{
			m_sRunningInfo.m_failureNumFromIOcard = m_sRunningInfo.m_failureNumFromIOcard + nSignNum - m_sRunningInfo.m_kickoutNumber;
		}
		m_sRunningInfo.m_kickoutNumber = nSignNum;
	}

	QString strValue,strEncoder,strTime;
	strValue ="	";
	strEncoder = QString(tr("Speed:") +m_sRunningInfo.strSpeed+strValue+tr("Coder Number")+":%1").arg(nCodeNum);
	strTime = strValue+QString(tr("Time:"))+QTime::currentTime().toString() + strValue+sVersion;
	
	if(surplusDays>0)
	{
		labelCoder->setText(strEncoder+strTime+tr("Remaining days of use：%1 ").arg(surplusDays)); //剩余使用天数：%1
	}else{
		labelCoder->setText(strEncoder+strTime);
	}

	//保存IO卡的数据准备发送
	if(m_sRunningInfo.m_bCheck && m_sSystemInfo.m_bIsIOCardOK)
	{
		MyStruct nTempStruct;
		if(pMainFrm->m_sSystemInfo.m_iSystemType == 1)
		{
			nTempStruct.nUnit = LEADING;
		}else if(pMainFrm->m_sSystemInfo.m_iSystemType == 2)
		{
			nTempStruct.nUnit = CLAMPING;
		}else if(pMainFrm->m_sSystemInfo.m_iSystemType == 3)
		{
			nTempStruct.nUnit = BACKING;
		}
		char* nTPIOtr;
		if(pMainFrm->nCameraErrorType.count()>0)
		{
			nTempStruct.nState = ALERT;
			memcpy(m_ptr,&nTempStruct,sizeof(MyStruct));
			nTPIOtr = m_ptr;
			nTPIOtr+=sizeof(MyStruct);
			MyErrorType nTest = pMainFrm->nCameraErrorType.first();
			nCameraErrorType.clear();
			nIOCard[16] = nTest.id+1;
			nIOCard[17] = nTest.nType;
			if(m_sSystemInfo.m_iSystemType == 2)
			{
				nIOCard[21] = test_widget->nInfo.m_checkedNum;//表示第四块接口卡的过检总数
				nIOCard[22] = test_widget->nInfo.m_checkedNum2;//表示第四块接口卡的踢废数目
				nIOCard[23] = test_widget->m_plc->nErrorType;
			}
			memcpy(nTPIOtr,nIOCard,24*sizeof(int));
			memset(nIOCard,0,24*sizeof(int));
			QByteArray ba(m_ptr,24*sizeof(int)+sizeof(MyStruct));
			nSocketMutex.lock();
			pMainFrm->ncSocketWriteData.push_back(ba);
			nSocketMutex.unlock();
		}else{
			if(nLastCheckNum != test_widget->nInfo.m_checkedNum ||nLastFailedNum != test_widget->nInfo.m_checkedNum2|| m_sRunningInfo.m_iKickMode2!=test_widget->m_plc->nErrorType)
			{
				test_widget->nTestCounter.lock();
				nLastCheckNum = test_widget->nInfo.m_checkedNum;
				nLastFailedNum = test_widget->nInfo.m_checkedNum2;
				m_sRunningInfo.m_iKickMode2 = test_widget->m_plc->nErrorType;
				test_widget->nTestCounter.unlock();
				nTempStruct.nState = ALERT;
				memcpy(m_ptr,&nTempStruct,sizeof(MyStruct));
				nTPIOtr = m_ptr;
				nTPIOtr+=sizeof(MyStruct);
				if(m_sSystemInfo.m_iSystemType == 2)
				{
					nIOCard[21] = nLastCheckNum;//表示第四块接口卡的过检总数
					nIOCard[22] = nLastFailedNum;//表示第四块接口卡的踢废数目
					nIOCard[23] = test_widget->m_plc->nErrorType;
				}
				memcpy(nTPIOtr,nIOCard,24*sizeof(int));
				memset(nIOCard,0,24*sizeof(int));
				QByteArray ba(m_ptr,24*sizeof(int)+sizeof(MyStruct));
				nSocketMutex.lock();
				pMainFrm->ncSocketWriteData.push_back(ba);
				nSocketMutex.unlock();
			}
		}
	}
}

void GlasswareDetectSystem::slots_updateCameraState(int nCam,int mode)
{
	cameraStatus_list.at(nCam)->BlinkCameraStatus(mode);
}
void GlasswareDetectSystem::slots_SetCameraStatus(int nCam,int mode)
{
	cameraStatus_list.at(nCam)->SetCameraStatus(mode);
}
//裁剪
void GlasswareDetectSystem::CarveImage(uchar* pRes,uchar* pTar,int iResWidth,int iResHeight,int iTarX,int iTarY,int iTarWidth,int iTarHeight)
{
	try
	{
		uchar* pTemp = pTar;
		uchar* pTempRes = pRes+iResWidth*(iTarY)+iTarX;
		for(int i = 0; i < iTarHeight; i++)
		{
			memcpy(pTemp,pTempRes,iTarWidth);
			pTemp += iTarWidth;
			pTempRes += iResWidth;
		}
	}
	catch(...)
	{
		pMainFrm->Logfile.write(tr("Error in image carve "),AbnormityLog);
	}
}

void GlasswareDetectSystem::slots_turnPage(int current_page, int iPara)
{
	if (iLastPage == current_page && iLastPage != 9)
	{
		return;
	}
	switch (current_page)
	{
	case 0:
		m_eCurrentMainPage = CarveSettingPage;
		statked_widget->setCurrentWidget(widget_carveSetting);
		m_eLastMainPage = m_eCurrentMainPage;
		iLastPage = 0;
		pMainFrm->Logfile.write(("into CarveSetting"),AbnormityLog);
		break;
	case 1:
		m_eCurrentMainPage = ManagementSettingPage;
		emit signals_intoManagementWidget();
		statked_widget->setCurrentWidget(widget_Management);
		m_eLastMainPage = m_eCurrentMainPage;
		iLastPage = 1;
		pMainFrm->Logfile.write(("into Management"),AbnormityLog);
		break;
	case 2:
		m_eCurrentMainPage = TestPage;
		emit signals_intoTestWidget();
		statked_widget->setCurrentWidget(test_widget);
		m_eLastMainPage = m_eCurrentMainPage;
		iLastPage = 2;
		pMainFrm->Logfile.write(("into TestPage"),AbnormityLog);
		break;
	case 3:
		m_eCurrentMainPage = AlgPage;
		statked_widget->setCurrentWidget(widget_alg);
		ShowCheckSet(iPara);
		m_eLastMainPage = m_eCurrentMainPage;
		iLastPage = 3;
		pMainFrm->Logfile.write(("into AlgPage"),AbnormityLog);
		break;
	case 4:
		slots_OnBtnStar();
		break;
	case 5:
		slots_OnExit();
		break;
		/*case 6:
		iLastPage = 6;
		Sleep(1000);
		SendDataToSever(0,NLEADING);
		break;
		case 7:
		iLastPage = 7;
		Sleep(1000);
		SendDataToSever(0,NCLAMPING);
		break;
		case 8:
		iLastPage = 8;
		Sleep(1000);
		SendDataToSever(0,NBACKING);
		break;*/
	case 9:
		iLastPage = 9;
		Sleep(1000);
		SendDataToSever(0,MAININTERFACE);
		pMainFrm->Logfile.write(("return severs interface"),AbnormityLog);
		break;
	}
}

void GlasswareDetectSystem::slots_OnBtnStar()
{
	if (m_sSystemInfo.m_bIsTest)
	{
		QMessageBox::information(this,tr("Infomation"),tr("Please Stop Test First!"));
		return;
	}
	ToolButton *TBtn = title_widget->button_list.at(4);
	if (!m_sRunningInfo.m_bCheck )//开始检测
	{
		//图像综合清零
		m_cCombine.m_MutexCombin.lock();
		m_cCombine.RemovAllResult();
		m_cCombine.RemovAllError();
		m_cCombine.m_MutexCombin.unlock();

		if (m_sSystemInfo.m_bLoadModel)
		{
			//// 使能接口卡
			if (m_sSystemInfo.m_bIsIOCardOK)
			{
				//for (int i = 0; i< m_sSystemInfo.iIOCardCount;i++)
				{
					pMainFrm->Logfile.write(QString(("OpenIOCard%1")).arg(0),OperationLog,0);
					m_vIOCard[0]->enable(true);
				}
			}
			for (int i = 0; i < m_sSystemInfo.iCamCount;i++)
			{
				m_sRealCamInfo[i].m_iImageIdxLast[0] = 0;
				m_sRealCamInfo[i].m_iImageIdxLast[1] = 0;
			}
			m_sRunningInfo.nGSoap_ErrorTypeCount[0]=0;
			m_sRunningInfo.nGSoap_ErrorCamCount[0]=0;
			/*m_sRunningInfo.nGSoap_ErrorTypeCount[2]=0;
			m_sRunningInfo.nGSoap_ErrorCamCount[2]=0;*/

			pMainFrm->Logfile.write(("Start Check"),OperationLog);
			m_sRunningInfo.m_bCheck = true;
		}
		else
		{
			QMessageBox::information(this,tr("Error"),tr("No Model,Please Load Model!"));
			return;
		}
		QPixmap pixmap(":/toolWidget/stop");
		TBtn->setText(tr("Stop"));
		TBtn->setIcon(pixmap);
		TBtn->bStatus = true;
	}

	else if (m_sRunningInfo.m_bCheck)//停止检测
	{
		if (m_sSystemInfo.m_bIsIOCardOK)
		{
			//for (int i = 0; i< m_sSystemInfo.iIOCardCount;i++)
			{
				pMainFrm->Logfile.write(QString(("CloseIOCard%1")).arg(0),OperationLog,0);
				m_vIOCard[0]->enable(false);

			}
		}
		// 停止算法检测 
		m_sRunningInfo.m_bCheck = false;
		pMainFrm->Logfile.write(("Stop Check"),OperationLog);

		QPixmap pixmap(":/toolWidget/start");
		TBtn->setText(tr("Start"));
		TBtn->setIcon(pixmap);

		for (int i = 0;i<m_sSystemInfo.iCamCount;i++)
		{
			s_SystemInfoforAlg sSystemInfoforAlg;
			sSystemInfoforAlg.bIsChecking = false;
			m_cBottleCheck[i].setsSystemInfo(sSystemInfoforAlg);
		}
		TBtn->bStatus = false;
	}
}

void GlasswareDetectSystem::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::lightGray);
	painter.drawPixmap(QRect(0, 0, this->width(), this->height()), QPixmap(skin));
}
//弹出提示信息对话框
void GlasswareDetectSystem::slots_MessageBoxMainThread(s_MSGBoxInfo msgbox)
{
	QMessageBox::information(this,msgbox.strMsgtitle,msgbox.strMsgInfo);	
}
//释放IO卡
void  GlasswareDetectSystem::ReleaseIOCard()
{
	if (m_sSystemInfo.m_bIsIOCardOK)
	{
		m_vIOCard[0]->CloseIOCard();
	}
	delete m_vIOCard[0];
}
// 关闭相机 [11/11/2010 zhaodt]
void GlasswareDetectSystem::CloseCam()
{
	pMainFrm->Logfile.write(("CloseCam"),OperationLog);

	for (int i=0;i<m_sSystemInfo.iRealCamCount;i++)
	{
		if (m_sRealCamInfo[i].m_bCameraInitSuccess && m_sRealCamInfo[i].m_bGrabIsStart) 
		{
			m_sRealCamInfo[i].m_pGrabber->StopGrab();
			m_sRealCamInfo[i].m_bGrabIsStart=FALSE;// 是否开始采集状态
		}
	}	
	Sleep(1000);
	for (int i=0;i<m_sSystemInfo.iRealCamCount;i++)
	{
		if (m_sRealCamInfo[i].m_pGrabber!=NULL)
		{
			m_sRealCamInfo[i].m_pGrabber->Close();
		}
	}
}
//释放图像资源
void GlasswareDetectSystem::ReleaseImage()
{
	for(int i = 0 ; i < m_sSystemInfo.iRealCamCount; i++)
	{
		delete m_sRealCamInfo[i].m_pRealImage;
		m_sRealCamInfo[i].m_pRealImage = NULL;
	}
	for(int i = 0 ; i < m_sSystemInfo.iCamCount; i++)
	{
		delete m_sCarvedCamInfo[i].m_pActiveImage;
		m_sCarvedCamInfo[i].m_pActiveImage = NULL;

		delete[] m_sCarvedCamInfo[i].m_pGrabTemp;
		m_sCarvedCamInfo[i].m_pGrabTemp = NULL;
		nQueue[i].releaseMemory();
		if (m_detectElement[i].bIsImageNormalCompelet)
		{
			delete m_detectElement[i].ImageNormal->myImage;
		}
	}
}
//释放所有资源
void GlasswareDetectSystem::ReleaseAll()
{
	m_bIsThreadDead = TRUE;
	for(int i = 0; i < m_sSystemInfo.iCamCount; i++)
	{
		s_Status sReturnStatus = m_cBottleCheck[i].Free();
		for (int j = 0; j < 256;j++)
		{
			delete []m_sCarvedCamInfo[i].sImageLocInfo[j].m_AlgImageLocInfos.sXldPoint.nRowsAry;
			delete []m_sCarvedCamInfo[i].sImageLocInfo[j].m_AlgImageLocInfos.sXldPoint.nColsAry;
		}
	}
	if (CherkerAry.pCheckerlist != NULL)
	{
		delete[] CherkerAry.pCheckerlist;
	}
	ReleaseImage();
}

void GlasswareDetectSystem::directoryChanged(QString path)
{
	QMessageBox::information(NULL, tr("Directory change"), path);
}

//功能：动态切换系统语言
bool GlasswareDetectSystem::changeLanguage(int nLangIdx)
{
	QSettings sysSet("daheng","GlassDetectSystem");
	static QTranslator *translator = NULL, *qtDlgCN = NULL;
	bool bRtn = true;
	if (nLangIdx == 0)//中文
	{
		translator = new QTranslator;
		qtDlgCN = new QTranslator;
		if (translator->load("glasswaredetectsystem_zh.qm"))
		{
			qApp->installTranslator(translator);
			//中文成功后，加载Qt对话框标准翻译文件，20141202
			if (qtDlgCN->load("glasswaredetectsystem_zh.qm"))
			{
				qApp->installTranslator(qtDlgCN);
			}
			//保存设置
			sysSet.setValue("nLangIdx",nLangIdx);
		}
		else
		{
			QMessageBox::information(this,tr("Information"),tr("Load Language pack [glasswaredetectsystem_zh.qm] fail!"));
			//保存设置
			sysSet.setValue("nLangIdx",1);
			bRtn = false;
		}
	}
	return bRtn;
}

void GlasswareDetectSystem::ShowCheckSet(int nCamIdx,int signalNumber)
{
	try
	{
		s_AlgModelPara  sAlgModelPara;	
		QImage tempIamge;

		if(widget_carveSetting->image_widget->bIsShowErrorImage[nCamIdx]&&pMainFrm->m_SavePicture[nCamIdx].pThat!=NULL)
		{
			tempIamge=pMainFrm->m_SavePicture[nCamIdx].m_Picture;
			sAlgModelPara.sImgLocInfo = widget_carveSetting->image_widget->sAlgImageLocInfo[nCamIdx];

		}else{
			pMainFrm->nQueue[nCamIdx].mGrabLocker.lock();
			if(pMainFrm->nQueue[nCamIdx].listGrab.size()==0)
			{
				pMainFrm->nQueue[nCamIdx].mGrabLocker.unlock();
				return;
			}
			CGrabElement *pElement = pMainFrm->nQueue[nCamIdx].listGrab.last();
			tempIamge = (*pElement->myImage);
			sAlgModelPara.sImgLocInfo = pElement->sImgLocInfo;
			pMainFrm->nQueue[nCamIdx].mGrabLocker.unlock();
		}
		m_cBottleModel.CloseModelDlg();
		sAlgModelPara.sImgPara.nChannel = 1;
		sAlgModelPara.sImgPara.nHeight = tempIamge.height();
		sAlgModelPara.sImgPara.nWidth = tempIamge.width();
		sAlgModelPara.sImgPara.pcData = (char*)tempIamge.bits();
		
		if (sAlgModelPara.sImgPara.nHeight != pMainFrm->m_sCarvedCamInfo[nCamIdx].m_iImageHeight)
		{
			return;
		}
		if (sAlgModelPara.sImgPara.nWidth != pMainFrm->m_sCarvedCamInfo[nCamIdx].m_iImageWidth)
		{
			return;
		}		
		
		for (int i=0;i<m_sSystemInfo.iCamCount;i++)
		{
			CherkerAry.pCheckerlist[i].nID = i;
			CherkerAry.pCheckerlist[i].pChecker = &m_cBottleCheck[i];
		}	
		int widthd = widget_alg->geometry().width();
		int heightd	= widget_alg->geometry().height();
		if (widthd < 150 || heightd < 150)
		{
			return;
		}	
		s_Status  sReturnStatus = m_cBottleModel.SetModelDlg(sAlgModelPara,&m_cBottleCheck[nCamIdx],CherkerAry,widget_alg);
		if (sReturnStatus.nErrorID != RETURN_OK)
		{
			return;
		}
		statked_widget->setCurrentWidget(widget_alg);
		m_eCurrentMainPage = AlgPage;
		m_eLastMainPage = AlgPage;
		iLastPage = 3;
	}
	catch (...)
	{
	}
	pMainFrm->Logfile.write(("Into Alg Page")+QString("CamraNo:%1").arg(nCamIdx+1),OperationLog,0);
	return;	
}
void GlasswareDetectSystem::slots_OnExit(bool ifyanz)
{
	QSettings iniDataSet(m_sConfigInfo.m_strDataPath,QSettings::IniFormat);
	iniDataSet.setIniCodec(QTextCodec::codecForName("GBK"));
	QString strSession;
	strSession=QString("/system/checkedNum");
	iniDataSet.setValue(strSession,m_sRunningInfo.m_checkedNum);

	strSession = QString("/system/failureNum");
	iniDataSet.setValue(strSession,m_sRunningInfo.m_failureNumFromIOcard);

	if (ifyanz || QMessageBox::Yes == QMessageBox::question(this,tr("Exit"),
		tr("Are you sure to exit?"),
		QMessageBox::Yes | QMessageBox::No))	
	{
		if (m_sSystemInfo.m_bIsTest)
		{
			QMessageBox::information(this,tr("Infomation"),tr("Please Stop Test First!"));
			return;
		}
		if (m_sRunningInfo.m_bCheck )//开始检测
		{
			QMessageBox::information(this,tr("Infomation"),tr("Please Stop Detection First!"));
			return;		
		}
		
		EquipRuntime::Instance()->EquipExitLogFile();
		ToolButton *TBtn = title_widget->button_list.at(4);
		pMainFrm->Logfile.write(("Close ModelDlg!"),OperationLog);
		s_Status  sReturnStatus = m_cBottleModel.CloseModelDlg();
		if (sReturnStatus.nErrorID != RETURN_OK)
		{
			pMainFrm->Logfile.write(("Error in Close ModelDlg--OnExit"),AbnormityLog);
			return;
		}
		CloseCam();
		ReleaseAll();
		ReleaseIOCard();
		exit(0);
		//close();
	}
}
int GlasswareDetectSystem::ReadImageSignal(int nImageNum,int cameraID)
{
	//接口卡获取图像号
	if(m_sSystemInfo.m_iSystemType != 2)
	{
		switch(nImageNum) 
		{
		case 1:
			return pMainFrm->m_vIOCard[0]->ReadImageSignal(28);
		case 2:
			return pMainFrm->m_vIOCard[0]->ReadImageSignal(31);
		case 3:
			return pMainFrm->m_vIOCard[0]->ReadImageSignal(29);
		case 4:
			return pMainFrm->m_vIOCard[0]->ReadImageSignal(30);
		case 5:
			return pMainFrm->m_vIOCard[0]->ReadImageSignal(49);
		case 6:
			return pMainFrm->m_vIOCard[0]->ReadImageSignal(50);
		default:
			return 0;
		}
	}else{
		switch(nImageNum) 
		{
		case 1:
			return pMainFrm->m_vIOCard[0]->ReadImageSignal(49);
		case 2:
			return pMainFrm->m_vIOCard[0]->ReadImageSignal(50);
		case 3:
			return pMainFrm->m_vIOCard[0]->ReadImageSignal(51);
		case 4:
			return pMainFrm->m_vIOCard[0]->ReadImageSignal(52);
		case 5:
			return pMainFrm->m_vIOCard[0]->ReadImageSignal(35);
		default:
			return 0;
		}
	}
}

void GlasswareDetectSystem::InitCamImage(int iCameraNo)
{
	for (int i=0;i<m_sSystemInfo.iRealCamCount;i++)
	{
		if (i==0)
		{
			m_sSystemInfo.m_iMaxCameraImageWidth     = m_sRealCamInfo[i].m_iImageWidth;
			m_sSystemInfo.m_iMaxCameraImageHeight    = m_sRealCamInfo[i].m_iImageHeight;
			m_sSystemInfo.m_iMaxCameraImageSize      = m_sRealCamInfo[i].m_iImageSize;
			m_sSystemInfo.m_iMaxCameraImagePixelSize = (m_sRealCamInfo[i].m_iImageBitCount+7)/8;
		}
		else
		{
			if (m_sRealCamInfo[i].m_iImageWidth > m_sSystemInfo.m_iMaxCameraImageWidth)
			{
				m_sSystemInfo.m_iMaxCameraImageWidth = m_sRealCamInfo[i].m_iImageWidth;
			}				
			if (m_sRealCamInfo[i].m_iImageHeight > m_sSystemInfo.m_iMaxCameraImageHeight)
			{
				m_sSystemInfo.m_iMaxCameraImageHeight = m_sRealCamInfo[i].m_iImageHeight;
			}				
			if (((m_sRealCamInfo[i].m_iImageBitCount+7)/8) > m_sSystemInfo.m_iMaxCameraImagePixelSize)
			{
				m_sSystemInfo.m_iMaxCameraImagePixelSize = ((m_sRealCamInfo[i].m_iImageBitCount+7)/8);
			}			
		}
		m_sSystemInfo.m_iMaxCameraImageSize = m_sSystemInfo.m_iMaxCameraImageWidth*m_sSystemInfo.m_iMaxCameraImageHeight;
	}
	ReadCorveConfig();

	QString strSession;
	
	int i = iCameraNo;
	m_sCarvedCamInfo[i].m_iImageBitCount = m_sRealCamInfo[i].m_iImageBitCount;   //图像位数从相机处继承[8/7/2013 nanjc]
	m_sCarvedCamInfo[i].m_iImageRoAngle = m_sRealCamInfo[i].m_iImageRoAngle;
	m_sRunningInfo.m_cErrorTypeInfo[i].m_iErrorTypeCount = m_sErrorInfo.m_iErrorTypeCount;
	if (m_sCarvedCamInfo[i].m_pActiveImage!=NULL)
	{
		delete m_sCarvedCamInfo[i].m_pActiveImage; 
		m_sCarvedCamInfo[i].m_pActiveImage = NULL;
	}
	m_sCarvedCamInfo[i].m_pActiveImage=new QImage(m_sCarvedCamInfo[i].m_iImageWidth,m_sCarvedCamInfo[i].m_iImageHeight,m_sCarvedCamInfo[i].m_iImageBitCount);// 用于实时显示

	m_sCarvedCamInfo[i].m_pActiveImage->setColorTable(m_vcolorTable);
	BYTE* pByte = m_sCarvedCamInfo[i].m_pActiveImage->bits();
	int iLength = m_sCarvedCamInfo[i].m_pActiveImage->byteCount();
	memset((pByte),0,(iLength));
	
	nQueue[i].mDetectLocker.lock();
	nQueue[i].mGrabLocker.lock();
	nQueue[i].InitCarveQueue(m_sCarvedCamInfo[i].m_iImageWidth, m_sCarvedCamInfo[i].m_iImageHeight,m_sRealCamInfo[i].m_iImageWidth,m_sRealCamInfo[i].m_iImageHeight,m_sCarvedCamInfo[i].m_iImageBitCount, 10, true);
	nQueue[i].mGrabLocker.unlock();
	nQueue[i].mDetectLocker.unlock();
	//SetCarvedCamInfo();
}
bool GlasswareDetectSystem::RoAngle(uchar* pRes,uchar* pTar,int iResWidth,int iResHeight,int iAngle)
{
	int iTarWidth;
	int iTarHeight;
	if(pRes == NULL || iResWidth == 0 || iResHeight == 0)
	{
		return FALSE;
	}
	if (iAngle == 90)
	{
		iTarWidth = iResHeight;
		iTarHeight = iResWidth;
		for (int i=0;i<iResHeight;i++)
		{
			for (int j=0;j<iResWidth;j++) 
			{
				*(pTar+j*iTarWidth+(iTarWidth-i-1)) = *(pRes+i*iResWidth+j);
			}
		}
	}
	if (iAngle == 270)
	{	
		iTarWidth = iResHeight;
		iTarHeight = iResWidth;
		for (int i=0;i<iResHeight;i++)
		{
			for (int j=0;j<iResWidth;j++) 
			{
				*(pTar+(iTarHeight-j-1)*iTarWidth+i) = *(pRes+i*iResWidth+j);
			}
		}
	}
	if (iAngle == 180)
	{
		iTarWidth = iResWidth;
		iTarHeight = iResHeight;
		for (int i=0;i<iResHeight;i++)
		{
			for (int j=0;j<iResWidth;j++) 
			{
				*(pTar+(iTarHeight-i-1)*iTarWidth+(iTarWidth-j-1))=*(pRes+i*iResWidth+j);
			}
		}
	}
	return TRUE;
}

void GlasswareDetectSystem::SetLanguage(int pLang)
{
	sLanguage = pLang;
}

void GlasswareDetectSystem::slot_SockScreen()
{
	QTime time = QTime::currentTime();
	if(n_EndTime != time.minute())
	{
		n_EndTime = time.minute();
		SendDataToSever(0,CONNECT);
	}
}

void GlasswareDetectSystem::InitLastData()
{
	QSettings iniDataSet(m_sConfigInfo.m_strDataPath,QSettings::IniFormat);
	iniDataSet.setIniCodec(QTextCodec::codecForName("GBK"));
	QString strSession;
	strSession=QString("/system/checkedNum");
	m_sRunningInfo.m_checkedNum=iniDataSet.value(strSession,0).toInt();

	strSession=QString("/system/failureNum");
	m_sRunningInfo.m_failureNumFromIOcard=iniDataSet.value(strSession,0).toInt();
}
#ifdef JIAMI_INITIA
void GlasswareDetectSystem::MonitorLicense()
{
	QString  g_UidChar = "06a6914a-d863-43e1-800e-7e2eece22fd7";
	ver_code uucode;
	m_ProgramLicense.GetVerCode(&uucode);
	QString strCode = QString("%1-%2-%3-%4%5-%6%7%8%9%10%11")
		.arg(uucode.Data1,8,16,QChar('0')).arg(uucode.Data2,4,16,QChar('0')).arg(uucode.Data3,4,16,QChar('0'))
		.arg((int)uucode.Data4[0],2,16,QChar('0')).arg((int)uucode.Data4[1],2,16,QChar('0'))
		.arg((int)uucode.Data4[2],2,16,QChar('0')).arg((int)uucode.Data4[3],2,16,QChar('0'))
		.arg((int)uucode.Data4[4],2,16,QChar('0')).arg((int)uucode.Data4[5],2,16,QChar('0'))
		.arg((int)uucode.Data4[6],2,16,QChar('0')).arg((int)uucode.Data4[7],2,16,QChar('0'));
	if (g_UidChar == strCode)
	{
		//验证License
		s_KeyVerfResult res = m_ProgramLicense.CheckLicenseValid(true);
		if (res.nError <= 0)
		{
			//int m_nLicenseDays = res.nDays;
			int m_nLicenseDays = m_ProgramLicense.ReadHardwareID("getexpdate");
			if (m_nLicenseDays<=10 && m_nLicenseDays>0)
			{
				//弹出提示框
				showAllert();
			}
			//更新剩余时间
			surplusDays = m_nLicenseDays;
		}else{
			m_sSystemInfo.m_bIsTest = false;
			m_sRunningInfo.m_bCheck = true;
			slots_OnBtnStar();
			slots_OnExit(true);
		}
	}else{
		QMessageBox::information(this,tr("Error"),tr("Encryption verification failed, will exit the program!"));//加密验证失败,即将退出程序!
		m_sSystemInfo.m_bIsTest = false;
		m_sRunningInfo.m_bCheck = true;
		slots_OnBtnStar();
		slots_OnExit(true);
	}
}
void GlasswareDetectSystem::showAllert()
{
	pushLicense* m_tempLicense=new pushLicense; 
	m_tempLicense->slots_ShowWarning(0,tr("The authorization to use the equipment is about to expire \n, please contact business personnel!")); //设备使用授权即将到期\n请联系商务人员！
	m_tempLicense->MaxNumber->start();
}
bool GlasswareDetectSystem::CheckLicense()
{
	QString  g_UidChar = "06a6914a-d863-43e1-800e-7e2eece22fd7";
	ver_code uucode;
	m_ProgramLicense.GetVerCode(&uucode);
	QString strCode = QString("%1-%2-%3-%4%5-%6%7%8%9%10%11")
		.arg(uucode.Data1,8,16,QChar('0')).arg(uucode.Data2,4,16,QChar('0')).arg(uucode.Data3,4,16,QChar('0'))
		.arg((int)uucode.Data4[0],2,16,QChar('0')).arg((int)uucode.Data4[1],2,16,QChar('0'))
		.arg((int)uucode.Data4[2],2,16,QChar('0')).arg((int)uucode.Data4[3],2,16,QChar('0'))
		.arg((int)uucode.Data4[4],2,16,QChar('0')).arg((int)uucode.Data4[5],2,16,QChar('0'))
		.arg((int)uucode.Data4[6],2,16,QChar('0')).arg((int)uucode.Data4[7],2,16,QChar('0'));
	if (g_UidChar == strCode)
	{
		//验证License
		s_KeyVerfResult res = m_ProgramLicense.CheckLicenseValid(true);
		if (res.nError <= 0)//未超时
		{
			int nDogValue = (int)m_ProgramLicense.ReadDog();
			if (nDogValue == 0)
			{
				//读取加密狗参数异常 代码：22
				QMessageBox::information(this,tr("Error"),tr("License expired or dongle abnormal! Error code: 22"));//License过期或加密狗异常！错误代码：22
				return false;
			}
			int m_nLicenseDays = m_ProgramLicense.ReadHardwareID("getexpdate");
			//m_nLicenseDays = res.nDays;
			surplusDays = m_nLicenseDays;
			if (m_nLicenseDays<=10 && m_nLicenseDays>0)
			{
				showAllert();
			}
			//传递窗口句柄
			m_ProgramLicense.SetMainWnd((HWND)this->winId());
			return true; 
		}
		else
		{
			QMessageBox::information(this,tr("Error"),tr("License expired or dongle abnormal! Error code: %1").arg(res.nError)); //License过期或加密狗异常！错误代码：%1
			return false;
		}
	}
	else
	{
		QMessageBox::information(this,tr("Error"),tr("Encryption authentication failed!")); //加密验证失败
		return false;
	}
	return true;
}
#else
void GlasswareDetectSystem::MonitorLicense()
{
}
bool GlasswareDetectSystem::CheckLicense()
{
	return TRUE;
}
#endif // JIAMI_INITIA


