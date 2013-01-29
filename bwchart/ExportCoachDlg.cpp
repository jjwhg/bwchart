// ExportCoachDlg.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "ExportCoachDlg.h"
#include "replay.h"
#include "bwcoachdata.h"
#include "dlgbrowser.h"
#include "../bwcoach/bwtraining.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(ExportCoachDlg, CDialog)
	//{{AFX_MSG_MAP(ExportCoachDlg)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-------------------------------------------------------------------------------------

ExportCoachDlg::ExportCoachDlg(CWnd* pParent, Replay *pReplay)
	: CDialog(ExportCoachDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(ExportCoachDlg)
	m_author = _T("");
	m_desc = _T("");
	m_duration = 0;
	m_title = _T("");
	m_includeUnits = FALSE;
	m_includeWorkers = FALSE;
	//}}AFX_DATA_INIT
	m_pReplay = pReplay;
	memset(m_enabled,1,sizeof(m_enabled));

	// load parameters
	m_author = AfxGetApp()->GetProfileString("BWCOACH","AUTHOR","Your name");
	m_desc = AfxGetApp()->GetProfileString("BWCOACH","DESC","Description of training file");
	m_duration = AfxGetApp()->GetProfileInt("BWCOACH","DURATION",10);
	m_title = AfxGetApp()->GetProfileString("BWCOACH","TITLE","Quick title");
	m_includeUnits = AfxGetApp()->GetProfileInt("BWCOACH","UNITS",1);
	m_includeWorkers = AfxGetApp()->GetProfileInt("BWCOACH","WORKERS",0);

	m_pImageList = new CImageList();
}

ExportCoachDlg::~ExportCoachDlg()
{
	delete m_pImageList;
}

//-------------------------------------------------------------------------------------

void ExportCoachDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ExportCoachDlg)
	DDX_Control(pDX, IDC_LIST1, m_plStats);
	DDX_Text(pDX, IDC_AUTHOR, m_author);
	DDX_Text(pDX, IDC_DESC, m_desc);
	DDX_Text(pDX, IDC_DURATION, m_duration);
	DDX_Text(pDX, IDC_TITLE, m_title);
	DDX_Check(pDX, IDC_EXPORTUNITS, m_includeUnits);
	DDX_Check(pDX, IDC_EXPORTWORKERS, m_includeWorkers);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------------------------

