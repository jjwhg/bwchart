#if !defined(AFX_DLGEDITCOMMENT_H__4BBA8B88_C8AE_423E_863B_5870605D75B5__INCLUDED_)
#define AFX_DLGEDITCOMMENT_H__4BBA8B88_C8AE_423E_863B_5870605D75B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgEditComment.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgEditComment dialog

class DlgEditComment : public CDialog
{
// Construction
public:
	DlgEditComment(class ReplayInfo *rep, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgEditComment)
	enum { IDD = IDD_DLGEDITCOMMENT_DIALOG };
	CString	m_comment;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgEditComment)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	class ReplayInfo *m_rep;

	// Generated message map functions
	//{{AFX_MSG(DlgEditComment)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEDITCOMMENT_H__4BBA8B88_C8AE_423E_863B_5870605D75B5__INCLUDED_)
