// DlgMap.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgBWChart.h"
#include "DlgMap.h"
#include "DlgStats.h"
#include "replay.h"
#include "gradient.h"
#include "hsvrgb.h"
#include "regparam.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int vspacing = 20;
int barWidth = 0;
const int hplayer = 8;
const int vtop = 0;
const int vbottom = 28;
const int hleft = 8;
const int margin=4;
const int MEASURE_COUNT=3;
const COLORREF clrRatio = RGB(128,128,128); 
const COLORREF clrTime = RGB(0,0,0); 

static CString barName[3];

//----------------------------------------------------------------------------------------

DlgMap::DlgMap(Replay *replay, CFont *ft1, CFont *ft2, CWnd* pParent /*=NULL*/)
	: CDialog(DlgMap::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgMap)
	m_buildingsAlwaysOn = FALSE;
	m_mineralsOn = FALSE;
	//}}AFX_DATA_INIT
	m_replay=replay;
	m_bIsAnimating=false;
	m_time=0;
	m_pLabelBoldFont=ft1;
	m_pLayerFont=ft2;
	m_vtop =0;
	m_vright=0;
	m_rectTime.SetRectEmpty();
	m_timer=0;

	// load parameters
	_Parameters(true);

	barName[0].LoadString(IDS_BUILDS);
	barName[1].LoadString(IDS_MOVES);
	barName[2].LoadString(IDS_UNITS);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgMap::Animate(bool val) 
{
	m_bIsAnimating=val; 
	_ToggleAnimateButtons(!val); 
	if(!val) _StartTimer();
}

//-----------------------------------------------------------------------------------------------------------------

