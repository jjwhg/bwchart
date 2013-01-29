// DlgStats.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgStats.h"
#include "regparam.h"
#include "bwrepapi.h"
#include "DlgHelp.h"
#include "DlgMap.h"
#include "Dlgbwchart.h"
#include "gradient.h"
#include "hsvrgb.h"
#include "overlaywnd.h"
#include "ExportCoachDlg.h"
#include "bezier.h"
#include <math.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAXPLAYERONVIEW 6
#define HSCROLL_DIVIDER 2
#define TIMERSPEED 8

//-----------------------------------------------------------------------------------------------------------------

// constants for chart painting
int hleft=30;
const int haxisleft = 2;
const int vplayer=30;
const int hplayer=52;
const int hright=8;
const int vtop=24;
const int vbottom=12;
const int evtWidth=4;
const int evtHeight=8;
const int layerHeight=14;
const COLORREF clrCursor = RGB(200,50,50); 
const COLORREF clrMineral = RGB(50,50,150); 
const COLORREF clrGrid = RGB(64,64,64); 
const COLORREF clrLayer1 = RGB(32,32,32); 
const COLORREF clrLayer2 = RGB(16,16,16); 
const COLORREF clrLayerTxt1 = RGB(0,0,0); 
const COLORREF clrLayerTxt2 = RGB(64,64,64); 
const COLORREF clrAction = RGB(255,206,70); 
const COLORREF clrBOLine[3] = {RGB(30,30,120),RGB(80,30,30),RGB(30,80,30)};
const COLORREF clrBOName[3] = {RGB(100,150,190), RGB(200,50,90), RGB(50,200,90) };

#define clrPlayer OPTIONSCHART->GetColor(DlgOptionsChart::CLR_PLAYERS) 
#define clrUnitName OPTIONSCHART->GetColor(DlgOptionsChart::CLR_OTHER) 
#define clrRatio OPTIONSCHART->GetColor(DlgOptionsChart::CLR_OTHER) 
#define clrTime OPTIONSCHART->GetColor(DlgOptionsChart::CLR_OTHER) 

static bool firstPoint=true;
static int lastMaxX=0;
static int lastHotPointX=0;
static const char *lastHotPoint=0;

static CString strDrop;
static CString strExpand;
static CString strLeave;
static CString strMinGas;
static CString strSupplyUnit;
static CString strMinimapPing;

// dividers for splines
static int gSplineCount[DlgStats::__MAXCHART]=
{
	//APM
	200,
	//RESOURCES,
	200,
	//UNITS
	200,
	//ACTIONS
	0,
	//BUILDINGS
	0,
	//UPGRADES
	0,
	//APMDIST
	0,
	//BUILDORDER
	0,
	//HOTKEYS
	0,
	//MAPCOVERAGE
	100,
	//MIX_APMHOTKEYS
	0
};

//-----------------------------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DlgStats, CDialog)
	//{{AFX_MSG_MAP(DlgStats)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_GETEVENTS, OnGetevents)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTEVENTS, OnItemchangedListevents)
	ON_WM_LBUTTONDOWN()
	ON_CBN_SELCHANGE(IDC_ZOOM, OnSelchangeZoom)
	ON_WM_HSCROLL()
	ON_NOTIFY(NM_DBLCLK, IDC_PLSTATS, OnDblclkPlstats)
	ON_BN_CLICKED(IDC_GAZ, OnUpdateChart)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_CHARTTYPE, OnSelchangeCharttype)
	ON_BN_CLICKED(IDC_DHELP, OnHelp)
	ON_BN_CLICKED(IDC_TESTREPLAYS, OnTestreplays)
	ON_BN_CLICKED(IDC_ADDEVENT, OnAddevent)
	ON_NOTIFY(NM_RCLICK, IDC_PLSTATS, OnRclickPlstats)
	ON_BN_CLICKED(IDC_SEEMAP, OnSeemap)
	ON_BN_CLICKED(IDC_ANIMATE, OnAnimate)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_PAUSE, OnPause)
	ON_BN_CLICKED(IDC_SPEEDMINUS, OnSpeedminus)
	ON_BN_CLICKED(IDC_SPEEDPLUS, OnSpeedplus)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LISTEVENTS, OnGetdispinfoListevents)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY(NM_RCLICK, IDC_LISTEVENTS, OnRclickListevents)
	ON_BN_CLICKED(IDC_FLT_SELECT, OnFilterChange)
	ON_BN_CLICKED(IDC_NEXT_SUSPECT, OnNextSuspect)
	ON_BN_CLICKED(IDC_MINERALS, OnUpdateChart)
	ON_BN_CLICKED(IDC_SUPPLY, OnUpdateChart)
	ON_BN_CLICKED(IDC_ACTIONS, OnUpdateChart)
	ON_BN_CLICKED(IDC_UNITS, OnUpdateChart)
	ON_BN_CLICKED(IDC_USESECONDS, OnUpdateChart)
	ON_BN_CLICKED(IDC_SPEED, OnUpdateChart)
	ON_BN_CLICKED(IDC_SINGLECHART, OnUpdateChart)
	ON_BN_CLICKED(IDC_UNITSONBO, OnUpdateChart)
	ON_BN_CLICKED(IDC_HOTPOINTS, OnUpdateChart)
	ON_BN_CLICKED(IDC_UPM, OnUpdateChart)
	ON_BN_CLICKED(IDC_BPM, OnUpdateChart)
	ON_BN_CLICKED(IDC_PERCENTAGE, OnUpdateChart)
	ON_BN_CLICKED(IDC_HKSELECT, OnUpdateChart)
	ON_BN_CLICKED(IDC_FLT_BUILD, OnFilterChange)
	ON_BN_CLICKED(IDC_FLT_TRAIN, OnFilterChange)
	ON_BN_CLICKED(IDC_FLT_SUSPECT, OnFilterChange)
	ON_BN_CLICKED(IDC_FLT_HACK, OnFilterChange)
	ON_BN_CLICKED(IDC_FLT_OTHERS, OnFilterChange)
	ON_BN_CLICKED(IDC_FLT_CHAT, OnFilterChange)
	ON_BN_CLICKED(IDC_SORTDIST, OnUpdateChart)
	ON_CBN_SELCHANGE(IDC_APMSTYLE, OnSelchangeApmstyle)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID__REMOVEPLAYER,OnRemovePlayer)
	ON_COMMAND(ID__ENABLEDISABLE,OnEnableDisable)
	ON_COMMAND(ID__WATCHREPLAY_BW116,OnWatchReplay116)
	ON_COMMAND(ID__WATCHREPLAY_BW115,OnWatchReplay115)
	ON_COMMAND(ID__WATCHREPLAY_BW114,OnWatchReplay114)
	ON_COMMAND(ID__WATCHREPLAY_BW113,OnWatchReplay113)
	ON_COMMAND(ID__WATCHREPLAY_BW112,OnWatchReplay112)
	ON_COMMAND(ID__WATCHREPLAY_BW111,OnWatchReplay111)
	ON_COMMAND(ID__WATCHREPLAY_BW110,OnWatchReplay110)
	ON_COMMAND(ID__WATCHREPLAY_BW109,OnWatchReplay109)
	ON_COMMAND(ID__WATCHREPLAYINBW_SC,OnWatchReplaySC)
	ON_COMMAND(ID__WATCHREPLAYINBW_AUTO,OnWatchReplay)
	ON_COMMAND(ID_FF_EXPORT_TOTEXTFILE,OnExportToText)
	ON_COMMAND(ID__EXPORTTO_BWCOACH,OnExportToBWCoach)
//	ON_COMMAND(ID_FF_EXPORTTO_HTMLFILE,OnExportToHtml)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_Parameters(bool bLoad)
{
	int defval[__MAXCHART];
	int defvalUnits[__MAXCHART];
	int defvalApm[__MAXCHART];
	memset(defval,0,sizeof(defval));
	memset(defvalUnits,0,sizeof(defvalUnits));
	memset(defvalApm,0,sizeof(defval));
	defvalUnits[MAPCOVERAGE]=1;
	defvalApm[MAPCOVERAGE]=2;
	defvalApm[APM]=2;

	PINT("main",seeMinerals,TRUE);
	PINT("main",seeGaz,TRUE);
	PINT("main",seeSupply,FALSE);
	PINT("main",seeActions,FALSE);
	PINTARRAY("main",singleChart,__MAXCHART,defval);
	PINTARRAY("main",seeUnits,__MAXCHART,defvalUnits);
	PINTARRAY("main",apmStyle,__MAXCHART,defvalApm);
	PINT("main",useSeconds,FALSE);
	PINT("main",seeSpeed,FALSE);
	PINT("main",seeUPM,FALSE);
	PINT("main",seeBPM,FALSE);
	PINT("main",chartType,0);
	PINT("main",seeUnitsOnBO,TRUE);
	PINT("main",seeHotPoints,TRUE);
	PINT("main",seePercent,TRUE);
	PINT("main",animationSpeed,2);
	PINT("main",hlist,120);
	PINT("main",wlist,0);
	PINT("main",viewHKselect,FALSE);
	PINT("filter",fltSelect,TRUE);
	PINT("filter",fltTrain,TRUE);
	PINT("filter",fltBuild,TRUE);
	PINT("filter",fltSuspect,TRUE);
	PINT("filter",fltHack,TRUE);
	PINT("filter",fltOthers,TRUE);
	PINT("filter",fltChat,TRUE);
	PINT("main",sortDist,FALSE);
}

//-----------------------------------------------------------------------------------------------------------------

/*
static void _GetRemoteItemText(HANDLE hProcess, HWND hlv, LV_ITEM* plvi, int item, int subitem, char *itemText)
{
	// Initialize a local LV_ITEM structure
	LV_ITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.iItem = item;
	lvi.iSubItem = subitem; 
	// NOTE: The text data immediately follows the LV_ITEM structure
	//       in the memory block allocated in the remote process.
	lvi.pszText = (LPTSTR) (plvi + 1); 
	lvi.cchTextMax = 100; 

	// Write the local LV_ITEM structure to the remote memory block
	if(!WriteProcessMemory(hProcess, plvi, &lvi, sizeof(lvi), NULL))
	{
		::MessageBox(0,__TEXT("Cant write into SuperView's memory"), "bwchart", MB_OK | MB_ICONWARNING);
		return;
	}

	// Tell the ListView control to fill the remote LV_ITEM structure
	ListView_GetItem(hlv, plvi);

	// Read the remote text string into our buffer
	if(!ReadProcessMemory(hProcess, plvi + 1, itemText, 256, NULL))
	{
		::MessageBox(0,__TEXT("Cant read into SuperView's memory"), "bwchart", MB_OK | MB_ICONWARNING);
		return;
	}
}

//-----------------------------------------------------------------------------------------------------------------

typedef
LPVOID (__stdcall * PFNVIRTALLEX)(HANDLE, LPVOID, SIZE_T, DWORD,DWORD);
static HANDLE ghFileMapping=0;

static PVOID _AllocSharedMemory(HANDLE hProcess, int size)
{
    OSVERSIONINFO osvi = { sizeof(osvi) };

    GetVersionEx( &osvi );

    if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
       // We're on NT, so use VirtualAllocEx to allocate memory in the other
        // process address space.  Alas, we can't just call VirtualAllocEx
        // since it's not defined in the Windows 95 KERNEL32.DLL.
		return VirtualAllocEx(hProcess,	NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    }
    else if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
    {
        // In Windows 9X, create a small memory mapped file.  On this
        // platform, memory mapped files are above 2GB, and thus are
        // accessible by all processes.
        ghFileMapping = CreateFileMapping(
                                    INVALID_HANDLE_VALUE, 0,
                                    PAGE_READWRITE | SEC_COMMIT,
                                    0,
                                    size,
                                    0 );
        if ( ghFileMapping )
        {
            LPVOID pStubMemory = MapViewOfFile( ghFileMapping,
                                                FILE_MAP_WRITE,
                                                0, 0,
                                                size );
            return pStubMemory;
        }
        else
		{
            CloseHandle( ghFileMapping );
			ghFileMapping=0;
		}
    }

    return 0;
}

//-----------------------------------------------------------------------------------------------------------------

static void _FreeSharedMemory(HANDLE hProcess, PVOID padr)
{
    OSVERSIONINFO osvi = { sizeof(osvi) };

    GetVersionEx( &osvi );

    if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
         // We're on NT, so use VirtualFreeEx 
        VirtualFreeEx(hProcess, padr, 0, MEM_RELEASE);
    }
    else if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
    {
        // In Windows 9X, create a small memory mapped file.  On this
        // platform, memory mapped files are above 2GB, and thus are
        // accessible by all processes.
 		UnmapViewOfFile(padr);
		CloseHandle( ghFileMapping );
    }
}

//-----------------------------------------------------------------------------------------------------------------


#define CMPUNIT(val,rval) if(_stricmp(itemParam,val)==0) {strcpy(itemParam,rval); return;}
#define CMPBUILD(val,rval) if(_stricmp(lastp,val)==0) {strcpy(lastp,rval); return;}

// adjust values
static void _AdjustAction(char *itemType, char *itemParam)
{
	if(_stricmp(itemType,"Train")==0)
	{
		CMPUNIT("46","Scout");
		CMPUNIT("47","Arbiter");
		CMPUNIT("3C","Corsair");
		CMPUNIT("45","Shuttle");
		CMPUNIT("53","Reaver");
		CMPUNIT("01","Ghost");
		CMPUNIT("0C","Battlecruiser");
		CMPUNIT("Vulture","Goliath");
		CMPUNIT("Goliath","Vulture");
		CMPUNIT("0E","Nuke");
		return;
	}
	else if(_stricmp(itemType,"2A")==0) {strcpy(itemType,"Train"); strcpy(itemParam,"Archon"); return;}
	else if(_stricmp(itemType,"5A")==0) {strcpy(itemType,"Train"); strcpy(itemParam,"Dark Archon"); return;}
	else if(_stricmp(itemType,"Build")==0)
	{
		// extract building name
		char *lastp=strrchr(itemParam,',');
		if(lastp!=0) lastp++; else lastp=itemParam;
		while(*lastp==' ') lastp++; 

		CMPBUILD("AA","Arbiter Tribunal");
		CMPBUILD("AB","Robotics Support Bay");
		CMPBUILD("AC","Shield Battery");
		CMPBUILD("75","Covert Ops");
		CMPBUILD("76","Physics Lab");
		CMPBUILD("6C","Nuclear Silo");
		CMPBUILD("Acadamy","Academy");
		return;
	}
	else if(_stricmp(itemType,"Research")==0 || _stricmp(itemType,"Upgrade")==0)
	{
		CMPUNIT("27","Gravitic Booster");
		CMPUNIT("22","Zealot Speed");
		CMPUNIT("15","Recall");
		CMPUNIT("0E","Protoss Air Attack");
		CMPUNIT("23","Scarab Damage");
		CMPUNIT("26","Sensor Array");
		CMPUNIT("16","Statis Field");
		CMPUNIT("14","Hallucination");
		CMPUNIT("0F","Plasma Shield");
		CMPUNIT("24","Reaver Capacity");
		CMPUNIT("2C","Khaydarin Core");
		CMPUNIT("28","Khaydarin Amulet");
		CMPUNIT("29","Apial Sensors");
		CMPUNIT("25","Gravitic Drive");
		CMPUNIT("1B","Mind Control");
		CMPUNIT("2A","Gravitic Thrusters");
		CMPUNIT("1F","Maelstrom");
		CMPUNIT("31","Argus Talisman");
		CMPUNIT("19","Disruption Web");
		CMPUNIT("2F","Argus Jewel");
		CMPUNIT("01","Lockdown");
		CMPUNIT("02","EMP Shockwave");
		CMPUNIT("09","Cloaking Field");
		CMPUNIT("11","Vulture Speed");
		CMPUNIT("0A","Personal Cloaking");
		CMPUNIT("08","Yamato Gun");
		CMPUNIT("17","Colossus Reactor");
		CMPUNIT("18","Restoration");
		CMPUNIT("1E","Optical Flare");
		CMPUNIT("33","Medic Energy");
		return;
	}
}

//-----------------------------------------------------------------------------------------------------------------

bool  DlgStats::_GetListViewContent(HWND hlv)
{
	// Get the count of items in the ListView control
	int nCount = ListView_GetItemCount(hlv);

	// Open a handle to the remote process's kernel object
	DWORD dwProcessId;
	GetWindowThreadProcessId(hlv, &dwProcessId);
	HANDLE hProcess = OpenProcess(
	PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, 
	FALSE, dwProcessId);

	if (hProcess == NULL) {
		MessageBox(__TEXT("Could not communicate with SuperView"), "bwchart", MB_OK | MB_ICONWARNING);
		return false;
	}

	// Allocate memory in the remote process's address space
	LV_ITEM* plvi = (LV_ITEM*) //VirtualAllocEx(hProcess,	NULL, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		_AllocSharedMemory(hProcess, 4096);

 	if (plvi == NULL) {
		DWORD dw=GetLastError();
		CString msg;
		msg.Format("Could not allocate virtual memory %lu",dw);
		MessageBox("Sorry, but BWchart cannot work under Windows 95/98", "bwchart", MB_OK | MB_ICONWARNING);
		return false;
	}

	// Get each ListView item's text data
	for (int nIndex = 0; nIndex < nCount; nIndex++) 
	{
		// read elapsed time
		char itemTime[256];
		_GetRemoteItemText(hProcess, hlv, plvi, nIndex, 0, itemTime);
		int nPos = m_listEvents.InsertItem(nIndex,itemTime, 0);
		m_listEvents.SetItemData(nPos,(DWORD)_atoi64(itemTime));

		// read player name
		char itemPlayer[256];
		_GetRemoteItemText(hProcess, hlv, plvi, nIndex, 1, itemPlayer);
		m_listEvents.SetItemText(nPos,1,itemPlayer);

		// read event type
		char itemType[256];
		_GetRemoteItemText(hProcess, hlv, plvi, nIndex, 2, itemType);

		// read event parameters
		char itemParam[256];
		_GetRemoteItemText(hProcess, hlv, plvi, nIndex, 3, itemParam);

		// adjust values
		_AdjustAction(itemType,itemParam);

		// update list view
		m_listEvents.SetItemText(nPos,2,itemType);
		m_listEvents.SetItemText(nPos,3,itemParam);

		// record event
		//m_replay.AddEvent(atoi(itemTime),itemPlayer,itemType,itemParam);
	}

	// Free the memory in the remote process's address space
	_FreeSharedMemory(hProcess, plvi);
	//VirtualFreeEx(hProcess, plvi, 0, MEM_RELEASE);

	// Cleanup and put our results on the clipboard
	CloseHandle(hProcess);

	return true;
}

//-----------------------------------------------------------------------------------------------------------------

BOOL CALLBACK EnumChildProc(
  HWND hwnd,      // handle to child window
  LPARAM lParam   // application-defined value
)
{
	char classname[128];
	GetClassName(hwnd,classname,sizeof(classname));
	if(_stricmp(classname,"SysListView32")==0)
	{
		HWND hlv = ::GetNextWindow(hwnd, GW_HWNDNEXT);
		if(hlv==0) return FALSE;

		hlv = ::GetNextWindow(hlv, GW_HWNDNEXT);
		if(hlv==0) return FALSE;

		*((HWND*)lParam) = hlv;
		return FALSE;
	}
	return TRUE;
}
*/

