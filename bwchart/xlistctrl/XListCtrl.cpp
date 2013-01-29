// XListCtrl.cpp  Version 1.3
//
// Author:  Hans Dietrich
//          hdietrich2@hotmail.com
//
// This code is based on "Neat Stuff to do in List Controls Using Custom Draw"
// by Michael Dunn. See http://www.codeproject.com/listctrl/lvcustomdraw.asp
//
// Thanks to David Patrick for pointing out how to subclass header control
// if CXListCtrl is created via Create() instead of via dialog template.
//
// This software is released into the public domain.
// You are free to use it in any way you like.
//
// This software is provided "as is" with no expressed
// or implied warranty.  I accept no liability for any
// damage or loss of business that this software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT NEAR WM_XLISTCTRL_COMBO_SELECTION  = ::RegisterWindowMessage(_T("WM_XLISTCTRL_COMBO_SELECTION"));
UINT NEAR WM_XLISTCTRL_CHECKBOX_CLICKED = ::RegisterWindowMessage(_T("WM_XLISTCTRL_CHECKBOX_CLICKED"));

/////////////////////////////////////////////////////////////////////////////
// CXListCtrl

BEGIN_MESSAGE_MAP(CXListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CXListCtrl)
	ON_NOTIFY_REFLECT_EX(LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_PAINT()
	ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
#ifndef NO_XLISTCTRL_TOOL_TIPS
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
#endif
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////////
// ctor
CXListCtrl::CXListCtrl()
{
	m_dwExtendedStyleX      = 0;
	m_bHeaderIsSubclassed   = FALSE;

	m_cr3DFace              = ::GetSysColor(COLOR_3DFACE);
	m_cr3DHighLight         = ::GetSysColor(COLOR_3DHIGHLIGHT);
	m_cr3DShadow            = ::GetSysColor(COLOR_3DSHADOW);
	m_crBtnFace             = ::GetSysColor(COLOR_BTNFACE);
	m_crBtnShadow           = ::GetSysColor(COLOR_BTNSHADOW);
	m_crBtnText             = ::GetSysColor(COLOR_BTNTEXT);
	m_crGrayText            = ::GetSysColor(COLOR_GRAYTEXT);
	m_crHighLight           = ::GetSysColor(COLOR_HIGHLIGHT);
	m_crHighLightText       = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_crWindow              = ::GetSysColor(COLOR_WINDOW);
	m_crWindowText          = ::GetSysColor(COLOR_WINDOWTEXT);
}

///////////////////////////////////////////////////////////////////////////////
// dtor
CXListCtrl::~CXListCtrl()
{
}

///////////////////////////////////////////////////////////////////////////////
// PreSubclassWindow
void CXListCtrl::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();

	// for Dialog based applications, this is a good place
	// to subclass the header control because the OnCreate()
	// function does not get called.

	//SubclassHeaderControl();
}

///////////////////////////////////////////////////////////////////////////////
// OnCreate
int CXListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
	{
		ASSERT(FALSE);
		return -1;
	}

	// When the CXListCtrl object is created via a call to Create(), instead
	// of via a dialog box template, we must subclass the header control
	// window here because it does not exist when the PreSubclassWindow()
	// function is called.

	//SubclassHeaderControl();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// SubclassHeaderControl
void CXListCtrl::SubclassHeaderControl()
{
	if (m_bHeaderIsSubclassed)
		return;

	// if the list control has a header control window, then
	// subclass it

	// Thanks to Alberto Gattegno and Alon Peleg  and their article
	// "A Multiline Header Control Inside a CListCtrl" for easy way
	// to determine if the header control exists.

	CHeaderCtrl* pHeader = GetHeaderCtrl();
	if (pHeader)
	{
		VERIFY(m_HeaderCtrl.SubclassWindow(pHeader->m_hWnd));
		m_bHeaderIsSubclassed = TRUE;
	}
}

