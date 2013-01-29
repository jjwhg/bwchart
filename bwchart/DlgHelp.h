#if !defined(AFX_DLGHELP_H__BC5125DF_6BA9_4B2C_BD44_58CA35B0B8FE__INCLUDED_)
#define AFX_DLGHELP_H__BC5125DF_6BA9_4B2C_BD44_58CA35B0B8FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgHelp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgHelp dialog

class DlgHelp : public CDialog
{
// Construction
public:
	DlgHelp(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgHelp)
	enum { IDD = IDD_DLGHELP_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgHelp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgHelp)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGHELP_H__BC5125DF_6BA9_4B2C_BD44_58CA35B0B8FE__INCLUDED_)
