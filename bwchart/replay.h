#ifndef __replay_h
#define __replay_h

#include "memblock.h"
#include "BWrepAPI.h"
#include "BWrepActions.h"
#include "bwmap.h"

class BOTree;
class BONodeList;

#include "../common/audioheader.h"

#include <assert.h>

#define MINAPMVALIDTIME 2880  // 2'00
#define MINAPMVALIDTIMEFORMAX 4320  // 3'00
#define MAXACTIONPERMINUTE 800
#define MAXSELECTION 12
#define IdxAction(cmd,subcmd) (subcmd==-1 ? cmd : BWrepGameData::_CMD_MAX_+subcmd)
#define RES_INTERVAL_TICK 25 // ticks

extern const char *_MkTime(const IStarcraftGame *header, unsigned long time, bool hhmmss);

//--- default chart colors -------------

#define DEF_CLR_MINERAL	RGB(90,90,200)
#define DEF_CLR_GAS RGB(0,200,0)
#define DEF_CLR_SUPPLY RGB(200,200,0)
#define DEF_CLR_UNITS RGB(200,0,0)
#define DEF_CLR_APM RGB(255,206,73)
#define DEF_CLR_BPM RGB(0,196,196)
#define DEF_CLR_UPM RGB(128,0,128)
#define DEF_CLR_MICRO RGB(32,0,140)
#define DEF_CLR_MACRO RGB(160,0,16)
#define DEF_CLR_MAPCOVERAGE RGB(73,206,255)
#define DEF_CLR_MMCOVERAGE RGB(206,73,73)

//--- default chart line size ---------

#define DEF_LSIZE_MINERAL 2
#define DEF_LSIZE_GAS 2
#define DEF_LSIZE_SUPPLY 1
#define DEF_LSIZE_UNITS 1
#define DEF_LSIZE_APM 1
#define DEF_LSIZE_BPM 1
#define DEF_LSIZE_UPM 1
#define DEF_LSIZE_MICRO 1
#define DEF_LSIZE_MACRO 1
#define DEF_LSIZE_MAPCOVERAGE 2
#define DEF_LSIZE_MMCOVERAGE 1

//------------------------------------------------------------------------------------------------------------

// event type
class ReplayEvtType
{
public:
	ReplayEvtType(int cmd, int subcmd=-1) : m_cmd(cmd), m_subcmd(subcmd) {}
	unsigned short m_cmd;
	short m_subcmd;

	int GetIdxAction() const {return IdxAction(m_cmd,m_subcmd);}
	bool operator ==(const ReplayEvtType& src) const {return src.m_cmd==m_cmd && src.m_subcmd==m_subcmd;}

	const char *GetType() const;

	static const char *GetTypeFromIdx(int idx);
	static COLORREF GetTypeColor(int idx);
};

//------------------------------------------------------------------------------------------------------------

class ReplayResource
{
public:
	enum {A_TOTAL, A_BUILD,A_TRAIN,A_OTHER,A_HOTKEY,A_MICRO,A_MACRO,__A_MAX};

private:
	bool m_initDone;

	unsigned short m_actionCount[__A_MAX];

	// resources at a specific time
	unsigned short m_supply; // supply *2 to handle 0.5 supply values
	unsigned long m_minerals;
	unsigned long m_gaz;
	unsigned short m_units;

	//activity measurement
	unsigned short m_actionPerMinute;
	unsigned short m_buildPerMinute;
	unsigned short m_unitPerMinute;
	unsigned short m_legalActionPerMinute;
	unsigned short m_microAPM;
	unsigned short m_macroAPM;
	unsigned __int64 m_mapCoverageUnit;
	unsigned short m_mapCoverageBuild;
	unsigned short m_mapMovingMapCoverage;

public:
	ReplayResource() {Clear();}
	void Clear(){memset(this,0,sizeof(*this));}
	void ClearAPM()
	{
		// reset activity measurement maximums
		SetAPM(0);
		SetLegalAPM(0);
		SetBPM(0);
		SetUPM(0);
		SetMicroAPM(0);
		SetMacroAPM(0);
	}
	// init done?
	bool IsInitDone() const {return m_initDone;}
	void SetInitDone() {m_initDone=true;}

	// init by cloning other resource
	void Clone(const ReplayResource& res) {*this=res;memset(m_actionCount,0,sizeof(m_actionCount));}

	// add action in time slot
	void AddAction(int type, bool updateTotal=true) {m_actionCount[type]++;if(updateTotal) m_actionCount[A_TOTAL]++;}
	void RemoveAction(int type, bool updateTotal=true) {if(m_actionCount[type]>0) {m_actionCount[type]--;if(updateTotal) m_actionCount[A_TOTAL]--;}}
	unsigned short GetActionCount(int type=-1) const {return (type==-1) ? _TotalAction() : m_actionCount[type];}
	unsigned short _TotalAction() const {return m_actionCount[A_TOTAL];}

