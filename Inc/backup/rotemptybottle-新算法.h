#ifndef ROTEMPTYBOTTLE_H
#define ROTEMPTYBOTTLE_H

//#include <QtCore/qglobal.h>
//
//#ifdef ROTEMPTYBOTTLE_LIB
//# define ROTEMPTYBOTTLE_EXPORT Q_DECL_EXPORT
//#else
//# define ROTEMPTYBOTTLE_EXPORT Q_DECL_IMPORT
//#endif
#define DLL_EXT _declspec(dllexport) 

#include "alg_struct.h"
using namespace Alg;

#include <vector>
#include <list>
using namespace std;

const int RETURN_OK           = 0; //��������
const int RETURN_CHECK_INIT   = 1; //��ʼ������
const int RETURN_MODEL_MANAGE = 2; //ģ��������
const int RETURN_PARA_MANAGE  = 3; //���������ô���
const int RETURN_CHECK_ERROR  = 4; //����㷨����
const int RETURN_CHECK_FREE   = 5; //�ͷŴ���

class DLL_EXT RotEmptyBottle
{
public:
	RotEmptyBottle();
	~RotEmptyBottle();

private:

};
class CBottleCheck;
typedef struct 
{
	int nID;   //����ID
	CBottleCheck  *pChecker;   //��������
}s_CheckerList;
///*****����Ϊ�㷨��̬����ؽӿڶ���*****//

// ��������
class CAlgorithm  
{
public:
	CAlgorithm()
	{
	}
	virtual ~CAlgorithm()
	{
	}
	virtual Alg::s_Status init(Alg::s_AlgInitParam sAInitParam)=0;
	virtual Alg::s_Status Free()=0;
	virtual Alg::s_Status Check(Alg::s_AlgCInP sAlgCInP,Alg::s_AlgCheckResult **sAlgCheckResult)=0;
	virtual Alg::s_Status CopySizePara(Alg::s_SizePara4Copy &sSizePara)=0;
};

// ģ�������
class CModel 
{
public:
	CModel()
	{
	}
	virtual ~CModel()
	{
}
	virtual Alg::s_Status SetModelDlg(Alg::s_AlgModelPara sAlgModelPara,CBottleCheck *pChecker,
		list< s_CheckerList> & sCheckerList)=0;

};


///*****����Ϊ�㷨������������*****//

/************************************************************************
	���ܣ���ʼ����⶯̬�⣬������������̬�⿪ʼǰ���Ǹı�������������ǰ
    int nWidth, int nHeight������ͼ����
	���Ƕ�����������Ҫͳһ��ʼ���Ĳ��� 
/************************************************************************/
s_Status DLL_EXT init_bottle_module(int nWidth, int nHeight, int nChannel);

s_Status DLL_EXT free_bottle_module();

//���ӿ���
class DLL_EXT CBottleCheck : public CAlgorithm
{
public:
	CBottleCheck();
	virtual ~CBottleCheck();
	
   /************************************************************************
	    ���ܣ���ʼ����������
 	/************************************************************************/
	s_Status init(Alg::s_AlgInitParam sAlgInitParam);
	/************************************************************************
	    ���ܣ��ͷż����������Դ
 	/************************************************************************/
	s_Status Free();	
	/************************************************************************
	    ���ܣ��������
	    s_AlgCInP sAlgCInP  ����������
        s_AlgCheckResult **sAlgCheckResult   ����������
 	/************************************************************************/
	s_Status Check(s_AlgCInP sAlgCInP,s_AlgCheckResult **sAlgCheckResult);	
	/************************************************************************
	    ���ܣ����ڳߴ��������ۺ�ʱ���Ʋ�������ǰ���ģ���ļ�
 	/************************************************************************/	
	Alg::s_Status CopySizePara(Alg::s_SizePara4Copy &sSizePara);

public:
	CAlgorithm *m_pAlg;
};

//ģ��ӿ��� 
class DLL_EXT CBottleModel : public CModel
{
public:
	// �����ָ�룬���ڸ��¶Ի��������õ�����
	CBottleCheck *m_pChecker;

	CBottleModel();
	virtual ~CBottleModel();
	
	/************************************************************************
	    ���ܣ����ü��ģ�����
        s_AlgModelPara sAlgModelPara ģ���������ݣ���ʱֻ����ͼ����Ϣ
	    CBottleCheck *pChecker  ��ǰ���ָ��
        CList< s_SetBottleChecker, s_SetBottleChecker &> & sListSetChecker ���м����������-
		        ������������ʹ��
	/************************************************************************/	
	Alg::s_Status SetModelDlg(Alg::s_AlgModelPara sAlgModelPara,CBottleCheck *pChecker,
		list< s_CheckerList> &sCheckerList);

protected:
	CModel *m_pModel;

};
#endif // ROTEMPTYBOTTLE_H
