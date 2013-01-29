#ifndef _REPLAYDB_H
#define _REPLAYDB_H

extern const char * _ConverToHex(const char *str);
extern char * _ConverFromHex(const char *str);

#define TAG_VERSION "__VERSION_"

#define BWREP_VERSION_109 0x1D
#define BWREP_VERSION_110 0x1E
#define BWREP_VERSION_111 0x1F
#define BWREP_VERSION_112 0x20
#define BWREP_VERSION_113 0x21
#define BWREP_VERSION_114 0x22
#define BWREP_VERSION_115 0x23
#define BWREP_VERSION_116 0x24

#include "../common/audioheader.h"

//------------------------------------------------------------

class ReplayInfo : public CObject
{
private:
	static void _BuildUserDataFileName(CString& rpath, const char *file);
	mutable int m_matchUp;

public:
	// ctor
	ReplayInfo() : m_engineType(0), m_engineVersion(0), m_matchUp(0), m_isRWA(false)
	{
		memset(m_apm,0,sizeof(m_apm));
		memset(m_race,0,sizeof(m_race));
		memset(m_start,0,sizeof(m_start));
		memset(m_apmDev,0,sizeof(m_apmDev));
		m_playerCount=0;
		m_hackCount=0;
	}
	ReplayInfo(const ReplayInfo& src)
	{
		m_playerCount=src.m_playerCount;
		m_hackCount=src.m_hackCount;
		m_path = src.m_path;
		m_map = src.m_map;
		m_mainMap = src.m_mainMap;
		m_date = src.m_date;
		m_filedate = src.m_filedate;
		m_comment = src.m_comment;
		m_duration = src.m_duration; // in seconds
		m_engineType=src.m_engineType;
		m_engineVersion=src.m_engineVersion;
		m_matchUp=src.m_matchUp;
		m_isRWA=src.m_isRWA;
		m_author=src.m_author;
		for(int i=0;i<MAXPLAYER;i++) 
		{
			m_mainName[i]=src.m_mainName[i]; 
			m_player[i] = src.m_player[i]; 
			m_apm[i]=src.m_apm[i]; 
			m_race[i]=src.m_race[i]; 
			m_start[i]=src.m_start[i]; 
			m_apmDev[i]=src.m_apmDev[i];
			m_bo[i]=src.m_bo[i];
		}
	}

	// get matchup
	enum {MU_UNKNOWN, MU_TvT, MU_TvZ, MU_PvT, MU_PvZ, MU_PvP, MU_ZvZ};
	int GetMatchup() const;

	// init/exit instance
	static void InitInstance();
	static void ExitInstance();		

	// engine info
	unsigned short m_engineType;
	unsigned short m_engineVersion;

	// file path
	CString m_path;

	// RWA?
	bool m_isRWA;
	CString m_author;

	// map name
	CString m_map;
	CString m_mainMap;

	// date of game creation
	CTime m_date;

	// date of file creation
	CTime m_filedate;

	// player count
	int m_playerCount;

	// hack count
	int m_hackCount;

	// player names
	enum {MAXPLAYER=8};
	CString m_player[MAXPLAYER];
	CString m_mainName[MAXPLAYER];

	// player apm
	int m_apm[MAXPLAYER];

	// player apm dev
	int m_apmDev[MAXPLAYER];

	// player race
	int m_race[MAXPLAYER];

	// player starting location
	int m_start[MAXPLAYER];

	// overall build order
	CString m_bo[MAXPLAYER];

	// game duration
	int m_duration; // in seconds

	// comment
	CString m_comment;

	// save replay
	void Save(int nfile);
	void SaveBO();

	// load replay
	enum {VER_F=0,VER_N=1, VER_P=2, VER_CURRENT};
	bool ExtractInfo(const char *dir, const char *file, char *data);

	// delete replay
	void Delete(int nfile);

	// return if replay is in favorites
	bool IsFavorite();

	// read line from replay file
	static void ReadLine(int nfile, const char *cnvDir, const char *cvnName, char *buffer, int bufsize);

	const char *Name() const
	{
		const char *p=strrchr(m_path,'\\');
		if(p!=0) p++; else p=m_path;
		return p;
	}

	const char *Dir(CString& tmp) const
	{
		tmp= m_path.Left(m_path.ReverseFind('\\'));
		return tmp;
	}

	// game date for sorting
	const char *DateForCompare(CString& str)
	{
		static int thisYear=0;
		if(thisYear==0) thisYear = CTime::GetCurrentTime().GetYear();
		if(m_date.GetYear()<1995 || m_date.GetYear()>thisYear)
			str = "!";
		else
			str = m_date.Format("%Y%m%d");
		return str;
	}

