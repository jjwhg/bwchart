#include "stdafx.h"
#include "bwchart.h"
#include "DlgOptions.h"
#include "newreplay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------------------------------------------

NewReplayNotify::NewReplayNotify() : m_hf(0)
{
}

//-----------------------------------------------------------------------------------------------------------------

bool NewReplayNotify::Start()
{
	// close previous (if any)
	if(m_hf!=0) {CloseHandle(m_hf); m_hf=0;}

	// get path to BW replays
	CString pathToReplays;
	HANDLE hf=INVALID_HANDLE_VALUE;
	DlgOptions::_GetStarcraftPath(pathToReplays);

	// get notification on replay being saved
	if(!pathToReplays.IsEmpty())
	{
		pathToReplays+="maps\\replays";
		m_hf = FindFirstChangeNotification(pathToReplays,false,FILE_NOTIFY_CHANGE_LAST_WRITE);
	}

	return m_hf!=0;
}

//-----------------------------------------------------------------------------------------------------------------

static CTime gnewest;
static CString gpath;

static void __FileTime(CFileFind& finder, CTime& tm)
{
	FILETIME test;
	finder.GetCreationTime(&test);
	if(test.dwLowDateTime==0 && test.dwHighDateTime==0)
	{
		finder.GetLastWriteTime(tm);
	}
	else
	{
		finder.GetCreationTime(tm);
	}
}

void NewReplayNotify::_BrowseReplays(const char *dir, int &idx)
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
			continue;
		}

		// rep file?
		CString ext;
		ext=finder.GetFileName().Right(4);
		if(ext.CompareNoCase(".rep")!=0) continue;

		// load it
		if(idx==0) 
		{
			gpath=finder.GetFilePath(); 
			__FileTime(finder, gnewest);
		}
		else 
		{
			CTime tmp;
			__FileTime(finder, tmp);
			if(tmp>gnewest) {gnewest=tmp; gpath=finder.GetFilePath();}
		}
		idx++;
	}
}

//------------------------------------------------------------------------------------

bool NewReplayNotify::CollectReplay()
{
	// handler started?
	if(m_hf==0) return false;

	// any new file in the replay dir?
	m_repfile="";
	if(WaitForSingleObject(m_hf, 125) != WAIT_OBJECT_0) 
		return false;

	// get path to BW replays
	CString path;
	DlgOptions::_GetStarcraftPath(path);
	if(path.IsEmpty()) return false;
	path+="maps\\replays";

	// search for most recent replay file
	int idx=0;
	_BrowseReplays(path, idx);

	// restart handler
	Start();

	// return replay path
	m_repfile = gpath;
	return true;
}

//------------------------------------------------------------------------------------
