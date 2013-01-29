// DlgBrowser.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DlgBrowser.h"
#include "DlgEditComment.h"
#include "DlgBWChart.h"
#include "DlgFastAka.h"
#include "DlgRename.h"
#include "replay.h"
#include "dirutil.h"
#include "bwdb.h"
#include "progressdlg.h"
#include "regparam.h"
#include <io.h>

#include "../common/mergeaudio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef CSIDL_PROGRAM_FILES
#define CSIDL_PROGRAM_FILES             0x0026
#endif

#define WATCHFILE "!replay.rep"
#define CORRUPTED_DIR "corrupted"

BEGIN_MESSAGE_MAP(DlgBrowser, CDialog)
	//{{AFX_MSG_MAP(DlgBrowser)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_REPS, OnColumnclickReps)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LISTBOS, OnColumnclickBos)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LISTPLAYERS, OnColumnclickPlayers)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LISTMAPS, OnColumnclickMaps)
	ON_NOTIFY(NM_RCLICK, IDC_REPS, OnRclickReps)
	ON_NOTIFY(NM_DBLCLK, IDC_REPS, OnDblclkReps)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_REPS, OnItemchangedReps)
	ON_BN_CLICKED(IDC_EDITCOMMENT, OnEditcomment)
	ON_BN_CLICKED(IDC_AUTOREFRESH, OnAutorefresh)
	ON_BN_CLICKED(IDC_ADDREPLAY, OnAddreplay)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_PFILTERON, OnToggleFilter)
	ON_EN_CHANGE(IDC_FPLAYER, OnChangeFplayer)
	ON_WM_TIMER()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_REPS, OnGetdispinfoReps)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTPLAYERS, OnItemchangedListplayers)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTBOS, OnItemchangedListBos)
	ON_NOTIFY(NM_RCLICK, IDC_LISTPLAYERS, OnRclickListplayers)
	ON_BN_CLICKED(IDC_MUZERG, OnMatchup)
	ON_BN_CLICKED(IDC_RWAONLY, OnRwaonly)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LISTPLAYERS, OnGetdispinfoListplayers)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LISTBOS, OnGetdispinfoListBos)
	ON_EN_CHANGE(IDC_FPOSITIONS, OnChangeFpositions)
	ON_EN_CHANGE(IDC_FMAP, OnChangeFmap)
	ON_NOTIFY(TCN_SELCHANGE, IDC_VTAB, OnSelchangeVtab)
	ON_BN_CLICKED(IDC_CLEANMISSING, OnCleanmissing)
	ON_BN_CLICKED(IDC_BO_BUILDING, OnToggleBOOption)
	ON_EN_CHANGE(IDC_BOMAXOBJ, OnChangeBomaxobj)
	ON_BN_CLICKED(IDC_POSFILTERON, OnToggleFilter)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio)
	ON_BN_CLICKED(IDC_MUTERRAN, OnMatchup)
	ON_BN_CLICKED(IDC_MUTOSS, OnMatchup)
	ON_BN_CLICKED(IDC_MUXVX, OnMatchup)
	ON_BN_CLICKED(IDC_MAPFILTERON, OnToggleFilter)
	ON_BN_CLICKED(IDC_BO_UNIT, OnToggleBOOption)
	ON_BN_CLICKED(IDC_BO_RESEARCH, OnToggleBOOption)
	ON_BN_CLICKED(IDC_BO_UPGRADE, OnToggleBOOption)
	ON_BN_CLICKED(IDC_BO_SUPPLY, OnToggleBOOption)
	ON_CBN_SELCHANGE(IDC_COMBOMU, OnSelchangeCombomu)
	ON_BN_CLICKED(IDC_MUTERRAN2, OnToggleBOOption)
	ON_BN_CLICKED(IDC_MUTOSS2, OnToggleBOOption)
	ON_BN_CLICKED(IDC_MUZERG2, OnToggleBOOption)
	ON_BN_CLICKED(IDC_FILTERONBO, OnToggleFilterOnBo)
	ON_COMMAND(ID_F_LOADREPLAY,OnLoadReplay)
	ON_COMMAND(ID__ADDREPLAYEVENTS,OnAddReplayEvents)
	ON_COMMAND(ID__ADDTOFAVORITES,OnAddToFavorites)
	ON_COMMAND(ID__EDITCOMMENTS,OnEditcomment) 
	ON_COMMAND(ID__WATCHREPLAY_BW109,OnWatchReplay109)
	ON_COMMAND(ID__WATCHREPLAY_BW110,OnWatchReplay110)
	ON_COMMAND(ID__WATCHREPLAY_BW111,OnWatchReplay111)
	ON_COMMAND(ID__WATCHREPLAY_BW112,OnWatchReplay112)
	ON_COMMAND(ID__WATCHREPLAY_BW113,OnWatchReplay113)
	ON_COMMAND(ID__WATCHREPLAY_BW114,OnWatchReplay114)
	ON_COMMAND(ID__WATCHREPLAY_BW115,OnWatchReplay115)
	ON_COMMAND(ID__WATCHREPLAY_BW116,OnWatchReplay116)
	ON_COMMAND(ID__WATCHREPLAYINBW_SC,OnWatchReplaySC)
	ON_COMMAND(ID__WATCHREPLAYINBW_AUTO,OnWatchReplay)
	ON_COMMAND(ID__UPDATEAKAS,OnUpdateAkas)
	ON_COMMAND(ID__OPENDIRECTORY,OnOpenDirectory)
	ON_COMMAND(ID_G_LISTREPLAYS,OnListReplays)
	ON_COMMAND(ID__FILE_MOVETORECYCLEBIN,OnMoveToBin)
	ON_COMMAND(ID__MOVETOFOLDER,OnMoveToFolder)
	ON_COMMAND(ID__FILE_RENAME,OnRenameReplays)
	ON_MESSAGE(WM_AUTOREFRESH,OnMsgAutoRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//--------------------------------------------------------------------------------------------------------------

DlgBrowser::DlgBrowser(CWnd* pParent /*=NULL*/)
	: CDialog(DlgBrowser::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgBrowser)
	m_rootdir = _T("");
	m_groupby = -1;
	m_replayCount = _T("");
	m_mapCount = _T("");
	m_playerCount = _T("");
	m_autoRefresh = FALSE;
	m_pfilteron = FALSE;
	m_filterPlayer = _T("");
	m_recursive = FALSE;
	m_muT = FALSE;
	m_muP = FALSE;
	m_muZ = FALSE;
	m_muXvX = FALSE;
	m_rwaOnly = FALSE;
	m_filterPosition = _T("");
	m_posfilteron = FALSE;
	m_filterMap = _T("");
	m_mapFilterOn = FALSE;
	m_boBuilding = FALSE;
	m_boResearch = FALSE;
	m_boUnit = FALSE;
	m_boUpgrade = FALSE;
	m_boSupply = FALSE;
	m_boMaxObj = 0;
	m_bomuT = FALSE;
	m_bomuP = FALSE;
	m_bomuZ = FALSE;
	m_refreshReplays = false;
	//}}AFX_DATA_INIT

	m_bRefreshDone=0;
	m_progDlg=0;
	m_reftimer=0;
	m_noRepaint=false;
	m_bDBLoaded=false;

	//selected replay
	m_selectedReplay=0;
	m_idxSelectedReplay=0;

	// selected player
	m_selectedPlayer=0;
	m_idxSelectedPlayer=0;

	// load parameters
	_Parameters(true);

	// get saved root dir (if any)
	if(m_rootdir.IsEmpty()) m_rootdir = AfxGetApp()->GetProfileString("BROWSER","ROOTDIR",""); // compatibility with 1.00V

	// if we dont have a root dir
	if(m_rootdir.IsEmpty())
	{
		// assume starcraft is in the Program Files dir
		_GetStarcraftPath(m_rootdir);
		if(m_rootdir.IsEmpty())
		{
			char windir[255];
			SHGetSpecialFolderPath(GetSafeHwnd(),windir,CSIDL_PROGRAM_FILES,FALSE);
			if(windir[strlen(windir)-1]!='\\') strcat(windir,"\\");
			strcat(windir,"Starcraft\\");
			m_rootdir=windir;
		}
		m_rootdir+="maps\\";
	}

	// sorting info
	memset(m_Descending,1,sizeof(m_Descending));
	m_currentSortIdx=0;
	memset(m_Descending2,1,sizeof(m_Descending2));
	m_currentSortIdx2=0;
	memset(m_Descending3,1,sizeof(m_Descending3));
	m_currentSortIdx3=0;
	memset(m_Descending4,1,sizeof(m_Descending4));
	m_currentSortIdx4=0;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgBrowser)
	DDX_Control(pDX, IDC_COMBOMU, m_bomu);
	DDX_Control(pDX, IDC_VTAB, m_vtab);
	DDX_Control(pDX, IDC_LISTMAPS, m_listMaps);
	DDX_Control(pDX, IDC_LISTPLAYERS, m_listPlayers);
	DDX_Control(pDX, IDC_LISTBOS, m_listBos);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_REPS, m_reps);
	DDX_Text(pDX, IDC_DIR, m_rootdir);
	DDX_Radio(pDX, IDC_RADIO1, m_groupby);
	DDX_Text(pDX, IDC_REPLAYCOUNT, m_replayCount);
	DDX_Text(pDX, IDC_MAPCOUNT, m_mapCount);
	DDX_Text(pDX, IDC_PLAYERCOUNT, m_playerCount);
	DDX_Check(pDX, IDC_AUTOREFRESH, m_autoRefresh);
	DDX_Check(pDX, IDC_PFILTERON, m_pfilteron);
	DDX_Text(pDX, IDC_FPLAYER, m_filterPlayer);
	DDX_Check(pDX, IDC_RECURSIVE, m_recursive);
	DDX_Check(pDX, IDC_MUTERRAN, m_muT);
	DDX_Check(pDX, IDC_MUTOSS, m_muP);
	DDX_Check(pDX, IDC_MUZERG, m_muZ);
	DDX_Check(pDX, IDC_MUXVX, m_muXvX);
	DDX_Check(pDX, IDC_RWAONLY, m_rwaOnly);
	DDX_Text(pDX, IDC_FPOSITIONS, m_filterPosition);
	DDX_Check(pDX, IDC_POSFILTERON, m_posfilteron);
	DDX_Text(pDX, IDC_FMAP, m_filterMap);
	DDX_Check(pDX, IDC_MAPFILTERON, m_mapFilterOn);
	DDX_Check(pDX, IDC_BO_BUILDING, m_boBuilding);
	DDX_Check(pDX, IDC_BO_RESEARCH, m_boResearch);
	DDX_Check(pDX, IDC_BO_UNIT, m_boUnit);
	DDX_Check(pDX, IDC_BO_UPGRADE, m_boUpgrade);
	DDX_Check(pDX, IDC_BO_SUPPLY, m_boSupply);
	DDX_Text(pDX, IDC_BOMAXOBJ, m_boMaxObj);
	DDX_Check(pDX, IDC_MUTERRAN2, m_bomuT);
	DDX_Check(pDX, IDC_MUTOSS2, m_bomuP);
	DDX_Check(pDX, IDC_MUZERG2, m_bomuZ);
	//}}AFX_DATA_MAP
}

//-----------------------------------------------------------------------------------------------------------------

