// RotEmptyBottle.h : main header file for the RotEmptyBottle DLL
//

#if !defined(AFX_RotEmptyBottle_H__C73C8DA9_C273_498C_A159_C2EE408C9D34__INCLUDED_)
#define AFX_RotEmptyBottle_H__C73C8DA9_C273_498C_A159_C2EE408C9D34__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#include <afxtempl.h>
#include "alg_struct.h"

#define DLL_EXT _declspec(dllexport) 
using namespace Alg;

const int RETURN_OK           = 0; //��������
const int RETURN_CHECK_INIT   = 1; //��ʼ������
const int RETURN_MODEL_MANAGE = 2; //ģ��������
const int RETURN_PARA_MANAGE  = 3; //���������ô���
const int RETURN_CHECK_ERROR  = 4; //����㷨����
const int RETURN_CHECK_FREE   = 5; //�ͷŴ���

/////////////////////////////////////////////////////////////////////////////
// CRotEmptyBottleApp
// See RotEmptyBottle.cpp for the implementation of this class
//

class CRotEmptyBottleApp : public CWinApp
{
public:
	CRotEmptyBottleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRotEmptyBottleApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CRotEmptyBottleApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////
/************************************************************************
	���ܣ���ʼ����⶯̬�⣬������������̬�⿪ʼǰ���Ǹı�������������ǰ
    int nWidth, int nHeight������ͼ����
	���Ƕ�����������Ҫͳһ��ʼ���Ĳ���
 
/************************************************************************/
Alg::s_Status DLL_EXT init_bottle_module(int nWidth, int nHeight, int nChannel);
Alg::s_Status DLL_EXT free_bottle_module();
/////////////////////////////////////////////////////////////////////////////

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
	virtual Alg::s_Status SetAlgParam(Alg::s_AlgModelPara sAlgModelPara)=0;
	virtual Alg::s_Status SetCheckParam(Alg::s_AlgCheckPara sAlgCheckPara)=0;
	virtual Alg::s_Status Check(Alg::s_AlgCInP sAlgCInP,Alg::s_AlgCheckResult **sAlgCheckResult)=0;
	virtual Alg::s_Status UpdateParam()=0;
	virtual Alg::s_Status GetAlgInfo(Alg::s_AlgInfo sAlgInfo)=0;
	virtual Alg::s_Status CopySizePara(Alg::s_SizePara4Copy &sSizePara)=0;
	//////////////////////////////////////////////////////////////////////////
	// ����µĵ�������


	//////////////////////////////////////////////////////////////////////////
	

};

// ��������㷨��ӿ���
class DLL_EXT CBottleCheck : public CAlgorithm
{
public:
	CBottleCheck();
	virtual ~CBottleCheck();
	
   /************************************************************************
	    ���ܣ���ʼ������������������
	    CString strTaskPath  ��ǰ����洢·��
        s_AlgInitParam sAlgInitParam   ģ��·�������ڲ�ʹ��
        s_RegionProperty sDstRegionProperty ��������
		����ֵ��	sError.nErrorType = 1;  û��license�ļ�
 	/************************************************************************/
	Alg::s_Status init(Alg::s_AlgInitParam sAlgInitParam);
	/************************************************************************
	    ���ܣ��ͷż����������Դ
 	/************************************************************************/
	Alg::s_Status Free();
	/************************************************************************
	    ���ܣ������������ԶԻ���----��������ǲ����Ѿ���ʹ��
	    s_AlgImageInfo sAlgImageInfo  ��ǰ����洢·��
        s_RegionProperty sRegionProperty   ������
        CList<s_RegionProperty,s_RegionProperty&> &sRegionList ������
 	/************************************************************************/
	Alg::s_Status SetAlgParam(Alg::s_AlgModelPara sAlgModelPara);
	/************************************************************************
	    ���ܣ���������������Ի���---��������ǲ����Ѿ���ʹ��
	    s_AlgImageInfo sAlgImageInfo  ��ǰ����洢·��
        s_RegionProperty sRegionProperty   ������
        CList<s_RegionProperty,s_RegionProperty&> &sRegionList ������
	/************************************************************************/	
	Alg::s_Status SetCheckParam(Alg::s_AlgCheckPara sAlgCheckPara);
	/************************************************************************
	    ���ܣ��������
	    s_AlgCheckInputParam sAlgCheckInputParam  ����������
        s_AlgCheckOutputParam *sAlgCheckOutputResult   ����������
 	/************************************************************************/
	Alg::s_Status Check(Alg::s_AlgCInP sAlgCInP,Alg::s_AlgCheckResult **sAlgCheckResult);
	/************************************************************************
	    ���ܣ���������--��������ǲ����Ѿ���ʹ��
 	/************************************************************************/
	Alg::s_Status UpdateParam();
	/************************************************************************
	    ���ܣ�����㷨��̬����Ϣ����汾��Ϣ��ʹ�������㷨����Ϣ��
 	/************************************************************************/	
	Alg::s_Status GetAlgInfo(Alg::s_AlgInfo sAlgInfo);
	/************************************************************************
	    ���ܣ����ڳߴ��������ۺ�ʱ���Ʋ�������ǰ���ģ���ļ�
 	/************************************************************************/	
	Alg::s_Status CopySizePara(Alg::s_SizePara4Copy &sSizePara);


public:
	CAlgorithm *m_pAlg;
};