///////////////////////////////////////////////////////////////////////////////
// OnCustomDraw
void CXListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

	// Take the default processing unless we set this to something else below.
	*pResult = CDRF_DODEFAULT;

	// First thing - check the draw stage. If it's the control's prepaint
	// stage, then tell Windows we want messages for every item.

	if (pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		// This is the notification message for an item.  We'll request
		// notifications before each subitem's prepaint stage.

		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if (pLVCD->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM))
	{
		// This is the prepaint stage for a subitem. Here's where we set the
		// item's text and background colors. Our return value will tell
		// Windows to draw the subitem itself, but it will use the new colors
		// we set here.

		int nItem = static_cast<int> (pLVCD->nmcd.dwItemSpec);
		int nSubItem = pLVCD->iSubItem;

		XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) pLVCD->nmcd.lItemlParam;
		//ASSERT(pXLCD);

		COLORREF crText  = m_crWindowText;
		COLORREF crBkgnd = m_crWindow;

		if (pXLCD)
		{
			crText  = pXLCD[nSubItem].crText;
			crBkgnd = pXLCD[nSubItem].crBackground;

			if (!pXLCD[0].bEnabled)
				crText = m_crGrayText;
		}

		// store the colors back in the NMLVCUSTOMDRAW struct
		pLVCD->clrText = crText;
		pLVCD->clrTextBk = crBkgnd;

		CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
		CRect rect;
		GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rect);

		{
			rect.left += DrawImage(nItem, nSubItem, pDC, crText, crBkgnd, rect, pXLCD);

			DrawText(nItem, nSubItem, pDC, crText, crBkgnd, rect, pXLCD);

			*pResult = CDRF_SKIPDEFAULT;	// We've painted everything.
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// GetDrawColors
void CXListCtrl::GetDrawColors(int nItem,
							   int nSubItem,
							   COLORREF& colorText,
							   COLORREF& colorBkgnd)
{
	DWORD dwStyle    = GetStyle();
	DWORD dwExStyle  = GetExtendedStyle();

	COLORREF crText  = colorText;
	COLORREF crBkgnd = colorBkgnd;

	if (GetItemState(nItem, LVIS_SELECTED))
	{
		if (dwExStyle & LVS_EX_FULLROWSELECT)
		{
			// selected?  if so, draw highlight background
			crText  = m_crHighLightText;
			crBkgnd = m_crHighLight;

			// has focus?  if not, draw gray background
			if (m_hWnd != ::GetFocus())
			{
				if (dwStyle & LVS_SHOWSELALWAYS)
				{
					crText  = m_crWindowText;
					crBkgnd = m_crBtnFace;
				}
				else
				{
					crText  = colorText;
					crBkgnd = colorBkgnd;
				}
			}
		}
		else	// not full row select
		{
			if (nSubItem == 0)
			{
				// selected?  if so, draw highlight background
				crText  = m_crHighLightText;
				crBkgnd = m_crHighLight;

				// has focus?  if not, draw gray background
				if (m_hWnd != ::GetFocus())
				{
					if (dwStyle & LVS_SHOWSELALWAYS)
					{
						crText  = m_crWindowText;
						crBkgnd = m_crBtnFace;
					}
					else
					{
						crText  = colorText;
						crBkgnd = colorBkgnd;
					}
				}
			}
		}
	}

	colorText = crText;
	colorBkgnd = crBkgnd;
}

///////////////////////////////////////////////////////////////////////////////
// DrawImage
int CXListCtrl::DrawImage(int nItem,
						  int nSubItem,
						  CDC* pDC,
						  COLORREF crText,
						  COLORREF crBkgnd,
						  CRect rect,
  						  XLISTCTRLDATA *pXLCD)
{
	GetDrawColors(nItem, nSubItem, crText, crBkgnd);

	pDC->FillSolidRect(&rect, crBkgnd);

	int nWidth = 0;
	rect.left += m_HeaderCtrl.GetSpacing();

	CImageList* pImageList = GetImageList(LVSIL_SMALL);
	if (pImageList)
	{
		SIZE sizeImage;
		sizeImage.cx = sizeImage.cy = 0;
		IMAGEINFO info;

		int nImage = -1;
		if (pXLCD)
			nImage = pXLCD[nSubItem].nImage;

		if (nImage == -1)
			return 0;

		if (pImageList->GetImageInfo(nImage, &info))
		{
			sizeImage.cx = info.rcImage.right - info.rcImage.left;
			sizeImage.cy = info.rcImage.bottom - info.rcImage.top;
		}

		if (nImage >= 0)
		{
			if (rect.Width() > 0)
			{
				POINT point;

				point.y = rect.CenterPoint().y - (sizeImage.cy >> 1);
				point.x = rect.left;

				SIZE size;
				size.cx = rect.Width() < sizeImage.cx ? rect.Width() : sizeImage.cx;
				size.cy = rect.Height() < sizeImage.cy ? rect.Height() : sizeImage.cy;

				// save image list background color
				COLORREF rgb = pImageList->GetBkColor();

				// set image list background color
				pImageList->SetBkColor(crBkgnd);
				pImageList->DrawIndirect(pDC, nImage, point, size, CPoint(0, 0));
				pImageList->SetBkColor(rgb);

				nWidth = sizeImage.cx + m_HeaderCtrl.GetSpacing();
			}
		}
	}

	return nWidth;
}

///////////////////////////////////////////////////////////////////////////////
// DrawText
void CXListCtrl::DrawText(int nItem,
						  int nSubItem,
						  CDC *pDC,
						  COLORREF crText,
						  COLORREF crBkgnd,
						  CRect& rect,
						  XLISTCTRLDATA *pXLCD)
{
	ASSERT(pDC);
	//ASSERT(pXLCD);

	GetDrawColors(nItem, nSubItem, crText, crBkgnd);

	pDC->FillSolidRect(&rect, crBkgnd);

	CString str;
	str = GetItemText(nItem, nSubItem);
	if (str.IsEmpty()) return;

	if(str[0]=='#')
	{
		// format : "#RRGGBB"
		char color[6+1];
		strncpy(color,(const char*)(str)+1,6);
		color[6]=0;
		int r,g,b;
		sscanf(color,"%2x",&r);
		sscanf(&color[2],"%2x",&g);
		sscanf(&color[4],"%2x",&b);
		crText=RGB(r,g,b);
		str = str.Mid(7);
	}

	// get text justification
	HDITEM hditem;
	hditem.mask = HDI_FORMAT;
	m_HeaderCtrl.GetItem(nSubItem, &hditem);
	int nFmt = hditem.fmt & HDF_JUSTIFYMASK;
	UINT nFormat = DT_VCENTER | DT_SINGLELINE;
	if (nFmt == HDF_CENTER)
		nFormat |= DT_CENTER;
	else if (nFmt == HDF_LEFT)
		nFormat |= DT_LEFT;
	else
		nFormat |= DT_RIGHT;

	CFont *pOldFont = NULL;
	CFont boldfont;

	// check if bold specified for subitem
	if (pXLCD && pXLCD[nSubItem].bBold)
	{
		CFont *font = pDC->GetCurrentFont();
		if (font)
		{
			LOGFONT lf;
			font->GetLogFont(&lf);
			lf.lfWeight = FW_BOLD;
			boldfont.CreateFontIndirect(&lf);
			pOldFont = pDC->SelectObject(&boldfont);
		}
	}

	rect.left+=4;
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(crText);
	pDC->SetBkColor(crBkgnd);
	pDC->DrawText(str, &rect, nFormat);
	if (pOldFont)
		pDC->SelectObject(pOldFont);
}

///////////////////////////////////////////////////////////////////////////////
// GetSubItemRect
BOOL CXListCtrl::GetSubItemRect(int nItem,
								int nSubItem,
								int nArea,
								CRect& rect)
{
	ASSERT(nItem >= 0);
	ASSERT(nItem < GetItemCount());
	if ((nItem < 0) || nItem >= GetItemCount())
		return FALSE;
	ASSERT(nSubItem >= 0);
	ASSERT(nSubItem < GetColumns());
	if ((nSubItem < 0) || nSubItem >= GetColumns())
		return FALSE;

	BOOL bRC = CListCtrl::GetSubItemRect(nItem, nSubItem, nArea, rect);

	// if nSubItem == 0, the rect returned by CListCtrl::GetSubItemRect
	// is the entire row, so use left edge of second subitem

	if (nSubItem == 0)
	{
		if (GetColumns() > 1)
		{
			CRect rect1;
			bRC = GetSubItemRect(nItem, 1, LVIR_BOUNDS, rect1);
			rect.right = rect1.left;
		}
	}

	return bRC;
}

///////////////////////////////////////////////////////////////////////////////
// OnPaint
void CXListCtrl::OnPaint()
{
    Default();
	if (GetItemCount() <= 0)
	{
		CDC* pDC = GetDC();
		int nSavedDC = pDC->SaveDC();

		CRect rc;
		GetWindowRect(&rc);
		ScreenToClient(&rc);
		CHeaderCtrl* pHC = GetHeaderCtrl();
		if (pHC != NULL)
		{
			CRect rcH;
			pHC->GetItemRect(0, &rcH);
			rc.top += rcH.bottom;
		}
		rc.top += 10;
		CString strText;
		strText = _T("There are no items to show in this view.");

		COLORREF crText = m_crWindowText;
		COLORREF crBkgnd = m_crWindow;

		CBrush brush(crBkgnd);
		pDC->FillRect(rc, &brush);

		/*
		pDC->SetTextColor(crText);
		pDC->SetBkColor(crBkgnd);
		pDC->SelectStockObject(ANSI_VAR_FONT);
		pDC->DrawText(strText, -1, rc, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP);
		*/
		pDC->RestoreDC(nSavedDC);
		ReleaseDC(pDC);
	}
}

///////////////////////////////////////////////////////////////////////////////
// GetEnabled
//
// Note that GetEnabled and SetEnabled only Get/Set the enabled flag from
// subitem 0, since this is a per-row flag.
//
BOOL CXListCtrl::GetEnabled(int nItem)
{
	ASSERT(nItem >= 0);
	ASSERT(nItem < GetItemCount());
	if ((nItem < 0) || nItem >= GetItemCount())
		return FALSE;

	XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
	if (!pXLCD)
	{
		return FALSE;
	}

	return pXLCD[0].bEnabled;
}

///////////////////////////////////////////////////////////////////////////////
// SetEnabled
BOOL CXListCtrl::SetEnabled(int nItem, BOOL bEnable)
{
	ASSERT(nItem >= 0);
	ASSERT(nItem < GetItemCount());
	if ((nItem < 0) || nItem >= GetItemCount())
		return FALSE;

	XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
	if (!pXLCD)
	{
		return FALSE;
	}

	pXLCD[0].bEnabled = bEnable;

	CRect rect;
	GetItemRect(nItem, &rect, LVIR_BOUNDS);
	InvalidateRect(&rect);
	UpdateWindow();

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// SetCurSel
BOOL CXListCtrl::SetCurSel(int nItem)
{
	return SetItemState(nItem, LVIS_FOCUSED | LVIS_SELECTED,
		LVIS_FOCUSED | LVIS_SELECTED);
}

///////////////////////////////////////////////////////////////////////////////
// GetCurSel - returns selected item number, or -1 if no item selected
//
// Note:  for single-selection lists only
//
int CXListCtrl::GetCurSel()
{
	POSITION pos = GetFirstSelectedItemPosition();
	int nSelectedItem = -1;
	if (pos != NULL)
		nSelectedItem = GetNextSelectedItem(pos);
	return nSelectedItem;
}

///////////////////////////////////////////////////////////////////////////////
// UpdateSubItem
void CXListCtrl::UpdateSubItem(int nItem, int nSubItem)
{
	ASSERT(nItem >= 0);
	ASSERT(nItem < GetItemCount());
	if ((nItem < 0) || nItem >= GetItemCount())
		return;
	ASSERT(nSubItem >= 0);
	ASSERT(nSubItem < GetColumns());
	if ((nSubItem < 0) || nSubItem >= GetColumns())
		return;

	CRect rect;
	if (nSubItem == -1)
	{
		GetItemRect(nItem, &rect, LVIR_BOUNDS);
	}
	else
	{
		GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rect);
	}

	InvalidateRect(&rect);
	UpdateWindow();
}

///////////////////////////////////////////////////////////////////////////////
// GetColumns
int CXListCtrl::GetColumns()
{
	return GetHeaderCtrl()->GetItemCount();
}

///////////////////////////////////////////////////////////////////////////////
// GetHeaderCheckedState
//
// The GetHeaderCheckedState and SetHeaderCheckedState may be used to toggle
// the checkbox in a column header.
//     0 = no checkbox
//     1 = unchecked
//     2 = checked
//
int CXListCtrl::GetHeaderCheckedState(int nSubItem)
{
	ASSERT(nSubItem >= 0);
	ASSERT(nSubItem < GetColumns());
	if ((nSubItem < 0) || nSubItem >= GetColumns())
		return -1;

	HDITEM hditem;

	// use the image index (0 or 1) to indicate the checked status
	hditem.mask = HDI_IMAGE;
	m_HeaderCtrl.GetItem(nSubItem, &hditem);
	return hditem.iImage;
}

///////////////////////////////////////////////////////////////////////////////
// SetHeaderCheckedState
BOOL CXListCtrl::SetHeaderCheckedState(int nSubItem, int nCheckedState)
{
	ASSERT(nSubItem >= 0);
	ASSERT(nSubItem < GetColumns());
	if ((nSubItem < 0) || nSubItem >= GetColumns())
		return FALSE;
	ASSERT(nCheckedState == 0 || nCheckedState == 1 || nCheckedState == 2);

	HDITEM hditem;

	hditem.mask = HDI_IMAGE;
	hditem.iImage = nCheckedState;
	m_HeaderCtrl.SetItem(nSubItem, &hditem);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// OnColumnClick
BOOL CXListCtrl::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pnmlv = (NMLISTVIEW*)pNMHDR;

	int nSubItem = pnmlv->iSubItem;

	int nCheckedState = GetHeaderCheckedState(nSubItem);

	// 0 = no checkbox
	if (nCheckedState != XHEADERCTRL_NO_IMAGE)
	{
		nCheckedState = (nCheckedState == 1) ? 2 : 1;
		SetHeaderCheckedState(nSubItem, nCheckedState);

		m_HeaderCtrl.UpdateWindow();

		for (int nItem = 0; nItem < GetItemCount(); nItem++)
		{
			XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
			if (!pXLCD)
			{
				continue;
			}

			if (pXLCD[nSubItem].nCheckedState != -1)
			{
				pXLCD[nSubItem].nCheckedState = nCheckedState - 1;
				UpdateSubItem(nItem, nSubItem);
			}
		}
	}

	*pResult = 0;
	return FALSE;		// return FALSE to send message to parent also -
						// NOTE:  MSDN documentation is incorrect
}

///////////////////////////////////////////////////////////////////////////////
// OnSysColorChange
void CXListCtrl::OnSysColorChange()
{
	TRACE(_T("in CXListCtrl::OnSysColorChange\n"));

	CListCtrl::OnSysColorChange();

	m_cr3DFace        = ::GetSysColor(COLOR_3DFACE);
	m_cr3DHighLight   = ::GetSysColor(COLOR_3DHIGHLIGHT);
	m_cr3DShadow      = ::GetSysColor(COLOR_3DSHADOW);
	m_crBtnFace       = ::GetSysColor(COLOR_BTNFACE);
	m_crBtnShadow     = ::GetSysColor(COLOR_BTNSHADOW);
	m_crBtnText       = ::GetSysColor(COLOR_BTNTEXT);
	m_crGrayText      = ::GetSysColor(COLOR_GRAYTEXT);
	m_crHighLight     = ::GetSysColor(COLOR_HIGHLIGHT);
	m_crHighLightText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_crWindow        = ::GetSysColor(COLOR_WINDOW);
	m_crWindowText    = ::GetSysColor(COLOR_WINDOWTEXT);
}

#ifndef NO_XLISTCTRL_TOOL_TIPS

///////////////////////////////////////////////////////////////////////////////
// OnToolHitTest
int CXListCtrl::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
{
	LVHITTESTINFO lvhitTestInfo;
	
	lvhitTestInfo.pt = point;
	
	int nItem = ListView_SubItemHitTest(this->m_hWnd, &lvhitTestInfo);
	int nSubItem = lvhitTestInfo.iSubItem;
	TRACE(_T("in CToolTipListCtrl::OnToolHitTest: %d,%d\n"), nItem, nSubItem);

	UINT nFlags = lvhitTestInfo.flags;

	// nFlags is 0 if the SubItemHitTest fails
	// Therefore, 0 & <anything> will equal false
	if (nFlags & LVHT_ONITEMLABEL)
	{
		// If it did fall on a list item,
		// and it was also hit one of the
		// item specific subitems we wish to show tool tips for
		
		// get the client (area occupied by this control
		RECT rcClient;
		GetClientRect(&rcClient);
		
		// fill in the TOOLINFO structure
		pTI->hwnd = m_hWnd;
		pTI->uId = (UINT) (nItem * 1000 + nSubItem + 1);
		pTI->lpszText = LPSTR_TEXTCALLBACK;
		pTI->rect = rcClient;
		
		return pTI->uId;	// By returning a unique value per listItem,
							// we ensure that when the mouse moves over another
							// list item, the tooltip will change
	}
	else
	{
		//Otherwise, we aren't interested, so let the message propagate
		return -1;
	}
}

///////////////////////////////////////////////////////////////////////////////
// OnToolTipText
BOOL CXListCtrl::OnToolTipText(UINT /*id*/, NMHDR * pNMHDR, LRESULT * pResult)
{
	UINT nID = pNMHDR->idFrom;
	TRACE(_T("in CXListCtrl::OnToolTipText: id=%d\n"), nID);
	
	// check if this is the automatic tooltip of the control
	if (nID == 0) 
		return TRUE;	// do not allow display of automatic tooltip,
						// or our tooltip will disappear
	
	// handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	
	*pResult = 0;
	
	// get the mouse position
	const MSG* pMessage;
	pMessage = GetCurrentMessage();
	ASSERT(pMessage);
	CPoint pt;
	pt = pMessage->pt;		// get the point from the message
	ScreenToClient(&pt);	// convert the point's coords to be relative to this control
	
	// see if the point falls onto a list item
	
	LVHITTESTINFO lvhitTestInfo;
	
	lvhitTestInfo.pt = pt;
	
	int nItem = SubItemHitTest(&lvhitTestInfo);
	int nSubItem = lvhitTestInfo.iSubItem;
	
	UINT nFlags = lvhitTestInfo.flags;
	
	// nFlags is 0 if the SubItemHitTest fails
	// Therefore, 0 & <anything> will equal false
	if (nFlags & LVHT_ONITEMLABEL)
	{
		// If it did fall on a list item,
		// and it was also hit one of the
		// item specific subitems we wish to show tooltips for
		
		CString strToolTip;
		strToolTip = _T("");

		XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
		if (pXLCD)
		{
			strToolTip = pXLCD[nSubItem].strToolTip;
		}

		if (!strToolTip.IsEmpty())
		{
			// If there was a CString associated with the list item,
			// copy it's text (up to 80 characters worth, limitation 
			// of the TOOLTIPTEXT structure) into the TOOLTIPTEXT 
			// structure's szText member
			
#ifndef _UNICODE
			if (pNMHDR->code == TTN_NEEDTEXTA)
				lstrcpyn(pTTTA->szText, strToolTip, 80);
			else
				_mbstowcsz(pTTTW->szText, strToolTip, 80);
#else
			if (pNMHDR->code == TTN_NEEDTEXTA)
				_wcstombsz(pTTTA->szText, strToolTip, 80);
			else
				lstrcpyn(pTTTW->szText, strToolTip, 80);
#endif
			return FALSE;	 // we found a tool tip,
		}
	}
	
	return FALSE;	// we didn't handle the message, let the 
					// framework continue propagating the message
}

///////////////////////////////////////////////////////////////////////////////
// SetItemToolTipText
BOOL CXListCtrl::SetItemToolTipText(int nItem, int nSubItem, LPCTSTR lpszToolTipText)
{
	ASSERT(nItem >= 0);
	ASSERT(nItem < GetItemCount());
	if ((nItem < 0) || nItem >= GetItemCount())
		return FALSE;
	ASSERT(nSubItem >= 0);
	ASSERT(nSubItem < GetColumns());
	if ((nSubItem < 0) || nSubItem >= GetColumns())
		return FALSE;

	XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
	if (!pXLCD)
	{
		return FALSE;
	}

	pXLCD[nSubItem].strToolTip = lpszToolTipText;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// GetItemToolTipText
CString CXListCtrl::GetItemToolTipText(int nItem, int nSubItem)
{
	CString strToolTip;
	strToolTip = _T("");

	ASSERT(nItem >= 0);
	ASSERT(nItem < GetItemCount());
	if ((nItem < 0) || nItem >= GetItemCount())
		return strToolTip;
	ASSERT(nSubItem >= 0);
	ASSERT(nSubItem < GetColumns());
	if ((nSubItem < 0) || nSubItem >= GetColumns())
		return strToolTip;

	XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
	if (pXLCD)
	{
		strToolTip = pXLCD[nSubItem].strToolTip;
	}

	return strToolTip;
}

///////////////////////////////////////////////////////////////////////////////
// DeleteAllToolTips
void CXListCtrl::DeleteAllToolTips()
{
	int nRow = GetItemCount();
	int nCol = GetColumns();

	for (int nItem = 0; nItem < nRow; nItem++)
	{
		XLISTCTRLDATA *pXLCD = (XLISTCTRLDATA *) CListCtrl::GetItemData(nItem);
		if (pXLCD)
			for (int nSubItem = 0; nSubItem < nCol; nSubItem++)
				pXLCD[nSubItem].strToolTip = _T("");
	}
}

#endif

