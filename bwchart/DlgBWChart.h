#if !defined(AFX_DLGBWCHART_H__0B385931_5C1D_4E39_A97F_5C4C434A1ED1__INCLUDED_)
#define AFX_DLGBWCHART_H__0B385931_5C1D_4E39_A97F_5C4C434A1ED1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgBWChart.h : header file
//

#include "DlgStats.h"
#include "DlgBrowser.h"
#include "DlgFavorites.h"
#include "DlgAkas.h"
#include "DlgMainOptions.h"
#include "newreplay.h"

#define NVERSION "1.04C"
#define NBUILD "Build 1"

#define WM_LOADREPLAY WM_USER+89
#define WM_ADDFAVORITE WM_USER+90
#define WM_AUTOREFRESH WM_USER+91
#define WM_CHECKFORUPDATE WM_USER+92
								 
class DlgBWChart : public CDialog
{
// Construction
public:
	DlgBWChart(const char *cmdline, CWnd* pParent = NULL);   // standard constructor
	~DlgBWChart();

	DlgBrowser* pGetBrowser() const {return m_dlgBrowser;}
	DlgAkas* pGetAkas() const {return m_dlgAkas;}
	DlgOptions* pGetOptions() const {return m_dlgOptions->Options();}
	DlgOptWatch* pGetOptionsW() const {return m_dlgOptions->OptionsW();}
	DlgOptionsChart* pGetOptionsChart() const {return m_dlgOptions->OptionsChart();}
	DlgStats* pGetStats() const {return m_dlgStats;}
	DlgFavorites* pGetFavorites() const {return m_dlgFavs;}	
	bool HasBOFile() const {return m_boFileExists;}
	void SetHasBOFile() {m_boFileExists=true;}

	void UpdateFileAssociation();

// Dialog Data
	//{{AFX_DATA(DlgBWChart)
	enum { IDD = IDD_DLGBWCHART_DIALOG };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgBWChart)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_cmdline;
	HICON m_hIcon;
	DlgStats* m_dlgStats;
	DlgBrowser* m_dlgBrowser;
	DlgFavorites* m_dlgFavs;
	DlgAkas * m_dlgAkas;
	DlgMainOptions *m_dlgOptions;
	UINT m_timer;
	CFont *m_pLayerFont;
	NewReplayNotify m_handler;
	bool m_boFileExists;

	void _CreateFonts();
	void _DestroyFonts();

	void _UpdateScreen(int tab);
	void _Resize(int cx, int cy);

	// associates the .rep extension with us
	bool _RegisterFileType(const char *exename, const char *ext, const char *typeName, const char *typeDesc);
	// unassociates the .rep extension with us
	bool _UnregisterFileType(const char *typeName);

	// Generated message map functions
	//{{AFX_MSG(DlgBWChart)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMenuAkas();
	afx_msg void OnMenuBrowser();
	afx_msg void OnMenuCharts();
	afx_msg void OnMenuFavorites();
	afx_msg void OnMenuOptions();
	//}}AFX_MSG
	afx_msg LRESULT OnRemoteLoadReplay(WPARAM w, LPARAM lError);
	afx_msg LRESULT OnRemoteAddFavorite(WPARAM w, LPARAM lClear);
	afx_msg LRESULT OnCheckForUpdate(WPARAM w, LPARAM lClear);
	DECLARE_MESSAGE_MAP()
};

#define MAINWND ((DlgBWChart*)AfxGetMainWnd())

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGBWCHART_H__0B385931_5C1D_4E39_A97F_5C4C434A1ED1__INCLUDED_)
