// DlgBWChart.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgBWChart.h"
#include "DlgStats.h"
#include "DlgAbout.h"
#include "DlgBrowser.h"
#include "DlgAkas.h"
#include "DlgMainOptions.h"
#include "bwdb.h"
#include "hsvrgb.h"
#include "gradient.h"
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(DlgBWChart, CDialog)
	//{{AFX_MSG_MAP(DlgBWChart)
	ON_WM_SIZE()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_MENU_AKAS, OnMenuAkas)
	ON_BN_CLICKED(IDC_MENU_BROWSER, OnMenuBrowser)
	ON_BN_CLICKED(IDC_MENU_CHARTS, OnMenuCharts)
	ON_BN_CLICKED(IDC_MENU_FAVORITES, OnMenuFavorites)
	ON_BN_CLICKED(IDC_MENU_OPTIONS, OnMenuOptions)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_LOADREPLAY,OnRemoteLoadReplay)
	ON_MESSAGE(WM_ADDFAVORITE,OnRemoteAddFavorite)
	ON_MESSAGE(WM_CHECKFORUPDATE,OnCheckForUpdate)
END_MESSAGE_MAP()

static bool initdone=false;

//--------------------------------------------------------------------------------------------------------------

DlgBWChart::DlgBWChart(const char *cmdline, CWnd* pParent /*=NULL*/)
	: CDialog(DlgBWChart::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgBWChart)
	//}}AFX_DATA_INIT
	m_cmdline = cmdline;
	m_dlgOptions = new DlgMainOptions(this); // build options first
	m_dlgStats = new DlgStats(this);
	m_dlgBrowser = new DlgBrowser(this);
	m_dlgFavs = new DlgFavorites(this);
	m_dlgAkas = new DlgAkas(this);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_timer=0;

	_CreateFonts();

	// associates the .rep extension with us
	UpdateFileAssociation();

	// init replay database
	m_boFileExists = BWChartDB::InitInstance(pGetOptions()->m_savein==0 ? true : false);

	// init akas
	m_dlgAkas->InitInstance();
}

//--------------------------------------------------------------------------------------------------------------

DlgBWChart::~DlgBWChart()
{
	BWChartDB::ExitInstance();

	_DestroyFonts();

	delete m_dlgOptions;
	delete m_dlgAkas;
	delete m_dlgFavs;
	delete m_dlgBrowser;
	delete m_dlgStats;
}
  
//------------------------------------------------------------------------------------------------------------

void DlgBWChart::_CreateFonts()
{
	// retrieve a standard VARIABLE FONT from system
	HFONT hFont = (HFONT)GetStockObject( ANSI_VAR_FONT);
	CFont *pRefFont = CFont::FromHandle(hFont);
	LOGFONT LFont;

	// get font description
	memset(&LFont,0,sizeof(LOGFONT));
	pRefFont->GetLogFont(&LFont);

	// create our layer Font
	pRefFont->GetLogFont(&LFont);
	LFont.lfHeight = 14;
	LFont.lfWidth = 0;
	LFont.lfQuality|=ANTIALIASED_QUALITY;
	strcpy(LFont.lfFaceName,"Arial");
	m_pLayerFont = new CFont();
	m_pLayerFont->CreateFontIndirect(&LFont);
}

//------------------------------------------------------------------------------------------------------------

void DlgBWChart::_DestroyFonts()
{
	delete m_pLayerFont;
}
//--------------------------------------------------------------------------------------------------------------

void DlgBWChart::UpdateFileAssociation()
{
	// associates the .rep extension with us
	if(pGetOptions()->m_fileasso)
	{
		char exename[255];
		GetModuleFileName(0,exename,sizeof(exename));
		_RegisterFileType(exename, ".rep", "BWChart", "Starcraft Replay File");
	}
	else
	{
		_UnregisterFileType(".rep");
	}
}

//------------------------------------------------------------------------------------

// unassociates the .rep extension with us
bool DlgBWChart::_UnregisterFileType(const char *ext)
{
	// associates the .nvb extension with a type
	if ( ERROR_SUCCESS != ::RegDeleteKey(HKEY_CLASSES_ROOT, (LPCTSTR)ext))
		return false;
	return true;
}

//------------------------------------------------------------------------------------

