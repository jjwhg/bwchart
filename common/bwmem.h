#ifndef __BWMEM_H
#define __BWMEM_H





//
// player info
//
class BWrepPlayerInfo
{
public:
	enum RACE
	{
		RACE_ZERG, 
		RACE_TERRAN, 
		RACE_PROTOSS, 
		RACE_RANDOM=6	
	};

	enum TYPE
	{
		TYPE_NONE, 
		TYPE_COMPUTER, 
		TYPE_PLAYER
	};

	long m_number;									// 0-11
	long m_slot;									// -1 if computer or none, else 0-7
	char m_type;
	char m_race;
	char m_unknown;									// normally 0, only for race 6 it is 1
	char m_name[25];
};

class BWMem
{
public:
	static void ReadBytes(HANDLE hProcess, char *adr, char *buffer, int size);
	static void WriteBytes(HANDLE hProcess, char *adr, const char *buffer, int size);
	static void ReadPlayerDesc(HANDLE hProcess, BWrepPlayerInfo *player, int idx);
	static BWrepPlayerInfo *GetPlayerDesc(HANDLE hProcess);

	// display player message
	static void DisplayPlayerMessage(HANDLE hProcess, HWND hWndBW, const char *msgtext, int durationS);

	// get game time (in tick unit, divide by 24 to get seconds)
	static unsigned long GetTick(HANDLE hProcess);

	// get player mineral resources
	static unsigned long GetMineral(HANDLE hProcess);

	// get player gas resources
	static unsigned long GetGas(HANDLE hProcess);

	// get player supply
	static void GetSupply(HANDLE hProcess, int *used, int *available);

	// get replay file (if any)
	static const char *GetReplay(HANDLE hProcess, char *repfile);

	// set BW engine version
	enum {PATCH_UNKNOWN, PATCH_110, PATCH_111, PATCH_112};
	static void SetBWPatch(int patch);

	//get address for a specific info
	static unsigned long Addr(int info);

	// BW bytes	(6 changed into 4)
	enum
	{
	MINERAL1,	
	GameTime,	
	TICK	,	
	REPLAYFILE,	

	INGAMEBIS,  // from superpinguin

	MineralEnd,	

	MSGTEXT0,	// 218 bytes
	MSGTICK0,	// 4 bytes
	MSGINDEX,	// byte

	LOGIN,		 
	PlayerDesc	,
	Slot1		 ,
	Slot2		  ,
	Slot3		   ,
	Slot4			,
	Slot5			 ,
	Slot6			  ,
	Slot7			   ,
	Slot8				,
	BNETFLAG			 ,
	GAMEBNET			  ,
	MAPBNET				   ,
	HPLAYER					,

	
	INGAME		 ,// including game lobby on bnet, but not game results
	GAME		  ,
	MAP			   ,

	REALMINERAL		,
	GAS				 ,
	SUPPLY_STR		  ,
		__BWMEM__MAX
	};
private:
	static BWrepPlayerInfo *_GetAnyPlayerDesc(HANDLE hProcess, const char *playerName);

	// bw engien version (patch number)
	static int m_patch;

	// BW bytes	addresses
	static unsigned long gByteAddress[__BWMEM__MAX];

};


class BWGameDetection
{
	bool m_startConditions[2];
	unsigned long m_gameTime;
	unsigned long m_lastGameMineralEnd;
	bool m_bIsReplay;
	bool m_bIsBNET;
	bool m_bGameInProgress; // real game, not a replay
	char m_playerName[32];

	bool _bGameStarted() const;

public:
	BWGameDetection() : m_gameTime(0), m_lastGameMineralEnd(0),
	m_bIsReplay(false), m_bIsBNET(false) {}

	bool IsReplay() const {return m_bIsReplay;}
	bool IsBNET() const {return m_bIsBNET;}
	const char *PlayerName() const {return m_playerName;}

	void ResetGameInfo(HANDLE hProcess);
	bool IsGameStarted(HANDLE hProcess);
	bool CheckEndOfGame(HANDLE hProcess);
	bool IsReplayInProgress(HANDLE hProcess);
	bool IsGameInProgress(HANDLE hProcess);
};

#endif
