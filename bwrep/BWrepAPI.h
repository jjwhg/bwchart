//----------------------------------------------------------------------------------------------------
// Replay Loader - HurtnTime+jca+fractaled (May 2003)
//----------------------------------------------------------------------------------------------------
// Classes you should use:
//
// class BWrepFile   : object to use to load a replay
// class BWrepHeader : contains replay header (replay creator, map name, all players info, etc)
// class BWrepPlayer : contains information on a player (given by BWrepHeader object)
//----------------------------------------------------------------------------------------------------
#ifndef INC_BWREPAPI_H
#define INC_BWREPAPI_H

#include "dllexports.h"
#include "bwrepactions.h"
#include "bwrepgamedata.h"
#include "bwrepmap.h"
#include "replayinterface.h"

#include <stdio.h> 
#include <time.h> 

#ifndef BWLIB_NOACTIONS
// comment that line to compile without the BWRepActions.cpp file
#define USE_ACTIONS_CODE
	#ifndef BWLIB_NOMAP
		// comment that line to compile without the BWRepMap.cpp file
		#define USE_MAP_CODE
		// comment that line to compile without the RWA stuff
		#define USE_RWA_CODE
	#endif
#endif

//using namespace std;

const int kBWREP_PLAYERNAME_SIZE = 0x19;
const int kBWREP_HEADER_SIZE	 = 0x279;
const int kBWREP_ID			     = 0x53526572;
const int kBWREP_NUM_PLAYERS	 = 12;
const int kBWREP_NAME_SIZE		 = 24;
const int kBWREP_MAPNAME_SIZE	 = 26;
const int kBWREP_NUM_SLOT		 = 8;
const int kBWREP_GNAME_SIZE      = 28;

#pragma pack(push, 1)

//
// player info
//
class _BWrepPlayer
{
public:
	long m_number;									// 0-11
	long m_slot;									// -1 if computer or none, else 0-7
	char m_type;
	char m_race;
	char m_unknown;									// normally 0, only for race 6 it is 1
	char m_name[kBWREP_PLAYERNAME_SIZE];
};

//
// player info
//
class DllExport BWrepPlayer : public IStarcraftPlayer
{
public:
	BWrepPlayer();
	~BWrepPlayer();

	//
	// Query functions
	//
	virtual const char*	getName()		const;
	virtual long		getSlot()		const;
	virtual TYPE		getType()		const;
	virtual RACE		getRace()		const;
	virtual const char	getUnknown()	const;

	// Race of player
	virtual bool isTerran()				const;
	virtual bool isZerg()				const;
	virtual bool isProtoss()			const;

	// Type of player
	virtual bool isPlayer()				const;
	virtual bool isComputer()			const;
	virtual bool isEmpty()				const;

	//
	// Edit functions: for use by BWrepHeader
	//
	bool setName(const char* szName);
	bool setSlot(long newSlot);
	bool setType(TYPE newType);
	bool setRace(RACE newRace);
	bool setUnknown(const char newUnknown);

	long m_number;									// 0-11
private:
	long m_slot;									// -1 if computer or none, else 0-7
	char m_type;
	char m_race;
	char m_unknown;									// normally 0, only for race 6 it is 1
	char m_name[kBWREP_PLAYERNAME_SIZE];
};

//
// rep header
//
class DllExport BWrepHeader : public IStarcraftGame
{
public:
	BWrepHeader();
	~BWrepHeader();

	//
	// Query functions
	//
	virtual int getEngine() const {return m_engine;}
	virtual long getGameLength() const {return m_frames;}  // in ticks
	virtual const char*	getGameName()			const;
	virtual const char*	getGameCreatorName()	const;
	virtual const char* getMapName()			const;
	virtual char		getMapType()			const;
	virtual unsigned short getMapWidth() const {return m_mapsizeW;}
	virtual unsigned short getMapHeight() const {return m_mapsizeH;}
	virtual time_t		getCreationDate()			const {return m_creationDate;}

	// get player info from the playerid in BWrepAction
	virtual bool		getPlayerFromAction(const IStarcraftPlayer*& player, int playerid) const;

