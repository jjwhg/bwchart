#if !defined(AFX_DLGFASTAKA_H__50A2DB73_AFDD_495C_ACC1_BB922EDFC3BE__INCLUDED_)
#define AFX_DLGFASTAKA_H__50A2DB73_AFDD_495C_ACC1_BB922EDFC3BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgFastAka.h : header file
//

class ReplayInfo;

/////////////////////////////////////////////////////////////////////////////
// DlgFastAka dialog

class DlgFastAka : public CDialog
{
// Construction
public:
	DlgFastAka(ReplayInfo *replay, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgFastAka)
	enum { IDD = IDD_DLGFASTAKA_DIALOG };
	CComboBox	m_maps;
	CComboBox	m_players2;
	CComboBox	m_players1;
	CString	m_player2;
	CString	m_player1;
	CString	m_map;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgFastAka)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	ReplayInfo *m_replay;
	bool m_bAkasModified;

	void _DisableCombo(CComboBox *combo, UINT btid, UINT txid);
	void _AddAka(CComboBox *combo, const char *newaka, class AkaList *akalist, UINT btid, UINT txid) ;
	void _PrepareCombo(CComboBox *combo, class AkaList *akalist, const char *straka, const char *regentry, UINT btid, UINT txid);

	// Generated message map functions
	//{{AFX_MSG(DlgFastAka)
	afx_msg void OnAsso1();
	afx_msg void OnAsso2();
	afx_msg void OnAsso3();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGFASTAKA_H__50A2DB73_AFDD_495C_ACC1_BB922EDFC3BE__INCLUDED_)