	// read resources
	unsigned long Minerals() const {return m_minerals;}
	unsigned long Gaz() const {return m_gaz;}
	unsigned short Units() const {return m_units;}
	unsigned short Supply() const {return m_supply/2;}

	// read activity measurement
	unsigned short LegalAPM() const {return m_legalActionPerMinute;}
	unsigned short APM() const {return m_actionPerMinute;}
	unsigned short BPM() const {return m_buildPerMinute;}
	unsigned short UPM() const {return m_unitPerMinute;}
	unsigned short MicroAPM() const {return m_microAPM;}
	unsigned short MacroAPM() const {return m_macroAPM;}
	unsigned short MapCoverage() const {return m_mapCoverageBuild;}
	unsigned __int64 MapCoverageUnit() const {return m_mapCoverageUnit;}
	unsigned short MovingMapCoverage() const {return m_mapMovingMapCoverage;}

	// update resources
	void UpdateResourceBuild(int unitID);
	void UpdateResourceTrain(int unitID, int unitsToAdd);
	void UpdateResourceUpgrade(int techID);

	// update activity measurement
	void SetAPM(int apm) {m_actionPerMinute=(unsigned short)apm;}
	void SetMicroAPM(int apm) {m_microAPM=(unsigned short)apm;}
	void SetMacroAPM(int apm) {m_macroAPM=(unsigned short)apm;}
	void SetLegalAPM(int apm) {m_legalActionPerMinute=(unsigned short)apm;}
	void SetBPM(int apm) {m_buildPerMinute=(unsigned short)apm;}
	void SetUPM(int apm) {m_unitPerMinute=(unsigned short)apm;}
	void SetMapCoverage(int build) {m_mapCoverageBuild=(unsigned short)build;}
	void SetMapCoverageUnit(unsigned __int64 unit) {m_mapCoverageUnit=unit;}
	void SetMovingMapCoverage(int val) {m_mapMovingMapCoverage=(unsigned short)val;}

	// update max value
	void UpdateMax(const ReplayResource& res, bool updateAPM);

	// return value
	unsigned long Value(int i) const
	{
		if(i==0) return m_minerals;
		if(i==1) return m_gaz;
		if(i==2) return m_supply/2;
		if(i==3) return m_units;
		if(i==4) return APM();
		if(i==5) return BPM();
		if(i==6) return UPM();
		if(i==7) return MicroAPM();
		if(i==8) return MacroAPM();
		if(i==9) return MapCoverage();
		if(i==10) return MovingMapCoverage();
		return 0;
	}

	// return pre-defined colors for each resource
	enum {CLR_MINERAL,CLR_GAS,CLR_SUPPLY,CLR_UNITS,CLR_APM,CLR_BPM,CLR_UPM,CLR_MICRO,CLR_MACRO,CLR_MAPCOVERAGE,CLR_MMCOVERAGE,__CLR_MAX};
	static COLORREF m_gColors[__CLR_MAX];
	static int m_gLineSize[__CLR_MAX];
	static COLORREF GetUniqueColor(int i) {return i>=__CLR_MAX ? RGB(255,255,255) : m_gColors[i];}
	static int GetLineSize(int i) {return i>=__CLR_MAX ? 1 : m_gLineSize[i];}

	static int MaxValue() {return __CLR_MAX;}
	static COLORREF GetColor(int i, int player=-1, int maxplayer=-1);
	static int _FindTech(const char *parameters);
};

//------------------------------------------------------------------------------------------------------------

// event description
class ReplayEvt
{
private:
	const IStarcraftAction *m_action;
	ReplayEvtType m_type;
	unsigned char m_unitIdx; // unit if type is CMD_TRAIN, building if CMD_BUILD, tech if CMD_UPGRADE/CMD_RESEARCH
	bool m_bDiscarded;
	unsigned char m_selection;
	bool m_suspect;
	bool m_hack;
	bool m_isobj; // take info from gAllUnits, otherwise from gAllTechs

	// resources
	//ReplayResource m_res;
	class ReplayEvtList *m_parent;

public:
	// ctor
	ReplayEvt(ReplayEvtList *parent, const IStarcraftAction *action, unsigned char sel, int cmd, int subcmd, int objid, const ReplayEvt *prevEvt, bool suspect); 

	// update resources
	void UpdateResourceTrain(int unitID,int unitsToAdd);
	void UpdateResourceBuild(int unitID);
	void UpdateResourceUpgrade(int techID);

