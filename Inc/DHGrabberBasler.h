/************************************************************************/
/* Copyright (c) 2013, �й���㼯�ű���ͼ���Ӿ������ֹ�˾�Ӿ�ϵͳ��  
/* All rights reserved.													
/*																		
/* �ļ����ƣ� DHGrabberForMER.h	
/* ժҪ�� �����ɼ����ࣨ��˾ˮ�������

/* ��ǰ�汾��1.0.0.3
/* �޸��ߣ���ͬ
/* ��Ҫ�޸����ݣ�ʵ�ֻ����ĳ�ʼ���������ɼ���ֹͣ�ɼ�����֡�ɼ��Ͳ������ý��� ���ֺ����滻Ϊ�°�sdk����
/* ������ڣ�2016��7��15��

/************************************************************************/

#if !defined(DHGrabber_h_)
#define DHGrabber_h_

/*
�ɼ������ͱ�Ŷ�������
0:ģ��ɼ���
1:DALSA X64�ɼ���
2:AVT���
3:ǧ�������
4:��˾CG300ģ��ɼ���
5:��˾1394���
6:EURESYS �ɼ���
7:��˾USB���
8::��˾ˮ��ϵ�����*/

const int GBMaxFileNameLen = 255;
const int GBMaxTextLen = 255;

enum SBaslerCameraControl_Type{
	Type_Basler_Freerun, //����������ڴ���
	Type_Basler_Line1, //����������ⴥ��
	Type_Basler_ExposureTimeAbs, //����������ع�ʱ��
	Type_Basler_GainRaw, //�������������
	Type_Basler_AcquisitionFrameRateAbs, //���������Ƶ��
	Type_Basler_Width, //ͼƬ�Ŀ���
	Type_Basler_Height, //ͼƬ�ĸ߶�
	Type_Basler_LineSource, //�ƵĴ����ź�
};
typedef struct _GBSignalInfoStruct//�ص���Ϣ�ṹ
{
	PVOID Context;								//���ӵ�лص������Ķ����thisָ��
	int iGrabberTypeSN;							// �ɼ������ͱ��
	int nGrabberSN;								//�ɼ������
	int nErrorCode;								//������루0��������
	int nFrameCount;							//�Ѳɼ���֡����
	char strDescription[GBMaxTextLen];			//������Ϣ

	int iReserve1;								// �����ֶ�1
	int iReserve2;								// �����ֶ�2
	
}s_GBSIGNALINFO;//�ص���Ϣ�ṹ

//�ص�����ָ������
typedef void (WINAPI *PGBCALLBACK)(const s_GBSIGNALINFO* SignalInfo);

typedef struct _GBInitStruct//�ɼ�����ʼ���ṹ
{
	int iGrabberTypeSN;						// �ɼ������ͱ��
	int nGrabberSN;							//�ɼ������
    char strDeviceName[GBMaxTextLen];		//�豸����
	char strDeviceMark[GBMaxTextLen];		//�豸��ʶ
	char strGrabberFile[GBMaxFileNameLen];	//���ڳ�ʼ�����ļ�
	PGBCALLBACK CallBackFunc;				//�ص�����ָ��
	PVOID Context;							//���ӵ�лص������Ķ����thisָ��
	
	int iReserve1;							// �����ֶ�1
	int iReserve2;							// �����ֶ�2

}s_GBINITSTRUCT;//�ɼ�����ʼ���ṹ

typedef struct _GBErrorInfoStruct//������Ϣ�ṹ
{
   	int nErrorCode;							//������루0��������
	char strErrorDescription[GBMaxTextLen];	//��������
	char strErrorRemark[GBMaxTextLen];		//������Ϣ

}s_GBERRORINFO;//������Ϣ�ṹ

//��������
enum GBParamID
{
	GBImageWidth = 0,		//ͼ����� (��λ:�ֽ�)
	GBImageHeight,			//ͼ��߶� (��λ:�ֽ�)
	GBImagePixelSize,		//ͼ�����ش�С (��λ:�ֽ�)
	GBImageBufferSize,		//ͼ�񻺳�����С (��λ:�ֽ�)
	GBImageBufferAddr,		//ͼ�񻺳�����ַ
	GBGrabberTypeSN			//�ɼ������ͱ��
};

//������붨��
const int GBOK = 0;

