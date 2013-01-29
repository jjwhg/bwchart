#if !defined(AFX_EXPORTCOACHDLG_H__CC9D5B56_D4DE_4BF7_8061_9DC1E694CF38__INCLUDED_)
#define AFX_EXPORTCOACHDLG_H__CC9D5B56_D4DE_4BF7_8061_9DC1E694CF38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportCoachDlg.h : header file
//

class Replay;

/////////////////////////////////////////////////////////////////////////////
// ExportCoachDlg dialog

class ExportCoachDlg : public CDialog
{
// Construction
public:
	ExportCoachDlg(CWnd* pParent, Replay *pReplay);   // standard constructor
	~ExportCoachDlg();

// Dialog Data
	//{{AFX_DATA(ExportCoachDlg)
	enum { IDD = IDD_EXPORTCOACHDLG_DIALOG };
	CListCtrl	m_plStats;
	CString	m_author;
	CString	m_desc;
	int		m_duration;
	CString	m_title;
	BOOL	m_includeUnits;
	BOOL	m_includeWorkers;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ExportCoachDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	Replay *m_pReplay;
	bool m_enabled[64];
	CImageList *m_pImageList;

	// display player stats
	void _DisplayPlayerStats();
	void _ToggleIgnorePlayer();
	bool _GetExportFileName(CString& file);
	bool _ExportTo(const char *file);

	// Generated message map functions
	//{{AFX_MSG(ExportCoachDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTCOACHDLG_H__CC9D5B56_D4DE_4BF7_8061_9DC1E694CF38__INCLUDED_)
