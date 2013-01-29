#if !defined(AFX_DLGMAP_H__DC720A5E_A6E2_416E_9ED8_25627ACF92DF__INCLUDED_)
#define AFX_DLGMAP_H__DC720A5E_A6E2_416E_9ED8_25627ACF92DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgMap.h : header file
//

class Replay;

/////////////////////////////////////////////////////////////////////////////
// DlgMap dialog

class DlgMap : public CDialog
{
// Construction
public:
	DlgMap(Replay *replay, CFont *ft1, CFont *ft2, CWnd* pParent = NULL);   // standard constructor

	void UpdateReplay(Replay *replay);
	void UpdateTime(unsigned long time);
	void ResetTime(unsigned long time);
	void Animate(bool val);
// Dialog Data
	//{{AFX_DATA(DlgMap)
	enum { IDD = IDD_DLGMAP_DIALOG };
	BOOL	m_buildingsAlwaysOn;
	BOOL	m_mineralsOn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgMap)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	Replay* m_replay;
	CRect m_mapRect;
	CRect m_rectTime;
	CRect m_rectStats;
	bool m_bIsAnimating;
	unsigned long m_time;
	CFont *m_pLabelBoldFont;
	CFont *m_pLayerFont;
	int m_vtop;
	int m_vright;
	UINT m_timer;

	void _ForwardToParent(UINT id);
	void _ToggleAnimateButtons(bool enable);
	void _Parameters(bool bLoad);
	void _StartTimer();
	void _StopTimer();
	void _PaintStatName(CDC *pDC, CRect& prect, const char *type);
	void _PaintStatBar(CDC *pDC, CRect& rect, int val, int peak, COLORREF clr);
	void _PaintPlayersStats(CDC *pDC, CRect& rect);
	void _PaintTime(CDC *pDC, const CRect& rect);
	void _PaintAnimatedMap(CDC *pDC, const CRect& rect);
	void _PaintStaticMap(CDC *pDC, const CRect& rect, HBITMAP hBitmap);
	void _PaintPlayerName(CDC *pDC, CRect& rect, class ReplayEvtList *list, int player, COLORREF clr);
	void _PaintPlayers(CDC *pDC, CRect& rect);

	// Generated message map functions
	//{{AFX_MSG(DlgMap)
	afx_msg void OnPaint();
	virtual void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBuildon();
	afx_msg void OnAnimate();
	afx_msg void OnPause();
	afx_msg void OnSpeedminus();
	afx_msg void OnSpeedplus();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMAP_H__DC720A5E_A6E2_416E_9ED8_25627ACF92DF__INCLUDED_)
