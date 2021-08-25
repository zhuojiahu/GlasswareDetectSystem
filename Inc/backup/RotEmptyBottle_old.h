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

const int RETURN_OK           = 0; //返回正常
const int RETURN_CHECK_INIT   = 1; //初始化错误
const int RETURN_MODEL_MANAGE = 2; //模板管理错误
const int RETURN_PARA_MANAGE  = 3; //检测参数设置错误
const int RETURN_CHECK_ERROR  = 4; //检测算法出错
const int RETURN_CHECK_FREE   = 5; //释放错误

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
	功能：初始化检测动态库，必须在整个动态库开始前或是改变任务后调用区域前
    int nWidth, int nHeight该任务图像宽高
	这是对于所有类需要统一初始化的参数
 
/************************************************************************/
Alg::s_Status DLL_EXT init_bottle_module(int nWidth, int nHeight, int nChannel);
Alg::s_Status DLL_EXT free_bottle_module();
/////////////////////////////////////////////////////////////////////////////

// 检测抽象类
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
	// 添加新的导出函数


	//////////////////////////////////////////////////////////////////////////
	

};

// 导出检测算法库接口类
class DLL_EXT CBottleCheck : public CAlgorithm
{
public:
	CBottleCheck();
	virtual ~CBottleCheck();
	
   /************************************************************************
	    功能：初始化检测区域参数设置类
	    CString strTaskPath  当前任务存储路径
        s_AlgInitParam sAlgInitParam   模板路径，现在不使用
        s_RegionProperty sDstRegionProperty 区域属性
		返回值：	sError.nErrorType = 1;  没有license文件
 	/************************************************************************/
	Alg::s_Status init(Alg::s_AlgInitParam sAlgInitParam);
	/************************************************************************
	    功能：释放检测类分配的资源
 	/************************************************************************/
	Alg::s_Status Free();
	/************************************************************************
	    功能：弹出区域属性对话框----这个现在是不是已经不使用
	    s_AlgImageInfo sAlgImageInfo  当前任务存储路径
        s_RegionProperty sRegionProperty   待补充
        CList<s_RegionProperty,s_RegionProperty&> &sRegionList 待补充
 	/************************************************************************/
	Alg::s_Status SetAlgParam(Alg::s_AlgModelPara sAlgModelPara);
	/************************************************************************
	    功能：弹出区域检测参数对话框---这个现在是不是已经不使用
	    s_AlgImageInfo sAlgImageInfo  当前任务存储路径
        s_RegionProperty sRegionProperty   待补充
        CList<s_RegionProperty,s_RegionProperty&> &sRegionList 待补充
	/************************************************************************/	
	Alg::s_Status SetCheckParam(Alg::s_AlgCheckPara sAlgCheckPara);
	/************************************************************************
	    功能：质量检测
	    s_AlgCheckInputParam sAlgCheckInputParam  检测输入参数
        s_AlgCheckOutputParam *sAlgCheckOutputResult   检测输出参数
 	/************************************************************************/
	Alg::s_Status Check(Alg::s_AlgCInP sAlgCInP,Alg::s_AlgCheckResult **sAlgCheckResult);
	/************************************************************************
	    功能：参数更新--这个现在是不是已经不使用
 	/************************************************************************/
	Alg::s_Status UpdateParam();
	/************************************************************************
	    功能：获得算法动态库信息，如版本信息，使用其他算法库信息等
 	/************************************************************************/	
	Alg::s_Status GetAlgInfo(Alg::s_AlgInfo sAlgInfo);
	/************************************************************************
	    功能：用于尺寸检测数据综合时复制参数到当前相机模板文件
 	/************************************************************************/	
	Alg::s_Status CopySizePara(Alg::s_SizePara4Copy &sSizePara);


public:
	CAlgorithm *m_pAlg;
};

//////////////////////////////////////////////////////////////////////////
typedef struct 
{
	int nID;   //区域ID
	CBottleCheck  *pChecker;   //检测类对象
}s_CheckerList;

// 模板抽象类
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

// 导出算法属性设置接口类
class DLL_EXT CBottleModel : public CModel
{
public:
	// 检测类指针，用于更新对话框上设置的数据
	CBottleCheck *m_pChecker;

	CBottleModel();
	virtual ~CBottleModel();
	/************************************************************************
	    功能：初始化检测区域参数设置类
	    sAlgModelInit 初始化模板设置对话框参数
 	/************************************************************************/
	Alg::s_Status init(Alg::s_AlgModelInit sAlgModelInit);
	/************************************************************************
	    功能：设置区域检测参数
        s_AlgModelPara sAlgModelPara 待补充
	    CBottleCheck *pChecker  当前检测区域指针
        CList< s_SetBottleChecker, s_SetBottleChecker &> & sListSetChecker 所有检测区域链表，用于-
		        更新所有区域使用
	/************************************************************************/
	
	Alg::s_Status SetModelDlg(Alg::s_AlgModelPara sAlgModelPara,CBottleCheck *pChecker,
		CList< s_CheckerList, s_CheckerList &> &sCheckerList);

protected:
	CModel *m_pModel;

};

//////////////////////////////////////////////////////////////////////////
// 检测参数抽象类
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
// 导出检测参数设置接口类
class DLL_EXT CBottlePara : public CPara
{
public:
	// 检测类指针，用于更新对话框上设置的数据
	CBottleCheck *m_pChecker;

	CBottlePara();
	virtual ~CBottlePara();
	/************************************************************************
	    功能：初始化检测区域参数设置类
	    s_AlgCheckInit sAlgInitParam   检测参数设置初始化参数
 	/************************************************************************/
	Alg::s_Status init(Alg::s_AlgCheckInit sAlgInitParam);
	/************************************************************************
	    功能：设置区域检测参数
		s_AlgCheckPara sAlgCheckPara  检测参数设置参数
	    CBottleCheck *pChecker  当前检测区域类指针
        CList< s_SetBottleChecker, s_SetBottleChecker &> & sListSetChecker 所有检测区域链表，用于-
		        更新所有区域使用
	/************************************************************************/
	Alg::s_Status SetParamDlg(Alg::s_AlgCheckPara sAlgCheckPara, CBottleCheck *pChecker,
		CList< s_CheckerList, s_CheckerList &> &sCheckerList);
	/************************************************************************
	    功能：更新检测参数，当服务器修改后前端程序调用的函数
        CList< s_SetBottleChecker, s_SetBottleChecker &> & sListSetChecker --检测链表
	/************************************************************************/	
	Alg::s_Status UpdateParam(CList< s_CheckerList, s_CheckerList &> &sCheckerList);

protected:
	CPara *m_pPara;
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RotEmptyBottle_H__C73C8DA9_C273_498C_A159_C2EE408C9D34__INCLUDED_)
