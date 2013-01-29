//------------------------------------------------------------------
#if !defined(AFX_OVERLAYWND_H__4AE59101_4AB5_46A5_B82A_0E899FFE2AA5__INCLUDED_)
#define AFX_OVERLAYWND_H__4AE59101_4AB5_46A5_B82A_0E899FFE2AA5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OverlayWnd.h : header file
//

class DlgStats;

class OverlayWnd : public CWnd
{
// Construction
public:
	OverlayWnd(DlgStats *parent);
	virtual ~OverlayWnd();

	// show/hide
	void Show(bool val) {if(::IsWindow(m_hWnd)) ShowWindow(val?SW_SHOWNOACTIVATE:SW_HIDE);}

	// move
	void MoveTo(int x, int y) {SetWindowPos(&CWnd::wndTop,x,y,0,0,SWP_NOSIZE);}

	// change text (adjust window size to text)
	void SetText(const char *text, CWnd *wnd, CPoint pt);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(OverlayWnd)
	//}}AFX_VIRTUAL


	// Generated message map functions
protected:
	//{{AFX_MSG(OverlayWnd)
	afx_msg void OnPaint();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CString m_text;
	CSize m_size;
	DlgStats *m_mainwnd;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OVERLAYWND_H__4AE59101_4AB5_46A5_B82A_0E899FFE2AA5__INCLUDED_)
