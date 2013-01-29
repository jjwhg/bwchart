
#pragma once

//-----------------------------------------------
//
// player info
//
class IStarcraftPlayer
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

	//
	// Query functions
	//
	virtual const char*	getName()		const=0;
	virtual long		getSlot()		const=0;
	virtual TYPE		getType()		const=0;
	virtual RACE		getRace()		const=0;
	virtual const char	getUnknown()	const=0;

	// Race of player
	virtual bool isTerran()				const=0;
	virtual bool isZerg()				const=0;
	virtual bool isProtoss()			const=0;

	// Type of player
	virtual bool isPlayer()				const=0;
	virtual bool isComputer()			const=0;
	virtual bool isEmpty()				const=0;
};

//-----------------------------------------------

//interface
class IUnitIDToObjectID
{
public:
	// convert a unitid to an objectid (depends on time because of some units morphing into others)
	virtual short Convert(short unitID, unsigned long time) const=0;
};

//-----------------------------------------------

//
// rep header
//
class IStarcraftGame
{
public:
	enum ENGINE
	{
		ENGINE_STARCRAFT=0, 
		ENGINE_BROODWAR=1, 
		ENGINE_STARCRAFT2=2
	};

	//
	// Query functions
	//
	virtual int getEngine() const=0;
	virtual long getGameLength() const=0;
	virtual const char*	getGameName()			const=0;
	virtual const char*	getGameCreatorName()	const=0;
	virtual const char* getMapName()			const=0;
	virtual char		getMapType()			const=0;
	virtual unsigned short getMapWidth() const=0;
	virtual unsigned short getMapHeight() const=0;
	virtual time_t		getCreationDate()			const=0;

	// get player info from the playerid in IStarcraftAction
	virtual bool		getPlayerFromAction(const IStarcraftPlayer*& player, int playerid) const=0;

	// get player from index in player array (used for playerid in BWrepUnitDesc)
	virtual bool		getPlayerFromIdx(const IStarcraftPlayer*& player, int idx) const=0;

	// Logical queries
	virtual long		getLogicalPlayerCount()	const=0;
	virtual bool		getLogicalPlayers(const IStarcraftPlayer*& player, int i) const=0;

	// get player color
	enum {CLR_RED,CLR_BLUE,CLR_PURPLE,CLR_ORANGE,CLR_BROWN,CLR_WHITE,CLR_YELLOW};
	virtual int GetPlayerColor(int idx)=0;

	// get player id from a spot index
	virtual int GetPlayerIDFromSpot(int spotidx) const=0;

	// convert tick to seconds
	virtual int Tick2Sec(unsigned long tick) const =0;

	// convert seconds to tick
	virtual unsigned long Sec2Tick(int sec) const =0;

	// build object name: if we have the object id interface, convert unitid to a real object name, otherwise simply use the unit id
	virtual const char *MkUnitID2String(char *buffer, unsigned short unitid, const IUnitIDToObjectID *interf, unsigned long time) const=0;
};

//-----------------------------------------------

// any action
class IStarcraftAction
{
public:
	// action time (in "tick" units. Divide by 23 to get the approximate time in seconds. not very accurate)
	virtual unsigned long GetTime() const=0;

	// action name
	virtual const char *GetName() const=0;

	// action id (returns a eACTIONNAME from BWrepGameData.h)
	virtual int GetID() const=0;

	// player id (use IStarcraftGame::getPlayer to get player name)
	virtual int GetPlayerID() const=0;

	// parameters as text
	virtual const char *GetParameters(IUnitIDToObjectID *interf=0) const=0;

	// units IDs as text
	virtual const char *GetUnitsID(IUnitIDToObjectID *interf) const=0;

	// pointer on parameters (must be casted to the correct BWrepActionXXX::Params)
	virtual const void *GetParamStruct(int* pSize=0) const=0;

	// to associate user data with an action
	enum {MAXUSERDATA=2};
	virtual void SetUserData(int idx, unsigned long data)=0;
	virtual unsigned long GetUserData(int idx) const=0;
};

//-----------------------------------------------

// decoded actions list (it's an array really)
class IStarcraftActionList
{
public:
	// get pointer on nth action
	virtual const IStarcraftAction *GetAction(int i) const=0;

	// get action count
	virtual int GetActionCount() const=0;
};

//-----------------------------------------------

// section info
class IStarcraftMapSection
{
public:
	virtual const char *GetTitle() const=0;
	virtual unsigned long GetSize() const=0;
	virtual const unsigned char *GetData() const=0;
};

//----------------------------------------------------------------------------------------------------

// map info
class IStarcraftMap
{
public:
	// map dimensions
	virtual int GetWidth() const=0;
	virtual int GetHeight() const=0;

	// find section by name
	virtual const IStarcraftMapSection* GetSection(const char *name) const=0;

	// get tile section info (2 bytes per map square)
	virtual const IStarcraftMapSection* GetTileSection() const=0;
};


//-----------------------------------------------

// replay interface
class IStarcraftReplay
{
public:
	// load replay (at least the header)
	enum {LOADMAP=1, LOADACTIONS=2, ADDACTIONS=4};
	virtual bool Load(const char* pszFileName, int options=LOADMAP|LOADACTIONS, void *rwaheader=0, int size=0)=0;

	// get offset in file of audio part (if any, 0 if none)
	virtual unsigned long GetAudioOffset(const char* pszFileName, void *header, int size)=0;

	// replay header
	virtual const IStarcraftGame* QueryHeader() const=0;

	// actions list
	virtual const IStarcraftActionList* QueryActions() const=0;

	// game map
	virtual const IStarcraftMap* QueryMap() const=0;

	// release object
	virtual void Release()=0;
};

//-----------------------------------------------

// replay factory interface
class IStarcraftReplayFactory
{
public:
	// create replay instance
	virtual IStarcraftReplay *CreateReplayInstance(const char *filename)=0;
};


// globale methods to get a factory object
IStarcraftReplayFactory *QueryFactory();

//-----------------------------------------------
