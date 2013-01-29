#if !defined(AFX_DLGBROWSER_H__257B9C85_94D1_4188_8C45_6B1871F41387__INCLUDED_)
#define AFX_DLGBROWSER_H__257B9C85_94D1_4188_8C45_6B1871F41387__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgBrowser.h : header file
//

#include "bwdb.h"
#include "replay.h"
#include "replaydb.h"
#include "botree.h"
#include "xlistctrl.h"

// bw supposed version
#define BW_116 116
#define BW_115 115
#define BW_114 114
#define BW_113 113
#define BW_112 112
#define BW_111 111
#define BW_110 110
#define BW_109 109
#define BW_SC 1
#define BW_AUTO -1


//------------------------------------------------------------

class DlgFilterPosition
{
public:
	DlgFilterPosition(int a=0, int b=0) : m_p1((unsigned short)a), m_p2((unsigned short)b) {}
	bool operator ==(const DlgFilterPosition& other) const
	{
		return (m_p1==other.m_p1 && m_p2==other.m_p2) || (m_p1==other.m_p2 && m_p2==other.m_p1);
	}
	unsigned short m_p1;
	unsigned short m_p2;
};

#define MAXPLAYERFILTER 4
#define MAXPOSITIONFILTER 16

class DlgFilter
{
public:
	DlgFilter() : m_pfilteron(FALSE), m_posfilteron(FALSE), m_boFilterOn(FALSE), m_muT(true), m_muZ(true), m_muP(true), m_muXvX(true), 
		m_rwaOnly(false),m_filterPositionsCount(0), m_filterPlayerCount(0), m_race(0),
		m_bomuT(false), m_bomuP(false), m_bomuZ(false) {}

	// name filter on/off
	BOOL m_pfilteron;
	// position filter on/off
	BOOL m_posfilteron;
	// map filter on/off
	BOOL m_mapFilterOn;
	// bo filter on/off
	BOOL m_boFilterOn;
	// player name(s)
	CString	m_filterPlayer[MAXPLAYERFILTER];
	int m_filterPlayerCount;
	// map name(s)
	CString	m_filterMap[MAXPLAYERFILTER];
	int m_filterMapCount;
	// positions
	DlgFilterPosition m_filterPositions[MAXPOSITIONFILTER];
	int m_filterPositionsCount;
	// match ups
	bool m_muT;
	bool m_muZ;
	bool m_muP;
	bool m_muXvX;
	// display rwa only on/off
	bool m_rwaOnly;
	// bo race
	int m_race;
	bool m_bomuT;
	bool m_bomuP;
	bool m_bomuZ;

	// return true if player name matches player filter
	bool MatchPlayerName(const char *pname) const
	{
		bool found=false;
		CString name(pname); 
		name.MakeLower();
		for(int m=0;!found && m<m_filterPlayerCount;m++)
			if(name.Find(m_filterPlayer[m])>=0) 
				{found=true; break;}
		return found;
	}

	// return true if map name matches map filter
	bool MatchMapName(const char *pname) const
	{
		bool found=false;
		CString name(pname); 
		name.MakeLower();
		for(int m=0;!found && m<m_filterMapCount;m++)
			if(name.Find(m_filterMap[m])>=0) 
				{found=true; break;}
		return found;
	}

};

//------------------------------------------------------------

class BuildOrder : public CObject
{
public:
	BuildOrder(ReplayInfo* rep, int player) : m_player(player), m_rep(rep), m_count(0), m_percent(0) {}
	ReplayInfo* m_rep;
	BONodeList m_bo;
	int m_player;
	int m_count;
	int m_percent;
	CString m_desc;
};

//------------------------------------------------------------