	// selection
	unsigned char GetSelection() const {return m_selection;}

	// event type (select, move, train, hatch, etc)
	const char * strType() const;
	const char * strTypeColor() const;
	const ReplayEvtType& Type() const {return m_type;}
	ReplayEvtType* GetTypePtr() {return &m_type;}
	int GetBWCoachID() const;

	// returns trues if event is suspect
	bool IsSuspect() const {return m_suspect;}
	void SetSuspect() {m_suspect=true;}

	// returns trues if event is a hack signature
	bool IsHack() const {return m_hack;}
	void SetHack() {m_hack=true;}

	// unit index
	void SetUnitIdx(int idx) {m_unitIdx=idx;} 
	int UnitIdx() const {return m_unitIdx;} // if type is TYP_TRAIN

	// discard
	bool IsDiscarded() const {return m_bDiscarded;}
	void Discard() {m_bDiscarded=true;}

	// resources
	const ReplayResource& Resources() const;
	ReplayResource* pResources();

	// action
	const IStarcraftAction *GetAction() const {return m_action;}
	IStarcraftAction *GetActionPtr() {return (IStarcraftAction *)m_action;}
	void SetAction(const IStarcraftAction *action) {m_action=action;}

	unsigned long Time() const {assert(m_action!=0); return m_action->GetTime();}
	int ActionID() const {return m_action->GetID();}
	COLORREF GetColor() const;

	static void Clear();

	DECLARE_LOCAL_HEAP(32768)
};

//------------------------------------------------------------------------------------------------------------

class ReplayObjectSequence
{
public:
	//Ctor
	enum {MAXBO=24};
	ReplayObjectSequence(int type=OBJECT,int maxc=16) : m_count(0), m_maxCount(maxc), m_objectType(type) {}

	// object type
	enum {OBJECT,RESEARCH,UPGRADE};

	// add object (building or unit) to build order
	void AddObject(unsigned long time, unsigned int objid);

	// remove object (building or unit) from build order
	void RemoveObject(unsigned long time, unsigned int objid);

	// insert object in ascending time (returns false if the object could not be inserted)
	bool InsertObject(unsigned long time, unsigned int objid);

	// update object time
	void UpdateTime(int idx, unsigned long time);

	// get object count
	int GetCount() const {return m_count;}

	// get time for ith object
	unsigned long GetTime(int i) const {return m_time[i];}

	// get ith object
	unsigned int GetObject(int i) const {return m_object[i];}

	// get ith object name
	const char *GetName(int i) const {return m_objectType==OBJECT ? BWrepGameData::GetObjectNameFromID(m_object[i]):
							m_objectType==RESEARCH ? BWrepGameData::GetResearchNameFromID(m_object[i]):
							BWrepGameData::GetUpgradeNameFromID(m_object[i]);}

	// get latest time
	unsigned long GetLastTime() const {return m_count==0 ? 0 : m_time[m_count-1];}

	// returns true if bo is full of objects
	bool IsFull() const {return m_count==m_maxCount;}

	// return bo as a string (one char per object, nmax objects maximum)
	const char *GetAsString(CString& str, int nmax) const;

private:
	// buildings or units
	unsigned long m_time[MAXBO];
	unsigned int m_object[MAXBO];
	int m_count;
	int m_maxCount;

	// object type
	int m_objectType;
};

//------------------------------------------------------------------------------------------------------------

class ReplayBuildOrder
{
private:
	ReplayObjectSequence m_buildings;
	ReplayObjectSequence m_units;
	ReplayObjectSequence m_research;
	ReplayObjectSequence m_upgrade;
public:
	//ctor
	ReplayBuildOrder() : 
	    m_units(ReplayObjectSequence::OBJECT,8),  // limit units to 8
		m_research(ReplayObjectSequence::RESEARCH), 
		m_upgrade(ReplayObjectSequence::UPGRADE)  {}

	ReplayObjectSequence& Buildings() {return m_buildings;}
	ReplayObjectSequence& Units() {return m_units;}
	ReplayObjectSequence& Research() {return m_research;}
	ReplayObjectSequence& Upgrade() {return m_upgrade;}
	const ReplayObjectSequence& Buildings() const {return m_buildings;}
	const ReplayObjectSequence& Units() const {return m_units;}
	const ReplayObjectSequence& Research() const {return m_research;}
	const ReplayObjectSequence& Upgrade() const {return m_upgrade;}

	void AddBuildOrder(int actionID, unsigned long time, int objectID);
	void RemoveBuildOrder(int actionID, unsigned long time, int objectID);