// associates the .rep extension with us
bool DlgBWChart::_RegisterFileType(const char *exename, const char *ext, const char *typeName, const char *typeDesc)
{
	// create key for extension
	HKEY hKey;
	if ( ERROR_SUCCESS != ::RegCreateKey(HKEY_CLASSES_ROOT, ext,&hKey) ) 
		return false;

	// associates the .rep extension with a type
	if ( ERROR_SUCCESS != ::RegSetValueEx(hKey, 0, 0, REG_SZ, (LPBYTE)typeName, 1+lstrlen(typeName)))
		return false;

	// close key
	RegCloseKey(hKey);

	// create key for type
	if ( ERROR_SUCCESS != RegCreateKey(HKEY_CLASSES_ROOT, typeName,&hKey) ) 
		return false;

	// associates the type with us
	if ( ERROR_SUCCESS != ::RegSetValueEx(hKey, 0, 0, REG_SZ, (LPBYTE)typeDesc, 1+lstrlen(typeDesc)))
		return false;

	// make it a short file name
	char tmpexename[255];
	strcpy(tmpexename,exename);
	char szShort[255];
	if(GetShortPathName(tmpexename, szShort,sizeof(szShort))!=0)
		strcpy(tmpexename,szShort);

	// create key for default icon
	HKEY hKeyIcon;
	if ( ERROR_SUCCESS != RegCreateKey(hKey, "DefaultIcon",&hKeyIcon) ) 
		return false;

	// set value
	char iconname[255];
	sprintf(iconname,"%s,0",tmpexename);
	::RegSetValueEx(hKeyIcon, 0, 0, REG_SZ, (LPBYTE)iconname, 1+lstrlen(iconname));

	// close key
	RegCloseKey(hKeyIcon);

	// create excutable name to open the file
	strcpy(tmpexename,exename);
	if(GetShortPathName(tmpexename, szShort,sizeof(szShort))!=0)
		strcpy(tmpexename,szShort);

	// create key for executable name
	HKEY hKeyExe;
	if ( ERROR_SUCCESS != RegCreateKey(hKey, "shell\\open",&hKeyExe) ) 
		return false;

	// set command label
	const char *label="Open with &BWChart";
	::RegSetValueEx(hKeyExe, 0, 0, REG_SZ, (LPBYTE)label, 1+lstrlen(label));

	// close key
	RegCloseKey(hKeyExe);

	// create key for executable name
	if ( ERROR_SUCCESS != RegCreateKey(hKey, "shell\\open\\command",&hKeyExe) ) 
		return false;

	// set the name of the executable
	strcat(tmpexename," \"%1\"");
	::RegSetValueEx(hKeyExe, 0, 0, REG_SZ, (LPBYTE)tmpexename, 1+lstrlen(tmpexename));

	// close key
	RegCloseKey(hKeyExe);

	// close key
	RegCloseKey(hKey);

	return true;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBWChart::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgBWChart)
	//}}AFX_DATA_MAP
}

//--------------------------------------------------------------------------------------------------------------

BOOL DlgBWChart::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOXMENU);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// set dialog font
	GetDlgItem(IDC_MENU_BROWSER)->SetFont(m_pLayerFont);
	GetDlgItem(IDC_MENU_CHARTS)->SetFont(m_pLayerFont);
	GetDlgItem(IDC_MENU_FAVORITES)->SetFont(m_pLayerFont);
	GetDlgItem(IDC_MENU_AKAS)->SetFont(m_pLayerFont);
	GetDlgItem(IDC_MENU_OPTIONS)->SetFont(m_pLayerFont);

	// change title
	SetWindowText("BWChart "NVERSION" (C) 2003 JCA");

	// create dialogs
	m_dlgOptions->Create(DlgMainOptions::IDD,this);
	m_dlgStats->Create(DlgStats::IDD,this);
	m_dlgBrowser->Create(DlgBrowser::IDD,this);
 	m_dlgFavs->Create(DlgFavorites::IDD,this);
 	m_dlgAkas->Create(DlgAkas::IDD,this);

	// set position & size of everything
	//_Resize(0,0);

	// select default tab
	int deftab = AfxGetApp()->GetProfileInt("BWCHART_STATS","DEFTAB",1);

	// auto refresh?
	if(m_dlgBrowser->m_autoRefresh) 
		m_dlgBrowser->PostMessage(WM_AUTOREFRESH);

	// need to load a replay?
	if(!m_cmdline.IsEmpty())
	{
		m_cmdline.Replace('\"',' ');
		m_cmdline.TrimLeft();
		m_cmdline.TrimRight();
		if(_access(m_cmdline,0)==0)
		{
			m_dlgStats->LoadReplay(m_cmdline,true);
			deftab=0;
		}
	}
	else
	{
		// load replay on startup
		CString rep = AfxGetApp()->GetProfileString("MAIN","LASTREPLAY");
		if(!rep.IsEmpty() && pGetOptions()->m_autoLoad) m_dlgStats->LoadReplay(rep,true);
	}

	// display main dialog
	_UpdateScreen(deftab);

	// size window
	CSize cs;
	int x = AfxGetApp()->GetProfileInt("MAINFRAME","X",-1);
	int y = AfxGetApp()->GetProfileInt("MAINFRAME","Y",-1);
	int cx = AfxGetApp()->GetProfileInt("MAINFRAME","CX",-1);
	int cy = AfxGetApp()->GetProfileInt("MAINFRAME","CY",-1);
	if(x>=0 && y>=0 && cx>160 && cy>40) SetWindowPos(0,x,y,cx,cy,0);

	// The one and only window has been initialized, so show and update it.
	if(AfxGetApp()->GetProfileInt("MAINFRAME","MAXIMIZED",0)!=0)
		ShowWindow(SW_SHOWMAXIMIZED);

	// set position & size of everything
	_Resize(0,0);

	// check for udpate
	m_timer = SetTimer(1,5000,0);

	initdone=true;

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//--------------------------------------------------------------------------------------------------------------