void DlgBrowser::_Parameters(bool bLoad)
{
	PINT("BWCHART_BROWSER",autoRefresh,TRUE);
	PSTRING("BWCHART_BROWSER",rootdir,"");
	PINT("BWCHART_BROWSER",groupby,0);
	PINT("BWCHART_BROWSER",sortrep,0);
	PINT("BWCHART_BROWSER",sortpla,0);
	PINT("BWCHART_BROWSER",sortmap,0);
	PINT("BWCHART_BROWSER",pfilteron,FALSE);
	PINT("BWCHART_BROWSER",posfilteron,FALSE);
	PINT("BWCHART_BROWSER",mapFilterOn,FALSE);
	PINT("BWCHART_BROWSER",recursive,TRUE);
	PSTRING("BWCHART_BROWSER",filterPlayer,"");
	PSTRING("BWCHART_BROWSER",filterPosition,"");
	PSTRING("BWCHART_BROWSER",filterMap,"");
	PINT("BWCHART_BROWSER",muT,TRUE);
	PINT("BWCHART_BROWSER",muZ,TRUE);
	PINT("BWCHART_BROWSER",muP,TRUE);
	PINT("BWCHART_BROWSER",muXvX,TRUE);
	PINT("BWCHART_BROWSER",rwaOnly,FALSE);
	PSTRING("BWCHART_BROWSER",lastFolder,"");
   	PINT("BWCHART_BROWSER",boBuilding,TRUE);
	PINT("BWCHART_BROWSER",boResearch,TRUE);
	PINT("BWCHART_BROWSER",boUnit,FALSE);
	PINT("BWCHART_BROWSER",boUpgrade,FALSE);
	PINT("BWCHART_BROWSER",boSupply,FALSE);
	PINT("BWCHART_BROWSER",boMaxObj,6);
	PINT("BWCHART_BROWSER",bomuT,1);
	PINT("BWCHART_BROWSER",bomuP,1);
	PINT("BWCHART_BROWSER",bomuZ,1);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	_Resize();
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_Resize()
{
	if(m_reps.m_hWnd == NULL)
		return;      // Return if window is not created yet.

	// Get size of dialog window.
	CRect rect;
	GetClientRect(&rect);

	// Adjust the rectangle width
	rect.DeflateRect(7,0);

	// Adjust the rectangle position & height
	CRect rectCtrl;
	GetDlgItem(IDC_BOMAXOBJ)->GetWindowRect(&rectCtrl);
	ScreenToClient(&rectCtrl);
	rect.top=rectCtrl.top+rectCtrl.Height()+8;
	rect.bottom-=8;

	// adjust position for vertical tab
	CRect rectVtab;
	m_vtab.GetWindowRect(&rectVtab);
	ScreenToClient(rectVtab);
	rectVtab.left=rect.left;
	rectVtab.top=rect.top;
	rectVtab.bottom=rect.bottom-20;
	m_vtab.MoveWindow(&rectVtab, TRUE);

	rect.left += rectVtab.Width()-2;

	// Move the list control to the new position and size.
	m_listPlayers.MoveWindow(&rect, TRUE);
	m_listMaps.MoveWindow(&rect, TRUE);
	m_listBos.MoveWindow(&rect, TRUE);

	// adjust position replay list
	int right = rect.right;
	rect.bottom-=20;
	m_reps.MoveWindow(&rect, TRUE);   

	// adjust position for replay comment
	rect.top=rect.bottom+6; rect.bottom=rect.top+16; rect.right=70+rectVtab.Width();
	GetDlgItem(IDC_COMMENT1)->MoveWindow(&rect, TRUE);
	rect.left=rect.right+4; rect.right=right-48;
	GetDlgItem(IDC_COMMENT)->MoveWindow(&rect, TRUE);
	rect.left=rect.right+4; rect.right=right;
	GetDlgItem(IDC_EDITCOMMENT)->SetWindowPos(0,rect.left,rect.top,0,0, SWP_NOZORDER|SWP_NOSIZE);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnBrowse() 
{
	const char *newpath = UtilDir::BrowseDir("Select a directory with replays",m_rootdir);
	if(newpath!=0) 
	{
		// update dir
		m_rootdir = newpath;
		UpdateData(FALSE);

		// refresh screen
		OnRefresh() ;
	}
}

//--------------------------------------------------------------------------------------------------------------

// show or hide bo filters
void DlgBrowser::_ShowBoFilter(bool show)
{
	UINT boid[]={IDC_STATICBO,IDC_BOMAXOBJ,IDC_SPIN1,IDC_BO_BUILDING,IDC_BO_UNIT,IDC_BO_RESEARCH,IDC_BO_UPGRADE,IDC_BO_SUPPLY,
		IDC_COMBOMU,IDC_STATICVS,IDC_MUTERRAN2,IDC_MUZERG2,IDC_MUTOSS2,IDC_BOREPLAYS,0};
	for(int i=0;boid[i]!=0;i++)
		GetDlgItem(boid[i])->ShowWindow(show?SW_SHOWNOACTIVATE:SW_HIDE);

	GetDlgItem(IDC_FILTERONBO)->ShowWindow(!show?SW_SHOWNOACTIVATE:SW_HIDE);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnRadio() 
{

	UpdateData(TRUE);
	switch(m_groupby)
	{
	case 0:
		if(m_refreshReplays) _DisplayList(false);
		m_listPlayers.ShowWindow(SW_HIDE);
		m_listMaps.ShowWindow(SW_HIDE);
		m_reps.ShowWindow(SW_SHOW);
		m_reps.SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
		GetDlgItem(IDC_COMMENT1)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_COMMENT)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDITCOMMENT)->ShowWindow(SW_SHOW);
		m_listBos.ShowWindow(SW_HIDE);
		_ShowBoFilter(false);
		break;
	case 1:
		m_reps.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMMENT1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMMENT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDITCOMMENT)->ShowWindow(SW_HIDE);
		m_listMaps.ShowWindow(SW_HIDE);
		m_listPlayers.ShowWindow(SW_SHOW);
		m_listBos.ShowWindow(SW_HIDE);
		_ShowBoFilter(false);
		break;
	case 2:
		m_reps.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMMENT1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMMENT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDITCOMMENT)->ShowWindow(SW_HIDE);
		m_listPlayers.ShowWindow(SW_HIDE);
		m_listMaps.ShowWindow(SW_SHOW);
		m_listBos.ShowWindow(SW_HIDE);
		_ShowBoFilter(false);
		break;
	case 3:
		m_reps.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMMENT1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMMENT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDITCOMMENT)->ShowWindow(SW_HIDE);
		m_listPlayers.ShowWindow(SW_HIDE);
		m_listMaps.ShowWindow(SW_HIDE);
		m_listBos.ShowWindow(SW_SHOW);
		_ShowBoFilter(true);
		break;
	}
}

//--------------------------------------------------------------------------------------------------------------

BOOL DlgBrowser::OnInitDialog() 
{
	UINT repCol[]={IDS_COL_REPLAYNAME,IDS_COL_PLAYER1,IDS_COL_APM1,IDS_COL_PLAYER2,IDS_COL_APM2	
		,IDS_COL_MAP,IDS_COL_DURATION,IDS_COL_TYPE	,IDS_COL_RWAAUTHOR,IDS_COL_GAMEDATE,IDS_COL_ENGINE	
		,IDS_COL_COMMENT,IDS_COL_DIRECTORY,IDS_COL_FILEDATE,IDS_COL_POS1,IDS_COL_POS2,IDS_HACKCOUNT};
	int repWidth[]={195,120,45,120,45,125,60,40,110,80,55,300,300,80,45,45,45};

	UINT plCol[]={IDS_COL_PLAYERNAME,IDS_COL_AVGAPM,IDS_COL_APMPM,IDS_COL_PERCDEV	,IDS_COL_AVGGAMEDURATION
		,IDS_COL_GAMESPLAYED,IDS_COL_ZERGPERC,IDS_COL_RANPERC,IDS_COL_TOSSPERC,
			IDS_COL_AVGAPMAST,IDS_COL_AVGAPMASZ,IDS_COL_AVGAPMASP};
	int plWidth[]={155,70,60,45,110,85,50,50,50,70,70,70};

	UINT mapCol[]={IDS_COL_MAPNAME,IDS_COL_AVGAPM,IDS_COL_AVGGAMEDURATION,IDS_COL_GAMESPLAYED};
	int mapWidth[]={155,80,120,100};

	UINT boCol[]={IDS_COL_BOCOUNT,IDS_COL_BOPERCENT,IDS_COL_BOCONTENT};
	int boWidth[]={50,40,600};
	
	CDialog::OnInitDialog();

	// set spin range for bo max object
	CSpinButtonCtrl *spin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN1);
	spin->SetRange(1,ReplayObjectSequence::MAXBO);
	
	// prepare replays list control
	m_reps.SetExtendedStyle(m_reps.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP+LVS_EX_FULLROWSELECT);
	CString colTitle;
	for(int i=0;i<sizeof(repCol)/sizeof(repCol[0]);i++)
	{
		if(repCol[i]!=0) colTitle.LoadString(repCol[i]); else colTitle="";
		m_reps.InsertColumn(i, colTitle,LVCFMT_LEFT,repWidth[i],i);
	}

	// Set the background color
	COLORREF clrBack = RGB(255,255,255);
	m_reps.SetBkColor(clrBack);
	m_reps.SetTextBkColor(clrBack);
	m_reps.SetTextColor(RGB(10,10,10));

	// prepare players list control
	m_listPlayers.SetExtendedStyle(m_listPlayers.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP+LVS_EX_FULLROWSELECT);
	for(int i=0;i<sizeof(plCol)/sizeof(plCol[0]);i++)
	{
		if(plCol[i]!=0) colTitle.LoadString(plCol[i]); else colTitle="";
		m_listPlayers.InsertColumn(i, colTitle,LVCFMT_LEFT,plWidth[i],i);
	}

	// Set the background color
	m_listPlayers.SetBkColor(clrBack);
	m_listPlayers.SetTextBkColor(clrBack);
	m_listPlayers.SetTextColor(RGB(10,10,10));

	// prepare maps list control
	m_listMaps.SetExtendedStyle(m_listMaps.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP+LVS_EX_FULLROWSELECT);
	for(int i=0;i<sizeof(mapCol)/sizeof(mapCol[0]);i++)
	{
		if(mapCol[i]!=0) colTitle.LoadString(mapCol[i]); else colTitle="";
		m_listMaps.InsertColumn(i, colTitle,LVCFMT_LEFT,mapWidth[i],i);
	}

	// Set the background color
	m_listMaps.SetBkColor(clrBack);
	m_listMaps.SetTextBkColor(clrBack);
	m_listMaps.SetTextColor(RGB(10,10,10));

	// prepare bos list control
	m_listBos.SetExtendedStyle(m_listBos.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP+LVS_EX_FULLROWSELECT);
	for(int i=0;i<sizeof(boCol)/sizeof(boCol[0]);i++)
	{
		if(boCol[i]!=0) colTitle.LoadString(boCol[i]); else colTitle="";
		m_listBos.InsertColumn(i, colTitle,LVCFMT_LEFT,boWidth[i],i);
	}

	// Set the background color
	m_listBos.SetBkColor(clrBack);
	m_listBos.SetTextBkColor(clrBack);
	m_listBos.SetTextColor(RGB(10,10,10));
	m_listBos.SubclassHeaderControl();

	// by rep
	UpdateData(FALSE);
	OnRadio();

	// load column width
	LoadColumns(&m_reps, "reps");
	LoadColumns(&m_listPlayers, "ply");
	LoadColumns(&m_listMaps, "map");
	LoadColumns(&m_listBos, "bo");

	// init vertical tab
	m_vtab.InsertItem(0,"Replays");
	m_vtab.InsertItem(1,"Players");
	m_vtab.InsertItem(2,"Maps");
	m_vtab.InsertItem(3,"BOs");
	m_vtab.SetCurSel(m_groupby);

	// bo match ups
	int pos = m_bomu.AddString("<All>");
	m_bomu.SetItemData(pos,IStarcraftPlayer::RACE_RANDOM);
	pos = m_bomu.AddString("T");
	m_bomu.SetItemData(pos,IStarcraftPlayer::RACE_TERRAN);
	pos = m_bomu.AddString("P");
	m_bomu.SetItemData(pos,IStarcraftPlayer::RACE_PROTOSS);
	pos = m_bomu.AddString("Z");
	m_bomu.SetItemData(pos,IStarcraftPlayer::RACE_ZERG);
	m_bomu.SetCurSel(AfxGetApp()->GetProfileInt("BWCHART_BROWSER","BOMU",0));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//-----------------------------------------------------------------------------------------------------------------

// add build order
void DlgBrowser::_AddBuildOrder(const CString& bo, ReplayInfo *rep, int pidx)
{
	BuildOrder *pbo = new BuildOrder(rep,pidx);
	pbo->m_bo.FromString(bo);
	pbo->m_desc = bo;
	m_bos.Add(pbo);
}

//-----------------------------------------------------------------------------------------------------------------

// add replay
ReplayInfo * DlgBrowser::_AddReplay(const ReplayInfo& tmpRep)
{
	AkaList *akalist = &MAINWND->pGetAkas()->m_akalist;
	AkaList *akalistmap = &MAINWND->pGetAkas()->m_akalistMap;

	m_allPlayers.RemoveAll();

	// create replay
	ReplayInfo *rep = new ReplayInfo(tmpRep);
	assert(rep!=0);

	// for each player in the replay
	int k=0;
	for(; k<rep->m_playerCount; k++)
	{
		// trim player name
		rep->m_player[k].TrimRight();
		const char *pname=rep->m_player[k];

		// do we know that player already?
		//if(!_FindAllPlayer(pname)) m_allPlayers.Add(pname);

		// do we have an aka for that player?
		const Aka *aka = akalist->GetAkaFromName(pname);
		if(aka!=0) pname = aka->MainName();
		rep->m_mainName[k]=pname;
		
		//ASSERT(!(strncmp(pname,"Obi-1_ElkyNobi",14)==0 && aka==0));

		// update player data
		_AddPlayer(pname, rep, k);

		// add build order
		if(!rep->m_bo[k].IsEmpty())
			_AddBuildOrder(rep->m_bo[k],rep,k);
	}

	// trim map name
	rep->m_map.TrimRight();
	const char *pname=rep->m_map;

	// do we know that map already?
	if(!_FindAllMap(pname)) m_allMaps.Add(pname);

	// do we have an aka for that map?
	const Aka *aka = akalistmap->GetAkaFromName(pname);
	if(aka!=0) {pname = aka->MainName(); rep->m_mainMap=pname;}
	else pname = BWChartDB::ClarifyMapName(rep->m_mainMap,pname);

	// update map data
	_AddMap(pname, rep, k);

	// insert replay 
	m_replays.Add(rep);

	return rep;
}

//-----------------------------------------------------------------------------------------------------------------

const CStringArray *DlgBrowser::GetPlayers() const
{
	// if complete players list not yet built
	if(m_allPlayers.GetSize()==0)
	{
		// for each replay
		for(int i=0;i<m_replays.GetSize();i++)
		{
			ReplayInfo *rep = (ReplayInfo *)m_replays.GetAt(i);
			assert(rep!=0);
			// for each player in the replay
			for(int k=0; k<rep->m_playerCount; k++)
			{
				// trim player name
				const char *pname=rep->m_player[k];

				// do we know that player already?
				if(!_FindAllPlayer(pname)) m_allPlayers.Add(pname);
			}
		}
	}
	return &m_allPlayers;
}

//-----------------------------------------------------------------------------------------------------------------

