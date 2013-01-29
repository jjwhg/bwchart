#include"stdafx.h"
#include"replaydb.h"
#include"bwdb.h"
#include"resource.h"
#include"DlgBWChart.h"
#include"bwrepapi.h"
#include<io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------------------------------------------

#define SRCEOL "\r\n"
#define TOKENEOL "_%#$%_"

void ReplayInfo::Save(int file)
{
	int i;

	// insert in replay list file
	// format:  version \ file date \ game date \ map \ player count \ {player\apm\race\apmdev} \ duration \ engine \ rwa
	CString desc;
	desc.Format("%d\\%s\\%s\\%s\\%d\\",VER_CURRENT,(const char*)m_filedate.Format("%d/%m/%Y"),
		(const char*)m_date.Format("%d/%m/%Y"),(const char*)m_map,m_playerCount);
	// player info
	for(i=0;i<m_playerCount;i++)
	{
		CString playerDef;
		playerDef.Format("%s \\%d\\%d\\%d\\%d\\", m_player[i],m_apm[i],m_race[i],m_apmDev[i],m_start[i]);
		desc += playerDef;
	}
	//duration
	CString strDuration;
	strDuration.Format("%d\\", m_duration);
	desc += strDuration;
	//engine version
	strDuration.Format("%02d:%d\\", m_engineType,m_engineVersion);
	desc += strDuration;
	//RWA flag & author
	strDuration.Format("%d:%s\\",m_isRWA?1:0,(const char*)m_author);
	desc += strDuration;
	//hack count
	strDuration.Format("%d\\",m_hackCount);
	desc += strDuration;

	// write to file
	CString tmpDir;
	BWChartDB::WriteEntry(file,Dir(tmpDir),Name(),desc);

	// dont write comments or bo if we are not writing into the main replays list
	if(file!=BWChartDB::FILE_MAIN) return;

	// write comments to file
	if(!m_comment.IsEmpty())
	{
		// replace EOL with special char
		CString com(m_comment);
		com.Replace(SRCEOL,TOKENEOL);
		// write to file
		BWChartDB::WriteEntry(BWChartDB::FILE_COMMENTS,Dir(tmpDir),Name(),com);
	}

	// bo info (for all players)
	SaveBO();
}

//-----------------------------------------------------------------------------------------------------------------

void ReplayInfo::SaveBO()
{
	CString desc;

	// bo info (for all players)
	for(int i=0;i<m_playerCount;i++)
	{
		CString boDef;
		boDef.Format("%s\\", (const char*)m_bo[i]);
		desc += boDef;
	}

	// write bos to file
	CString tmpDir;
	BWChartDB::WriteEntry(BWChartDB::FILE_BOS,Dir(tmpDir),Name(),desc,false);
}

//-----------------------------------------------------------------------------------------------------------------

static char *strtokbis( char *strToken, const char *delimiter )
{
	static char *next=0;
	if(strToken==0) strToken = next;
	if(strToken==0) return 0;
	next=strchr(strToken,delimiter[0]);
	if(next!=0) {*next=0; next++; return strToken;}
	return 0;
}

//-----------------------------------------------------------------------------------------------------------------

// dir/file/data must be in regular format
bool ReplayInfo::ExtractInfo(const char *dir, const char *file, char *data)
{
	// rebuild path to replay
	char path[256];
	strcpy(path,dir); 
	if(path[strlen(path)-1]!='\\') strcat(path,"\\");
	strcat(path,file);
	m_path=path;

	// read info from file
	// format:  version \ file date \ game date \ map \ player count \ {player\apm\race\apmdev} \ duration \ engine

	// line version
	char *p=strtokbis(data,"\\");
	if(p==0) return false;
	int version=atoi(p);
	// file date
	p=strtokbis(0,"\\");
	if(p==0) return false;
	m_filedate=CTime(atoi(p+6),atoi(p+3),atoi(p),0,0,0);
	// game date
	p=strtokbis(0,"\\");
	if(p==0) return false;
	m_date=CTime(atoi(p+6),atoi(p+3),atoi(p),0,0,0);
	// map name
	p=strtokbis(0,"\\");
	if(p==0) return false;
	m_map=p;
	p=strtokbis(0,"\\");
	if(p==0) return false;
	m_playerCount=atoi(p);
	for(int i=0;i<m_playerCount;i++)
	{
		//player name
		p=strtokbis(0,"\\");
		if(p==0) return false;
		m_player[i]=p;
		// apm
		p=strtokbis(0,"\\");
		if(p==0) return false;
		m_apm[i]=atoi(p);				  
		// race
		p=strtokbis(0,"\\");
		if(p==0) return false;
		m_race[i]=atoi(p);
		// apm dev
		p=strtokbis(0,"\\");
		if(p==0) return false;
		m_apmDev[i]=atoi(p);
		// start location
		if(version>VER_P)
		{
			p=strtokbis(0,"\\");
			if(p==0) return false;
			m_start[i]=atoi(p);
		}
	}
	p=strtokbis(0,"\\");
	if(p==0) return false;
	m_duration=atoi(p);

	// engine version (optional)
	p=strtokbis(0,"\\");
	if(p!=0)
	{
		m_engineType=atoi(p);
		m_engineVersion=atoi(p+3);
	}

	// RWA flag (optional)
	p=strtokbis(0,"\\");
	if(p!=0)
	{
		m_isRWA=atoi(p)!=0;
		m_author=p+2;
	}

	// hack count (optional)
	p=strtokbis(0,"\\");
	if(p!=0)
	{
		m_hackCount=atoi(p);
	}

	// try to load comments
	char buffini[2048];
	BWChartDB::ReadEntry(BWChartDB::FILE_COMMENTS,dir,file,buffini,sizeof(buffini));
	if(buffini[0]!=0)
	{
		// replace EOL with special char
		CString com(buffini);
		com.Replace(TOKENEOL,SRCEOL);
		m_comment = com;
	}

	// try to load bo
	BWChartDB::ReadEntry(BWChartDB::FILE_BOS,dir,file,buffini,sizeof(buffini),false);
	if(buffini[0]!=0)
	{
		int pidx=0;
		char *p=strtokbis(buffini,"\\");
		while(p!=0)
		{
			m_bo[pidx++]=p;
			p=strtokbis(0,"\\");
		}
	}

	return true;
}