	void MakeBoNodeList(BONodeList* bo);
};

//------------------------------------------------------------------------------------------------------------

class BWElement
{
private:
	// time first identified
	unsigned long m_timeFirst;
	// unit id
	short m_unitID;
	// corresponding object id in time
	enum {MAXOBJID=5};
	short m_objectID[MAXOBJID];
	// time when object id is identified
	unsigned long m_time[MAXOBJID];
	// how many identities found
	short m_count;
	// owner event list
	class ReplayEvtList *m_owner;
public:
	BWElement(short unitid,unsigned long timeFirst, ReplayEvtList *owner) : 
	  m_timeFirst(timeFirst), m_unitID(unitid), m_count(0), m_owner(owner) 
			{for(int i=0;i<MAXOBJID;i++){m_objectID[i]=-1;m_time[i]=0;}}
	short UnitID() const {return m_unitID;}
	enum {MAXTIME=0x7FFFFFFF};
	void SetObjectID(short objectID,unsigned long time,unsigned long realtime, bool reset=false);
	short ObjectID(unsigned long time, unsigned long* timeIdentification=0) const;
	unsigned long TimeFirstSeen() const {return m_timeFirst;}

	DECLARE_LOCAL_HEAP(32768)
};

class BWElementList	: public IUnitIDToObjectID
{
public:
	// ctor
	BWElementList(Replay *replay) : m_replay(replay) {}

	// parent replay
	Replay *m_replay;

	// add new element
	BWElement * AddElement(class ReplayEvtList *evtlist, short unitID, unsigned long timeFirstSeen, bool *isnew=0);

	// identify object in time
	void SetObjectID(class ReplayEvtList *evtlist,short unitID, short objectID, unsigned long time,unsigned long realtime);

	// get object identity from unit ID and time
	short GetObjectID(short unitID, unsigned long time) const;

	// get object count
	int GetCount() const {return m_elements.GetCount();}

	// find element from unitID
	BWElement *FindElement(short unitID) const;

	// clear array
	void Clear() {m_elements.Clear();}

	// IUnitIDToObjectID implementation
	virtual short Convert(short unitID, unsigned long time) const;

	static int BWElementList::compare( const void *arg1, const void *arg2 );

private:
	MemoryBlock m_elements;  // block with all elements of type BWElement
};

//------------------------------------------------------------------------------------------------------------

class HotKey
{
public:
	HotKey() : m_unitcount(0) {memset(m_hotkeyUnits,0,sizeof(m_hotkeyUnits));}
	HotKey(const HotKey& src) {m_unitcount=src.m_unitcount;memcpy(m_hotkeyUnits,src.m_hotkeyUnits,sizeof(m_hotkeyUnits));}
	int m_unitcount;
	short m_hotkeyUnits[MAXSELECTION];
};

class HotKeyEvent : public CObject
{
	HotKey *m_phk;
public:
	HotKeyEvent(unsigned long time, short slot, HotKey *hk, short type) : m_time(time), m_slot(slot), m_phk(hk), m_type(type) {}
	//~HotKeyEvent() {if(m_isAssignment) delete m_phk;}
	const HotKey *GetHotKey() const {return m_phk;}
	unsigned long m_time; // tick
	short m_slot;
	enum {ASSIGN,SELECT,ADD};
	short m_type;
};

//------------------------------------------------------------------------------------------------------------

// events for one player
class ReplayEvtList : public CObject
{
public:
	enum {DIST_UNIT=0,DIST_ACTION,DIST_BUILDING,DIST_UPGRADE,DIST_ACTIONFORAPM,__MAXDIST};
	enum {MAXHOTKEY=16};

private:
	// parent replay
	Replay *m_replay;

	// player name & id
	char m_playername[128+1];
	int m_playerid;

	// all resources
	ReplayResource *m_resources;
	int m_currentSlot;

	// player race
	int m_race;
	MemoryBlock m_events;  // block with all events of type ReplayEvt
	ReplayResource m_resmax; // all peaks for that list
	bool m_bEnabled;

	// starting location
	int m_startX;
	int m_startY;

	// hot key events
	MemoryBlock m_hkevents;  // block with all events of type HotKeyEvent
 	bool m_hotkeyIsUsed[MAXHOTKEY];

	// actions from the beginning to ignore for APM
	int m_eventsBegin;

	// actions discarded (for computing VAPM)
	int m_discardedActions;

	// last action id
	int m_lastActionID;
	int m_lastSelection;

	// build order
	ReplayBuildOrder m_bo;

	// number of units selected 
	int m_currentSelection;
	short m_selectedUnits[MAXSELECTION];

