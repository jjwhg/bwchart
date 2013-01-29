// DlgMainOptions.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgMainOptions.h"
#include "DlgOptions.h"
#include "DlgOptWatch.h"
#include "DlgOptionsChart.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgMainOptions dialog


DlgMainOptions::DlgMainOptions(CWnd* pParent /*=NULL*/)
	: CDialog(DlgMainOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgMainOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_dlgOpt = new DlgOptions(this);
	m_dlgOptWatch = new DlgOptWatch(this);
	m_dlgOptChart = new DlgOptionsChart(this);
}

DlgMainOptions::~DlgMainOptions()
{
	delete m_dlgOptChart;
	delete m_dlgOpt;
	delete m_dlgOptWatch;
}


void DlgMainOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgMainOptions)
	DDX_Control(pDX, IDC_TAB1, m_tab);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgMainOptions, CDialog)
	//{{AFX_MSG_MAP(DlgMainOptions)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgMainOptions message handlers

void DlgMainOptions::_UpdateScreen(int tab) 
{
	switch(tab)
	{
	case 0:
		//watch replay
		m_dlgOptWatch->ShowWindow(SW_SHOW);
		m_dlgOpt->ShowWindow(SW_HIDE);
		m_dlgOptChart->ShowWindow(SW_HIDE);
		break;
	case 1:
		// other options
		m_dlgOptChart->ShowWindow(SW_SHOW);
		m_dlgOptWatch->ShowWindow(SW_HIDE);
		m_dlgOpt->ShowWindow(SW_HIDE);
		break;
	case 2:
		// charts
		m_dlgOpt->ShowWindow(SW_SHOW);
		m_dlgOptWatch->ShowWindow(SW_HIDE);
		m_dlgOptChart->ShowWindow(SW_HIDE);
		break;
	}

	AfxGetApp()->WriteProfileInt("MOPTIONS","TAB",tab);
}


BOOL DlgMainOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// create dialogs
	m_dlgOptChart->Create(DlgOptionsChart::IDD,this);
	m_dlgOpt->Create(DlgOptions::IDD,this);
	m_dlgOptWatch->Create(DlgOptWatch::IDD,this);

	CString title;
	title.LoadString(IDS_WATCHREPLAY);
	m_tab.InsertItem(0,title);
	title.LoadString(IDS_CHARTS);
	m_tab.InsertItem(1,title);
	title.LoadString(IDS_OTHEROPTIONS);
	m_tab.InsertItem(2,title);

	m_tab.SetCurSel(AfxGetApp()->GetProfileInt("MOPTIONS","TAB",0));
	_UpdateScreen(m_tab.GetCurSel());
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgMainOptions::_Resize()
{
	if(!::IsWindow(m_dlgOpt->GetSafeHwnd())) return;

	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(8,4);
	m_tab.MoveWindow(&rect,TRUE);
	rect.top +=21;
	rect.DeflateRect(4,4);
	m_dlgOpt->MoveWindow(&rect,TRUE);
	m_dlgOptWatch->MoveWindow(&rect,TRUE);
	m_dlgOptChart->MoveWindow(&rect,TRUE);
}

void DlgMainOptions::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	if(cx!=0 && cy!=0) _Resize();
}

void DlgMainOptions::SaveValues()
{
	m_dlgOpt->InnerSaveValues();
	m_dlgOptWatch->InnerSaveValues();
	m_dlgOptChart->InnerSaveValues();
}

void DlgMainOptions::OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_UpdateScreen(m_tab.GetCurSel());
	*pResult = 0;
}
