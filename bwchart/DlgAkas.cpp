// DlgAkas.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgAkas.h"
#include "DlgEditAkas.h"
#include "DlgBWChart.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgAkas dialog


DlgAkas::DlgAkas(CWnd* pParent /*=NULL*/)
: CDialog(DlgAkas::IDD, pParent), m_akalist(BWChartDB::FILE_AKAS), m_akalistMap(BWChartDB::FILE_MAPAKAS)
{
	//{{AFX_DATA_INIT(DlgAkas)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void DlgAkas::InitInstance()
{
	m_akalist.Load();
	m_akalistMap.Load();
}

void DlgAkas::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgAkas)
	DDX_Control(pDX, IDC_AKAMAPS, m_akaMaps);
	DDX_Control(pDX, IDC_AKAPLAYERS, m_akaPlayers);
	DDX_Control(pDX, IDC_PLAYERS, m_players);
	DDX_Control(pDX, IDC_MAPS, m_maps);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgAkas, CDialog)
	//{{AFX_MSG_MAP(DlgAkas)
	ON_BN_CLICKED(IDC_ADDAKAMAP, OnAddakamap)
	ON_BN_CLICKED(IDC_ADDAKAPLAYER, OnAddakaplayer)
	ON_BN_CLICKED(IDC_ADDMAP, OnAddmap)
	ON_BN_CLICKED(IDC_ADDPLAYER, OnAddplayer)
	ON_BN_CLICKED(IDC_DELMAP, OnDelmap)
	ON_BN_CLICKED(IDC_DELPLAYER, OnDelplayer)
	ON_CBN_SELCHANGE(IDC_MAPS, OnSelchangeMaps)
	ON_CBN_SELCHANGE(IDC_PLAYERS, OnSelchangePlayers)
	ON_LBN_DBLCLK(IDC_AKAPLAYERS, OnDblclkAkaplayers)
	ON_LBN_DBLCLK(IDC_AKAMAPS, OnDblclkAkamaps)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------------------------------------------

void DlgAkas::OnAddmap() 
{
	DlgEditAkas dlg(&m_akalistMap,0,DlgEditAkas::MAPS);
	if(dlg.DoModal()==IDOK)
	{
		// save new player
		m_akalistMap.Save(dlg.m_player);

		// update dialog
		_FillMaps(dlg.m_mainName);

		// update browser
		MAINWND->pGetBrowser()->RefreshAkas();
	}
}

//-------------------------------------------------------------------------------------------------------

void DlgAkas::OnAddplayer() 
{
	DlgEditAkas dlg(&m_akalist,0,DlgEditAkas::PLAYERS);
	if(dlg.DoModal()==IDOK)
	{
		// save new player
		m_akalist.Save(dlg.m_player);

		// update dialog
		_FillPlayers(dlg.m_mainName);

		// update browser
		MAINWND->pGetBrowser()->RefreshAkas();
	}
}

//-------------------------------------------------------------------------------------------------------

void DlgAkas::OnDelmap() 
{
	// get selection
	int sel = m_maps.GetCurSel();
	if(sel<0) return;

	// get selected aka
	Aka *aka = (Aka*)m_maps.GetItemData(sel);

	// confirm?
	CString msg;
	msg.Format(IDS_DELETEAKAS,aka->MainName());
	if(MessageBox(msg,0,MB_YESNO)==IDNO) return;

	// remove 
	m_akalistMap.RemovePlayer(aka);

	// update dialog
	_FillMaps();

	// update browser
	MAINWND->pGetBrowser()->RefreshAkas();
}

//-------------------------------------------------------------------------------------------------------

void DlgAkas::OnDelplayer() 
{
	// get selection
	int sel = m_players.GetCurSel();
	if(sel<0) return;

	// get selected aka
	Aka *aka = (Aka*)m_players.GetItemData(sel);

	// confirm?
	CString msg;
	msg.Format(IDS_DELETEAKAS,aka->MainName());
	if(MessageBox(msg,0,MB_YESNO)==IDNO) return;

	// remove 
	m_akalist.RemovePlayer(aka);

	// update dialog
	_FillPlayers();

	// update browser
	MAINWND->pGetBrowser()->RefreshAkas();
}

//-------------------------------------------------------------------------------------------------------

void DlgAkas::OnAddakamap() 
{
	// get selection
	int sel = m_maps.GetCurSel();
	if(sel<0) return;

	// get aka
	Aka *aka = (Aka*)m_maps.GetItemData(sel);

	// open dialog
	DlgEditAkas dlg(&m_akalistMap,aka,DlgEditAkas::MAPS);
	if(dlg.DoModal()==IDOK)
	{
		// save new player
		m_akalistMap.Save(dlg.m_player);

		// update screen
		_FillMaps(dlg.m_mainName);

		// update browser
		MAINWND->pGetBrowser()->RefreshAkas();
	}
}

//-------------------------------------------------------------------------------------------------------

void DlgAkas::OnAddakaplayer() 
{
	// get selection
	int sel = m_players.GetCurSel();
	if(sel<0) return;

	// get aka
	Aka *aka = (Aka*)m_players.GetItemData(sel);

	// open dialog
	DlgEditAkas dlg(&m_akalist,aka,DlgEditAkas::PLAYERS);
	if(dlg.DoModal()==IDOK)
	{
		// save new player
		m_akalist.Save(dlg.m_player);

		// update screen
		_FillPlayers(dlg.m_mainName);

		// update browser
		MAINWND->pGetBrowser()->RefreshAkas();
	}
}

//-------------------------------------------------------------------------------------------------------

void DlgAkas::OnSelchangeMaps() 
{
	// get selection
	int sel = m_maps.GetCurSel();
	if(sel<0) return;

	// get aka
	Aka *aka = (Aka*)m_maps.GetItemData(sel);

	// display names
	m_akaMaps.ResetContent();
	for(int i=0; i<aka->GetAkaCount(); i++)
		m_akaMaps.AddString(aka->GetAka(i));
}

//-------------------------------------------------------------------------------------------------------

void DlgAkas::OnSelchangePlayers() 
{
	// get selection
	int sel = m_players.GetCurSel();
	if(sel<0) return;

	// get aka
	Aka *aka = (Aka*)m_players.GetItemData(sel);

	// display names
	m_akaPlayers.ResetContent();
	for(int i=0; i<aka->GetAkaCount(); i++)
		m_akaPlayers.AddString(aka->GetAka(i));
}

//-------------------------------------------------------------------------------------------------------

void DlgAkas::_FillPlayers(const char *name)
{
	// fill player combo
	m_akalist.FillCombo(&m_players,"SELPLAYER",name);
	OnSelchangePlayers();
}

//-------------------------------------------------------------------------------------------------------

void DlgAkas::_FillMaps(const char *name)
{
	// fill player combo
	m_akalistMap.FillCombo(&m_maps,"SELMAP",name);
	OnSelchangeMaps();
}

//-------------------------------------------------------------------------------------------------------

BOOL DlgAkas::OnInitDialog() 
{
	CDialog::OnInitDialog();

	_FillPlayers();
	_FillMaps();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//-------------------------------------------------------------------------------------------------------

void DlgAkas::OnDblclkAkaplayers() 
{
	OnAddakaplayer();
}

void DlgAkas::OnDblclkAkamaps() 
{
	OnAddakamap();
}

//-------------------------------------------------------------------------------------------------------

void DlgAkas::RefreshAkas()
{
	_FillPlayers();
	_FillMaps();
}

//-------------------------------------------------------------------------------------------------------