	// array for unit & building distribution
	unsigned long *m_objects;
	int m_similarUnits;

	// array for unit distribution
	unsigned long *m_upgrades;
	unsigned long *m_upgradesCount;

	// array for event type distribution
	unsigned long m_type[MAXACTIONTYPE];
	unsigned long m_typeAPM[MAXACTIONTYPE];

	// peaks for distribution
	unsigned long m_peak[__MAXDIST];

	// totals for distribution
	unsigned long m_total[__MAXDIST];

	// hot keys
	HotKey m_hotkey[MAXHOTKEY];

	// pointer to replay map
	ReplayMapAnimated *m_mapAnim;

	// map surface for map coverage
	MapSurface *m_mapSurface;
	int m_mapDividerX;
	int m_mapDividerY;

	// apm standard dev
	mutable int m_apmDev;

	// minimum apm
	int m_apmMini;

	// activity measurement
	long m_bpmAcc;
	long m_bpmSpeed;
	long m_upmAcc;
	long m_upmSpeed;

	// all selectable elements
	BWElementList m_elems_;

	// true if player built corresponding building
	bool m_bHasAcademy;
	bool m_bHasFleetBeacon;
	bool m_bHasReaver;
	bool m_bHasCarrier;
	bool m_hasCovertOps;

	// unit distribution
	int GetMaxUnit() const;
	const char *GetUnitName(int idx) const;
	COLORREF GetUnitColor(int idx) const;

	// upgrade distribution
	int GetMaxUpgrade() const;
	const char *GetUpgradeName(int idx) const;
	COLORREF GetUpgradeColor(int idx) const;

	// building distribution
	int GetMaxBuilding() const;
	const char *GetBuildingName(int idx) const;
	COLORREF GetBuildingColor(int idx) const;

	// special stuff to do for some actions
	bool _HandleSelection(const IStarcraftAction *action);
	bool _UpdateSelection(const BWrepActionSelect::Params *p, unsigned long time);
	int _HandleBuild(const IStarcraftAction *action, int& bx, int& by);
	void _AdjustData(const IStarcraftAction *action, int& actionID, int &unitID, const char * &parameters, int& subcmd );
	bool _IsValidBuildEvent(ReplayEvt *evt, ReplayEvt *prevEvt, int unitID);
	bool _IsValidTrainEvent(ReplayEvt *evt, ReplayEvt *prevEvt, int unitID);
	bool _IsValidUpgradeEvent(ReplayEvt *evt, ReplayEvt *prevEvt, int techID);
	int _ActionPerMinute(unsigned long time, int eventCount) const;
	int _BuildActionPerMinute(const IStarcraftAction *action, const ReplayEvt *prevEvt,bool bIsValidEvent);
	int _TrainActionPerMinute(const ReplayEvt *evt, const ReplayEvt *prevEvt,bool bIsValidEvent);
	void _Discard(ReplayEvt *evt);

	// elements identification
	void _IdentifyTrain(int objectID,unsigned long time);
	void _IdentifyBuild(int objectID,unsigned long time, char btype);
	void _IdentifyUpgrade(int upgID, unsigned long time);
	void _IdentifyResearch(int resID, unsigned long time);
	void _IdentifyCommand(int cmdID, int subcmd, unsigned long time);

	// add actions
	void _AddBuilding(int idx, int x, int y, unsigned long time);

	// to call when event is finished creating
	void _Complete(ReplayEvt *evt, int currentSelection);

public:
	ReplayEvtList(Replay *replay, ReplayMapAnimated *mapAnim, const char *playername, int id, int race);
	~ReplayEvtList();

	// process map coverage
	void ProcessMapCoverage(unsigned long actime);

	// get resource from time
	ReplayResource *GetResourceFromIdx(int idx) {return &m_resources[idx];}
	ReplayResource *GetResource(unsigned long tick) {return &m_resources[tick/RES_INTERVAL_TICK];}
	ReplayResource *GetPrevResource(unsigned long tick) {return &m_resources[(tick/RES_INTERVAL_TICK)-1];}
	void InitResource(unsigned long tick);
	int GetSlotCount() const;
	int Time2Slot(unsigned long tick) const {return tick/RES_INTERVAL_TICK;}
	unsigned long Slot2Time(int slot) const {return slot>=0 ? slot*RES_INTERVAL_TICK : 0;}

	// get selectable element object ID	(-1 if not found)
	short GetObjectID(short unitID, unsigned long time) const 
		{return GetElemListConst()->GetObjectID(unitID,time);}
 	BWElementList *GetElemList() {return &m_elems_;}
	const BWElementList *GetElemListConst() const {return &m_elems_;}
	void ClearElements() {GetElemList()->Clear();}

