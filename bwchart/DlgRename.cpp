// DlgRename.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgRename.h"
#include "replaydb.h"
#include "bwdb.h"
#include "dirutil.h"
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//------------------------------------------------------------------------------------


DlgRename::DlgRename(CListCtrl* reps, CObArray *filter, CWnd* pParent /*=NULL*/)
	: CDialog(DlgRename::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgRename)
	m_format = _T("");
	m_userdef1 = _T("");
	m_replacement1 = _T("");
	//}}AFX_DATA_INIT
	m_format = AfxGetApp()->GetProfileString("RENAME","FORMAT","");
	m_userdef1 = AfxGetApp()->GetProfileString("RENAME","USERDEF1","");
	m_replacement1 = AfxGetApp()->GetProfileString("RENAME","REPLACE1","");
	m_reps=reps;
	m_filterReplays=filter;
	m_timer=0;
}

//------------------------------------------------------------------------------------

DlgRename::~DlgRename()
{
}

//------------------------------------------------------------------------------------

void DlgRename::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgRename)
	DDX_Control(pDX, IDC_REPLAYS, m_replays);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Text(pDX, IDC_FORMAT, m_format);
	DDX_Text(pDX, IDC_USERDEF1, m_userdef1);
	DDX_Text(pDX, IDC_REPLACEMENT1, m_replacement1);
	//}}AFX_DATA_MAP
}

//------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DlgRename, CDialog)
	//{{AFX_MSG_MAP(DlgRename)
	ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_EN_CHANGE(IDC_FORMAT, OnChangeFormat)
	ON_EN_CHANGE(IDC_REPLACEMENT1, OnChangeFormat)
	ON_EN_CHANGE(IDC_USERDEF1, OnChangeFormat)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//------------------------------------------------------------------------------------

void DlgRename::OnDefault() 
{
	m_format = "%gmt %p1_%t1+%p2_%t2(%map)";
	UpdateData(FALSE);
	_Display();
}

//------------------------------------------------------------------------------------

void DlgRename::OnRename() 
{
	UpdateData(TRUE);

	// save parameters
	AfxGetApp()->WriteProfileString("RENAME","FORMAT",m_format );	
	AfxGetApp()->WriteProfileString("RENAME","USERDEF1",m_userdef1);
	AfxGetApp()->WriteProfileString("RENAME","REPLACE1",m_replacement1);

	// init progress bar
	CWaitCursor wait;
	m_progress.SetPos(0);
	int count = m_replays.GetItemCount();
	m_progress.SetRange(0,count);
	m_progress.ShowWindow(SW_SHOW);

	// for all selected replays
	POSITION pos = m_reps->GetFirstSelectedItemPosition();
	int idx=0;
	while(pos!=0) 
	{
		// get replay
		int nItem = (m_reps->GetNextSelectedItem(pos));
		ReplayInfo *rep = (ReplayInfo *)m_filterReplays->GetAt(nItem);

		// move file
		char newpath[255+1];
		CString dir;
		UtilDir::AddFileName(newpath,rep->Dir(dir),_NewName(rep));
		if(!MoveFile(rep->m_path,newpath)) continue;

		// update database
		CString str;
		BWChartDB::Delete(BWChartDB::FILE_MAIN,rep->Dir(str),rep->Name());
		rep->m_path = newpath;
		rep->Save(BWChartDB::FILE_MAIN);

		// update progress bar
		idx++;
		m_progress.SetPos(idx);
	}

	// hide progress bar
	m_progress.ShowWindow(SW_HIDE);
	CDialog::OnOK();
}

//------------------------------------------------------------------------------------

