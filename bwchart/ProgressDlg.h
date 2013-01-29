#if !defined(AFX_PROGRESSDLG_H__F64824E3_AE0C_4EF9_AE63_C49999A034A8__INCLUDED_)
#define AFX_PROGRESSDLG_H__F64824E3_AE0C_4EF9_AE63_C49999A034A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressDlg.h : header file
//

#include"resource.h"

/////////////////////////////////////////////////////////////////////////////
// ProgressDlg dialog

class ProgressDlg : public CDialog
{
// Construction
public:
	ProgressDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ProgressDlg)
	enum { IDD = IDD_PROGRESSBAR };
	CProgressCtrl	m_progress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ProgressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ProgressDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSDLG_H__F64824E3_AE0C_4EF9_AE63_C49999A034A8__INCLUDED_)
