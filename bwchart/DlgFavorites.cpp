// DlgFavorites.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DlgFavorites.h"
#include "DlgBrowser.h"
#include "DlgEditComment.h"
#include "DlgBWChart.h"
#include "DlgFastAka.h"
#include "bwdb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(DlgFavorites, CDialog)
	//{{AFX_MSG_MAP(DlgFavorites)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_REPS, OnDblclkReps)
	ON_NOTIFY(NM_RCLICK, IDC_REPS, OnRclickReps)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_REPS, OnColumnclickReps)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_REPS, OnItemchangedReps)
	ON_BN_CLICKED(IDC_EDITCOMMENT, OnEditcomment)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_F_LOADREPLAY,OnLoadReplay)
	ON_COMMAND(ID__ADDREPLAYEVENTS,OnAddReplay)
	ON_COMMAND(ID__REMOVEFAVORITES,OnRemoveFavorites)
	ON_COMMAND(ID__EDITCOMMENTS,OnEditcomment) 
	ON_COMMAND(ID__WATCHREPLAYINBW,OnWatchReplay)
	ON_COMMAND(ID__UPDATEAKAS,OnUpdateAkas)
END_MESSAGE_MAP()

//--------------------------------------------------------------------------------------------------------------

DlgFavorites::DlgFavorites(CWnd* pParent /*=NULL*/)
	: CDialog(DlgFavorites::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgFavorites)
	m_replayCount = _T("");
	//}}AFX_DATA_INIT

	m_selectedReplay=0;
	m_idxSelectedReplay=-1;

	// sorting info
	memset(m_Descending,1,sizeof(m_Descending));
	m_currentSortIdx=0;
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgFavorites)
	DDX_Control(pDX, IDC_REPS, m_reps);
	DDX_Text(pDX, IDC_REPLAYCOUNT, m_replayCount);
	//}}AFX_DATA_MAP
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::_UpdateCounter()
{
	m_replayCount.Format("%d replays",m_replays.GetSize());
	UpdateData(FALSE);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgFavorites::ProcessEntry(const char * section, const char *entry, const char *data, int percentage)
{
	AkaList *akalist = &MAINWND->pGetAkas()->m_akalist;
	AkaList *akalistmap = &MAINWND->pGetAkas()->m_akalistMap;

	// create replay
	ReplayInfo *rep = new ReplayInfo;

	// load replay (reppath,repname,data must be in regular format)
	CString rdata(BWChartDB::ConverFromHex(data));
	rep->ExtractInfo(section,entry,(char*)(const char*)rdata);

	// trim player name
	for(int k=0; k<rep->m_playerCount; k++)
	{
		rep->m_player[k].TrimRight();
		const char *pname=rep->m_player[k];

		// do we have an aka for that player?
		const Aka *aka = akalist->GetAkaFromName(pname);
		if(aka!=0) pname = aka->MainName();
		rep->m_mainName[k]=pname;
	}

	// trim map name
	rep->m_map.TrimRight();
	const char *pname=rep->m_map;

	// do we have an aka for that player?
	const Aka *aka = akalistmap->GetAkaFromName(pname);
	if(aka!=0) pname = aka->MainName();
	rep->m_mainMap = pname;

	// add replay
	m_replays.Add(rep);
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::_Load()
{
	// clear array
	m_replays.RemoveAll();

	// load entries from favorites file
	LoadFile(BWChartDB::FILE_FAVORITES);

	// refresh display
	_DisplayFavorites();
}

//--------------------------------------------------------------------------------------------------------------

BOOL DlgFavorites::OnInitDialog() 
{
	UINT repCol[]={IDS_COL_REPLAYNAME,IDS_COL_PLAYER1,IDS_COL_APM1,IDS_COL_PLAYER2,IDS_COL_APM2	
		,IDS_COL_MAP,IDS_COL_DURATION,IDS_COL_TYPE	,IDS_COL_RWAAUTHOR,IDS_COL_GAMEDATE,IDS_COL_ENGINE	
		,IDS_COL_COMMENT,IDS_COL_DIRECTORY,IDS_COL_FILEDATE,IDS_COL_POS1,IDS_COL_POS2,IDS_HACKCOUNT};
	int repWidth[]={195,120,45,120,45,125,60,40,110,80,55,300,300,80,45,45,45};

	CDialog::OnInitDialog();
	
	// prepare replays list control
	m_reps.SetExtendedStyle(m_reps.GetExtendedStyle()|LVS_EX_HEADERDRAGDROP+LVS_EX_FULLROWSELECT);
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

	// load favorites
	_Load();

	// load column settings
	DlgBrowser::LoadColumns(&m_reps,"fav");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::_Resize()
{
	if(m_reps.m_hWnd == NULL)
		return;      // Return if window is not created yet.

	// Get size of dialog window.
	CRect rect;
	GetClientRect(&rect);

	// Adjust the rectangle
	rect.DeflateRect(4,18);
	rect.OffsetRect(3,12);

	// special rect for replay list
	int right = rect.right;
	rect.bottom-=20;
	m_reps.MoveWindow(&rect, TRUE);   
	rect.top=rect.bottom+6; rect.bottom=rect.top+16; rect.right=70;
	GetDlgItem(IDC_COMMENT1)->MoveWindow(&rect, TRUE);
	rect.left=rect.right+4; rect.right=right-48;
	GetDlgItem(IDC_COMMENT)->MoveWindow(&rect, TRUE);
	rect.left=rect.right+4; rect.right=right;
	GetDlgItem(IDC_EDITCOMMENT)->SetWindowPos(0,rect.left,rect.top,0,0, SWP_NOZORDER|SWP_NOSIZE);
}

//--------------------------------------------------------------------------------------------------------------

int DlgFavorites::_FindReplay(const char *path)
{
	for(int i=0; i<m_replays.GetSize(); i++)
	{
		ReplayInfo *rep = (ReplayInfo *)m_replays.GetAt(i);
		if(rep->m_path == path) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------------------------------------------

void DlgFavorites::SaveFavorite(ReplayInfo *rep)
{
	// insert in replay list file
	rep->Save(BWChartDB::FILE_FAVORITES);

	// add to internal list
	ReplayInfo *copyrep = new ReplayInfo(*rep);
	m_replays.Add(copyrep);

	// refresh display
	_DisplayFavorites();
}

//-----------------------------------------------------------------------------------------------------------------

void DlgFavorites::RemoveFavorite(ReplayInfo *rep, bool rebuildList)
{
	// do we have it?
	int idx =_FindReplay(rep->m_path);
	if(idx<0) return;

	// remove replay from list file
	rep->Delete(BWChartDB::FILE_FAVORITES);

	// remove from internal list
	m_replays.RemoveAt(idx);
	delete rep;

	// refresh display
	if(rebuildList) _DisplayFavorites();
}

//-----------------------------------------------------------------------------------------------------------------

void DlgFavorites::_DisplayFavorites()
{
	m_reps.DeleteAllItems();
	for(int i=0;i<m_replays.GetSize();i++)
	{
		ReplayInfo *rep = (ReplayInfo *)m_replays.GetAt(i);
		_InsertReplay(&m_reps,rep, i);
	}

	_UpdateCounter();
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	if(cx!=0 && cy!=0) _Resize();
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::OnRclickReps(NMHDR* pNMHDR, LRESULT* pResult) 
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

		// load menu
		CMenu menu;
		menu.LoadMenu(IDR_POPUPFAV);
		CMenu *pSub = menu.GetSubMenu(0);

		// display popup menu
		m_reps.ClientToScreen(&pt);
		pSub->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}
	
	*pResult = 0;
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::OnDblclkReps(NMHDR* pNMHDR, LRESULT* pResult) 
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
		AfxGetMainWnd()->PostMessage(WM_LOADREPLAY,(WPARAM)m_selectedReplay,1);
	}
	
	*pResult = 0;
}

//----------------------------------------------------------------------------------------

void DlgFavorites::OnLoadReplay() 
{
	if(m_selectedReplay!=0)
		AfxGetMainWnd()->PostMessage(WM_LOADREPLAY,(WPARAM)m_selectedReplay,1);
}

//----------------------------------------------------------------------------------------

void DlgFavorites::OnAddReplay() 
{
	if(m_selectedReplay!=0)
		AfxGetMainWnd()->PostMessage(WM_LOADREPLAY,(WPARAM)m_selectedReplay,0);
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::OnRemoveFavorites()
{
	if(m_selectedReplay!=0)
	{
		RemoveFavorite(m_selectedReplay,true);
		m_selectedReplay=0;
		m_idxSelectedReplay=-1;
	}
}

//------------------------------------------------------------------------------------

static bool gbAscendingFav=false;

extern bool gbAscending;
extern int CALLBACK CompareReplay(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

int CALLBACK CompareReplayFav(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	bool tmp = gbAscending; 
	gbAscending	= gbAscendingFav;
	int diff = CompareReplay(lParam1, lParam2, lParamSort) ;
	gbAscending = tmp;
	return diff;
}

//------------------------------------------------------------------------------------

// sort list
void DlgFavorites::_SortReplay(int item)
{
	// if we are sorting again on current sorting column
	if(item==m_currentSortIdx)
	{
		// revert sorting order
		m_Descending[m_currentSortIdx]=!m_Descending[m_currentSortIdx];
	}
	else
	{
		// new sorting column
		if(item>=0) m_currentSortIdx = item;
	}

	// sort items
	gbAscendingFav = !m_Descending[m_currentSortIdx];
	m_reps.SortItems(CompareReplayFav,m_currentSortIdx);
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::OnColumnclickReps(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMLV = (NM_LISTVIEW*)pNMHDR;
	_SortReplay(pNMLV->iSubItem);	
	*pResult = 0;
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::_SelectedItem(int nItem)
{
	// get selected item
	m_idxSelectedReplay=nItem;
	m_selectedReplay = (ReplayInfo *)m_reps.GetItemData(nItem);
	// display comment
	if(m_selectedReplay!=0) SetDlgItemText(IDC_COMMENT,m_selectedReplay->m_comment);
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::OnItemchangedReps(NMHDR* pNMHDR, LRESULT* pResult) 
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

void DlgFavorites::OnEditcomment() 
{
	if(m_selectedReplay==0) return;

	// open dialog
	DlgEditComment dlg(m_selectedReplay, this);	
	if(dlg.DoModal()==IDOK)
	{
		// save replay in favorites database
		m_selectedReplay->Save(BWChartDB::FILE_FAVORITES);
		// update also main database
		CString path;
		m_selectedReplay->Save(BWChartDB::FILE_MAIN);
		// update list
		m_reps.SetItemText(m_idxSelectedReplay,9,m_selectedReplay->m_comment);
		// update bottom comment
		SetDlgItemText(IDC_COMMENT,m_selectedReplay->m_comment);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::OnWatchReplay()
{						  
	if(m_selectedReplay!=0)
		MAINWND->pGetBrowser()->StartReplay(m_selectedReplay);
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::OnUpdateAkas()
{
	if(m_selectedReplay==0) return;
	DlgFastAka dlg(m_selectedReplay);
	dlg.DoModal();
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::Refresh()
{
	_Load();
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::OnPaint() 
{
	CPaintDC dc(this);
	m_reps.RedrawWindow(0,0,RDW_FRAME|RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::OnDestroy() 
{
	// save column settings
	DlgBrowser::SaveColumns(&m_reps,"fav");
	CDialog::OnDestroy();
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::_InsertReplay(CListCtrl *reps, ReplayInfo *rep, int idx, const DlgFilter* filter)
{
	if(idx==-1) idx = reps->GetItemCount();

	CString apm;
	CString player;
	char *strRace[4]={"Z","T","P","U"};

	// check filter
	if(filter!=0)
	{
		// filter player name
		if(filter->m_pfilteron)
		{
			int found=0;
			for(int k=0;found<filter->m_filterPlayerCount && k<rep->m_playerCount;k++)
				if(filter->MatchPlayerName(rep->m_mainName[k]))
					found++;
			if(found!=filter->m_filterPlayerCount) return;
		}
	}

	// rep name
	int nPos = reps->InsertItem(idx,rep->Name(), 0);

	// player 1
	if(!rep->m_player[0].IsEmpty()) player.Format("%s (%s)",rep->m_mainName[0],strRace[rep->m_race[0]]);
	reps->SetItemText(nPos,1,player);

	// apm1
	apm.Format("%d",rep->m_apm[0]);
	reps->SetItemText(nPos,2,apm);

	// Player 2
	player="";
	if(!rep->m_player[1].IsEmpty()) player.Format("%s (%s)",rep->m_mainName[1],strRace[rep->m_race[1]]);
	reps->SetItemText(nPos,3,player);

	// apm2
	apm.Format("%d",rep->m_apm[1]);
	reps->SetItemText(nPos,4,apm);

	// map name
	reps->SetItemText(nPos,5,rep->m_mainMap);

	// duration
	reps->SetItemText(nPos,6,rep->Duration(apm));

	// game type
	reps->SetItemText(nPos,7,rep->GameType());

	// RWA author
	reps->SetItemText(nPos,8,rep->m_author);

	// game date
	reps->SetItemText(nPos,9,rep->DateForDisplay(apm));

	// engine
	reps->SetItemText(nPos,10,rep->EngineVersion(apm));

	// comment
	reps->SetItemText(nPos,11,rep->m_comment);

	// path
	CString tmpDir;
	reps->SetItemText(nPos,12,rep->Dir(tmpDir));

	// file date
	reps->SetItemText(nPos,13,rep->m_filedate.Format("%d %b %Y"));

	// player 1 location
	apm.Format("%d",rep->m_start[0]);
	reps->SetItemText(nPos,14,apm);

	// player 2 location
	apm.Format("%d",rep->m_start[1]);
	reps->SetItemText(nPos,15,apm);

	// hack count
	apm.Format("%d",rep->m_hackCount);
	reps->SetItemText(nPos,16,apm);

	// item data
	reps->SetItemData(nPos,(DWORD)rep);
}

//--------------------------------------------------------------------------------------------------------------

void DlgFavorites::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar==VK_DELETE)
	{
		if(m_selectedReplay!=0)
		{
			// remove selected replay
			RemoveFavorite(m_selectedReplay,false);
			// delete line
			m_reps.DeleteItem(m_idxSelectedReplay);
			// if list is not empty, select new line
			if(m_reps.GetItemCount()>0)
				m_reps.SetItemState(m_idxSelectedReplay,LVIS_SELECTED+LVIS_FOCUSED,LVIS_SELECTED+LVIS_FOCUSED);
			// update replay count
			_UpdateCounter();
		}
	}
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

//--------------------------------------------------------------------------------------------------------------
