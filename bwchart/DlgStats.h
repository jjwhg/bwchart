// DlgStats.h : header file
//

#if !defined(AFX_DLGSTATS_H__BEA25FD2_E539_4E83_88B6_5B8C24F8AE4D__INCLUDED_)
#define AFX_DLGSTATS_H__BEA25FD2_E539_4E83_88B6_5B8C24F8AE4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include"replay.h"
#include"xlistctrl.h"

class DlgMap;
class OverlayWnd;

//-----------------------------------------------------------------------------------------------------------------

class DrawingTools
{
public:
	enum{maxcurve=64};
	CPen *m_penMineral[maxcurve];
	CPen *m_penMineralDark[maxcurve];
	CPen *m_penMineralS[maxcurve];
	COLORREF m_clr[maxcurve];

	DrawingTools() {for(int i=0; i<maxcurve;i++) m_penMineralDark[i]=m_penMineralS[i]=m_penMineral[i]=0;}
	~DrawingTools() {Clear();}
	void Clear()
	{
		for(int i=0; i<maxcurve;i++) 
		{
			delete m_penMineral[i]; 
			m_penMineral[i]=0;
			delete m_penMineralS[i]; 
			m_penMineralS[i]=0;
			delete m_penMineralDark[i]; 
			m_penMineralDark[i]=0;
		}
	}
};

//-----------------------------------------------------------------------------------------------------------------

class DlgStats : public CDialog
{
// Construction
public:
	DlgStats(CWnd* pParent = NULL);	// standard constructor
	~DlgStats();

	void LoadReplay(const char *reppath, bool bClear);
	void StopAnimation();
	void UpdateBkgBitmap();

	CFont *GetLabelFont() const {return m_pLabelFont;}
	CFont *GetSmallFont() const {return m_pSmallFont;}
	CFont *GetLayerFont() const {return m_pLayerFont;}
	CFont *GetLabelBoldFont() const {return m_pLabelBoldFont;}

// Dialog Data
	//{{AFX_DATA(DlgStats)
	enum { IDD = IDD_BWCHART_DIALOG };
	CProgressCtrl	m_progress;
	CStatic	m_gamed;
	CStatic	m_dlbclick;
	CListCtrl	m_plStats;
	CScrollBar	m_scrollerV;
	CScrollBar	m_scroller;
	CXListCtrl	m_listEvents;
	int		m_zoom;
	BOOL	m_seeMinerals;
	BOOL	m_seeGaz;
	BOOL	m_seeSupply;
	BOOL	m_seeActions;
	BOOL	m_useSeconds;
	BOOL	m_seeSpeed;
	int		m_chartType;
	BOOL	m_seeUnitsOnBO;
	CString	m_exactTime;
	BOOL	m_seeBPM;
	BOOL	m_seeUPM;
	BOOL	m_seeHotPoints;
	BOOL	m_seePercent;
	BOOL	m_viewHKselect;
	BOOL	m_fltSelect;
	BOOL	m_fltSuspect;
	BOOL    m_fltHack;
	BOOL	m_fltTrain;
	BOOL	m_fltBuild;
	BOOL	m_fltOthers;
	BOOL	m_fltChat;
	CString	m_suspectInfo;
	BOOL	m_sortDist;
	//}}AFX_DATA

	enum {APM,RESOURCES,UNITS,ACTIONS,BUILDINGS,UPGRADES,APMDIST,BUILDORDER,HOTKEYS,MAPCOVERAGE,
			MIX_APMHOTKEYS, __MAXCHART};

	BOOL	m_singleChart[__MAXCHART];
	BOOL	m_seeUnits[__MAXCHART];
	int		m_apmStyle[__MAXCHART];;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgStats)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void OnCancel() {}
	virtual void OnOK() {}
	//}}AFX_VIRTUAL