bool ExportCoachDlg::_GetExportFileName(CString& file) 
{
	static char szFilter[] = "Training File (*"TRAINING_EXT")|*"TRAINING_EXT"|All Files (*.*)|*.*||";

 	CFileDialog dlg(FALSE,TRAINING_EXT,"",0,szFilter,this);
	CString str = AfxGetApp()->GetProfileString("BWCOACH","EXPDIR","");
	if(!str.IsEmpty()) dlg.m_ofn.lpstrInitialDir = str;
	if(dlg.DoModal()==IDOK)
	{
		file = dlg.GetPathName();
		AfxGetApp()->WriteProfileString("BWCOACH","EXPDIR",file);
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------

bool ExportCoachDlg::_ExportTo(const char *file) 
{
	// remove previous file if any
	remove(file);

	// create TrFile object
	TrFile *trfile = new TrFile(false,true);
	trfile->pHeader()->SetAuthor(m_author);
	trfile->pHeader()->SetTitle(m_title);
	trfile->pHeader()->SetDesc(m_desc);
	trfile->pHeader()->SetDate(CTime::GetCurrentTime().Format("%d/%m/%Y"));

	// empty all elements list for disabled players
	CString msg;
 	for(int i=0; i<m_pReplay->GetPlayerCount(); i++)
	{
		// get player event list
		if(!m_enabled[i]) continue;
		ReplayEvtList *list = m_pReplay->GetEvtList(i);

		// create goal list
		TrGoalList *bo = new TrGoalList(list->PlayerName());
		bo->SetRace(list->GetRaceIdx()+1);

		// for all events
		for(int j=0;j<list->GetEventCount(); j++)
		{
			// stop when we reach event that is after limit
			ReplayEvt *evt = list->GetEvent(j);
			int eventTimeS = m_pReplay->QueryFile()->QueryHeader()->Tick2Sec(evt->Time());
			if(eventTimeS>m_duration*60) break;

			//skip discarded events
			if(evt->IsDiscarded()) continue;
				   
			// is it a build/train/upgrade/research?
			const IStarcraftAction *action = evt->GetAction();
			int actionID = action->GetID();
			if(actionID==BWrepGameData::CMD_BUILD || actionID==BWrepGameData::CMD_MORPH || 
				(m_includeUnits && (actionID==BWrepGameData::CMD_TRAIN || actionID==BWrepGameData::CMD_HATCH ||
				actionID == BWrepGameData::CMD_MERGEARCHON || actionID == BWrepGameData::CMD_MERGEDARKARCHON)) ||
				actionID == BWrepGameData::CMD_UPGRADE || actionID == BWrepGameData::CMD_RESEARCH)
			{
				// if it's a BUILD command for Terran
				bool isLand=false;
				if(actionID==BWrepGameData::CMD_BUILD)
				{
					// make sure it's not just a "land"
					const BWrepActionBuild::Params *p = (const BWrepActionBuild::Params *)action->GetParamStruct();
					if(p->m_buildingtype==BWrepGameData::BTYP_LAND) isLand=true;
				}

				// add goal
				int idx = evt->GetBWCoachID();
				if(idx>0 && (m_includeWorkers || (idx!=6 && idx!=61 && idx!=118)))
				{
					msg = BWCoachData::GetMessage(idx);
					char wav[8];
					if(!isLand) sprintf(wav,"%d",idx); else {wav[0]=0; msg.Replace("Build","Land");}
					TrGoal *goal = new TrGoal(eventTimeS,isLand?0:BWCoachData::GetDuration(idx),msg,wav);
					bo->AddGoal(goal);
				}
			}
		}

		// update bo description
		bo->SetAuthor(m_author);
		bo->SetDesc("<update bo description>");
		bo->SetDate(CTime::GetCurrentTime().Format("%d/%m/%Y"));

		// add bo to TrFile
		trfile->AddBO(bo);
	}

	// create COMMON goal list
	TrGoalList *boCommon = new TrGoalList(SECTION_COMMON);

	// add good luck goal
	boCommon->AddGoal(new TrGoal(15,0,"Good Luck!","goodluck.wav"));

	// add bo to TrFile
	trfile->AddBO(boCommon);

	// save file
	trfile->Save(file);
	delete trfile;

	return true;
}

//-------------------------------------------------------------------------------------

void ExportCoachDlg::OnOK() 
{
	// save parameters
	UpdateData(TRUE);
	AfxGetApp()->WriteProfileString("BWCOACH","AUTHOR",m_author);
	AfxGetApp()->WriteProfileString("BWCOACH","DESC",m_desc);
	AfxGetApp()->WriteProfileInt("BWCOACH","DURATION",m_duration);
	AfxGetApp()->WriteProfileString("BWCOACH","TITLE",m_title);
	AfxGetApp()->WriteProfileInt("BWCOACH","UNITS",m_includeUnits);
	AfxGetApp()->WriteProfileInt("BWCOACH","WORKERS",m_includeWorkers);
	
	// get export file name
	CString file;
	if(_GetExportFileName(file))
	{
		if(_ExportTo(file))
			DlgBrowser::StartProcess("notepad.exe", file, 0, false);
		CDialog::OnOK();
	}
}

//-------------------------------------------------------------------------------------

void ExportCoachDlg::_ToggleIgnorePlayer() 
{
	POSITION pos = m_plStats.GetFirstSelectedItemPosition();
	if(pos!=0)
	{
		// get selected item
		int nItem = m_plStats.GetNextSelectedItem(pos);
		m_enabled[nItem]=!m_enabled[nItem];
		// update list
		LVITEM item = {LVIF_IMAGE ,nItem,0,0,0,0,0,m_enabled[nItem]?1:0,0,0}; 
		m_plStats.SetItem(&item);
	}
}

//-----------------------------------------------------------------------------------------------------------------

// display player stats
void ExportCoachDlg::_DisplayPlayerStats()
{
	m_plStats.DeleteAllItems();

	// for each player
	for(int i=0; i<m_pReplay->GetPlayerCount(); i++)
	{
		// get event list
		ReplayEvtList *list = m_pReplay->GetEvtList(i);
		m_enabled[i] = list->IsEnabled();

		// insert player stats
		CString str;
		int nPos = m_plStats.InsertItem(i,list->PlayerName(), m_enabled[i]?1:0);

		str.Format("%d",list->GetEventCount());
		m_plStats.SetItemText(nPos,1,str);

		str.Format("%d",list->GetActionPerMinute());
		m_plStats.SetItemText(nPos,2,str);

		m_plStats.SetItemData(nPos,(DWORD)list);
	}
}

//-----------------------------------------------------------------------------------------------------------------

BOOL ExportCoachDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// player stats
	m_plStats.SetExtendedStyle(m_plStats.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP+LVS_EX_FULLROWSELECT);
	m_plStats.InsertColumn(0, "Player",LVCFMT_LEFT,200);
	m_plStats.InsertColumn(1, "Actions",LVCFMT_LEFT,50,1);
	m_plStats.InsertColumn(2, "APM",LVCFMT_LEFT,40,2);

	// Set the background color
	COLORREF clrBack = RGB(255,255,255);
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

	// display player stats
	_DisplayPlayerStats();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//-----------------------------------------------------------------------------------------------------------------

void ExportCoachDlg::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_ToggleIgnorePlayer();
	*pResult = 0;
}

//-----------------------------------------------------------------------------------------------------------------
