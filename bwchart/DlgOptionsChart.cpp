// DlgOptionsChart.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgOptionsChart.h"
#include "Dlgbwchart.h"
#include "dirutil.h"
#include "bwdb.h"
#include "chkupdate.h"
#include "regparam.h"
#include "gradient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEF_CLR_MAP RGB(155,20,10)
#define DEF_CLR_PLAYERS RGB(225,225,225)
#define DEF_CLR_OTHER RGB(128,128,128)

extern unsigned long gSuspectLimit;

//----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DlgOptionsChart, CDialog)
	//{{AFX_MSG_MAP(DlgOptionsChart)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BROWSEBMP, OnBrowsebmp)
	ON_BN_CLICKED(IDC_USEBMP, OnUsebmp)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BGGRADIENT, OnBggradient)
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_DEFAULTS, OnDefaults)
	ON_CBN_SELCHANGE(IDC_COMBOTYPE, OnSelchangeCombotype)
	ON_BN_CLICKED(IDC_DEFAULTS2, OnDefaults2)
	ON_CBN_SELCHANGE(IDC_COMBOLSIZE, OnSelchangeCombolsize)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BGGRADIENT, OnUpdateControl)
	ON_BN_CLICKED(IDC_BGGRID, OnUpdateControl)
	ON_BN_CLICKED(IDC_COLOREDEVENTLIST, OnUpdateControl)
	ON_BN_CLICKED(IDC_RADIO1, OnUpdatePosition)
	ON_BN_CLICKED(IDC_RADIO2, OnUpdatePosition)
	ON_BN_CLICKED(IDC_RADIO3, OnUpdatePosition)
END_MESSAGE_MAP()

//----------------------------------------------------------------------------------------------

