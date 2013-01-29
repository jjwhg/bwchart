#if !defined(AFX_DLGOPTIONS_H__1C78E671_F979_4289_A084_D8DA28711453__INCLUDED_)
#define AFX_DLGOPTIONS_H__1C78E671_F979_4289_A084_D8DA28711453__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgOptions.h : header file
//

#define OPTIONS ((DlgBWChart*)m_pParentWnd)->pGetOptions()

/////////////////////////////////////////////////////////////////////////////
// DlgOptions dialog

class DlgOptions : public CDialog
{
// Construction
public:
	DlgOptions(CWnd* pParent = NULL);   // standard constructor

	void InnerSaveValues();

	static void _GetStarcraftPath(CString& path);
	static void CheckForUpdate(bool automatic);
	static void SetLanguage();

// Dialog Data
	//{{AFX_DATA(DlgOptions)
	enum { IDD = IDD_DLGOPTIONS };
	BOOL	m_autoCheck;
	int		m_savein;
	BOOL	m_fileasso;
	BOOL	m_autoadd;
	int		m_suspectLimit2;
	BOOL	m_autoLoad;
	BOOL	m_autoLoadDB;
	int		m_language;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnCancel() {}
	virtual void OnOK() {}
	//}}AFX_VIRTUAL


// Implementation
protected:

	void _DefaultPath(CString& path, const char *defexe);
	void _Parameters(bool bLoad);

	// Generated message map functions
	//{{AFX_MSG(DlgOptions)
	afx_msg void OnDestroy();
	afx_msg void OnClear();
	afx_msg void OnCheckupdate();
	afx_msg void OnFileasso();
	afx_msg void OnRadiosave();
	afx_msg void OnChangeSuspectLimit();
	afx_msg void OnButton1();
	//}}AFX_MSG
	afx_msg void OnUpdateControl();
	afx_msg void OnLanguageChange();
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGOPTIONS_H__1C78E671_F979_4289_A084_D8DA28711453__INCLUDED_)
