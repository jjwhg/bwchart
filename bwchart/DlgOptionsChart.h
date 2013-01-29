#ifndef __DLGOPTIONSCHART_H
#define __DLGOPTIONSCHART_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgOptionsChart.h : header file
//

#include "mybitmap.h"

#define OPTIONSCHART ((DlgBWChart*)m_pParentWnd)->pGetOptionsChart()

/////////////////////////////////////////////////////////////////////////////
// DlgOptionsChart dialog

class DlgOptionsChart : public CDialog
{
// Construction
public:
	DlgOptionsChart(CWnd* pParent = NULL);   // standard constructor

	// save parameters
	void InnerSaveValues();

	// paint background
	void PaintBackground(CDC *pDC, const CRect& rect);

	// get color
	enum {CLR_MAP, CLR_PLAYERS, CLR_OTHER, 
		CLR_CHART_MIN, CLR_CHART_GAS, CLR_CHART_SUPPLY, CLR_CHART_UNITS, CLR_CHART_APM, 
		CLR_CHART_BPM, CLR_CHART_UPM, CLR_MAPCOVERAGE, _CLRMAX};
	COLORREF GetColor(int idx) {return m_colors[idx];}

	static void _GetStarcraftPath(CString& path);
	static void CheckForUpdate(bool automatic);

// Dialog Data
	//{{AFX_DATA(DlgOptionsChart)
	enum { IDD = IDD_DLGOPTIONS_CHART };
	CComboBox	m_comboLSize;
	BOOL	m_bggrid;
	BOOL	m_maxapm;
	BOOL	m_bggradient;
	BOOL	m_coloredevents;
	CString	m_bmppath;
	BOOL	m_usebmp;
	int		m_imgposition;
	int		m_comboType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgOptionsChart)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnCancel() {}
	virtual void OnOK() {}
	//}}AFX_VIRTUAL


// Implementation
protected:
	MyBitmap m_bkgbmp;
	COLORREF m_colors[_CLRMAX];
	int m_linesize[_CLRMAX];

	void _DefaultPath(CString& path, const char *defexe);
	void _Parameters(bool bLoad);
	void _UpdateBitmap();
	void _PaintColor(CDC *pDC, UINT ctrlid, int idx);
	bool _SelectColor(int idx, const CRect& rect);
	bool _IsInColorRect(const POINT& pt, UINT ctrlid, CRect& rectFrame);
	void _PaintText(CDC *pDC, CRect& rect, int idx);

	// Generated message map functions
	//{{AFX_MSG(DlgOptionsChart)
	afx_msg void OnDestroy();
	afx_msg void OnBrowsebmp();
	afx_msg void OnUsebmp();
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnBggradient();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDefaults();
	afx_msg void OnSelchangeCombotype();
	afx_msg void OnDefaults2();
	afx_msg void OnSelchangeCombolsize();
	//}}AFX_MSG
	afx_msg void OnUpdateControl();
	afx_msg void OnUpdatePosition();
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif 