void DlgMap::_ToggleAnimateButtons(bool enable)
{
	CString str;
	str.LoadString(enable ? IDS_ANIMATE:IDS_STOP);
	SetDlgItemText(IDC_ANIMATE,str);
	GetDlgItem(IDC_SPEEDPLUS)->EnableWindow(enable ? FALSE : TRUE);
	GetDlgItem(IDC_SPEEDMINUS)->EnableWindow(enable ? FALSE : TRUE);
	GetDlgItem(IDC_PAUSE)->EnableWindow(enable ? FALSE : TRUE);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgMap::_Parameters(bool bLoad)
{
	PINT("BWCHART_MAP",buildingsAlwaysOn,TRUE);
	PINT("BWCHART_MAP",mineralsOn,TRUE);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgMap)
	DDX_Check(pDX, IDC_BUILDON, m_buildingsAlwaysOn);
	DDX_Check(pDX, IDC_MINERALON, m_mineralsOn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgMap, CDialog)
	//{{AFX_MSG_MAP(DlgMap)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUILDON, OnBuildon)
	ON_BN_CLICKED(IDC_MINERALON, OnBuildon)
	ON_BN_CLICKED(IDC_ANIMATE, OnAnimate)
	ON_BN_CLICKED(IDC_PAUSE, OnPause)
	ON_BN_CLICKED(IDC_SPEEDMINUS, OnSpeedminus)
	ON_BN_CLICKED(IDC_SPEEDPLUS, OnSpeedplus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//----------------------------------------------------------------------------------------

void DlgMap::UpdateTime(unsigned long time)
{
	// repaint map
	InvalidateRect(&m_mapRect,FALSE);
	// if time changed
	if(m_time!=time) 
	{
		// repaint time
		InvalidateRect(&m_rectTime,FALSE);
		// repaint stats
		InvalidateRect(&m_rectStats,FALSE);	
		m_time=time;
	}
	
	UpdateWindow();
}

//----------------------------------------------------------------------------------------

void DlgMap::ResetTime(unsigned long time)
{
	if(m_replay!=0 && m_replay->GetMapAnim()!=0)
	{
		m_replay->GetMapAnim()->Start();
		UpdateTime(time);
	}
}

//----------------------------------------------------------------------------------------

void DlgMap::_PaintAnimatedMap(CDC *pDC, const CRect& rect)
{
 	// game animated map
	ReplayMapAnimated *map = m_replay->GetMapAnim();

	// build map until current time
	int options=0;
	if(m_buildingsAlwaysOn) options|=ReplayMap::BUILDINGS_ON;
	if(m_mineralsOn) options|=ReplayMap::MINERALS_ON;
	HBITMAP hBitmap = map->BuildMap(m_time,options);

	// paint map
	_PaintStaticMap(pDC, rect, hBitmap);
}

//----------------------------------------------------------------------------------------

void DlgMap::_PaintStaticMap(CDC *pDC, const CRect& rect, HBITMAP hBitmap)
{
	CBitmap *bmp = CBitmap::FromHandle(hBitmap);
	
	int w = m_replay->GetMapAnim()->GetWidth();
	int h = m_replay->GetMapAnim()->GetHeight();

	CDC dcMemory;
	dcMemory.CreateCompatibleDC(pDC);
	CBitmap *old=dcMemory.SelectObject(bmp);
	pDC->StretchBlt(rect.left,rect.top,rect.Width(),rect.Height(),&dcMemory,0,0,w,h,SRCCOPY);
	//pDC->FillSolidRect(&rect,RGB(255,0,0));
	dcMemory.SelectObject(old);
}

//----------------------------------------------------------------------------------------

void DlgMap::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// paint map
	if(m_replay!=0 && m_replay->IsDone() && m_replay->GetMapAnim()!=0)
	{
		// compute map rect
		CRect rect;
		GetClientRect(&rect);
		m_mapRect.SetRect(rect.left+hleft,rect.top+m_vtop,rect.right-m_vright,rect.bottom-vbottom);

		// paint map
		_PaintAnimatedMap(&dc, m_mapRect);

		// paint time
		m_rectTime.SetRect(rect.left+94,rect.bottom-vbottom+8,m_mapRect.right,rect.bottom);
		_PaintTime(&dc, m_rectTime);

		// paint player stats
		m_rectStats = m_mapRect; m_rectStats.left=m_mapRect.right; m_rectStats.right = m_rectStats.left + m_vright;
		_PaintPlayersStats(&dc, m_rectStats);		

		// paint players
		GetClientRect(&rect);
		rect.bottom=m_vtop;
		_PaintPlayers(&dc, rect);
	}
}

//----------------------------------------------------------------------------------------

void DlgMap::OnCancel() 
{
	ShowWindow(SW_HIDE);
}

//----------------------------------------------------------------------------------------

void DlgMap::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	int bottom = cy-vbottom+9;
	int left=hleft;
	if(::IsWindow(GetDlgItem(IDC_BUILDON)->GetSafeHwnd()))
		GetDlgItem(IDC_BUILDON)->SetWindowPos(0,left,bottom+1,0,0,SWP_NOSIZE|SWP_NOZORDER);
	left+=65;
	if(::IsWindow(GetDlgItem(IDC_MINERALON)->GetSafeHwnd()))
		GetDlgItem(IDC_MINERALON)->SetWindowPos(0,left,bottom+1,0,0,SWP_NOSIZE|SWP_NOZORDER);
	left+=65;
	
	if(::IsWindow(GetDlgItem(IDC_ANIMATE)->GetSafeHwnd())) 
		GetDlgItem(IDC_ANIMATE)->SetWindowPos(0,left,bottom,49,17,SWP_NOZORDER);
	left+=49;
	if(::IsWindow(GetDlgItem(IDC_SPEEDPLUS)->GetSafeHwnd())) 
		GetDlgItem(IDC_SPEEDPLUS)->SetWindowPos(0,left,bottom,19,17,SWP_NOZORDER);
	left+=19;
	if(::IsWindow(GetDlgItem(IDC_SPEEDMINUS)->GetSafeHwnd())) 
		GetDlgItem(IDC_SPEEDMINUS)->SetWindowPos(0,left,bottom,19,17,SWP_NOZORDER);
	left+=19;
	if(::IsWindow(GetDlgItem(IDC_PAUSE)->GetSafeHwnd())) 
		GetDlgItem(IDC_PAUSE)->SetWindowPos(0,left,bottom,19,17,SWP_NOZORDER);
	Invalidate();
}

