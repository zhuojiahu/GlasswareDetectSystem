#include "DetectThread.h"
#include <QMessageBox>
#include "glasswaredetectsystem.h"
#include <math.h>
#include <QMessageBox>
extern GlasswareDetectSystem *pMainFrm;
DetectThread::DetectThread(QObject *parent,int temp)
	: QThread(parent),tempOri()
{
	m_threadId = temp;
	m_bStopThread = false;
	dirSaveImagePath = new QDir;
	iMaxErrorType = 0;
	iMaxErrorArea = 0;
	iErrorType = 0;
	
}
DetectThread::~DetectThread()
{
	delete dirSaveImagePath;
}
void DetectThread::run()
{
	while (!pMainFrm->m_bIsThreadDead)
	{
		if(!m_bStopThread)
		{
			ProcessHanlde(m_threadId);		
		}
		Sleep(1);
	}
}
void DetectThread::ProcessHanlde(int Camera)
{
	if(pMainFrm->nQueue[Camera].listDetect.count()>0)
	{
		pMainFrm->nQueue[Camera].mDetectLocker.lock();
		CDetectElement DetectElement = pMainFrm->nQueue[Camera].listDetect.first();
		pMainFrm->nQueue[Camera].listDetect.removeFirst();
		pMainFrm->nQueue[Camera].mDetectLocker.unlock();
		iCamera = DetectElement.iCameraNormal;
		//��תԭʼͼƬ

		if (90 == pMainFrm->m_sRealCamInfo[iCamera].m_iImageRoAngle || 180 == pMainFrm->m_sRealCamInfo[iCamera].m_iImageRoAngle|| 270 == pMainFrm->m_sRealCamInfo[iCamera].m_iImageRoAngle)
		{
			pMainFrm->RoAngle(DetectElement.ImageNormal->SourceImage->bits(),pMainFrm->m_sRealCamInfo[iCamera].m_pRealImage->bits(),\
				DetectElement.ImageNormal->nWidth,DetectElement.ImageNormal->nHeight,pMainFrm->m_sRealCamInfo[iCamera].m_iImageRoAngle);
		}
		else
		{
			long lImageSize = pMainFrm->m_sRealCamInfo[iCamera].m_iImageWidth * pMainFrm->m_sRealCamInfo[iCamera].m_iImageHeight;
			memcpy(pMainFrm->m_sRealCamInfo[iCamera].m_pRealImage->bits(),DetectElement.ImageNormal->SourceImage->bits(),lImageSize);
		}
		//�ü�ԭʼͼƬ
		pMainFrm->nQueue[iCamera].mGrabLocker.lock();
		pMainFrm->CarveImage(pMainFrm->m_sRealCamInfo[iCamera].m_pRealImage->bits(),pMainFrm->m_sCarvedCamInfo[iCamera].m_pGrabTemp,\
			pMainFrm->m_sRealCamInfo[iCamera].m_iImageWidth,pMainFrm->m_sRealCamInfo[iCamera].m_iImageHeight, pMainFrm->m_sCarvedCamInfo[iCamera].i_ImageX,pMainFrm->m_sCarvedCamInfo[iCamera].i_ImageY,\
			pMainFrm->m_sCarvedCamInfo[iCamera].m_iImageWidth,pMainFrm->m_sCarvedCamInfo[iCamera].m_iImageHeight);			

		memcpy(DetectElement.ImageNormal->myImage->bits(), pMainFrm->m_sCarvedCamInfo[iCamera].m_pGrabTemp, \
			pMainFrm->m_sCarvedCamInfo[iCamera].m_iImageWidth*pMainFrm->m_sCarvedCamInfo[iCamera].m_iImageHeight);
		//�����⻷��
		pMainFrm->nQueue[iCamera].mGrabLocker.unlock();
		if(0 == DetectElement.iType)
		{
			//CountRuningData(iCamera);
			DetectNormal(DetectElement.ImageNormal);
			if (pMainFrm->nQueue[iCamera].InitID == DetectElement.ImageNormal->initID)
			{ 
				pMainFrm->nQueue[iCamera].mGrabLocker.lock();
				pMainFrm->nQueue[iCamera].listGrab.push_back(DetectElement.ImageNormal);
				pMainFrm->nQueue[iCamera].mGrabLocker.unlock();
			}
			else
			{
				//pMainFrm->Logfile.write(tr("delete a element!"),AbnormityLog);
				delete DetectElement.ImageNormal->SourceImage;
				delete DetectElement.ImageNormal->myImage;
				DetectElement.ImageNormal = NULL;
				delete DetectElement.ImageNormal;
			}
		}else{
			//iStressCamera = DetectElement.iCameraNormal;
			//Sleep(1);
			DetectStress(DetectElement.ImageNormal);
			if (pMainFrm->nQueue[iCamera].InitID == DetectElement.ImageNormal->initID)
			{
				pMainFrm->nQueue[iCamera].mGrabLocker.lock();
				pMainFrm->nQueue[iCamera].listGrab.push_back(DetectElement.ImageNormal);
				pMainFrm->nQueue[iCamera].mGrabLocker.unlock();
			}
			else
			{
				//pMainFrm->Logfile.write(tr("delete a element!"),AbnormityLog);
				delete DetectElement.ImageNormal->SourceImage;
				delete DetectElement.ImageNormal->myImage;
				DetectElement.ImageNormal->myImage = NULL;
				delete DetectElement.ImageNormal;
			}
		}
	}
}
void DetectThread::DetectNormal(CGrabElement *pElement)
{
	checkTimecost.StartSpeedTest();

	bCheckResult[iCamera] = false;
	iErrorType = 0;
	iMaxErrorType = 0;
	iMaxErrorArea = 0;
	pElement->cErrorRectList.clear();
	try
	{
		rotateImage(pElement);
		if (pMainFrm->m_sRunningInfo.m_bCheck)
		{
			try
			{
				checkImage(pElement, 1);
				bool bOK = getCheckResult(pElement);
				if (!bOK)
				{
					return;
				}
			}
			catch (...)
			{
			}
		}
	}
	catch(...)
	{
	}
	try
	{
		kickOutBad(pElement->nSignalNo);
		saveImage(pElement);
		//������ͼ������������
		if (bCheckResult[iCamera])
		{
			addErrorImageList(pElement);
		}
	}
	catch (...)
	{
	}
	checkTimecost.StopSpeedTest();
	pElement->dCostTime = checkTimecost.dfTime;

	//ˢ��ͼ���״̬
	if (pMainFrm->nQueue[iCamera].InitID == pElement->initID)
	{
		upDateState(pElement->myImage,pElement->nSignalNo,pElement->dCostTime, pElement->nMouldID, pElement->cErrorRectList,pElement->initID);
	}
	pElement = NULL;
}
void DetectThread::DetectStress(CGrabElement *pElement)
{
	checkTimecost.StartSpeedTest();
	bCheckResult[iCamera] = false;
	iErrorType = 0;
	iMaxErrorType = 0;
	iMaxErrorArea = 0;
	pElement->cErrorRectList.clear();

	try
	{
		rotateImage(pElement);
		if (pMainFrm->m_sRunningInfo.m_bCheck)
		{
			try
			{
				checkImage(pElement, 2);
				bool bOK = getCheckResult(pElement);
				if (!bOK)
				{
					return;
				}
			}
			catch (...)
			{
			}
		}
	}
	catch(...)
	{
	}
	try
	{
		kickOutBad(pElement->nSignalNo);
		//����ͼ��
		saveImage(pElement);
		//������ͼ������������
		if (bCheckResult[iCamera])
		{
			addErrorImageList(pElement);
		}
	}
	catch (...)
	{
	}
	checkTimecost.StopSpeedTest();
	pElement->dCostTime = checkTimecost.dfTime;
	//ˢ��ͼ���״̬
	if (pMainFrm->nQueue[iCamera].InitID == pElement->initID)
	{
		upDateState(pElement->myImage,pElement->nSignalNo,pElement->dCostTime, pElement->nMouldID, pElement->cErrorRectList,pElement->initID);
	}
	pElement = NULL;
}
//��תͼ��
void DetectThread::rotateImage(CGrabElement *pElement)
{
	sAlgCInp.sInputParam.nWidth = pElement->myImage->width();
	sAlgCInp.sInputParam.nHeight = pElement->myImage->height();
	sAlgCInp.sInputParam.pcData = (char*)pElement->myImage->bits();
	if(pMainFrm->m_sCarvedCamInfo[iCamera].m_iImageAngle != 0)
	{
		sAlgCInp.nParam = pMainFrm->m_sCarvedCamInfo[iCamera].m_iImageAngle;
		pMainFrm->m_cBottleRotate[iCamera].Check(sAlgCInp, &pAlgCheckResult);
	}
}
//���
void DetectThread::checkImage(CGrabElement *pElement,int iCheckMode)
{
	sAlgCInp.sInputParam.nHeight = pElement->myImage->height();
	sAlgCInp.sInputParam.nWidth = pElement->myImage->width();
	sAlgCInp.sInputParam.nChannel = 1;
	sAlgCInp.sInputParam.pcData = (char*)pElement->myImage->bits();
	sReturnStatus = pMainFrm->m_cBottleCheck[iCamera].Check(sAlgCInp,&pAlgCheckResult);
}
//��ȡ�����
bool DetectThread::getCheckResult(CGrabElement *pElement)
{
	if (sReturnStatus.nErrorID != 0)
	{
		return false;
	}
	if(iCamera == 0)
	{
		CountRuningData(0);
	}
	//��ȡ���ģ���������
	GetModelDotData(pElement);
	if (pAlgCheckResult->nSizeError >0) //�д����Ҵ����δ�رռ��
	{
		//������ƿͳ��
		bCheckResult[iCamera] = true;
		pElement->cErrorParaList.clear(); //�����

		for (int j=0;j<pAlgCheckResult->nSizeError;j++) //����㷨���ش����������
		{
			s_ErrorPara  sErrorPara;
			sErrorPara = pAlgCheckResult->vErrorParaAry[j];
			if(sErrorPara.nArea > iMaxErrorArea)
			{
				iMaxErrorArea = sErrorPara.nArea;
				iMaxErrorType = sErrorPara.nErrorType;
			}
			QRect rect(sErrorPara.rRctError.left,sErrorPara.rRctError.top,sErrorPara.rRctError.right - sErrorPara.rRctError.left,sErrorPara.rRctError.bottom - sErrorPara.rRctError.top);
			if (iMaxErrorType > pMainFrm->m_sErrorInfo.m_iErrorTypeCount)
			{
				iMaxErrorType = pMainFrm->m_sErrorInfo.m_iErrorTypeCount+1;
				sErrorPara.nErrorType = pMainFrm->m_sErrorInfo.m_iErrorTypeCount+1;
			}
			//	�Ҳ���ԭ�㲻�߷�
			if (1 == sErrorPara.nErrorType&&(1 == pMainFrm->m_sSystemInfo.m_iNoRejectIfNoOrigin[iCamera] || 1 == pMainFrm->m_sSystemInfo.m_NoKickIfNoFind ))
			{
				bCheckResult[iCamera] = false;
			}
			// Ԥ���������߷�
			if (2 == sErrorPara.nErrorType&&(1 == pMainFrm->m_sSystemInfo.m_iNoRejectIfROIfail[iCamera] || 1 == pMainFrm->m_sSystemInfo.m_NoKickIfROIFail ))
			{
				bCheckResult[iCamera] = false;
			}			
			if(sErrorPara.nErrorType==39)
			{
				bCheckResult[iCamera] = true;
			}
			//���㷨���ش�������������
			if (bCheckResult[iCamera])
			{
				pElement->cErrorRectList.append(rect);
				pElement->cErrorParaList.append(sErrorPara);
				emit signals_upDateCamera(iCamera,1 );
			}
			//���������ۺ�
			//�Ҳ���ԭ�㲻�ۺ�
			if (1 == sErrorPara.nErrorType&&(1 == pMainFrm->m_sSystemInfo.m_iNoRejectIfNoOrigin[iCamera] || 1 == pMainFrm->m_sSystemInfo.m_NoKickIfNoFind ))
			{
				;
			}
			else if (2 == sErrorPara.nErrorType&&(1 == pMainFrm->m_sSystemInfo.m_iNoRejectIfROIfail[iCamera] || 1 == pMainFrm->m_sSystemInfo.m_NoKickIfROIFail ))
			{
				;
			}
			else
			{
				//pMainFrm->m_cCombine.m_MutexCombin.lock();
				pMainFrm->m_cCombine.AddError(pElement->nSignalNo,iCamera,sErrorPara);
				//pMainFrm->m_cCombine.m_MutexCombin.unlock();
			}
		}	

		iErrorType = iMaxErrorType;
		pElement->nCheckRet = iErrorType;
	}
	else//û�д������
	{
		emit signals_upDateCamera(iCamera,0);
		s_ErrorPara sErrorPara;
		sErrorPara.nArea = 0;
		sErrorPara.nErrorType = 0;
		//pMainFrm->m_cCombine.m_MutexCombin.lock();
		pMainFrm->m_cCombine.AddError(pElement->nSignalNo,iCamera,sErrorPara);
		//pMainFrm->m_cCombine.m_MutexCombin.unlock();
	}
	return true;
}
void DetectThread::kickOutBad(int nSignalNo)
{
	int tmpResult=0;
	switch (pMainFrm->m_sRunningInfo.m_iKickMode)
	{
	case 0:			// ������ 
		tmpResult=1;
		break;
	case 1:			// ������
		tmpResult=0;
		break;
	case 2:			// ������
		tmpResult=bCheckResult[iCamera];
		break;
	}
	CountDefectIOCard(nSignalNo,tmpResult);
}
void DetectThread::CountDefectIOCard(int nSignalNo,int tmpResult)
{
	int comResult = -1;//�ۺϺ�Ľ��
	pMainFrm->m_cCombine.AddResult(nSignalNo,iCamera,tmpResult);
	if (pMainFrm->m_cCombine.ConbineResult(nSignalNo,0,comResult))//ͼ����������ۺ�
	{
		//for	(int i = nSignalNo - 5; i<nSignalNo ;i++)
		//{
		//	if (!pMainFrm->m_cCombine.IsReject((i+256)%256))
		//	{
		//		pMainFrm->m_sRunningInfo.nGSoap_ErrorTypeCount[2]++;
		//		s_ResultInfo sResultInfo;
		//		sResultInfo.tmpResult = pMainFrm->m_cCombine.m_Rlts[(i+256)%256].iResult;
		//		sResultInfo.nImgNo = (i+256)%256;
		//		sResultInfo.nIOCardNum = 0;
		//		if (pMainFrm->m_sSystemInfo.m_bIsIOCardOK)
		//		{
		//			//��ʱʹ�����ñ�����Ϊ�ܵ��ۺ��߷���Ŀ by zl
		//			pMainFrm->m_sRunningInfo.nGSoap_ErrorCamCount[2] += 1;
		//			pMainFrm->m_vIOCard[sResultInfo.nIOCardNum]->SendResult(sResultInfo);
		//		}
		//		pMainFrm->m_cCombine.SetReject((i+256)%256);
		//	}
		//}

		for	(int i = nSignalNo; i < nSignalNo + 5;i++)
		{
			pMainFrm->m_cCombine.SetReject(i%256,false);
		}
		//��ʱʹ�����ñ�����Ϊ�ܵ��ۺϹ�����Ŀ by zl
		pMainFrm->m_sRunningInfo.nGSoap_ErrorTypeCount[0]++;
		s_ResultInfo sResultInfo;
		sResultInfo.tmpResult = comResult;
		sResultInfo.nImgNo = nSignalNo;
		sResultInfo.nIOCardNum = 0;
		if (pMainFrm->m_sSystemInfo.m_bIsIOCardOK)
		{
			pMainFrm->m_vIOCard[sResultInfo.nIOCardNum]->SendResult(sResultInfo);
		}
		/*if(comResult)
		{
		pMainFrm->m_sRunningInfo.m_failureNumFromIOcard++;
		}*/
		pMainFrm->m_cCombine.SetReject(nSignalNo);
		//ȱ��ͳ��
		pMainFrm->m_cCombine.RemoveOneResult(nSignalNo);
		if (pMainFrm->m_sRunningInfo.m_bCheck)	
		{
			pMainFrm->nAllConut++;
			int iErrorCamera = pMainFrm->m_cCombine.ErrorCamera(nSignalNo);
			s_ErrorPara sComErrorpara = pMainFrm->m_cCombine.ConbineError(nSignalNo);
			if (pMainFrm->m_sRunningInfo.m_cErrorTypeInfo[iErrorCamera].ErrorTypeJudge(sComErrorpara.nErrorType))
			{
				pMainFrm->m_sRunningInfo.m_cErrorTypeInfo[iErrorCamera].iErrorCountByType[sComErrorpara.nErrorType]+=1;
				pMainFrm->m_sRunningInfo.m_iErrorCamCount[iErrorCamera] += 1;
				//��ʱʹ�����ñ�����Ϊ�ܵ��ۺ��߷���Ŀ by zl
				pMainFrm->m_sRunningInfo.nGSoap_ErrorCamCount[0] += 1;//��ͬ����
				pMainFrm->m_sRunningInfo.m_iErrorTypeCount[sComErrorpara.nErrorType] +=1;
				
				/*if(comResult)
				{
					pMainFrm->m_sRunningInfo.m_failureNum2++;
				}*/
				//MyErrorType test;
				//test.id = iErrorCamera;
				//test.nType = sComErrorpara.nErrorType;
				//pMainFrm->nCameraErrorType.push_back(test);
				//emit signals_updateAlert(sComErrorpara.nErrorType);//����
				//memcpy(pMainFrm->nSendData+(pMainFrm->nAllConut-1)*24+iCamera-1,&sComErrorpara.nErrorType,sizeof(int));
			}
			else
			{
				pMainFrm->m_sRunningInfo.m_cErrorTypeInfo[iErrorCamera].iErrorCountByType[0]+=1;
				pMainFrm->m_sRunningInfo.m_iErrorTypeCount[0] +=1;
			}
			//if(pMainFrm->nAllConut == 4)
			//{
			//	memcpy(pMainFrm->nCheckSendData,pMainFrm->nSendData,256*24);
			//	emit signals_AddDataToBase(pMainFrm->m_sRunningInfo.m_failureNum2,pMainFrm->nAllConut,pMainFrm->nCheckSendData);//���ݴ��䵽������
			//	memset(pMainFrm->nSendData,0,256*24);
			//	pMainFrm->m_sRunningInfo.m_failureNum2 = 0;
			//	pMainFrm->nAllConut = 0;
			//}
		}
	}
}

