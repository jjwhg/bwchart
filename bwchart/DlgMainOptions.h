#if !defined(AFX_DLGMAINOPTIONS_H__36BD6A8C_A513_4567_994B_06D2B1FA8F05__INCLUDED_)
#define AFX_DLGMAINOPTIONS_H__36BD6A8C_A513_4567_994B_06D2B1FA8F05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgMainOptions.h : header file
//

#include"dlgoptions.h"
#include"dlgoptwatch.h"
#include"dlgoptionschart.h"

class DlgMainOptions : public CDialog
{
// Construction
public:
	DlgMainOptions(CWnd* pParent = NULL);   // standard constructor
	~DlgMainOptions();

	DlgOptions *Options() {return m_dlgOpt;}
	DlgOptWatch *OptionsW() {return m_dlgOptWatch;}
	DlgOptionsChart *OptionsChart() {return m_dlgOptChart;}

	void SaveValues();

// Dialog Data
	//{{AFX_DATA(DlgMainOptions)
	enum { IDD = IDD_MAIN_OPTIONS };
	CTabCtrl	m_tab;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgMainOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	DlgOptions *m_dlgOpt;
	DlgOptWatch *m_dlgOptWatch;
	DlgOptionsChart *m_dlgOptChart;

	void _Resize();
	void _UpdateScreen(int tab);

	// Generated message map functions
	//{{AFX_MSG(DlgMainOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMAINOPTIONS_H__36BD6A8C_A513_4567_994B_06D2B1FA8F05__INCLUDED_)
