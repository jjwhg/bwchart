#if !defined(AFX_DLGRENAME_H__C73E8E05_3CCD_4235_A395_B4FE1508ACD3__INCLUDED_)
#define AFX_DLGRENAME_H__C73E8E05_3CCD_4235_A395_B4FE1508ACD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgRename.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgRename dialog

class DlgRename : public CDialog
{
// Construction
public:
	DlgRename(CListCtrl* reps, CObArray *filter, CWnd* pParent = NULL);   // standard constructor
	~DlgRename();

// Dialog Data
	//{{AFX_DATA(DlgRename)
	enum { IDD = IDD_DLGRENAME_DIALOG };
	CListCtrl	m_replays;
	CProgressCtrl	m_progress;
	CString	m_format;
	CString	m_userdef1;
	CString	m_replacement1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgRename)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// filtered replays
	CListCtrl*	m_reps;
	CObArray *m_filterReplays;
	UINT m_timer;

	void _Display();
	const char *_NewName(class ReplayInfo *rep);

	// Generated message map functions
	//{{AFX_MSG(DlgRename)
	afx_msg void OnDefault();
	afx_msg void OnRename();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFormat();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGRENAME_H__C73E8E05_3CCD_4235_A395_B4FE1508ACD3__INCLUDED_)