class DlgBrowser : public CDialog, public BWChartDB, public IBOLister
{
// Construction
public:
	DlgBrowser(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgBrowser)
	enum { IDD = IDD_DLGBROWSER_DIALOG };
	CComboBox	m_bomu;
	CTabCtrl	m_vtab;
	CListCtrl	m_reps;
	CListCtrl	m_listMaps;
	CListCtrl	m_listPlayers;
	CXListCtrl	m_listBos;
	CProgressCtrl	m_progress;
	CString	m_rootdir;
	int		m_groupby;
	CString	m_replayCount;
	CString	m_mapCount;
	CString	m_playerCount;
	BOOL	m_autoRefresh;
	BOOL	m_pfilteron;
	CString	m_filterPlayer;
	BOOL	m_recursive;
	BOOL	m_muT;
	BOOL	m_muP;
	BOOL	m_muZ;
	BOOL	m_muXvX;
	BOOL	m_rwaOnly;
	CString	m_filterPosition;
	BOOL	m_posfilteron;
	CString	m_filterMap;
	BOOL	m_mapFilterOn;
	BOOL	m_boBuilding;
	BOOL	m_boResearch;
	BOOL	m_boUnit;
	BOOL	m_boUpgrade;
	BOOL	m_boSupply;
	int		m_boMaxObj;
	BOOL	m_bomuT;
	BOOL	m_bomuP;
	BOOL	m_bomuZ;
	bool m_refreshReplays;
	//}}AFX_DATA

	const CStringArray* GetPlayers() const;
	const CStringArray* GetMaps() const {return &m_allMaps;}
	void Refresh(bool msg);
	bool IsRefreshDone() const {return m_bRefreshDone;}
	void Load();
	void RefreshAkas();
	void StartReplay(ReplayInfo* selectedReplay, int version=-1) ;

	// start a process
	static int StartProcess(const char *pszExe, const char *pszCmdLine, const char *pszCurrentDir=0, bool bHideWindow=false);

	// load and save the column settings of a list control
	static void LoadColumns(CListCtrl *list, const char *name);
	static void SaveColumns(CListCtrl *list, const char *name);
	
	void ProcessEntry(const char * section, const char *entry, const char *data, int percentage);
	ReplayInfo * _ProcessReplay(const char *dir, CFileFind& finder);

	// add replay in database (returns true if replay could be loaded)
	bool AddReplay(const char *reppath, bool msg, bool display);

	// IBOLister implementation
	virtual void AddBO(BONodeList* bo, int count);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgBrowser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnCancel() {}
	virtual void OnOK() {}
	//}}AFX_VIRTUAL

// Implementation
protected:
	Replay m_replay;
	int m_addedReplays;
	bool m_bRefreshDone;
	bool m_bDBLoaded;
	class ProgressDlg *m_progDlg;
	UINT m_reftimer;
	CString m_lastFolder;
	bool m_noRepaint;

	// selected replay
	ReplayInfo* m_selectedReplay;
	int m_idxSelectedReplay;

	// selected player
	PlayerInfo* m_selectedPlayer;
	int m_idxSelectedPlayer;

	// corrupted replays
	CStringArray m_corrupted;

	// sorting
	bool m_Descending[16];
	int m_currentSortIdx;
	bool m_Descending2[16];
	int m_currentSortIdx2;
	bool m_Descending3[16];
	int m_currentSortIdx3;
	bool m_Descending4[16];
	int m_currentSortIdx4;
	
	// sorting columns
	int m_sortrep;
	int m_sortpla;
	int m_sortmap;
	int m_sortbo;

	// replays
	XObArray m_replays;
	// filtered replays
	CObArray m_filterReplays;
	// players (only one per aka)
	XObArray m_players;
	// filtered players
	CObArray m_filterPlayers;
	// maps
	XObArray m_maps;
	// all players (with the original name they had in the replay)
	mutable CStringArray m_allPlayers;
	// all maps
	CStringArray m_allMaps;
	// all build orders
	XObArray m_bos;
	// filtered build orders
	XObArray m_filteredBos;

	bool _LoadReplay(const char *path, const CTime& creationDate, ReplayInfo& tmpRep);
	void _BuildFilter(DlgFilter& filter);
	void _UpdateBuildOrder();
	void _MoveCorruptedReplays();
	ReplayInfo * _GetReplayFromIdx(unsigned long idx) const;
	PlayerInfo * _GetPlayerFromIdx(unsigned long idx) const;
	void _DisplayList(bool rebuildBos=true);
	void _AskForRefresh(int timeMS=1000);
	void _Parameters(bool bLoad);
	void _DoReplay(UINT msg, int param);
	void _ClearAll();
	ReplayInfo* _FindReplay(const char *path) const;
	PlayerInfo* _FindPlayer(const char *name) const;
	bool _FindAllPlayer(const char *name) const;
	MapInfo* _FindMap(const char *name) const;
	bool _FindAllMap(const char *name) const;
	void _MoveReplayToBin();
	void _MoveReplayToFolder(const char *folder);
	void _MemorizeSelection(CObArray& selection);
	void _BuildBOFile();