void DlgOptionsChart::_DefaultPath(CString& path, const char *defexe)
{
	if(path.IsEmpty()) 
	{
		_GetStarcraftPath(path);
		if(path.Right(1)!="\\") path += "\\";
		path += defexe;
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::_Parameters(bool bLoad)
{
	PINT("BWCHART_OPTIONS",maxapm,1);
	PINT("BWCHART_OPTIONS",bggrid,1);
	PINT("BWCHART_OPTIONS",bggradient, 1);
	PINT("BWCHART_OPTIONS",coloredevents, 1);
	PINT("BWCHART_OPTIONS",usebmp,0);
	PINT("BWCHART_OPTIONS",imgposition,2);
	PINT("BWCHART_OPTIONS",colors[0],DEF_CLR_MAP);
	PINT("BWCHART_OPTIONS",colors[1],DEF_CLR_PLAYERS);
	PINT("BWCHART_OPTIONS",colors[2],DEF_CLR_OTHER);
	PINT("BWCHART_OPTIONS",colors[3],DEF_CLR_MINERAL);
	PINT("BWCHART_OPTIONS",colors[4],DEF_CLR_GAS);
	PINT("BWCHART_OPTIONS",colors[5],DEF_CLR_SUPPLY);
	PINT("BWCHART_OPTIONS",colors[6],DEF_CLR_UNITS);
	PINT("BWCHART_OPTIONS",colors[7],DEF_CLR_APM);
	PINT("BWCHART_OPTIONS",colors[8],DEF_CLR_BPM);
	PINT("BWCHART_OPTIONS",colors[9],DEF_CLR_UPM);
	PINT("BWCHART_OPTIONS",colors[10],DEF_CLR_MAPCOVERAGE);
	PINT("BWCHART_OPTIONS",linesize[3],DEF_LSIZE_MINERAL);
	PINT("BWCHART_OPTIONS",linesize[4],DEF_LSIZE_GAS);
	PINT("BWCHART_OPTIONS",linesize[5],DEF_LSIZE_SUPPLY);
	PINT("BWCHART_OPTIONS",linesize[6],DEF_LSIZE_UNITS);
	PINT("BWCHART_OPTIONS",linesize[7],DEF_LSIZE_APM);
	PINT("BWCHART_OPTIONS",linesize[8],DEF_LSIZE_BPM);
	PINT("BWCHART_OPTIONS",linesize[9],DEF_LSIZE_UPM);
	PINT("BWCHART_OPTIONS",linesize[10],DEF_LSIZE_MAPCOVERAGE);
	PINT("BWCHART_OPTIONS",comboType,0);
	PSTRING("BWCHART_OPTIONS",bmppath,"");
}

//----------------------------------------------------------------------------------------------

DlgOptionsChart::DlgOptionsChart(CWnd* pParent /*=NULL*/)
	: CDialog(DlgOptionsChart::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgOptionsChart)
	m_bggrid = FALSE;
	m_maxapm = FALSE;
	m_bggradient = FALSE;
	m_coloredevents = FALSE;
	m_bmppath = _T("");
	m_usebmp = FALSE;
	m_imgposition = -1;
	m_comboType = -1;
	//}}AFX_DATA_INIT

	_Parameters(true);

}


void DlgOptionsChart::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgOptionsChart)
	DDX_Control(pDX, IDC_COMBOLSIZE, m_comboLSize);
	DDX_Check(pDX, IDC_BGGRID, m_bggrid);
	DDX_Check(pDX, IDC_MAXAPMMARKER, m_maxapm);
	DDX_Check(pDX, IDC_BGGRADIENT, m_bggradient);
	DDX_Check(pDX, IDC_COLOREDEVENTLIST, m_coloredevents);
	DDX_Text(pDX, IDC_BMPPATH, m_bmppath);
	DDX_Check(pDX, IDC_USEBMP, m_usebmp);
	DDX_Radio(pDX, IDC_RADIO1, m_imgposition);
	DDX_CBIndex(pDX, IDC_COMBOTYPE, m_comboType);
	//}}AFX_DATA_MAP
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::_GetStarcraftPath(CString& path) 
{
	// open key for control panel
	HKEY hKey;
	if ( ERROR_SUCCESS != RegCreateKey(HKEY_LOCAL_MACHINE, "Software\\Blizzard Entertainment\\Starcraft",&hKey) ) 
		return;

	// get program location
	char buffer[255];
	unsigned long size = sizeof(buffer);
	unsigned long type = REG_SZ;
	RegQueryValueEx(hKey,"Program", 0, &type, (unsigned char*)buffer, &size);
	char *p=strrchr(buffer,'\\'); if(p!=0) p[1]=0;
	path=buffer;

	// close key
	RegCloseKey(hKey);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::InnerSaveValues()
{
	UpdateData(TRUE);
	_Parameters(false);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::OnDestroy() 
{
	InnerSaveValues();
	CDialog::OnDestroy();
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::OnUpdateControl() 
{
	UpdateData(TRUE);
}

//--------------------------------------------------------------------------------------------------------------

#define SAVECOLORS(n,m)\
		ReplayResource::m_gColors[ReplayResource::n]=m_colors[m];\
		ReplayResource::m_gLineSize[ReplayResource::n]=m_linesize[m]

void DlgOptionsChart::_UpdateBitmap()
{
	// repaint this window
	Invalidate();

	// update replay colors	and line size
	SAVECOLORS(CLR_MINERAL,CLR_CHART_MIN);
	SAVECOLORS(CLR_GAS,CLR_CHART_GAS);
	SAVECOLORS(CLR_SUPPLY,CLR_CHART_SUPPLY);
	SAVECOLORS(CLR_UNITS,CLR_CHART_UNITS);
	SAVECOLORS(CLR_APM,CLR_CHART_APM);
	SAVECOLORS(CLR_BPM,CLR_CHART_BPM);
	SAVECOLORS(CLR_UPM,CLR_CHART_UPM);
	SAVECOLORS(CLR_MAPCOVERAGE,CLR_MAPCOVERAGE);

	// repaint stats
	MAINWND->pGetStats()->UpdateBkgBitmap();
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::OnBrowsebmp() 
{
	static char BASED_CODE szFilter[] = "Bitmap (*.bmp)|*.bmp|All Files (*.*)|*.*||";

 	CFileDialog dlg(TRUE,"bmp","",0,szFilter,this);
	if(!m_bmppath.IsEmpty()) dlg.m_ofn.lpstrInitialDir = m_bmppath;
	if(dlg.DoModal()==IDOK)
	{
		m_bmppath = dlg.GetPathName();
		m_usebmp=1;
		UpdateData(FALSE);
		if(m_usebmp) m_bkgbmp.LoadFromFile(m_bmppath);
		_UpdateBitmap();
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::OnUsebmp() 
{
	UpdateData(TRUE);
	if(m_usebmp) m_bkgbmp.LoadFromFile(m_bmppath);
	_UpdateBitmap();
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::OnUpdatePosition() 
{
	UpdateData(TRUE);
	_UpdateBitmap();
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::_PaintColor(CDC *pDC, UINT ctrlid, int idx) 
{
	// fill rect with color
	CRect rect;
	GetDlgItem(ctrlid)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	pDC->FillSolidRect(&rect,m_colors[idx]);

	// draw border
	pDC->Draw3dRect(&rect,0,0);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::_PaintText(CDC *pDC, CRect& rect, int idx) 
{
	char *str[]={"Map","Player name","Others","Mineral","Gas","Supply","Units","APM","BPM","UPM","Map Coverage"};
	for(int i=0;i<3;i++)
	{
		CRect rtxt(rect);
		rtxt.OffsetRect(8+150*i,8+idx*18);
		COLORREF oldClr=pDC->SetTextColor(m_colors[idx]);
		int mode = pDC->SetBkMode(TRANSPARENT);
		pDC->DrawText(str[idx],-1,&rtxt,DT_SINGLELINE|DT_LEFT);
		pDC->SetBkMode(mode);
		pDC->SetTextColor(oldClr);
	}
}

//--------------------------------------------------------------------------------------------------------------

bool DlgOptionsChart::_IsInColorRect(const POINT& pt, UINT ctrlid, CRect& rectFrame) 
{
	// fill rect with color
	GetDlgItem(ctrlid)->GetWindowRect(&rectFrame);
	ScreenToClient(&rectFrame);
	return rectFrame.PtInRect(pt) ? true : false;
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// get image rect
	CRect rect;
	GetDlgItem(IDC_PREVIEW)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.DeflateRect(16,4);
	rect.top+=12;

	// fill chart background
	PaintBackground(&dc,rect);

	// fill colors
	_PaintColor(&dc,IDC_COLOR_MAPNAME,0);
	_PaintColor(&dc,IDC_COLOR_PLAYERNAME,1);
	_PaintColor(&dc,IDC_COLOR_OTHER,2);
	_PaintColor(&dc,IDC_COLOR_CHART,CLR_CHART_MIN+m_comboType);

	// print test text
	for(int i=0;i<_CLRMAX; i++) 
		_PaintText(&dc,rect,i);
}

//--------------------------------------------------------------------------------------------------------------

BOOL DlgOptionsChart::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// load bitmap
	if(m_usebmp) m_bkgbmp.LoadFromFile(m_bmppath);

	// init replay colors
	SAVECOLORS(CLR_MINERAL,CLR_CHART_MIN);
	SAVECOLORS(CLR_GAS,CLR_CHART_GAS);
	SAVECOLORS(CLR_SUPPLY,CLR_CHART_SUPPLY);
	SAVECOLORS(CLR_UNITS,CLR_CHART_UNITS);
	SAVECOLORS(CLR_APM,CLR_CHART_APM);
	SAVECOLORS(CLR_BPM,CLR_CHART_BPM);
	SAVECOLORS(CLR_UPM,CLR_CHART_UPM);
	SAVECOLORS(CLR_MAPCOVERAGE,CLR_MAPCOVERAGE);

	// init line size combo
	m_comboLSize.SetCurSel(m_linesize[m_comboType+CLR_CHART_MIN]-1);
	  	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::PaintBackground(CDC *pDC, const CRect& rect)
{
	// fill chart background
	if(m_usebmp && m_bkgbmp.HasBitmap())
	{
		// paint background bitmap
		if(m_imgposition==0)
			m_bkgbmp.PaintCenter(pDC,rect);
		else if(m_imgposition==1)
			m_bkgbmp.Tile(pDC,rect);
		else
			m_bkgbmp.Paint(pDC,rect,true);
	}
	else if(m_bggradient)
	{
		// fill bkg with gradient
		Gradient::Fill(pDC,rect,RGB(0,0,30),RGB(0,0,0),GRADIENT_FILL_RECT_V);
	}
	else
	{
		// paint solid bkg
		pDC->FillSolidRect(rect,RGB(0,0,0));
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::OnBggradient() 
{
	UpdateData(TRUE);
	if(!m_usebmp) _UpdateBitmap();
}

//--------------------------------------------------------------------------------------------------------------

bool DlgOptionsChart::_SelectColor(int idx, const CRect& /*rect*/)
{
	CColorDialog dlg(m_colors[idx]);
	dlg.m_cc.Flags |= CC_FULLOPEN;
	if(dlg.DoModal()==IDOK)
	{
		m_colors[idx] = dlg.GetColor();
		_UpdateBitmap();
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rect;
	if(_IsInColorRect(point,IDC_COLOR_MAPNAME,rect))
		_SelectColor(CLR_MAP,rect);
	else if(_IsInColorRect(point,IDC_COLOR_PLAYERNAME,rect))
		_SelectColor(CLR_PLAYERS,rect);
	else if(_IsInColorRect(point,IDC_COLOR_OTHER,rect))
		_SelectColor(CLR_OTHER,rect);
	else if(_IsInColorRect(point,IDC_COLOR_CHART,rect))
		_SelectColor(CLR_CHART_MIN+m_comboType,rect);
	
	CDialog::OnLButtonDown(nFlags, point);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::OnDefaults() 
{
	m_colors[0]=DEF_CLR_MAP;
	m_colors[1]=DEF_CLR_PLAYERS;
	m_colors[2]=DEF_CLR_OTHER;

	_UpdateBitmap();
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::OnDefaults2() 
{
	m_colors[CLR_CHART_MIN]=DEF_CLR_MINERAL;
	m_colors[CLR_CHART_GAS]=DEF_CLR_GAS;
	m_colors[CLR_CHART_SUPPLY]=DEF_CLR_SUPPLY;
	m_colors[CLR_CHART_UNITS]=DEF_CLR_UNITS;
	m_colors[CLR_CHART_APM]=DEF_CLR_APM;
	m_colors[CLR_CHART_BPM]=DEF_CLR_BPM;
	m_colors[CLR_CHART_UPM]=DEF_CLR_UPM;
	m_colors[CLR_MAPCOVERAGE]=DEF_CLR_MAPCOVERAGE;
	_UpdateBitmap();
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::OnSelchangeCombotype() 
{
	UpdateData(TRUE);

	// update rect with color
	CRect rectFrame;
	GetDlgItem(IDC_COLOR_CHART)->GetWindowRect(&rectFrame);
	ScreenToClient(&rectFrame);
	InvalidateRect(rectFrame);

	// update line size
	m_comboLSize.SetCurSel(ReplayResource::m_gLineSize[m_comboType]-1);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptionsChart::OnSelchangeCombolsize() 
{
	m_linesize[m_comboType+CLR_CHART_MIN] = ReplayResource::m_gLineSize[m_comboType] = m_comboLSize.GetCurSel()+1;
	_UpdateBitmap();
}

//--------------------------------------------------------------------------------------------------------------
