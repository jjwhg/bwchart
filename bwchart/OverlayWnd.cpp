//------------------------------------------------------------------
// OverlayWnd.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "dlgstats.h"
#include "OverlayWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//------------------------------------------------------------------

BEGIN_MESSAGE_MAP(OverlayWnd, CWnd)
	//{{AFX_MSG_MAP(OverlayWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//------------------------------------------------------------------

OverlayWnd::OverlayWnd(DlgStats *parent)
{
	 //::GetStockObject(WHITE_BRUSH)
	CString wndclass = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW,::LoadCursor(NULL, IDC_ARROW), (HBRUSH)0,0);
	CreateEx(WS_EX_TOOLWINDOW,wndclass,"",WS_POPUP|WS_DISABLED,CRect(0,0,100,50),0,0);
	m_mainwnd = parent;
}

//------------------------------------------------------------------

OverlayWnd::~OverlayWnd()
{
}

//--------------------------------------------------------------------------------

static void _GetScreenOneRect(CRect &rectDesk)
{
	// init rect
	rectDesk.SetRectEmpty();
	rectDesk.right = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
	rectDesk.bottom = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

//------------------------------------------------------------------

// change text (adjust window size to text)
void OverlayWnd::SetText(const char *text, CWnd *wnd, CPoint pt)
{
	if(!::IsWindow(m_hWnd)) return;

	//empty text?
	if(text[0]==0) {Show(false);return;}

	// convert x,y to screen coordinates
	wnd->ClientToScreen(&pt);

	// move under the mouse
	pt.y+=28;

	// text changed?
	bool repaint=false;
	if(m_text.Compare(text)!=0)
	{
		// store text
		m_text = text;

		// get text size
		HDC hdc = ::GetDC(0);
		HGDIOBJ old = ::SelectObject(hdc,m_mainwnd->GetLayerFont());
		::GetTextExtentPoint32(hdc,m_text,m_text.GetLength(),&m_size);
		CRect rect(0,0,m_size.cx,m_size.cy);
		::DrawText(hdc,m_text,-1,&rect,DT_CALCRECT|DT_LEFT|DT_TOP);
		m_size.cx = rect.Width();
		m_size.cy = rect.Height();
		::SelectObject(hdc,old);
		::ReleaseDC(0,hdc);

		// adjust window size
		m_size.cx = 4+(int)((float)m_size.cx*0.85f);
		m_size.cy = 4+(int)((float)m_size.cy*0.90f);
		SetWindowPos(&CWnd::wndTop,0,0,m_size.cx,m_size.cy,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

		// repaint
		repaint=true;
	}

	// center on mouse
	pt.x-=m_size.cx/2;

	// adjust if window is out of screen
	CRect rectDesk;
	_GetScreenOneRect(rectDesk);
	int outside = pt.x+m_size.cx - rectDesk.right;
	if(outside>0) pt.x-=outside;
	if(pt.x<20) pt.x=20;

	// adjust window position
	SetWindowPos(&CWnd::wndTop,pt.x,pt.y,0,0,SWP_NOSIZE|SWP_NOACTIVATE);

	// if window is not visible, show it
	if(!IsWindowVisible()) Show(true);

	// repaint
	if(repaint)
	{
		Invalidate(FALSE);
		UpdateWindow();
	}
}


//------------------------------------------------------------------

void OverlayWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// fill background
	CRect rect;
	GetClientRect(&rect);
	dc.FillSolidRect(&rect,RGB(255,255,196));

	// paint info
	int mode = dc.SetBkMode(TRANSPARENT);
	CFont *oldFont = dc.SelectObject(m_mainwnd->GetLayerFont());
	COLORREF clr = dc.SetTextColor(RGB(0,0,0));
	rect.DeflateRect(3,2);
	dc.DrawText(m_text,&rect,DT_LEFT|DT_TOP|DT_NOCLIP);
	rect.InflateRect(3,2);
	dc.SetTextColor(clr);
	dc.SelectObject(oldFont);
	dc.SetBkMode(mode);

	// draw border
	dc.Draw3dRect(&rect,RGB(96,96,96),RGB(96,96,96));
}

//------------------------------------------------------------------

void OverlayWnd::OnClose() {}

//------------------------------------------------------------------