// Implementation
protected:
	Replay m_replay;
	CRect m_boardRect;
	CFont *m_pLabelFont;
	CFont *m_pSmallFont;
	CFont *m_pLayerFont;
	CFont *m_pLabelBoldFont;
	CSize m_lineDev;
	CSize m_pageDev;
	CImageList *m_pImageList;
	ReplayEvtList *m_selectedPlayerList;
	DlgMap *m_dlgmap;
	bool m_bIsAnimating;
	UINT m_timer;

	// rect for mixed charts
	enum {MAXMIXEDCHART=4};
	CRect m_mixedRect[MAXMIXEDCHART];
	int m_mixedChartType[MAXMIXEDCHART];
	int m_mixedCount;
	int m_MixedPlayerIdx;

	//overlay wnd
	OverlayWnd *m_over;

	// current list being drawn
	ReplayEvtList* m_list;

	// resize
	enum {NONE=0,VERTICAL_RESIZE=1, HORIZONTAL_RESIZE=2};
	int m_resizing;
	int m_hlist;
	int m_wlist;
	int m_ystart;
	int m_xstart;

	// tools for drawing charts
	enum {MAXPLAYER=32};
	DrawingTools m_dtools[MAXPLAYER];

	unsigned long m_timeBegin;
	unsigned long m_timeEnd;
	unsigned long m_timeCursor;
	bool m_lockListView;
	int m_animationSpeed; //1=realtime
	int m_prevAnimationSpeed;

	int m_maxPlayerOnBoard;
	int m_selectedPlayer;
	int m_selectedAction;

	//ratios for values/pixels
	// compute all ratios
	float m_fvinc[DrawingTools::maxcurve];
	float m_finc;
	int m_dataAreaX;

	// map title
	CRect m_rectMapName;
						 
	// event symbol shapes	
	typedef enum {
		SYMBOLMIN=0,
		SQUARE,
		RECTANGLE,
		FLAG,
		FLAGEMPTY,
		TEALEFT,
		TEARIGHT,
		TEATOP,
		TEABOTTOM,
		ROUNDRECT,
		SYMBOLMAX} eSHAPE;
	eSHAPE m_shape;

	// Generated message map functions
	//{{AFX_MSG(DlgStats)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnGetevents();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnItemchangedListevents(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSelchangeZoom();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDblclkPlstats(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateChart();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeCharttype();
	afx_msg void OnHelp();
	afx_msg void OnTestreplays();
	afx_msg void OnAddevent();
	afx_msg void OnRclickPlstats(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSeemap();
	afx_msg void OnAnimate();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPause();
	afx_msg void OnSpeedminus();
	afx_msg void OnSpeedplus();
	afx_msg void OnGetdispinfoListevents(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRclickListevents(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFilterChange();
	afx_msg void OnNextSuspect();
	afx_msg void OnSelchangeApmstyle();
	//}}AFX_MSG
	afx_msg void OnRemovePlayer();
	afx_msg void OnEnableDisable();
	afx_msg void OnWatchReplay116();
	afx_msg void OnWatchReplay115();
	afx_msg void OnWatchReplay114();
	afx_msg void OnWatchReplay113();
	afx_msg void OnWatchReplay112();
	afx_msg void OnWatchReplay111();
	afx_msg void OnWatchReplay110();
	afx_msg void OnWatchReplay109();
	afx_msg void OnWatchReplaySC();
	afx_msg void OnWatchReplay();
	afx_msg void OnExportToText();
	afx_msg void OnExportToHtml();
	afx_msg void OnExportToBWCoach();
	DECLARE_MESSAGE_MAP()

	void _StartCurrentReplay(int mode);
	void _PaintCheckBoxColor(CDC *pDC);
	ReplayEvt *_GetEventFromIdx(unsigned long idx);
	void _PaintHotPoint(CDC *pDC, int x, int y, const ReplayEvt *evt, COLORREF clr);
	void _GetCursorRect(CRect& rect, unsigned long time, int width);
	void _ToggleAnimateButtons(bool enable);
	void _UpdateAnimSpeed();
	void _Parameters(bool bLoad);
	bool _GetListViewContent(HWND hlv);
	void _GetReplayEvents();
	void _PrepareListView();
	void _PaintChart(CDC *pDC, int chartType, const CRect& rect, int minMargin=0);
	void _PaintCharts(CDC *pDC);
	void _PaintMixedCharts(CDC *pDC, int playerIdx, int *chartType, int count);
	void _PaintMultipleCharts(CDC *pDC);
	void _PaintSingleChart(CDC *pDC);
	void _PaintEvents(CDC *pDC, int chartType, const CRect& rect, const ReplayResource& resmax, int player=-1);
	void _PaintOneEvent(CDC *pDC,const CRect* pRect, const ReplayEvt *evt);
	void _PaintCursor(CDC *pDC, const CRect& rect);
	void _PaintPlayerName(CDC *pDC, const CRect& rect, ReplayEvtList *list, int playerIdx, int playerPos, COLORREF clr, int offv=0);
	void _PaintGrid(CDC *pDC, const CRect& rect, const ReplayResource& resmax);
	void _PaintActionsName(CDC *pDC, const CRect& rect);
	enum {MSK_XLINE=1, MSK_YLEFTLINE=2, MSK_YRIGHTLINE=4,MSK_ALL=255};
	void _PaintAxis(CDC *pDC, const CRect& rect, const ReplayResource& resmax, int mask,
		unsigned long timeBegin, unsigned long timeEnd);
	void _PaintAxisLines(CDC *pDC, const CRect& rect, int mask=255);
	void _PaintBackgroundLayer(CDC *pDC, const CRect& rect, const ReplayResource& resmax);
	void _PaintForegroundLayer(CDC *pDC, const CRect& rect, const ReplayResource& resmax, int mask=255);
	void _PaintMapName(CDC *pDC, CRect& rect);
	void _PaintResources2(CDC *pDC, int ybottom, int cx, DrawingTools *tools, ReplayResource *res, unsigned long time, class CKnotArray& knot, int knotidx);

	// paint a spline curve
	void _PaintSpline(CDC *pDC, const class CKnotArray& knots, int curveidx);

	void _CreateFonts();
	void _DestroyFonts();
	void _Resize(int cx, int cy);
	unsigned long _GetEventFromTime(unsigned long cursor);
	void _SetTimeCursor(unsigned long value, bool bUpdateListView=true, bool bIgnoreAnim=true);
	BOOL _OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll);
	BOOL _OnScrollBy(CSize sizeScroll, BOOL bDoScroll);
	void _AdjustWindow();
	void _DisplayPlayerStats();
	void _ToggleIgnorePlayer();
	void _UpdateActionFilter(bool refresh);
	void _PaintDistribution(CDC *pDC, const CRect& rect, int type);
	void _PaintBuildOrder(CDC *pDC, const CRect& rect);
	void _PaintHotKeys(CDC *pDC, const CRect& rect);
	void _PaintHotKeyEvent(CDC *pDC, CRect& rect, int offset);
	void _BrowseReplays(const char *dir, int& count, int &idx, bool bCount);
	bool _GetReplayFileName(CString& path);
	void _PaintBO(CDC *pDC, const CRect& rect, const ReplayObjectSequence& bo, int offset);
	void _PaintSentinel(CDC *pDC, CRect& rect);
	void _ComputeHotkeySymbolRect(const ReplayEvtList *list, const HotKeyEvent *hkevt, CRect& datarect, CRect& symRect);
	void _GetDataRectForPlayer(int plidx, CRect& rect, int pcount);
	void _CheckForHotKey(ReplayEvtList *list, CPoint& point, CRect& datarect, int delta=0);
	void _GetHotKeyDesc(ReplayEvtList *list, int slot, CString& info);
	inline unsigned long _GetActionCount();
	bool _GetFileName(const char *filter, const char *ext, const char *def, CString& file);
	void _SelectAction(int idx);

	DrawingTools* _GetDrawingTools(int player);
	void _InitDrawingTools(int player, int maxplayer, int lineWidth);
	void _GetResizeRect(CRect& resizeRect);
	void _GetHorizResizeRect(CRect& resizeRect);

	// init all pens for drawing all the charts
	void _InitAllDrawingTools();

	// get index of first enabled player
	int _GetFirstEnabledPlayer() const;

	// compute vertical increment for grid and axis
	float _ComputeGridInc(unsigned long& tminc, unsigned long& maxres, const CRect& rect, const ReplayResource& resmax, int chartType, bool useGivenMax=false) const;

	// draw a local maximum
	void _DrawLocalMax(CDC *pDC, int rx, int ry, const COLORREF clr, int& lastMaxX, int maxval);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSTATS_H__BEA25FD2_E539_4E83_88B6_5B8C24F8AE4D__INCLUDED_)