class CGrabber
{
//����
public:
	CGrabber()
	{
	}
	virtual ~CGrabber()
	{
	}
	
	//��ʼ��
	virtual BOOL Init(const s_GBINITSTRUCT* pInitParam) = 0;

	//�ر�
	virtual BOOL Close() = 0;

	//��ʼ�ɼ�
	virtual BOOL StartGrab() = 0;

	//ֹͣ�ɼ�
	virtual BOOL StopGrab() = 0;
	
	//��֡�ɼ�
	virtual BOOL Snapshot() = 0;

	//���ò���
	virtual BOOL SetParamInt(GBParamID Param, int nInputVal) = 0;
	
	//�õ�����
	virtual BOOL GetParamInt(GBParamID Param, int &nOutputVal) = 0;

	virtual double BalserSetParam(SBaslerCameraControl_Type parm,int exposeTime) = 0;
	//���ò����Ի���
	virtual void CallParamDialog() = 0;
	
	//�õ�������Ϣ
	virtual void GetLastErrorInfo(s_GBERRORINFO *pErrorInfo) = 0;
};

#endif// !defined(DHGrabber_h_)

#if !defined(DHGrabberBasler_h_)
#define DHGrabberBasler_h_

enum SBaslerCameraControl_Type{
	Type_Basler_Freeru = 0, //����������ڴ���
	Type_Basler_Line1, //����������ⴥ��
	Type_Basler_ExposureTimeAbs, //����������ع�ʱ��
	Type_Basler_GainRaw, //�������������
	Type_Basler_AcquisitionFrameRateAbs, //���������Ƶ��
	Type_Basler_Width, //ͼƬ�Ŀ���
	Type_Basler_Height, //ͼƬ�ĸ߶�
	Type_Basler_LineSource, //�ƵĴ����ź�
	Get_Basler_ExposureTimeAbs
};
//�ɼ���
class _declspec(dllexport) CDHGrabberBasler : public CGrabber
{
	//����
public:
	CDHGrabberBasler();
	virtual ~CDHGrabberBasler();
	
	//��ʼ��
	BOOL Init(const s_GBINITSTRUCT* pInitParam);
	
	//�ر�
	BOOL Close();
	
	//��ʼ�ɼ�
	BOOL StartGrab();
	
	//ֹͣ�ɼ�
	BOOL StopGrab();
	
	// ��֡�ɼ� 
	BOOL Snapshot();
	
	//���ò���
	BOOL SetParamInt(GBParamID Param, int nInputVal);
	
	//�õ�����
	BOOL GetParamInt(GBParamID Param, int &nOutputVal);
	
	//���ò����Ի���
	void CallParamDialog();
	
	//�õ�������Ϣ
	void GetLastErrorInfo(s_GBERRORINFO *pErrorInfo);
	
	//////////////////////////////////////////////////////////////////////////
	// ��˾���ר�нӿ�	
	BOOL MERSetParamInt(MERParamID Param, int nInputVal1,int nInputVal2 = 0,int nInputVal3 = 0,int nInputVal4 = 0,int nInputVal5 = 0,void *sInputVal6 = NULL);
	BOOL MERGetParamInt(MERParamID Param, int &nOutputVal1,int &nOutputVal2,int &nOutputVal3);	
	BOOL MERGetParamString(MERParamID Param, char* sOutputString,int size);	
	int  MERSetOutputParamInt(int nOutputPort,int nOutputVal);//��������Զ���������� 
	//����ֵ 0:�ɹ� 1:��֧���Զ����ź� 2:�����Զ����ź�ʧ�� 3:�򿪶˿�ʧ�� 4:д���ֵʧ�� 
    //��ȡ�������
	BOOL MERGetCameraCount(int &nCameraCount);
	//����Ż�ȡ������к�,��Ŵ�0��ʼ
    BOOL MerGetCameraSN(int nCameraNumber,char sCameraSN[MaxSNLen]);
	
	void SaveParamToINI();		// �������������Ϣ
	
	void BalserSetParam(SBaslerCameraControl_Type parm,int exposeTime);
	//////////////////////////////////////////////////////////////////////////
	int CamerColorOrGray;
	//����
protected:
	CGrabber *m_pGrabber;
};

#endif// !defined(DHGrabberMER_h_)