void DetectThread::saveImage(CGrabElement *pElement)
{
	if (1 == pMainFrm->m_sSystemInfo.m_iSaveNormalErrorImageByTime)
	{
		if (bCheckResult[iCamera])
		{
			QDateTime time = QDateTime::currentDateTime();
			QString strSaveImagePath = QString(pMainFrm->m_sConfigInfo.m_strAppPath + tr("SaveImageByTime\\") +\
				tr("normal image\\") + time.date().toString("yyyy-MM-dd") + tr("\\camera%1").arg(iCamera+1)) + "\\" + time.time().toString("hh");
			bool exist = dirSaveImagePath->exists(strSaveImagePath);
			if(!exist)
			{
				dirSaveImagePath->mkpath(strSaveImagePath);
			}
			QString strSavePath = QString(strSaveImagePath + "/image number%1_%2%3%4.bmp").arg(pElement->nSignalNo).arg(time.time().hour()).arg(time.time().minute()).arg(time.time().second());
			pElement->myImage->mirrored().save(strSavePath);
		}
	}
	if (1 == pMainFrm->m_sSystemInfo.m_iSaveStressErrorImageByTime)
	{
		if (bCheckResult[iCamera])
		{
			QDateTime time = QDateTime::currentDateTime();
			QString strSaveImagePath = QString(pMainFrm->m_sConfigInfo.m_strAppPath + tr("SaveImageByTime\\") +\
				tr("stress image\\") + time.date().toString("yyyy-MM-dd") + tr("\\camera%1").arg(iCamera+1)) + "\\" + time.time().toString("hh");
			bool exist = dirSaveImagePath->exists(strSaveImagePath);
			if(!exist)
			{
				dirSaveImagePath->mkpath(strSaveImagePath);
			}
			QString strSavePath = QString(strSaveImagePath + "/image number%1_%2%3%4.bmp").arg(pElement->nSignalNo).arg(time.time().hour()).arg(time.time().minute()).arg(time.time().second());
			pElement->myImage->mirrored().save(strSavePath);
		}

	}
	if (AllImage == pMainFrm->m_sRunningInfo.m_eSaveImageType || AllImageInCount == pMainFrm->m_sRunningInfo.m_eSaveImageType)
	{
		if (0 == pMainFrm->m_sSystemInfo.m_bSaveCamera[iCamera])
		{
			return;
		}
		QTime time = QTime::currentTime();
		QString strSaveImagePath = QString(pMainFrm->m_sConfigInfo.m_strAppPath + "SaveImage/All-image/camera%1/").arg(iCamera+1);
		bool exist = dirSaveImagePath->exists(strSaveImagePath);
		if(!exist)
		{
			dirSaveImagePath->mkpath(strSaveImagePath);
		}
		if (AllImage == pMainFrm->m_sRunningInfo.m_eSaveImageType)
		{
			QString strSavePath = QString(strSaveImagePath + "image_%1_%2%3%4_%5.bmp").arg(pElement->nSignalNo).arg(time.hour()).arg(time.minute()).arg(time.second()).arg(time.msec());
			pElement->myImage->mirrored().save(strSavePath);
		}
		if (AllImageInCount == pMainFrm->m_sRunningInfo.m_eSaveImageType)
		{
			pMainFrm->m_sRunningInfo.m_mutexRunningInfo.lock();
			if (pMainFrm->m_sRunningInfo.m_iSaveImgCount[iCamera] > 0)
			{
				QString strSavePath = QString(strSaveImagePath + "image_%1_%2%3%4_%5.bmp").arg(pElement->nSignalNo).arg(time.hour()).arg(time.minute()).arg(time.second()).arg(time.msec());
				pElement->myImage->mirrored().save(strSavePath);
				pMainFrm->m_sRunningInfo.m_iSaveImgCount[iCamera]--;
			}
			int itempSavemode = 0;
			for (int i = 0 ; i<pMainFrm->m_sSystemInfo.iCamCount;i++)
			{
				if (pMainFrm->m_sSystemInfo.m_bSaveCamera[i] == 1)
				{
					itempSavemode = 1;
				}
			}
			if (0 == itempSavemode)
			{
				pMainFrm->m_sRunningInfo.m_eSaveImageType = NotSave;
			}
			pMainFrm->m_sRunningInfo.m_mutexRunningInfo.unlock();
		}
	}else if (FailureImage == pMainFrm->m_sRunningInfo.m_eSaveImageType||FailureImageInCount == pMainFrm->m_sRunningInfo.m_eSaveImageType)
	{
		if (0 == pMainFrm->m_sSystemInfo.m_bSaveCamera[iCamera])
		{
			return;
		}
		if (0 == pMainFrm->m_sSystemInfo.m_bSaveErrorType[iErrorType])
		{
			return;
		}
		QTime time = QTime::currentTime();
		QString strSaveImagePath = QString(pMainFrm->m_sConfigInfo.m_strAppPath + "SaveImage/Failure-image/camera%1").arg(iCamera+1);
		bool exist = dirSaveImagePath->exists(strSaveImagePath);
		if(!exist)
		{
			dirSaveImagePath->mkpath(strSaveImagePath);
		}
		if (FailureImage == pMainFrm->m_sRunningInfo.m_eSaveImageType)
		{
			QString strSavePath = QString(strSaveImagePath + "image_%1_%2%3%4_%5.bmp").arg(pElement->nSignalNo).arg(time.hour()).arg(time.minute()).arg(time.second()).arg(time.msec());
			pElement->myImage->mirrored().save(strSavePath);
		}
		if (FailureImageInCount == pMainFrm->m_sRunningInfo.m_eSaveImageType)
		{
			pMainFrm->m_sRunningInfo.m_mutexRunningInfo.lock();
			if (pMainFrm->m_sRunningInfo.m_iSaveImgCount[iCamera] > 0)
			{
				QString strSavePath = QString(strSaveImagePath + "image_%1_%2%3%4_%5.bmp").arg(pElement->nSignalNo).arg(time.hour()).arg(time.minute()).arg(time.second()).arg(time.msec());
				pElement->myImage->mirrored().save(strSavePath);
				pMainFrm->m_sRunningInfo.m_iSaveImgCount[iCamera]--;
			}
			if (0 == pMainFrm->m_sRunningInfo.m_iSaveImgCount[iCamera])
			{
				pMainFrm->m_sSystemInfo.m_bSaveCamera[iCamera] = 0;
			}
			int itempSavemode = 0;
			for (int i = 0 ; i<pMainFrm->m_sSystemInfo.iCamCount;i++)
			{
				if (pMainFrm->m_sSystemInfo.m_bSaveCamera[i] == 1)
				{
					itempSavemode = 1;
				}
			}
			if (0 == itempSavemode)
			{
				pMainFrm->m_sRunningInfo.m_eSaveImageType = NotSave;
			}
			pMainFrm->m_sRunningInfo.m_mutexRunningInfo.unlock();
		}
	}
}
//��ȱ��ͼ������������
void DetectThread::addErrorImageList(CGrabElement *pElement)
{
	pMainFrm->m_ErrorList.m_mutexmErrorList.lock();
	CGrabElement *pErrorElement = pMainFrm->m_ErrorList.listError.last();
	pMainFrm->m_ErrorList.listError.removeLast();
	pErrorElement->nCamSN = pElement->nCamSN;
	pErrorElement->dCostTime = pElement->dCostTime;
	pErrorElement->nCheckRet = pElement->nCheckRet;
	pErrorElement->nSignalNo = pElement->nSignalNo; 
	pErrorElement->cErrorRectList = pElement->cErrorRectList;
	pErrorElement->cErrorParaList = pElement->cErrorParaList;
	//******************************************
	if (pErrorElement->myImage != NULL)
	{
		delete pErrorElement->myImage;
		pErrorElement->myImage = NULL;
	}
	pErrorElement->myImage = new QImage(*pElement->myImage);

	pMainFrm->m_ErrorList.listError.push_front(pErrorElement);
	pMainFrm->m_ErrorList.m_mutexmErrorList.unlock();

	emit signals_AddErrorTableView(pElement->nCamSN,pElement->nSignalNo,pElement->cErrorParaList.first().nErrorType);
}
//������ʾ״̬
void DetectThread::upDateState( QImage* myImage, int signalNo,double costTime, int nMouldID, QList<QRect> listErrorRectList,int ImageCount)
{
	QString camera = QString::number(iCamera);
	QString imageSN = QString::number(signalNo);
	QString time = QString::number(costTime,'f',2);
	QString result = pMainFrm->m_sErrorInfo.m_vstrErrorType.at(iErrorType);
	QString mouldID = QString::number(nMouldID);

	if(0 == pMainFrm->test_widget->ifshowImage)//ȫ��ˢ��
	{
		emit signals_updateActiveImg(iCamera,signalNo,costTime,iErrorType);//���¼��е�ͼ����ʾ
		emit signals_updateImage(myImage, camera, imageSN, time, result, mouldID, listErrorRectList, ImageCount);
	}
	else if(1 == pMainFrm->test_widget->ifshowImage)//ֻˢ��ͼ
	{
		if (bCheckResult[iCamera])
		{
			emit signals_updateActiveImg(iCamera,signalNo,costTime,iErrorType);//���¼��е�ͼ����ʾ
			emit signals_updateImage(myImage, camera, imageSN, time, result, mouldID, listErrorRectList, ImageCount);
		}
	}
	//emit signals_updateCameraFailureRate();
}

void DetectThread::GetModelDotData(CGrabElement *pElement)
{
	pElement->nMouldID = pAlgCheckResult->nMouldID;
	if (pAlgCheckResult->nMouldID>0 && pAlgCheckResult->nMouldID < 100)
	{
		pMainFrm->m_sRunningInfo.nModelCheckedCount++;
	}
}
void DetectThread::CountRuningData( int cameraNumber )
{
	static DWORD lastSpeed;
	static int nSpeedCount;
	if(lastSpeed == 0)
	{
		lastSpeed = timeGetTime();
	}
	else
	{
		nSpeedCount++;			
		int nCurTime = timeGetTime();
		//����ƿ�Ӵ����ٶ� = ����/ʱ�䣨min����������1s����һ��
		if (nCurTime-lastSpeed > 1000)
		{
			int nCurSpeed = nSpeedCount*1.0 / (nCurTime-lastSpeed) * 60000;
			lastSpeed = nCurTime;
			nSpeedCount = 0;
			pMainFrm->m_sRunningInfo.strSpeed = QString::number(nCurSpeed);
			emit signals_showspeed(nCurSpeed);
		}
	}
}