// update player data
void DlgBrowser::_AddPlayer(const char *pname, ReplayInfo *rep, int k)
{
	PlayerInfo *pl = _FindPlayer(pname);
	if(pl==0) {pl = new PlayerInfo; m_players.Add(pl); pl->m_name = pname; assert(pl!=0);}
	pl->m_apm += rep->m_apm[k];
	pl->m_games++;
	pl->m_duration+=rep->m_duration;
	if(rep->m_race[k]==IStarcraftPlayer::RACE_TERRAN)
	{
		pl->m_apmAsT += rep->m_apm[k];
		pl->m_gamesAsT++;
	}
	else if(rep->m_race[k]==IStarcraftPlayer::RACE_ZERG)
	{
		pl->m_apmAsZ += rep->m_apm[k];
		pl->m_gamesAsZ++;
	}
	if(rep->m_race[k]==IStarcraftPlayer::RACE_PROTOSS)
	{
		pl->m_apmAsP += rep->m_apm[k];
		pl->m_gamesAsP++;
	}

	pl->AddRace(rep->m_race[k]);
	pl->AddApmDev(rep->m_apmDev[k]);
}

//-----------------------------------------------------------------------------------------------------------------

// update map data
void DlgBrowser::_AddMap(const char *pname, ReplayInfo *rep, int k)
{
	MapInfo *map = _FindMap(pname);
	if(map==0) {map = new MapInfo; m_maps.Add(map); map->m_name = pname; assert(map!=0);}
	for(k=0; k<rep->m_playerCount; k++)
	{
		map->m_apm += rep->m_apm[k];
		map->m_apmCount++;
	}
	map->m_duration+=rep->m_duration;
	map->m_games++;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgBrowser::RefreshAkas()
{
	AkaList *akalist = &MAINWND->pGetAkas()->m_akalist;
	AkaList *akalistmap = &MAINWND->pGetAkas()->m_akalistMap;
	const Aka *aka;
	CWaitCursor wait;

	// clear players & map list
	m_players.RemoveAll();
	m_maps.RemoveAll();

	// for each replay
	for(int i=0; i<m_replays.GetSize(); i++)
	{
		// get replay
		ReplayInfo *rep = (ReplayInfo *)m_replays.GetAt(i);

		// for each player
		int k=0;
		for(; k<rep->m_playerCount; k++)
		{
			// do we have an aka for that player?
			const char *pname = rep->m_player[k];
			aka = akalist->GetAkaFromName(pname);
			if(aka!=0) pname=aka->MainName();
			rep->m_mainName[k] = pname;

			// update player data
			_AddPlayer(pname, rep, k);
		}

		// do we have an aka for that map?
		const char *pname=rep->m_map;
		aka = akalistmap->GetAkaFromName(pname);
		if(aka!=0) {pname = aka->MainName(); rep->m_mainMap=pname;}
		else pname = BWChartDB::ClarifyMapName(rep->m_mainMap,pname);

		// update map data
		_AddMap(pname, rep, k);
	}

	// display all
	_DisplayList();
}

//-----------------------------------------------------------------------------------------------------------------

bool DlgBrowser::_LoadReplay(const char *path, const CTime& creationDate, ReplayInfo& tmpRep)
{
	bool bAddToMem=true;

	// parse it
	if(m_replay.Load(path,false,0,true)==0)
	{
		// engine info
		tmpRep.m_engineType = m_replay.QueryFile()->QueryHeader()->getEngine();

		// replay is an RWA?
		if(m_replay.RWAHeader()!=0)
		{
			tmpRep.m_author = m_replay.RWAHeader()->author;
			tmpRep.m_isRWA = true;
		}

		// extract game date
		time_t date = m_replay.QueryFile()->QueryHeader()->getCreationDate();
		if(localtime(&date)!=0) tmpRep.m_date = CTime(date);
		else tmpRep.m_date = CTime(1971,1,1,0,0,0);

		// figure out engine version from date
		int year = max(2003,min(2009,tmpRep.m_date.GetYear()));
		switch(year)
		{
			case 2009:
				tmpRep.m_engineVersion= BWREP_VERSION_116;
				break;
			case 2008:
				if(tmpRep.m_date.GetMonth()>11) 
					tmpRep.m_engineVersion= BWREP_VERSION_116;
				else
					tmpRep.m_engineVersion= BWREP_VERSION_115;
				break;
			case 2007:
				if(tmpRep.m_date.GetMonth()>5) 
					tmpRep.m_engineVersion= BWREP_VERSION_115;
				else
					tmpRep.m_engineVersion= BWREP_VERSION_114;
				break;
			case 2006:
				if(tmpRep.m_date.GetMonth()>7) 
					tmpRep.m_engineVersion= BWREP_VERSION_114;
				else
					tmpRep.m_engineVersion= BWREP_VERSION_113;
				break;
			case 2005:
				if(tmpRep.m_date.GetMonth()>6) 
					tmpRep.m_engineVersion= BWREP_VERSION_113;
				else if(tmpRep.m_date.GetMonth()>2)
					tmpRep.m_engineVersion= BWREP_VERSION_112;
				else
					tmpRep.m_engineVersion= BWREP_VERSION_111;
				break;
			case 2004:
				if(tmpRep.m_date.GetMonth()>4)
					tmpRep.m_engineVersion= BWREP_VERSION_111;
				else
					tmpRep.m_engineVersion= BWREP_VERSION_110;
				break;
			case 2003:
				if(tmpRep.m_date.GetMonth()>3)
					tmpRep.m_engineVersion= BWREP_VERSION_110;
				else
					tmpRep.m_engineVersion= BWREP_VERSION_109;
				break;
		}

		//replay path
		tmpRep.m_path = path;

		//map name
		tmpRep.m_map= m_replay.MapName();

		// hack count
		tmpRep.m_hackCount = m_replay.GetHackCount();

		// list all players
		tmpRep.m_playerCount = min(ReplayInfo::MAXPLAYER,m_replay.GetPlayerCount());
		int j=0;
		for(int k=0; k<tmpRep.m_playerCount; k++)
		{
			// keep only "enabled" players, skip observers
			ReplayEvtList * list = m_replay.GetEvtList(k);
			if(!list->IsEnabled()) continue;
			tmpRep.m_player[j] = list->PlayerName();
			tmpRep.m_apm[j]= list->GetActionPerMinute();
			tmpRep.m_apmDev[j]= list->GetStandardAPMDev(-1,-1);
			tmpRep.m_race[j] = list->GetRaceIdx();
			tmpRep.m_start[j] = list->GetStartingLocation();
			list->GetFinalBuildOrder(tmpRep.m_bo[j]);
			j++;
		}
		tmpRep.m_playerCount=j;
		tmpRep.m_duration = m_replay.QueryFile()->QueryHeader()->Tick2Sec(m_replay.GetEndTime());
		tmpRep.m_filedate=creationDate;
	}
	else
	{
		// corrupted replay
		bAddToMem=false;
		ASSERT(0);
	}

	return bAddToMem;
}

//-----------------------------------------------------------------------------------------------------------------

// filter paint messages
static void _ProcessPaint()
{
	MSG msg;
	while(PeekMessage(&msg,0,WM_PAINT,WM_PAINT,PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

//-----------------------------------------------------------------------------------------------------------------

ReplayInfo * DlgBrowser::_ProcessReplay(const char *dir, CFileFind& finder)
{
	ReplayInfo *rep=0;
	CString tmpPath;
	ReplayInfo tmpRep;
	bool bAddToFile=true;
	bool bAddToMem=true;

	// filter paint messages
	_ProcessPaint();

	// ignore our "watch replay" file
	if(finder.GetFileName().CollateNoCase(WATCHFILE)==0) return 0;

	// do we have that replay already?
	char buffini[2048];
	BWChartDB::ReadEntry(BWChartDB::FILE_MAIN,dir,finder.GetFileName(),buffini,sizeof(buffini));
	if(buffini[0]!=0)
	{
		// load it from database
		if(!tmpRep.ExtractInfo(dir,finder.GetFileName(),buffini)) return 0;
		bAddToFile=false;
	}
	else  
	{
		// parse it
		CTime cdate;
		finder.GetCreationTime(cdate);
		if((bAddToMem=_LoadReplay(finder.GetFilePath(),cdate, tmpRep)))
		{
			if(tmpRep.m_playerCount>0) 
				m_addedReplays++;
		}
		else
		{
			// corrupted replay
			m_corrupted.Add(finder.GetFilePath());
		}
	}

	// add in memory
	if(bAddToMem && tmpRep.m_playerCount>0)
	{
		// add replay
		rep = _AddReplay(tmpRep);

		// insert in replay list file
		if(bAddToFile) 
			rep->Save(BWChartDB::FILE_MAIN);
	}

	return rep;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgBrowser::_BrowseReplays(const char *dir, int& count, int &idx, bool bCount)
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
		if(finder.IsDirectory() && m_recursive)
		{
			// . & ..
			if(finder.IsDots()) continue;

			// skip corrupted
			if(finder.GetFileName()==CORRUPTED_DIR) continue;

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
			// process replay
			_ProcessReplay(dir,finder);

			// update progress bar
			idx++;
			if(m_progDlg!=0) m_progDlg->m_progress.SetPos((100*idx)/count);
			else m_progress.SetPos((100*idx)/count);
		}
	}
}


//--------------------------------------------------------------------------------------------------------------

bool DlgBrowser::_InsertReplayVirtual(ReplayInfo *rep, const DlgFilter* filter)
{
	// check filter
	if(filter!=0)
	{
		// rwa only?
		if(filter->m_rwaOnly && !rep->m_isRWA) return false;

		// filter match up
		int mu = rep->GetMatchup();
		if(filter->m_muT || filter->m_muZ || filter->m_muP)
		{
			if(!filter->m_muT && (mu==ReplayInfo::MU_TvT || mu==ReplayInfo::MU_TvZ || mu==ReplayInfo::MU_PvT)) return false;
			if(!filter->m_muZ && (mu==ReplayInfo::MU_TvZ || mu==ReplayInfo::MU_PvZ || mu==ReplayInfo::MU_ZvZ)) return false;
			if(!filter->m_muP && (mu==ReplayInfo::MU_PvT || mu==ReplayInfo::MU_PvZ || mu==ReplayInfo::MU_PvP)) return false;
			if(!filter->m_muXvX && (mu==ReplayInfo::MU_PvP || mu==ReplayInfo::MU_ZvZ || mu==ReplayInfo::MU_TvT)) return false;
		}

		// filter player name
		if(filter->m_pfilteron)
		{
			int found=0;
			for(int k=0;found<filter->m_filterPlayerCount && k<rep->m_playerCount;k++)
				if(filter->MatchPlayerName(rep->m_mainName[k]))
					found++;
			if(found!=filter->m_filterPlayerCount) return false;
		}

		// filter map name
		if(filter->m_mapFilterOn)
		{
			if(!filter->MatchMapName(rep->m_mainMap)) 
				return false;
		}

		// filter positions
		if(filter->m_posfilteron)
		{
			// for 1v1 only
			if(rep->m_playerCount!=2) return false;

			DlgFilterPosition pos(rep->m_start[0],rep->m_start[1]);
			bool keep=false;
			for(int k=0;!keep && k<filter->m_filterPositionsCount;k++)
				if(pos==filter->m_filterPositions[k])
					keep=true;
			if(!keep) return false;
		}

		// filter bos
		if(filter->m_boFilterOn)
		{
			// for each selected bo
			int options = _GetBOOptions();
			POSITION pos = m_listBos.GetFirstSelectedItemPosition();
			bool found = false;
			while (!found && pos)
			{
				// get bo
				int nItem = m_listBos.GetNextSelectedItem(pos);
				BuildOrder *pbo = (BuildOrder *)m_filteredBos.GetAt(nItem);

				// convert to string
				CString strbo;
				pbo->m_bo.ToString(strbo,false,options);

				// for each player in the replay
				int k=0;
				
				for(; !found && k<rep->m_playerCount; k++)
				{
					// get player build order
					BONodeList boplayer;
					boplayer.FromString(rep->m_bo[k]);
					// filter it out according to current options
					CString strborep;
					boplayer.ToString(strborep,false,options);
					// compare with selected bo
					if(strncmp(strborep,strbo,strbo.GetLength())==0)
						found=true;
				}
			}
			// no player matches any of the selected build order
			if(!found) return false;
		}
	}

	// insert replay
	return true;
}

//--------------------------------------------------------------------------------------------------------------

// return true if bo matches the filter
bool DlgBrowser::_InsertBOVirtual(BuildOrder *pbo, const DlgFilter *filter)
{
	ReplayInfo *rep = pbo->m_rep;

	// check filter
	if(filter!=0)
	{
		// rwa only?
		if(filter->m_rwaOnly && !rep->m_isRWA) return false;

		// filter player race
		if(filter->m_race!=IStarcraftPlayer::RACE_RANDOM && rep->m_race[pbo->m_player] != filter->m_race)
			return false;

		// filter match up
		int oidx = pbo->m_player==0 ? 1 : 0;
		if(rep->m_race[oidx]==IStarcraftPlayer::RACE_TERRAN && !filter->m_bomuT) return false;
		if(rep->m_race[oidx]==IStarcraftPlayer::RACE_PROTOSS && !filter->m_bomuP) return false;
		if(rep->m_race[oidx]==IStarcraftPlayer::RACE_ZERG && !filter->m_bomuZ) return false;

		// filter player name
		if(filter->m_pfilteron)
		{
			if(!filter->MatchPlayerName(rep->m_mainName[pbo->m_player]))
				return false;
		}

		// filter map name
		if(filter->m_mapFilterOn)
		{
			if(!filter->MatchMapName(rep->m_mainMap)) 
				return false;
		}

		// filter positions
		if(filter->m_posfilteron)
		{
			int start = rep->m_start[pbo->m_player];
			bool keep=false;
			for(int k=0;!keep && k<filter->m_filterPositionsCount;k++)
				if(filter->m_filterPositions[k].m_p1 == start)
					keep=true;
			if(!keep) return false;
		}
	}

	// insert replay
	return true;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_InsertPlayerVirtual(PlayerInfo *ply, const DlgFilter *filter)
{
//	ASSERT(!(strncmp(ply->m_name,"Obi-1",5)==0));

	// check filter
	if(filter!=0)
	{
		// filter player name
		if(filter->m_pfilteron)
		{
			if(!filter->MatchPlayerName(ply->m_name)) 
				return;
		}
	}

	// insert player
	m_filterPlayers.Add(ply);
}

//--------------------------------------------------------------------------------------------------------------

// insert a line in the map list
void DlgBrowser::_InsertMap(MapInfo *map, int idx)
{
	if(idx==-1) idx = m_listMaps.GetItemCount();

	CString apm;
	int nPos = m_listMaps.InsertItem(idx,map->m_name, 0);
	apm.Format("%d",map->m_apmCount==0?0:map->m_apm/map->m_apmCount);
	m_listMaps.SetItemText(nPos,1,apm);
	m_listMaps.SetItemText(nPos,2,map->AvgDuration(apm));
	apm.Format("%d",map->m_games);
	m_listMaps.SetItemText(nPos,3,apm);
	m_listMaps.SetItemData(nPos,(DWORD)map);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_UpdateCounter()
{
	//m_replayCount.Format("%d replays",m_reps.GetItemCount());
	//m_playerCount.Format("%d players",m_listPlayers.GetItemCount());
	//m_mapCount.Format("%d maps",m_listMaps.GetItemCount());
	m_replayCount.Format("Content: %d replays, %d players, %d maps",m_reps.GetItemCount(),m_listPlayers.GetItemCount(),m_listMaps.GetItemCount());
	UpdateData(FALSE);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnRefresh() 
{
	m_progress.ShowWindow(SW_SHOW);
	Refresh(true);
	m_progress.ShowWindow(SW_HIDE);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_ClearAll() 
{
	// clear all
	m_filterReplays.RemoveAll();
	m_filterPlayers.RemoveAll();
	m_reps.DeleteAllItems();
	m_listPlayers.DeleteAllItems();
	m_listMaps.DeleteAllItems();
	m_replays.RemoveAll();
	m_players.RemoveAll();
	m_maps.RemoveAll();
	m_allPlayers.RemoveAll();
	m_allMaps.RemoveAll();
	m_addedReplays=0;
	m_corrupted.RemoveAll();
	m_bos.RemoveAll();
	m_filteredBos.RemoveAll();
}

//-----------------------------------------------------------------------------------------------------------------

void DlgBrowser::ProcessEntry(const char * section, const char *entry, const char *data, int percentage)
{
	// create replay
	ReplayInfo tmpRep;

	// load replay (reppath,repname,data must be in regular format)
	CString rdata(BWChartDB::ConverFromHex(data));
	tmpRep.ExtractInfo(section,entry,(char*)(const char*)rdata);

	// add replay
	if(tmpRep.m_playerCount>0) 
	{
		if(!m_recursive)
		{
			CString dir;
			tmpRep.Dir(dir);
			if(dir==m_rootdir) 
				_AddReplay(tmpRep);
		}
		else
		{
			_AddReplay(tmpRep);
		}
	}

	// update progress bar
	if(m_progDlg!=0) m_progDlg->m_progress.SetPos(percentage);
	else m_progress.SetPos(percentage);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::Load() 
{
	CWaitCursor wait;

	// clear all
	_ClearAll();

	// load entries from replay database
	DWORD now = GetTickCount();
	LoadFile(BWChartDB::FILE_MAIN);
	now = GetTickCount() - now;
	//CString debug;
	//debug.Format("%lu\r\n",now);
	//OutputDebugString(debug);

	// any replay?
	if(m_replays.GetSize()==0 && !m_bRefreshDone)
	{
		// want to build replay database?
		CString msg;
		msg.Format(IDS_EMPTYDB,(const char*)m_rootdir);
		bool dontask = AfxGetApp()->GetProfileInt("BROWSER","DONTASK",0)?true:false;
		UINT res = dontask ? IDNO : MessageBox(msg,0,MB_YESNOCANCEL);
		if(res==IDYES)
		{
			m_bDBLoaded=true;
			Refresh(true);
			return;
		}
		else if(res==IDCANCEL)
		{
			AfxGetApp()->WriteProfileInt("BROWSER","DONTASK",1);
		}
	}

	// do we have a BO file?
	if(!MAINWND->HasBOFile())
	{
		if(AfxMessageBox(IDS_BUILDBOFILE,MB_YESNO)==IDYES)
			_BuildBOFile();
	}

	// display players & maps
	_DisplayList();
	m_bRefreshDone=true;
	m_bDBLoaded=true;
	m_progress.SetPos(0);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_MoveCorruptedReplays()
{
	// create directory for corrupted replays
	CString cdir;
	_GetStarcraftPath(cdir);
	cdir+="maps\\replays\\"CORRUPTED_DIR"\\";
	UtilDir::CreatDirRecursive(cdir);

	// move files there
	for(int i=0;i<m_corrupted.GetSize();i++)
	{
		CString dst;
		dst = m_corrupted.GetAt(i);
		dst = dst.Mid(dst.ReverseFind('\\')+1);
		dst = cdir+dst;

		UtilDir::MakeFileWritable(dst);
		if(CopyFile(m_corrupted.GetAt(i),dst,FALSE))
		{
			UtilDir::MakeFileWritable(m_corrupted.GetAt(i));
			::DeleteFile(m_corrupted.GetAt(i));
		}
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::Refresh(bool bMsg) 
{
	CWaitCursor wait;

	// shall load the replay first?
	if(!m_bDBLoaded) {Load(); return;}

	UpdateData(TRUE);

	// save browse dir
	_Parameters(false);

	// clear all
	_ClearAll();

	// count available replays
	int count=0;
	int idx=0;
	_BrowseReplays(m_rootdir, count, idx, true);

	// load them
	_BrowseReplays(m_rootdir, count, idx, false);

	// display players & maps
	_DisplayList();
 	m_bRefreshDone=true;

	// clear replay object
	m_replay.Clear();

	// display message
	m_progress.ShowWindow(SW_HIDE);
	if((m_addedReplays>0 && bMsg) || (m_corrupted.GetSize()>0)) 
	{
		// regular database update message
		CString msg;
		if(m_addedReplays>0)
		{
			if(m_addedReplays==1)
				msg.Format(IDS_ONEREPLAYADDED,m_addedReplays);
			else
				msg.Format(IDS_MULREPLAYADDED,m_addedReplays);
		}

		// any corrupted replays?
		if(m_corrupted.GetSize()>0)
		{
			// build description
			CString warning;
			msg+="\r\n\r\n";
			warning.Format(IDS_CORRUPTED,m_corrupted.GetSize());
			for(int i=0;i<m_corrupted.GetSize();i++)
			{
				warning+="\r\n";
				warning+=m_corrupted.GetAt(i);
			}

			CString moverep;
			moverep.LoadString(IDS_MOREREPLAYS);
			warning+="\r\n\r\nDo you want to move those replays to a special folder so bwchart ignores them?";

			// do we want to move them away?
			msg+=warning;
			if(MessageBox(msg,"WARNING",MB_YESNO)==IDYES)
				_MoveCorruptedReplays();
		}
		else
			MessageBox(msg);
	}
}

//--------------------------------------------------------------------------------------------------------------

ReplayInfo* DlgBrowser::_FindReplay(const char *path) const
{
	for(int i=0; i<m_replays.GetSize(); i++)
	{
		ReplayInfo *rep = (ReplayInfo *)m_replays.GetAt(i);
		if(rep->m_path == path) return rep;
	}
	return 0;
}

PlayerInfo* DlgBrowser::_FindPlayer(const char *name) const
{
	for(int i=0; i<m_players.GetSize(); i++)
	{
		PlayerInfo *rep = (PlayerInfo *)m_players.GetAt(i);
		if(rep->m_name.CompareNoCase(name)==0) 
			return rep;
	}
	return 0;
}

bool DlgBrowser::_FindAllPlayer(const char *name) const
{
	for(int i=0; i<m_allPlayers.GetSize(); i++)
	{
		CString ply = (CString)m_allPlayers.GetAt(i);
		if(ply.CompareNoCase(name)==0) 
			return true;
	}
	return false;
}

MapInfo* DlgBrowser::_FindMap(const char *name)	const
{
	for(int i=0; i<m_maps.GetSize(); i++)
	{
		MapInfo *rep = (MapInfo *)m_maps.GetAt(i);
		if(rep->m_name.CompareNoCase(name)==0) 
			return rep;
	}
	return 0;
}

bool DlgBrowser::_FindAllMap(const char *name) const
{
	for(int i=0; i<m_allMaps.GetSize(); i++)
	{
		CString map = (CString)m_allMaps.GetAt(i);
		if(map.CompareNoCase(name)==0) 
			return true;
	}
	return false;
}

//------------------------------------------------------------------------------------

static bool gbAscendingBo=false;

int CALLBACK CompareBuildOrder(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int diff=0;
	BuildOrder *bo1 = (BuildOrder *)lParam1;
	BuildOrder *bo2 = (BuildOrder *)lParam2;

	switch(lParamSort)
	{
		// count
		case 0 :
			diff = bo1->m_count - bo2->m_count;
			break;
		// percent
		case 1 :
			diff = bo1->m_percent - bo2->m_percent;
			break;
		// content
		case 2:
			diff = _stricmp(bo1->m_desc,bo2->m_desc);
			break;
	}

	return gbAscendingBo ? diff : -diff;
}

//------------------------------------------------------------------------------------

static bool gbAscendingPl=false;

int CALLBACK ComparePlayer(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int diff=0;
	CString str1;
	CString str2;
	PlayerInfo *rep1 = (PlayerInfo *)lParam1;
	PlayerInfo *rep2 = (PlayerInfo *)lParam2;

	// make sure the TOTALS are always at the end
	if(rep1==0) return 1;
	if(rep2==0) return -1;

	switch(lParamSort)
	{
		// player name
		case 0 :
			diff = _stricmp(rep1->m_name,rep2->m_name);
			break;
		// player  average apm
		case 1:
			diff = rep1->AvgApm() - rep2->AvgApm();
			break;
		// player  average apm dev
		case 2:
			diff = rep1->GetApmDev() - rep2->GetApmDev();
			break;
		// player  average apm dev in %
		case 3:
			diff = rep1->GetApmDevPer() - rep2->GetApmDevPer();
			break;
		// player average duration
		case 4:
			diff = rep1->AvgDur() - rep2->AvgDur();
			break;
		// games count
		case 5 :
			diff = rep1->m_games - rep2->m_games;
			break;
		// Race distribution
		case 6 : case 7 : case 8 :
			diff = rep1->GetRaceCount(lParamSort-6) - rep2->GetRaceCount(lParamSort-6);
			break;
		// player  average apm as T
		case 9:				    
			diff = rep1->AvgApmAsT() - rep2->AvgApmAsT();
			break;
		// player  average apm as Z
		case 10:				    
			diff = rep1->AvgApmAsZ() - rep2->AvgApmAsZ();
			break;
		// player  average apm as P
		case 11:				    
			diff = rep1->AvgApmAsP() - rep2->AvgApmAsP();
			break;
		default:
			assert(0);
			break;
	}

	return gbAscendingPl ? diff : -diff;
}

//------------------------------------------------------------------------------------

static bool gbAscendingMap=false;

int CALLBACK CompareMap(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int diff=0;
	CString str1;
	CString str2;
	MapInfo *rep1 = (MapInfo *)lParam1;
	MapInfo *rep2 = (MapInfo *)lParam2;

	// make sure the TOTALS are always at the end
	if(rep1==0) return 1;
	if(rep2==0) return -1;

	switch(lParamSort)
	{
		// map name
		case 0 :
			diff = _stricmp(rep1->m_name,rep2->m_name);
			break;
		// map  average apm
		case 1:
			diff = rep1->AvgApm() - rep2->AvgApm();
			break;
		// map average duration
		case 2:
			diff = rep1->AvgDur() - rep2->AvgDur();
			break;
		// map count
		case 3 :
			diff = rep1->m_games - rep2->m_games;
			break;
		default:
			assert(0);
			break;
	}

	return gbAscendingMap ? diff : -diff;
}

//------------------------------------------------------------------------------------

bool gbAscending=false;

int CALLBACK CompareReplay(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int diff=0;
	CString str1;
	CString str2;
	ReplayInfo *rep1 = (ReplayInfo *)lParam1;
	ReplayInfo *rep2 = (ReplayInfo *)lParam2;

	// make sure the TOTALS are always at the end
	if(rep1==0) return 1;
	if(rep2==0) return -1;

	switch(lParamSort)
	{
		// file name
		case 0 :
			diff = _stricmp(rep1->Name(),rep2->Name());
			break;
		// player 1 name
		case 1 :
			diff = _stricmp(rep1->m_mainName[0],rep2->m_mainName[0]);
			break;
		// player 1 apm
		case 2:
			diff = rep1->m_apm[0] - rep2->m_apm[0];
			break;
		// player 2 name
		case 3 :
			diff = _stricmp(rep1->m_mainName[1],rep2->m_mainName[1]);
			break;
		// player 2 apm
		case 4:
			diff = rep1->m_apm[1] - rep2->m_apm[1];
			break;
		// map name
		case 5 :
			diff = _stricmp(rep1->m_mainMap,rep2->m_mainMap);
			break;
		// duration
		case 6:
			diff = rep1->m_duration - rep2->m_duration;
			break;
		// game type
		case 7 :
			diff = strcmp(rep1->GameType(),rep2->GameType());
			break;
		// RWA author
		case 8 :
			diff = _stricmp(rep1->m_author,rep2->m_author);
			break;
		// game date
		case 9 :
			diff = strcmp(rep1->DateForCompare(str1),rep2->DateForCompare(str2));
			break;
		// engine
		case 10 :
			{
			long val1 = ((long)rep1->m_engineType)<<8 | rep1->m_engineVersion;
			long val2 = ((long)rep2->m_engineType)<<8 | rep2->m_engineVersion;
			diff = val1 - val2;
			}
			break;
		// comment
		case 11 :
			diff = _stricmp(rep1->m_comment,rep2->m_comment);
			break;
		// directory
		case 12 :
			{
			CString dir1,dir2;
			diff = _stricmp(rep1->Dir(dir1),rep2->Dir(dir2));
			}
			break;
		// file date
		case 13 :
			diff = strcmp(rep1->FileDate(str1),rep2->FileDate(str2));
			break;
		// player 1 starting location
		case 14:
			diff = rep1->m_start[0] - rep2->m_start[0];
			break;
		// player 2 starting location
		case 15:
			diff = rep1->m_start[1] - rep2->m_start[1];
			break;
		// hack count
		case 16:
			diff = rep1->m_hackCount - rep2->m_hackCount;
			break;
		default:
			assert(0);
			break;
	}

	return gbAscending ? diff : -diff;
}

//------------------------------------------------------------------------------------

static LPARAM lParamSort=0;

int QCompareReplay( const void *arg1, const void *arg2 )
{
	return CompareReplay((LPARAM)*(CObject**)arg1, (LPARAM)*(CObject**)arg2, lParamSort);
}

// sort list
void DlgBrowser::_SortReplay(int item, bool reverse)
{

	// if we are sorting again on current sorting column
	if(item==m_currentSortIdx)
	{
		// revert sorting order
		if(reverse) m_Descending[m_currentSortIdx]=!m_Descending[m_currentSortIdx];
	}
	else
	{
		// new sorting column
		if(item>=0) m_currentSortIdx = item;
	}

	// sort items
	gbAscending = !m_Descending[m_currentSortIdx];
	//m_reps.SortItems(CompareReplay,m_currentSortIdx);
	lParamSort = m_currentSortIdx;
	qsort(m_filterReplays.GetData(),	m_filterReplays.GetSize(), sizeof(CObject*),QCompareReplay);
}

//------------------------------------------------------------------------------------

static LPARAM lParamSortPl=0;

int QComparePlayer( const void *arg1, const void *arg2 )
{
	return ComparePlayer((LPARAM)*(CObject**)arg1, (LPARAM)*(CObject**)arg2, lParamSortPl);
}

// sort players list
void DlgBrowser::_SortPlayer(int item, bool reverse)
{
	// if we are sorting again on current sorting column
	if(item==m_currentSortIdx2)
	{
		// revert sorting order
		if(reverse) m_Descending2[m_currentSortIdx2]=!m_Descending2[m_currentSortIdx2];
	}
	else
	{
		// new sorting column
		if(item>=0) m_currentSortIdx2 = item;
	}

	// sort items
	gbAscendingPl = !m_Descending2[m_currentSortIdx2];
	//m_listPlayers.SortItems(ComparePlayer,m_currentSortIdx2);
	lParamSortPl = m_currentSortIdx2;
	qsort(m_filterPlayers.GetData(), m_filterPlayers.GetSize(), sizeof(CObject*),QComparePlayer);
}

//------------------------------------------------------------------------------------

// sort list
void DlgBrowser::_SortMap(int item, bool reverse)
{
	// if we are sorting again on current sorting column
	if(item==m_currentSortIdx3)
	{
		// revert sorting order
		if(reverse) m_Descending3[m_currentSortIdx3]=!m_Descending3[m_currentSortIdx3];
	}
	else
	{
		// new sorting column
		if(item>=0) m_currentSortIdx3 = item;
	}

	// sort items
	gbAscendingMap = !m_Descending3[m_currentSortIdx3];
	m_listMaps.SortItems(CompareMap,m_currentSortIdx3);
}


//------------------------------------------------------------------------------------

static LPARAM lParamSortBo=0;

int QCompareBO( const void *arg1, const void *arg2 )
{
	return CompareBuildOrder((LPARAM)*(CObject**)arg1, (LPARAM)*(CObject**)arg2, lParamSortBo);
}

// sort build order list
void DlgBrowser::_SortBuildOrder(int item, bool reverse)
{
	// if we are sorting again on current sorting column
	if(item==m_currentSortIdx4)
	{
		// revert sorting order
		if(reverse) m_Descending4[m_currentSortIdx4]=!m_Descending4[m_currentSortIdx4];
	}
	else
	{
		// new sorting column
		if(item>=0) m_currentSortIdx4 = item;
	}

	// sort items
	gbAscendingBo = !m_Descending4[m_currentSortIdx4];
	lParamSortBo = m_currentSortIdx4;
	qsort(m_filteredBos.GetData(), m_filteredBos.GetSize(), sizeof(CObject*),QCompareBO);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnColumnclickBos(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMLV = (NM_LISTVIEW*)pNMHDR;
	_SortBuildOrder(pNMLV->iSubItem);	
	m_sortbo=pNMLV->iSubItem;
	*pResult = 0;

	// repaint list control
	m_listBos.Invalidate();
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnColumnclickReps(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMLV = (NM_LISTVIEW*)pNMHDR;
	_SortReplay(pNMLV->iSubItem);	
	m_sortrep=pNMLV->iSubItem;
	*pResult = 0;

	// repaint list control
	m_reps.Invalidate();
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnColumnclickPlayers(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMLV = (NM_LISTVIEW*)pNMHDR;
	_SortPlayer(pNMLV->iSubItem);	
	m_sortpla=pNMLV->iSubItem;
	*pResult = 0;

	// repaint list control
	m_listPlayers.Invalidate();
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnColumnclickMaps(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMLV = (NM_LISTVIEW*)pNMHDR;
	_SortMap(pNMLV->iSubItem);	
	m_sortmap=pNMLV->iSubItem;
	*pResult = 0;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnRclickReps(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CPoint pt;
	GetCursorPos(&pt);

	// select only one replay
	//for(int i=0;i<m_reps.GetItemCount();i++)
	//	m_reps.SetItemState(i,0, LVIS_SELECTED+LVIS_FOCUSED);

	// find corresponding item
	m_reps.ScreenToClient(&pt);
	UINT uFlags;
	int nItem = m_reps.HitTest(pt,&uFlags);
	if(nItem!=-1 && (uFlags & LVHT_ONITEMLABEL)!=0)
	{
		// select item
		//m_reps.SetItemState(nItem,LVIS_SELECTED+LVIS_FOCUSED, LVIS_SELECTED+LVIS_FOCUSED);

		// load menu
		CMenu menu;
		if(m_reps.GetSelectedCount()>1)
			menu.LoadMenu(IDR_POPUPREPLAYMULT);
		else
			menu.LoadMenu(IDR_POPUPREPLAY);
		CMenu *pSub = menu.GetSubMenu(0);

		// display popup menu
		m_reps.ClientToScreen(&pt);
		pSub->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}
	
	*pResult = 0;
}

//----------------------------------------------------------------------------------------

void DlgBrowser::_DoReplay(UINT msg, int param)
{
	if(m_selectedReplay!=0)
	{
		// replay file still exist?
		if(_access(m_selectedReplay->m_path,0)!=0)
		{
			CString msg;
			msg.Format(IDS_REMOVEREP,(const char*)m_selectedReplay->m_path);
			if(MessageBox(msg,0,MB_YESNO)==IDYES)
			{
				CString tmpDir;
				BWChartDB::Delete(BWChartDB::FILE_MAIN,m_selectedReplay->Dir(tmpDir),m_selectedReplay->Name());
				Load();
			}
			return;
		}
		// do action
		AfxGetMainWnd()->PostMessage(msg,(WPARAM)m_selectedReplay,param);
	}
}

//----------------------------------------------------------------------------------------

void DlgBrowser::OnLoadReplay() 
{
	_DoReplay(WM_LOADREPLAY,1);
}

//----------------------------------------------------------------------------------------

void DlgBrowser::OnAddReplayEvents() 
{
	_DoReplay(WM_LOADREPLAY,0);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnAddToFavorites()
{
	POSITION pos = m_reps.GetFirstSelectedItemPosition();
	while(pos!=0) 
	{
		int nItem = (m_reps.GetNextSelectedItem(pos));
		_SelectedItem(nItem);
		_DoReplay(WM_ADDFAVORITE,0);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnDblclkReps(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CPoint pt;
	GetCursorPos(&pt);

	// select only one replay
	for(int i=0;i<m_reps.GetItemCount();i++)
		m_reps.SetItemState(i,0, LVIS_SELECTED+LVIS_FOCUSED);

	// find corresponding item
	m_reps.ScreenToClient(&pt);
	UINT uFlags;
	int nItem = m_reps.HitTest(pt,&uFlags);
	if(nItem!=-1 && (uFlags & LVHT_ONITEMLABEL)!=0)
	{
		// select item
		m_reps.SetItemState(nItem,LVIS_SELECTED+LVIS_FOCUSED, LVIS_SELECTED+LVIS_FOCUSED);
		// load replay
		_DoReplay(WM_LOADREPLAY,1);
	}
	
	*pResult = 0;
}

//---------------------------------------------------------------------------------------

ReplayInfo * DlgBrowser::_GetReplayFromIdx(unsigned long idx) const
{
	ReplayInfo *rep = (ReplayInfo *)m_filterReplays.GetAt(idx);
	assert(rep!=0);
	return rep;
}

//---------------------------------------------------------------------------------------

PlayerInfo * DlgBrowser::_GetPlayerFromIdx(unsigned long idx) const
{
	PlayerInfo *ply = (PlayerInfo *)m_filterPlayers.GetAt(idx);
	assert(ply!=0);
	return ply;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_SelectedItem(int nItem)
{
	// get selected item
	m_selectedReplay = _GetReplayFromIdx(nItem);
	m_idxSelectedReplay = nItem;
	// display comment
	if(m_selectedReplay!=0) SetDlgItemText(IDC_COMMENT,m_selectedReplay->m_comment);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_SelectedPlayer(int nItem)
{
	// get selected item
	m_selectedPlayer = _GetPlayerFromIdx(nItem);
	m_idxSelectedPlayer = nItem;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnItemchangedReps(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if(pNMListView->uNewState&(LVIS_SELECTED+LVIS_FOCUSED))
	{
		// select item
		POSITION pos = m_reps.GetFirstSelectedItemPosition();
		if(pos!=0) _SelectedItem(m_reps.GetNextSelectedItem(pos));
	}
	*pResult=0;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnEditcomment() 
{
	if(m_selectedReplay==0) return;

	// open dialog
	DlgEditComment dlg(m_selectedReplay, this);	
	if(dlg.DoModal()==IDOK)
	{
		// save replay in database
		m_selectedReplay->Save(BWChartDB::FILE_MAIN);
		// update also favorites database
		//???????????
		// update list
		m_reps.Invalidate();
		// update bottom comment
		SetDlgItemText(IDC_COMMENT,m_selectedReplay->m_comment);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnAutorefresh() 
{
	// auto refresh
	UpdateData(TRUE);
}

//--------------------------------------------------------------------------------------------------------------

LRESULT DlgBrowser::OnMsgAutoRefresh(WPARAM , LPARAM )
{
	if(!OPTIONS->m_autoLoadDB) return 0L;

	// filter messages
	MSG msg;
	while(PeekMessage(&msg,0,0,0,PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// open progress window
	m_progDlg = new ProgressDlg(this); assert(m_progDlg!=0);
	m_progDlg->Create(ProgressDlg::IDD,this);
	m_progDlg->ShowWindow(SW_SHOW);

	// refresh replays
	Load();

	// destroy progress window
	m_progDlg->DestroyWindow();
	delete m_progDlg;
	m_progDlg=0;

	return 0L;
}

//------------------------------------------------------------------------------------

// launch a different process
int DlgBrowser::StartProcess(const char *pszExe, const char *pszCmdLine, const char *pszCurrentDir, bool bHideWindow)
{
	int nvErr=-1;
	char *pszTmpCmdLine;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

	// init structures
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	if(bHideWindow)
	{
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
	}

	// make a tmp copy of the command line because CreateProcess will modify it
	pszTmpCmdLine = new char[(strlen(pszExe)+ (pszCmdLine!=0 ? strlen(pszCmdLine) : 0) + 4)];
	if(pszTmpCmdLine == 0) return -1;
	strcpy(pszTmpCmdLine,"\"");
	strcat(pszTmpCmdLine,pszExe);
	strcat(pszTmpCmdLine,"\" ");
	if(pszCmdLine!=0) strcat(pszTmpCmdLine,pszCmdLine);

	// start process
	if(CreateProcess(NULL,pszTmpCmdLine,NULL,NULL,FALSE,0,NULL,pszCurrentDir,&si,&pi))
	{
		// give 5s to start
		WaitForInputIdle(pi.hProcess,5000);

	    // Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		nvErr=0;
	}

	// delete temporary command line
	if(pszTmpCmdLine!=0) delete []pszTmpCmdLine;

	return nvErr;
}

//--------------------------------------------------------------------------------------------------------------

HWND DlgBrowser::_WaitForProcess(const char *wndclassname, int timeoutS)
{
	// wait for it to open (5 s time out)
	DWORD now; now = GetTickCount();
	HWND hWnd=0;
	while(hWnd==0 && (GetTickCount()-now)<(unsigned long)timeoutS*1000)
	{
		hWnd = ::FindWindow(wndclassname,0);

		// filter messages
		MSG msg;
		while(PeekMessage(&msg,0,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return hWnd;
}

//--------------------------------------------------------------------------------------------------------------

HWND DlgBrowser::_StartBW(ReplayInfo* selectedReplay, int version, CString& exe) 
{
	CString dir;

	switch(version)
	{
		case BW_AUTO:
			if(selectedReplay->m_engineType==IStarcraftGame::ENGINE_BROODWAR && selectedReplay->m_engineVersion<BWREP_VERSION_110)
			{
				// use bw 1.09 exe
				exe = OPTIONSW->m_bwexe109;
			}
			else if(version==BW_110 || (selectedReplay->m_engineType==IStarcraftGame::ENGINE_BROODWAR && selectedReplay->m_engineVersion<BWREP_VERSION_111))
			{
				// build path to starcraft 1.10
				exe = OPTIONSW->m_bwexe110;
			}
			else if(version==BW_111 || (selectedReplay->m_engineType==IStarcraftGame::ENGINE_BROODWAR && selectedReplay->m_engineVersion<BWREP_VERSION_112))
			{
				// build path to starcraft 1.11
				exe = OPTIONSW->m_bwexe111;
			}
			else if(version==BW_112 || (selectedReplay->m_engineType==IStarcraftGame::ENGINE_BROODWAR && selectedReplay->m_engineVersion<BWREP_VERSION_113))
			{
				// build path to starcraft 1.12
				exe = OPTIONSW->m_bwexe112;
			}
			else if(version==BW_113 || (selectedReplay->m_engineType==IStarcraftGame::ENGINE_BROODWAR && selectedReplay->m_engineVersion<BWREP_VERSION_114))
			{
				// build path to starcraft 1.13
				exe = OPTIONSW->m_bwexe113;
			}
			else if(version==BW_114 || (selectedReplay->m_engineType==IStarcraftGame::ENGINE_BROODWAR && selectedReplay->m_engineVersion<BWREP_VERSION_115))
			{
				// build path to starcraft 1.14
				exe = OPTIONSW->m_bwexe114;
			}
			else if(version==BW_115 || (selectedReplay->m_engineType==IStarcraftGame::ENGINE_BROODWAR && selectedReplay->m_engineVersion<BWREP_VERSION_116))
			{
				// build path to starcraft 1.15
				exe = OPTIONSW->m_bwexe115;
			}
			else
			{
				// build path to starcraft 1.16
				exe = OPTIONSW->m_bwexe116;
			}
			break;
		case BW_109:
			// use bw 1.09 exe
			exe = OPTIONSW->m_bwexe109;
			break;
		case BW_110:
			// build path to starcraft 1.10
			exe = OPTIONSW->m_bwexe110;
			break;
		case BW_111:
			// build path to starcraft 1.11
			exe = OPTIONSW->m_bwexe111;
			break;
		case BW_112:
			// build path to starcraft 1.12
			exe = OPTIONSW->m_bwexe112;
			break;
		case BW_113:
			// build path to starcraft 1.13
			exe = OPTIONSW->m_bwexe113;
			break;
		case BW_114:
			// build path to starcraft 1.14
			exe = OPTIONSW->m_bwexe114;
			break;
		default:
			// build path to starcraft 1.15
			exe = OPTIONSW->m_bwexe115;
			break;
	}

	if(exe.IsEmpty())
	{
		AfxMessageBox(IDS_EMPTYBWPATH);
		return 0;
	}

	// search for BW window
	HWND hWnd = ::FindWindow("SWarClass",0);
	if(hWnd != 0) goto Activate;

	// extract dir
	dir = exe .Left(exe.ReverseFind('\\')+1);

	// start process
	if(StartProcess(exe, "", dir, false)!=0)
	{
		AfxMessageBox(IDS_CANTSTARTBW);
		return 0;
	}

	// wait for it to open (5 s time out)
	if((hWnd=_WaitForProcess("SWarClass", 5))==0)
	{
		AfxMessageBox(IDS_NOBWWINDOW);
		return 0;
	} 

	// wait for BW to load main scren
	if(OPTIONSW->m_autoKeys) Sleep(OPTIONSW->m_timeWait); 

Activate:
	// make BW the active window
	::SetForegroundWindow(hWnd);
	::SetActiveWindow(hWnd);
	::ShowWindow(hWnd,SW_SHOWMAXIMIZED);
	Sleep(500);
	return hWnd;
}

//------------------------------------------------------------------------------------

BOOL CALLBACK _enumproc(
  HWND hwnd,      // handle to parent window
  LPARAM lParam   // application-defined value
)
{
	char buffer[128];
	HWND *bwpfound = (HWND *)lParam;
	::GetWindowText(hwnd,buffer,sizeof(buffer));
	if(_strnicmp(buffer,"bwplayer ",9)==0) {*bwpfound=hwnd; return FALSE;}
	return TRUE;
}

static HWND _FindBWPlayerWindow()
{
	HWND bwpfound=0;
	EnumWindows(_enumproc,(LPARAM)&bwpfound);
	return bwpfound;
}

//--------------------------------------------------------------------------------------------------------------

// if replay a RWA?
bool DlgBrowser::_IsRWA(const char *path) const
{
	if(_strnicmp(path,"RWA_",4)==0) return true;

	// if replay big enough to be RWA?
	FILE *fp = fopen(path,"rb");
	if(fp==0) return false;
	fseek(fp,0,SEEK_END);
	long size= ftell(fp);
	fclose(fp);
	return (size>=700000);
}

//--------------------------------------------------------------------------------------------------------------

// start BWPLayer if needed
void DlgBrowser::_StartRWA(const char *path, int version)
{
	// auto start RWA?
	if(!OPTIONSW->m_autoStartRWA || version<=BW_109) return;

	// if replay big enough to be RWA?
	if(!_IsRWA(path)) return;

	// start BWPlayer
	version -= BW_110;
	HWND hwnd;
	if((hwnd=_FindBWPlayerWindow())==0) 
	{
		CString cmdline;
		cmdline.Format("version=%d",version);
		StartProcess(OPTIONSW->m_bwplayer, cmdline, 0, false);
	}
	else
	{
		::PostMessage(hwnd,WM_BWPLAYERVERSION,(WPARAM)version,0);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::StartReplay(ReplayInfo* selectedReplay, int version) 
{
	CWaitCursor wait;
	CString dstrep;
	CString bwexe;
	int i;

	if(selectedReplay==0) return;
			 
	// start BWPLayer if needed
	_StartRWA(selectedReplay->m_path,version);

	// start BW
	HWND hWnd = _StartBW(selectedReplay, version, bwexe);
	//if(hWnd==0) return;

	// update replay version
	if(version == BW_109)
		_UpdateReplayVersion(selectedReplay,BWREP_VERSION_109);
	else if(version == BW_110)
		_UpdateReplayVersion(selectedReplay,BWREP_VERSION_110);
	else if(version == BW_111)
		_UpdateReplayVersion(selectedReplay,BWREP_VERSION_111);
	else if(version == BW_112)
		_UpdateReplayVersion(selectedReplay,BWREP_VERSION_112);
	else if(version == BW_113)
		_UpdateReplayVersion(selectedReplay,BWREP_VERSION_113);
	else if(version == BW_114)
		_UpdateReplayVersion(selectedReplay,BWREP_VERSION_114);
	else if(version == BW_115)
		_UpdateReplayVersion(selectedReplay,BWREP_VERSION_115);
	else if(version == BW_116)
		_UpdateReplayVersion(selectedReplay,BWREP_VERSION_116);

	// build path to maps
	if(!bwexe.IsEmpty()) bwexe = bwexe .Left(bwexe.ReverseFind('\\')+1);
	if(bwexe.IsEmpty()) goto Error;

	// create bwchart directory for replay
	bwexe+="maps\\replays\\!bwchart";
	UtilDir::CreatDirRecursive(bwexe);

	//copy the replay we want to see
	dstrep =bwexe+"\\"WATCHFILE;
	UtilDir::MakeFileWritable(dstrep);
	if(!CopyFile(selectedReplay->m_path,dstrep,FALSE))
	{
		AfxMessageBox(IDS_CANTCOPY);
		goto Error;
	}
	
	// send keystrokes to watch replay
	if(OPTIONSW->m_autoKeys && hWnd!=0)
	{
		const char *keys; keys = (version==BW_AUTO && selectedReplay->m_engineType==IStarcraftGame::ENGINE_STARCRAFT) || version==BW_SC ? OPTIONSW->m_keyseq2 : OPTIONSW->m_keyseq;
		int wait; wait=OPTIONSW->m_timeWait2;
		for(i=0;i<(int)strlen(keys);i++)
		{
			if(i>0) Sleep(wait);
			WPARAM c=(WPARAM)keys[i];
			if(c=='*') {c=VK_DOWN; wait=250;}
			else if(c=='?') {c=VK_UP; wait=250;}
			else if(c=='$') {c=VK_HOME; wait=250;}
			else if(c=='=') {wait=250;}
			else wait=OPTIONSW->m_timeWait2;
			if(c=='=')
			{
				int x=550, y=400;
				::PostMessage(hWnd,WM_LBUTTONDOWN,0,MAKELPARAM(x,y));
				::PostMessage(hWnd,WM_LBUTTONUP,0,MAKELPARAM(x,y));
			}
			else
			{
				::PostMessage(hWnd,WM_KEYDOWN,c,0);
				::PostMessage(hWnd,WM_KEYUP,c,0);
			}
		}
	}
	return;

Error:
	// make us the active window
	::SetForegroundWindow(AfxGetMainWnd()->GetSafeHwnd());
	::SetActiveWindow(AfxGetMainWnd()->GetSafeHwnd());
	::ShowWindow(AfxGetMainWnd()->GetSafeHwnd(),SW_SHOW);
	return;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_GetStarcraftPath(CString& path) 
{
	DlgOptions::_GetStarcraftPath(path);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_UpdateReplayVersion(ReplayInfo* selectedReplay, int version)
{
	selectedReplay->m_engineVersion = version;
	// save replay in database
	selectedReplay->Save(BWChartDB::FILE_MAIN);
	// update also favorites database
	if(selectedReplay->IsFavorite()) 
	{
		selectedReplay->Save(BWChartDB::FILE_FAVORITES);
		MAINWND->pGetFavorites()->Refresh();
	}
	// update list
	m_reps.Invalidate();
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnWatchReplay111()
{
	StartReplay(m_selectedReplay, BW_111);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnWatchReplay112()
{
	StartReplay(m_selectedReplay, BW_112);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnWatchReplay113()
{
	StartReplay(m_selectedReplay, BW_113);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnWatchReplay114()
{
	StartReplay(m_selectedReplay, BW_114);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnWatchReplay115()
{
	StartReplay(m_selectedReplay, BW_115);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnWatchReplay116()
{
	StartReplay(m_selectedReplay, BW_116);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnWatchReplay110()
{
	StartReplay(m_selectedReplay, BW_110);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnWatchReplay109()
{
	StartReplay(m_selectedReplay, BW_109);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnWatchReplaySC()
{
	StartReplay(m_selectedReplay, BW_SC);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnWatchReplay()
{
	StartReplay(m_selectedReplay,BW_AUTO);
}

//--------------------------------------------------------------------------------------------------------------

// add replay in database
bool DlgBrowser::AddReplay(const char *reppath, bool msg, bool display) 
{
	CFileFind finder;
	BOOL bWorking = finder.FindFile(reppath);
	if(!bWorking) return false;

	// get replay file info
	finder.FindNextFile();

	// do we have that replay already?
	char buffini[2048];
	BWChartDB::ReadEntry(BWChartDB::FILE_MAIN,finder.GetRoot(),finder.GetFileName(),buffini,sizeof(buffini));
	if(buffini[0]!=0)
	{
		// remove it 
		ReplayInfo *existingRep = _FindReplay(reppath);
		if(existingRep!=0)	_RemoveReplay(existingRep);
	}

	// process replay
	ReplayInfo *rep = _ProcessReplay(finder.GetRoot(),finder);
	if(rep==0) return false;

	_DisplayList();

	// display
	if(display)
	{
		m_selectedReplay = rep;
		OnLoadReplay();
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnAddreplay() 
{
	static char BASED_CODE szFilter[] = "Replay (*.rep)|*.rep|All Files (*.*)|*.*||";

 	CFileDialog dlg(TRUE,"rep","",0,szFilter,this);
	//dlg.m_ofn.lpstrInitialDir = MusicDB::GetNetCastDir();
	if(dlg.DoModal()==IDOK)
	{
		AddReplay(dlg.GetPathName(),true,false); 
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnUpdateAkas()
{
	if(m_selectedReplay==0) return;
	DlgFastAka dlg(m_selectedReplay);
	dlg.DoModal();
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::LoadColumns(CListCtrl *list, const char *name)
{
	// get column count
	int nColumnCount =0;
	CHeaderCtrl* pHeaderCtrl = list->GetHeaderCtrl();
	if(pHeaderCtrl != NULL) nColumnCount = pHeaderCtrl->GetItemCount();

	// load column width
	int nOrder[32];
	for(int i=0;i<nColumnCount;i++)
	{
		CString entry;
		entry.Format("%s_col_%d",name,i+1);
		int w = AfxGetApp()->GetProfileInt("LISTCTRL",entry,-1);
		if(w>0) list->SetColumnWidth(i,w);
		entry.Format("%s_ord_%d",name,i+1);
		nOrder[i] = AfxGetApp()->GetProfileInt("LISTCTRL",entry,i);
	}

	// update column order
	list->SetColumnOrderArray(nColumnCount, nOrder);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::SaveColumns(CListCtrl *list, const char *name)
{
	// get column count
	int nColumnCount =0;
	CHeaderCtrl* pHeaderCtrl = list->GetHeaderCtrl();
	if(pHeaderCtrl != NULL) nColumnCount = pHeaderCtrl->GetItemCount();

	// get column ordering
	int nOrder[32];
	list->GetColumnOrderArray(nOrder, nColumnCount);

	// save column width
	for(int i=0;i<nColumnCount;i++)
	{
		CString entry;
		entry.Format("%s_col_%d",name,i+1);
		AfxGetApp()->WriteProfileInt("LISTCTRL",entry,list->GetColumnWidth(i));
		entry.Format("%s_ord_%d",name,i+1);
		AfxGetApp()->WriteProfileInt("LISTCTRL",entry,nOrder[i]);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnDestroy() 
{
	// save parameters
	UpdateData(TRUE);
	_Parameters(false);

	// save column width
	SaveColumns(&m_reps, "reps");
	SaveColumns(&m_listPlayers, "ply");
	SaveColumns(&m_listMaps, "map");

	CDialog::OnDestroy();
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_BuildFilter(DlgFilter& filter)
{
	// init filter
	filter.m_pfilteron = m_pfilteron;
	filter.m_posfilteron = m_posfilteron;
	filter.m_mapFilterOn = m_mapFilterOn;
	filter.m_boFilterOn = ((CButton*)GetDlgItem(IDC_FILTERONBO))->GetCheck()==BST_CHECKED;
	filter.m_muT = m_muT ? true : false;
	filter.m_muZ = m_muZ ? true : false;
	filter.m_muP = m_muP ? true : false;
	filter.m_muXvX = m_muXvX ? true : false;
	filter.m_rwaOnly = m_rwaOnly ? true:false;

	// bo player race
	int sel = m_bomu.GetCurSel();
	if(sel==CB_ERR) sel=0;
	filter.m_race = m_bomu.GetItemData(sel);
	filter.m_bomuT = m_bomuT ? true:false;
	filter.m_bomuP = m_bomuP ? true:false;
	filter.m_bomuZ = m_bomuZ ? true:false;

	// if we filter on positions
	if(filter.m_posfilteron)
	{
		// for each pair of positions
		char positions[255];
		strcpy(positions,m_filterPosition);
		char *p=strtok(positions,",");
		filter.m_filterPositionsCount=0;
		while(p!=0 && filter.m_filterPositionsCount<MAXPOSITIONFILTER)
		{
			// extract positions
			char *p2=strchr(p,'/');
			filter.m_filterPositions[filter.m_filterPositionsCount].m_p1 = atoi(p);
			filter.m_filterPositions[filter.m_filterPositionsCount].m_p2 = p2!=0 ? atoi(p2+1) : atoi(p);
			filter.m_filterPositionsCount++;
			p=strtok(0,",");
		}
	}

	// if we filter on player names
	if(filter.m_pfilteron)
	{
		// for each player names in the filter
		char *allnames = _strdup(m_filterPlayer);
		char *p=strtok(allnames,", ");
		filter.m_filterPlayerCount=0;
		while(p!=0 && filter.m_filterPlayerCount<MAXPLAYERFILTER)
		{
			// extract player name
			filter.m_filterPlayer[filter.m_filterPlayerCount]=p;
			filter.m_filterPlayer[filter.m_filterPlayerCount].MakeLower();
			filter.m_filterPlayerCount++;
			p=strtok(0,", ");
		}
		free(allnames);
	}

	// if we filter on map name
	if(filter.m_mapFilterOn)
	{
		// for each map name in the filter
		char *allnames = _strdup(m_filterMap);
		char *p=strtok(allnames,",");
		filter.m_filterMapCount=0;
		while(p!=0 && filter.m_filterMapCount<MAXPLAYERFILTER)
		{
			// extract map name
			filter.m_filterMap[filter.m_filterMapCount]=p;
			filter.m_filterMap[filter.m_filterMapCount].MakeLower();
			filter.m_filterMapCount++;
			p=strtok(0,",");
		}
		free(allnames);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_DisplayList(bool rebuildBos) 
{
	CWaitCursor wait;

	UpdateData(TRUE);

	// clear list control
	m_refreshReplays = false;
	m_filterReplays.RemoveAll();
	m_filterPlayers.RemoveAll();
	m_reps.DeleteAllItems();
	m_listPlayers.DeleteAllItems();
	m_listMaps.DeleteAllItems();
	if(rebuildBos) 
	{
		m_filteredBos.RemoveAll();
		m_listBos.DeleteAllItems();
	}
	
	// init filter
	DlgFilter filter;
	_BuildFilter(filter);

	// display reps
	for(int i=0; i<m_replays.GetSize(); i++)
	{
		ReplayInfo *rep=(ReplayInfo *)m_replays.GetAt(i);
		if(_InsertReplayVirtual(rep, &filter))
		  	m_filterReplays.Add(rep);
	}
	m_reps.SetItemCountEx(m_filterReplays.GetSize(), LVSICF_NOSCROLL|LVSICF_NOINVALIDATEALL);

	// display players
	for(int i=0; i<m_players.GetSize(); i++)
		_InsertPlayerVirtual((PlayerInfo *)m_players.GetAt(i), &filter);
	m_listPlayers.SetItemCountEx(m_filterPlayers.GetSize(), LVSICF_NOSCROLL|LVSICF_NOINVALIDATEALL);

	// display maps
	for(int i=0; i<m_maps.GetSize(); i++)
		_InsertMap((MapInfo *)m_maps.GetAt(i), i);

	// build tree of unique build orders (with counting)
	if(rebuildBos)
		_UpdateBuildOrder();

	// presort everything
	_SortReplay(m_sortrep,false);
	_SortPlayer(m_sortpla,false);
	_SortMap(m_sortmap,false);

	// update counters
	_UpdateCounter();
}

//--------------------------------------------------------------------------------------------------------------

// IBOLister implementation
void DlgBrowser::AddBO(BONodeList* bo, int count)
{
	BuildOrder *pbo = new BuildOrder(0,0);
	pbo->m_bo = *bo;
	bo->ToString(pbo->m_desc,true);
	pbo->m_count = count;
	m_filteredBos.Add(pbo);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnToggleFilter() 
{
	// filter activated or deactivated, update lists
	_DisplayList();
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_AskForRefresh(int timeMS) 
{
	// will refresh lists in 1 second
	if(m_reftimer!=0) KillTimer(m_reftimer);
	m_reftimer = SetTimer(1,timeMS,0);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnChangeFplayer() 
{
	UpdateData(TRUE);
	bool rebuild=false;
	// player name was erased?
	if(m_filterPlayer.IsEmpty())
	{
		if(m_pfilteron) {m_pfilteron=FALSE; rebuild=true;}
	}
	// player name was filled with something?
	else 
	{
		if(!m_pfilteron) m_pfilteron=TRUE; 
		rebuild=true;
	}
	if(rebuild) _AskForRefresh();
	UpdateData(FALSE);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnTimer(UINT nIDEvent) 
{
	//stop time
	KillTimer(m_reftimer);
	m_reftimer=0;
	// refresh lists
	_DisplayList();
	CDialog::OnTimer(nIDEvent);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnGetdispinfoReps(NMHDR* pNMHDR, LRESULT* pResult) 
{
	char *strRace[4]={"Z","T","P","U"};
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;
	CString player;
	CString apm;

	int iItemIndx= pItem->iItem;

	if (pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		ReplayInfo *rep = (ReplayInfo *)m_filterReplays.GetAt(iItemIndx);
		assert(rep!=0);

		switch(pItem->iSubItem)
		{
		case 0:
			strcpy(pItem->pszText,rep->Name());
			break;
		case 1:
			// player 1
			if(!rep->m_player[0].IsEmpty()) player.Format("%s (%s)",rep->m_mainName[0],strRace[rep->m_race[0]]);
			strcpy(pItem->pszText,player);
			break;

		case 2:
			// apm1
			apm.Format("%d",rep->m_apm[0]);
			strcpy(pItem->pszText,apm);
			break;

		case 3:
			// Player 2
			player="";
			if(!rep->m_player[1].IsEmpty()) player.Format("%s (%s)",rep->m_mainName[1],strRace[rep->m_race[1]]);
			strcpy(pItem->pszText,player);
			break;

		case 4:
			// apm2
			apm.Format("%d",rep->m_apm[1]);
			strcpy(pItem->pszText,apm);
			break;

		case 5:
			// map name
			strcpy(pItem->pszText,rep->m_mainMap);
			break;

		case 6:
			// duration
			strcpy(pItem->pszText,rep->Duration(apm));
			break;

		case 7:
			// game type
			strcpy(pItem->pszText,rep->GameType());
			break;

		case 8:
			// RWA author
			strcpy(pItem->pszText,rep->m_author);
			break;

		case 9:
			// game date
			strcpy(pItem->pszText,rep->DateForDisplay(apm));
			break;

		case 10:
			// engine
			strcpy(pItem->pszText,rep->EngineVersion(apm));
			break;

		case 11:
			// comment
			strcpy(pItem->pszText,rep->m_comment);
			break;

		case 12:
			// path
			{
			CString tmpDir;
			strcpy(pItem->pszText,rep->Dir(tmpDir));
			}
			break;

		case 13:
			// file date
			strcpy(pItem->pszText,rep->m_filedate.Format("%d %b %Y"));
			break;

		case 14:
			// player 1 starting location
			apm.Format("%d",rep->m_start[0]);
			strcpy(pItem->pszText,apm);
			break;

		case 15:
			// player 2 starting location
			apm.Format("%d",rep->m_start[1]);
			strcpy(pItem->pszText,apm);
			break;

		case 16:
			// hack count
			apm.Format("%d",rep->m_hackCount);
			strcpy(pItem->pszText,apm);
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

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnOpenDirectory()
{
	if(m_selectedReplay!=0)
	{
		CString str;
		ShellExecute(0,"open",m_selectedReplay->Dir(str),"","",SW_SHOW);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnListReplays()
{
	if(m_selectedPlayer!=0)
	{
		// set up player filter
		m_pfilteron = TRUE;
		m_filterPlayer = m_selectedPlayer->m_name;
		m_groupby=0;
		UpdateData(FALSE);
		// display replay list
		OnRadio();
		_DisplayList();
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnItemchangedListplayers(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if(pNMListView->uNewState&(LVIS_SELECTED+LVIS_FOCUSED))
	{
		// select item
		POSITION pos = m_listPlayers.GetFirstSelectedItemPosition();
		if(pos!=0) _SelectedPlayer(m_listPlayers.GetNextSelectedItem(pos));
	}
	*pResult=0;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnRclickListplayers(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CPoint pt;
	GetCursorPos(&pt);

	// select only one player
	for(int i=0;i<m_listPlayers.GetItemCount();i++)
		m_listPlayers.SetItemState(i,0, LVIS_SELECTED+LVIS_FOCUSED);

	// find corresponding item
	m_listPlayers.ScreenToClient(&pt);
	UINT uFlags;
	int nItem = m_listPlayers.HitTest(pt,&uFlags);
	if(nItem!=-1 && (uFlags & LVHT_ONITEMLABEL)!=0)
	{
		// select item
		m_listPlayers.SetItemState(nItem,LVIS_SELECTED+LVIS_FOCUSED, LVIS_SELECTED+LVIS_FOCUSED);

		// load menu
		CMenu menu;
		menu.LoadMenu(IDR_POPPLAYERS);
		CMenu *pSub = menu.GetSubMenu(0);

		// display popup menu
		m_listPlayers.ClientToScreen(&pt);
		pSub->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}
	
	*pResult = 0;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_MemorizeSelection(CObArray& selection)
{
	// memorize selection
	selection.RemoveAll();
	POSITION pos = m_reps.GetFirstSelectedItemPosition();
	while(pos!=0) 
	{
		int nItem = (m_reps.GetNextSelectedItem(pos));
		_SelectedItem(nItem);
		selection.Add(m_selectedReplay);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnMoveToBin()
{
	// memorize selection
	CObArray selection;
	_MemorizeSelection(selection);

	// move all selected replays to bin
	for(int i=0;i<selection.GetSize();i++)
	{
		m_selectedReplay = (ReplayInfo*)selection.GetAt(i);
		_MoveReplayToBin();
	}

	// update list
	_DisplayList();
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnMoveToFolder()
{
	// get foldername
	CString folderName;
	if(m_lastFolder.IsEmpty()) m_selectedReplay->Dir(m_lastFolder);
	const char *path=UtilDir::BrowseDir("Select folder",m_lastFolder);
	if(path==0) return;
	m_lastFolder=path;

	// memorize selection
	CObArray selection;
	_MemorizeSelection(selection);

	// move all selected replays to other folder
	for(int i=0;i<selection.GetSize();i++)
	{
		m_selectedReplay = (ReplayInfo*)selection.GetAt(i);
		_MoveReplayToFolder(m_lastFolder);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnRenameReplays()
{
	m_noRepaint=true;
	DlgRename dlg(&m_reps,&m_filterReplays,this);
	if(dlg.DoModal()==IDOK)
		_DisplayList();
	m_noRepaint=false;
	m_reps.Invalidate();
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_MoveReplayToBin()
{
	if(m_selectedReplay==0) return;

	// make sure file can be deleted
	UtilDir::MakeFileWritable(m_selectedReplay->m_path);

	// build path name with double 0 at the end
	char buffer[255];
	strcpy(buffer,m_selectedReplay->m_path);
	buffer[strlen(buffer)+1]=0;

	// move file to bin
	SHFILEOPSTRUCT info;
	memset(&info,0,sizeof(info));
	info.wFunc = FO_DELETE;
	info.pFrom = buffer;
	info.fFlags = FOF_ALLOWUNDO;
	if(SHFileOperation(&info)!=0) return;

	// remove replay from database
	if(!info.fAnyOperationsAborted) 
		_RemoveReplay(m_selectedReplay);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_MoveReplayToFolder(const char *folder)
{
	if(m_selectedReplay==0) return;

	// make sure file can be deleted
	UtilDir::MakeFileWritable(m_selectedReplay->m_path);

	// build source path name with double 0 at the end
	char buffer[255];
	strcpy(buffer,m_selectedReplay->m_path);
	buffer[strlen(buffer)+1]=0;

	// build destination path name with double 0 at the end
	char dest[255];
	strcpy(dest,folder);
	dest[strlen(dest)+1]=0;

	// move file to folder
	SHFILEOPSTRUCT info;
	memset(&info,0,sizeof(info));
	info.wFunc = FO_MOVE;
	info.pFrom = buffer;
	info.pTo = dest;
	info.fFlags = FOF_ALLOWUNDO;
	if(SHFileOperation(&info)!=0) return;

	// update database (change section)
	if(!info.fAnyOperationsAborted) 
	{
		CString str;
		BWChartDB::Delete(BWChartDB::FILE_MAIN,m_selectedReplay->Dir(str),m_selectedReplay->Name());
		char newpath[255];
		UtilDir::AddFileName(newpath,folder,m_selectedReplay->Name());
		m_selectedReplay->m_path = newpath;
		m_selectedReplay->Save(BWChartDB::FILE_MAIN);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnMatchup() 
{
	_AskForRefresh(500); 
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnRwaonly() 
{
	_AskForRefresh(250); 
}

//--------------------------------------------------------------------------------------------------------------

// fill the players list control
void DlgBrowser::OnGetdispinfoListplayers(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;
	CString apm;

	int iItemIndx= pItem->iItem;

	if (pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		PlayerInfo *ply = (PlayerInfo *)m_filterPlayers.GetAt(iItemIndx);
		assert(ply!=0);

		switch(pItem->iSubItem)
		{
		case 0:
			// player name
			strcpy(pItem->pszText,ply->m_name);
			break;
		case 1:
			// apm
			apm.Format("%d",ply->AvgApm());
			strcpy(pItem->pszText,apm);
			break;
		case 2:
			//apm dev
			apm.Format("%d",ply->GetApmDev());
			strcpy(pItem->pszText,apm);
			break;
		case 3:
			//apm dev in % of apm
			apm.Format("%d",ply->GetApmDevPer());
			strcpy(pItem->pszText,apm);
			break;
		case 4:
			// duration
			strcpy(pItem->pszText,ply->AvgDuration(apm));
			break;
		case 5:
			// games played
			apm.Format("%d",ply->m_games);
			strcpy(pItem->pszText,apm);
			break;
		case 6:
			// as zerg
			apm.Format("%d",ply->GetRaceCount(0));
			strcpy(pItem->pszText,apm);
			break;
		case 7:
			// as ran
			apm.Format("%d",ply->GetRaceCount(1));
			strcpy(pItem->pszText,apm);
			break;
		case 8:
			// as toss
			apm.Format("%d",ply->GetRaceCount(2));
			strcpy(pItem->pszText,apm);
			break;
		case 9:
			// apm as T
			apm.Format("%d",ply->AvgApmAsT());
			strcpy(pItem->pszText,apm);
			break;
		case 10:
			// apm as Z
			apm.Format("%d",ply->AvgApmAsZ());
			strcpy(pItem->pszText,apm);
			break;
		case 11:
			// apm as P
			apm.Format("%d",ply->AvgApmAsP());
			strcpy(pItem->pszText,apm);
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

//--------------------------------------------------------------------------------------------------------------

// fill the build order list control
void DlgBrowser::OnGetdispinfoListBos(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;
	CString count;

	int iItemIndx= pItem->iItem;

	if (pItem->mask & LVIF_TEXT && m_filteredBos.GetSize()!=0) //valid text buffer?
	{
		BuildOrder *pbo = (BuildOrder *)m_filteredBos.GetAt(iItemIndx);
		assert(pbo!=0);

		if(pbo->m_percent>0)
			strcpy(pItem->pszText,"#0000C0");
		else
			pItem->pszText[0]=0;

		switch(pItem->iSubItem)
		{
		case 0:
			// count
			count.Format("%d",pbo->m_count);
			strcat(pItem->pszText,count);
			break;
		case 1:
			// percent
			count.Format("%d",pbo->m_percent);
			strcat(pItem->pszText,count);
			break;
		case 2:
			// content
			strcat(pItem->pszText,pbo->m_desc);
			break;
		}
	}

	if(pItem->mask & LVIF_IMAGE) //valid image?
		pItem->iImage=0;
	
	*pResult = 0;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnChangeFpositions() 
{
	UpdateData(TRUE);
	bool rebuild=false;
	// position filter was erased?
	if(m_filterPosition.IsEmpty())
	{
		if(m_posfilteron) {m_posfilteron=FALSE; rebuild=true;}
	}
	// position was filled with something?
	else 
	{
		if(!m_posfilteron) m_posfilteron=TRUE; 
		rebuild=true;
	}
	if(rebuild) _AskForRefresh();
	UpdateData(FALSE);
}

//--------------------------------------------------------------------------------------------------------------

// remove replay from filtered list of replays
void DlgBrowser::_RemoveFilteredReplay(ReplayInfo *rep)
{
	for(int i=0;i<m_filterReplays.GetSize();i++)
	{
		ReplayInfo *reptmp = (ReplayInfo *)m_filterReplays.GetAt(i);
		if(rep==reptmp) {m_filterReplays.RemoveAt(i); return;}
	}
}

//--------------------------------------------------------------------------------------------------------------

// remove replay from list of replays
void DlgBrowser::_RemoveReplay(ReplayInfo *rep)
{
	// remove replay from filtered list of replays
	_RemoveFilteredReplay(rep);

	// remove replay from database
	CString str;
	BWChartDB::Delete(BWChartDB::FILE_MAIN,rep->Dir(str),rep->Name());

	//remove replay from memory
	for(int i=0;i<m_replays.GetSize();i++)
	{
		ReplayInfo *replay = (ReplayInfo *)m_replays.GetAt(i);
		if(rep==replay)
			{m_replays.RemoveAt(i); delete rep; break;}
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnChangeFmap() 
{
	UpdateData(TRUE);
	bool rebuild=false;
	// map filter was erased?
	if(m_filterMap.IsEmpty())
	{
		if(m_mapFilterOn) {m_mapFilterOn=FALSE; rebuild=true;}
	}
	// map name was filled with something?
	else 
	{
		if(!m_mapFilterOn) m_mapFilterOn=TRUE; 
		rebuild=true;
	}
	if(rebuild) _AskForRefresh();
	UpdateData(FALSE);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnSelchangeVtab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_groupby = m_vtab.GetCurSel();
	UpdateData(FALSE);
	OnRadio();
	
	*pResult = 0;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnCleanmissing() 
{
	CWaitCursor wait;

	//show progress bar
	m_progress.ShowWindow(SW_SHOW);

	// for each replay
	int removed=0;
	for(int i=0; i<m_replays.GetSize();)
	{
		// update progress bar
		m_progress.SetPos(100*i/m_replays.GetSize());

		// filter paint messages
		_ProcessPaint();

		// get replay
		ReplayInfo *rep = (ReplayInfo *)m_replays.GetAt(i);

		// does the replay file still exist?
		if(_access(rep->m_path,0)!=0)
		{
			// remove replay from filtered list of replays
			_RemoveFilteredReplay(rep);

			// remove replay from database
			CString str;
			BWChartDB::Delete(BWChartDB::FILE_MAIN,rep->Dir(str),rep->Name());

			//remove play from memory
			m_replays.RemoveAt(i); 
			delete rep; 
			removed++;
		}
		else
			i++;
	}

	//hide progress bar
	m_progress.ShowWindow(SW_HIDE);
	m_progress.SetPos(0);

	// end message
	if(removed>0)
	{
		CString msg;
		msg.Format(IDS_REMOVEDMISSING,removed);
		MessageBox(msg);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_BuildBOFile()
{
	CWaitCursor wait;

	// for each replay
	int removed=0;
	for(int i=0; i<m_replays.GetSize();)
	{
		// update progress bar
		m_progDlg->m_progress.SetPos(100*i/m_replays.GetSize());

		// filter paint messages
		_ProcessPaint();

		// get replay
		ReplayInfo *rep = (ReplayInfo *)m_replays.GetAt(i);

		// does the replay file still exist?
		if(_access(rep->m_path,0)!=0)
		{
			// remove replay from filtered list of replays
			_RemoveFilteredReplay(rep);

			// remove replay from database
			CString str;
			BWChartDB::Delete(BWChartDB::FILE_MAIN,rep->Dir(str),rep->Name());

			//remove play from memory
			m_replays.RemoveAt(i); 
			delete rep; 
			removed++;
		}
		else
		{
			// update replay's BOs
			if(_LoadReplay(rep->m_path,rep->m_filedate,*rep))
			{
				// save in buildorder.txt
				rep->SaveBO();

				// for each player in the replay
				for(int k=0; k<rep->m_playerCount; k++)
				{
					// add build order
					if(!rep->m_bo[k].IsEmpty())
						_AddBuildOrder(rep->m_bo[k],rep,k);
				}
			}
			i++;
		}
	}

	MAINWND->SetHasBOFile();
}

//--------------------------------------------------------------------------------------------------------------

// build order options filter
int DlgBrowser::_GetBOOptions() const
{
	int boOptions=0;
	if(m_boBuilding) boOptions|=BOTree::BUILDING;
	if(m_boUnit) boOptions|=BOTree::UNIT;
	if(m_boResearch) boOptions|=BOTree::RESEARCH;
	if(m_boUpgrade) boOptions|=BOTree::UPGRADE;
	if(m_boSupply) boOptions|=BOTree::DEPOTS;
	return boOptions;
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::_UpdateBuildOrder()
{
	CWaitCursor wait;

	// init filter
	DlgFilter filter;
	_BuildFilter(filter);

	// clear list control
	m_filteredBos.RemoveAll();
	m_listBos.DeleteAllItems();

	// build tree of unique build orders (with counting)
	BOTree tree;
	for(int i=0; i<m_bos.GetSize(); i++)
	{
		BuildOrder *pbo=(BuildOrder *)m_bos.GetAt(i);
		if(_InsertBOVirtual(pbo, &filter))
			tree.AddBO(pbo->m_bo,_GetBOOptions(),m_boMaxObj);
	}

	// display build orders
	m_filteredBos.RemoveAll();
	tree.ListUniqueBos(this);
	m_listBos.SetItemCountEx(m_filteredBos.GetSize(), LVSICF_NOSCROLL|LVSICF_NOINVALIDATEALL);

	// compute total
	int total = 0;
	for(int i=0;i<m_filteredBos.GetSize();i++)
	{
		BuildOrder *pbo=(BuildOrder *)m_filteredBos.GetAt(i);
		total+=pbo->m_count;
	}

	// compute percentage
	for(int i=0;i<m_filteredBos.GetSize();i++)
	{
		BuildOrder *pbo=(BuildOrder *)m_filteredBos.GetAt(i);
		pbo->m_percent = (100*pbo->m_count)/total;
	}

	// replays count
	CString str;
	str.Format("%d bos",total);
	SetDlgItemText(IDC_BOREPLAYS,str);

	//presort
	_SortBuildOrder(m_sortbo,false);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnToggleBOOption()
{
	UpdateData(TRUE);

	// update bo list
	_UpdateBuildOrder();

	// make sure we view the bos list
	if(m_vtab.GetCurSel()!=3)
	{
		m_vtab.SetCurSel(3);
		m_groupby = m_vtab.GetCurSel();
		UpdateData(FALSE);
		OnRadio();
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnChangeBomaxobj() 
{
	if(m_bDBLoaded)
	{
		UpdateData(TRUE);
		_UpdateBuildOrder();
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnSelchangeCombomu() 
{
	int sel = m_bomu.GetCurSel();
	AfxGetApp()->WriteProfileInt("BWCHART_BROWSER","BOMU",sel);
	_UpdateBuildOrder();
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnToggleFilterOnBo()
{
	// update replays/players/maps but not bos
	_DisplayList(false);
}

//--------------------------------------------------------------------------------------------------------------

void DlgBrowser::OnItemchangedListBos(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if(pNMListView->uNewState&(LVIS_SELECTED+LVIS_FOCUSED))
	{
		if(((CButton*)GetDlgItem(IDC_FILTERONBO))->GetCheck()==BST_CHECKED)
			m_refreshReplays = true;
	}
	*pResult=0;
}

//--------------------------------------------------------------------------------------------------------------