BOOL DlgRename::OnInitDialog() 
{
	CDialog::OnInitDialog();
	if(m_format.IsEmpty()) OnDefault();
	
	// prepare replays list control
	m_replays.SetExtendedStyle(m_replays.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP+LVS_EX_FULLROWSELECT);
	m_replays.InsertColumn(0, "Current Name",LVCFMT_LEFT,200);
	m_replays.InsertColumn(1, "New Name",LVCFMT_LEFT,200,1);

	// Set the background color
	COLORREF clrBack = RGB(255,255,255);
	m_replays.SetBkColor(clrBack);
	m_replays.SetTextBkColor(clrBack);
	m_replays.SetTextColor(RGB(10,10,10));

	m_timer = SetTimer(1,1000,0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//------------------------------------------------------------------------------------

const char *DlgRename::_NewName(ReplayInfo *rep)
{
	char *strRace[4]={"Z","T","P","U"};
	static char newname[64+1];
	newname[0]=0;
	CString p1;
	CString t1;
	CString p2;
	CString t2;
	CString apm1;
	CString apm2;
	CString pos1;
	CString pos2;
	CString map;
	CString snum;
	CString nname;
	CString gametype;
	int num=0;

	// need 2 players at least to rename
	if(rep->m_playerCount<2)
	{
		strcpy(newname,rep->Name());
		return newname;
	}

	// game type
	if(rep->m_playerCount>2) gametype=rep->GameType();

	// retrieve players name and race
	p1 = rep->m_mainName[0];
	p2 = rep->m_mainName[1];
	t1 = strRace[rep->m_race[0]];
	t2 = strRace[rep->m_race[1]];
	apm1.Format("%d",rep->m_apm[0]);
	apm2.Format("%d",rep->m_apm[1]);
	pos1.Format("%d",rep->m_start[0]);
	pos2.Format("%d",rep->m_start[1]);
	
	// retrieve map name
	map = rep->m_mainMap;
	map.MakeLower();
	map.Replace("wgtour","");
	map.Replace("wcg","");
	map.Replace("(2)","");
	map.Replace("(3)","");
	map.Replace("(4)","");
	map.Replace("(5)","");
	map.Replace("(6)","");
	map.Replace("(7)","");
	map.Replace("(8)","");
	map.Replace("[2])","");
	map.Replace("[3])","");
	map.Replace("[4])","");
	map.Replace("[5])","");
	map.Replace("[6])","");
	map.Replace("[7])","");
	map.Replace("[8])","");
	map.Replace("wgt9 - ","");
	map.Replace("wgt10 - ","");
	map.Replace("wgt11 - ","");
	map.Replace("wgt12 - ","");
	map.Replace("wgt13 - ","");
	map.Replace("wgt14 - ","");
	map = map.Left(5);

	// make sure it's all letters
	char alphamap[5+1];
	for(int i=0;i<5;i++) if(isalpha(map[i])) alphamap[i]=map[i]; else alphamap[i]=' ';
	alphamap[5]=0;
	map= alphamap;

	// remove edging spaces on map name
	map.TrimLeft();
	map.TrimRight();

	// search a valid name
	for(;;)
	{
		// replace all markers
		nname = m_format;
		nname.Replace("%apm1",apm1);
		nname.Replace("%apm2",apm2);
		nname.Replace("%t1",t1);
		nname.Replace("%t2",t2);
		nname.Replace("%p1",p1);
		nname.Replace("%p2",p2);
		nname.Replace("%map",map);
		nname.Replace("%gmt",gametype);
		nname.Replace("%pos1",pos1);
		nname.Replace("%pos2",pos2);

		// replace user defined stuff
		if(!m_userdef1.IsEmpty())
			nname.Replace(m_userdef1,m_replacement1);

		// remove edging spaces
		nname.TrimLeft();
		nname.TrimRight();

		// remove edging underscores
		if(nname.Left(1)=="_") nname = nname.Mid(2);
		if(nname.Right(1)=="_" && nname.GetLength()>1) nname = nname.Left(nname.GetLength()-1);

		// add number if needed
		if(num>0) 
		{
			snum.Format("_%d",num);
			nname += snum;
		}

		// add extension
		nname += ".rep";

		// remove empty markers that were between brackets
		nname.Replace("[]","");
		nname.Replace("()","");
		nname.Replace("{}","");

		// if name length is ok
		if(nname.GetLength()<=31) 
		{
			// make sure file doesn't already exist
			char path[255+1];
			CString dir;
			UtilDir::AddFileName(path,rep->Dir(dir),nname);
			if(_access(path,0)==0) {num++; continue;}
			else break;
		}

		// reduce size of markers
		if(map.GetLength()>3) 
			map = map.Left(map.GetLength()-1);
		else if(p2.GetLength()>1) 
		{
			map="";
			p2 = p2.Left(p2.GetLength()-1);
		}
		else if(p1.GetLength()>1) 
		{
			p2="";
			p1 = p1.Left(p1.GetLength()-1);
		}
		else 
		{		
			strcpy(newname,rep->Name());
			return newname;
		}
	}

	// return new name
	strcpy(newname,nname);
	return newname;
}

//------------------------------------------------------------------------------------

void DlgRename::_Display()
{
	UpdateData(TRUE);
	m_replays.DeleteAllItems();

	POSITION pos = m_reps->GetFirstSelectedItemPosition();
	int idx=0;
	while(pos!=0) 
	{
		int nItem = (m_reps->GetNextSelectedItem(pos));
		ReplayInfo *rep = (ReplayInfo *)m_filterReplays->GetAt(nItem);
		// rep name
		int nPos = m_replays.InsertItem(idx,rep->Name(), 0);
		m_replays.SetItemText(nPos,1,_NewName(rep));
		idx++;
	}
}

//------------------------------------------------------------------------------------

void DlgRename::OnChangeFormat() 
{
	if(m_timer!=0) KillTimer(m_timer);
	m_timer = SetTimer(1,2000,0);
}

//------------------------------------------------------------------------------------

void DlgRename::OnTimer(UINT nIDEvent) 
{
	KillTimer(m_timer);
	_Display();
	CDialog::OnTimer(nIDEvent);
}

//------------------------------------------------------------------------------------

void DlgRename::OnDestroy() 
{
	if(m_timer!=0) KillTimer(m_timer);
	m_timer=0;

	CDialog::OnDestroy();
}

//------------------------------------------------------------------------------------