//-----------------------------------------------------------------------------------------------------------------

bool ReplayInfo::IsFavorite()
{
	char buffini[2048];
	CString tmpDir;
	BWChartDB::ReadEntry(BWChartDB::FILE_FAVORITES,Dir(tmpDir),Name(),buffini,sizeof(buffini));
	return (buffini[0]!=0);
}

//-----------------------------------------------------------------------------------------------------------------

void ReplayInfo::Delete(int nfile)
{
	// remove replay from list file
	CString tmpDir;
	BWChartDB::Delete(nfile,Dir(tmpDir),Name());
}

//-----------------------------------------------------------------------------------------------------------------

// get matchup
int ReplayInfo::GetMatchup() const 
{
	if(m_matchUp==MU_UNKNOWN)
	{
		if(m_race[0]==IStarcraftPlayer::RACE_TERRAN && m_race[1]==IStarcraftPlayer::RACE_TERRAN) m_matchUp=MU_TvT;
		else if(m_race[0]==IStarcraftPlayer::RACE_TERRAN && m_race[1]==IStarcraftPlayer::RACE_ZERG) m_matchUp=MU_TvZ;
		else if(m_race[0]==IStarcraftPlayer::RACE_PROTOSS && m_race[1]==IStarcraftPlayer::RACE_TERRAN) m_matchUp=MU_PvT;
		else if(m_race[0]==IStarcraftPlayer::RACE_PROTOSS && m_race[1]==IStarcraftPlayer::RACE_ZERG) m_matchUp=MU_PvZ;
		else if(m_race[0]==IStarcraftPlayer::RACE_PROTOSS && m_race[1]==IStarcraftPlayer::RACE_PROTOSS) m_matchUp=MU_PvP;
		else if(m_race[0]==IStarcraftPlayer::RACE_ZERG && m_race[1]==IStarcraftPlayer::RACE_ZERG) m_matchUp=MU_ZvZ;
		else if(m_race[1]==IStarcraftPlayer::RACE_TERRAN && m_race[0]==IStarcraftPlayer::RACE_TERRAN) m_matchUp=MU_TvT;
		else if(m_race[1]==IStarcraftPlayer::RACE_TERRAN && m_race[0]==IStarcraftPlayer::RACE_ZERG) m_matchUp=MU_TvZ;
		else if(m_race[1]==IStarcraftPlayer::RACE_PROTOSS && m_race[0]==IStarcraftPlayer::RACE_TERRAN) m_matchUp=MU_PvT;
		else if(m_race[1]==IStarcraftPlayer::RACE_PROTOSS && m_race[0]==IStarcraftPlayer::RACE_ZERG) m_matchUp=MU_PvZ;
		else if(m_race[1]==IStarcraftPlayer::RACE_PROTOSS && m_race[0]==IStarcraftPlayer::RACE_PROTOSS) m_matchUp=MU_PvP;
		else if(m_race[1]==IStarcraftPlayer::RACE_ZERG && m_race[0]==IStarcraftPlayer::RACE_ZERG) m_matchUp=MU_ZvZ;

	}
	return m_matchUp;
}

//-----------------------------------------------------------------------------------------------------------------