	// build order options filter
	int _GetBOOptions() const;

	// update player data
	void _AddPlayer(const char *pname, ReplayInfo *rep, int k);
	// update map data
	void _AddMap(const char *pname, ReplayInfo *rep, int k);
	// add build order
	void _AddBuildOrder(const CString& bo, ReplayInfo *rep, int pidx);

	void _Resize();
	void _BrowseReplays(const char *dir, int& count, int &idx, bool bCount);
	void _InsertPlayerVirtual(PlayerInfo *rep, const DlgFilter *filter=0);
	void _InsertMap(MapInfo *map, int idx);
	bool _InsertBOVirtual(BuildOrder *pbo, const DlgFilter *filter);
		 
	ReplayInfo * _AddReplay(const ReplayInfo& tmpRep);
	void _UpdateCounter();
	void _SelectedItem(int nItem);
	void _SelectedPlayer(int nItem);
	HWND _StartBW(ReplayInfo* selectedReplay, int version, CString& exe) ;
	void _GetStarcraftPath(CString& path) ;
	void _UpdateReplayVersion(ReplayInfo* selectedReplay, int version);
	bool _InsertReplayVirtual(ReplayInfo *rep, const DlgFilter* filter=0);
	HWND _WaitForProcess(const char *wndclassname, int timeoutS);

	// remove replay from filtered list of replays
	void _RemoveFilteredReplay(ReplayInfo *rep);

	// remove replay from list of replays
	void _RemoveReplay(ReplayInfo *rep);

	// sort list
	void _SortReplay(int item, bool reverse=true);
	void _SortPlayer(int item, bool reverse=true);
	void _SortMap(int item, bool reverse=true);
	void _SortBuildOrder(int item, bool reverse=true);
													  
	// start BWPLayer if needed
	void _StartRWA(const char *path, int version);

	// if replay a RWA?
	bool _IsRWA(const char *path) const;

	// show or hide bo filters
	void _ShowBoFilter(bool show);

	// Generated message map functions
	//{{AFX_MSG(DlgBrowser)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBrowse();
	afx_msg void OnRadio();
	virtual BOOL OnInitDialog();
	afx_msg void OnRefresh();
	afx_msg void OnColumnclickReps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickBos(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickPlayers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickMaps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickReps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkReps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedReps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedListBos(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditcomment();
	afx_msg void OnAutorefresh();
	afx_msg void OnAddreplay();
	afx_msg void OnDestroy();
	afx_msg void OnToggleFilter();
	afx_msg void OnChangeFplayer();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnGetdispinfoReps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedListplayers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickListplayers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMatchup();
	afx_msg void OnRwaonly();
	afx_msg void OnGetdispinfoListplayers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetdispinfoListBos(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeFpositions();
	afx_msg void OnChangeFmap();
	afx_msg void OnSelchangeVtab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCleanmissing();
	afx_msg void OnToggleBOOption();
	afx_msg void OnChangeBomaxobj();
	afx_msg void OnSelchangeCombomu();
	afx_msg void OnToggleFilterOnBo();
	afx_msg void OnLoadReplay();
	afx_msg void OnAddReplayEvents();
	afx_msg void OnAddToFavorites();
	afx_msg void OnUpdateAkas();
	afx_msg void OnOpenDirectory();
	afx_msg void OnListReplays();
	afx_msg void OnMoveToBin();
	afx_msg void OnMoveToFolder();
	afx_msg void OnRenameReplays();
	afx_msg void OnWatchReplay();
	afx_msg void OnWatchReplay109();
	afx_msg void OnWatchReplay110();
	afx_msg void OnWatchReplay111();
	afx_msg void OnWatchReplay112();
	afx_msg void OnWatchReplay113();
	afx_msg void OnWatchReplay114();
	afx_msg void OnWatchReplay115();
	afx_msg void OnWatchReplay116();
	afx_msg void OnWatchReplaySC();
	afx_msg LRESULT OnMsgAutoRefresh(WPARAM , LPARAM );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGBROWSER_H__257B9C85_94D1_4188_8C45_6B1871F41387__INCLUDED_)
