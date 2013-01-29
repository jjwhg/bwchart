#if !defined(AFX_DLGAKAS_H__2711A91B_04C1_4EF6_8C71_2E8CF03CA483__INCLUDED_)
#define AFX_DLGAKAS_H__2711A91B_04C1_4EF6_8C71_2E8CF03CA483__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgAkas.h : header file
//

#include"aka.h"

/////////////////////////////////////////////////////////////////////////////
// DlgAkas dialog

class DlgAkas : public CDialog
{
// Construction
public:
	DlgAkas(CWnd* pParent = NULL);   // standard constructor

	void InitInstance();
	void RefreshAkas();

	AkaList m_akalist;
	AkaList m_akalistMap;
// Dialog Data
	//{{AFX_DATA(DlgAkas)
	enum { IDD = IDD_DLGAKAS };
	CListBox	m_akaMaps;
	CListBox	m_akaPlayers;
	CComboBox	m_players;
	CComboBox	m_maps;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgAkas)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnCancel() {}
	virtual void OnOK() {}
	//}}AFX_VIRTUAL

// Implementation
protected:

	void _FillPlayers(const char *name=0);
	void _FillMaps(const char *name=0);

	// Generated message map functions
	//{{AFX_MSG(DlgAkas)
	afx_msg void OnAddakamap();
	afx_msg void OnAddakaplayer();
	afx_msg void OnAddmap();
	afx_msg void OnAddplayer();
	afx_msg void OnDelmap();
	afx_msg void OnDelplayer();
	afx_msg void OnSelchangeMaps();
	afx_msg void OnSelchangePlayers();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkAkaplayers();
	afx_msg void OnDblclkAkamaps();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGAKAS_H__2711A91B_04C1_4EF6_8C71_2E8CF03CA483__INCLUDED_)