void DlgBWChart::_Resize(int cx, int cy)
{
	if(!::IsWindow(m_dlgStats->GetSafeHwnd())) return;

	// Get size of dialog window.
	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(2,8);
	rect.top += 40;

	// resize inner dialogs
	m_dlgStats->MoveWindow(&rect, TRUE);
	m_dlgBrowser->MoveWindow(&rect, TRUE);
	m_dlgFavs->MoveWindow(&rect, TRUE);
	m_dlgAkas->MoveWindow(&rect, TRUE);
	m_dlgOptions->MoveWindow(&rect, TRUE);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBWChart::_UpdateScreen(int tab)
{
	static bool saveOptions=false;

	if(saveOptions)
	{
		saveOptions=false;
		m_dlgOptions->SaveValues();
	}

	if(tab!=0) m_dlgStats->StopAnimation();

	switch(tab)
	{
	case 0:
		if(m_dlgAkas) m_dlgAkas->ShowWindow(SW_HIDE);
		m_dlgStats->ShowWindow(SW_SHOW);
		m_dlgBrowser->ShowWindow(SW_HIDE);
		m_dlgFavs->ShowWindow(SW_HIDE);
		m_dlgOptions->ShowWindow(SW_HIDE);
		m_dlgStats->GetDlgItem(IDC_CHARTTYPE)->SetFocus();	// list ctrl bug, must set focus to button
		break;
	case 1:
		if(m_dlgAkas) m_dlgAkas->ShowWindow(SW_HIDE);
		m_dlgBrowser->ShowWindow(SW_SHOW);
		m_dlgBrowser->GetDlgItem(IDC_REFRESH)->SetFocus();	// list ctrl bug, must set focus to button
		m_dlgStats->ShowWindow(SW_HIDE);
		m_dlgFavs->ShowWindow(SW_HIDE);
		m_dlgOptions->ShowWindow(SW_HIDE);
		break;
	case 2:
		if(m_dlgAkas) m_dlgAkas->ShowWindow(SW_HIDE);
		m_dlgStats->ShowWindow(SW_HIDE);
		m_dlgBrowser->ShowWindow(SW_HIDE);
		m_dlgFavs->GetDlgItem(IDC_EDITCOMMENT)->SetFocus();	// list ctrl bug, must set focus to button
		m_dlgFavs->ShowWindow(SW_SHOW);
		m_dlgOptions->ShowWindow(SW_HIDE);
		break;
	case 3:
		m_dlgStats->ShowWindow(SW_HIDE);
		m_dlgBrowser->ShowWindow(SW_HIDE);
		m_dlgFavs->ShowWindow(SW_HIDE);
		m_dlgAkas->GetDlgItem(IDC_PLAYERS)->SetFocus();	// list ctrl bug, must set focus to button
		m_dlgAkas->ShowWindow(SW_SHOW);
		m_dlgOptions->ShowWindow(SW_HIDE);
		break;
	case 4:
		m_dlgStats->ShowWindow(SW_HIDE);
		m_dlgBrowser->ShowWindow(SW_HIDE);
		m_dlgFavs->ShowWindow(SW_HIDE);
		m_dlgAkas->ShowWindow(SW_HIDE);
		//m_dlgOptions->GetDlgItem(IDC_TIMEWAIT)->SetFocus();	// list ctrl bug, must set focus to button
		m_dlgOptions->ShowWindow(SW_SHOW);
		saveOptions=true;
		break;
	}

	AfxGetApp()->WriteProfileInt("BWCHART_STATS","DEFTAB",tab) ;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBWChart::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	if(cx!=0 && cy!=0) _Resize(cx, cy);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBWChart::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

//--------------------------------------------------------------------------------------------------------------

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR DlgBWChart::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBWChart::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
}

//----------------------------------------------------------------------------------------

LRESULT DlgBWChart::OnRemoteLoadReplay(WPARAM w, LPARAM lClear)
{
	ReplayInfo *rep = (ReplayInfo *)w;
	if(rep==0) return 0L;

	// load replay
	m_dlgStats->LoadReplay(rep->m_path,lClear?true:false);

	// display stats
	_UpdateScreen(0);
	return 0L;
}

//----------------------------------------------------------------------------------------

LRESULT DlgBWChart::OnRemoteAddFavorite(WPARAM w, LPARAM lClear)
{
	ReplayInfo *rep = (ReplayInfo *)w;
	if(rep==0) return 0L;

	// add replay to favorites
	m_dlgFavs->SaveFavorite(rep);
	return 0L;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBWChart::OnDestroy() 
{
	// kill timer if any
	if(m_timer!=0) KillTimer(m_timer);

	// get window position
	CRect rect;
	GetWindowRect(&rect);
	
	// save it
	if(!IsZoomed() && !IsIconic())
	{
		AfxGetApp()->WriteProfileInt("MAINFRAME","X",rect.left);
		AfxGetApp()->WriteProfileInt("MAINFRAME","Y",rect.top);
		AfxGetApp()->WriteProfileInt("MAINFRAME","CX",rect.Width());
		AfxGetApp()->WriteProfileInt("MAINFRAME","CY",rect.Height());
	}
	AfxGetApp()->WriteProfileInt("MAINFRAME","MAXIMIZED",IsZoomed());

	CDialog::OnDestroy();
}

//----------------------------------------------------------------------------------------

LRESULT DlgBWChart::OnCheckForUpdate(WPARAM , LPARAM )
{
	DlgOptions::CheckForUpdate(true);
	return 0L;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBWChart::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent==1)
	{
		// kill current time
		KillTimer(m_timer);
		m_timer=0;

		// post message for version update check
		bool autoCheck = AfxGetApp()->GetProfileInt("BWCHART_OPTIONS","AUTOCHECK",1)?true:false;
		if(autoCheck) PostMessage(WM_CHECKFORUPDATE);

		// start timer for new replay polling
		m_timer = SetTimer(2,3000,0);
		m_handler.Start();
	}
	else if(nIDEvent==2)
	{
		// do we have a new replay?
		if(pGetOptions()->m_autoadd && m_handler.CollectReplay())
			if(!m_dlgBrowser->AddReplay(m_handler.GetReplay(),false,true))
				 m_dlgStats->LoadReplay(m_handler.GetReplay(),true);

	}
	CDialog::OnTimer(nIDEvent);
}

//--------------------------------------------------------------------------------------------------------------

BOOL DlgBWChart::OnEraseBkgnd(CDC* pDC) 
{
	COLORREF clr = GetSysColor(COLOR_BTNFACE);
	COLORREF clr2 = CHsvRgb::Darker(clr,0.68);
	COLORREF clr3 = CHsvRgb::Darker(clr,0.88);

	// fill top part of screen
	CRect rect;
	GetClientRect(&rect);
	rect.bottom=40;
	Gradient::Fill(pDC,rect,clr3,clr2,GRADIENT_FILL_RECT_V);
	//pDC->FillSolidRect(&rect,clr);

	// draw separation
	rect.top=rect.bottom-2;
	pDC->Draw3dRect(&rect,clr3,clr2);

	// fill bottom part of screen
	GetClientRect(&rect);
	rect.top=40;
	pDC->FillSolidRect(&rect,clr);
	
	return TRUE;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBWChart::OnMenuAkas() 
{
	_UpdateScreen(3);
}

void DlgBWChart::OnMenuBrowser() 
{
	_UpdateScreen(1);
	
}

void DlgBWChart::OnMenuCharts() 
{
	_UpdateScreen(0);
}

void DlgBWChart::OnMenuFavorites() 
{
	_UpdateScreen(2);	
}

void DlgBWChart::OnMenuOptions() 
{
	_UpdateScreen(4);	
}

//--------------------------------------------------------------------------------------------------------------