//-----------------------------------------------------------------------------------------------------------------

bool DlgStats::_GetReplayFileName(CString& path)
{
	static char BASED_CODE szFilter[] = "Replay (*.rep)|*.rep|All Files (*.*)|*.*||";

	// find initial path
	CString initialPath;
	initialPath = AfxGetApp()->GetProfileString("MAIN","LASTADDREPLAY");
	if(initialPath.IsEmpty()) DlgOptions::_GetStarcraftPath(initialPath);

	// stop animation if any
	StopAnimation(); 

	// open dialog
 	CFileDialog dlg(TRUE,"rep","",0,szFilter,this);
	dlg.m_ofn.lpstrInitialDir = initialPath;
	if(dlg.DoModal()==IDOK)
	{
		CWaitCursor wait;
		path = dlg.GetPathName();
		AfxGetApp()->WriteProfileString("MAIN","LASTADDREPLAY",path);
		return true;
	}

	return false;
}



/////////////////////////////////////////////////////////////////////////////
// DlgStats dialog

DlgStats::DlgStats(CWnd* pParent /*=NULL*/)
	: CDialog(DlgStats::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgStats)
	m_zoom = 0;
	m_seeMinerals = TRUE;
	m_seeGaz = TRUE;
	m_seeSupply = FALSE;
	m_seeActions = FALSE;
	m_useSeconds = FALSE;
	m_seeSpeed = FALSE;
	m_chartType = -1;
	m_seeUnitsOnBO = FALSE;
	m_exactTime = _T("");
	m_seeBPM = FALSE;
	m_seeUPM = FALSE;
	m_seeHotPoints = FALSE;
	m_seePercent = FALSE;
	m_viewHKselect = FALSE;
	m_fltSelect = FALSE;
	m_fltSuspect = FALSE;
	m_fltHack = FALSE;
	m_fltTrain = FALSE;
	m_fltBuild = FALSE;
	m_fltOthers = FALSE;
	m_fltChat = FALSE;
	m_suspectInfo = _T("");
	m_sortDist = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_timeBegin = 0;
	m_timeEnd = 0;
	m_lockListView=false;
	m_selectedPlayer=0;
	m_selectedAction=-1;
	m_maxPlayerOnBoard=0;
	m_selectedPlayerList=0;
	m_bIsAnimating=false;
	m_animationSpeed=2;
	m_prevAnimationSpeed=0;
	m_timer=0;
	m_mixedCount=0;
	m_MixedPlayerIdx=0;

	// map title
	m_rectMapName.SetRectEmpty();

	// resize
	m_resizing=NONE;
	m_hlist=120;
	m_wlist=0;

	// load parameters
	_Parameters(true);
	if(m_hlist<60) m_hlist=60;

	// scroller increments
	m_lineDev.cx = 5;
	m_lineDev.cy = 0;
	m_pageDev.cx = 50;
	m_pageDev.cy = 0;

	// create all fonts
	_CreateFonts();

	// create map
	m_dlgmap = new DlgMap(0,m_pLabelBoldFont,m_pLayerFont,this);

	// create overlay
	m_over = new OverlayWnd(this);

	// init time
	m_exactTime = _MkTime(0,0,true);
}

DlgStats::~DlgStats() 
{
	// delete overlay
	m_over->DestroyWindow();
	delete m_over;

	//delete map
	delete m_dlgmap; 

	// delete fonts
	_DestroyFonts(); 
}

void DlgStats::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgStats)
	DDX_Check(pDX, IDC_SINGLECHART, m_singleChart[m_chartType]);
	DDX_Check(pDX, IDC_UNITS, m_seeUnits[m_chartType]);
	DDX_CBIndex(pDX, IDC_APMSTYLE, m_apmStyle[m_chartType]);
	DDX_CBIndex(pDX, IDC_CHARTTYPE, m_chartType);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_GAMEDURATION, m_gamed);
	DDX_Control(pDX, IDC_DBLCLICK, m_dlbclick);
	DDX_Control(pDX, IDC_PLSTATS, m_plStats);
	DDX_Control(pDX, IDC_SCROLLBAR2, m_scrollerV);
	DDX_Control(pDX, IDC_SCROLLBAR1, m_scroller);
	DDX_Control(pDX, IDC_LISTEVENTS, m_listEvents);
	DDX_CBIndex(pDX, IDC_ZOOM, m_zoom);
	DDX_Check(pDX, IDC_MINERALS, m_seeMinerals);
	DDX_Check(pDX, IDC_GAZ, m_seeGaz);
	DDX_Check(pDX, IDC_SUPPLY, m_seeSupply);
	DDX_Check(pDX, IDC_ACTIONS, m_seeActions);
	DDX_Check(pDX, IDC_USESECONDS, m_useSeconds);
	DDX_Check(pDX, IDC_SPEED, m_seeSpeed);
	DDX_Check(pDX, IDC_UNITSONBO, m_seeUnitsOnBO);
	DDX_Text(pDX, IDC_EXACTTIME, m_exactTime);
	DDX_Check(pDX, IDC_BPM, m_seeBPM);
	DDX_Check(pDX, IDC_UPM, m_seeUPM);
	DDX_Check(pDX, IDC_HOTPOINTS, m_seeHotPoints);
	DDX_Check(pDX, IDC_PERCENTAGE, m_seePercent);
	DDX_Check(pDX, IDC_HKSELECT, m_viewHKselect);
	DDX_Check(pDX, IDC_FLT_SELECT, m_fltSelect);
	DDX_Check(pDX, IDC_FLT_CHAT, m_fltChat);
	DDX_Check(pDX, IDC_FLT_SUSPECT, m_fltSuspect);
	DDX_Check(pDX, IDC_FLT_HACK, m_fltHack);
	DDX_Check(pDX, IDC_FLT_TRAIN, m_fltTrain);
	DDX_Check(pDX, IDC_FLT_BUILD, m_fltBuild);
	DDX_Check(pDX, IDC_FLT_OTHERS, m_fltOthers);
	DDX_Text(pDX, IDC_SUSPECT_INFO, m_suspectInfo);
	DDX_Check(pDX, IDC_SORTDIST, m_sortDist);
	//}}AFX_DATA_MAP
}

//------------------------------------------------------------------------------------------------------------

void DlgStats::_CreateFonts()
{
	// get system language
	LANGID lng = GetSystemDefaultLangID();

	//if(lng==0x412) // korea
	//if(lng==0x0804) //chinese

	// retrieve a standard VARIABLE FONT from system
	HFONT hFont = (HFONT)GetStockObject( ANSI_VAR_FONT);
	CFont *pRefFont = CFont::FromHandle(hFont);
	LOGFONT LFont;

	// get font description
	memset(&LFont,0,sizeof(LOGFONT));
	pRefFont->GetLogFont(&LFont);

	// create our label Font
	LFont.lfHeight = 18;
	LFont.lfWidth = 0;
	LFont.lfQuality|=ANTIALIASED_QUALITY;
	strcpy(LFont.lfFaceName,"Arial");
	m_pLabelFont = new CFont();
	m_pLabelFont->CreateFontIndirect(&LFont);

	// create our bold label Font
	LFont.lfWeight=700;
	m_pLabelBoldFont = new CFont();
	m_pLabelBoldFont->CreateFontIndirect(&LFont);

	// create our layer Font
	pRefFont->GetLogFont(&LFont);
	LFont.lfHeight = 14;
	LFont.lfWidth = 0;
	LFont.lfQuality|=ANTIALIASED_QUALITY;
	strcpy(LFont.lfFaceName,"Arial");
	m_pLayerFont = new CFont();
	m_pLayerFont->CreateFontIndirect(&LFont);

	// create our small label Font
	LFont.lfHeight = 10;
	LFont.lfWidth = 0;
	LFont.lfQuality&=~ANTIALIASED_QUALITY;
	strcpy(LFont.lfFaceName,"Small Fonts");
	m_pSmallFont = new CFont();
	m_pSmallFont->CreateFontIndirect(&LFont);

	// create image list
	m_pImageList = new CImageList();
}

//------------------------------------------------------------------------------------------------------------

void DlgStats::_DestroyFonts()
{
	delete m_pLabelBoldFont;
	delete m_pLabelFont;
	delete m_pSmallFont;
	delete m_pLayerFont;
	delete m_pImageList;
}

//------------------------------------------------------------------------------------------------------------

// prepare list view
void DlgStats::_PrepareListView()
{
	UINT evCol[]={IDS_COL_TIME,IDS_COL_PLAYER,IDS_COL_ACTION,IDS_COL_PARAMETERS,0,IDS_COL_UNITSID};
	int evWidth[]={50,115,80,180,20,180};
	UINT stCol[]={IDS_COL_PLAYER,IDS_COL_ACTIONS,IDS_COL_APM,IDS_COL_NULL,IDS_COL_VAPM,IDS_COL_MINERAL,IDS_COL_GAS,
		IDS_COL_SUPPLY,IDS_COL_UNITS,IDS_COL_APMPM,IDS_COL_APMMAX,IDS_COL_APMMIN,IDS_COL_BASE,IDS_COL_MICRO,IDS_COL_MACRO};
	int stWidth[]={115,50,40,35,45,50,50,50,50,50,60,60,40,40,45};

	// Allow the header controls item to be movable by the user.
	// and set full row selection
	m_listEvents.SetExtendedStyle(m_listEvents.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP+LVS_EX_FULLROWSELECT);
	CString colTitle;
	for(int i=0;i<sizeof(evCol)/sizeof(evCol[0]);i++)
	{
		if(evCol[i]!=0) colTitle.LoadString(evCol[i]); else colTitle="";
		m_listEvents.InsertColumn(i, colTitle,LVCFMT_LEFT,evWidth[i],i);
	}

	// Set the background color
	COLORREF clrBack = RGB(255,255,255);
	m_listEvents.SetBkColor(clrBack);
	m_listEvents.SetTextBkColor(clrBack);
	m_listEvents.SetTextColor(RGB(10,10,10));
	m_listEvents.SubclassHeaderControl();

	// player stats
	m_plStats.SetExtendedStyle(m_plStats.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP+LVS_EX_FULLROWSELECT);
	for(int i=0;i<sizeof(stCol)/sizeof(stCol[0]);i++)
	{
		if(stCol[i]!=0) colTitle.LoadString(stCol[i]); else colTitle="";
		m_plStats.InsertColumn(i, colTitle,LVCFMT_LEFT,stWidth[i],i);
	}
	DlgBrowser::LoadColumns(&m_plStats,"plstats");

	// Set the background color
	m_plStats.SetBkColor(clrBack);
	m_plStats.SetTextBkColor(clrBack);
	m_plStats.SetTextColor(RGB(10,10,10));

	// create, initialize, and hook up image list
	m_pImageList->Create(16, 16, ILC_COLOR4, 2, 2);
	m_pImageList->SetBkColor(clrBack);

	// add icons
	CWinApp *pApp = AfxGetApp();
	m_pImageList->Add(pApp->LoadIcon(IDI_ICON_DISABLE));
	m_pImageList->Add(pApp->LoadIcon(IDI_ICON_OK));
	m_plStats.SetImageList(m_pImageList, LVSIL_SMALL);
}

//------------------------------------------------------------------------------------------------------------

void DlgStats::UpdateBkgBitmap()
{
	// re-init all pens for drawing all the charts
	_InitAllDrawingTools();

	// load background bitmap if any
	Invalidate();
}


//------------------------------------------------------------------------------------------------------------