	// set starting location
	void SetStartingLocation(int x, int y) {m_startX=x; m_startY=y;}
	int GetStartingLocation() const; // as clock value

	// update player id 
	int GetPlayerID() const {return m_playerid;}

	// compute standard deviation for APM
	int GetStandardAPMDev(int delta, int deltaMap);

	// micro APM
	int GetMicroAPM() const;

	// macro APM
	int GetMacroAPM() const;

	// hot key events
	unsigned long GetHKEventCount() const {return m_hkevents.GetCount();}
	const HotKeyEvent *GetHKEvent(int idx) const {return (const HotKeyEvent *)m_hkevents.GetPtr(idx*sizeof(HotKeyEvent));}
	unsigned long GetHKEventTimeMax() const {return GetHKEventCount()==0?0:GetHKEvent(GetHKEventCount()-1)->m_time;}
	bool IsHotKeyUsed(int slot) const {return m_hotkeyIsUsed[slot];}


	// add actions
	unsigned long AddEvent(IStarcraftAction *action, const char* &parameters);
	void AddUnit(int idx, unsigned long time);
	void AddEvtType(int idx, bool eventValidForAPM);
	void AddUpgrade(int idx);
	
	const char *PlayerName() const {return m_playername;}
	int GetEventCount() const {return m_events.GetCount();}
	const ReplayResource& ResourceMax() const {return m_resmax;}
	const char *GetRaceStr() const;
	int GetRaceIdx() const {return m_race;}
	bool IsEnabled() const {return m_bEnabled;}
	void EnablePlayer(bool val) {m_bEnabled=val;}
	int GetDiscardedActions() const {return m_discardedActions;}

	// cursor = value between 0 and full span time
	// will find the nearest event 
	unsigned long GetEventFromTime(unsigned long cursor);

	// get event
	const ReplayEvt* GetEvent(unsigned long i) const {return (const ReplayEvt*)(m_events.GetPtr(i*sizeof(ReplayEvt)));}
	ReplayEvt* GetEvent(unsigned long i) {return (ReplayEvt*)(m_events.GetPtr(i*sizeof(ReplayEvt)));}

	// get max number of indexes for a distribution
	int GetDistMax(int type) const 
	{
		if(type==DIST_UNIT) return GetMaxUnit();
		if(type==DIST_ACTION || type==DIST_ACTIONFORAPM) return MAXACTIONTYPE;
		if(type==DIST_UPGRADE) return GetMaxUpgrade();
		if(type==DIST_BUILDING) return GetMaxBuilding();
		return 0;
	}

	// what is the count for a particular element at index idx?
	int GetDistCount(int type, int idx); 

	// compute number of non null elements in the distribution
	int GetDistNonNullCount(int type)
	{
		int count=0;
		int distmax = GetDistMax(type);
		for(int i=0; i<distmax; i++)
			if(GetDistCount(type,i)>0) count++;
		return count;
	}

	// what is the peak count of all elements
	int GetDistPeak(int type) const
	{
		assert(type>=0 && type<__MAXDIST);
		if(type==DIST_ACTIONFORAPM) return GetActionPerMinute(false,m_peak[type]);
		return m_peak[type];
	}

	// what is the total count of all elements
	int GetDistTotal(int type) const
	{
		assert(type>=0 && type<__MAXDIST);
		if(type==DIST_ACTIONFORAPM) return GetActionPerMinute(false);
		return m_total[type];
	}

	// what is the name for the element at idx
	const char *GetDistName(int type, int idx) const 
	{
		if(type==DIST_UNIT) return GetUnitName(idx);
		if(type==DIST_ACTION || type==DIST_ACTIONFORAPM) return ReplayEvtType::GetTypeFromIdx(idx);
		if(type==DIST_UPGRADE) return GetUpgradeName(idx);
		if(type==DIST_BUILDING) return GetBuildingName(idx);
		return "";
	}

	// what is the color for the element at idx
	COLORREF GetDistColor(int type,int idx) const
	{
		if(type==DIST_UNIT) return GetUnitColor(idx);
		if(type==DIST_ACTION || type==DIST_ACTIONFORAPM) return ReplayEvtType::GetTypeColor(idx);
		if(type==DIST_UPGRADE) return GetUpgradeColor(idx);
		if(type==DIST_BUILDING) return GetBuildingColor(idx);
		return 0;
	}

	// actions per minutes
	int GetActionPerMinute(bool bValidOnly=false, int eventCount=-1) const; 

	// valid actions per minutes
	int GetValidActionPerMinute() const {return GetActionPerMinute(true);}