//----------------------------------------------------------------------------------------

void DlgMap::_StartTimer()
{
	if(m_timer==0) m_timer=SetTimer(2,125,0);
}

//----------------------------------------------------------------------------------------

void DlgMap::_StopTimer()
{
	if(m_timer>0) KillTimer(m_timer);
	m_timer=0;
}

//----------------------------------------------------------------------------------------

void DlgMap::UpdateReplay(Replay *replay) 
{
	bool refresh = (m_timer!=0) && replay==m_replay;

	m_replay = replay; 
	if(m_replay!=0 && m_replay->GetMapAnim()!=0)
	{
		// start animation
		if(!refresh)
		{
			m_time=0;
			m_replay->GetMapAnim()->Start();

			// update title
			CString title;
			if(m_replay->GetMapAnim()!=0) 
				title.Format("%s (%dx%d)",m_replay->MapName(),m_replay->GetMapAnim()->GetWidth(),m_replay->GetMapAnim()->GetHeight());
			else
				title=m_replay->MapName();
			SetWindowText(title);
		}

		// compute a good window size
		m_vtop = max(2,m_replay->GetEnabledPlayerCount())*vspacing+vtop;
		int w = 5*(m_replay->GetMapAnim()->GetWidth())/2;
		int h = 5*(m_replay->GetMapAnim()->GetHeight())/2+m_vtop+vbottom;

		// horizontal space per bar
		m_vright = (35*w)/100;
		barWidth=0;
		for(;m_replay->GetEnabledPlayerCount()>0;)
		{
			barWidth = m_vright/m_replay->GetEnabledPlayerCount();
			barWidth /= MEASURE_COUNT;
			if(barWidth>32) break; else m_vright+=25;
		}

		// resize window
		SetWindowPos(0,0,0,w+m_vright,h,SWP_NOMOVE|SWP_NOZORDER);

		// start timer
		if(refresh)
			UpdateTime(m_time);
		else
			_StartTimer();
	}
	else
		SetWindowText("");

	// repaint everything
	Invalidate();
}

//----------------------------------------------------------------------------------------

void DlgMap::_PaintPlayers(CDC *pDC, CRect& rect)
{
	// for each player
	CRect prect=rect;
	for(int i=0; i<m_replay->GetPlayerCount(); i++)
	{
		// get event list
		ReplayEvtList *list = m_replay->GetEvtList(i);
		if(!list->IsEnabled()) continue;
		_PaintPlayerName(pDC, prect, list, i, ReplayMap::GetPlayerColor(list->GetPlayerID()));
		prect.OffsetRect(0,vspacing);
	}
}

//----------------------------------------------------------------------------------------

void DlgMap::_PaintStatBar(CDC *pDC, CRect& prect, int val, int peak, COLORREF clr)
{
	// paint bar
	if(peak==0) return;

	// paint bar
	prect.DeflateRect(margin,margin);
	int barHeight = ((prect.Height()-20)*val) / peak;
	CRect barRect=prect;
	barRect.top = barRect.bottom - barHeight;
	//pDC->FillSolidRect(&barRect,clr);
	Gradient::Fill(pDC,barRect,clr,CHsvRgb::Darker(clr,0.6));
	prect.InflateRect(margin,margin);
	
	// set gdi stuff
	int old = pDC->GetBkMode();
	COLORREF oldClr = pDC->GetBkColor();
	CFont *oldFont = pDC->SelectObject(m_pLayerFont);
	COLORREF oldTClr = pDC->SetTextColor(clrTime);

	// paint value
	CString strVal;
	strVal.Format("%d",val);
	pDC->SetBkMode(OPAQUE);
	pDC->SetBkColor(RGB(0,0,0));
	barRect.bottom = barRect.top+2;
	barRect.top = barRect.bottom -18;
	pDC->DrawText(strVal,barRect,DT_CENTER|DT_SINGLELINE|DT_VCENTER);

	// restore every gdi stuff
	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldTClr);
	pDC->SetBkMode(old);
	pDC->SetBkColor(oldClr);
}

