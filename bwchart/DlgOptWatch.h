#if !defined(AFX_DLGOPTWATCH_H__4A59551A_7B0B_4583_8D68_26904DB92E75__INCLUDED_)
#define AFX_DLGOPTWATCH_H__4A59551A_7B0B_4583_8D68_26904DB92E75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgOptWatch.h : header file
//

#define OPTIONSW ((DlgBWChart*)m_pParentWnd)->pGetOptionsW()

class DlgOptWatch : public CDialog
{
// Construction
public:
	DlgOptWatch(CWnd* pParent = NULL);   // standard constructor

	void InnerSaveValues();

// Dialog Data
	//{{AFX_DATA(DlgOptWatch)
	enum { IDD = IDD_DLGOPTIONS1 };
	int		m_timeWait;
	CString	m_keyseq;
	int		m_timeWait2;
	CString	m_keyseq2;
	BOOL	m_autoKeys;
	CString	m_bwplayer;
	BOOL	m_autoStartRWA;
	CString	m_bwexe109;
	CString	m_bwexe110;
	CString	m_bwexe111;
	CString	m_bwexe112;
	CString	m_bwexe113;
	CString	m_bwexe114;
	CString	m_bwexe115;
	CString	m_bwexe116;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgOptWatch)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void _Parameters(bool bLoad);
	void _DefaultPath(CString& path, const char *defexe);
	void _GetStarcraftPath(CString& path);
	void _BrowseExe(CString& path);

	// Generated message map functions
	//{{AFX_MSG(DlgOptWatch)
	afx_msg void OnDestroy();
	afx_msg void OnBrowse();
	afx_msg void OnBrowsedir();
	afx_msg void OnBrowseplayer();
	afx_msg void OnBrowsedirv11();
	afx_msg void OnBrowsedirv12();
	afx_msg void OnBrowsedirv13();
	afx_msg void OnBrowsedirv14();
	afx_msg void OnBrowsedirv15();
	afx_msg void OnBrowsedirv16();
	//}}AFX_MSG
	afx_msg void OnUpdateControl();
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGOPTWATCH_H__4A59551A_7B0B_4583_8D68_26904DB92E75__INCLUDED_)