	// similar units
	void IncSimilar() {m_similarUnits++;}
	void ResetSimilar() {m_similarUnits=0;}
	int GetSimilar() const {return m_similarUnits;}

	// selected units
	void AddToSelection(short unitid); 
	void RemoveFromSelection(short unitid); 
	int GetSelection() const {return m_currentSelection;}
	int GetSelectionForHatch(int unitType, unsigned long now) const;
	short GetSelectedUnitID(int i) const {return i<m_currentSelection ? m_selectedUnits[i] : -1;}

	// assign hotkey
	void AssignHotKey(int slot, unsigned long time);
	void SelectHotKey(int slot, unsigned long time);
	void AddHotKey(int slot, unsigned long time);

	// build order for buildings
	const ReplayObjectSequence& GetBuildOrder() const {return m_bo.Buildings();}
	// build order for units
	const ReplayObjectSequence& GetBuildOrderUnits() const {return m_bo.Units();}
	// build order for research
	const ReplayObjectSequence& GetBuildOrderResearch() const {return m_bo.Research();}
	// build order for upgrades
	const ReplayObjectSequence& GetBuildOrderUpgrade() const {return m_bo.Upgrade();}

	// get final build order as a string
	void GetFinalBuildOrder(CString& bo);

	// adjust all actions addresses
	void AdjustActionPointers(char *oldbase, char *newbase);

	// return name of orginial object name for a suspect event
	bool GetSuspectEventOrigin(const IStarcraftAction *action, CString& origin, bool hhmmss);

	// get minimum apm
	int GetMiniAPM() const {return m_apmMini;}
};

//------------------------------------------------------------------------------------------------------------

// full replay information (only one loaded in memory at any time)
class Replay
{
private:
	// list of ReplayEvtList objects
	XObArray m_players; 
	XObArray m_deletedPlayers; 
	int *m_listref;

	// enabled actions (IStarcraftAction* objects)
	MemoryBlock m_enabledActions;

	// filter for enabled actions
	int m_filter;
	
	// suspect events
	int m_suspectCount;

	// hack events
	int m_hackCount;

	//audio header for RWA
	AudioHeader m_hdrRWA;
	bool m_isRWA;

	// current apm style
	int m_apmStyle;
	int m_mapStyle;

	IStarcraftReplay *m_gfile;
	CString m_filename;
	unsigned long m_timeEnd;
	ReplayResource m_resmax; // all peaks for the whole replay
	bool m_Done;
	mutable unsigned long m_lastBOTime;
	mutable unsigned long m_lastHKEventTimeMax;
	int m_overalActionCount;
	ReplayEvtList *m_currentlist;

	// animated map
	ReplayMapAnimated *m_mapAnim;

	// returns true if we have event for a player 
	bool _HaveEventsForPlayer(const char *name, const CStringArray& existingPlayers) const;
	void _GetUniquePlayerName(CString& playerName, const CStringArray& existingPlayers);

	// return max APM for whole replay but ignoring players that are disabled
	void _UpdateMaxAPMForEnabledPlayers();

	// rebuild enabled action list
	void _BuildEnableActionList();

	// mark suspicious events
	void _MarkSuspiciousEvents();

	// mark events that are HACK signatures
	void _MarkHackCommands();

	// get previous player action
	const IStarcraftAction *_GetPreviousPlayerAction(int i, int playerID) const;

	unsigned long _AddEvent(IStarcraftAction *action, const char *playername, int race, const char* &parameters);
	void _Sort();
	void _CreateTileset();
	void _ClearMaps();
	void _ComputeActionDistribution();
	ReplayEvtList *_GetListFromPlayerName(const char *playername, int race);

public:
	Replay() : m_timeEnd(0), m_lastBOTime(0), m_lastHKEventTimeMax(0), m_Done(false), m_mapAnim(0), m_gfile(0),
		m_listref(0), m_filter(FLT_ALL), m_isRWA(false), m_apmStyle(APM_MEDIUM), m_mapStyle(APM_MEDIUM), m_suspectCount(0), m_hackCount(0) {}
	~Replay() { delete m_mapAnim; delete[]m_listref;if(m_gfile!=0) m_gfile->Release();m_gfile=0;}

	// load replay
	int Load(const char *filename, bool buildEnActionList, class CListCtrl *listv, bool bClear);

	// get game length in ticks
	unsigned long GetGameLength() const {return m_gfile->QueryHeader()->getGameLength();}

	//audio header for RWA (or null if not an RWA)
	AudioHeader *RWAHeader() {return m_isRWA ? &m_hdrRWA : 0;}

	// get player count
	int GetPlayerCount() const {return m_players.GetSize();};