	// game date for display
	const char *DateForDisplay(CString& str)
	{
		static int thisYear=0;
		if(thisYear==0) thisYear = CTime::GetCurrentTime().GetYear();
		if(m_date.GetYear()<1995 || m_date.GetYear()>thisYear)
			str = "Unknown";
		else
			str = m_date.Format("%d %b %Y");
		return str;
	}

	const char *FileDate(CString& str)
	{
		str = m_filedate.Format("%Y%m%d");
		return str;
	}

	const char *Duration(CString& str)
	{
		int totals = m_duration;
		int h = totals/3600; totals-=h*3600;
		int m = totals/60; totals-=m*60;
		int s = totals;
		str.Format("%02d:%02d:%02d",h,m,s);
		return str;
	}

	const char *EngineVersion(CString& str)
	{
		if(m_engineType==0) str="SC";
		else str.Format("BW %s",m_engineVersion<BWREP_VERSION_110 ? "1.09" : 
								m_engineVersion<BWREP_VERSION_111 ? "1.10" : 
								m_engineVersion<BWREP_VERSION_112 ? "1.11" :
								m_engineVersion<BWREP_VERSION_113 ? "1.12" :
								m_engineVersion<BWREP_VERSION_114 ? "1.13" :
								m_engineVersion<BWREP_VERSION_115 ? "1.14" :
								m_engineVersion<BWREP_VERSION_116 ? "1.15" :"1.16");
		return str;
	}

	const char *GameType() const
	{
		// game type (based on enabled players count)
		if(m_playerCount>6) return "4v4";
		else if(m_playerCount==6) return "3v3";
		else if(m_playerCount==5) return "ffa";
		else if(m_playerCount==4) return "2v2";
		else if(m_playerCount==3) return "ffa";
		return "1v1";
	}
};

class BaseInfo : public CObject
{
public:
	BaseInfo() : m_games(0), m_apm(0), m_duration(0),m_apmAsT(0),m_apmAsZ(0),m_apmAsP(0),
			m_gamesAsT(0),m_gamesAsZ(0),m_gamesAsP(0){}

	CString m_name;
	int m_games;
	int m_gamesAsT;
	int m_gamesAsZ;
	int m_gamesAsP;
	int m_apm;
	int m_apmAsT;
	int m_apmAsZ;
	int m_apmAsP;
	int m_duration; // in seconds

	int AvgApm() const {return m_games==0?0:m_apm/m_games;}
	int AvgDur() const {return m_games==0?0:m_duration/m_games;}
	int AvgApmAsT() const {return m_gamesAsT==0?0:m_apmAsT/m_gamesAsT;}
	int AvgApmAsZ() const {return m_gamesAsZ==0?0:m_apmAsZ/m_gamesAsZ;}
	int AvgApmAsP() const {return m_gamesAsP==0?0:m_apmAsP/m_gamesAsP;}

	const char *AvgDuration(CString& str)
	{
		int totals = m_games==0 ? 0:m_duration/m_games;
		int h = totals/3600; totals-=h*3600;
		int m = totals/60; totals-=m*60;
		int s = totals;
		str.Format("%02d:%02d:%02d",h,m,s);
		return str;
	}
};

class PlayerInfo : public BaseInfo
{
	int m_raceDist[4];
	int m_apmDev;
public:
	PlayerInfo() {m_raceDist[0]=m_raceDist[1]=m_raceDist[2]=m_raceDist[3]=0;m_apmDev=0;}

	void AddRace(int race) {m_raceDist[race]++;}
	void AddApmDev(int apmdev) {m_apmDev+=apmdev;}
	int GetRaceCount(int race) const {int tot=(m_raceDist[0]+m_raceDist[1]+m_raceDist[2]+m_raceDist[3]); return tot==0?0:(100*m_raceDist[race])/tot;}
	int GetApmDev() const {return m_games==0?0:m_apmDev/m_games;}
	int GetApmDevPer() const {return (m_games==0 || AvgApm()==0)?0:(100*GetApmDev())/AvgApm();}
};

class MapInfo : public BaseInfo
{
public:
	MapInfo() : m_apmCount(0) {}
	int m_apmCount;

	int AvgApm() const {return m_apmCount==0?0:m_apm/m_apmCount;}
	int AvgDur() const {return m_games==0?0:m_duration/m_games;}
};

//------------------------------------------------------------

#endif