//////////////////////////////////////////////////////////////////////////
typedef struct 
{
	int nID;   //����ID
	CBottleCheck  *pChecker;   //��������
}s_CheckerList;

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
	virtual Alg::s_Status init(Alg::s_AlgModelInit sAlgModelInit)=0;
	virtual Alg::s_Status SetModelDlg(Alg::s_AlgModelPara sAlgModelPara,CBottleCheck *pChecker,
		CList< s_CheckerList, s_CheckerList &> & sCheckerList)=0;

};

// �����㷨�������ýӿ���
class DLL_EXT CBottleModel : public CModel
{
public:
	// �����ָ�룬���ڸ��¶Ի��������õ�����
	CBottleCheck *m_pChecker;

	CBottleModel();
	virtual ~CBottleModel();
	/************************************************************************
	    ���ܣ���ʼ������������������
	    sAlgModelInit ��ʼ��ģ�����öԻ������
 	/************************************************************************/
	Alg::s_Status init(Alg::s_AlgModelInit sAlgModelInit);
	/************************************************************************
	    ���ܣ��������������
        s_AlgModelPara sAlgModelPara ������
	    CBottleCheck *pChecker  ��ǰ�������ָ��
        CList< s_SetBottleChecker, s_SetBottleChecker &> & sListSetChecker ���м��������������-
		        ������������ʹ��
	/************************************************************************/
	
	Alg::s_Status SetModelDlg(Alg::s_AlgModelPara sAlgModelPara,CBottleCheck *pChecker,
		CList< s_CheckerList, s_CheckerList &> &sCheckerList);

protected:
	CModel *m_pModel;

};

//////////////////////////////////////////////////////////////////////////
// ������������
class CPara 
{
public:
	CPara()
	{
	}
	virtual ~CPara()
	{
	}
	virtual Alg::s_Status init(Alg::s_AlgCheckInit sAlgInitParam)=0;
	virtual Alg::s_Status SetParamDlg(Alg::s_AlgCheckPara sAlgCheckPara, CBottleCheck *pChecker, 
		CList< s_CheckerList, s_CheckerList &> & sCheckerList)=0;
	virtual Alg::s_Status  UpdateParam(CList< s_CheckerList, s_CheckerList &> & sCheckerList)=0;

};
// �������������ýӿ���
class DLL_EXT CBottlePara : public CPara
{
public:
	// �����ָ�룬���ڸ��¶Ի��������õ�����
	CBottleCheck *m_pChecker;

	CBottlePara();
	virtual ~CBottlePara();
	/************************************************************************
	    ���ܣ���ʼ������������������
	    s_AlgCheckInit sAlgInitParam   ���������ó�ʼ������
 	/************************************************************************/
	Alg::s_Status init(Alg::s_AlgCheckInit sAlgInitParam);
	/************************************************************************
	    ���ܣ��������������
		s_AlgCheckPara sAlgCheckPara  ���������ò���
	    CBottleCheck *pChecker  ��ǰ���������ָ��
        CList< s_SetBottleChecker, s_SetBottleChecker &> & sListSetChecker ���м��������������-
		        ������������ʹ��
	/************************************************************************/
	Alg::s_Status SetParamDlg(Alg::s_AlgCheckPara sAlgCheckPara, CBottleCheck *pChecker,
		CList< s_CheckerList, s_CheckerList &> &sCheckerList);
	/************************************************************************
	    ���ܣ����¼����������������޸ĺ�ǰ�˳�����õĺ���
        CList< s_SetBottleChecker, s_SetBottleChecker &> & sListSetChecker --�������
	/************************************************************************/	
	Alg::s_Status UpdateParam(CList< s_CheckerList, s_CheckerList &> &sCheckerList);

protected:
	CPara *m_pPara;
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RotEmptyBottle_H__C73C8DA9_C273_498C_A159_C2EE408C9D34__INCLUDED_)