	// _access to replay info
	const IStarcraftReplay* QueryFile() const {return m_gfile;}

	// get file name
	const char *GetFileName() const {return m_filename;}

	// get event list for one player
	ReplayEvtList* GetEvtList(int i) {return (ReplayEvtList*)m_players.GetAt(i);}
	const ReplayEvtList* GetEvtList(int i) const {return (ReplayEvtList*)m_players.GetAt(i);}
	const ReplayEvtList* GetEvtListFromPlayerID(int playerid) const {return GetEvtList(m_listref[playerid]);}

	// return object id from any unit id
	bool GetAnyObjectID(short unitID, unsigned long time, short *objID);

	// clear everything
	void Clear();

	// update filter for enabled actions
	enum {FLT_SELECT=1,FLT_BUILD=2,FLT_TRAIN=4,FLT_SUSPECT=8,FLT_OTHERS=16,FLT_HACK=32,FLT_CHAT=64,FLT_ALL=255};
	void UpdateFilter(int filter) {m_filter=filter; _BuildEnableActionList();}

	// get end time for all events in the replay object
	unsigned long GetEndTime() const {return m_timeEnd;}

	// get max resources 
	const ReplayResource& ResourceMax() const {return m_resmax;}

	// get map name
	const char *MapName() const;

	// don't draw anything before IsDone retuns true
	bool IsDone() const {return m_Done;}

	// remove events from a particular player
	void RemovePlayer(ReplayEvtList *list, class CListCtrl *listv);

	// game animated map
	ReplayMapAnimated *GetMapAnim() const {return m_mapAnim;}

	// get time of latest action in the build orders
	unsigned long GetLastBuildOrderTime() const 
	{
		if(m_lastBOTime==0)
		{
			for(int i=0;i<GetPlayerCount(); i++) 
			{
				if(GetEvtList(i)->GetBuildOrder().GetLastTime()>m_lastBOTime) 
					m_lastBOTime = GetEvtList(i)->GetBuildOrder().GetLastTime();
				if(GetEvtList(i)->GetBuildOrderUnits().GetLastTime()>m_lastBOTime) 
					m_lastBOTime = GetEvtList(i)->GetBuildOrderUnits().GetLastTime();
			}
		}
		return m_lastBOTime+300; // use some margin for names to be displayed entirely
	}

	// get time of latest hot key action 
	unsigned long GetHKEventTimeMax() const 
	{
		if(m_lastHKEventTimeMax==0)
		{
			for(int i=0;i<GetPlayerCount(); i++) 
			{
				if(GetEvtList(i)->GetHKEventTimeMax()>m_lastHKEventTimeMax) 
					m_lastHKEventTimeMax = GetEvtList(i)->GetHKEventTimeMax();
			}
		}
		return m_lastHKEventTimeMax+300; // use some margin for names to be displayed entirely
	}

	// how many players are enabled?
	int GetEnabledPlayerCount() 
	{
		int count=0;
		for(int i=0;i<GetPlayerCount(); i++) if(GetEvtList(i)->IsEnabled()) count++;
		return count;
	};

	// what it the distribution peak for all players, for one distribution type
	int GetDistPeak(int type) 
	{
		int peak=0;
		for(int i=0;i<GetPlayerCount(); i++) 
			if(GetEvtList(i)->GetDistPeak(type)>peak) peak=GetEvtList(i)->GetDistPeak(type);
		return peak;
	}

	// suspect events
	int GetSuspectCount() const {return m_suspectCount;}

	// hack events
	int GetHackCount() const {return m_hackCount;}

	// how many different action types do we have in that replay
	int GetTypeCount() const {return m_overalActionCount;}
	const char *GetTypeStr(int idx) const;
	int GetTypeIdx(const ReplayEvt *evt) const;

	// enable or disable a player
	void EnablePlayer(ReplayEvtList *list, bool val);

	// get action from enabled players only
	const IStarcraftAction *GetEnAction(int iItemIndx) const {return *((const IStarcraftAction **)m_enabledActions.GetPtr(iItemIndx*sizeof(IStarcraftAction *)));}
	unsigned long GetEnActionCount() const {return m_enabledActions.GetCount();}

	// find next suspect event
	int GetNextSuspectEvent(int selectedAction);

	// export events to text file
	int ExportToText(const char *textfile, bool useSeconds, char cSep);

	// reprocess apm for all players
	enum {APM_VFLAT,APM_FLAT,APM_MEDIUM,APM_DYNAMIC,APM_VDYNAMIC, __APM_MAX};
	bool UpdateAPM(int apmStyle, int mapStyle);

	static void DebugDisplayList();
};

//------------------------------------------------------------------------------------------------------------

#endif
