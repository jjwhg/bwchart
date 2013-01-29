#if !defined(AFX_DLGFAVORITES_H__9030E42E_CA9A_496C_A4FF_B577616DA93F__INCLUDED_)
#define AFX_DLGFAVORITES_H__9030E42E_CA9A_496C_A4FF_B577616DA93F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgFavorites.h : header file
//

#include "replay.h"
#include "replaydb.h"
#include "bwdb.h"

class DlgFavorites : public CDialog, public BWChartDB
{
// Construction
public:
	DlgFavorites(CWnd* pParent = NULL);   // standard constructor

	void SaveFavorite(ReplayInfo *rep);
	void RemoveFavorite(ReplayInfo *rep, bool rebuildList);
	void _Resize(); 
	void ProcessEntry(const char * section, const char *entry, const char *data, int percentage);
	void Refresh();

// Dialog Data
	//{{AFX_DATA(DlgFavorites)
	enum { IDD = IDD_DLGFAVORITES };
	CListCtrl	m_reps;
	CString	m_replayCount;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgFavorites)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnCancel() {}
	virtual void OnOK() {}
	//}}AFX_VIRTUAL

// Implementation
protected:
	XObArray m_replays;
	ReplayInfo *m_selectedReplay;
	int m_idxSelectedReplay;

	// sorting
	bool m_Descending[10];
	int m_currentSortIdx;

	void _UpdateCounter();
	int _FindReplay(const char *path);
	void _DisplayFavorites();
	void _Load();
	void _SelectedItem(int nItem);
	void _InsertReplay(CListCtrl *reps, class ReplayInfo *rep, int idx, const class DlgFilter* filter=0);

	// sort list
	void _SortReplay(int item);

	// Generated message map functions
	//{{AFX_MSG(DlgFavorites)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblclkReps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickReps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickReps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedReps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditcomment();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	afx_msg void OnWatchReplay();
	afx_msg void OnLoadReplay();
	afx_msg void OnAddReplay();
	afx_msg void OnRemoveFavorites();
	afx_msg void OnUpdateAkas();
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGFAVORITES_H__9030E42E_CA9A_496C_A4FF_B577616DA93F__INCLUDED_)