//----------------------------------------------------------------------------------------

void DlgMap::_PaintStatName(CDC *pDC, CRect& prect, const char *type)
{
	// set gdi stuff
	int old = pDC->GetBkMode();
	COLORREF oldClr = pDC->GetBkColor();
	CFont *oldFont = pDC->SelectObject(m_pLayerFont);
	COLORREF oldTClr = pDC->SetTextColor(clrTime);

	// paint name
	pDC->SetBkMode(OPAQUE);
	pDC->SetBkColor(RGB(0,0,0));
	pDC->DrawText(type,prect,DT_CENTER|DT_SINGLELINE|DT_VCENTER);

	// restore every gdi stuff
	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldTClr);
	pDC->SetBkMode(old);
	pDC->SetBkColor(oldClr);
}

//----------------------------------------------------------------------------------------

void DlgMap::_PaintPlayersStats(CDC *pDC, CRect& rect)
{
 	// game animated map
	ReplayMapAnimated *map = m_replay->GetMapAnim();

	// fill background
	pDC->FillSolidRect(&rect,RGB(0,0,0));

	// for each player
	int mspacing = m_vright/MEASURE_COUNT;
	for(int i=0,j=0; i<m_replay->GetPlayerCount(); i++)
	{
		// get event list
		ReplayEvtList *list = m_replay->GetEvtList(i);
		if(!list->IsEnabled()) continue;

		// compute bar rect
		CRect prect=rect;
		prect.left = rect.left+barWidth*j;
		prect.right = prect.left+barWidth;
		prect.bottom -=16;
		j++;

		// paint build count
		_PaintStatBar(pDC, prect, map->GetBuildCount(list->GetPlayerID()), map->GetPeakBuildCount(), 
			ReplayMap::GetPlayerColor(list->GetPlayerID()));

		// paint move count
		prect.OffsetRect(mspacing,0);
		_PaintStatBar(pDC, prect, map->GetMoveCount(list->GetPlayerID()), map->GetPeakMoveCount(), 
			ReplayMap::GetPlayerColor(list->GetPlayerID()));

		// paint unit count
		prect.OffsetRect(mspacing,0);
		_PaintStatBar(pDC, prect, map->GetUnitCount(list->GetPlayerID()), map->GetPeakUnitCount(), 
			ReplayMap::GetPlayerColor(list->GetPlayerID()));
	}

	// paint stats names
	for(int i=0;i<MEASURE_COUNT;i++)
	{
		// compute bar rect
		CRect prect=rect;
		prect.left = rect.left+mspacing*i;
		prect.right = prect.left+mspacing;
		prect.top = prect.bottom -16;
		_PaintStatName(pDC, prect, barName[i]);
	}
}

//----------------------------------------------------------------------------------------

void DlgMap::_PaintTime(CDC *pDC, const CRect& rect)
{
	// select font & color
	CFont *oldFont = pDC->SelectObject(m_pLabelBoldFont);
	COLORREF oldClr = pDC->SetTextColor(clrTime);
	COLORREF oldClr2 = pDC->SetBkColor(GetSysColor(COLOR_BTNFACE));
	int oldMode = pDC->SetBkMode(OPAQUE);

	// convert time as string
	CString strTime;
	int sec = m_replay->QueryFile()->QueryHeader()->Tick2Sec(m_time);
	int h = sec/3600; sec-=h*3600;
	int m = sec/60; sec-=m*60;
	strTime.Format("%02d:%02d:%02d",h,m,sec);

	// draw time
	pDC->DrawText(strTime,m_rectTime,DT_RIGHT|DT_SINGLELINE|DT_TOP);

	// restore every gdi stuff
	pDC->SetBkMode(oldMode);
	pDC->SetBkColor(oldClr2);
	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldClr);
}

