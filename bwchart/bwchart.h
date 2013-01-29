// bwchart.h : main header file for the BWCHART application
//

#if !defined(AFX_BWCHART_H__D31879A0_B891_4F1F_BF74_65878E140A98__INCLUDED_)
#define AFX_BWCHART_H__D31879A0_B891_4F1F_BF74_65878E140A98__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CBwchartApp:
// See bwchart.cpp for the implementation of this class
//

class CBwchartApp : public CWinApp
{
public:
	CBwchartApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBwchartApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CBwchartApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int _CommandLineMode();
	int m_exitCode;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BWCHART_H__D31879A0_B891_4F1F_BF74_65878E140A98__INCLUDED_)
