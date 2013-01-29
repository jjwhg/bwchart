#if !defined(AFX_DLGEDITAKAS_H__A62DE412_9323_4A60_B58A_CDE951D6626F__INCLUDED_)
#define AFX_DLGEDITAKAS_H__A62DE412_9323_4A60_B58A_CDE951D6626F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgEditAkas.h : header file
//

class Aka;
class AkaList;

/////////////////////////////////////////////////////////////////////////////
// DlgEditAkas dialog

class DlgEditAkas : public CDialog
{
// Construction
public:
	enum {PLAYERS,MAPS};
	DlgEditAkas(AkaList *akalist, Aka *player, int type, CWnd* pParent = NULL);   // standard constructor

	Aka *m_player;

// Dialog Data
	//{{AFX_DATA(DlgEditAkas)
	enum { IDD = IDD_DLGEDITAKAS_DIALOG };
	CListBox	m_listSugg;
	CString	m_mainName;
	CString	m_akas;
	int		m_suggType;
	CString	m_searchFor;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgEditAkas)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	AkaList *m_akalist;
	CString m_initialMainName;
	int m_type;
	bool m_bAskForRefresh;

	void _FillSugg();

	// Generated message map functions
	//{{AFX_MSG(DlgEditAkas)
	afx_msg void OnAddaka();
	afx_msg void OnRadio();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeMainname();
	afx_msg void OnChangeSearchfor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEDITAKAS_H__A62DE412_9323_4A60_B58A_CDE951D6626F__INCLUDED_)