//----------------------------------------------------------------------------------------

BOOL DlgMap::OnEraseBkgnd(CDC* pDC) 
{
	// erase whole window
	CRect rect;
	GetClientRect(&rect);
	rect.bottom -=20;
	pDC->FillSolidRect(&rect,RGB(0,0,0));
	GetClientRect(&rect);
	rect.top = rect.bottom-20;
	pDC->FillSolidRect(&rect,GetSysColor(COLOR_BTNFACE));
	return TRUE;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgMap::_PaintPlayerName(CDC *pDC, CRect& rect, ReplayEvtList *list, int player, COLORREF clr)
{
	// select font & color
	CFont *oldFont = pDC->SelectObject(m_pLabelBoldFont);
	COLORREF oldClr = pDC->SetTextColor(clr);
	COLORREF bkClr = pDC->GetBkColor();

	// draw player name
	CString pname;
	CRect rectTxt;
	pname.Format("%s (%s)",list->PlayerName(),list->GetRaceStr());
	rectTxt.SetRect(rect.left+hplayer,rect.top+vtop,rect.left+hplayer+200,rect.top+vtop+vspacing);
	pDC->SetTextColor(clr);
	pDC->DrawText(pname,rectTxt,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
	CRect rectRatio=rectTxt;

	// text size
	int afterText = pDC->GetTextExtent(pname).cx;
	
	// just display total actions & APM
	pname.Format("[%d, %d APM]",list->GetEventCount(),list->GetActionPerMinute());
	rectRatio.OffsetRect(afterText+8,0);
	pDC->SelectObject(m_pLayerFont);
	pDC->SetTextColor(clrRatio);
	pDC->SetBkColor(RGB(0,0,0));
	pDC->DrawText(pname,rectRatio,DT_LEFT|DT_SINGLELINE|DT_VCENTER);

	// restore every gdi stuff
	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldClr);
	pDC->SetBkColor(bkClr);
}

//----------------------------------------------------------------------------------------

void DlgMap::OnTimer(UINT nIDEvent) 
{
	if(m_bIsAnimating) _StopTimer();
	else if(IsWindowVisible()) UpdateTime(m_time);
	CDialog::OnTimer(nIDEvent);
}

//----------------------------------------------------------------------------------------

void DlgMap::OnDestroy() 
{
	_StopTimer();
	CDialog::OnDestroy();
}

//----------------------------------------------------------------------------------------

HBRUSH DlgMap::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	pDC->SetTextColor(RGB(225,225,225));
	pDC->SetBkColor(RGB(0,0,0));
	pDC->SetBkMode(TRANSPARENT);

	return (HBRUSH)GetStockObject(BLACK_BRUSH);
}

//----------------------------------------------------------------------------------------

void DlgMap::OnBuildon() 
{
	UpdateData(TRUE);
	// save parameters
	_Parameters(false);
}

//----------------------------------------------------------------------------------------

void DlgMap::_ForwardToParent(UINT id) 
{
	HWND hwnd; 
	DlgStats *parent  = MAINWND->pGetStats();
	parent->GetDlgItem(id,&hwnd);
	parent->SendMessage(WM_COMMAND,MAKEWPARAM(id,BN_CLICKED),(LPARAM)hwnd);
}

//----------------------------------------------------------------------------------------

void DlgMap::OnAnimate() 
{
	_ForwardToParent(IDC_ANIMATE); 
}

void DlgMap::OnPause() 
{
	_ForwardToParent(IDC_PAUSE); 
}

void DlgMap::OnSpeedminus() 
{
	_ForwardToParent(IDC_SPEEDMINUS); 
}

void DlgMap::OnSpeedplus() 
{
	_ForwardToParent(IDC_SPEEDPLUS); 
}

//----------------------------------------------------------------------------------------