	// get player from index in player array (used for playerid in BWrepUnitDesc)
	virtual bool		getPlayerFromIdx(const IStarcraftPlayer*& player, int idx) const;

	// Logical queries
	virtual long		getLogicalPlayerCount()	const;
	virtual bool		getLogicalPlayers(const IStarcraftPlayer*& player, int i) const;

	// get player color
	virtual int GetPlayerColor(int idx) {return m_spotcolor[idx];}

	// get player id from a spot index
	virtual int GetPlayerIDFromSpot(int spotidx) const;

	// convert tick to seconds
	virtual int Tick2Sec(unsigned long tick) const ;

	// convert seconds to tick
	virtual unsigned long Sec2Tick(int sec) const;

	// build object name: if we have the object id interface, convert unitid to a real object name, otherwise simply use the unit id
	virtual const char *MkUnitID2String(char *buffer, unsigned short unitid, const IUnitIDToObjectID *interf, unsigned long time) const
	{
		return BWrepAction::MkUnitID2String(buffer,unitid,interf,time);
	}

	//
	// Edit functions
	//
	bool setGameName(const char* szName);
	bool setGameCreatorName(const char* szName);
	bool setMapType(char cMapType);
	bool setMapName(const char* szName);
	void checkPlayerNameUnicity();

    char		m_engine;							// is 0x01 BW, 0x00 for SC

private:
    long		m_frames;
    char		m_fillb;							// is 0x00 everytime
    char		m_fillc;							// is 0x00 everytime
    char		m_filld;							// is 0x48 everytime
    time_t      m_creationDate;
    char		m_ka2[8];							// every byte is 0x08
    long		m_ka3;								// is 0x00 everytime
    char		m_gamename[kBWREP_GNAME_SIZE];
	unsigned short m_mapsizeW;
	unsigned short m_mapsizeH;
    char		m_fill2[16];
    char		m_gamecreator[kBWREP_NAME_SIZE];
    char		m_maptype;							// 0x3C for bw reps
    char		m_mapname[kBWREP_MAPNAME_SIZE];
    char		m_fill3[38];
    _BWrepPlayer m_player[kBWREP_NUM_PLAYERS];
    long		m_spotcolor[8];						// player color (thx to TravelToAiur)
    char		m_spot[8];							// unknown, flag if spot[n] is used

	// copy with interface
	BWrepPlayer m_oPlayer[kBWREP_NUM_PLAYERS];
};

//----------------------------------------------------------------------------------------------------

#pragma pack(pop)

//
// user rep file _access
//
class DllExport BWrepFile : public IStarcraftReplay
{
private:
	// replay file handle
	FILE*		m_pFile;

	// offset of RWA (if any)
	unsigned long m_rwaoffset;

	bool _Open(const char* pszFileName);
	bool _Close();
	bool _LoadActions(FILE *fp, bool clear, bool decode);
	bool _LoadMap(FILE *fp, bool decode);

	// load extra information stored after the regular replay data
	bool _LoadExtra(void *rwaheader, int hdrsize);

	// replay header
	BWrepHeader m_oHeader;

	// actions list
	#ifdef USE_ACTIONS_CODE
	BWrepActionList m_oActions;
	#endif

	// game map
	#ifdef USE_MAP_CODE
	BWrepMap m_oMap;
	#endif

public:
	// ctor
	BWrepFile();
	virtual ~BWrepFile();

	// load replay (at least the header)
	virtual bool Load(const char* pszFileName, int options=LOADMAP|LOADACTIONS, void *rwaheader=0, int size=0);

	// get offset in file of audio part (if any, 0 if none)
	virtual unsigned long GetAudioOffset(const char* pszFileName, void *header, int size);

	// replay header
	virtual const IStarcraftGame* QueryHeader() const {return &m_oHeader;}

	// actions list
	virtual const IStarcraftActionList* QueryActions() const {return &m_oActions;}

	// game map
	virtual const IStarcraftMap* QueryMap() const {return &m_oMap;}

	// release object
	virtual void Release() {delete this;}
};


#endif // INC_BWREPAPI_H