BOOL DlgStats::OnInitDialog()
{
	CDialog::OnInitDialog();

	// init chart type
	CComboBox *charttype = (CComboBox *)GetDlgItem(IDC_CHARTTYPE);
	CString msg;
	msg.LoadString(IDS_CT_APM);
	charttype->AddString(msg);
	msg.LoadString(IDS_CT_RESOURCES);
	charttype->AddString(msg);
	msg.LoadString(IDS_CT_UNITDIST);
	charttype->AddString(msg);
	msg.LoadString(IDS_CT_ACTIONDIST);
	charttype->AddString(msg);
	msg.LoadString(IDS_CT_BUILDDIST);
	charttype->AddString(msg);
	msg.LoadString(IDS_CT_UPGRADES);
	charttype->AddString(msg);
	msg.LoadString(IDS_CT_APMDIST);
	charttype->AddString(msg);
	msg.LoadString(IDS_CT_BO);
	charttype->AddString(msg);
	msg.LoadString(IDS_CT_HOTKEYS);
	charttype->AddString(msg);
	msg.LoadString(IDS_CT_MAPCOVERGAGE);
	charttype->AddString(msg);
	msg.LoadString(IDS_CT_MIX_APMHOTKEYS);
	charttype->AddString(msg);
	charttype->SetCurSel(m_chartType);

	// prepare list view
	_PrepareListView();

	// resize
	CRect rect;
	GetClientRect(&rect);
	_Resize(rect.Width(),rect.Height());

	// update screen
	OnSelchangeCharttype();

	// create map window
	m_dlgmap->Create(DlgMap::IDD,(CWnd*)this);

	// load some strings
	strDrop.LoadString(IDS_DROP);
	strExpand.LoadString(IDS_EXPAND);
	strLeave.LoadString(IDS_LEAVE);
	strMinGas.LoadString(IDS_MINGAS);
	strSupplyUnit.LoadString(IDS_SUPPLYUNIT);
	strMinimapPing.LoadString(IDS_MINIMAPPING);


#ifndef NDEBUG
	GetDlgItem(IDC_TESTREPLAYS)->ShowWindow(SW_SHOW);
#endif

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//-----------------------------------------------------------------------------------------------------------------

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void DlgStats::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (!IsIconic())
	{
		// fill chart background
		OPTIONSCHART->PaintBackground(&dc,m_boardRect);

		// draw tracking rect for vertical resizing
		CRect resizeRect;
		_GetResizeRect(resizeRect);
		resizeRect.DeflateRect(resizeRect.Width()/3,1);
		CRect resizeRect2(resizeRect);
		resizeRect2.right = resizeRect2.left+10;
		dc.Draw3dRect(&resizeRect2,RGB(220,220,220),RGB(180,180,180));
		resizeRect2=resizeRect;
		resizeRect2.left = resizeRect2.right-10;
		dc.Draw3dRect(&resizeRect2,RGB(220,220,220),RGB(180,180,180));

		// draw tracking rect for horizontal resizing
		_GetHorizResizeRect(resizeRect);
		resizeRect.DeflateRect(2,resizeRect.Height()/3);
		resizeRect2=resizeRect;
		resizeRect2.bottom = resizeRect2.top+10;
		dc.Draw3dRect(&resizeRect2,RGB(220,220,220),RGB(180,180,180));
		resizeRect2=resizeRect;
		resizeRect2.top = resizeRect2.bottom-10;
		dc.Draw3dRect(&resizeRect2,RGB(220,220,220),RGB(180,180,180));

		// paint charts
		if(m_replay.IsDone()) _PaintCharts(&dc);
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_GetCursorRect(CRect& rect, unsigned long time, int width)
{
	// only repaint around the time cursor
	rect = m_boardRect;
	rect.left += m_dataAreaX;
	float finc = (float)(rect.Width()-hleft-hright)/(float)(m_timeEnd - m_timeBegin);
	float fx=(float)(rect.left+hleft) + finc * (time-m_timeBegin);
	rect.right = width+(int)fx;
	rect.left = -width+(int)fx;;
}

//-----------------------------------------------------------------------------------------------------------------

// bUserControl = true when the cursor is changed by the user
//
void DlgStats::_SetTimeCursor(unsigned long value, bool bUpdateListView, bool bUserControl)
{
	// repaint current cursor zone
	CRect rect;
	_GetCursorRect(rect, m_timeCursor, 2);
	InvalidateRect(rect,FALSE);

	// update cursor
	m_timeCursor = value;
	_GetCursorRect(rect, m_timeCursor, 2);
	if(m_zoom>0 && (m_timeCursor<m_timeBegin || m_timeCursor>m_timeEnd)) 
	{
		// the time window has changed, we must repaint everything
		_AdjustWindow();
		rect = m_boardRect;
	}

	// update scroller
	m_scroller.SetScrollPos(value/HSCROLL_DIVIDER);

	// if we are animating
	if(m_bIsAnimating)
	{
		// if the animation timer is the one who changed the cursor
		if(!bUserControl)
		{
			// only repaint around the time cursor
			int width = 16 + m_animationSpeed;
			_GetCursorRect(rect, m_timeCursor, width);
		}
		else
		{
			// the user has changed the cursor during the animation
			// we must repaint everything
			rect = m_boardRect;
		}
	}

	// update map
	if(bUserControl) m_dlgmap->ResetTime(m_timeCursor);

	// repaint view
	InvalidateRect(rect,FALSE);

	// update list of events
	if(bUpdateListView) 
	{
		// select corresponding line in list view
		unsigned long idx = _GetEventFromTime(m_timeCursor);
		m_lockListView=true;
		m_listEvents.SetItemState(idx,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
		m_listEvents.EnsureVisible(idx,FALSE);
		m_lockListView=false;
	}

	// update exact time
	m_exactTime = _MkTime(m_replay.QueryFile()->QueryHeader(),value,true);
	UpdateData(FALSE);
}

//-----------------------------------------------------------------------------------------------------------------

// display player stats
void DlgStats::_DisplayPlayerStats()
{
	m_plStats.DeleteAllItems();

	// for each player
	for(int i=0; i<m_replay.GetPlayerCount(); i++)
	{
		// get event list
		ReplayEvtList *list = m_replay.GetEvtList(i);

		// insert player stats
		CString str;
		int nPos = m_plStats.InsertItem(i,list->PlayerName(), list->IsEnabled()?1:0);

		str.Format("%d",list->GetEventCount());
		m_plStats.SetItemText(nPos,1,str);

		str.Format("%d",list->GetActionPerMinute());
		m_plStats.SetItemText(nPos,2,str);

		str.Format("%d",list->GetDiscardedActions());
		m_plStats.SetItemText(nPos,3,str);

		str.Format("%d",list->GetValidActionPerMinute());
		m_plStats.SetItemText(nPos,4,str);
		
		str.Format("%d",list->ResourceMax().Minerals());
		m_plStats.SetItemText(nPos,5,str);
		
		str.Format("%d",list->ResourceMax().Gaz());
		m_plStats.SetItemText(nPos,6,str);
		
		str.Format("%d",(int)list->ResourceMax().Supply());
		m_plStats.SetItemText(nPos,7,str);
		
		str.Format("%d",list->ResourceMax().Units());
		m_plStats.SetItemText(nPos,8,str);
		
		str.Format("%d",list->GetStandardAPMDev(-1,-1));
		m_plStats.SetItemText(nPos,9,str);
		
		str.Format("%d",list->ResourceMax().LegalAPM());
		m_plStats.SetItemText(nPos,10,str);

		str.Format("%d",list->GetMiniAPM());
		m_plStats.SetItemText(nPos,11,str);

		str.Format("%d",list->GetStartingLocation());
		m_plStats.SetItemText(nPos,12,str);

		str.Format("%d",list->GetMicroAPM());
		m_plStats.SetItemText(nPos,13,str);

		str.Format("%d",list->GetMacroAPM());
		m_plStats.SetItemText(nPos,14,str);

		m_plStats.SetItemData(nPos,(DWORD)list);
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::LoadReplay(const char *reppath, bool bClear) 
{
	CWaitCursor wait;

	// stop animation if any
	StopAnimation(); 

	// clear list views
	if(bClear) m_listEvents.DeleteAllItems();

	// update map
	m_dlgmap->UpdateReplay(0);

	// load replay
	if(m_replay.Load(reppath,true,&m_listEvents,bClear)!=0)
	{
		CString msg;
		msg.Format(IDS_CORRUPTEDBIS,reppath);
		MessageBox(msg,0,MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	AfxGetApp()->WriteProfileString("MAIN","LASTREPLAY",reppath);
	SetDlgItemText(IDC_REPLAYFILE,reppath);

	// udpate suspect events counts
	CString chktitle;
	int suspectCount = m_replay.GetSuspectCount();
	chktitle.Format(IDS_SUSPICIOUS,suspectCount);
	GetDlgItem(IDC_FLT_SUSPECT)->SetWindowText(chktitle);
	GetDlgItem(IDC_FLT_SUSPECT)->EnableWindow(suspectCount==0?FALSE:TRUE);

	// udpate hack events counts
	int hackCount = m_replay.GetHackCount();
	chktitle.Format(IDS_HACK,hackCount);
	GetDlgItem(IDC_FLT_HACK)->SetWindowText(chktitle);
	GetDlgItem(IDC_FLT_HACK)->EnableWindow(hackCount==0?FALSE:TRUE);

	// update "next" button
	GetDlgItem(IDC_NEXT_SUSPECT)->EnableWindow((suspectCount+hackCount)==0?FALSE:TRUE);

	// display player stats
	_DisplayPlayerStats();

	// init view
	m_maxPlayerOnBoard = m_replay.GetPlayerCount();
	m_timeBegin = 0;
	m_timeCursor = 0;
	m_timeEnd = (m_chartType==BUILDORDER) ? m_replay.GetLastBuildOrderTime() : m_replay.GetEndTime();
	m_scroller.SetScrollRange(0,m_timeEnd/HSCROLL_DIVIDER);

	// init all pens for drawing all the charts
	_InitAllDrawingTools();

	// display game duration and game date
	CTime cdate;
	CString str;
	time_t date = m_replay.QueryFile()->QueryHeader()->getCreationDate();
	if(localtime(&date)!=0) cdate = CTime(date);
	else cdate = CTime(1971,1,1,0,0,0);
	str.Format(IDS_MKDURATION,_MkTime(m_replay.QueryFile()->QueryHeader(),m_replay.GetEndTime(),true), (const char*)cdate.Format("%d %b %y"));
	SetDlgItemText(IDC_GAMEDURATION,str);

	// update map
	m_dlgmap->UpdateReplay(&m_replay);
	GetDlgItem(IDC_SEEMAP)->EnableWindow(m_replay.GetMapAnim()==0 ? FALSE :TRUE);
	if(m_replay.GetMapAnim()==0) m_dlgmap->ShowWindow(SW_HIDE);

	// init action filter
	_UpdateActionFilter(true);

	// update apm with current style
	m_replay.UpdateAPM(m_apmStyle[APM], m_apmStyle[MAPCOVERAGE]);
	
	//repaint
	Invalidate(FALSE);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnGetevents() 
{
	// get path
	CString path;
	if(!_GetReplayFileName(path))
		return;

	// load replay
	LoadReplay(path,true);
}

//-----------------------------------------------------------------------------------------------------------------

#define CHANGEPOS(id,x,y)\
{CWnd *wnd = GetDlgItem(id);\
if(wnd && ::IsWindow(wnd->GetSafeHwnd()))\
wnd->SetWindowPos(0,x,y,0,0,SWP_NOZORDER|SWP_NOSIZE);}

#define CHANGEPOSSIZE(id,x,y,w,h)\
{CWnd *wnd = GetDlgItem(id);\
if(wnd && ::IsWindow(wnd->GetSafeHwnd()))\
wnd->SetWindowPos(0,x,y,w,h,SWP_NOZORDER);}

void DlgStats::_Resize(int cx, int cy)
{
	if(!::IsWindow(m_scroller)) return;

	// if window was reduced a lot, make sure event list width & height is not too big
	if(m_wlist==0) m_wlist=min(460,(80*cx/100));
	if(m_wlist>(90*cx/100)) m_wlist = 90*cx/100;
	if(m_hlist>(70*cy/100)) m_hlist = 70*cy/100;

	int hlist=m_hlist;
	int wlist=m_wlist;

	// find position of lowest control on the top
	CRect rectCtrl;
	GetDlgItem(IDC_SPEED)->GetWindowRect(&rectCtrl);
	ScreenToClient(&rectCtrl);

	int top=rectCtrl.top+rectCtrl.Height()+8;
	int left=10;
	int right=16;
	int wlist2=cx - wlist - right-left-2;
	m_boardRect.SetRect(left,top,max(16,cx-left-right),max(top+16,cy-32-hlist));

	// scrollers
	if(::IsWindow(m_scroller)) 
		m_scroller.SetWindowPos(0,left,m_boardRect.bottom,m_boardRect.Width()-218,17,SWP_NOZORDER);
	if(::IsWindow(m_scrollerV)) 
		m_scrollerV.SetWindowPos(0,m_boardRect.right,top,17,m_boardRect.Height(),SWP_NOZORDER);

	// event list
	if(::IsWindow(m_listEvents)) 
		m_listEvents.SetWindowPos(0,m_boardRect.left,cy-hlist-8+20,wlist,hlist-40,SWP_NOZORDER);

	// cursor time
 	CHANGEPOS(IDC_EXACTTIME,m_boardRect.Width()-200,m_boardRect.bottom+2);

	// filter check boxes
	CHANGEPOS(IDC_FLT_SELECT,8+m_boardRect.left,cy-hlist-8);
	CHANGEPOS(IDC_FLT_BUILD,70+m_boardRect.left,cy-hlist-8);
	CHANGEPOS(IDC_FLT_TRAIN,146+m_boardRect.left,cy-hlist-8);
	CHANGEPOS(IDC_FLT_OTHERS,222+m_boardRect.left,cy-hlist-8);
	CHANGEPOS(IDC_FLT_SUSPECT,280+m_boardRect.left,cy-hlist-8);
	CHANGEPOS(IDC_FLT_HACK,380+m_boardRect.left,cy-hlist-8);
	CHANGEPOS(IDC_FLT_CHAT,480+m_boardRect.left,cy-hlist-8);
	CHANGEPOSSIZE(IDC_SUSPECT_INFO,8+m_boardRect.left,cy-22,wlist-70,12);
	CHANGEPOS(IDC_NEXT_SUSPECT,m_boardRect.left+wlist-58,cy-24);

	// player list
	if(::IsWindow(m_plStats)) 
		m_plStats.SetWindowPos(0,m_boardRect.left+wlist+8,cy-hlist-8,wlist2,hlist-20,SWP_NOZORDER);

	// little help text
	if(::IsWindow(m_dlbclick)) 
		m_dlbclick.SetWindowPos(0,m_boardRect.left+wlist+8,cy-20,0,0,SWP_NOZORDER|SWP_NOSIZE);

	// buttons
	CHANGEPOS(IDC_DHELP,m_boardRect.right-36,cy-22);
	CHANGEPOSSIZE(IDC_ANIMATE,m_boardRect.Width()-79,m_boardRect.bottom,49,17);
	CHANGEPOSSIZE(IDC_SEEMAP,m_boardRect.Width()-148,m_boardRect.bottom,69,17);
	CHANGEPOSSIZE(IDC_SPEEDPLUS,m_boardRect.Width()-30,m_boardRect.bottom,19,17);
	CHANGEPOSSIZE(IDC_SPEEDMINUS,m_boardRect.Width()-11,m_boardRect.bottom,19,17);
	CHANGEPOSSIZE(IDC_PAUSE,m_boardRect.Width()+8,m_boardRect.bottom,19,17);
	Invalidate(TRUE);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	if(cx!=0 && cy!=0) _Resize(cx, cy);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintOneEvent(CDC *pDC, const CRect* pRect, const ReplayEvt *evt)
{
	COLORREF m_color = evt->GetColor();

	COLORREF oldColor = pDC->GetBkColor();

	switch(m_shape)
	{
	case SQUARE:
		pDC->FillSolidRect(pRect,m_color);
		break;
	case RECTANGLE:
		pDC->Draw3dRect(pRect,m_color,m_color);
		break;
	case FLAG:
	case FLAGEMPTY:
		{
			CRect rect = *pRect;
			rect.bottom = rect.top+rect.Height()/2;
			rect.right = rect.left + (3*rect.Width())/4;
			if(m_shape==FLAG)
				pDC->FillSolidRect(&rect,m_color);
			else
				pDC->Draw3dRect(&rect,m_color,m_color);
			rect = *pRect;
			rect.right = rect.left+1;
			pDC->FillSolidRect(&rect,m_color);
		}
		break;
	case TEALEFT:
	case TEARIGHT:
		{
		CRect rect = *pRect;
		if(m_shape==TEALEFT) rect.right = rect.left+1;
		rect.left = rect.right-1;
		pDC->FillSolidRect(&rect,m_color);
		rect.right = pRect->right;
		rect.left = pRect->left;
		rect.top=pRect->top+pRect->Height()/2;
		rect.bottom = rect.top+1;
		pDC->FillSolidRect(&rect,m_color);
		}
		break;
	case TEATOP:
	case TEABOTTOM:
		{
		CRect rect = *pRect;
		if(m_shape==TEATOP) rect.bottom = rect.top+1;
		rect.top = rect.bottom-1;
		pDC->FillSolidRect(&rect,m_color);
		rect.top = pRect->top;
		rect.bottom = pRect->bottom;
		rect.left=pRect->left+pRect->Width()/2;
		rect.right = rect.left+1;
		pDC->FillSolidRect(&rect,m_color);
		}
		break;
	case ROUNDRECT:
		CPen pen(PS_SOLID,1,m_color);
		CBrush brush(m_color);
		CPen *oldPen = pDC->SelectObject(&pen);
		CBrush *oldBrush = pDC->SelectObject(&brush);
		pDC->RoundRect(pRect,CPoint(4,4));
		pDC->SelectObject(oldBrush);
		pDC->SelectObject(oldPen);
		break;
	}
	pDC->SetBkColor(oldColor);
}

//-----------------------------------------------------------------------------------------------------------------

DrawingTools* DlgStats::_GetDrawingTools(int player)
{
	if(player>=0) return &m_dtools[player+1];
	return &m_dtools[0];
}

//-----------------------------------------------------------------------------------------------------------------

// init all pens for drawing all the charts
void DlgStats::_InitAllDrawingTools()
{		
	// init drawing tools for multiple frame mode
	_InitDrawingTools(-1,m_maxPlayerOnBoard,1);

	// init drawing tools for all players on one frame mode
	for(int i=0;i<m_maxPlayerOnBoard;i++)
		_InitDrawingTools(i,m_maxPlayerOnBoard,2);
}

//-----------------------------------------------------------------------------------------------------------------

// init all pens for drawing the charts of one player
void DlgStats::_InitDrawingTools(int player, int maxplayer, int /*lineWidth*/)
{		
	// clear tools
	DrawingTools *tools = _GetDrawingTools(player);
	tools->Clear();

	// create pen for minerals
	for(int i=0; i<DrawingTools::maxcurve;i++) 
	{
		int lineSize = ReplayResource::GetLineSize(i);
		tools->m_clr[i] = ReplayResource::GetColor(i,player,maxplayer);
		tools->m_penMineral[i] = new CPen(PS_SOLID,lineSize,tools->m_clr[i]);
		tools->m_penMineralS[i] = new CPen(PS_SOLID,1,tools->m_clr[i]);
		tools->m_penMineralDark[i] = new CPen(PS_SOLID,1,CHsvRgb::Darker(tools->m_clr[i],0.70));
	}
}

//-----------------------------------------------------------------------------------------------------------------

// draw a little text to show a hot point
void DlgStats::_PaintHotPoint(CDC *pDC, int x, int y, const ReplayEvt *evt, COLORREF clr)
{
	const char *hotpoint=0;
	const IStarcraftAction *action = evt->GetAction();
	int off=8;

	//if event is a hack
	if(evt->IsHack())
	{
		// make color lighter
		COLORREF clrt = RGB(255,0,0);

		// draw a little triangle there
		int off=3;
		CPen clr(PS_SOLID,1,clrt);
		CPen clr2(PS_SOLID,1,RGB(160,0,0));
		CPen *old=(CPen*)pDC->SelectObject(&clr);
		for(int k=0;k<3;k++)
		{
			pDC->MoveTo(x-k,y-off-k);
			pDC->LineTo(x+k+1,y-off-k);
		}
		pDC->SelectObject(&clr2);
		for(int k=3;k<5;k++)
		{
			pDC->MoveTo(x-k,y-off-k);
			pDC->LineTo(x+k+1,y-off-k);
		}
		pDC->SelectObject(old);
	}

	// Expand?
	if(action->GetID()==BWrepGameData::CMD_BUILD)
	{
		int buildID = evt->UnitIdx();
		if(buildID == BWrepGameData::OBJ_COMMANDCENTER || buildID == BWrepGameData::OBJ_HATCHERY || buildID == BWrepGameData::OBJ_NEXUS)
			hotpoint = strExpand;
		if(buildID == BWrepGameData::OBJ_COMMANDCENTER)
		{
			// make sure it's not just a "land"
			const BWrepActionBuild::Params *p = (const BWrepActionBuild::Params *)action->GetParamStruct();
			if(p->m_buildingtype!=BWrepGameData::BTYP_BUILD) hotpoint=0;
		}
	}
	// Leave game?
	else if(action->GetID()==BWrepGameData::CMD_LEAVEGAME)
	{
		hotpoint = strLeave;
	}
	// Drop?
	else if(action->GetID()==BWrepGameData::CMD_UNLOAD || action->GetID()==BWrepGameData::CMD_UNLOADALL ||
		(action->GetID()==BWrepGameData::CMD_ATTACK && evt->Type().m_subcmd==BWrepGameData::ATT_UNLOAD))
	{
		hotpoint = strDrop;
	}
 	// Minimap ping?
	else if(action->GetID()==BWrepGameData::CMD_MINIMAPPING)
	{
		hotpoint = strMinimapPing;
	}

	// no hot point?
	if(hotpoint==0) return;

	// previous hotpoint too close?
	if((x-lastHotPointX)<32 && lastHotPoint==hotpoint) return;

	// make color lighter
	clr = CHsvRgb::Darker(clr,1.5);

	// get  text size
	int w = pDC->GetTextExtent(hotpoint).cx;
	CRect rectTxt(x-w/2-off,y-off,x+w/2-off,y-10-off);

	// draw text
	pDC->SetTextColor(clr);
	int omode = pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(hotpoint,&rectTxt,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
	pDC->SetBkMode(omode);

	// draw joining line
	pDC->MoveTo(x-off,y-off);
	pDC->LineTo(x,y);
	lastHotPointX = x;
	lastHotPoint = hotpoint;
}

//-----------------------------------------------------------------------------------------------------------------

// draw a local maximum
void DlgStats::_DrawLocalMax(CDC *pDC, int rx, int ry, const COLORREF clr, int& lastMaxX, int maxval)
{
	// if it's not too close to previous Max
	if(rx>lastMaxX+32)
	{
		// draw a little triangle there
		lastMaxX = rx;
		for(int k=0;k<5;k++)
		{
			pDC->MoveTo(rx-k,ry-3-k);
			pDC->LineTo(rx+k+1,ry-3-k);
		}

		// draw max value
		CRect rectTxt(rx-16,ry-20,rx+16,ry-10);
		CString strMax;
		strMax.Format("%d",maxval);
		CFont *oldFont=pDC->SelectObject(m_pSmallFont);
		COLORREF oldClr=pDC->SetTextColor(clr);
		int omode = pDC->SetBkMode(TRANSPARENT);
		pDC->DrawText(strMax,rectTxt,DT_CENTER|DT_SINGLELINE);
		pDC->SetBkMode(omode);
		pDC->SetTextColor(oldClr);
		pDC->SelectObject(oldFont);
	}
}

//-----------------------------------------------------------------------------------------------------------------

static const double BWPI = 3.1415926535897932384626433832795;

static double _CosineInterpolate(double y1,double y2,double mu)
{
	double mu2 = (1-cos(mu*BWPI))/2;
	return(y1*(1-mu2)+y2*mu2);
}

//-----------------------------------------------------------------------------------------------------------------

class CKnotArray
{
public:
	//ctor
	CKnotArray(int knotcount) : m_knotcount(knotcount) 
	{
		m_knots = new CPoint*[ReplayResource::__CLR_MAX];
		for(int i=0;i<ReplayResource::__CLR_MAX;i++)
			m_knots[i] = new CPoint[knotcount]; 
		memset(m_isUsed,0,sizeof(m_isUsed));
	}

	//dtor
	~CKnotArray() 
	{
		for(int i=0;i<ReplayResource::__CLR_MAX;i++)
			delete[]m_knots[i];
		delete[]m_knots;
	}

	// access
	CPoint *GetKnots(int curveidx) const {return m_knots[curveidx];}
	int GetKnotCount() const {return m_knotcount;}
	bool IsUsed(int i) const {return m_isUsed[i];}
	void SetIsUsed(int i) {m_isUsed[i]=true;}
private:
	CPoint **m_knots;
	int m_knotcount;
	bool m_isUsed[ReplayResource::__CLR_MAX];
};

//-----------------------------------------------------------------------------------------------------------------

// draw resource lines for one resource slot
void DlgStats::_PaintResources2(CDC *pDC, int ybottom, int cx, DrawingTools *tools, ReplayResource *res, unsigned long time, CKnotArray& knot, int knotidx)
{
	static int rx[DrawingTools::maxcurve],ry[DrawingTools::maxcurve];
	static int orx[DrawingTools::maxcurve],ory[DrawingTools::maxcurve];

	// save all current points
	for(int j=0; j<ReplayResource::MaxValue(); j++)
	{
		orx[j] = rx[j];
		ory[j] = ry[j];
	}

	// for each resource type
	for(int j=0; j<ReplayResource::MaxValue(); j++)
	{
		if(j==0 && (!m_seeMinerals || m_chartType!=RESOURCES)) continue;
		if(j==1 && (!m_seeGaz || m_chartType!=RESOURCES)) continue;
		if(j==2 && (!m_seeSupply || m_chartType!=RESOURCES)) continue;
		if(j==3 && (!m_seeUnits[m_chartType] || m_chartType!=RESOURCES)) continue;
 		if(j==4 && (m_chartType!=APM)) continue;
		if(j==5 && (!m_seeBPM || m_chartType!=APM)) continue;
		if(j==6 && (!m_seeUPM || m_chartType!=APM)) continue;
		if(j==7 && (m_chartType!=APM)) continue; // micro
		if(j==8) continue;
		if(j==9 && m_chartType!=MAPCOVERAGE) continue;
		if(j==10 && (!m_seeUnits[m_chartType] || m_chartType!=MAPCOVERAGE)) continue;
		 
		// we had a previous point, move beginning of line on it
		CPoint firstpt(rx[j],ry[j]);
		if(!firstPoint)	pDC->MoveTo(rx[j],ry[j]);

		// compute position
		rx[j] = cx;
		ry[j] = ybottom - (int)((float)(res->Value(j))*m_fvinc[j]);

		// use splines?
		bool nodraw=false;
		if(j<4 || j==9 || j==10 || (j==4 && !m_seeSpeed)) {knot.GetKnots(j)[knotidx]=CPoint(cx,ry[j]); knot.SetIsUsed(j); nodraw=true;}

		// if upm or bpm 
		if(j==5 || j==6)
		{
			// paint upm / bpm bar
			int w = max(4,(int)m_finc);
			pDC->FillSolidRect(cx-w/2,ry[j],w,ybottom-ry[j],tools->m_clr[j]);
		}
		else
		{
			// paint segment for resource line
			if(!firstPoint)	pDC->SelectObject(tools->m_penMineral[j]);
			if(!nodraw)
			{
				if(!firstPoint && j!=7)	
				{
					// if segment is long enough, and not a straight line
					if(cx-firstpt.x>8 && firstpt.y!=ry[j])
					{
						// interpolate it
						for(int x=firstpt.x;x<=cx;x++)
						{
							double mu = (double)(x-firstpt.x)/(double)(cx-firstpt.x);
							double y = _CosineInterpolate(firstpt.y,ry[j],mu);
							pDC->LineTo(x,(int)y);
						}
					}
					else
						pDC->LineTo(rx[j],ry[j]);
				}
			}

			// is it a max for APM?
			if(j==4 && OPTIONSCHART->m_maxapm && res->Value(j) == m_list->ResourceMax().LegalAPM() && time>=MINAPMVALIDTIMEFORMAX)
				_DrawLocalMax(pDC, rx[j], ry[j], tools->m_clr[j], lastMaxX, m_list->ResourceMax().LegalAPM());
			
			// is it a max for MAP coverage?
			if(j==10 && OPTIONSCHART->m_maxapm && res->Value(j) == m_list->ResourceMax().MovingMapCoverage())
				_DrawLocalMax(pDC, rx[j], ry[j], tools->m_clr[j], lastMaxX, m_list->ResourceMax().MovingMapCoverage());
		}
	}

	// compute position
	rx[4] = cx;
	ry[4] = ybottom - (int)((float)(res->Value(4))*m_fvinc[4]);

	// if macro is on
	if(m_chartType==APM && m_seeSpeed && !firstPoint)
	{
		// paint surface
		POINT pt[4];
		pt[0].x = orx[4]; pt[1].x = rx[4]; pt[2].x = rx[7]; pt[3].x = orx[7];
		pt[0].y = ory[4]; pt[1].y = ry[4]; pt[2].y = ry[7]; pt[3].y = ory[7];
		CBrush bkgpoly(tools->m_clr[4]);
		CBrush  *oldb = pDC->SelectObject(&bkgpoly);
		pDC->SelectObject(tools->m_penMineral[4]);
		pDC->Polygon(pt, 4);
		pDC->SelectObject(oldb);

		// draw up and bottom lines in darker color for antialiasing
		//CPen penred(PS_SOLID,2,RGB(235,0,0));
		//CPen pengreen(PS_SOLID,2,RGB(0,0,235));
		pDC->SelectObject(tools->m_penMineralDark[4]);
		//pDC->SelectObject(&penred);
		pDC->MoveTo(pt[0].x,pt[0].y);
		pDC->LineTo(pt[1].x,pt[1].y);
		//pDC->SelectObject(&pengreen);
		pDC->MoveTo(pt[2].x,pt[2].y);
		pDC->LineTo(pt[3].x,pt[3].y);
	}


	firstPoint=false;
}

//-----------------------------------------------------------------------------------------------------------------

// paint a spline curve
void DlgStats::_PaintSpline(CDC *pDC, const CKnotArray& knots, int curveidx)
{
	// compute control points
	CPoint* firstControlPoints=0;
	CPoint* secondControlPoints=0;
	BezierSpline::GetCurveControlPoints(knots.GetKnots(curveidx), knots.GetKnotCount(), firstControlPoints, secondControlPoints);

	// allocate point array for call to PolyBezier
	int ptcount = 3*(knots.GetKnotCount()-1)+1;
	POINT* pts = new POINT[ptcount];
	for(int i=0;i<knots.GetKnotCount()-1;i++)
	{	
		int step = i*3;
		pts[step]=knots.GetKnots(curveidx)[i];
		pts[step+1]=firstControlPoints[i];
		pts[step+2]=secondControlPoints[i];
	}
	pts[ptcount-1]=knots.GetKnots(curveidx)[knots.GetKnotCount()-1];
	
	// draw curve
	pDC->PolyBezier(pts,ptcount);

	// delete arrays
	delete[]pts;
	delete[]firstControlPoints;
	delete[]secondControlPoints;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintEvents(CDC *pDC, int chartType, const CRect& rect, const ReplayResource& resmax, int player)
{
	// any event to draw?
	if(m_list->GetEventCount()==0) return;

	// compute all ratios depending on the max of every resource
	float rheight = (float)(rect.Height()-vtop-vbottom);
  	m_fvinc[0] = m_fvinc[1] = rheight/(float)(max(resmax.Minerals(),resmax.Gaz()));
  	m_fvinc[2] = m_fvinc[3] = rheight/(float)(max(resmax.Supply(),resmax.Units()));
  	m_fvinc[4] = rheight/(float)resmax.APM();
  	m_fvinc[5] = rheight/((float)resmax.BPM()*4.0f);
  	m_fvinc[6] = rheight/((float)resmax.UPM()*2.0f);
	m_fvinc[7] = rheight/(float)resmax.APM();
	m_fvinc[8] = rheight/(float)(resmax.MacroAPM()*4);
	m_fvinc[9] = m_fvinc[10] = rheight/(float)(max(resmax.MapCoverage(),resmax.MovingMapCoverage()));
	m_finc = (float)(rect.Width()-hleft-hright)/(float)(m_timeEnd - m_timeBegin);

	// get drawing tools
	DrawingTools *tools = _GetDrawingTools(player);
	CPen *oldPen = (CPen*)pDC->SelectObject(tools->m_penMineral[0]);
	int oldMode = pDC->GetBkMode();
	COLORREF oldclr = pDC->GetBkColor();
 	CFont *oldfont = pDC->SelectObject(m_pSmallFont);

	// where do we start painting?
	unsigned long timeBegin = m_timeBegin; 
	//unsigned long timeBegin = m_bIsAnimating ? m_timeCursor : m_timeBegin; 
	//if(m_timeCursor<500) timeBegin = 0; else timeBegin-=500;

	// reset last max data
	lastMaxX = 0;
	lastHotPointX = 0;
	lastHotPoint = 0;

	// for all resource slots
	firstPoint=true;
	int slotBegin =  m_list->Time2Slot(m_timeBegin);
	int slotEnd =  m_list->Time2Slot(m_timeEnd); // m_list->GetSlotCount()
	int slotCount = slotEnd-slotBegin;
	int divider = max(1,slotCount/gSplineCount[chartType]);
	int knotcount = slotCount==0 ? 0 : (slotCount/divider);
	CKnotArray *knots = knotcount==0 ? 0 : new CKnotArray(knotcount);
	for(int slot = slotBegin; slot<slotEnd; slot++)
	{
		// get resource object
		ReplayResource *res = m_list->GetResourceFromIdx(slot);
		unsigned long tick = m_list->Slot2Time(slot);

		// compute event position on X
		float fx=(float)(rect.left+hleft) + m_finc * (tick-m_timeBegin);
		int cx = (int)fx;

		// draw resource lines
		int knotslot = min(knotcount-1,(slot-slotBegin)/divider);
		_PaintResources2(pDC, rect.bottom - vbottom, cx, tools, res, tick, *knots, knotslot);
	}

	if(knots!=0)
	{
		// paint splines
		for(int i=0;i<ReplayResource::MaxValue();i++)
			if(knots->IsUsed(i))
			{
				pDC->SelectObject(tools->m_penMineral[i]);
				_PaintSpline(pDC,*knots,i);
			}
		delete knots;
	}

	if(m_chartType==RESOURCES && (m_seeActions || (m_seeMinerals && m_seeHotPoints)))
	{
		// for each event in the current window, paint hot points
		int layer=0;
		unsigned long eventMax = (unsigned long)m_list->GetEventCount();
		CRect rectEvt;
		firstPoint=true;
		lastMaxX = 0;
		lastHotPointX = 0;
		lastHotPoint = 0;
		for(unsigned long idx = m_list->GetEventFromTime(timeBegin);;idx++)
		{
			// get event description
			if(idx>=eventMax) break;
			const ReplayEvt *evt = m_list->GetEvent(idx);
			if(evt->Time()>m_timeEnd) break;

			// during animation, we only paint until cursor
			if(m_bIsAnimating && evt->Time()>m_timeCursor) break;

			// compute event position on X
			float fx=(float)(rect.left+hleft) + m_finc * (evt->Time()-m_timeBegin);
			int cx = (int)fx;

			// if we paint actions
			if(m_seeActions) 
			{
				// compute event rect
				rectEvt.left=cx-evtWidth/2;
				rectEvt.right=cx+evtWidth/2;
				layer = m_replay.GetTypeIdx(evt);
				rectEvt.bottom=rect.bottom-vbottom-layer*layerHeight-(layerHeight-evtHeight)/2;
				rectEvt.top=rectEvt.bottom-evtHeight-(layerHeight-evtHeight)/2+1;
				if(rectEvt.top>rect.top) _PaintOneEvent(pDC,&rectEvt,evt);
			}

			// draw hot point (if any)
			firstPoint=false;
			if(m_seeMinerals && m_seeHotPoints && !evt->IsDiscarded()) 
			{
				int y = rect.bottom-vbottom - (int)((float)(evt->Resources().Value(0))*m_fvinc[0]);
				pDC->SelectObject(tools->m_penMineralS[0]);
				_PaintHotPoint(pDC, cx, y, evt, tools->m_clr[0]);
			}
		}
	}
	else if(m_chartType==APM)
	{
		// for each event in the current window, paint hot points
		unsigned long eventMax = (unsigned long)m_list->GetEventCount();
		CRect rectEvt;
		for(unsigned long idx = m_list->GetEventFromTime(timeBegin);;idx++)
		{
			// get event description
			if(idx>=eventMax) break;
			const ReplayEvt *evt = m_list->GetEvent(idx);
			if(evt->Time()>m_timeEnd) break;
			if(evt->ActionID()!=BWrepGameData::CMD_MESSAGE) continue;

			// during animation, we only paint until cursor
			if(m_bIsAnimating && evt->Time()>m_timeCursor) break;

			// compute event position on X
			float fx=(float)(rect.left+hleft) + m_finc * (evt->Time()-m_timeBegin);
			int cx = (int)fx;

			// if we paint actions
			if(m_seeHotPoints) 
			{
				// compute event rect
				rectEvt.left=cx-evtWidth/2;
				rectEvt.right=cx+evtWidth/2;
				int layer = 0;
				rectEvt.bottom=rect.bottom-vbottom-layer*layerHeight-(layerHeight-evtHeight)/2;
				rectEvt.top=rectEvt.bottom-evtHeight-(layerHeight-evtHeight)/2+1;
				if(rectEvt.top>rect.top) _PaintOneEvent(pDC,&rectEvt,evt);
			}
		}
	}
	else if(m_chartType==MAPCOVERAGE)
	{
	}

	// restore every gdi stuff
	pDC->SelectObject(oldPen);
	pDC->SetBkMode(oldMode);
	pDC->SetBkColor(oldclr);
	pDC->SelectObject(oldfont);
}

//-----------------------------------------------------------------------------------------------------------------

// compute vertical increment for grid and axis
float DlgStats::_ComputeGridInc(unsigned long& tminc, unsigned long& maxres, const CRect& rect, const ReplayResource& resmax, int chartType, bool useGivenMax) const
{
	if(!useGivenMax)
	{
		// compute max value
		switch(chartType)
		{
			case APM :
				maxres = resmax.APM(); 
				break;
			case MAPCOVERAGE :
				maxres =  max(resmax.MapCoverage(),resmax.MovingMapCoverage()); 
				break;
			default: 
				maxres = max(resmax.Minerals(),resmax.Gaz());
		}
	}

	// compute vertical pixel per value
	float fvinc = (float)(rect.Height()-vbottom-vtop)/(float)(maxres);

	// compute grid increment
	tminc=((maxres/10)/1000)*1000;
	if(tminc==0) tminc=tminc=((maxres/10)/100)*100;
	if(tminc==0) tminc=((maxres/10)/10)*10;
	if(tminc==0) tminc=5;
	if(maxres<15) tminc=1;

	return fvinc;
}

void DlgStats::_PaintGrid(CDC *pDC, const CRect& rect, const ReplayResource& resmax)
{
	if(!OPTIONSCHART->m_bggrid) return;

	// create pen for axis
	CPen *penGrid = new CPen(PS_DOT,1,clrGrid);
	CPen *oldPen = pDC->SelectObject(penGrid);
	int oldmode = pDC->SetBkMode(TRANSPARENT);

	// draw horizontal lines of grid
	unsigned long maxres = 0;
	unsigned long tminc;
	float fvinc = _ComputeGridInc(tminc, maxres, rect, resmax,m_chartType);
	for(unsigned long tr=tminc; tr<(unsigned long)maxres; tr+=tminc)
	{
		// draw grid
		float fy=(float)(rect.bottom-vbottom) - fvinc * tr;
		pDC->MoveTo(rect.left+hleft+1,(int)fy);
		pDC->LineTo(rect.right-hright,(int)fy);
	} 

	// restore every gdi stuff
	pDC->SetBkMode(oldmode);
	pDC->SelectObject(oldPen);
	delete penGrid;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintActionsName(CDC *pDC, const CRect& rect)
{
	if(!m_seeActions || m_chartType!=RESOURCES) return;

	// draw layers
	CString strNum;
	CRect rectTxt;
	CFont *oldFont = pDC->SelectObject(m_pLayerFont);

	// compute first layer rect
	CRect rectLayer=rect;
	rectLayer.top = rectLayer.bottom-layerHeight;
	rectLayer.OffsetRect(hleft,-vbottom);
	rectLayer.right-=hleft+hright;
	COLORREF bkc = pDC->GetBkColor();

	// for each layer
	for(int k=0; k<m_replay.GetTypeCount();k++)
	{
		// fill layer rect
		COLORREF clrLayer = (k%2)==0 ? clrLayer1 : clrLayer2;
		pDC->FillSolidRect(&rectLayer,clrLayer);

		// draw layer action name
		clrLayer = (k%2)==0 ? clrLayerTxt1 : clrLayerTxt2;
		rectTxt = rectLayer;
		rectTxt.left+=28;
		rectTxt.right = rectTxt.left+128;
		pDC->SetTextColor(clrLayer);
		pDC->DrawText(m_replay.GetTypeStr(k),rectTxt,DT_LEFT|DT_SINGLELINE);

		//next layer
		rectLayer.OffsetRect(0,-layerHeight);
		if(rectLayer.bottom-layerHeight < rect.top) break;
	}
	// restore bkcolor
	pDC->SetBkColor(bkc);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintAxis(CDC *pDC, const CRect& rect, const ReplayResource& resmax, int mask, 
						  unsigned long timeBegin, unsigned long timeEnd)
{
	// create pen for axis
	CPen *penTime = new CPen(PS_SOLID,1,clrTime);
	CPen *penAction = new CPen(PS_SOLID,1,clrAction);
 	CPen *oldPen = (CPen*)pDC->SelectObject(penTime);
	int oldmode = pDC->SetBkMode(TRANSPARENT);

	// draw X axis values
	CString strNum;
	CRect rectTxt;
	float finc = timeEnd>timeBegin ? (float)(rect.Width()-hleft-hright)/(float)(timeEnd - timeBegin) : 0.0f;
	unsigned long tminc = 10;
	unsigned long maxres;
	while(timeEnd>timeBegin && (int)(tminc*finc)<50) tminc*=2;
	COLORREF oldClr = pDC->SetTextColor(clrTime);
	CFont *oldFont=pDC->SelectObject(m_pSmallFont);
	if(mask&MSK_XLINE && timeEnd>timeBegin)
	{
		for(unsigned long tm=timeBegin; tm<timeEnd; tm+=tminc)
		{
			float fx=(float)(rect.left+hleft) + finc * (tm-timeBegin);
			strNum = _MkTime(m_replay.QueryFile()->QueryHeader(),tm, m_useSeconds?true:false);
			int cx = (int)fx;
			// draw time
			rectTxt.SetRect(cx,rect.bottom-vbottom+2,cx+128,rect.bottom);
			pDC->DrawText(strNum,rectTxt,DT_LEFT|DT_SINGLELINE);
			// draw line
			pDC->MoveTo(cx,rect.bottom-vbottom-1);
			pDC->LineTo(cx,rect.bottom-vbottom+2);
		}
	}

	if((m_seeMinerals || m_seeGaz) && m_chartType==RESOURCES)
	{
		// draw left Y axis values (minerals and gas)
		int xpos = rect.left+hleft;
		float fvinc = _ComputeGridInc(tminc, maxres, rect, resmax, m_chartType);
		if(mask&MSK_YLEFTLINE)
		{
			for(unsigned long tr=tminc; tr<(unsigned long)maxres; tr+=tminc)
			{
				float fy=(float)(rect.bottom-vbottom) - fvinc * tr;
				strNum.Format("%lu",tr);
				int cy = (int)fy;
				// draw line
				pDC->MoveTo(xpos-1,cy);
				pDC->LineTo(xpos+2,cy);
				// draw time
				rectTxt.SetRect(rect.left+haxisleft,cy-8,rect.left+hleft-3,cy+8);
				pDC->DrawText(strNum,rectTxt,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
			} 

			// draw left Y axis title
			rectTxt.SetRect(rect.left+haxisleft,rect.top,rect.left+hleft+16,rect.top+16);
			pDC->DrawText(strMinGas,rectTxt,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
		}
	}

	if((m_seeSupply || m_seeUnits[m_chartType]) && m_chartType==RESOURCES)
	{
		// draw right Y axis values (supply & units)
		if(mask&MSK_YRIGHTLINE)
		{
			maxres = resmax.Supply();
			float fvinc = _ComputeGridInc(tminc, maxres, rect, resmax, m_chartType,true);
			for(unsigned long tr=tminc; tr<(unsigned long)maxres; tr+=tminc)
			{
				float fy=(float)(rect.bottom-vbottom) - fvinc * tr;
				strNum.Format("%lu",tr);
				int cy = (int)fy;
				// draw line
				pDC->MoveTo(rect.right-hright-1,cy);
				pDC->LineTo(rect.right-hright+2,cy);
				// draw time
				rectTxt.SetRect(rect.right-hright-4-64,cy-8,rect.right-hright-4,cy+8);
				pDC->DrawText(strNum,rectTxt,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
			} 

			// draw right Y axis title
			rectTxt.SetRect(rect.right-hright-64,rect.top,rect.right-hright-4,rect.top+16);
			pDC->DrawText(strSupplyUnit,rectTxt,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
		}
	}
	
	if(m_chartType==APM)
	{
		pDC->SelectObject(penAction);
		pDC->SetTextColor(clrAction);

		// draw actions/minute Y axis values
		int xpos = rect.left+hleft;
		float fvinc = _ComputeGridInc(tminc, maxres, rect, resmax, m_chartType);
		for(unsigned long tr=tminc; tr<(unsigned long)maxres; tr+=tminc)
		{
			float fy=(float)(rect.bottom-vbottom) - fvinc * tr;
			strNum.Format("%lu",tr);
			int cy = (int)fy;
			// draw line
			pDC->MoveTo(xpos-1,cy);
			pDC->LineTo(xpos+2,cy);
			// draw time
			rectTxt.SetRect(rect.left+haxisleft,cy-8,rect.left+hleft-3,cy+8);
			pDC->DrawText(strNum,rectTxt,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
		} 

		// draw actions/minute Y axis title
		rectTxt.SetRect(rect.left+haxisleft,rect.top,rect.left+hleft-3,rect.top+16);
		pDC->DrawText("APM",rectTxt,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
	
		// draw right Y axis values (supply & units)
		if(mask&MSK_YRIGHTLINE)
		{
			fvinc = _ComputeGridInc(tminc, maxres, rect, resmax, m_chartType);
			for(unsigned long tr=tminc; tr<(unsigned long)maxres; tr+=tminc)
			{
				float fy=(float)(rect.bottom-vbottom) - fvinc * tr;
				strNum.Format("%lu",tr);
				int cy = (int)fy;
				// draw line
				pDC->MoveTo(rect.right-hright-1,cy);
				pDC->LineTo(rect.right-hright+2,cy);
				// draw time
				rectTxt.SetRect(rect.right-hright-4-64,cy-8,rect.right-hright-4,cy+8);
				pDC->DrawText(strNum,rectTxt,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
			} 

			// draw right Y axis title
			rectTxt.SetRect(rect.right-hright-64,rect.top,rect.right-hright-4,rect.top+16);
			pDC->DrawText("APM",rectTxt,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
		}
	}
	else if(m_chartType==MAPCOVERAGE)
	{
		pDC->SelectObject(penAction);
		pDC->SetTextColor(clrAction);

		// draw coverage Y axis values
		int xpos = rect.left+hleft;
		float fvinc = _ComputeGridInc(tminc, maxres, rect, resmax, m_chartType);
		for(unsigned long tr=tminc; tr<(unsigned long)maxres; tr+=tminc)
		{
			float fy=(float)(rect.bottom-vbottom) - fvinc * tr;
			strNum.Format("%lu",tr);
			int cy = (int)fy;
			// draw line
			pDC->MoveTo(xpos-1,cy);
			pDC->LineTo(xpos+2,cy);
			// draw time
			rectTxt.SetRect(rect.left+haxisleft,cy-8,rect.left+hleft-3,cy+8);
			pDC->DrawText(strNum,rectTxt,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
		} 

		// draw building map coverage Y axis title
		rectTxt.SetRect(rect.left+haxisleft,rect.top,rect.left+hleft-3,rect.top+16);
		pDC->DrawText("%Map",rectTxt,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);

		if(m_seeUnits[m_chartType])
		{
			// draw right Y axis values (unit map coverage)
			if(mask&MSK_YRIGHTLINE)
			{
				maxres = resmax.MovingMapCoverage();
				float fvinc = _ComputeGridInc(tminc, maxres, rect, resmax, m_chartType,true);
				for(unsigned long tr=tminc; tr<(unsigned long)maxres; tr+=tminc)
				{
					float fy=(float)(rect.bottom-vbottom) - fvinc * tr;
					strNum.Format("%lu",tr);
					int cy = (int)fy;
					// draw line
					pDC->MoveTo(rect.right-hright-1,cy);
					pDC->LineTo(rect.right-hright+2,cy);
					// draw time
					rectTxt.SetRect(rect.right-hright-4-64,cy-8,rect.right-hright-4,cy+8);
					pDC->DrawText(strNum,rectTxt,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
				} 

				// draw right Y axis title
				rectTxt.SetRect(rect.right-hright-64,rect.top,rect.right-hright-4,rect.top+16);
				pDC->DrawText("%Map",rectTxt,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
			}
		}
	}

	// restore every gdi stuff
	pDC->SetBkMode(oldmode);
	pDC->SelectObject(oldPen);
	pDC->SetTextColor(oldClr);
	pDC->SelectObject(oldFont);
	delete penTime;
	delete penAction;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintAxisLines(CDC *pDC, const CRect& rect, int mask)
{
	// create pen for axis
	CPen *penTime = new CPen(PS_SOLID,1,clrTime);
 	CPen *oldPen = (CPen*)pDC->SelectObject(penTime);

	// draw X axis time line
	if(mask&MSK_XLINE)
	{
		pDC->MoveTo(rect.left+hleft,rect.bottom-vbottom);
		pDC->LineTo(rect.right-hright,rect.bottom-vbottom);
	}

	// draw left Y axis time line
	if(mask&MSK_YLEFTLINE)
	{
		pDC->MoveTo(rect.left+hleft,rect.bottom-vbottom);
		pDC->LineTo(rect.left+hleft,rect.top+vtop);
	}

	// draw right Y axis time line
	if(mask&MSK_YRIGHTLINE)
	{
		pDC->MoveTo(rect.right-hright,rect.bottom-vbottom);
		pDC->LineTo(rect.right-hright,rect.top+vtop);
	}

	// restore every gdi stuff
	pDC->SelectObject(oldPen);
	delete penTime;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintBackgroundLayer(CDC *pDC, const CRect& rect, const ReplayResource& resmax)
{
	// draw horizontal lines of grid
	_PaintGrid(pDC, rect, resmax);

	// draw layers
	_PaintActionsName(pDC, rect);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintForegroundLayer(CDC *pDC, const CRect& rect, const ReplayResource& resmax, int mask)
{
	// draw X & Yaxis time line
	_PaintAxisLines(pDC, rect, mask);

	// draw X & Y axis values
	_PaintAxis(pDC, rect, resmax,MSK_ALL,m_timeBegin,m_timeEnd);

	// draw cursor
	_PaintCursor(pDC, rect);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintMapName(CDC *pDC, CRect& rect)
{
	if(m_chartType!=RESOURCES && m_chartType!=APM) return;

	// select font & color
	CFont *oldFont = pDC->SelectObject(m_pLabelBoldFont);
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	// build title
	CString title(m_replay.MapName());
	if(m_replay.RWAHeader()!=0)
	{
		CString audioby;
		audioby.Format(IDS_AUDIOCOMMENT,m_replay.RWAHeader()->author);
		title+=audioby;
	}

	//get title size with current font
	CSize sz = pDC->GetTextExtent(title);

	// draw text
	CRect rectTxt;
	rectTxt.SetRect(rect.left+hplayer,rect.top,rect.left+hplayer+sz.cx,rect.top+28);
	pDC->SetTextColor(OPTIONSCHART->GetColor(DlgOptionsChart::CLR_MAP));
	pDC->DrawText(title,rectTxt,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
	m_rectMapName = rectTxt;

	// restore
	pDC->SetBkMode(oldMode);
	pDC->SelectObject(oldFont);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintSentinel(CDC *pDC, CRect& rectTxt)
{
	if(m_chartType!=RESOURCES && m_chartType!=APM) return;

	CString gamename = m_replay.QueryFile()->QueryHeader()->getGameName();
	if(gamename.Left(11)=="BWSentinel ")
	{
		// select font & color
		CFont *oldFont = pDC->SelectObject(m_pSmallFont);
		int oldMode = pDC->SetBkMode(TRANSPARENT);

		// draw text
		pDC->SetTextColor(clrPlayer);
		pDC->DrawText("(sentinel on)",rectTxt,DT_LEFT|DT_SINGLELINE|DT_VCENTER);

		// restore
		pDC->SetBkMode(oldMode);
		pDC->SelectObject(oldFont);
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintPlayerName(CDC *pDC, const CRect& rectpl, ReplayEvtList *list, int playerIdx, int playerPos, COLORREF clr, int offv)
{
	CRect rect(rectpl);
	rect.OffsetRect(4,0);

	// select font & color
	CFont *oldFont = pDC->SelectObject(m_pLabelBoldFont);
	COLORREF oldClr = pDC->SetTextColor(clr);
	COLORREF bkClr = pDC->GetBkColor();
  	int oldMode = pDC->GetBkMode();

	// draw player name
	CString pname;
	CRect rectTxt;
	pname.Format("%s (%s)",list->PlayerName(),list->GetRaceStr());
	rectTxt.SetRect(rect.left+hplayer,rect.top+offv,rect.left+hplayer+200,rect.top+offv+28);
	if(playerPos>=0) rectTxt.OffsetRect(0,playerPos*28);
	if(m_chartType!=RESOURCES && m_chartType!=APM && m_mixedCount==0) rectTxt.OffsetRect(-hplayer+hleft,0);
	pDC->SetTextColor(clr);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(pname,rectTxt,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
	CRect rectRatio=rectTxt;

	// text size
	int afterText = pDC->GetTextExtent(pname).cx;
	
	// draw colors of resource lines
	if(playerIdx>=0)
	{
		rectTxt.top+=22;
		rectTxt.right=rectTxt.left+12;
		rectTxt.bottom=rectTxt.top+3;
		for(int j=0; j<ReplayResource::MaxValue(); j++)
		{
			if(j==0 && (!m_seeMinerals || m_chartType!=RESOURCES)) continue;
			if(j==1 && (!m_seeGaz || m_chartType!=RESOURCES)) continue;
			if(j==2 && (!m_seeSupply || m_chartType!=RESOURCES)) continue;
			if(j==3 && (!m_seeUnits[m_chartType] || m_chartType!=RESOURCES)) continue;
 			if(j==4 && (!m_seeSpeed || m_chartType!=APM)) continue;
			if(j==5 && (!m_seeBPM || m_chartType!=APM)) continue;
			if(j==6 && (!m_seeUPM || m_chartType!=APM)) continue;
			if(j==7 || j==8) continue;
			pDC->FillSolidRect(&rectTxt,_GetDrawingTools(playerIdx)->m_clr[j]);
			rectTxt.OffsetRect(rectTxt.Width()+3,0);
		}
	}
	
	// add player info
	if(m_chartType>=UNITS && m_chartType<=UPGRADES)
	{
		// display distribution total
		pname.Format(IDS_TOTAL,list->GetDistTotal(m_chartType-UNITS));
	}
	else
	{
		// display APM stuff
		//if(m_maxPlayerOnBoard==2)
		//{
			// compute action ratio
			//ReplayEvtList *list1 = m_replay.GetEvtList(0);
			//ReplayEvtList *list2 = m_replay.GetEvtList(1);
			//float ratio = list==list1 ? (float)list1->GetEventCount()/(float)list2->GetEventCount() : (float)list2->GetEventCount()/(float)list1->GetEventCount();
			//pname.Format("[%.2f, %d, %d APM]",ratio,list->GetEventCount(),list->GetActionPerMinute());
		//}
		pname.Format("[%d actions APM=%d]",list->GetEventCount(),list->GetActionPerMinute());
	}
	rectRatio.OffsetRect(afterText+8,0);
	pDC->SelectObject(m_pLayerFont);
	pDC->SetTextColor(clrRatio);
	pDC->SetBkColor(RGB(0,0,0));
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(pname,rectRatio,DT_LEFT|DT_SINGLELINE|DT_VCENTER);

	// restore every gdi stuff
	pDC->SetBkMode(oldMode);
	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldClr);
	pDC->SetBkColor(bkClr);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintCursor(CDC *pDC, const CRect& rect)
{
	// compute position
	float finc = (float)(rect.Width()-hleft-hright)/(float)(m_timeEnd - m_timeBegin);
	float fxc=(float)(rect.left+hleft) + finc * (m_timeCursor-m_timeBegin);

	// prepare pen
	CPen *penCursor = new CPen(PS_DOT,1,clrCursor);
	CPen *oldPen = pDC->SelectObject(penCursor);
	int oldmode = pDC->SetBkMode(TRANSPARENT);

	// draw cursor
	pDC->MoveTo((int)fxc,rect.top);
	pDC->LineTo((int)fxc,rect.bottom);

	// restore every gdi stuff
	pDC->SetBkMode(oldmode);
	pDC->SelectObject(oldPen);
	delete penCursor;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintBO(CDC *pDC, const CRect& rect, const ReplayObjectSequence& bo, int offset)
{
	// draw building names
	int levelh = (rect.Height()-32-vplayer)/4;
	for(int i=0;i<bo.GetCount();i++)
	{
		float finc = (float)(rect.Width()-hleft-hright)/(float)(m_timeEnd - m_timeBegin);
		float fxc=(float)(rect.left+hleft) + finc * bo.GetTime(i);
		int y = rect.top+vplayer+16 + ((i+1)%4)*levelh + offset;
		pDC->MoveTo((int)fxc,y);
		pDC->LineTo((int)fxc,rect.bottom-vbottom);

		// text size
		CString pname = bo.GetName(i);
		int wText = pDC->GetTextExtent(pname).cx;

		CRect rectText;
		y-=16;
		rectText.SetRect((int)fxc-wText/2, y, (int)fxc+wText/2, y+16);
		rectText.InflateRect(1,1);
		pDC->DrawText(pname,rectText,DT_CENTER|DT_SINGLELINE|DT_VCENTER);
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintBuildOrder(CDC *pDC, const CRect& rect)
{
	// select font & color
	CFont *oldFont = pDC->SelectObject(m_pLayerFont);
	COLORREF oldClr = pDC->SetTextColor(clrBOName[0]);
	int oldMode = pDC->GetBkMode();
 	CPen *penCursor = new CPen(PS_SOLID,1,clrBOLine[0]);
	CPen *penCursor2 = new CPen(PS_SOLID,1,clrBOLine[1]);
	CPen *penCursor3 = new CPen(PS_SOLID,1,clrBOLine[2]);

	// draw X & Y axis values
	ReplayResource resmax;
	_PaintAxis(pDC, rect, resmax, MSK_XLINE,m_timeBegin,m_timeEnd);

	// draw building names
	CPen *oldPen = pDC->SelectObject(penCursor);
	pDC->SetBkMode(TRANSPARENT);
	_PaintBO(pDC, rect, m_list->GetBuildOrder(),0);

	// draw upgrades
	pDC->SetTextColor(clrBOName[2]);
	pDC->SelectObject(penCursor3);
	_PaintBO(pDC, rect, m_list->GetBuildOrderUpgrade(),10);

	// draw research
	pDC->SetTextColor(clrBOName[2]);
	pDC->SelectObject(penCursor3);
	_PaintBO(pDC, rect, m_list->GetBuildOrderResearch(),20);

	// draw units names
	if(m_seeUnitsOnBO)
	{
		pDC->SetTextColor(clrBOName[1]);
		pDC->SelectObject(penCursor2);
		_PaintBO(pDC, rect, m_list->GetBuildOrderUnits(),30);
	}

	// paint foreground layer (axis lines + cursor)
	_PaintForegroundLayer(pDC, rect, resmax, MSK_XLINE);

	// restore every gdi stuff
	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldClr);
	pDC->SetBkMode(oldMode);
	delete penCursor3;
	delete penCursor2;
	delete penCursor;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_ComputeHotkeySymbolRect(const ReplayEvtList *list, const HotKeyEvent *hkevt, CRect& datarect, CRect& symRect)
{
	// compute symbol position
	int levelhk = (datarect.Height()-8-vplayer)/10;
	float finc = (float)(datarect.Width()-hleft-hright)/(float)(m_timeEnd - m_timeBegin);
	float fxc=(float)(datarect.left+hleft) + finc * (hkevt->m_time - m_timeBegin);
	int y = datarect.top+vplayer + hkevt->m_slot*levelhk;
	int wText = 10;
	symRect.SetRect((int)fxc-wText/2, y, (int)fxc+wText/2, y+levelhk);
	symRect.InflateRect(2,2);
	symRect.DeflateRect(0,(levelhk-wText)/2);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintHotKeyEvent(CDC *pDC, CRect& datarect, int offset)
{
	char text[3];
	CRect symRect;

	// draw hotkey events
	for(int i=0;i<(int)m_list->GetHKEventCount();i++)
	{
		// get event
		const HotKeyEvent *hkevt = m_list->GetHKEvent(i);
		if(hkevt->m_time < m_timeBegin || hkevt->m_time > m_timeEnd) continue;

		// view hotkey selection?
		if(!m_viewHKselect && hkevt->m_type==HotKeyEvent::SELECT) continue;

		// compute symbol position
		_ComputeHotkeySymbolRect(m_list, hkevt, datarect, symRect);

		// text size
		sprintf(text,"%d", hkevt->m_slot==9?0:hkevt->m_slot+1);

		// draw text
		CRect rectText;
		pDC->SetTextColor(hkevt->m_type!=HotKeyEvent::ASSIGN ? RGB(100,0,100):RGB(0,0,0));
		if(hkevt->m_type==HotKeyEvent::ASSIGN)
			pDC->FillSolidRect(symRect,RGB(100,0,200));
		else if(hkevt->m_type==HotKeyEvent::ADD)
			pDC->Draw3dRect(symRect,RGB(100,0,200),RGB(100,0,200));
		pDC->DrawText(text,symRect,DT_CENTER|DT_SINGLELINE|DT_VCENTER);
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintHotKeys(CDC *pDC, const CRect& rectc)
{
	// select font & color
	CFont *oldFont = pDC->SelectObject(m_pLayerFont);
	int oldMode = pDC->GetBkMode();
	int oldbkClr = pDC->GetBkColor();

	// height of a key strip
	int levelhk = (rectc.Height()-8-vplayer)/10;

	// offset rect
	CRect rect(rectc);
	rect.left += m_dataAreaX;

	// draw X & Y axis values
	ReplayResource resmax;
	_PaintAxis(pDC, rect, resmax, MSK_XLINE,m_timeBegin,m_timeEnd);

	// draw hotkey numbers
	pDC->SetBkMode(TRANSPARENT);
	int oldclr = pDC->SetTextColor(RGB(0,0,0));
	COLORREF clrkey=RGB(255,255,0);
	COLORREF clrkeydark=CHsvRgb::Darker(clrkey,0.50);
	for(int i=0;i<10;i++)
	{
		// compute symbol position
		int y = rect.top+vplayer + i*levelhk;

		// text size
		CString hkslot;
		hkslot.Format("%d",i==9?0:i+1);

		// compute key rect
		CRect rectText;
		int wkey = min (24,levelhk-4);
		int hkey = (85*levelhk)/100;
		int delta = (levelhk-hkey)/2;
		rectText.SetRect(rect.left+6-m_dataAreaX,y+delta,0,y+hkey+delta);
		rectText.right = rectText.left + wkey;
		while(rectText.Width()<4) rectText.InflateRect(1,1);

		// fill key rect
		COLORREF clr = m_list->IsHotKeyUsed(i) ? clrkey : clrkeydark;
		//pDC->FillSolidRect(rectText,clr);
		Gradient::Fill(pDC,rectText,clr,CHsvRgb::Darker(clr,0.70));
		pDC->Draw3dRect(rectText,clr,CHsvRgb::Darker(clr,0.80));

		// draw text
		pDC->DrawText(hkslot,rectText,DT_CENTER|DT_SINGLELINE|DT_VCENTER);
	}

	// draw hotkey events
	_PaintHotKeyEvent(pDC, rect,0);

	// paint foreground layer (axis lines + cursor)
	_PaintForegroundLayer(pDC, rect, resmax, MSK_XLINE);

	// restore rect
	rect.left -= m_dataAreaX;

	// restore every gdi stuff
	pDC->SetBkMode(oldMode);
	pDC->SetTextColor(oldclr);
	pDC->SetBkColor(oldbkClr);
}

//-----------------------------------------------------------------------------------------------------------------

static int _gType;
static ReplayEvtList* _gList;

static int _gCompareElements( const void *arg1, const void *arg2 )
{
	int i1 = *((int*)arg1);
	int i2 = *((int*)arg2);
	return _gList->GetDistCount(_gType,i2)-_gList->GetDistCount(_gType,i1);
}


void DlgStats::_PaintDistribution(CDC *pDC, const CRect& rect, int type)
{
	//empty distribution?
	if(m_list->GetDistTotal(type)==0) return;

	// select font & color
	CFont *oldFont = pDC->SelectObject(m_pLabelFont);
	COLORREF oldClr = pDC->SetTextColor(clrUnitName);
	COLORREF bkClr = pDC->GetBkColor();

	// compute number of non null elements in the distribution
	int elemcount=m_list->GetDistNonNullCount(type);
	int dvalmax = m_seePercent ? 50 : m_replay.GetDistPeak(type);

	// skip elements with 0.0% count
	if(m_seePercent)
	{
		for(int i=0; i<m_list->GetDistMax(type); i++)
		{
			int dval = (100*m_list->GetDistCount(type,i))/m_list->GetDistTotal(type);
			if(dval>dvalmax) dvalmax=dval;
			if(m_list->GetDistCount(type,i)>0 && dval==0) 
				elemcount--;
		}
	}

	// compute bar height
	CRect barRect = rect;
	barRect.top+=32;
	barRect.DeflateRect(hleft,0);
	int barHeight = elemcount==0 ? 0 : min(40,barRect.Height()/elemcount); 

	// select adequate font
	pDC->SelectObject((barHeight<18) ? ((barHeight<10) ? m_pSmallFont : m_pLayerFont) : m_pLabelFont);
	int barStart = 150; //(barHeight<18) ? ((barHeight<10) ? 80 : 110) : 150;

	// create array of pointer to distribution elements
	int *elemIdx = new int[m_list->GetDistMax(type)];
	int j=0;
	for(int i=0; i<m_list->GetDistMax(type); i++)
	{
		// skip elements with 0 count
		if(m_list->GetDistCount(type,i)==0) continue;
		// skip elements with 0.0% count
		if(m_seePercent && (100*m_list->GetDistCount(type,i))/m_list->GetDistTotal(type)==0) continue;
		elemIdx[j]=i;
		j++;
	}

	// sort array
	if(m_sortDist)
	{
		_gType=type;
		_gList=m_list;
		qsort(elemIdx,j,sizeof(int),_gCompareElements);
	}

	// for each element
	CString val;
	barRect.bottom = barRect.top + barHeight;
	int barSpacing = (15*barHeight)/100;
	//for(int i=0; i<m_list->GetDistMax(type); i++)
	for(int k=0; k<j; k++)
	{
		int i=elemIdx[k];

		/*
		// skip elements with 0 count
		if(m_list->GetDistCount(type,i)==0) continue;
		// skip elements with 0.0% count
		if(m_seePercent && (100*m_list->GetDistCount(type,i))/m_list->GetDistTotal(type)==0) continue;
		*/

		// display element name
		//pDC->SetBkColor(RGB(0,0,0));
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(clrUnitName);
		pDC->DrawText(m_list->GetDistName(type,i),barRect,DT_LEFT|DT_SINGLELINE|DT_VCENTER);

		// what value
		int dval = m_seePercent ? (100*m_list->GetDistCount(type,i))/m_list->GetDistTotal(type) : m_list->GetDistCount(type,i);			

		// draw bar
		int barWidth = dval*(barRect.Width() - (barStart+42))/dvalmax;
		CRect zbar=barRect;
		zbar.DeflateRect(barStart,barSpacing,0,barSpacing);
		zbar.right =zbar.left+barWidth;
		COLORREF clr = m_list->GetDistColor(type,i);
		Gradient::Fill(pDC,&zbar,clr,CHsvRgb::Darker(clr,0.6),GRADIENT_FILL_RECT_V);
		//pDC->FillSolidRect(&zbar,clr);

		// draw value
		if(m_seePercent)
		{
			/*
			if(dval==0)
			{
				double fval = (100.0*m_list->GetDistCount(type,i))/m_list->GetDistTotal(type);
				val.Format("%.1f%%",fval);
			}
			else
			*/
				val.Format("%d%%",dval);
		}
		else
			val.Format("%d",dval);
		pDC->SetTextColor(clrUnitName);
		int oldMode=pDC->SetBkMode(TRANSPARENT);
		zbar.left=zbar.right+4;
		zbar.right+=42;
		pDC->DrawText(val,zbar,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
		pDC->SetBkMode(oldMode);

		// next element
		barRect.OffsetRect(0,barHeight);
	}

	delete[]elemIdx;

	// restore every gdi stuff
	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldClr);
	pDC->SetBkColor(bkClr);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_GetDataRectForPlayer(int plidx, CRect& rect, int pcount)
{
	assert(pcount>0);
	rect = m_boardRect;
	rect.bottom = rect.top+rect.Height()/pcount;
	rect.OffsetRect(0,rect.Height()*plidx);
}

//-----------------------------------------------------------------------------------------------------------------

// paint one chart for one player
void DlgStats::_PaintChart(CDC *pDC, int chartType, const CRect& rect, int minMargin)
{
	//what kind of chart do we paint?
	hleft = max(minMargin,6);
	if(chartType==BUILDORDER)
	{
		// build order
		_PaintBuildOrder(pDC,rect);
	}
	else if(chartType==HOTKEYS)
	{
		// build hotkeys
		_PaintHotKeys(pDC,rect);
	}
	else if(chartType!=RESOURCES && chartType!=MAPCOVERAGE && chartType!=APM)
	{
		// any distribution
		_PaintDistribution(pDC,rect,chartType-2);
	}
	else
	{
		hleft = max(minMargin,30);

		// paint background layer
		_PaintBackgroundLayer(pDC, rect, m_list->ResourceMax());
		
		// draw selected events & resources
		_PaintEvents(pDC, chartType, rect, m_list->ResourceMax());

		// paint foreground layer
		_PaintForegroundLayer(pDC, rect, m_list->ResourceMax());
	}

	// draw player name
	_PaintPlayerName(pDC, rect, m_list, -1, -1, clrPlayer);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintMultipleCharts(CDC *pDC)
{
	m_rectMapName.SetRectEmpty();

	// for each player on board
	CRect rect;
	m_shape = SQUARE;
	for(int i=0,j=0; i<m_replay.GetPlayerCount() && j<m_maxPlayerOnBoard; i++)
	{
		// get event list for that player
		m_list = m_replay.GetEvtList(i);
		if(!m_list->IsEnabled()) continue;
		
		// get rect for the player's charts
		_GetDataRectForPlayer(j, rect, m_maxPlayerOnBoard);

		// paint one chart for one player
		_PaintChart(pDC, m_chartType, rect);

		// offset shape for next player
		m_shape=(eSHAPE)((int)m_shape+1);
		j++;
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintMixedCharts(CDC *pDC, int playerIdx, int *chartType, int count)
{
	m_rectMapName.SetRectEmpty();

	// for required chart
	CRect rect;
	m_shape = SQUARE;
	m_mixedCount=count;
	m_MixedPlayerIdx=playerIdx;
	for(int i=0; i<count; i++)
	{
		// get event list for that player
		m_list = m_replay.GetEvtList(playerIdx);
		
		// get rect for the chart
		_GetDataRectForPlayer(i, rect, count);

		// paint one chart 
		int ctype = m_chartType;
		m_chartType = chartType[i];
		_PaintChart(pDC, chartType[i], rect, 35);
		m_mixedRect[i]=rect;
		m_mixedChartType[i]=m_chartType;
		m_chartType=ctype;
	}
}

//-----------------------------------------------------------------------------------------------------------------

// get index of first enabled player
int DlgStats::_GetFirstEnabledPlayer() const
{
	// for each player on board
	for(int i=0; i<m_replay.GetPlayerCount(); i++)
		if(m_replay.GetEvtList(i)->IsEnabled()) 
			return i;
	return 0;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintSingleChart(CDC *pDC)
{
	CRect rect = m_boardRect;

	hleft = 30;

	// paint background layer
	_PaintBackgroundLayer(pDC, rect, m_replay.ResourceMax());

	// for each player on board
	m_shape = SQUARE;
	for(int i=0,j=0; j<m_maxPlayerOnBoard; i++)
	{
		// get event list for that player
		m_list = m_replay.GetEvtList(i);
		if(!m_list->IsEnabled()) continue;
		
		// draw selected events & resources
		_PaintEvents(pDC, m_chartType, rect, m_replay.ResourceMax(),i);
		m_shape=(eSHAPE)((int)m_shape+1);
		j++;
	}

	// paint foreground layer
	_PaintForegroundLayer(pDC, rect, m_replay.ResourceMax());

	// paint map name
	_PaintMapName(pDC,rect);
	int offv = vplayer-8;

	// draw players name
	for(int i=0,j=0; j<m_maxPlayerOnBoard; i++)
	{
		// get event list for that player
		ReplayEvtList* list = m_replay.GetEvtList(i);
		if(!list->IsEnabled()) continue;
		_PaintPlayerName(pDC, rect, list, i,j, clrPlayer,offv);
		j++;
	}

	// paint sentinel logo if needed
	rect.SetRect(rect.left+hplayer+220,rect.top,rect.left+hplayer+280,rect.top+28);
	_PaintSentinel(pDC, rect);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintCheckBoxColor(CDC *pDC)
{
	UINT cid[]={IDC_MINERALS,IDC_GAZ,IDC_SUPPLY,IDC_UNITS};

	for(int i=0;i<sizeof(cid)/sizeof(cid[0]);i++)
	{
		CRect rect;
		GetDlgItem(cid[i])->GetWindowRect(&rect);
		ScreenToClient(&rect);
		rect.OffsetRect(-8,0);
		rect.DeflateRect(0,2);
		rect.right = rect.left+6; rect.top--;
		pDC->FillSolidRect(&rect,_GetDrawingTools(0)->m_clr[i]);
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_PaintCharts(CDC *pDC)
{
	m_maxPlayerOnBoard = min(MAXPLAYERONVIEW,m_replay.GetEnabledPlayerCount());
	if(m_maxPlayerOnBoard>0)
	{
		if(m_chartType==MIX_APMHOTKEYS)
		{
			int ctype[]={APM,HOTKEYS};
			_PaintMixedCharts(pDC,_GetFirstEnabledPlayer(),ctype,sizeof(ctype)/sizeof(ctype[0]));
		}
		else if(!m_singleChart[m_chartType] || (m_chartType!=RESOURCES && m_chartType!=APM && m_chartType!=MAPCOVERAGE))
			_PaintMultipleCharts(pDC);
		else
			_PaintSingleChart(pDC);

		_PaintCheckBoxColor(pDC);
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_SelectAction(int idx)
{
	// update selected player
	char szPlayerName[128];
	m_listEvents.GetItemText(idx,1,szPlayerName,sizeof(szPlayerName));
	for(int i=0; i<m_replay.GetPlayerCount(); i++)
		if(_stricmp(m_replay.GetEvtList(i)->PlayerName(),szPlayerName)==0) 
			{m_selectedPlayer = i; break;}
		
	// update current cursor pos
	ReplayEvt *evt = _GetEventFromIdx(idx);
	_SetTimeCursor(evt->Time(),false);

	// suspect event?
	GetDlgItem(IDC_SUSPECT_INFO)->SetWindowText("");
	if(evt->IsSuspect())
	{
		CString origin;
		ReplayEvtList *list = (ReplayEvtList *)evt->GetAction()->GetUserData(0);
		list->GetSuspectEventOrigin(evt->GetAction(),origin,m_useSeconds?true:false);
		GetDlgItem(IDC_SUSPECT_INFO)->SetWindowText(origin);
	}

	// remember selected action index
	m_selectedAction = idx;
}

//---------------------------------------------------------------------------------------

void DlgStats::OnItemchangedListevents(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if(pNMListView->uNewState&(LVIS_SELECTED+LVIS_FOCUSED) && !m_lockListView && (pNMListView->iItem!=-1))
	{
		// update selected player & cursor pos
		_SelectAction(pNMListView->iItem);
	}
	*pResult = 0;
}

//---------------------------------------------------------------------------------------

inline unsigned long DlgStats::_GetActionCount()
{
	return m_replay.GetEnActionCount();//(unsigned long)m_replay.QueryFile()->QueryActions()->GetActionCount();
}

//---------------------------------------------------------------------------------------

ReplayEvt * DlgStats::_GetEventFromIdx(unsigned long idx)
{
	assert(idx<_GetActionCount());
	const IStarcraftAction *action = m_replay.GetEnAction(idx); //m_replay.QueryFile()->QueryActions()->GetAction(idx);
	ReplayEvtList *list = (ReplayEvtList *)action->GetUserData(0);
	ReplayEvt *evt = list->GetEvent(action->GetUserData(1));
	return evt;
}

//---------------------------------------------------------------------------------------

// cursor = value between 0 and full span time
// will find the nearest event 
unsigned long DlgStats::_GetEventFromTime(unsigned long cursor)
{
	unsigned long eventCount = _GetActionCount();
	if(eventCount==0) return 0;

	unsigned long nSlot = 0;
	unsigned long low = 0;
	unsigned long high = eventCount - 1;
	unsigned long beginTimeTS = 0;
	unsigned long i;
	unsigned long eventTime;

	// for all events in the list
	while(true)
	{
		i= (high+low)/2;
		ASSERT(high>=low);

		// are we beyond the arrays boundaries?
		if(i>=eventCount) {nSlot=0;break;}

		// get event time
		ReplayEvt *evt = _GetEventFromIdx(i);
		eventTime = evt->Time();

		// compare times		
		LONGLONG delta = eventTime-beginTimeTS;
		LONGLONG nCmp = (LONGLONG )cursor - delta;

		// if event time is the same, return index
		if(nCmp==0) 
		{
			nSlot = i; 
			goto Exit;
		}
		else if(nCmp<0) 
		{
			if(high==low) {nSlot = low; break;}
			high = i-1;
			if(high<low) {nSlot=low; break;}
			if(high<0) {nSlot=0; break;}
		}
		else
		{
			if(high==low) {nSlot = low+1; break;}
			low = i+1;
			if(low>high) {nSlot=high; break;}
			if(low>=eventCount-1) {nSlot=eventCount-1; break;}
		}
	}
	
	ASSERT(nSlot<eventCount);

Exit:
	// make sure event belong to the right player
	char szPlayerName[128];
	int nSlotBegin=nSlot;
	for(;nSlot<eventCount;)
	{
		// get event time
		ReplayEvt *evt = _GetEventFromIdx(nSlot); assert(evt!=0);
		if(evt->Time()>cursor) break;
		m_listEvents.GetItemText(nSlot,1,szPlayerName,sizeof(szPlayerName));
		if(strcmp(m_replay.GetEvtList(m_selectedPlayer)->PlayerName(),szPlayerName)!=0) nSlot++; else break;
	}
	if(nSlot>=eventCount) nSlot=nSlotBegin;
	else
	{
		ReplayEvt *evt = _GetEventFromIdx(nSlot);
		if(evt->Time()>cursor) nSlot=nSlotBegin;
	}
	return nSlot;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_AdjustWindow() 
{
	unsigned long windowWidth = (unsigned long)((double)m_replay.GetEndTime()/pow(2.0f,m_zoom));
	if(m_timeCursor<windowWidth/2)
	{
		m_timeBegin = 0;
		m_timeEnd = windowWidth;
	}
	else if(m_timeCursor+windowWidth/2>m_replay.GetEndTime())
	{
		m_timeBegin = m_replay.GetEndTime()-windowWidth;
		m_timeEnd = m_replay.GetEndTime();
	}
	else
	{
		m_timeBegin = m_timeCursor-windowWidth/2;
		m_timeEnd = m_timeCursor+windowWidth/2;

	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnSelchangeZoom() 
{
	UpdateData(TRUE);
	if(m_zoom==0)
	{
		m_timeBegin = 0;
		m_timeEnd = m_replay.GetEndTime();
	}
	else
	{
		_AdjustWindow();
	}
	
	Invalidate(FALSE);
}

//--------------------------------------------------------------------------------

BOOL DlgStats::_OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	// calc new x position
	int x = m_scroller.GetScrollPos();
	int xOrig = x;

	switch (LOBYTE(nScrollCode))
	{
	case SB_TOP:
		x = 0;
		break;
	case SB_BOTTOM:
		x = INT_MAX;
		break;
	case SB_LINEUP:
		x -= m_lineDev.cx;
		break;
	case SB_LINEDOWN:
		x += m_lineDev.cx;
		break;
	case SB_PAGEUP:
		x -= m_pageDev.cx;
		break;
	case SB_PAGEDOWN:
		x += m_pageDev.cx;
		break;
	case SB_THUMBTRACK:
		x = nPos;
		break;
	}

	// calc new y position
	int y = m_scrollerV.GetScrollPos();
	int yOrig = y;

	switch (HIBYTE(nScrollCode))
	{
	case SB_TOP:
		y = 0;
		break;
	case SB_BOTTOM:
		y = INT_MAX;
		break;
	case SB_LINEUP:
		y -= m_lineDev.cy;
		break;
	case SB_LINEDOWN:
		y += m_lineDev.cy;
		break;
	case SB_PAGEUP:
		y -= m_pageDev.cy;
		break;
	case SB_PAGEDOWN:
		y += m_pageDev.cy;
		break;
	case SB_THUMBTRACK:
		y = nPos;
		break;
	}

	BOOL bResult = _OnScrollBy(CSize(x - xOrig, y - yOrig), bDoScroll);
	/*
	if (bResult && bDoScroll)
	{
		Invalidate();
		UpdateWindow();
	} */

	return bResult;
}

//------------------------------------------------------------------------------------------

BOOL DlgStats::_OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
	int xOrig, x;
	int yOrig, y;

	// don't scroll if there is no valid scroll range (ie. no scroll bar)
	CScrollBar* pBar;
	DWORD dwStyle = GetStyle();
	pBar = &m_scrollerV;
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_VSCROLL)))
	{
		// vertical scroll bar not enabled
		sizeScroll.cy = 0;
	}
	pBar = &m_scroller;
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_HSCROLL)))
	{
		// horizontal scroll bar not enabled
		sizeScroll.cx = 0;
	}

	// adjust current x position
	xOrig = x = m_scroller.GetScrollPos();
	//int xMax = GetScrollLimit(SB_HORZ);
	int xMax, xMin ;
	m_scroller.GetScrollRange(&xMin,&xMax) ;
	x += sizeScroll.cx;
	if (x < xMin)
		x = xMin;
	else if (x > xMax)
		x = xMax;

	// adjust current y position
	yOrig = y = m_scrollerV.GetScrollPos();
	//int yMax = GetScrollLimit(SB_VERT);
	int yMax, yMin ;
	m_scrollerV.GetScrollRange(&yMin,&yMax) ;
	
	y += sizeScroll.cy;
	if (y < 0)
		y = 0;
	else if (y > yMax)
		y = yMax;

	// did anything change?
	if (x == xOrig && y == yOrig)
		return FALSE;

	if (bDoScroll)
	{
		// do scroll and update scroll positions
		if (x != xOrig)
		{
			_SetTimeCursor(x*HSCROLL_DIVIDER);
		}
		if (y != yOrig)
		{
			m_scrollerV.SetScrollPos(y);
			//???????????
		}
	}
	return TRUE;
}

//---------------------------------------------------------------------------------------

void DlgStats::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	_OnScroll(MAKEWORD(nSBCode, -1), nPos, TRUE);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnUpdateChart() 
{
	BOOL oldUseSeconds = m_useSeconds;
	UpdateData(TRUE);
	InvalidateRect(&m_boardRect,FALSE);	
	if(oldUseSeconds != m_useSeconds)
		m_listEvents.Invalidate();
}

//-------------------------------------------------------------------------------------

void DlgStats::_UpdateActionFilter(bool refresh) 
{
	// compute filter
	int filter=0;
	if(m_fltSelect) filter+=Replay::FLT_SELECT;
	if(m_fltBuild) filter+=Replay::FLT_BUILD;
	if(m_fltTrain) filter+=Replay::FLT_TRAIN;
	if(m_fltSuspect) filter+=Replay::FLT_SUSPECT;
	if(m_fltHack) filter+=Replay::FLT_HACK;
	if(m_fltOthers) filter+=Replay::FLT_OTHERS;
	if(m_fltChat) filter+=Replay::FLT_CHAT;
	m_replay.UpdateFilter(filter);

	// update list view
	if(refresh)
	{
		m_listEvents.SetItemCountEx(m_replay.GetEnActionCount(), LVSICF_NOSCROLL|LVSICF_NOINVALIDATEALL);
		m_listEvents.Invalidate();
	}
}

//-------------------------------------------------------------------------------------

void DlgStats::_ToggleIgnorePlayer() 
{
	POSITION pos = m_plStats.GetFirstSelectedItemPosition();
	if(pos!=0)
	{
		// get selected item
		int nItem = m_plStats.GetNextSelectedItem(pos);
		ReplayEvtList *list = (ReplayEvtList *)m_plStats.GetItemData(nItem);
		if(list!=0)
		{
			// enable/disable player
			m_replay.EnablePlayer(list,list->IsEnabled()?false:true);
			LVITEM item = {LVIF_IMAGE ,nItem,0,0,0,0,0,list->IsEnabled()?1:0,0,0}; 
			m_plStats.SetItem(&item);
			// update list view
			int nItem=-1;
			m_listEvents.SetItemCountEx(m_replay.GetEnActionCount(), LVSICF_NOSCROLL|LVSICF_NOINVALIDATEALL);
			m_listEvents.Invalidate();
			/*
			POSITION pos = m_listEvents.GetFirstSelectedItemPosition();
			if (pos != NULL) nItem = pList->GetNextSelectedItem(pos);
			//if(nItem>=m_replay.GetEnActionCount()) m_listEvents.SnItem=m_replay.GetEnActionCount()-1;
			*/

			//repaint
			InvalidateRect(m_boardRect,TRUE);

			// update map
			m_dlgmap->UpdateReplay(&m_replay);
 		}
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnEnableDisable()
{
	_ToggleIgnorePlayer();
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnDblclkPlstats(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_ToggleIgnorePlayer();
	*pResult = 0;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnDestroy() 
{
	// if we are on pause, unpause
	if(m_prevAnimationSpeed!=0) OnPause();

	// save parameters
	_Parameters(false);
	DlgBrowser::SaveColumns(&m_plStats,"plstats");

	CDialog::OnDestroy();
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnSelchangeCharttype() 
{
	static bool gbTimeWndHasChanged = false; 

	OnUpdateChart();

	// update shared "single chart" checkbox
	CButton *btn = (CButton *)GetDlgItem(IDC_SINGLECHART);
	btn->SetCheck(m_singleChart[m_chartType]);
	btn->EnableWindow((m_chartType==RESOURCES || m_chartType==APM || m_chartType==MAPCOVERAGE)?TRUE:FALSE);

	// update shared "see units" checkbox
	btn = (CButton *)GetDlgItem(IDC_UNITS);
	btn->SetCheck(m_seeUnits[m_chartType]);
	btn->EnableWindow((m_chartType==RESOURCES || m_chartType==MAPCOVERAGE)?TRUE:FALSE);

	// update shared "apm style" combo box
	CComboBox *cbx = (CComboBox *)GetDlgItem(IDC_APMSTYLE);
	cbx->SetCurSel(m_apmStyle[m_chartType]);
	cbx->EnableWindow((m_chartType==APM || m_chartType==MAPCOVERAGE)?TRUE:FALSE);

	m_dataAreaX = 0;
	m_mixedCount=0;
	m_MixedPlayerIdx=0;

	// shall we restore the full timeline? 
	if(gbTimeWndHasChanged)
	{
		m_timeBegin = 0;
		m_timeEnd = m_replay.GetEndTime();
		gbTimeWndHasChanged = false; 
		m_zoom=0;
		UpdateData(FALSE);
	}

	switch(m_chartType)
	{
	case MAPCOVERAGE:
		{
			// disable useless options
			UINT disable[]={IDC_ANIMATE,IDC_PERCENTAGE,IDC_MINERALS,IDC_GAZ,IDC_SUPPLY,	IDC_SORTDIST,
			IDC_SPEED,IDC_BPM,IDC_UPM,IDC_ACTIONS,IDC_HOTPOINTS,IDC_UNITSONBO,IDC_HKSELECT};
			for(int i=0;i<sizeof(disable)/sizeof(disable[0]);i++) GetDlgItem(disable[i])->EnableWindow(FALSE);

			// enable useful options
			UINT enable[]={IDC_USESECONDS,IDC_ZOOM,IDC_UNITS,IDC_APMSTYLE};
			for(int i=0;i<sizeof(enable)/sizeof(enable[0]);i++) GetDlgItem(enable[i])->EnableWindow(TRUE);
		}
		break;
	case MIX_APMHOTKEYS:
		{
			m_dataAreaX = 0;

			// disable useless options
			UINT disable[]={IDC_PERCENTAGE,IDC_MINERALS,IDC_GAZ,IDC_SUPPLY,	IDC_SORTDIST,IDC_UNITS,
				IDC_ACTIONS,IDC_HOTPOINTS,IDC_UNITSONBO,IDC_APMSTYLE};
			for(int i=0;i<sizeof(disable)/sizeof(disable[0]);i++) GetDlgItem(disable[i])->EnableWindow(FALSE);

			// enable useful options
			UINT enable[]={IDC_USESECONDS,IDC_ZOOM,IDC_HKSELECT,IDC_ANIMATE,IDC_SPEED,IDC_BPM,IDC_UPM,IDC_APMSTYLE};
			for(int i=0;i<sizeof(enable)/sizeof(enable[0]);i++) GetDlgItem(enable[i])->EnableWindow(TRUE);
		}
		break;
	case HOTKEYS:
		{
			m_dataAreaX = 32;

			// disable useless options
			UINT disable[]={IDC_ANIMATE,IDC_PERCENTAGE,IDC_MINERALS,IDC_GAZ,IDC_SUPPLY,	IDC_SORTDIST,
			IDC_SPEED,IDC_BPM,IDC_UPM,IDC_UNITS,IDC_ACTIONS,IDC_HOTPOINTS,IDC_UNITSONBO,IDC_APMSTYLE};
			for(int i=0;i<sizeof(disable)/sizeof(disable[0]);i++) GetDlgItem(disable[i])->EnableWindow(FALSE);

			// enable useful options
			UINT enable[]={IDC_USESECONDS,IDC_ZOOM,IDC_HKSELECT};
			for(int i=0;i<sizeof(enable)/sizeof(enable[0]);i++) GetDlgItem(enable[i])->EnableWindow(TRUE);
		}
		break;
	case BUILDORDER:
		{
			// build order
			m_timeBegin = 0;
			m_timeEnd = m_replay.GetLastBuildOrderTime();
			gbTimeWndHasChanged = true;

			// enable useful options
			GetDlgItem(IDC_UNITSONBO)->EnableWindow(TRUE);
			GetDlgItem(IDC_USESECONDS)->EnableWindow(TRUE);

			// disable useless options
			UINT disable[]={IDC_ZOOM,IDC_ANIMATE,IDC_PERCENTAGE,IDC_MINERALS,IDC_GAZ,IDC_SUPPLY, IDC_SORTDIST,
			IDC_SPEED,IDC_BPM,IDC_UPM,IDC_UNITS,IDC_ACTIONS,IDC_HOTPOINTS,IDC_HKSELECT,IDC_APMSTYLE};
			for(int i=0;i<sizeof(disable)/sizeof(disable[0]);i++) GetDlgItem(disable[i])->EnableWindow(FALSE);
		}
		break;
	case RESOURCES:
		{
			// disable useless options
			UINT disable[]={IDC_SPEED,IDC_BPM,IDC_UPM,
				IDC_HKSELECT,IDC_APMSTYLE,IDC_SORTDIST,IDC_PERCENTAGE,IDC_UNITSONBO};
			for(int i=0;i<sizeof(disable)/sizeof(disable[0]);i++) GetDlgItem(disable[i])->EnableWindow(FALSE);

			// enable useful options
			UINT enable[]={IDC_ZOOM,IDC_ANIMATE,IDC_MINERALS,IDC_GAZ,IDC_SUPPLY,
				IDC_UNITS,IDC_USESECONDS,IDC_ACTIONS,IDC_HOTPOINTS};
			for(int i=0;i<sizeof(enable)/sizeof(enable[0]);i++) GetDlgItem(enable[i])->EnableWindow(TRUE);
		}
		break;
	case APM:
		{
			// disable useless options
			UINT disable[]={IDC_MINERALS,IDC_GAZ,IDC_SUPPLY,IDC_UNITS,IDC_ACTIONS,
				IDC_HKSELECT,IDC_SORTDIST,IDC_PERCENTAGE,IDC_UNITSONBO};
			for(int i=0;i<sizeof(disable)/sizeof(disable[0]);i++) GetDlgItem(disable[i])->EnableWindow(FALSE);

			// enable useful options
			UINT enable[]={IDC_ZOOM,IDC_ANIMATE,IDC_SPEED,IDC_BPM,IDC_UPM,IDC_USESECONDS,IDC_APMSTYLE};
			for(int i=0;i<sizeof(enable)/sizeof(enable[0]);i++) GetDlgItem(enable[i])->EnableWindow(TRUE);
		}
		break;
	default:
		{
		// any distribution
		GetDlgItem(IDC_PERCENTAGE)->EnableWindow(TRUE);
		GetDlgItem(IDC_SORTDIST)->EnableWindow(TRUE);

		// disable useless options
		UINT disable[]={IDC_ZOOM,IDC_ANIMATE,IDC_MINERALS,IDC_GAZ,IDC_SUPPLY,IDC_UNITSONBO,
		IDC_SPEED,IDC_BPM,IDC_UPM,IDC_UNITS,IDC_ACTIONS,IDC_HOTPOINTS,IDC_HKSELECT,IDC_APMSTYLE};
		for(int i=0;i<sizeof(disable)/sizeof(disable[0]);i++) GetDlgItem(disable[i])->EnableWindow(FALSE);
		}
		break;
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnHelp() 
{
	DlgHelp	dlg;
	dlg.DoModal();
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_BrowseReplays(const char *dir, int& count, int &idx, bool bCount)
{
	CFileFind finder;
	char mask[255];
	strcpy(mask,dir);
	if(mask[strlen(mask)-1]!='\\') strcat(mask,"\\");
	strcat(mask,"*.*");

	// load all replays
	BOOL bWorking = finder.FindFile(mask);
	while (bWorking)
	{
		// find next package
		bWorking = finder.FindNextFile();

		//dir?
		if(finder.IsDirectory())
		{
			// . & ..
			if(finder.IsDots()) continue;

			// recurse
			char subdir[255];
			strcpy(subdir,dir);
			if(subdir[strlen(subdir)-1]!='\\') strcat(subdir,"\\");
			strcat(subdir,finder.GetFileName());
			_BrowseReplays(subdir, count, idx, bCount);
			continue;
		}

		// rep file?
		CString ext;
		ext=finder.GetFileName().Right(4);
		if(ext.CompareNoCase(".rep")!=0) continue;

		if(bCount)
		{
			count++;
		}
		else
		{
			// load it
			if(m_replay.Load(finder.GetFilePath(),false,0,true)!=0)
			{
				CString msg;
				msg.Format(IDS_CANTLOAD,(const char*)finder.GetFilePath());
				MessageBox(msg);
			}
			idx++;

			// update progress bar
			m_progress.SetPos((100*idx)/count);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnTestreplays() 
{
	m_progress.ShowWindow(SW_SHOW);

	// count available replays
	int count=0;
	int idx=0;
	_BrowseReplays("e:\\program files\\starcraft\\maps\\", count, idx, true);

	// laod them
	_BrowseReplays("e:\\program files\\starcraft\\maps\\", count, idx, false);

	m_replay.Clear();
	m_progress.ShowWindow(SW_HIDE);
}

//-----------------------------------------------------------------------------------------------------------------

static bool gbAscendingAction=true;

int CALLBACK CompareAction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int diff=0;
	const ReplayEvt *evt1 = (const ReplayEvt *)lParam1;
	const ReplayEvt *evt2 = (const ReplayEvt *)lParam2;
	unsigned long rep1 = evt1->Time();
	unsigned long rep2 = evt2->Time();

	switch(lParamSort)
	{
		case 0:
			diff = rep1 - rep2;
			break;
		default:
			assert(0);
			break;
	}

	return gbAscendingAction ? diff : -diff;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnAddevent() 
{
	// get path
	CString path;
	if(!_GetReplayFileName(path))
		return;

	// add events from that replay
	LoadReplay(path,false);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnRclickPlstats(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CPoint pt;
	GetCursorPos(&pt);

	// select only one player
	for(int i=0;i<m_plStats.GetItemCount();i++)
		m_plStats.SetItemState(i,0, LVIS_SELECTED+LVIS_FOCUSED);

	// find corresponding item
	m_plStats.ScreenToClient(&pt);
	UINT uFlags;
	int nItem = m_plStats.HitTest(pt,&uFlags);
	if(nItem!=-1 && (uFlags & LVHT_ONITEMLABEL)!=0)
	{
		// select item
		m_selectedPlayerList = (ReplayEvtList *)m_plStats.GetItemData(nItem);
		m_plStats.SetItemState(nItem,LVIS_SELECTED+LVIS_FOCUSED, LVIS_SELECTED+LVIS_FOCUSED);

		// load menu
		CMenu menu;
		menu.LoadMenu(IDR_POPUPPLAYER);
		CMenu *pSub = menu.GetSubMenu(0);

		// display popup menu
		m_plStats.ClientToScreen(&pt);
		pSub->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}
	
	*pResult = 0;
}

//----------------------------------------------------------------------------------------

void DlgStats::OnRemovePlayer() 
{
	if(m_selectedPlayerList!=0)
	{
		// remove player events
		m_replay.RemovePlayer(m_selectedPlayerList,&m_listEvents);
		m_selectedPlayer=0;

		// update player stats list
		_DisplayPlayerStats();

		//repaint
		Invalidate(TRUE);
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnSeemap() 
{
	m_dlgmap->ShowWindow(SW_SHOW);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::StopAnimation() 
{
	if(m_bIsAnimating) OnAnimate();
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_UpdateAnimSpeed()
{
	CString str;
	if(m_bIsAnimating) str.Format("(x%d)",m_animationSpeed);
	SetDlgItemText(IDC_ANIMSPEED,str);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_ToggleAnimateButtons(bool enable)
{
	CString str;
	str.LoadString(enable ? IDS_ANIMATE:IDS_STOP);
	SetDlgItemText(IDC_ANIMATE,str);
	GetDlgItem(IDC_SPEEDPLUS)->EnableWindow(enable ? FALSE : TRUE);
	GetDlgItem(IDC_SPEEDMINUS)->EnableWindow(enable ? FALSE : TRUE);
	GetDlgItem(IDC_PAUSE)->EnableWindow(enable ? FALSE : TRUE);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnAnimate() 
{
	if(m_bIsAnimating)
	{
		// if we are on pause, unpause
		if(m_prevAnimationSpeed!=0) OnPause();

		//stopping
		KillTimer(m_timer);
		m_bIsAnimating=false;
		_ToggleAnimateButtons(true);
		GetDlgItem(IDC_ZOOM)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHARTTYPE)->EnableWindow(TRUE);

		// tell map
		m_dlgmap->Animate(false);

		// repaint
		Invalidate();
	}
	else if(m_replay.IsDone())
	{
		// starting 
		m_timeBegin = 0;
		m_timeEnd = m_replay.GetEndTime();
		//m_timeCursor = 0;
		m_zoom=0;
		UpdateData(FALSE);
		_ToggleAnimateButtons(false);
		GetDlgItem(IDC_ZOOM)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHARTTYPE)->EnableWindow(FALSE);
		m_bIsAnimating=true;
		_UpdateAnimSpeed();
		InvalidateRect(m_boardRect,TRUE);
		UpdateWindow();
		
		// show animated map
		m_dlgmap->Animate(true);
		m_dlgmap->ShowWindow(SW_SHOW);

		// start timer
		m_timer = SetTimer(1,1000/TIMERSPEED,0);
	}
	
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnTimer(UINT nIDEvent) 
{
	// compute next position
	unsigned long newpos = m_timeCursor+m_replay.QueryFile()->QueryHeader()->Sec2Tick(m_animationSpeed)/TIMERSPEED;

	// if we reach the end, stop animation
	if(newpos>m_timeEnd) OnAnimate();

	// udpate cursor
	if(m_animationSpeed>0) _SetTimeCursor(newpos, true,false);

	// update map
	if(m_dlgmap->IsWindowVisible()) m_dlgmap->UpdateTime(newpos);

	CDialog::OnTimer(nIDEvent);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnPause() 
{
	if(m_prevAnimationSpeed==0)
	{
		m_prevAnimationSpeed = m_animationSpeed;
		m_animationSpeed = 0;
	}
	else
	{
		m_animationSpeed = m_prevAnimationSpeed;
		m_prevAnimationSpeed = 0;
	}
}

void DlgStats::OnSpeedminus() 
{
	if(m_animationSpeed>1) m_animationSpeed/=2;
	_UpdateAnimSpeed();
}

void DlgStats::OnSpeedplus() 
{
	if(m_animationSpeed<32) m_animationSpeed*=2;
	_UpdateAnimSpeed();
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnGetdispinfoListevents(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;

	int iItemIndx= pItem->iItem;

	if (pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		// get action
		const IStarcraftAction *action = m_replay.GetEnAction(iItemIndx);//m_replay.QueryFile()->QueryActions()->GetAction(iItemIndx);
		assert(action!=0);

		// get corresponding actionlist
		ReplayEvtList *list = (ReplayEvtList *)action->GetUserData(0);
		assert(list!=0);

		// get event description
		ReplayEvt *evt = list->GetEvent(action->GetUserData(1));
		assert(evt!=0);

		// display value
		switch(pItem->iSubItem)
		{
			case 0: //time
				strcpy(pItem->pszText,_MkTime(m_replay.QueryFile()->QueryHeader(),evt->Time(),m_useSeconds?true:false));
				break;
			case 1: //player
				if(evt->IsSuspect() || evt->IsHack())
					sprintf(pItem->pszText,"#FF0000%s",list->PlayerName());
				else
					strcpy(pItem->pszText,list->PlayerName());
				break;
			case 2: //action
				//assert(evt->Time()!=37925);
				if(OPTIONSCHART->m_coloredevents)
					sprintf(pItem->pszText,"%s%s",evt->strTypeColor(),evt->strType());
				else
					sprintf(pItem->pszText,"%s",evt->strType());
				break;
			case 3: //parameters
				strcpy(pItem->pszText,action->GetParameters(list->GetElemList()));
				break;
			case 4: //discard & suspect flag
				strcpy(pItem->pszText,evt->IsDiscarded()?"*":evt->IsSuspect()?"#FF0000?":evt->IsHack()?"#FF0000!":"");
				break;
			case 5: // units ID
				strcpy(pItem->pszText,action->GetUnitsID(list->GetElemList()));
				break;
			default:
				assert(0);
				break;
		}
		assert(pItem->cchTextMax>(int)strlen(pItem->pszText));
	}

	if(pItem->mask & LVIF_IMAGE) //valid image?
			pItem->iImage=0;
	
	*pResult = 0;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_GetResizeRect(CRect& resizeRect)
{
	GetClientRect(&resizeRect);
	resizeRect.top = resizeRect.bottom - m_hlist - 8 - 6;
	resizeRect.bottom =resizeRect.top+6;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_GetHorizResizeRect(CRect& resizeRect)
{
	GetClientRect(&resizeRect);
	resizeRect.top = resizeRect.bottom - m_hlist - 8 - 6;
	resizeRect.left = m_boardRect.left+ m_wlist;
	resizeRect.right = m_boardRect.left+m_wlist+8;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// get tracking rect for resizing
	CRect resizeRect;
	_GetResizeRect(resizeRect);
	CRect horizResizeRect;
	_GetHorizResizeRect(horizResizeRect);

	// clicking on the resize part?
	if(resizeRect.PtInRect(point))
	{
		m_resizing=VERTICAL_RESIZE;
		SetCapture();
		m_ystart = point.y;
	}
	// clicking on the horizontal resize part?
	else if(horizResizeRect.PtInRect(point))
	{
		m_resizing=HORIZONTAL_RESIZE;
		SetCapture();
		m_xstart = point.x;
	}
	// clicking on map name?
	else if(m_rectMapName.PtInRect(point))
	{
		OnSeemap(); 
	}
	else
	{
		CRect rect = m_boardRect;
		rect.left+=hleft+m_dataAreaX;
		rect.right-=hright;

		// clicking on the graphics?
		if(rect.PtInRect(point) && m_maxPlayerOnBoard>0 && 
			(m_chartType==APM || m_chartType==RESOURCES || m_chartType==BUILDORDER || m_chartType==MAPCOVERAGE || m_chartType==HOTKEYS || m_chartType>=MIX_APMHOTKEYS))
		{
			// what player?
			if(!m_singleChart[m_chartType]) m_selectedPlayer = (point.y-m_boardRect.top) / (m_boardRect.Height() / m_maxPlayerOnBoard);

			// change time cursor
			float fx = (float)(point.x-rect.left);
			float finc = (float)(rect.Width())/(float)(m_timeEnd - m_timeBegin);
			_SetTimeCursor(m_timeBegin + (unsigned long)(fx/finc));
		}
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(m_resizing!=NONE)
	{
		// release capture
		ReleaseCapture();

		// compute new layout
		if(m_resizing==VERTICAL_RESIZE)
		{
			int newhlist = m_hlist + (m_ystart-point.y);
			m_hlist = newhlist;
		}
		else
		{
			int newwlist = m_wlist + (point.x-m_xstart);
			m_wlist = newwlist;
		}

		// resize
		CRect rect;
		GetClientRect(&rect);
		_Resize(rect.Width(),rect.Height());

		// end resizing
		m_resizing=NONE;
	}
	
	CDialog::OnLButtonUp(nFlags, point);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_GetHotKeyDesc(ReplayEvtList *list, int slot, CString& info)
{
	info="";

	// browse hotkey events
	for(int i=0;i<(int)list->GetHKEventCount();i++)
	{
		// get event
		const HotKeyEvent *hkevt = list->GetHKEvent(i);

		// skip events for other slots
		if(hkevt->m_slot!=slot) continue;

		// skip hotkey selection
		if(hkevt->m_type==HotKeyEvent::SELECT) continue;

		// new line
		if(!info.IsEmpty()) info+="\r\n";

		// build unit list as string
		char buffer[64];
		strcpy(buffer,_MkTime(m_replay.QueryFile()->QueryHeader(),hkevt->m_time,m_useSeconds?true:false));
		info+=buffer+CString(" =>  ");
		const HotKey * hk = hkevt->GetHotKey();
		for(int uidx=0;uidx<hk->m_unitcount;uidx++)
		{
			if(uidx>0) info+=", ";
			m_replay.QueryFile()->QueryHeader()->MkUnitID2String(buffer, hk->m_hotkeyUnits[uidx], list->GetElemList(), hkevt->m_time);
			info += buffer;
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::_CheckForHotKey(ReplayEvtList *list, CPoint& point, CRect& datarect, int delta)
{
	CRect symRect;

	// check hotkey numbers
	if(point.x < datarect.left+m_dataAreaX+delta)
	{
		// height of a key strip
		int levelhk = (datarect.Height()-8-vplayer)/10;
		// check all keys
		for(int i=0;i<10;i++)
		{
			// compute symbol position
			int y = datarect.top+vplayer + i*levelhk;
			CRect rectkey(datarect.left+6,y,datarect.left+6+m_dataAreaX+delta,y+levelhk);
			if(rectkey.PtInRect(point))
			{
				// build description
				CString info;
				_GetHotKeyDesc(list, i, info);
				// update overlay window
				m_over->SetText(info,this,point);
				return;
			}
		}
	}

	// check hotkey events
	datarect.left+=m_dataAreaX;
	for(int i=0;i<(int)list->GetHKEventCount();i++)
	{
		// get event
		const HotKeyEvent *hkevt = list->GetHKEvent(i);
		if(hkevt->m_time < m_timeBegin || hkevt->m_time > m_timeEnd) continue;

		// view hotkey selection?
		if(!m_viewHKselect && hkevt->m_type==HotKeyEvent::SELECT) continue;

		// compute symbol position
		_ComputeHotkeySymbolRect(list, hkevt, datarect, symRect);

		// is mouse over this event?
		const HotKey * hk = hkevt->GetHotKey();
		if(symRect.PtInRect(point) && hk!=0)
		{
			// build unit list as string
			CString info;
			char buffer[64];
			for(int uidx=0;uidx<hk->m_unitcount;uidx++)
			{
				if(!info.IsEmpty()) info+="\r\n";
				m_replay.QueryFile()->QueryHeader()->MkUnitID2String(buffer, hk->m_hotkeyUnits[uidx], list->GetElemList(), hkevt->m_time);
				info += buffer;
			}
			// update overlay window
			m_over->SetText(info,this,point);
			return;
		}
	}
	m_over->Show(false);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(m_resizing!=NONE)
	{
		m_over->Show(false);
		//int newhlist = m_hlist + (m_ystart-point.y);
		//CRect resizeRect = m_boardRect;
		//resizeRect.top =newhlist;
		//resizeRect.bottom =newhlist+2;
		//DrawTrackRect(
	}
	else if(m_chartType==HOTKEYS)
	{
		// for each player on board
		CRect rect;
		for(int i=0,j=0; i<m_replay.GetPlayerCount() && j<m_maxPlayerOnBoard; i++)
		{
			// get event list for that player
			ReplayEvtList *list = m_replay.GetEvtList(i);
			if(!list->IsEnabled()) continue;

			// get rect for the player's charts
			_GetDataRectForPlayer(j++, rect, m_maxPlayerOnBoard);

			// is mouse in that rect?
			if(rect.PtInRect(point))
			{
				// check if mouse is over a hotkey symbol
				_CheckForHotKey(list,point, rect);
				return;
			}
		}

		m_over->Show(false);
	}
	else if(m_chartType==MIX_APMHOTKEYS)
	{
		if(m_MixedPlayerIdx<m_replay.GetPlayerCount() && m_mixedCount>0)
		{
			// get event list for current player
			ReplayEvtList *list = m_replay.GetEvtList(m_MixedPlayerIdx);

			// get rect for the player's charts
			CRect rect;
			_GetDataRectForPlayer(1, rect, m_mixedCount);

			// is mouse in that rect?
			if(rect.PtInRect(point))
			{
				// check if mouse is over a hotkey symbol
				_CheckForHotKey(list,point, rect, 32);
				return;
			}
		}

		m_over->Show(false);
	}
	
	CDialog::OnMouseMove(nFlags, point);
}

//-----------------------------------------------------------------------------------------------------------------

BOOL DlgStats::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// get tracking rect for vertical resizing
	CRect resizeRect;
	_GetResizeRect(resizeRect);

	// get tracking rect for horizontal resizing
	CRect horizResizeRect;
	_GetHorizResizeRect(horizResizeRect);
	
	// mouse over the resize part?
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	if(resizeRect.PtInRect(point))
	{
		// top/bottom resize
		::SetCursor(::LoadCursor(0,IDC_SIZENS));
		return TRUE; 
	}
	else if(horizResizeRect.PtInRect(point))
	{
		// left/right resize
		::SetCursor(::LoadCursor(0,IDC_SIZEWE));
		return TRUE; 
	}
	else if(m_rectMapName.PtInRect(point))
	{
		// map name
		::SetCursor(AfxGetApp()->LoadStandardCursor(MAKEINTRESOURCE(32649)));
		return TRUE; 
	}
	
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if(!m_replay.IsDone()) return;

	CPoint pt;
	GetCursorPos(&pt);

	// load menu
	CMenu menu;
	menu.LoadMenu(IDR_WATCHREP);
	CMenu *pSub = menu.GetSubMenu(0);

	// display popup menu
	pSub->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, pt.x, pt.y, this);
	
	CDialog::OnRButtonDown(nFlags, point);
}

//--------------------------------------------------------------------------------------------------------------

void DlgStats::_StartCurrentReplay(int mode)
{
	CFileFind finder;
	BOOL bWorking = finder.FindFile(m_replay.GetFileName());
	if(bWorking)
	{
		// get replay file info
		finder.FindNextFile();

		// get replay info object from replay
		ReplayInfo *rep = MAINWND->pGetBrowser()->_ProcessReplay(finder.GetRoot(),finder);
		if(rep) MAINWND->pGetBrowser()->StartReplay(rep, mode);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgStats::OnWatchReplay111()
{
	_StartCurrentReplay(BW_111);
}

//--------------------------------------------------------------------------------------------------------------

void DlgStats::OnWatchReplay112()
{
	_StartCurrentReplay(BW_112);
}

//--------------------------------------------------------------------------------------------------------------

void DlgStats::OnWatchReplay114()
{
	_StartCurrentReplay(BW_114);
}

//--------------------------------------------------------------------------------------------------------------

void DlgStats::OnWatchReplay115()
{
	_StartCurrentReplay(BW_115);
}

//--------------------------------------------------------------------------------------------------------------

void DlgStats::OnWatchReplay116()
{
	_StartCurrentReplay(BW_116);
}

//--------------------------------------------------------------------------------------------------------------

void DlgStats::OnWatchReplay113()
{
	_StartCurrentReplay(BW_113);
}

//--------------------------------------------------------------------------------------------------------------

void DlgStats::OnWatchReplay110()
{
	_StartCurrentReplay(BW_110);
}

//--------------------------------------------------------------------------------------------------------------

void DlgStats::OnWatchReplay109()
{
	_StartCurrentReplay(BW_109);
}

//--------------------------------------------------------------------------------------------------------------

void DlgStats::OnWatchReplaySC()
{
	_StartCurrentReplay(BW_SC);
}

//--------------------------------------------------------------------------------------------------------------

void DlgStats::OnWatchReplay()
{
	_StartCurrentReplay(BW_AUTO);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnRclickListevents(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(m_replay.IsDone()) 
	{
		// load menu
		CMenu menu;
		menu.LoadMenu(IDR_MENU_EVENTS);
		CMenu *pSub = menu.GetSubMenu(0);

		// display popup menu
		CPoint pt;
		GetCursorPos(&pt);
		pSub->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, pt.x, pt.y, this);	
	}
	
	*pResult = 0;
}

//-------------------------------------------------------------------------------------

static char szFilterTxt[] = "Text File (*.txt)|*.txt|All Files (*.*)|*.*||";
static char szFilterHtml[] = "HTML File (*.html)|*.html|All Files (*.*)|*.*||";

bool DlgStats::_GetFileName(const char *filter, const char *ext, const char *def, CString& file) 
{
 	CFileDialog dlg(FALSE,ext,def,0,filter,this);
	CString str = AfxGetApp()->GetProfileString("BWCHART_MAIN","EXPDIR","");
	if(!str.IsEmpty()) dlg.m_ofn.lpstrInitialDir = str;
	if(dlg.DoModal()==IDOK)
	{
		file = dlg.GetPathName();
		AfxGetApp()->WriteProfileString("BWCHART_MAIN","EXPDIR",file);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnExportToText()
{
	assert(m_replay.IsDone());

	// get export file
	CString file;
	if(!_GetFileName(szFilterTxt, "txt", "bwchart.txt", file)) return;

	// create file
	CWaitCursor wait;
	int err = m_replay.ExportToText(file,m_useSeconds?true:false,'\t');
	if(err==-1) {AfxMessageBox(IDS_CANTCREATFILE); return;}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnExportToBWCoach()
{
	assert(m_replay.IsDone());
	ExportCoachDlg dlg(this,&m_replay);
	dlg.DoModal();
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnExportToHtml()
{
	assert(m_replay.IsDone());

	// get export file
	CString file;
	if(!_GetFileName(szFilterHtml, "html", "bwchart.html", file)) return;

	// create file
	FILE *fp=fopen(file,"wb");
	if(fp==0) {AfxMessageBox(IDS_CANTCREATFILE); return;}

	// list events in list view
	CWaitCursor wait;
	for(unsigned long i=0;i<m_replay.GetEnActionCount(); i++)
	{
		// get action
		const IStarcraftAction *action = m_replay.GetEnAction((int)i);
	}

	//close file
	fclose(fp);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnFilterChange() 
{
	UpdateData(TRUE);
	_UpdateActionFilter(true);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnNextSuspect() 
{
	if(!m_replay.IsDone()) return;

	// find next suspect event
	int newidx = m_replay.GetNextSuspectEvent(m_selectedAction);
	if(newidx!=-1)
	{
		// select corresponding line in list view
		m_lockListView=true;
		m_listEvents.SetItemState(newidx,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
		m_listEvents.EnsureVisible(newidx,FALSE);
		m_lockListView=false;

		// update selected player & cursor pos
		_SelectAction(newidx);
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgStats::OnSelchangeApmstyle() 
{
	UpdateData(TRUE);
	if(m_replay.IsDone())
	{
		// if apm style was changed
		if(m_replay.UpdateAPM(m_apmStyle[APM],m_apmStyle[MAPCOVERAGE]))
		{
			// repaint chart
			InvalidateRect(m_boardRect,FALSE);
			// update player stats
			_DisplayPlayerStats();
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------

