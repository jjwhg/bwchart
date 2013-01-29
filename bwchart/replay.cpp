#include "stdafx.h"
#include "replay.h"
#include "hsvrgb.h"
#include "progressdlg.h"
#include "botree.h"
#include <assert.h>
#include <math.h>

#include "../common/mergeaudio.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// events after this time cannot be "suspicious"
unsigned long gSuspectLimit=15; // minutes

// time window for various apm styles
static int gTimeWindow[Replay::__APM_MAX]={60,30,15,8,4};

// time window for various map coverage styles
static int gTimeWindowMap[Replay::__APM_MAX]={120,90,60,30,15};

#define MOVE_SCALE 32

// side of square for map coverage
#define MAPCOVERAGE_SIDE 8

IMPLEMENT_LOCAL_HEAP(ReplayEvt)
IMPLEMENT_LOCAL_HEAP(BWElement)

//------------------------------------------------------------------------------------------------------------

// pre-defined colors for each resource
COLORREF ReplayResource::m_gColors[__CLR_MAX]=
{
	// mineral
	DEF_CLR_MINERAL,
	// gas
	DEF_CLR_GAS,
	// supply
	DEF_CLR_SUPPLY,
	// units
	DEF_CLR_UNITS,
	// actions per minute
	DEF_CLR_APM,
	// build actions per minute
	DEF_CLR_BPM,
	// unit actions per minute
	DEF_CLR_UPM,
	// unit micro apm
	DEF_CLR_MICRO,
	// unit macro apm
	DEF_CLR_MACRO,
	// map coverage
	DEF_CLR_MAPCOVERAGE,
	// moving map coverage
	DEF_CLR_MMCOVERAGE
};

// pre-defined line sizefor each resource
int ReplayResource::m_gLineSize[__CLR_MAX]=
{
	// mineral
	DEF_LSIZE_MINERAL,
	// gas
	DEF_LSIZE_GAS,
	// supply
	DEF_LSIZE_SUPPLY,
	// units
	DEF_LSIZE_UNITS,
	// actions per minute
	DEF_LSIZE_APM,
	// build actions per minute
	DEF_LSIZE_BPM,
	// unit actions per minute
	DEF_LSIZE_UPM,
	// unit micro apm
	DEF_LSIZE_MICRO,
	// unit macro apm
	DEF_LSIZE_MACRO,
	// map coverage
	DEF_LSIZE_MAPCOVERAGE,
	// moving map coverage
	DEF_LSIZE_MMCOVERAGE
};

//-----------------------------------------------------------------------------------------------------------------

const char *_MkTime(const IStarcraftGame *header, unsigned long time, bool hhmmss)
{
	static char buf[12];
	if(hhmmss)
	{
		int sec = (header==0)?0:header->Tick2Sec(time);
		int h = sec/3600; sec-=h*3600;
		int m = sec/60; sec-=m*60;
		sprintf(buf,"%02d:%02d:%02d",h,m,sec);
	}
	else
	{
		sprintf(buf,"%lu",time);
	}
	return buf;
}

//------------------------------------------------------------------------------------------------------------

void ReplayResource::UpdateMax(const ReplayResource& res, bool updateAPM)
{
	if(res.m_gaz>m_gaz) m_gaz=res.m_gaz;
	if(res.m_minerals>m_minerals) m_minerals=res.m_minerals;
	if(res.m_supply>m_supply) m_supply=res.m_supply;
	if(res.m_units>m_units) m_units=res.m_units;
	if(res.m_actionPerMinute>m_actionPerMinute) m_actionPerMinute=res.m_actionPerMinute;
	if(res.m_microAPM>m_microAPM) m_microAPM=res.m_microAPM;
	if(res.m_macroAPM>m_macroAPM) m_macroAPM=res.m_macroAPM;
	if(updateAPM && res.m_legalActionPerMinute>m_legalActionPerMinute) m_legalActionPerMinute=res.m_legalActionPerMinute;
	if(res.m_buildPerMinute>m_buildPerMinute) m_buildPerMinute=res.m_buildPerMinute;
	if(res.m_unitPerMinute>m_unitPerMinute) m_unitPerMinute=res.m_unitPerMinute;
	if(res.m_mapCoverageUnit>m_mapCoverageUnit) m_mapCoverageUnit=res.m_mapCoverageUnit;
	if(res.m_mapCoverageBuild>m_mapCoverageBuild) m_mapCoverageBuild=res.m_mapCoverageBuild;
	if(res.m_mapMovingMapCoverage>m_mapMovingMapCoverage) m_mapMovingMapCoverage = res.m_mapMovingMapCoverage;
}

//------------------------------------------------------------------------------------------------------------

void ReplayResource::UpdateResourceTrain(int unitID, int unitsToAdd)
{
	// check unit id
	assert(unitID>=0 && unitID<MAXUNIT);
	if(gAllUnits[unitID].IsEmpty()) 
	{
		OutputDebugString("unknown unit\r\n"); ASSERT(0);
		return;
	}

	// how many units should we add?
	for(int k=0; k<unitsToAdd; k++)
	{
		// add resources for it
		m_minerals+=gAllUnits[unitID].mineral;
		m_gaz+=gAllUnits[unitID].gaz;
		m_supply+=gAllUnits[unitID].supply;
		m_units++;
	}
}

//------------------------------------------------------------------------------------------------------------

void ReplayResource::UpdateResourceBuild(int unitID)
{
	// check unit id
	assert(unitID>=0 && unitID<MAXUNIT);
	if(gAllUnits[unitID].IsEmpty()) 
	{
		OutputDebugString("unknown building\r\n"); ASSERT(0);
		return;
	}

	// update resources
	m_minerals+=gAllUnits[unitID].mineral;
	m_gaz+=gAllUnits[unitID].gaz;
}

//------------------------------------------------------------------------------------------------------------

int ReplayResource::_FindTech(const char *parameters)
{
	char buffer[255];
	strcpy(buffer,parameters);

	char *p=strchr(buffer,'(');
	if(p!=0)
	{
		p++;
		strtok(p,")");
	}
	else
		p=buffer;


	for(int i=0;i<MAXTECHNIC();i++)
	{
		if(_stricmp(p,gAllTechs[i].technic)==0) 
			return i;
	}
	OutputDebugString("unknown upgrade\r\n"); ASSERT(0);
	return -1;
}

//------------------------------------------------------------------------------------------------------------

void ReplayResource::UpdateResourceUpgrade(int techID)
{
	assert(techID>=0 && techID<MAXTECHNIC());
	m_minerals+=gAllTechs[techID].mineral;
	m_gaz+=gAllTechs[techID].gaz;
}

//------------------------------------------------------------------------------------------------------------

COLORREF ReplayResource::GetColor(int i, int player, int maxplayer)
{
	// if each player is on his own row
	COLORREF cref = GetUniqueColor(i);
	if(player==-1) return cref;

	// convert to HSV
	int h; double s,v;
	CHsvRgb::Rgb2Hsv(cref, &h, &s, &v);

	// all players are mixed on same chart
	h+=player*(maxplayer<=3?8:32);
	player=player%2;
	v = 1.0 - (double)player/(double)maxplayer;
	s = 1.0 - (double)player/((double)maxplayer*2.0);
	if(i>4) s-=0.2;
	return CHsvRgb::Hsv2Rgb(h%360,s,v).rgb();
}

//------------------------------------------------------------------------------------------------------------

COLORREF ReplayEvt::GetColor() const
{
	return ReplayEvtType::GetTypeColor(IdxAction(m_type.m_cmd,m_type.m_subcmd));
}

//------------------------------------------------------------------------------------------------------------

//ctor
ReplayEvtList::ReplayEvtList(Replay *replay, ReplayMapAnimated *mapAnim, const char *playername, int id, int race) : 
	m_race(race), m_bEnabled(false), m_similarUnits(0),	m_discardedActions(0), m_eventsBegin(0),
		m_playerid(id), m_mapAnim(mapAnim), m_events(sizeof(ReplayEvt)*500), m_apmDev(0), m_bHasAcademy(false),
	m_currentSelection(0), m_replay(replay), m_elems_(replay), m_bHasFleetBeacon(false), m_bHasReaver(false), 
	m_bHasCarrier(false), m_startX(0), m_startY(0), m_hasCovertOps(false), 	m_lastActionID(0),	
	m_lastSelection(0), m_mapSurface(0), m_mapDividerX(0), m_mapDividerY(0), m_currentSlot(-1)

{
	strcpy(m_playername,playername);

	// unit distribution
	m_objects = new unsigned long[MAXUNIT];
	memset(m_objects,0,sizeof(unsigned long)*MAXUNIT);
	memset(m_peak,0,sizeof(m_peak));
	memset(m_total,0,sizeof(m_total));

	// upgrade distribution
	m_upgrades = new unsigned long[MAXTECHNIC()];
	memset(m_upgrades,0,sizeof(unsigned long)*MAXTECHNIC());
	m_upgradesCount = new unsigned long[MAXTECHNIC()];
	memset(m_upgradesCount,0,sizeof(unsigned long)*MAXTECHNIC());

	// type distribution
	memset(m_type,0,sizeof(m_type));
	memset(m_typeAPM,0,sizeof(m_typeAPM));

	// hotkeys
	memset(m_hotkey,0,sizeof(m_hotkey));
	memset(m_hotkeyIsUsed,0,sizeof(m_hotkeyIsUsed));

	// activity measurement
	m_bpmAcc=0;
	m_bpmSpeed=0;
	m_upmAcc=0;
	m_upmSpeed=0;

	// allocate resources array 
	int resize = 1+replay->GetGameLength()/RES_INTERVAL_TICK;
	m_resources = new ReplayResource[resize];
	memset(m_resources,0,sizeof(ReplayResource)*resize);

	// allocate map surface
	if(mapAnim!=0)
	{
		m_mapDividerX = mapAnim->GetWidth()/MAPCOVERAGE_SIDE;
		m_mapDividerY = mapAnim->GetHeight()/MAPCOVERAGE_SIDE;
		m_mapSurface = new MapSurface(mapAnim->GetWidth()/m_mapDividerX,mapAnim->GetHeight()/m_mapDividerY);
	}
}

//dtor
ReplayEvtList::~ReplayEvtList() 
{
	// delete map surface
	delete m_mapSurface;

	// delete hotkeys
	const HotKey *hk=0;
	for(int i=0; i<(int)GetHKEventCount();i++)
	{
		const HotKeyEvent *hkevt = GetHKEvent(i);
		if(hkevt->m_type!=HotKeyEvent::SELECT) 
			delete (HotKey*)hkevt->GetHotKey();
	}

	delete[] m_objects; 
	m_objects=0;
	delete[] m_upgrades; 
	m_upgrades=0;
	delete[] m_upgradesCount; 
	m_upgradesCount=0;
	delete[]m_resources;
}

//-----------------------------------------------------------------------------------------------------------------

// to call when event is finished creating
void ReplayEvtList::_Complete(ReplayEvt *evt, int currentSelection)
{
	// get action id
	const IStarcraftAction *action = evt->GetAction();
	int actionID = action->GetID();
	bool bIsValidEvent = !evt->IsDiscarded();

	// if it's a valid event check its type
	int type=ReplayResource::A_OTHER;
	const ReplayEvtType& evttype = evt->Type();
	if(bIsValidEvent)
	{					
		switch(actionID)
		{
		case BWrepGameData::CMD_ATTACK :

			// for regular attack commands (not rally)
			if(evttype.m_subcmd != BWrepGameData::ATT_SETRALLY && evttype.m_subcmd != BWrepGameData::ATT_CLEARRALLY)
			{
				// if last action was a select with 1 unit, it was counted as macro
				if(m_lastActionID==BWrepGameData::CMD_SELECT && m_lastSelection==1)
				{
					//so fix this
					evt->pResources()->AddAction(ReplayResource::A_MICRO,false);
					evt->pResources()->RemoveAction(ReplayResource::A_MACRO,false);
				}
				// update micro
				evt->pResources()->AddAction(ReplayResource::A_MICRO,false);
			}
			else
			{
				// count set rally and clear rally as macro
				evt->pResources()->AddAction(ReplayResource::A_MACRO,false);
			}
			break;
		case BWrepGameData::CMD_MOVE :
			// if this is a move and selection has 1 unit
			if(m_currentSelection==1)
			{
				// find unit id
				BWElement *element;
				if((element=GetElemList()->FindElement(m_selectedUnits[0]))!=0)
				{
					// if unit is cc, nexus or hatchery, this is in fact a SET RALLY action
					short objID = element->ObjectID(action->GetTime());
					if(objID==BWrepGameData::OBJ_COMMANDCENTER || objID==BWrepGameData::OBJ_NEXUS || objID==BWrepGameData::OBJ_HATCHERY)
					{
						evt->GetTypePtr()->m_cmd = BWrepGameData::CMD_ATTACK; 
						evt->GetTypePtr()->m_subcmd = BWrepGameData::ATT_SETRALLY; 
					}
				}
			}

			// fall thru
			

		case BWrepGameData::CMD_STIM:
		case BWrepGameData::CMD_LIFT:
		case BWrepGameData::CMD_BURROW:
		case BWrepGameData::CMD_UNBURROW:
		case BWrepGameData::CMD_HOLDPOSITION:
		case BWrepGameData::CMD_UNLOADALL:
		case BWrepGameData::CMD_UNLOAD:
		case BWrepGameData::CMD_SIEGE:
		case BWrepGameData::CMD_UNSIEGE:
		case BWrepGameData::CMD_CLOAK:
		case BWrepGameData::CMD_DECLOAK:
		case BWrepGameData::CMD_SHIFTSELECT:
		case BWrepGameData::CMD_SHIFTDESELECT:
		case BWrepGameData::CMD_RETURNCARGO:
		case BWrepGameData::CMD_ARM:
			// if last action was a select with 1 unit, it was counted as macro
			if(m_lastActionID==BWrepGameData::CMD_SELECT && m_lastSelection==1)
			{
				//so fix this
				evt->pResources()->AddAction(ReplayResource::A_MICRO,false);
				evt->pResources()->RemoveAction(ReplayResource::A_MACRO,false);
			}
			// update micro apm
			evt->pResources()->AddAction(ReplayResource::A_MICRO,false);
			break;
		case BWrepGameData::CMD_SELECT:
			// select more than one unit is considered micro
			if(currentSelection>1)
				evt->pResources()->AddAction(ReplayResource::A_MICRO,false);
			else
				evt->pResources()->AddAction(ReplayResource::A_MACRO,false);
			break;
		// used for BPM
		case BWrepGameData::CMD_BUILD:
		case BWrepGameData::CMD_MORPH:
			// update macro apm
			type=ReplayResource::A_BUILD;
			evt->pResources()->AddAction(ReplayResource::A_MACRO,false);
			break;
		case BWrepGameData::CMD_HATCH:
			// if last action was a select with >1 unit, it was counted as micro
			if(m_lastActionID==BWrepGameData::CMD_SELECT && m_lastSelection>1)
			{
				//so fix this
				evt->pResources()->AddAction(ReplayResource::A_MACRO,false);
				evt->pResources()->RemoveAction(ReplayResource::A_MICRO,false);
			}
			type=ReplayResource::A_TRAIN;
			evt->pResources()->AddAction(ReplayResource::A_MACRO,false);
			break;
		// used for UPM
		case BWrepGameData::CMD_TRAIN:
		case BWrepGameData::CMD_MERGEDARKARCHON:
		case BWrepGameData::CMD_MERGEARCHON:
			type=ReplayResource::A_TRAIN;
			evt->pResources()->AddAction(ReplayResource::A_MACRO,false);
			break;
		case BWrepGameData::CMD_HOTKEY:
			type=ReplayResource::A_HOTKEY;
			if(currentSelection>1)
				evt->pResources()->AddAction(ReplayResource::A_MICRO,false);
			else
				evt->pResources()->AddAction(ReplayResource::A_MACRO,false);
			break;
		case BWrepGameData::CMD_CANCELTRAIN:
		case BWrepGameData::CMD_RESEARCH:
		case BWrepGameData::CMD_UPGRADE:
			// update macro apm
			evt->pResources()->AddAction(ReplayResource::A_MACRO,false);
			break;
		default:
			evt->pResources()->AddAction(ReplayResource::A_MICRO,false);
			break;
		}
	}

	m_lastActionID = actionID;
	m_lastSelection = currentSelection;

	// count action in resources
	evt->pResources()->AddAction(type);
}

//------------------------------------------------------------------------------------------------------------

// selected units
void ReplayEvtList::AddToSelection(short unitid) 
{
	for(int i=0;i<m_currentSelection;i++)
		if(m_selectedUnits[i]==unitid)
			return;
	if(m_currentSelection<MAXSELECTION)
	{
		m_selectedUnits[m_currentSelection]=unitid; 
		m_currentSelection++;
	}
	assert(m_currentSelection>=0 && m_currentSelection<=MAXSELECTION);
}

void ReplayEvtList::RemoveFromSelection(short unitid) 
{
	int i=0;
	while(i<m_currentSelection)
	{
		if(m_selectedUnits[i]==unitid) 
		{
			for(int j=i+1;j<m_currentSelection;j++)
				m_selectedUnits[j-1]=m_selectedUnits[j];
			m_currentSelection--;
		}
		else i++;
	}
	assert(m_currentSelection>=0 && m_currentSelection<=MAXSELECTION);
}

//------------------------------------------------------------------------------------------------------------

// assign hotkey
void ReplayEvtList::AssignHotKey(int slot, unsigned long time) 
{
	assert(slot<MAXHOTKEY); 
	if(slot==0) slot=9; else slot--;

	// same selection?
	if(m_hotkey[slot].m_unitcount==m_currentSelection)
	{
		// check if all units are the same
		bool diff=false;
		for(int i=0;i<m_currentSelection;i++)
			if(m_hotkey[slot].m_hotkeyUnits[i]!=m_selectedUnits[i])
				{diff=true;break;}
		// it's only a double assignement of the same stuff
		if(!diff) return;
	}

	// memorize current selection in hot key
	assert(m_currentSelection>=0 && m_currentSelection<=MAXSELECTION);
	m_hotkey[slot].m_unitcount=m_currentSelection;
	m_hotkeyIsUsed[slot]=true;
	for(int i=0;i<m_currentSelection;i++)
		m_hotkey[slot].m_hotkeyUnits[i]=m_selectedUnits[i];

	// add hot key event
	m_hkevents.Add(&HotKeyEvent(time, slot, new HotKey(m_hotkey[slot]),HotKeyEvent::ASSIGN),sizeof(HotKeyEvent));
}

//------------------------------------------------------------------------------------------------------------

// add unit to hotkey
void ReplayEvtList::AddHotKey(int slot, unsigned long time) 
{
	assert(slot<MAXHOTKEY); 
	if(slot==0) slot=9; else slot--;

	// try to add each selected unit
	for(int i=0;i<m_currentSelection;i++)
	{
		// do we already have that unit in the group?
		bool found=false;
		for(int j=0;j<m_hotkey[slot].m_unitcount;j++)
			if(m_hotkey[slot].m_hotkeyUnits[j]==m_selectedUnits[i])
				{found=true; break;}
		if(found) continue;

		// if we dont, add it
		if(m_hotkey[slot].m_unitcount==MAXSELECTION) return;
		m_hotkey[slot].m_hotkeyUnits[m_hotkey[slot].m_unitcount]=m_selectedUnits[i];
		m_hotkey[slot].m_unitcount++;

	}

	// add hot key event
	m_hkevents.Add(&HotKeyEvent(time, slot, new HotKey(m_hotkey[slot]),HotKeyEvent::ADD),sizeof(HotKeyEvent));
}

//------------------------------------------------------------------------------------------------------------

void ReplayEvtList::SelectHotKey(int slot, unsigned long time) 
{
	assert(slot<MAXHOTKEY); 
	if(slot==0) slot=9; else slot--;
	
	
	if(slot>=MAXHOTKEY) return;

	// same selection?
	if(m_hotkey[slot].m_unitcount==m_currentSelection)
	{
		// check if all units are the same
		bool diff=false;
		for(int i=0;i<m_currentSelection;i++)
			if(m_hotkey[slot].m_hotkeyUnits[i]!=m_selectedUnits[i])
				{diff=true;break;}
		// it's only a double selection of the same stuff
		if(!diff) return;
	}

	// make hot key unit the current selection
	m_currentSelection=m_hotkey[slot].m_unitcount;
	for(int i=0;i<m_currentSelection;i++)
		m_selectedUnits[i]=m_hotkey[slot].m_hotkeyUnits[i];

	// find lastest assignment
	const HotKey *hk=0;
	for(int i=(int)GetHKEventCount()-1; i>=0 ;i--)
	{
		const HotKeyEvent *hkevt = GetHKEvent(i);
		if(hkevt->m_type==HotKeyEvent::ASSIGN && hkevt->m_slot==slot) 
			{hk=hkevt->GetHotKey();break;}
	}

	// add hot key event
	m_hkevents.Add(&HotKeyEvent(time, slot,(HotKey *)hk,HotKeyEvent::SELECT),sizeof(HotKeyEvent));
}

//------------------------------------------------------------------------------------------------------------

void ReplayEvtList::AddUnit(int idx, unsigned long time) 
{
	assert(idx>=0 && idx<MAXUNIT);
	m_objects[idx]++; 
	m_total[DIST_UNIT]++;
	if(m_objects[idx]>m_peak[DIST_UNIT]) m_peak[DIST_UNIT]=m_objects[idx];

	if(m_mapAnim!=0) 
		m_mapAnim->AddUnit(&ReplayMapAction(0,0,0,0,m_playerid,time));
}

void ReplayEvtList::AddEvtType(int idx, bool eventValidForAPM) 
{
	assert(idx>=0 && idx<MAXACTIONTYPE);

	// if action is valid for APM
	if(eventValidForAPM) 
	{
		// update all counter for apm actions
		m_typeAPM[idx]++; 
		if(m_typeAPM[idx]>m_peak[DIST_ACTIONFORAPM]) m_peak[DIST_ACTIONFORAPM]=m_typeAPM[idx];
		m_total[DIST_ACTIONFORAPM]++;
	}

	// update all counter for regular actions
	m_type[idx]++; 
	m_total[DIST_ACTION]++;
	if(m_type[idx]>m_peak[DIST_ACTION]) m_peak[DIST_ACTION]=m_type[idx];
}

void ReplayEvtList::AddUpgrade(int idx) 
{
	assert(idx>=0 && idx<MAXTECHNIC());
	m_upgrades[idx]++; 
	m_upgradesCount[idx]++;
	m_total[DIST_UPGRADE]++;
	if(m_upgrades[idx]>m_peak[DIST_UPGRADE]) m_peak[DIST_UPGRADE]=m_upgrades[idx];
}

void ReplayEvtList::_AddBuilding(int idx, int x, int y, unsigned long time) 
{
	assert(idx>=0 && idx<MAXUNIT);
	m_objects[idx]++; 
	m_total[DIST_BUILDING]++;
	if(m_objects[idx]>m_peak[DIST_BUILDING]) m_peak[DIST_BUILDING]=m_objects[idx];

	if(idx==BWrepGameData::OBJ_ACADEMY) m_bHasAcademy=true;
	
	if(x>=0 && y>=0 && m_mapAnim!=0) 
	{
		int w=gAllUnits[idx].width;
		int h=gAllUnits[idx].height;
		//if(m_map) m_map->SetBuilding(x,y,w,h,MapElem(idx,m_playerid));
		if(m_mapAnim) 
		{
			m_mapSurface->IncSquare(x/m_mapDividerX,y/m_mapDividerY,MapElem(1,0));
			m_mapAnim->AddBuild(&ReplayMapAction(x,y,w,h,m_playerid,time));
		}
	}
}

const char *ReplayEvtList::GetUnitName(int idx) const 
{
	assert(idx>=0 && idx<BWrepGameData::g_ObjectsSize);
	//if(idx==BWrepGameData::OBJ_ARCHON) return "Archon";
	//if(idx==BWrepGameData::OBJ_DARKARCHON) return "Dark Archon";
	//if(idx==BWrepGameData::OBJ_INFESTEDTERRAN) return "Infested Terran";
	return BWrepGameData::GetObjectNameFromID(idx);
}
const char *ReplayEvtList::GetBuildingName(int idx) const 
{
	assert(idx>=0 && idx<BWrepGameData::g_ObjectsSize);
	return BWrepGameData::GetObjectNameFromID(idx);
}
const char *ReplayEvtList::GetUpgradeName(int idx) const {return gAllTechs[idx].technic;}

//------------------------------------------------------------------------------------------------------------

static CStringArray glist;

void Replay::DebugDisplayList()
{
	OutputDebugString("_____________________\r\n");
	for(int i=0;i<glist.GetSize();i++)
	{
		OutputDebugString(glist.GetAt(i));
		OutputDebugString("\r\n");
	}
}

//------------------------------------------------------------------------------------------------------------

bool ReplayEvtList::_UpdateSelection(const BWrepActionSelect::Params *p, unsigned long time)
{
	bool suspicious=false;

	// for every unit	
	for(int i=0;i<p->m_unitCount;i++)
	{
		// add to current selection
		AddToSelection(p->m_unitid[i]);
	}

	return suspicious;
}

//------------------------------------------------------------------------------------------------------------

bool ReplayEvtList::_HandleSelection(const IStarcraftAction *action)
{
	int actionID = action->GetID();
 	bool bNewSelection=false;
	bool suspicious=false;

	// adjust selection
	if(actionID==BWrepGameData::CMD_SELECT || actionID==BWrepGameData::CMD_DESELECTAUTO)
	{
		// set new selection
		const BWrepActionSelect::Params *p = (const BWrepActionSelect::Params *)action->GetParamStruct();
		m_currentSelection=0;
		bNewSelection=true;

		// add units in elements list
		suspicious = _UpdateSelection(p, action->GetTime());
	}
	else if(actionID==BWrepGameData::CMD_SHIFTSELECT)
	{
		// update existing selection
		const BWrepActionShiftSelect::Params *p = (const BWrepActionShiftSelect::Params *)action->GetParamStruct();

		// add units in elements list
		suspicious = _UpdateSelection(p, action->GetTime());
	}
	else if(actionID==BWrepGameData::CMD_SHIFTDESELECT)
	{
		// update existing selection
		const BWrepActionShiftSelect::Params *p = (const BWrepActionShiftSelect::Params *)action->GetParamStruct();
		for(int i=0;i<p->m_unitCount;i++)
		{
			RemoveFromSelection(p->m_unitid[i]);
			//GetElemList()->Add(p->m_unitid[i]);
		}
	}
	// handle hotkey
	else if(actionID==BWrepGameData::CMD_HOTKEY)
	{
		const BWrepActionHotKey::Params *p = (const BWrepActionHotKey::Params *)action->GetParamStruct();
		if(p->m_type==BWrepGameData::HOT_ASSIGN) 
			AssignHotKey(p->m_slot,action->GetTime());
		else if(p->m_type==BWrepGameData::HOT_ADD) 
			AddHotKey(p->m_slot,action->GetTime());
		else if(p->m_type==BWrepGameData::HOT_SELECT) 
		{
			SelectHotKey(p->m_slot,action->GetTime());
			bNewSelection=true;
		}
	}
	else if(actionID==BWrepGameData::CMD_ATTACK)
	{
		const BWrepActionAttack::Params *p = (const BWrepActionAttack::Params *)action->GetParamStruct();
		if(m_mapAnim) 
		{
			int x= p->m_pos1/MOVE_SCALE;
			int y= p->m_pos2/MOVE_SCALE;
			m_mapSurface->IncSquare(x/m_mapDividerX,y/m_mapDividerY,MapElem(0,1));
			m_mapAnim->AddMove(&ReplayMapAction(x,y,1,1,m_playerid,action->GetTime()));
		}
	}
	else if(actionID==BWrepGameData::CMD_MOVE)
	{
		const BWrepActionMove::Params *p = (const BWrepActionMove::Params *)action->GetParamStruct();
		if(m_mapAnim) 
		{
			int x= p->m_pos1/MOVE_SCALE;
			int y= p->m_pos2/MOVE_SCALE;
			m_mapSurface->IncSquare(x/m_mapDividerX,y/m_mapDividerY,MapElem(0,1));
			m_mapAnim->AddMove(&ReplayMapAction(x,y,1,1,m_playerid,action->GetTime()));
		}

		// initial move on mineral patch of first workers?
		if(p->m_unknown1!=0 && GetElemList()->GetCount()<=5 && m_currentSelection<=4)
		{
			short objID;
			if(m_race==IStarcraftPlayer::RACE_TERRAN)
				objID=BWrepGameData::OBJ_SCV;
			else if(m_race==IStarcraftPlayer::RACE_PROTOSS)
				objID=BWrepGameData::OBJ_PROBE;
			else
				objID=BWrepGameData::OBJ_DRONE;
			for(int i=0;i<m_currentSelection;i++)
				GetElemList()->SetObjectID(this, m_selectedUnits[i],objID,action->GetTime(),action->GetTime());
		}
	}
	
	// if a new element is selected, reset list of similar events
	if(bNewSelection) ResetSimilar();

	// returns true is move is suspect (hacking)
	return suspicious;
}

//------------------------------------------------------------------------------------------------------------

// returns >=0 if action is a real build of something
int ReplayEvtList::_HandleBuild(const IStarcraftAction *action, int& bx, int& by)
{
	int actionID = action->GetID();

	// is it a build?
	if(actionID == BWrepGameData::CMD_BUILD)
	{
		// will need to update build order
		const BWrepActionBuild::Params *p = (const BWrepActionBuild::Params *)action->GetParamStruct();
		if(p->m_buildingtype==BWrepGameData::BTYP_BUILD || p->m_buildingtype==BWrepGameData::BTYP_ADDON || 
			p->m_buildingtype==BWrepGameData::BTYP_WARP || p->m_buildingtype==BWrepGameData::BTYP_MORPH)
		{
			bx = p->m_pos1;
			by = p->m_pos2;
			return p->m_buildingid;
		}
	}
	// is it a morph? (existing zerg structure being morphed into something else)
	else if(actionID == BWrepGameData::CMD_MORPH)
	{
		// will need to update build order
		const BWrepActionMorph::Params *p = (const BWrepActionMorph::Params *)action->GetParamStruct();
		return p->m_buildingid;
	}

	// no need to update build order
	return -1;
}

//------------------------------------------------------------------------------------------------------------

void ReplayEvtList::_IdentifyUpgrade(int upgID, unsigned long time)
{
	for(int i=0; i<m_currentSelection;i++)
	{
		// identify selected element
		short unitID = GetSelectedUnitID(i);
		if(unitID==-1) continue;

		short buildID=-1;
		switch(upgID)
		{
		case BWrepGameData::UPG_TERRANINFANTRYARMOR:buildID=BWrepGameData::OBJ_ENGINEERINGBAY;break;	//0X00
		case BWrepGameData::UPG_TERRANVEHICLEPLATING:buildID=BWrepGameData::OBJ_ARMORY;break;
		case BWrepGameData::UPG_TERRANSHIPPLATING:buildID=BWrepGameData::OBJ_ARMORY;break;
		case BWrepGameData::UPG_ZERGCARAPACE:buildID=BWrepGameData::OBJ_EVOLUTIONCHAMBER;break;
		case BWrepGameData::UPG_ZERGFLYERCARAPACE:buildID=BWrepGameData::OBJ_SPIRE;break;
		case BWrepGameData::UPG_PROTOSSGROUNDARMOR:buildID=BWrepGameData::OBJ_FORGE;break;//0X05
		case BWrepGameData::UPG_PROTOSSAIRARMOR:buildID=BWrepGameData::OBJ_CYBERNETICSCORE;break;
		case BWrepGameData::UPG_TERRANINFANTRYWEAPONS:buildID=BWrepGameData::OBJ_ENGINEERINGBAY;break;
		case BWrepGameData::UPG_TERRANVEHICLEWEAPONS:buildID=BWrepGameData::OBJ_ARMORY;break;
		case BWrepGameData::UPG_TERRANSHIPWEAPONS:buildID=BWrepGameData::OBJ_ARMORY;break;
		case BWrepGameData::UPG_ZERGMELEEATTACKS:buildID=BWrepGameData::OBJ_EVOLUTIONCHAMBER;break;//0X0A
		case BWrepGameData::UPG_ZERGMISSILEATTACKS:buildID=BWrepGameData::OBJ_EVOLUTIONCHAMBER;break;
		case BWrepGameData::UPG_ZERGFLYERATTACKS:buildID=BWrepGameData::OBJ_SPIRE;break;
		case BWrepGameData::UPG_PROTOSSGROUNDWEAPONS:buildID=BWrepGameData::OBJ_FORGE;break;
		case BWrepGameData::UPG_PROTOSSAIRWEAPONS:buildID=BWrepGameData::OBJ_CYBERNETICSCORE;break;
		case BWrepGameData::UPG_PROTOSSPLASMASHIELDS:buildID=BWrepGameData::OBJ_FORGE;break;
		case BWrepGameData::UPG_U238SHELLS_MARINERANGE:buildID=BWrepGameData::OBJ_ACADEMY;break;	//0X10
		case BWrepGameData::UPG_IONTHRUSTERS_VULTURESPEED:buildID=BWrepGameData::OBJ_MACHINESHOP;break;
		case BWrepGameData::UPG_TITANREACTOR_SCIENCEVESSELENERGY:buildID=BWrepGameData::OBJ_SCIENCEFACILITY;break;
		case BWrepGameData::UPG_OCULARIMPLANTS_GHOSTSIGHT:buildID=BWrepGameData::OBJ_COVERTOPS;break;
		case BWrepGameData::UPG_MOEBIUSREACTOR_GHOSTENERGY:buildID=BWrepGameData::OBJ_COVERTOPS;break;//0X15
		case BWrepGameData::UPG_APOLLOREACTOR_WRAITHENERGY:buildID=BWrepGameData::OBJ_CONTROLTOWER;break;
		case BWrepGameData::UPG_COLOSSUSREACTOR_BATTLECRUISERENERGY:buildID=BWrepGameData::OBJ_PHYSICSLAB;break;
		case BWrepGameData::UPG_VENTRALSACS_OVERLORDTRANSPORT:buildID=BWrepGameData::OBJ_LAIR;break;
		case BWrepGameData::UPG_ANTENNAE_OVERLORDSIGHT:buildID=BWrepGameData::OBJ_LAIR;break;
		case BWrepGameData::UPG_PNEUMATIZEDCARAPACE_OVERLORDSPEED:buildID=BWrepGameData::OBJ_LAIR;break;//0X1A
		case BWrepGameData::UPG_METABOLICBOOST_ZERGLINGSPEED:buildID=BWrepGameData::OBJ_SPAWNINGPOOL;break;
		case BWrepGameData::UPG_ADRENALGLANDS_ZERGLINGATTACK:buildID=BWrepGameData::OBJ_SPAWNINGPOOL;break;
		case BWrepGameData::UPG_MUSCULARAUGMENTS_HYDRALISKSPEED:buildID=BWrepGameData::OBJ_HYDRALISKDEN;break;
		case BWrepGameData::UPG_GROOVEDSPINES_HYDRALISKRANGE:buildID=BWrepGameData::OBJ_HYDRALISKDEN;break;
		case BWrepGameData::UPG_GAMETEMEIOSIS_QUEENENERGY:buildID=BWrepGameData::OBJ_QUEENSNEST;break;
		case BWrepGameData::UPG_DEFILERENERGY:buildID=BWrepGameData::OBJ_DEFILERMOUND;break;	//0X20
		case BWrepGameData::UPG_SINGULARITYCHARGE_DRAGOONRANGE:buildID=BWrepGameData::OBJ_CYBERNETICSCORE;break;
		case BWrepGameData::UPG_LEGENHANCEMENT_ZEALOTSPEED:buildID=BWrepGameData::OBJ_CITADELOFADUN;break;
		case BWrepGameData::UPG_SCARABDAMAGE:buildID=BWrepGameData::OBJ_ROBOTICSSUPPORTBAY;break;
		case BWrepGameData::UPG_REAVERCAPACITY:buildID=BWrepGameData::OBJ_ROBOTICSSUPPORTBAY;break;
		case BWrepGameData::UPG_GRAVITICDRIVE_SHUTTLESPEED:buildID=BWrepGameData::OBJ_ROBOTICSSUPPORTBAY;break;//0X25
		case BWrepGameData::UPG_SENSORARRAY_OBSERVERSIGHT:buildID=BWrepGameData::OBJ_OBSERVATORY;break;
		case BWrepGameData::UPG_GRAVITICBOOSTER_OBSERVERSPEED:buildID=BWrepGameData::OBJ_OBSERVATORY;break;
		case BWrepGameData::UPG_KHAYDARINAMULET_TEMPLARENERGY:buildID=BWrepGameData::OBJ_TEMPLARARCHIVES;break;
		case BWrepGameData::UPG_APIALSENSORS_SCOUTSIGHT:buildID=BWrepGameData::OBJ_FLEETBEACON;break;
		case BWrepGameData::UPG_GRAVITICTHRUSTERS_SCOUTSPEED:buildID=BWrepGameData::OBJ_FLEETBEACON;break;//0X2A
		case BWrepGameData::UPG_CARRIERCAPACITY:buildID=BWrepGameData::OBJ_FLEETBEACON;break;
		case BWrepGameData::UPG_KHAYDARINCORE_ARBITERENERGY:buildID=BWrepGameData::OBJ_ARBITERTRIBUNAL;break;
		case BWrepGameData::UPG_ARGUSJEWEL_CORSAIRENERGY:buildID=BWrepGameData::OBJ_FLEETBEACON;break;
		case BWrepGameData::UPG_ARGUSTALISMAN_DARKTEMPLARENERGY:buildID=BWrepGameData::OBJ_TEMPLARARCHIVES;break;
		case BWrepGameData::UPG_CADUCEUSREACTOR_MEDICENERGY:buildID=BWrepGameData::OBJ_ACADEMY;break;
		case BWrepGameData::UPG_CHITINOUSPLATING_ULTRALISKARMOR:buildID=BWrepGameData::OBJ_ULTRALISKCAVERN;break;
		case BWrepGameData::UPG_ANABOLICSYNTHESIS_ULTRALISKSPEED:buildID=BWrepGameData::OBJ_ULTRALISKCAVERN;break;//0X35
		case BWrepGameData::UPG_CHARONBOOSTERS_GOLIATHRANGE:buildID=BWrepGameData::OBJ_MACHINESHOP;break;
		}

		if(buildID!=-1)
		{
			// we know what building this is
			GetElemList()->SetObjectID(this, unitID,buildID,time,time);
		}
	}
}

//------------------------------------------------------------------------------------------------------------

void ReplayEvtList::_IdentifyResearch(int resID, unsigned long time)
{
	if(m_currentSelection!=1) return;

	// identify selected element
	short unitID = GetSelectedUnitID(0);
	if(unitID==-1) return;

	short buildID=-1;
	switch(resID)
	{
		case BWrepGameData::RES_STIMPACK:buildID=BWrepGameData::OBJ_ACADEMY;break;
		case BWrepGameData::RES_LOCKDOWN:buildID=BWrepGameData::OBJ_COVERTOPS;break;
		case BWrepGameData::RES_EMPSHOCKWAVE:buildID=BWrepGameData::OBJ_SCIENCEFACILITY;break;
		case BWrepGameData::RES_SPIDERMINES:buildID=BWrepGameData::OBJ_MACHINESHOP;break;
		case BWrepGameData::RES_SIEGETANK:buildID=BWrepGameData::OBJ_MACHINESHOP;break;//0X05
		case BWrepGameData::RES_IRRADIATE:buildID=BWrepGameData::OBJ_SCIENCEFACILITY;break;
		case BWrepGameData::RES_YAMATOGUN:buildID=BWrepGameData::OBJ_PHYSICSLAB;break;
		case BWrepGameData::RES_CLOAKINGFIELD:buildID=BWrepGameData::OBJ_CONTROLTOWER;break;//WRAITHS
		case BWrepGameData::RES_PERSONALCLOAKING:buildID=BWrepGameData::OBJ_COVERTOPS;break;//0X0A
		case BWrepGameData::RES_BURROW:buildID=BWrepGameData::OBJ_HATCHERY;break;
		case BWrepGameData::RES_SPAWNBROODLING:buildID=BWrepGameData::OBJ_QUEENSNEST;break;
		case BWrepGameData::RES_PLAGUE:buildID=BWrepGameData::OBJ_DEFILERMOUND;break;
		case BWrepGameData::RES_CONSUME:buildID=BWrepGameData::OBJ_DEFILERMOUND;break;	//0X10
		case BWrepGameData::RES_ENSNARE:buildID=BWrepGameData::OBJ_QUEENSNEST;break;
		case BWrepGameData::RES_PSIONICSTORM:buildID=BWrepGameData::OBJ_TEMPLARARCHIVES;break;
		case BWrepGameData::RES_HALLUCINATION:buildID=BWrepGameData::OBJ_TEMPLARARCHIVES;break;
		case BWrepGameData::RES_RECALL:buildID=BWrepGameData::OBJ_ARBITERTRIBUNAL;break;//0X15
		case BWrepGameData::RES_STASISFIELD:buildID=BWrepGameData::OBJ_ARBITERTRIBUNAL;break;
		case BWrepGameData::RES_RESTORATION:buildID=BWrepGameData::OBJ_ACADEMY;break;
		case BWrepGameData::RES_DISRUPTIONWEB:buildID=BWrepGameData::OBJ_FLEETBEACON;break;
		case BWrepGameData::RES_MINDCONTROL:buildID=BWrepGameData::OBJ_TEMPLARARCHIVES;break;
		case BWrepGameData::RES_OPTICALFLARE:buildID=BWrepGameData::OBJ_ACADEMY;break;
		case BWrepGameData::RES_MAELSTROM:buildID=BWrepGameData::OBJ_TEMPLARARCHIVES;break;
		case BWrepGameData::RES_LURKERASPECT:buildID=BWrepGameData::OBJ_HYDRALISKDEN;break;	//0X20
	}

	if(buildID!=-1)
	{
		// we know what building this is
		GetElemList()->SetObjectID(this, unitID,buildID,time,time);
	}
}

//------------------------------------------------------------------------------------------------------------

void ReplayEvtList::_IdentifyTrain(int objectID, unsigned long time)
{
	for(int i=0; i<m_currentSelection;i++)
	{
		// identify selected element
		short unitID = GetSelectedUnitID(i);
		if(unitID==-1) continue;

		short buildID=-1;
		short buildID2=-1;
		switch(objectID)
		{
		case BWrepGameData::OBJ_MARINE		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_BARRACKS;break;	//0X00
		case BWrepGameData::OBJ_GHOST		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_BARRACKS;break;
		case BWrepGameData::OBJ_VULTURE		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_FACTORY;break;
		case BWrepGameData::OBJ_GOLIATH		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_FACTORY;break;
		case BWrepGameData::OBJ_SIEGETANK	:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_FACTORY;break; //0X05
		case BWrepGameData::OBJ_SCV			:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_COMMANDCENTER;break;
		case BWrepGameData::OBJ_WRAITH		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_STARPORT;break;
		case BWrepGameData::OBJ_SCIENCEVESSEL:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_STARPORT;break;
		case BWrepGameData::OBJ_DROPSHIP	:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_STARPORT;break;
		case BWrepGameData::OBJ_BATTLECRUISER:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_STARPORT;break;
		case BWrepGameData::OBJ_NUKE		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_NUCLEARSILO;break;
		case BWrepGameData::OBJ_FIREBAT		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_BARRACKS;break;	//0X20
		case BWrepGameData::OBJ_MEDIC		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_BARRACKS;break;
		case BWrepGameData::OBJ_INFESTEDTERRAN:buildID=-1;break;
		case BWrepGameData::OBJ_VALKYRIE	:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_STARPORT;break; //0X3A
		case BWrepGameData::OBJ_CORSAIR		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_STARGATE;break;
		case BWrepGameData::OBJ_DARKTEMPLAR	:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_GATEWAY;break;
		case BWrepGameData::OBJ_PROBE		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_NEXUS;break;	//0X40
		case BWrepGameData::OBJ_ZEALOT		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_GATEWAY;break;
		case BWrepGameData::OBJ_DRAGOON		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_GATEWAY;break;
		case BWrepGameData::OBJ_HIGHTEMPLAR	:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_GATEWAY;break;
		case BWrepGameData::OBJ_SCOUT		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_STARGATE;break;
		case BWrepGameData::OBJ_ARBITER		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_STARGATE;break;
		case BWrepGameData::OBJ_CARRIER		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_STARGATE;m_bHasCarrier=true;break;
		case BWrepGameData::OBJ_SHUTTLE		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_ROBOTICSFACILITY;break; //0X45
		case BWrepGameData::OBJ_REAVER		:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_ROBOTICSFACILITY;m_bHasReaver=true;break;
		case BWrepGameData::OBJ_OBSERVER	:if(m_currentSelection==1) buildID=BWrepGameData::OBJ_ROBOTICSFACILITY;break;

		case BWrepGameData::OBJ_LURKER		:buildID=BWrepGameData::OBJ_HYDRALISK;buildID2=objectID;break;
		case BWrepGameData::OBJ_ZERGLING	:buildID=BWrepGameData::OBJ_LARVA;buildID2=objectID;break; //0X25
		case BWrepGameData::OBJ_HYDRALISK	:buildID=BWrepGameData::OBJ_LARVA;buildID2=objectID;break;
		case BWrepGameData::OBJ_ULTRALISK	:buildID=BWrepGameData::OBJ_LARVA;buildID2=objectID;break;
		case BWrepGameData::OBJ_DRONE		:buildID=BWrepGameData::OBJ_LARVA;buildID2=objectID;break;
		case BWrepGameData::OBJ_OVERLORD	:buildID=BWrepGameData::OBJ_LARVA;buildID2=objectID;break; //0X2A
		case BWrepGameData::OBJ_MUTALISK	:buildID=BWrepGameData::OBJ_LARVA;buildID2=objectID;break;
		case BWrepGameData::OBJ_GUARDIAN	:buildID=BWrepGameData::OBJ_MUTALISK;buildID2=objectID;break;
		case BWrepGameData::OBJ_QUEEN		:buildID=BWrepGameData::OBJ_LARVA;buildID2=objectID;break;
		case BWrepGameData::OBJ_DEFILER		:buildID=BWrepGameData::OBJ_LARVA;buildID2=objectID;break;
		case BWrepGameData::OBJ_SCOURGE		:buildID=BWrepGameData::OBJ_LARVA;buildID2=objectID;break;
		case BWrepGameData::OBJ_DEVOURER	:buildID=BWrepGameData::OBJ_MUTALISK;buildID2=objectID;break;
		case BWrepGameData::OBJ_SCARAB		:buildID=BWrepGameData::OBJ_REAVER;break;
		case BWrepGameData::OBJ_INTERCEPTOR	:buildID=BWrepGameData::OBJ_CARRIER;break;
		}

		if(buildID!=-1)
		{
			// we know what building (or unit) this is
			GetElemList()->SetObjectID(this, unitID,buildID,time,time);

			// special case for zergs (buildID2 = id of the new unit after evolution)
			if(buildID2!=-1) GetElemList()->SetObjectID(this, unitID,buildID2,BWElement::MAXTIME,time);
		}
	}
}

//------------------------------------------------------------------------------------------------------------

void ReplayEvtList::_IdentifyBuild(int objectID, unsigned long time, char btype)
{
	if(m_currentSelection!=1) return;

	// identify selected element
	short unitID = GetSelectedUnitID(0);
	if(unitID==-1) return;

	short builderID=-1;
	short buildID2=-1;
	switch(objectID)
	{
		case BWrepGameData::OBJ_BARRACKS		:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_SCV;break;	//0X00
		case BWrepGameData::OBJ_FACTORY			:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_SCV;break;
		case BWrepGameData::OBJ_COMMANDCENTER	:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_SCV;break;
		case BWrepGameData::OBJ_STARPORT		:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_SCV;break;
		case BWrepGameData::OBJ_SCIENCEFACILITY	:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_SCV;break;
		case BWrepGameData::OBJ_ENGINEERINGBAY	:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_SCV;break;
		case BWrepGameData::OBJ_ARMORY			:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_SCV;break;
		case BWrepGameData::OBJ_MISSILETURRET	:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_SCV;break;
		case BWrepGameData::OBJ_BUNKER			:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_SCV;break;
		case BWrepGameData::OBJ_COVERTOPS		:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_SCIENCEFACILITY; m_hasCovertOps=true; break;
		case BWrepGameData::OBJ_PHYSICSLAB		:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_SCIENCEFACILITY;break;
		case BWrepGameData::OBJ_NUCLEARSILO		:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_COMMANDCENTER;break; //0X05
		case BWrepGameData::OBJ_COMSAT			:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_COMMANDCENTER;break; //0X05
		case BWrepGameData::OBJ_MACHINESHOP		:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_FACTORY;break; //0X05
		case BWrepGameData::OBJ_CONTROLTOWER	:if(btype==BWrepGameData::BTYP_BUILD) builderID=BWrepGameData::OBJ_STARPORT;break;
		case BWrepGameData::OBJ_STARGATE		:builderID=BWrepGameData::OBJ_PROBE;break;
		case BWrepGameData::OBJ_GATEWAY			:builderID=BWrepGameData::OBJ_PROBE;break;
		case BWrepGameData::OBJ_NEXUS			:builderID=BWrepGameData::OBJ_PROBE;break;
		case BWrepGameData::OBJ_PYLON			:builderID=BWrepGameData::OBJ_PROBE;break;	//0X20
		case BWrepGameData::OBJ_ASSIMILATOR		:builderID=BWrepGameData::OBJ_PROBE;break;	//0X20
		case BWrepGameData::OBJ_OBSERVATORY		:builderID=BWrepGameData::OBJ_PROBE;break;	//0X20
		case BWrepGameData::OBJ_PHOTONCANNON	:builderID=BWrepGameData::OBJ_PROBE;break;	//0X20
		case BWrepGameData::OBJ_CITADELOFADUN	:builderID=BWrepGameData::OBJ_PROBE;break;	//0X20
		case BWrepGameData::OBJ_CYBERNETICSCORE	:builderID=BWrepGameData::OBJ_PROBE;break;	//0X20
		case BWrepGameData::OBJ_TEMPLARARCHIVES	:builderID=BWrepGameData::OBJ_PROBE;break;	//0X20
		case BWrepGameData::OBJ_FORGE			:builderID=BWrepGameData::OBJ_PROBE;break;	//0X20
		case BWrepGameData::OBJ_FLEETBEACON		:builderID=BWrepGameData::OBJ_PROBE; m_bHasFleetBeacon=true; break;	//0X20
		case BWrepGameData::OBJ_ARBITERTRIBUNAL	:builderID=BWrepGameData::OBJ_PROBE;break;	//0X20
		case BWrepGameData::OBJ_ROBOTICSSUPPORTBAY:builderID=BWrepGameData::OBJ_PROBE;break;	//0X20
		case BWrepGameData::OBJ_SHIELDBATTERY	:builderID=BWrepGameData::OBJ_PROBE;break;	//0X20
		case BWrepGameData::OBJ_HATCHERY		:builderID=BWrepGameData::OBJ_DRONE;buildID2=objectID;break;
		case BWrepGameData::OBJ_HIVE			:builderID=BWrepGameData::OBJ_HATCHERY;buildID2=objectID;break;
		case BWrepGameData::OBJ_LAIR			:builderID=BWrepGameData::OBJ_HATCHERY;buildID2=objectID;break;
		case BWrepGameData::OBJ_HYDRALISKDEN	:builderID=BWrepGameData::OBJ_DRONE;buildID2=objectID;break;	//0X20
		case BWrepGameData::OBJ_DEFILERMOUND	:builderID=BWrepGameData::OBJ_DRONE;buildID2=objectID;break;	//0X20
		case BWrepGameData::OBJ_GREATERSPIRE	:builderID=BWrepGameData::OBJ_SPIRE;buildID2=objectID;break;	//0X20
		case BWrepGameData::OBJ_QUEENSNEST		:builderID=BWrepGameData::OBJ_DRONE;buildID2=objectID;break;	//0X20
		case BWrepGameData::OBJ_EVOLUTIONCHAMBER:builderID=BWrepGameData::OBJ_DRONE;buildID2=objectID;break;	//0X20
		case BWrepGameData::OBJ_ULTRALISKCAVERN	:builderID=BWrepGameData::OBJ_DRONE;buildID2=objectID;break;	//0X20
		case BWrepGameData::OBJ_SPIRE			:builderID=BWrepGameData::OBJ_DRONE;buildID2=objectID;break;	//0X20
		case BWrepGameData::OBJ_SPAWNINGPOOL	:builderID=BWrepGameData::OBJ_DRONE;buildID2=objectID;break;	//0X20
		case BWrepGameData::OBJ_CREEPCOLONY		:builderID=BWrepGameData::OBJ_DRONE;buildID2=objectID;break;	//0X20
		case BWrepGameData::OBJ_SPORECOLONY		:builderID=BWrepGameData::OBJ_CREEPCOLONY;buildID2=objectID;break;	//0X20
		case BWrepGameData::OBJ_SUNKENCOLONY	:builderID=BWrepGameData::OBJ_CREEPCOLONY;buildID2=objectID;break;	//0X20
		case BWrepGameData::OBJ_EXTRACTOR		:builderID=BWrepGameData::OBJ_DRONE;buildID2=objectID;break;	//0X20
	}

	if(builderID!=-1)
	{
		// we know what unit this is
		GetElemList()->SetObjectID(this, unitID,builderID,time,time);

		// special case for zergs (buildID2 = id of the new unit after evolution)
		if(buildID2!=-1) GetElemList()->SetObjectID(this, unitID,buildID2,BWElement::MAXTIME,time);
	}
}

//------------------------------------------------------------------------------------------------------------

void ReplayEvtList::_IdentifyCommand(int cmdID, int subcmd, unsigned long time)
{
	// for each selected element
	for(int i=0; i<m_currentSelection;i++)
	{
		// identify selected element
		short unitID = GetSelectedUnitID(i);
		if(unitID==-1) continue;

		short buildID=-1;
		short buildID2=-1;
		switch(cmdID)
		{
			case BWrepGameData::CMD_RETURNCARGO:
				if(m_race==IStarcraftPlayer::RACE_TERRAN)
					buildID2=buildID=BWrepGameData::OBJ_SCV;
				else if(m_race==IStarcraftPlayer::RACE_PROTOSS)
					buildID2=buildID=BWrepGameData::OBJ_PROBE;
				else
					buildID=BWrepGameData::OBJ_DRONE;
				break;
			case BWrepGameData::CMD_CLOAK:;
				if(m_race==IStarcraftPlayer::RACE_TERRAN)
				{
					if(!m_hasCovertOps) 
						{buildID2=buildID=BWrepGameData::OBJ_WRAITH;}	
				}
				break;	//CLOAK?
			case BWrepGameData::CMD_DECLOAK:;break;	//DECLOAK?
			case BWrepGameData::CMD_UNSIEGE:
			case BWrepGameData::CMD_SIEGE:
				buildID2=buildID=BWrepGameData::OBJ_SIEGETANK;break;
			case BWrepGameData::CMD_ARM:;break;
			case BWrepGameData::CMD_UNLOADALL:
				if(m_race==IStarcraftPlayer::RACE_TERRAN)
					buildID=BWrepGameData::OBJ_BUNKER;
				break;
			case BWrepGameData::CMD_UNLOAD:
				if(m_race==IStarcraftPlayer::RACE_TERRAN)
					buildID=BWrepGameData::OBJ_DROPSHIP;
				else if(m_race==IStarcraftPlayer::RACE_PROTOSS)
					buildID=BWrepGameData::OBJ_SHUTTLE;
				else
					buildID=BWrepGameData::OBJ_OVERLORD;
				buildID2=buildID;
				break;
			case BWrepGameData::CMD_MERGEARCHON:
				buildID=BWrepGameData::OBJ_HIGHTEMPLAR;
				buildID2=BWrepGameData::OBJ_ARCHON;
				break;	//0X2A
			case BWrepGameData::CMD_CANCELNUKE:buildID2=buildID=BWrepGameData::OBJ_GHOST;break;
			case BWrepGameData::CMD_STIM:buildID2=buildID= m_bHasAcademy ? BWrepGameData::OBJ_RINEORBAT : BWrepGameData::OBJ_MARINE;break;
			case BWrepGameData::CMD_MERGEDARKARCHON:
				buildID=BWrepGameData::OBJ_DARKTEMPLAR;
				buildID2=BWrepGameData::OBJ_DARKARCHON;
				break;
			case BWrepGameData::CMD_ATTACK: 
			{
				switch(subcmd)
				{
				case BWrepGameData::ATT_UNLOAD: 
					if(m_race==IStarcraftPlayer::RACE_TERRAN)
						buildID=BWrepGameData::OBJ_DROPSHIP;
					else if(m_race==IStarcraftPlayer::RACE_PROTOSS)
						buildID=BWrepGameData::OBJ_SHUTTLE;
					else
						buildID=BWrepGameData::OBJ_OVERLORD;
					buildID2=buildID;
					break;
				case BWrepGameData::ATT_INFESTCC: buildID2=buildID=BWrepGameData::OBJ_QUEEN; break;//0x1B,
				case BWrepGameData::ATT_REPAIR: buildID2=buildID=BWrepGameData::OBJ_SCV; break;//0x22,
				case BWrepGameData::ATT_GATHER: 
					if(m_race==IStarcraftPlayer::RACE_TERRAN)
						buildID2=buildID=BWrepGameData::OBJ_SCV;
					else if(m_race==IStarcraftPlayer::RACE_PROTOSS)
						buildID2=buildID=BWrepGameData::OBJ_PROBE;
					else
						buildID=BWrepGameData::OBJ_DRONE;
					break;
				case BWrepGameData::ATT_YAMATO: buildID2=buildID=BWrepGameData::OBJ_BATTLECRUISER; break;//0x71,
				case BWrepGameData::ATT_LOCKDOWN: buildID2=buildID=BWrepGameData::OBJ_GHOST; break;//0x73,
				case BWrepGameData::ATT_DARKSWARM: buildID2=buildID=BWrepGameData::OBJ_DEFILER; break;//0x77,
				case BWrepGameData::ATT_PARASITE: buildID2=buildID=BWrepGameData::OBJ_QUEEN; break;//0x78,
				case BWrepGameData::ATT_SPAWNBROODLING: buildID2=buildID=BWrepGameData::OBJ_QUEEN; break;//0x79,
				case BWrepGameData::ATT_EMP: buildID2=buildID=BWrepGameData::OBJ_SCIENCEVESSEL; break;//0x7A,	//0X007A
				case BWrepGameData::ATT_LAUNCHNUKE: buildID2=buildID=BWrepGameData::OBJ_GHOST; break;//0x7E,
				case BWrepGameData::ATT_LAYMINE: buildID2=buildID=BWrepGameData::OBJ_VULTURE; break;//0x84,
				case BWrepGameData::ATT_COMSATSCAN: buildID2=buildID=BWrepGameData::OBJ_COMSAT; break;//0x8B,
				case BWrepGameData::ATT_DEFENSEMATRIX: buildID2=buildID=BWrepGameData::OBJ_SCIENCEVESSEL; break;//0x8D,
				case BWrepGameData::ATT_RECALL: 
					if(m_race==IStarcraftPlayer::RACE_PROTOSS)
						{buildID2=buildID=BWrepGameData::OBJ_ARBITER;}
					break;//0x8F, // & LOCKDOWN
				case BWrepGameData::ATT_PLAGUE: buildID2=buildID=BWrepGameData::OBJ_DEFILER; break;//0x90,	//0X90
				case BWrepGameData::ATT_CONSUME: buildID2=buildID=BWrepGameData::OBJ_DEFILER; break;//0x91,
				case BWrepGameData::ATT_ENSNARE: buildID2=buildID=BWrepGameData::OBJ_QUEEN; break;//0x92,
				case BWrepGameData::ATT_STASIS: buildID2=buildID=BWrepGameData::OBJ_ARBITER; break;//0x93,
				case BWrepGameData::ATT_HEAL: buildID2=buildID=BWrepGameData::OBJ_MEDIC; break;//0xB1,
				case BWrepGameData::ATT_RESTORE: buildID2=buildID=BWrepGameData::OBJ_MEDIC; break;//0xB4,
				case BWrepGameData::ATT_DISRUPTIONWEB: buildID2=buildID=BWrepGameData::OBJ_CORSAIR; break;//0xB5, //0XB5
				case BWrepGameData::ATT_MINDCONTROL:buildID2=buildID=BWrepGameData::OBJ_DARKARCHON; break; 
				case BWrepGameData::ATT_FEEDBACK: buildID2=buildID=BWrepGameData::OBJ_DARKARCHON; break;
				case BWrepGameData::ATT_OPTICFLARE: buildID2=buildID=BWrepGameData::OBJ_MEDIC; break;//0xB9,
				case BWrepGameData::ATT_MAELSTROM: buildID2=buildID=BWrepGameData::OBJ_DARKARCHON; break;//0xBA, //0XBA
				case BWrepGameData::ATT_IRRADIATE: buildID2=buildID=BWrepGameData::OBJ_SCIENCEVESSEL; break;//0xC0,
				}
			}
		}

		if(buildID!=-1)
		{
			// we know what unit this is
			GetElemList()->SetObjectID(this, unitID,buildID,time,time);

			// special case for zergs & toss (buildID2 = id of the new unit after evolution)
			if(buildID2!=-1) GetElemList()->SetObjectID(this, unitID,buildID2,BWElement::MAXTIME,time);
		}
	}
}

//------------------------------------------------------------------------------------------------------------

void ReplayEvtList::_AdjustData(const IStarcraftAction *action, int& actionID, int &objectID, const char * &parameters, int& subcmd )
{
	static char tmpParam[255];
	const char *realParameters = action->GetParameters(GetElemList());

	// adjust action data
	if(actionID == BWrepGameData::CMD_BUILD) 
	{
		// get building id
		const BWrepActionBuild::Params *p = (const BWrepActionBuild::Params *)action->GetParamStruct();
		objectID = p->m_buildingid;

		// identify selected unit
		_IdentifyBuild(objectID,action->GetTime(),p->m_buildingtype);
	}
	else if(actionID == BWrepGameData::CMD_ARM) 
	{
		parameters="Interceptor/Scarab"; 
		actionID=BWrepGameData::CMD_TRAIN; 
		if(!m_bHasFleetBeacon)
			objectID=BWrepGameData::OBJ_SCARAB;
		else if(!m_bHasReaver && m_bHasCarrier)
			objectID=BWrepGameData::OBJ_INTERCEPTOR;
		else
			objectID=BWrepGameData::OBJ_SCARABORINTERCEPTOR;
	}
	else if(actionID == BWrepGameData::CMD_MERGEARCHON) 
	{
		//actionID=BWrepGameData::CMD_TRAIN; 
		parameters="Archon"; 
		objectID=BWrepGameData::OBJ_ARCHON;
	}
	else if(actionID == BWrepGameData::CMD_MERGEDARKARCHON) 
	{
		//actionID=BWrepGameData::CMD_TRAIN; 
		parameters="Dark Archon";
		objectID=BWrepGameData::OBJ_DARKARCHON;
	}
	else if(actionID == BWrepGameData::CMD_MORPH) 
	{
		// get building id
		const BWrepActionMorph::Params *p = (const BWrepActionMorph::Params *)action->GetParamStruct();
		objectID = p->m_buildingid;
		//actionID=BWrepGameData::CMD_BUILD;

		// identify selected unit
		_IdentifyBuild(objectID,action->GetTime(),0);
	}
	else if(actionID == BWrepGameData::CMD_UPGRADE) 
	{
		// get upgrade id
		const BWrepActionUpgrade::Params *p = (const BWrepActionUpgrade::Params *)action->GetParamStruct();
		objectID = p->m_upgid;

		// identify selected building
		_IdentifyUpgrade(objectID,action->GetTime());
	}
	else if(actionID == BWrepGameData::CMD_RESEARCH) 
	{
		// get research id
		const BWrepActionResearch::Params *p = (const BWrepActionResearch::Params *)action->GetParamStruct();
		objectID = p->m_techid;

		// identify selected building
		_IdentifyResearch(objectID,action->GetTime());
	}
	else if(actionID == BWrepGameData::CMD_ATTACK) 
	{
		// remove action at the end
		strcpy(tmpParam,realParameters);
		char *p=strrchr(tmpParam,',');
		if(p!=0) {*p=0; parameters=tmpParam;}
		if(tmpParam[strlen(tmpParam)-1]==',') tmpParam[strlen(tmpParam)-1]=0;
		const BWrepActionAttack::Params *pa = (const BWrepActionAttack::Params *)action->GetParamStruct();
		subcmd = pa->m_type;
	}
	else if(actionID == BWrepGameData::CMD_TRAIN) 
	{
		// get unit id
		const BWrepActionTrain::Params *p = (const BWrepActionTrain::Params *)action->GetParamStruct();
		objectID = p->m_unitType;

		// identify selected building or unit
		_IdentifyTrain(objectID,action->GetTime());
	}
	else if(actionID == BWrepGameData::CMD_HATCH) 
	{
		// get unit id
		const BWrepActionHatch::Params *p = (const BWrepActionHatch::Params *)action->GetParamStruct();
		objectID = p->m_unitType;

		// identify selected building or unit
		_IdentifyTrain(objectID,action->GetTime());
	}
	else if(actionID == BWrepGameData::CMD_SELECT) 
	{
		// get building id
		const BWrepActionSelect::Params *p = (const BWrepActionSelect::Params *)action->GetParamStruct();
		if(p->m_unitCount==0) actionID = BWrepGameData::CMD_DESELECTAUTO;
	}
	
	if(m_race==IStarcraftPlayer::RACE_TERRAN)
	{
		if(actionID == BWrepGameData::CMD_ATTACK && subcmd==BWrepGameData::ATT_RECALL) {subcmd=BWrepGameData::ATT_IRRADIATE;}
		else if(strcmp(realParameters,"Hallucination")==0) {parameters="Ghost Sight";}
		else if(strcmp(realParameters,"Recall")==0) {parameters="Moebius Reactor";}
		else if(strcmp(realParameters,"Stasis Field")==0) {parameters="Wraith Energy";}
	}
	else if(m_race==IStarcraftPlayer::RACE_PROTOSS)
	{
		if(strcmp(realParameters,"Wraith Energy")==0) {parameters="Stasis Field";}
		else if(strcmp(realParameters,"Ghost Sight")==0) {parameters="Hallucination";}
		else if(strcmp(realParameters,"Moebius Reactor")==0) {parameters="Recall";}
	}

	// try to identify unit from command
	_IdentifyCommand(actionID, subcmd, action->GetTime());
}

//------------------------------------------------------------------------------------------------------------

void ReplayBuildOrder::AddBuildOrder(int actionID, unsigned long time, int objectID)
{
	// is it a building?
	if(actionID == BWrepGameData::CMD_BUILD || actionID == BWrepGameData::CMD_MORPH) 
	{
		// if we already have objects in bo
		int count = Buildings().GetCount();
		if(count>0)
		{
			// get last object in bo
			unsigned int obj = Buildings().GetObject(count-1);
			if(objectID==(int)obj)
			{
				// dont count double extractor/ref/assim or double cc/nexus/hatch, etc
				if(obj==BWrepGameData::OBJ_EXTRACTOR || obj==BWrepGameData::OBJ_REFINERY || obj==BWrepGameData::OBJ_ASSIMILATOR ||
				   obj==BWrepGameData::OBJ_NEXUS || obj==BWrepGameData::OBJ_HATCHERY || obj==BWrepGameData::OBJ_COMMANDCENTER ||
				   obj==BWrepGameData::OBJ_ROBOTICSFACILITY || obj==BWrepGameData::OBJ_CYBERNETICSCORE || obj==BWrepGameData::OBJ_TEMPLARARCHIVES ||
				   obj==BWrepGameData::OBJ_FORGE || obj==BWrepGameData::OBJ_HYDRALISKDEN ||
				   obj==BWrepGameData::OBJ_SPIRE || obj==BWrepGameData::OBJ_ENGINEERINGBAY || 
				   obj==BWrepGameData::OBJ_BUNKER || obj==BWrepGameData::OBJ_CITADELOFADUN ||
				   obj==BWrepGameData::OBJ_ACADEMY
				   )
				{
					// update time
					Buildings().UpdateTime(count-1,time);
					return;
				}
			}
		}

		// add building in bo
		Buildings().AddObject(time,objectID);
	}
	// is it a unit?
	else if(actionID == BWrepGameData::CMD_TRAIN || actionID == BWrepGameData::CMD_HATCH)
	{
		// discard scv, probe and drone
		if(objectID!=BWrepGameData::OBJ_SCV && objectID!=BWrepGameData::OBJ_DRONE && objectID!=BWrepGameData::OBJ_PROBE)
		{
			if(!Buildings().IsFull() || time<=Buildings().GetLastTime())
				Units().AddObject(time,objectID);
		}
	}
	// is it an upgrade?
	else if(actionID == BWrepGameData::CMD_UPGRADE)
	{
		// if we already have objects in bo
		int count = Buildings().GetCount();
		if(count>0)
		{
			// get last object in bo
			unsigned int obj = Buildings().GetObject(count-1);
			if(objectID==(int)obj)
			{
				// dont count same upgrade twice
				// update time
				Buildings().UpdateTime(count-1,time);
				return;
			}
		}

		// add upgrade in bo
		Upgrade().AddObject(time,objectID);
	}
	// is it a a research?
	else if(actionID == BWrepGameData::CMD_RESEARCH)
	{
		// if we already have objects in bo
		int count = Buildings().GetCount();
		if(count>0)
		{
			// get last object in bo
			unsigned int obj = Buildings().GetObject(count-1);
			if(objectID==(int)obj)
			{
				// dont count same research twice
				// update time
				Buildings().UpdateTime(count-1,time);
				return;
			}
		}

		// add research in bo
		Research().AddObject(time,objectID);
	}
}

//------------------------------------------------------------------------------------------------------------

void ReplayBuildOrder::RemoveBuildOrder(int actionID, unsigned long time, int objectID)
{
	// is it a building?
	if(actionID == BWrepGameData::CMD_BUILD || actionID == BWrepGameData::CMD_MORPH) 
	{
		Buildings().RemoveObject(time,objectID);
	}
	// is it a unit?
	else if(actionID == BWrepGameData::CMD_TRAIN || actionID == BWrepGameData::CMD_HATCH)
	{
		Units().RemoveObject(time,objectID);
	}
	// is it an upgrade?
	else if(actionID == BWrepGameData::CMD_UPGRADE)
	{
		Upgrade().RemoveObject(time,objectID);
	}
	// is it a a research?
	else if(actionID == BWrepGameData::CMD_RESEARCH)
	{
		Research().RemoveObject(time,objectID);
	}
}

//------------------------------------------------------------------------------------------------------------

void ReplayEvtList::_Discard(ReplayEvt *evt)
{
	evt->Discard();
	m_discardedActions++;
}

//------------------------------------------------------------------------------------------------------------

bool ReplayEvtList::_IsValidBuildEvent(ReplayEvt *evt, ReplayEvt *prevEvt, int buildingID)
{
	// get previous event
	bool bValidEvent=true;
	int eventCount = GetEventCount();
	assert(prevEvt!=0);

	// decide time limit
	unsigned long timeLimit = evt->Time()<4000 ? 170 : evt->Time()<7000 ? 120 : 40;

	if(evt->ActionID()==BWrepGameData::CMD_MORPH)
	{
		// is it the same event than the previous one?
		while(prevEvt!=0 && (evt->Time()-prevEvt->Time()<=timeLimit))
		{
			// do we have the same building during the time limit?
			if(prevEvt->Type()==evt->Type() && prevEvt->UnitIdx()==buildingID)
			{
				bValidEvent=false;
				break;
			}

			eventCount--;
			prevEvt = eventCount==0 ? 0 : GetEvent(eventCount-1);
		}
	}

	// do we have a build event at same x,y position on the map?
	else if(evt->ActionID()==BWrepGameData::CMD_BUILD)
	{
		const BWrepActionBuild::Params *p = (const BWrepActionBuild::Params *)evt->GetAction()->GetParamStruct();
		int x=p->m_pos1; int y=p->m_pos2;
		timeLimit = m_replay->QueryFile()->QueryHeader()->Sec2Tick(30);
		while(bValidEvent && prevEvt!=0 && (evt->Time()-prevEvt->Time()<=timeLimit))
		{
			// do we have the same building during the time limit?
			if(prevEvt->Type()==BWrepGameData::CMD_BUILD && prevEvt->UnitIdx()==buildingID)
			{
				//int mindist = (buildingID == BWrepGameData::OBJ_COMMANDCENTER || buildingID == BWrepGameData::OBJ_NEXUS) ? 8 : 2;
				int mindistX = gAllUnits[buildingID].width;
				int mindistY = gAllUnits[buildingID].height;

				const BWrepActionBuild::Params *p = (const BWrepActionBuild::Params *)prevEvt->GetAction()->GetParamStruct();
				if(abs(p->m_pos1-x)<mindistX && abs(p->m_pos2-y)<mindistY) 
				{
					bValidEvent=false;
					break;
				}
			}

			eventCount--;
			prevEvt = eventCount==0 ? 0 : GetEvent(eventCount-1);
		}
	}	

	if(!bValidEvent)
	{
		// discard the first event
		_Discard(prevEvt); 
		// remove building from build order
		m_bo.RemoveBuildOrder(evt->ActionID(),prevEvt->Time(),buildingID);
	}

	return bValidEvent;
}

//------------------------------------------------------------------------------------------------------------

bool ReplayEvtList::_IsValidTrainEvent(ReplayEvt *evt, ReplayEvt *prevEvt, int unitID)
{
	// get previous event
	int eventCount = GetEventCount();
	assert(prevEvt!=0);

	// if we have a previous event
	bool bValidEvent=true;

	// is it the same event than the previous one?
	if(prevEvt->Type()==evt->Type() && prevEvt->UnitIdx()==unitID && evt->Time()-prevEvt->Time()<=10)
		IncSimilar();

	// compute acceptable amount of similar train depending on game advancement
	int limitSimilar = 5; 
	if(evt->Time()<2000) limitSimilar = 3;
	else if(evt->Time()<4000) limitSimilar = 4;
	else if(evt->Time()<8000) limitSimilar = 5;

	// if the sum of similar units great than the acceptable limit
	if(GetSimilar()>=limitSimilar) 
	{
		// discard event
		bValidEvent=false;
		_Discard(evt);
	}

	return bValidEvent;
}

//------------------------------------------------------------------------------------------------------------

bool ReplayEvtList::_IsValidUpgradeEvent(ReplayEvt *evt, ReplayEvt *prevEvt, int techID)
{
//	assert(evt->Time()!=18230);

	// check that we are not doing the same upgrade multiple times
	bool bValidEvent=true;
	if(m_upgradesCount[techID]==(unsigned int)gAllTechs[techID].maxtry)
		bValidEvent=false;

	// if event is possible
	if(bValidEvent)
	{
		// if we have a previous event
		if(prevEvt!=0)
		{
			// is it the same event than the previous one?
			if(prevEvt->Type()==evt->Type() && prevEvt->UnitIdx()==techID && evt->Time()-prevEvt->Time()<=15)
				bValidEvent=false;
		}
	}

	// discard event if necessary
	if(!bValidEvent) _Discard(evt);

	return bValidEvent;
}

//------------------------------------------------------------------------------------------------------------

// returns the number of units that must be hatched depending on the unit to hatch
// and the currently selected units. For example, if the unit to hatch is a lurker
// we count how many hydralisk we have in the selection. Unknown units are considered
// as potential hatch units.So for example, if we have 2 hydras and 1 unknown in the 
// current selection, that will make 3 lurkers, even though we're not 100% sure on the last one.
//
int ReplayEvtList::GetSelectionForHatch(int unitType, unsigned long now) const
{
	int unittocount=0;

	// define which units we must count depending on the unit we want to hatch
	switch(unitType)
	{
		case BWrepGameData::OBJ_LURKER:
			unittocount = BWrepGameData::OBJ_HYDRALISK;
			break;
		case BWrepGameData::OBJ_GUARDIAN:
		case BWrepGameData::OBJ_DEVOURER:
			unittocount = BWrepGameData::OBJ_MUTALISK;
			break;
		default:
			unittocount = BWrepGameData::OBJ_LARVA;
			break;
	}

	// count units in selection
	int count=0;
	for(int i=0;i<GetSelection();i++)
	{
		short unitID = GetSelectedUnitID(i);
		short objectid = GetObjectID(unitID, now);

		if(objectid==-1 || objectid==unitType)
			count++;
	}

	return count;

}

//------------------------------------------------------------------------------------------------------------

// process map coverage
void ReplayEvtList::ProcessMapCoverage(unsigned long actime)
{
	// new slot?
	if(m_mapSurface!=0)
	{
		int slot = actime/RES_INTERVAL_TICK;
		if(m_currentSlot!=slot)
		{
			// if we already processed a current slot
			if(m_currentSlot!=-1)
			{
				// process current map surface
				MapElem sum(0,0);
				unsigned __int64 unit=0;
				m_mapSurface->Sum(sum,unit);
				for(int i=m_currentSlot;i<slot;i++)
				{
					GetResourceFromIdx(i)->SetMapCoverage(sum.m_buildingid);
					GetResourceFromIdx(i)->SetMapCoverageUnit(unit);
				}
				m_mapSurface->ClearUnit();
			}
			m_currentSlot = slot;
		}
	}
}

//------------------------------------------------------------------------------------------------------------

unsigned long ReplayEvtList::AddEvent(IStarcraftAction *action, const char* &parameters)
{
	int actionID = action->GetID();
	int objectID = -1;
	int bx=-1,by=-1;
	int subcmd=-1;
	int eventCount = GetEventCount();
	bool eventValidForAPM=true;

	//assert(strcmp(parameters,"Guardian")!=0);
	//assert(action->GetTime()<19200);

	// count events in beginning of game
	if(action->GetTime()<MINAPMVALIDTIME) {eventValidForAPM=false; m_eventsBegin++;}

	// get previous event
	ReplayEvt *prevEvt = eventCount==0 ? 0 : GetEvent(eventCount-1);

	// is it a build of something? TO CALL BEFORE _AdjustData
	int buildingid = _HandleBuild(action,bx,by);

	// adjust action data & get unit ID
	_AdjustData(action, actionID, objectID, parameters, subcmd );

	// handle selection of units
	bool suspect = _HandleSelection(action);

	// create event - this will initialize the corresponding resource slot if not done yet
	ReplayEvt evt(this, action,GetSelection(), actionID,subcmd,objectID,prevEvt, suspect);

	// update resources
	if(prevEvt!=0)
	{
		if(actionID == BWrepGameData::CMD_TRAIN || actionID == BWrepGameData::CMD_HATCH || 
			actionID == BWrepGameData::CMD_MERGEARCHON || actionID == BWrepGameData::CMD_MERGEDARKARCHON) 
		{
			// is train event valid?
			if(_IsValidTrainEvent(&evt, prevEvt, objectID))
			{
				// update resources and distribution
				int unitsToAdd = 
					//actionID == BWrepGameData::CMD_HATCH ? GetSelectionForHatch(objectID,evt.Time()) : 
					actionID == BWrepGameData::CMD_HATCH ? GetSelection() : 
					actionID == BWrepGameData::CMD_MERGEARCHON ? GetSelection()/2 :
					actionID == BWrepGameData::CMD_MERGEDARKARCHON ? GetSelection()/2 : 1;
				if(unitsToAdd>0) evt.UpdateResourceTrain(objectID,unitsToAdd);
				else _Discard(&evt);

				// add each unit of the selection
				for(int k=0;k<unitsToAdd;k++)
				{
					AddUnit(objectID,evt.Time());

					// double units for zerlings and scourges
					if(objectID==BWrepGameData::OBJ_ZERGLING || objectID==BWrepGameData::OBJ_SCOURGE) 
						AddUnit(objectID,evt.Time());
				}

				// clear selection to avoid double hatch
				if(actionID == BWrepGameData::CMD_HATCH) m_currentSelection=0;
			}
		}
		else if(buildingid>=0)
		{
			// is build event valid?
			if(_IsValidBuildEvent(&evt, prevEvt, objectID))
			{
				// update resources and distribution
				evt.UpdateResourceBuild(objectID);
				_AddBuilding(objectID,bx,by,evt.Time());
			}
		}
		else if(actionID == BWrepGameData::CMD_UPGRADE || actionID == BWrepGameData::CMD_RESEARCH)  
		{
			// is upgrade event valid?
			int techID = ReplayResource::_FindTech(parameters==0?action->GetParameters(GetElemList()):parameters);
			if(techID>=0 && _IsValidUpgradeEvent(&evt, prevEvt, techID))
			{
				// update resources and distribution
				evt.UpdateResourceUpgrade(techID);
				AddUpgrade(techID);
			}
			evt.SetUnitIdx(techID);
		}
	}

	// add action in resources and update micro/macro 
	_Complete(&evt, m_currentSelection);
	
	// update action type distribution
	AddEvtType(evt.Type().GetIdxAction(), eventValidForAPM);

	// add event in list
	unsigned long offevt = m_events.Add(&evt,sizeof(evt));
	action->SetUserData(0,(unsigned long)this);
	action->SetUserData(1,offevt/sizeof(ReplayEvt));

	// was action discarded?
	if(!evt.IsDiscarded()) 
	{
		// update build order
		if((actionID == BWrepGameData::CMD_BUILD  || actionID == BWrepGameData::CMD_MORPH)) 
			objectID = buildingid;
		if(objectID>=0) 
		{
			m_bo.AddBuildOrder(actionID, action->GetTime(), objectID);
			// since player built something, we consider he is not an observer
			if(buildingid>=0) m_bEnabled = true;
		}
	}

	// new slot?
	ProcessMapCoverage(action->GetTime());
										
	// return idx of event
	return GetEventCount()-1;
}

//---------------------------------------------------------------------------------------

// compute standard deviation for APM
int ReplayEvtList::GetStandardAPMDev(int delta, int deltaMap)
{
	// already have apm?
	if(m_apmDev!=0 && delta==-1) return m_apmDev;

	// need regular apm?
	if(delta == -1) delta = gTimeWindow[Replay::APM_MEDIUM];

	unsigned long totdev=0;
	int apm = GetActionPerMinute();

	// reset activity measurement maximums
	m_resmax.ClearAPM();
	m_resmax.SetMovingMapCoverage(0);

	// compute max for local apm (after the 2 minute limit)
	int levalApmLocalMax=0;
	int slot;
	for(slot=0;slot<GetSlotCount();slot++)
	{
		ReplayResource *res = &m_resources[slot];

		// compute local actions per minute
		int totaction = 0;
		unsigned long tottime = 0;
		for(int j=min(GetSlotCount()-1,slot+delta); j>=0 && j>=slot-delta; j--)
		{
			ReplayResource *prevres = &m_resources[j];
			totaction += prevres->GetActionCount();
			tottime += RES_INTERVAL_TICK;
		}
		int apmlocal = tottime==0 ? 0 : m_replay->QueryFile()->QueryHeader()->Sec2Tick(60*totaction)/tottime;

		// update max
		unsigned long tick = Slot2Time(slot);
		if(tick>MINAPMVALIDTIME && apmlocal>levalApmLocalMax) 
			levalApmLocalMax = apmlocal;
	}

	// compute local apms and apm dev
	m_apmMini=apm;
	int marginsForMin = 60;
	unsigned long maxTickForMini = Slot2Time(GetSlotCount()-marginsForMin);
	unsigned long minTickForMini = Slot2Time(marginsForMin);
	for(slot=0;slot<GetSlotCount();slot++)
	{
		ReplayResource *res = &m_resources[slot];

		// compute local actions per minute on a window of slots
		int totaction = 0;
		int totactionBuild = 0;
		int totactionTrain = 0;
		int totactionMicro = 0;
		int totactionMacro = 0;
		unsigned long tottime = 0;
		for(int j=min(GetSlotCount()-1,slot+delta); j>=0 && j>=slot-delta; j--)
		{
			ReplayResource *prevres = &m_resources[j];
			totaction += prevres->GetActionCount();
			totactionBuild += prevres->GetActionCount(ReplayResource::A_BUILD);
			totactionTrain += prevres->GetActionCount(ReplayResource::A_TRAIN);
			totactionMicro += prevres->GetActionCount(ReplayResource::A_MICRO);
			totactionMacro += prevres->GetActionCount(ReplayResource::A_MACRO);
			tottime += RES_INTERVAL_TICK;
		}

		const IStarcraftGame *game = m_replay->QueryFile()->QueryHeader();
		int apmlocal = tottime==0 ? 0 : game->Sec2Tick(60*totaction)/tottime;
		int bpmlocal = tottime==0 ? 0 : game->Sec2Tick(60*totactionBuild)/tottime;
		int upmlocal = tottime==0 ? 0 : game->Sec2Tick(60*totactionTrain)/tottime;
		int apmmicro = tottime==0 ? 0 : game->Sec2Tick(60*totactionMicro)/tottime;
		int apmmacro = tottime==0 ? 0 : game->Sec2Tick(60*totactionMacro)/tottime;

		// udpate minimum apm
		unsigned long tick = Slot2Time(slot);
		if(apmlocal<m_apmMini && tick>=minTickForMini && tick<maxTickForMini) m_apmMini=apmlocal;

		// discard any local apm that is above max 
		if(apmlocal>levalApmLocalMax) apmlocal=levalApmLocalMax;
		if(apmmicro>levalApmLocalMax) apmmicro=levalApmLocalMax;
		if(apmmacro>levalApmLocalMax) apmmacro=levalApmLocalMax;

		// store
		res->SetAPM(apmlocal);
		res->SetMicroAPM(apmmicro);
		res->SetMacroAPM(apmmacro);
		res->SetLegalAPM(apmlocal);
		res->SetBPM(bpmlocal);
		res->SetUPM(upmlocal);

		// compute local map coverage for units on a window of slots
		unsigned __int64 mapcover = 0;
		for(int j=min(GetSlotCount()-1,slot+deltaMap); j>=0 && j>=slot-deltaMap; j--)
		{
			ReplayResource *prevres = &m_resources[j];
			mapcover |= prevres->MapCoverageUnit();
		}

		// count squares
		int squareCount=0;
		for(int j=0;j<64;j++)
		{
			if((mapcover&1)!=0) squareCount++;
			mapcover>>=1;
		}
		res->SetMovingMapCoverage(squareCount);

		// update all maxes for resources
		m_resmax.UpdateMax(*res,(tick>=MINAPMVALIDTIMEFORMAX));

		// compute local deviation
		int dev=apmlocal - apm;
		totdev+=abs(dev);

		// make sure all slots are initialized
		if(!res->IsInitDone()) 
			if(slot>0)
				res->Clone(m_resources[slot-1]);
	}

	// compute final deviation
	m_apmDev = GetSlotCount()==0?0:totdev/GetSlotCount();
	return m_apmDev;
}

//---------------------------------------------------------------------------------------

// cursor = value between 0 and full span time
// will find the nearest event 
unsigned long ReplayEvtList::GetEventFromTime(unsigned long cursor)
{
	unsigned long eventCount = GetEventCount();
	if(eventCount==0) return 0;

	unsigned long nSlot = 0;
	unsigned long low = 0;
	unsigned long high = eventCount - 1;
	unsigned long beginTimeTS = 0;
	unsigned long i;
	const ReplayEvt *event;

	// for all events in the list
	while(true)
	{
		i= (high+low)/2;
		ASSERT(high>=low);

		// are we beyond the arrays boundaries?
		if(i>=eventCount) {nSlot=0;break;}

		// get event
		event = GetEvent(i);
		ASSERT_POINTER(event ,ReplayEvt);

		// compare times		
		LONGLONG delta = event->Time()-beginTimeTS;
		LONGLONG nCmp = (LONGLONG )cursor - delta;

		// if event time is the same, return index
		if(nCmp==0) 
		{
			nSlot = i; 
			goto Exit;
		}
		else if(nCmp<0) 
		{
			if(high==low) {nSlot = low; break;}
			high = i-1;
			if(high<low) {nSlot=low; break;}
			if(high<0) {nSlot=0; break;}
		}
		else
		{
			if(high==low) {nSlot = low+1; break;}
			low = i+1;
			if(low>high) {nSlot=high; break;}
			if(low>=eventCount-1) {nSlot=eventCount-1; break;}
		}
	}
	
	ASSERT(nSlot<eventCount);

Exit:
	return nSlot;
}

//------------------------------------------------------------------------------------------------------------

ReplayEvtList *Replay::_GetListFromPlayerName(const char *playername, int race)
{
	// find existing list for that player (if any)
	ReplayEvtList *list=0;
	for(int i=0; i<GetPlayerCount(); i++)
	{
		list = GetEvtList(i);
		if(strcmp(list->PlayerName(),playername)==0) break;
		else list=0;
	}

	// if no existing list for that player
	if(list==0)
	{
		// add new one
		list = new ReplayEvtList(this, m_mapAnim, playername,GetPlayerCount(),race);
		m_players.Add(list);
	}

	return list;
}

//------------------------------------------------------------------------------------------------------------

unsigned long Replay::_AddEvent(IStarcraftAction *action, const char *playername, int race, const char* &parameters)
{
	unsigned long offevt=0;
	m_currentlist = 0;

	// record time for last action
	if(action->GetTime()>m_timeEnd) m_timeEnd=action->GetTime();

	// find existing list for that player (if any)
	m_currentlist = _GetListFromPlayerName(playername,race);

	// if we have a list
	if(m_currentlist) 
	{
		// add event in list
		offevt=m_currentlist->AddEvent(action,parameters);
	}

	return offevt;
}

//------------------------------------------------------------------------------------------------------------

// get unit count
int ReplayEvtList::GetMaxUnit() const {return MAXUNIT;}

// get upgrade count
int ReplayEvtList::GetMaxUpgrade() const {return MAXTECHNIC();}

// get building count
int ReplayEvtList::GetMaxBuilding() const {return MAXUNIT;}

//------------------------------------------------------------------------------------------------------------

COLORREF ReplayEvtList::GetUnitColor(int idx) const	
{
	assert(idx>=0 && idx<BWrepGameData::g_ObjectsSize);
	return gAllUnits[idx].clr;
}
COLORREF ReplayEvtList::GetBuildingColor(int idx) const	
{
	assert(idx>=0 && idx<BWrepGameData::g_ObjectsSize);
	return gAllUnits[idx].clr;
}
COLORREF ReplayEvtList::GetUpgradeColor(int idx) const	{return BWrepGameData::GetBarColor(idx, MAXTECHNIC());}

//------------------------------------------------------------------------------------------------------------

void Replay::_Sort()
{
	int avgAction=0;
	for(;m_players.GetSize()>1;)
	{
		bool swap=false;
		for(int i=0; i<m_players.GetSize()-1; i++)
		{
			ReplayEvtList *list1 = GetEvtList(i);
			ReplayEvtList *list2 = GetEvtList(i+1);
			if(list2->GetEventCount()>list1->GetEventCount())
			{
				//swap
				swap=true;
				m_players.SetAt(i,list2);
				m_players.SetAt(i+1,list1);
			}
		}
		if(!swap) break;
	}
	m_Done = true;

	// count average action	from 2 first players
	int count=0;
	for(int i=0; i<min(2,m_players.GetSize()); i++)
	{
		ReplayEvtList *list = GetEvtList(i);
		avgAction+=list->GetEventCount();
		count++;
	}
	if(count>0) avgAction/=count;

	// buid mapping table from playerid to idx
	delete[]m_listref; m_listref=0;
	m_listref = new int[m_players.GetSize()];
	int enabled=0;
	for(int i=0; i<m_players.GetSize(); i++)
	{
		ReplayEvtList *list = GetEvtList(i);
		m_listref[list->GetPlayerID()]=i;
		if(list->IsEnabled()) enabled++;
	}

	// zero or one player only??
	if(enabled<2)
	{
		if(m_players.GetSize()>0) GetEvtList(0)->EnablePlayer(true);
		if(m_players.GetSize()>1) GetEvtList(1)->EnablePlayer(true);
	}

	//ReplayEvt::DebugDisplayTypes();
	//DebugDisplayList();
}

//------------------------------------------------------------------------------------------------------------

// returns true if we have event for a player 
bool Replay::_HaveEventsForPlayer(const char *name, const CStringArray& existingPlayers) const
{
	for(int i=0; i<existingPlayers.GetSize(); i++)
	{
		if(_stricmp(name,existingPlayers[i])==0)
			return true;
	}
	return false;
}

//------------------------------------------------------------------------------------------------------------

void Replay::_GetUniquePlayerName(CString& playerName, const CStringArray& existingPlayers)
{
	CString tmp(playerName);
	int counter=1;
	while(_HaveEventsForPlayer(tmp,existingPlayers))
	{
		tmp.Format("%s.%d",(const char*)playerName,counter);
		counter++;
	}
	playerName=tmp;
}

//------------------------------------------------------------------------------------------------------------

void Replay::_ClearMaps()
{
	delete m_mapAnim;
	m_mapAnim=0;
}

//------------------------------------------------------------------------------------------------------------

void Replay::_CreateTileset()
{
	// clear all maps
	_ClearMaps();

	// allocate animation map
	m_mapAnim = new	ReplayMapAnimated(this, m_gfile->QueryHeader()->getMapWidth(),m_gfile->QueryHeader()->getMapHeight());
	MapSurface *tileset = m_mapAnim->GetTileSet();

	// get tile section info
	const IStarcraftMapSection *tile= m_gfile->QueryMap()->GetSection(SECTION_MTXM);
	if(tile==0) tile= m_gfile->QueryMap()->GetTileSection();

	// make sure size is ok
	int mapsize = m_gfile->QueryMap()->GetWidth()*m_gfile->QueryMap()->GetHeight();
	if(tile!=0 && (tile->GetSize()==(unsigned long)(2*mapsize) || tile->GetSize()==(unsigned long)mapsize))
	{
		int inc = tile->GetSize()/(unsigned long)mapsize;
		const unsigned char *psquare = tile->GetData();
		for(int j=0;j<m_gfile->QueryMap()->GetHeight();j++)
			for(int i=0;i<m_gfile->QueryMap()->GetWidth();i++)
			{
				tileset->SetSquare(i,j,MapElem(psquare[0],psquare[1]));
				psquare+=inc;
			}
	}
	else
	{
		assert(0);
	}

	// get unit section info
	const BWrepMapSectionUNIT *units= (const BWrepMapSectionUNIT *)m_gfile->QueryMap()->GetSection(SECTION_UNIT);
	if(units==0) return;
	for(int i=0;i<units->GetUnitCount();i++)
	{
		// add start locations
		BWrepMapSectionUNIT::BWrepUnitDesc *desc = units->GetUnitDesc(i);
		if(desc->unitid==BWrepMapSectionUNIT::UNIT_STARTLOCATION)
		{
			const IStarcraftPlayer *player;
			m_gfile->QueryHeader()->getPlayerFromIdx(player,desc->playerid);
			if(player->getName()!=0 && player->getName()[0]!=0)
			{
				// update start location
				ReplayEvtList *list = _GetListFromPlayerName(player->getName(),player->getRace());
				list->SetStartingLocation(desc->x/32,desc->y/32);

				// add building at start location (nexus, cc, hatchery are all 4x3)
				int x= desc->x/32;
				int y= desc->y/32;
				m_mapAnim->AddBuild(&ReplayMapAction(x,y,4,3,list->GetPlayerID(),0));
			}
		}
		else if(desc->unitid==BWrepMapSectionUNIT::UNIT_MINERAL1 || 
			desc->unitid==BWrepMapSectionUNIT::UNIT_MINERAL2 || 
			desc->unitid==BWrepMapSectionUNIT::UNIT_MINERAL3 || 
			desc->unitid==BWrepMapSectionUNIT::UNIT_GEYSER)
		{
			int wx=2, wy=1;
			if(desc->unitid==BWrepMapSectionUNIT::UNIT_GEYSER) {wx*=2;wy*=2;}
			int x= desc->x/32;
			int y= desc->y/32;
			m_mapAnim->AddBuild(&ReplayMapAction(x,y,wx,wy,MAPMINERAL,0));
		}
	}
}

//------------------------------------------------------------------------------------------------------------

// return location as clock value
int ReplayEvtList::GetStartingLocation() const 
{
	int w = m_replay->QueryFile()->QueryMap()->GetWidth();
	int h = m_replay->QueryFile()->QueryMap()->GetHeight();
	double division=12.0;
	double dx = m_startX - w/2;
	double dy = h/2 - m_startY;
	double r = sqrt((double)(dx*dx+dy*dy));
	double alpha = acos(dx/r);
	double pi=3.14159265358979;
	if(dy<0) alpha=-alpha;
	if(dx<0 && dy>0) alpha = (5.0*pi)/2.0-alpha;
	else alpha = pi/2.0-alpha;
	alpha+=pi/division;
	int loc=(int)(division*(alpha/(2.0*pi)));
	if(loc==0) loc=(int)division;

	// special case for Lost Temple
	CString mapname(m_replay->MapName());
	mapname.MakeLower();
	if(strstr(mapname,"temple")!=0)
	{
		// use the common positions 12,9,6,3
		if(loc==2||loc==4) loc=3;
		else if(loc==7||loc==5) loc=6;
		else if(loc==10||loc==8) loc=9;
	}

	return loc;
}

//------------------------------------------------------------------------------------------------------------

// listv==0 means we are only loading for replay browser, no graphics
//
int Replay::Load(const char *filename, bool buildEnActionList, CListCtrl *listv, bool bClear)
{
	DWORD tinter = 0;
	DWORD tload = 0;
	DWORD tstart = 0;
	int ferr=0,i=0;
	CStringArray existingPlayers;
	CStringArray names;
	CStringArray replacements;
	const IStarcraftPlayer *player;
	CString playerName;
	const char *parameters=0;

	// alloc replay
	if(bClear || m_gfile==0)
	{
		if(m_gfile!=0) m_gfile->Release();
		m_gfile = QueryFactory()->CreateReplayInstance(filename);
		if(m_gfile==0) return -1;
	}

	// save name in log file i ncase we crash
	AfxGetApp()->WriteProfileString("LOG","LASTREP",filename);

	// init units arrays
	BWrepGameData::InitUnits();

	// reset
	if(bClear) Clear();

	tstart = GetTickCount();

	// load file
	int prevCount = bClear ? 0 : m_gfile->QueryActions()->GetActionCount();
	char *oldbase = bClear ? 0 : (char*)m_gfile->QueryActions()->GetAction(0);
	int options = IStarcraftReplay::LOADACTIONS | IStarcraftReplay::LOADMAP;
	if(!bClear) options |= IStarcraftReplay::ADDACTIONS;
	if(!m_gfile->Load(filename,options,&m_hdrRWA,sizeof(AudioHeader))) {ferr=-1; goto Exit;}
	m_filename = filename;

	// any RWA data?
	m_isRWA=false;
	if(strncmp(m_hdrRWA.header,BWAUDIOMARKER,strlen(BWAUDIOMARKER))==0) 
	{
		// decrypt header
		DecryptHeader(&m_hdrRWA);
		m_isRWA=true;
	}

	tload = GetTickCount()-tstart;

	// fill list of existing player names
	for(i=0; i<m_players.GetSize(); i++)
	{
		const ReplayEvtList *list = GetEvtList(i);
		existingPlayers.Add(list->PlayerName());
	}

	// create map & tileset
	if(bClear) 
		_CreateTileset();
	// no map when multiple replays are mixed
	else
		_ClearMaps();

	// process actions
	for(i=prevCount; i<m_gfile->QueryActions()->GetActionCount(); i++)
	{
		// get action
		const IStarcraftAction *action = m_gfile->QueryActions()->GetAction(i);

		// get player
		m_gfile->QueryHeader()->getPlayerFromAction(player,action->GetPlayerID());
		playerName = player->getName();

		//do we already have data for that player?
		if(!bClear && _HaveEventsForPlayer(playerName,existingPlayers)) 
		{
			// do we already have a replacement
			bool found=false;
			for(int ir=0; ir<names.GetSize();ir++)
			{
				if(names[ir]==playerName) {found=true; playerName=replacements[ir]; break;}
			}
			// if we dont have a replacement yet
			if(!found)
			{
				// create one
				_GetUniquePlayerName(playerName,existingPlayers);
				names.Add(player->getName());
				replacements.Add(playerName);
			}
		}

		// record event
		parameters=0;
		_AddEvent((IStarcraftAction *)action,playerName,player->getRace(),parameters);

		// insert dummy event (just for incrementing the number of elements in the virtual list control)
		//if(listv!=0) 
		//	listv->InsertItem(i,"", 0);
	}

	tinter = GetTickCount()-tstart;

	// if we're going to display charts
	if(listv!=0) 
	{
		// compute overall action distribution (for chart)
		_ComputeActionDistribution();

		// adjust previous pointers to actions
		char *newbase = (char*)m_gfile->QueryActions()->GetAction(0);
		for(int i=0; !bClear && i<existingPlayers.GetSize();i++)
		{
			ReplayEvtList *list = GetEvtList(i);
			list->AdjustActionPointers(oldbase,newbase);
		}

		// compute standard deviation for APM & local activity measurements
		m_apmStyle = APM_MEDIUM;
		for(int i=existingPlayers.GetSize(); i<GetPlayerCount();i++)
		{
			ReplayEvtList *list = GetEvtList(i);

			// process last slots for map coverage
			list->ProcessMapCoverage(m_timeEnd);

			// update resources
			list->GetStandardAPMDev(gTimeWindow[m_apmStyle], gTimeWindowMap[m_mapStyle]);

			// update all maxes for resources
			m_resmax.UpdateMax(list->ResourceMax(),true);
		}
	}

	// if there wasnt any action at all
	if(GetPlayerCount()==0)
	{
		// read player list from replay header
		for(int i=0;i<m_gfile->QueryHeader()->getLogicalPlayerCount();i++)
		{
			// build corresponding event lists with no events
			const IStarcraftPlayer *player;
			m_gfile->QueryHeader()->getLogicalPlayers(player,i);
			ReplayEvtList *list = new ReplayEvtList(this, m_mapAnim, player->getName(),i,player->getRace());
			m_players.Add(list);
		}
	}

	// sort	player's list
	_Sort();

	// if we're going to display charts
	if(buildEnActionList)
	{
		// rebuild enabled action list
		_BuildEnableActionList();
		if(listv!=0) listv->SetItemCountEx(GetEnActionCount(), LVSICF_NOSCROLL|LVSICF_NOINVALIDATEALL);
	
	}

	// mark suspicious events
	_MarkSuspiciousEvents();

	// mark events that are HACK signatures
	_MarkHackCommands();
	
Exit:
	AfxGetApp()->WriteProfileString("LOG","LASTREP","");
	return ferr;
}

//------------------------------------------------------------------------------------------------------------

// reprocess apm for all players
bool Replay::UpdateAPM(int apmStyle, int mapStyle)
{
	// same as previous style?
	if(m_apmStyle == apmStyle && m_mapStyle==mapStyle) return false;

	// new style
	m_apmStyle = apmStyle;
	m_mapStyle = mapStyle;

	// reset activity measurement maximums
	m_resmax.ClearAPM();
	m_resmax.SetMovingMapCoverage(0);

	// compute standard deviation for APM & local activity measurements
	for(int i=0; i<GetPlayerCount();i++)
	{
		ReplayEvtList *list = GetEvtList(i);

		// update resources
		list->GetStandardAPMDev(gTimeWindow[m_apmStyle], gTimeWindowMap[m_mapStyle]);

		// update all maxes for resources
		m_resmax.UpdateMax(list->ResourceMax(),true);
	}

	return true;
}

//------------------------------------------------------------------------------------------------------------

// remove events from a particular player
void Replay::RemovePlayer(ReplayEvtList *plist, class CListCtrl *listv)
{					  
	CWaitCursor wait;

	// remove list of events
	int pcount = GetPlayerCount();
	for(int i=0,j=0;i<pcount; i++) 
	{
		ReplayEvtList *list = GetEvtList(i);
		if(list==plist)	
		{
			m_players.RemoveAt(i);
			m_deletedPlayers.Add(list);
			break;
		}
	}
}

//------------------------------------------------------------------------------------------------------------

const char *ReplayEvtList::GetRaceStr() const 
{
	return m_race==IStarcraftPlayer::RACE_TERRAN?"T":
	m_race==IStarcraftPlayer::RACE_ZERG?"Z":
	m_race==IStarcraftPlayer::RACE_PROTOSS?"P":
	"U";
}

//------------------------------------------------------------------------------------------------------------

void Replay::Clear() 
{
	m_Done=false; 
	m_players.RemoveAll(); 
	m_resmax.Clear(); 
	m_timeEnd=0;
	m_lastBOTime=0;
}

//------------------------------------------------------------------------------------------------------------

const char *Replay::MapName() const
{
	return m_gfile->QueryHeader()->getMapName();
}

//------------------------------------------------------------------------------------------------------------

void ReplayObjectSequence::AddObject(unsigned long time, unsigned int objid)
{
	if(m_count>=m_maxCount) return;
	assert(m_count==0 || time>=m_time[m_count-1]);
	m_time[m_count]=time;
	m_object[m_count]=objid;
	m_count++;
}

//------------------------------------------------------------------------------------------------------------

void ReplayObjectSequence::UpdateTime(int idx, unsigned long time)
{
	if(idx>=m_count) return;
	m_time[idx]=time;
}

//------------------------------------------------------------------------------------------------------------

bool ReplayObjectSequence::InsertObject(unsigned long time, unsigned int objid)
{
	// search insertion place
	int i=0;
	for(;i<m_count;i++)
	{
		// if found
		if(time < m_time[i])
		{
			// move everything after it
			for(int j=min(m_count,m_maxCount-1); j>=i+1;j--)
			{
				m_time[j] = m_time[j-1];
				m_object[j] = m_object[j-1];
			}
			break;
		}
	}

	// if we didnt find a place to insert, and the sequence if full
	if(i>=m_maxCount) return false;

	// store new object
	m_time[i]=time;
	m_object[i]=objid;

	// increment the object count 
	if(m_count+1<m_maxCount)
		m_count++;
	return true;
}

//------------------------------------------------------------------------------------------------------------

// remove object (building or unit) from build order
void ReplayObjectSequence::RemoveObject(unsigned long time, unsigned int objid)
{
	for(int i=0;i<m_count;i++)
	{
		if(m_time[i]==time && m_object[i]==objid)
		{
			for(int j=i; j<m_count-1;j++)
			{
				m_time[j] = m_time[j+1];
				m_object[j] = m_object[j+1];
			}
			m_count--;
			break;
		}
	}
}

//------------------------------------------------------------------------------------------------------------

// what is the count for a particular element at index idx
int ReplayEvtList::GetDistCount(int type, int idx) 
{
	if(type==DIST_UNIT) return gAllUnits[idx].IsBuilding() ? 0 : m_objects[idx];
	if(type==DIST_ACTION) return m_type[idx];
	if(type==DIST_UPGRADE) return m_upgrades[idx];
	if(type==DIST_BUILDING) return gAllUnits[idx].IsBuilding() ? m_objects[idx] : 0;
	if(type==DIST_ACTIONFORAPM) 
	{
		//return m_typeAPM[idx];
		return GetActionPerMinute(false,m_typeAPM[idx]);
	}
	return 0;
}

//------------------------------------------------------------------------------------------------------------

// return bo as a string (one char per object, nmax objects maximum)
const char *ReplayObjectSequence::GetAsString(CString& str, int nmax) const
{
	/*
	char buffer[MAXBO*2+1];
	buffer[0]=0;
	for(int i=0;i<max(nmax,m_count);i++)
	{
		sprintf(&buffer[strlen(buffer)]
	}
	*/
	return 0;
}

//------------------------------------------------------------------------------------------------------------

const char *ReplayEvtType::GetTypeFromIdx(int idx)
{
	if(idx<BWrepGameData::_CMD_MAX_)
		return BWrepGameData::GetActionNameFromID(idx);
	return BWrepGameData::GetAttackNameFromID(idx-BWrepGameData::_CMD_MAX_);
}

//------------------------------------------------------------------------------------------------------------

const char *ReplayEvtType::GetType() const
{
	return GetTypeFromIdx(IdxAction(m_cmd,m_subcmd));
}

//------------------------------------------------------------------------------------------------------------

COLORREF ReplayEvtType::GetTypeColor(int idx)
{
	assert(idx>=0 && idx<MAXACTIONTYPE);
	return gAllActions[idx].clr;
}

//------------------------------------------------------------------------------------------------------------

void Replay::_ComputeActionDistribution()
{
	m_overalActionCount=0;
	memset(gOverallActions,0,sizeof(gOverallActions));

	for(int i=0; i<GetPlayerCount(); i++)
	{
		ReplayEvtList *list = GetEvtList(i);
		int maxaction = list->GetDistMax(ReplayEvtList::DIST_ACTION);
		for(int j=0; j<maxaction; j++)
		{
			int count = list->GetDistCount(ReplayEvtList::DIST_ACTION,j);
			if(count>0) gAllActions[j].m_overallIdx = 1;
		}
	}

	for(int j=0; j<MAXACTIONTYPE; j++)
	{
		if(gAllActions[j].m_overallIdx>0)
		{
			gAllActions[j].m_overallIdx = m_overalActionCount;
			gOverallActions[m_overalActionCount].m_overallIdx = j;
			m_overalActionCount++;
		}
	}
}

//------------------------------------------------------------------------------------------------------------

const char *Replay::GetTypeStr(int idx) const 
{
	assert(gOverallActions[idx].m_overallIdx<MAXACTIONTYPE);
	return ReplayEvtType::GetTypeFromIdx(gOverallActions[idx].m_overallIdx);
}

//------------------------------------------------------------------------------------------------------------

int Replay::GetTypeIdx(const ReplayEvt *evt) const 
{
	assert(gAllActions[evt->Type().GetIdxAction()].m_overallIdx<m_overalActionCount);
	return gAllActions[evt->Type().GetIdxAction()].m_overallIdx;
}

//------------------------------------------------------------------------------------------------------------

// adjust all actions addresses
void ReplayEvtList::AdjustActionPointers(char *oldbase, char *newbase)
{
	for(int i=0;i<GetEventCount();i++)
	{
		ReplayEvt *evt = GetEvent(i);
		evt->SetAction((const BWrepAction*)(newbase+((char*)evt->GetAction()-oldbase)));
	}
}

//------------------------------------------------------------------------------------------------------------

// actions per minutes
int ReplayEvtList::GetActionPerMinute(bool bValidOnly, int eventCount) const 
{
	bool removeBeginEvents = eventCount==-1;

	// total event count
	if(eventCount==-1) eventCount = GetEventCount();
	if(eventCount==0) return 0;

	// if time frame is long enough
	float timeFrame = (float)GetEvent(GetEventCount()-1)->Time();
	if(timeFrame > MINAPMVALIDTIME)
	{
		// remove beginning events
		timeFrame -= MINAPMVALIDTIME;
		if(removeBeginEvents) eventCount-=m_eventsBegin;
	}

	// apm
	if(bValidOnly) eventCount -= GetDiscardedActions();
	if(eventCount<0) eventCount = 0;
	return (int)((float)m_replay->QueryFile()->QueryHeader()->Sec2Tick(60)*(float)eventCount/timeFrame);
}

//------------------------------------------------------------------------------------------------------------

// average micro APM for whole game duration
int ReplayEvtList::GetMicroAPM() const 
{

	int total=0,vslot=0;
	for(int slot=0;slot<GetSlotCount();slot++)
	{
		ReplayResource *res = &m_resources[slot];
		if(slot*RES_INTERVAL_TICK > MINAPMVALIDTIME)
		{
			total += res->MicroAPM(); 
			vslot++;
		}
	}
   return vslot==0 ? 0 : total / vslot;
}

//------------------------------------------------------------------------------------------------------------

// average macro APM for whole game duration
int ReplayEvtList::GetMacroAPM() const 
{

	int total=0,vslot=0;
	for(int slot=0;slot<GetSlotCount();slot++)
	{
		ReplayResource *res = &m_resources[slot];
		if(slot*RES_INTERVAL_TICK > MINAPMVALIDTIME)
		{
			total += res->MacroAPM(); 
			vslot++;
		}
	}
   return vslot==0 ? 0 : total / vslot;
}

//------------------------------------------------------------------------------------------------------------

// return max APM for whole replay but ignoring players that are disabled
void Replay::_UpdateMaxAPMForEnabledPlayers()
{
	int maxAPM=0;
	for(int i=0; i<GetPlayerCount(); i++)
	{
		const ReplayEvtList *list = GetEvtList(i);
		if(list->IsEnabled() && list->ResourceMax().APM()>maxAPM)
			maxAPM = list->ResourceMax().APM();
	}

	m_resmax.SetAPM(maxAPM);
}

//------------------------------------------------------------------------------------------------------------

int BWElementList::compare( const void *arg1, const void *arg2 )
{
   BWElement *elem1 = (BWElement *)arg1;
   BWElement *elem2 = (BWElement *)arg2;
   return elem1->UnitID() - elem2->UnitID();
}

BWElement * BWElementList::AddElement(class ReplayEvtList *evtlist, short unitID, unsigned long timeFirstSeen, bool *isnew)
{
	//assert(unitID!=3237);

	// if element already exist, dont add it again
	BWElement *pelem = FindElement(unitID);
	if(pelem!=0) {if(isnew) *isnew=false; return pelem;}

	// add element and sort array
	BWElement elem(unitID,timeFirstSeen,evtlist);
	m_elements.Add(&elem,sizeof(elem));
	qsort(m_elements.GetPtr(0),m_elements.GetCount(),sizeof(elem),compare);

	/*
	for(int i=0;i<(int)m_elements.GetCount();i++)
	{
		char buf[200];
		BWElement * elem = (BWElement *)m_elements.GetPtr(i*sizeof(BWElement));
		sprintf(buf,"%d=%d\r\n",elem->UnitID(),elem->ObjectID());
		OutputDebugString(buf);
	}
	OutputDebugString("___________\r\n");
	*/

	// element was added
	if(isnew) *isnew=true; 
	return FindElement(unitID);
}

void BWElementList::SetObjectID(class ReplayEvtList *evtlist, short unitID, short objectID, unsigned long time, unsigned long realtime)
{							
	// add element of get existing one
	BWElement *elem = AddElement(evtlist,unitID,realtime);
	// reset element?
	bool reset = (evtlist->GetRaceIdx()==IStarcraftPlayer::RACE_TERRAN);
	// add new identity
	if(elem!=0) elem->SetObjectID(objectID,time,realtime,reset);
}

short BWElementList::GetObjectID(short unitID, unsigned long time) const
{
	BWElement *elem = FindElement(unitID);
	return (elem==0) ? -1 : elem->ObjectID(time);
}

// IUnitIDToObjectID implementation
short BWElementList::Convert(short unitID, unsigned long time) const
{
	short objid = GetObjectID(unitID,time);

	// return object id from any unit id
	if(objid==-1) m_replay->GetAnyObjectID(unitID, time, &objid);
	return objid;
}

BWElement *BWElementList::FindElement(short unitID) const
{
	BWElement elem(unitID,0,0);
	return (BWElement *)bsearch(&elem, m_elements.GetPtr(0), m_elements.GetCount(), sizeof(elem), compare);
}

//------------------------------------------------------------------------------------------------------------

void BWElement::SetObjectID(short objectID,unsigned long time,unsigned long realtime, bool reset)
{

	if(m_unitID==3608)
		objectID=objectID;

	// reset content?
	if(reset) m_count=0;

	// do we already have that objectID?
	for(int i=0; i<m_count;i++)
		if(m_objectID[i]==objectID)
		{
			// we cancel all objects that we thought we had beyond that same object
			// ex: larva -> drone -> larva!!  Drone cant become larva again, so it means we were wrong on 2nd step
			//if(m_count==MAXOBJID) return; ///???????????????????????????
			m_count=i+1;

			for(int j=m_count;j<MAXOBJID;j++) {m_objectID[j]=-1; m_time[j]=0;}
			if(m_time[i]==0 || time<m_time[i]) m_time[i]=time;
			return;
		}

	//if(m_count>=MAXOBJID) return; //???????????????????????????

	// no we dont, add it
	assert(m_count<MAXOBJID);
	m_objectID[m_count]=objectID;
	m_time[m_count]=time;
	if(m_count>0 && m_time[m_count-1]==MAXTIME) m_time[m_count-1]=realtime-1;

	m_count++;
}

short BWElement::ObjectID(unsigned long time, unsigned long* timeIdentification) const
{
	// no object id?
	if(m_count==0) return -1;

	// search list of object id
	for(int i=0;i<m_count;i++)
	{
		// return the one for that time
		if(time<=m_time[i])
		{
			if(timeIdentification) *timeIdentification = m_time[i];
			return m_objectID[i]; 
		}
	}

	// return lastest object id
	if(timeIdentification) *timeIdentification = m_time[m_count-1];
	return m_objectID[m_count-1];
}

//------------------------------------------------------------------------------------------------------------

// rebuild enabled action list
void Replay::_BuildEnableActionList()
{
	// clear existing list
	m_enabledActions.Clear();

	// for each action
	int count = QueryFile()->QueryActions()->GetActionCount();
	m_suspectCount=0;
	m_hackCount=0;
	for(int i=0; i<count; i++)
	{
		// skip if player is disabled
		const IStarcraftAction *action = QueryFile()->QueryActions()->GetAction(i);
		ReplayEvtList *list = (ReplayEvtList *)action->GetUserData(0);
		if(!list->IsEnabled()) continue;

		// get event
		ReplayEvt *evt = list->GetEvent(action->GetUserData(1));
		assert(evt!=0);

		// count suspect events
		if(evt->IsSuspect()) m_suspectCount++;

		// count hack events
		if(evt->IsHack()) m_hackCount++;

		// apply filter (if any)
		bool add = (m_filter&FLT_OTHERS)!=0;
		if(action->GetID()==BWrepGameData::CMD_SELECT)
			add = (m_filter&FLT_SELECT)!=0;
		else if(action->GetID()==BWrepGameData::CMD_BUILD || action->GetID()==BWrepGameData::CMD_MORPH)
			add = (m_filter&FLT_BUILD)!=0;
		else if(action->GetID()==BWrepGameData::CMD_TRAIN || action->GetID()==BWrepGameData::CMD_HATCH)
			add = (m_filter&FLT_TRAIN)!=0;
		else if(action->GetID()==BWrepGameData::CMD_MESSAGE)
			add = (m_filter&FLT_CHAT)!=0;
	
		if(!add && (m_filter&FLT_SUSPECT)!=0)
		{
			if(evt->IsSuspect()) add=true;
		}

		if(!add && (m_filter&FLT_HACK)!=0)
		{
			if(evt->IsHack()) add=true;
		}

		// add action
		if(add) m_enabledActions.Add(&action,sizeof(action));
	}
}

//------------------------------------------------------------------------------------------------------------

// get previous player action
const IStarcraftAction *Replay::_GetPreviousPlayerAction(int i, int playerID) const
{
	// search for previous action for same player
	const IStarcraftAction *actPrev = 0;
	for(int j=i-1;j>=0;j--)
	{
		actPrev = QueryFile()->QueryActions()->GetAction(j);
		if(actPrev->GetPlayerID() == playerID)
			break;
		actPrev = 0;
	}

	return actPrev;
}

//------------------------------------------------------------------------------------------------------------

// mark events that are HACK signatures
void Replay::_MarkHackCommands()
{
	m_hackCount = 0;

	// for each action
	int count = QueryFile()->QueryActions()->GetActionCount();
	for(int i=0;i<count;i++)
	{
		// skip if player is disabled
		const IStarcraftAction *act = QueryFile()->QueryActions()->GetAction(i);
		ReplayEvtList *list = (ReplayEvtList *)act->GetUserData(0);
		if(!list->IsEnabled()) continue;

		// check selection hack
		if(act->GetID()==BWrepGameData::CMD_SELECT || act->GetID()==BWrepGameData::CMD_DESELECTAUTO || act->GetID()==BWrepGameData::CMD_SHIFTSELECT)
		{
			const BWrepActionSelect::Params *p = (const BWrepActionSelect::Params *)act->GetParamStruct();
			if(p->m_unitCount > MAXSELECTION) 
			{
				// selection hack
				ReplayEvt *evt = list->GetEvent(act->GetUserData(1));
				assert(evt!=0);
				m_hackCount++;
				evt->SetHack();
			}
			else if(p->m_unitCount>1 && list->GetRaceIdx()!=IStarcraftPlayer::RACE_ZERG)
			{
				// count hopw many buildings in the selection
				int buildingCount=0;
				for(int i=0;i<p->m_unitCount;i++)
				{
					short objid = list->GetObjectID(p->m_unitid[i],act->GetTime());
					if(BWrepGameData::IsBuilding((int)objid)) buildingCount++;
				}

				if(buildingCount>1)
				{
					// InHale selection hack
					ReplayEvt *evt = list->GetEvent(act->GetUserData(1));
					assert(evt!=0);
					m_hackCount++;
					evt->SetHack();
				}
			}
		}
		// PROTOSS MINERAL HACK: mineral goes up 4000, gas +400
		else if(act->GetID()==0x33)
		{
			int size;
			unsigned char *data = (unsigned char *)act->GetParamStruct(&size);
			if(size==12 && data[1]==0x15 &&  data[8]==0xE4 && 
				data[0]==0 && data[2]==0 && data[3]==0 && data[4]==0 && 
				data[5]==0 && data[6]==0 && data[7]==0 && data[9]==0 && data[10]==0 && data[11]==0)
			{
				// Protoss Mineral/Gaz hack
				ReplayEvt *evt = list->GetEvent(act->GetUserData(1));
				assert(evt!=0);
				m_hackCount++;
				evt->SetHack();
			}
		}
		// TERRAN MINERAL HACK : mineral goes up to 1000, CC explodes
		else if(act->GetID()==BWrepGameData::CMD_CANCELTRAIN)
		{
			// try to locate 5 repetitions of a train/cancel train pattern
			int index = i;
			const IStarcraftAction *actCurrent = act;
			for(int k=0;k<5;k++)
			{
				// first action must be a cancel train
				if(actCurrent->GetID()!=BWrepGameData::CMD_CANCELTRAIN) break;

				ReplayEvt *evt = list->GetEvent(actCurrent->GetUserData(1));
				if(evt->IsHack()) break;
														  
				// search for previous action for same player
				const IStarcraftAction *actPrev = _GetPreviousPlayerAction(index, act->GetPlayerID());

				// is it a train?
				if(actPrev==0 || actPrev->GetID()!=BWrepGameData::CMD_TRAIN) break;

				// is it for a ComSat?
				int size;
				BWrepActionTrain::Params *data = (BWrepActionTrain::Params *)actPrev->GetParamStruct(&size);
				if(data->m_unitType!=BWrepGameData::OBJ_COMSAT) break;

				if(k==4)
				{
					// Terran Mineral/Gaz hack
					ReplayEvt *evt = list->GetEvent(act->GetUserData(1));
					assert(evt!=0);
					m_hackCount++;
					evt->SetHack();
				}

				// move up
				index-=2;
				if(index<1) break;
				actCurrent = QueryFile()->QueryActions()->GetAction(index);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------

// mark suspicious events
void Replay::_MarkSuspiciousEvents()
{
	// empty all elements list for disabled players
 	for(int i=0; i<GetPlayerCount(); i++)
	{
		ReplayEvtList *list = GetEvtList(i);
		if(!list->IsEnabled()) list->ClearElements();
	}

	// for each action
	int count = QueryFile()->QueryActions()->GetActionCount();
	unsigned long timeLimit = QueryFile()->QueryHeader()->Sec2Tick(gSuspectLimit*60);
	m_suspectCount=0;
	for(int n=0; n<count; n++)
	{
		// skip if player is disabled
		const IStarcraftAction *action = QueryFile()->QueryActions()->GetAction(n);
		ReplayEvtList *list = (ReplayEvtList *)action->GetUserData(0);
		if(!list->IsEnabled()) continue;

		// is it a select?
		if(action->GetID()!=BWrepGameData::CMD_SELECT && action->GetID()!=BWrepGameData::CMD_SHIFTSELECT) continue;

		// behind time limit?
		if(action->GetTime()>=timeLimit) break;
	
		// for every seleted element (unit or building)
		const BWrepActionSelect::Params *p = (const BWrepActionSelect::Params *)action->GetParamStruct();
		for(int i=0;i<p->m_unitCount;i++)
		{
			// find element lists who know that element
			for(int j=0; j<GetPlayerCount(); j++)
			{
				// skip observers
				ReplayEvtList *ownerlist = GetEvtList(j);
				if(!ownerlist->IsEnabled()) continue;

				// find element in that event list 
				BWElementList *elemList = ownerlist->GetElemList();
				BWElement *element;
				if((element=elemList->FindElement(p->m_unitid[i]))!=0)
				{
					// get object id
					short objid = element->ObjectID(action->GetTime());
					if(objid!=-1)
					{
						// if we found it and it is identified, it means
						// the element belongs to the player associated with ownerlist
						if(ownerlist!=list)
						{
							// get event
							ReplayEvt *evt = list->GetEvent(action->GetUserData(1));
							assert(evt!=0);

							// event is suspect
							m_suspectCount++;
							evt->SetSuspect();
						}
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------

// enable or disable a player
void Replay::EnablePlayer(ReplayEvtList *list, bool val) 
{
	// enable replay event list
	list->EnablePlayer(val);

	// update max apm
	_UpdateMaxAPMForEnabledPlayers();

	// rebuild enabled action list
	_BuildEnableActionList();
}

//------------------------------------------------------------------------------------------------------------

// get event color
const char * ReplayEvt::strTypeColor() const 
{
	// hack color is RED
	if(IsHack()) return "#FF0000";

	static char buffer[7+1];
	COLORREF rgb = gActionColors[m_type.m_cmd];
	sprintf(buffer,"#%2X%2X%2X",GetRValue(rgb),GetGValue(rgb),GetBValue(rgb));
	return buffer;

}

// get event name
const char * ReplayEvt::strType() const 
{
	if(IsHack()) return "HACK";
	return m_type.GetType();
}

//------------------------------------------------------------------------------------------------------------

// find next suspect event
int Replay::GetNextSuspectEvent(int selectedAction)
{
	// no suspect events?
	if(m_suspectCount==0 && m_hackCount==0) return -1;

	// starting position
	int i=selectedAction+1;

	// search next suspect event
	while(true)
	{
		if(i>=(int)GetEnActionCount()) i=0;
		const IStarcraftAction *action = GetEnAction(i);
		ReplayEvtList *list = (ReplayEvtList *)action->GetUserData(0);
		ReplayEvt *evt = list->GetEvent(action->GetUserData(1));
		if(evt->IsSuspect() || evt->IsHack()) return i;
		i++;
	}

	return -1;
}

//------------------------------------------------------------------------------------------------------------

// return name of original object name for a suspect event
bool ReplayEvtList::GetSuspectEventOrigin(const IStarcraftAction *action, CString& origin, bool hhmmss)
{
	const BWrepActionSelect::Params *p = (const BWrepActionSelect::Params *)action->GetParamStruct();

	// get corresponding actionlist
	ReplayEvtList *alist = (ReplayEvtList *)action->GetUserData(0);
	assert(alist!=0);

	// for every unit	
	for(int i=0;i<p->m_unitCount;i++)
	{
		// check other element lists
		for(int j=0; j<m_replay->GetPlayerCount(); j++)
		{
			// skip observers and ourself
			ReplayEvtList *list = m_replay->GetEvtList(j);
			if(list->IsEnabled() && list!=this)
			{
				// if element already belongs to another player
				BWElementList *elemList = list->GetElemList();
				BWElement *element;
				if((element=elemList->FindElement(p->m_unitid[i]))!=0)
				{
					// get object id
					short objid = element->ObjectID(action->GetTime());
					const char *ptime = _MkTime(m_replay->QueryFile()->QueryHeader(),element->TimeFirstSeen(), hhmmss)	;
					if(objid!=-1)
						origin.Format("%s selects %s's %s identified at %s",alist->PlayerName(),list->PlayerName(),BWrepGameData::GetObjectNameFromID(objid),ptime);
					//else
					//	origin.Format("%s selects %s's unidentified building or unit identified at %s",alist->PlayerName(),list->PlayerName(),ptime);
					return true;
				}
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------------------------

// return object id from any unit id
bool Replay::GetAnyObjectID(short unitID, unsigned long time, short *objID)
{
	// check other element lists
	for(int j=0; j<GetPlayerCount(); j++)
	{
		// skip observers and ourself
		ReplayEvtList *list = GetEvtList(j);
		if(list->IsEnabled())
		{
			// if element belongs to this player
			BWElementList *elemList = list->GetElemList();
			BWElement *element;
			if((element=elemList->FindElement(unitID))!=0)
			{
				// get object id
				*objID = element->ObjectID(time);
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------------------------------------------

// export events to text file
int Replay::ExportToText(const char *textfile, bool useSeconds, char csep)
{
	assert(IsDone());

	// create file
	FILE *fp=fopen(textfile,"wb");
	if(fp==0) return -1;

	// list events in list view
	for(unsigned long i=0;i<GetEnActionCount(); i++)
	{
		// get action
		const IStarcraftAction *action = GetEnAction((int)i);

		// get corresponding actionlist
		ReplayEvtList *list = (ReplayEvtList *)action->GetUserData(0);
		assert(list!=0);

		// get event description
		ReplayEvt *evt = list->GetEvent(action->GetUserData(1));
		assert(evt!=0);

		//assert(evt->Time()!=15900);

		//time
		fprintf(fp,"%s%c",_MkTime(QueryFile()->QueryHeader(),evt->Time(),useSeconds?true:false),csep);
		//player
		fprintf(fp,"%s%c",list->PlayerName(),csep);
		//action
		fprintf(fp,"%s%c",evt->strType(),csep);
		//parameters
		fprintf(fp,"%s%c",action->GetParameters(list->GetElemList()),csep);
		//discard flag
		fprintf(fp,"%s%c",evt->IsDiscarded()?"*":"",csep);
		// units ID
		fprintf(fp,"%s\r\n",action->GetUnitsID(list->GetElemList()));
	}

	//close file
	fclose(fp);
	return 0;
}

//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------

// ctor
ReplayEvt::ReplayEvt(ReplayEvtList *parent, const IStarcraftAction *action, unsigned char sel, int cmd, int subcmd, int objid, const ReplayEvt *prevEvt, bool suspect) 
	: m_parent(parent)
	, m_action(action)
	, m_selection(sel)
	, m_type(cmd,subcmd)
	, m_unitIdx((unsigned char)objid)
	, m_bDiscarded(false)
	, m_suspect(suspect)
	, m_hack(false)
	, m_isobj(true)
{
	// initialize the corresponding time slot 
	ReplayResource *res = pResources();
	if(prevEvt!=0) 
	{
		// if resource slot for this tick has not yet been initialized, do it now
		if(!res->IsInitDone()) m_parent->InitResource(Time());
	}
	else
		res->SetInitDone();
}

//-----------------------------------------------------------------------------------------------------------------

int ReplayEvt::GetBWCoachID() const
{
	return m_isobj ? gAllUnits[m_unitIdx].bwCoachID : gAllTechs[m_unitIdx].bwCoachID;
}

//-----------------------------------------------------------------------------------------------------------------

// resources
const ReplayResource& ReplayEvt::Resources() const 
{
	return *(m_parent->GetResource(Time()));
}

ReplayResource* ReplayEvt::pResources() 
{
	return m_parent->GetResource(Time());
}

//-----------------------------------------------------------------------------------------------------------------

// update resources
void ReplayEvt::UpdateResourceTrain(int unitID,int unitsToAdd) 
{
	m_unitIdx = unitID; 
	pResources()->UpdateResourceTrain(unitID,unitsToAdd);
}

void ReplayEvt::UpdateResourceBuild(int unitID) 
{
	m_unitIdx = unitID; 
	pResources()->UpdateResourceBuild(unitID);
}

void ReplayEvt::UpdateResourceUpgrade(int techID) 
{
	m_isobj=false;
	m_unitIdx = techID; 
	pResources()->UpdateResourceUpgrade(techID);
}

//-----------------------------------------------------------------------------------------------------------------

// RECURSIVE
void ReplayEvtList::InitResource(unsigned long tick)
{
	if(tick<RES_INTERVAL_TICK) return;
	ReplayResource *res = GetResource(tick);
	ReplayResource *prev = GetPrevResource(tick);
	if(!prev->IsInitDone()) InitResource(tick-RES_INTERVAL_TICK);
	res->Clone(*prev);
}

//-----------------------------------------------------------------------------------------------------------------

int ReplayEvtList::GetSlotCount() const 
{
	return m_replay->GetGameLength()/RES_INTERVAL_TICK;
}

//-----------------------------------------------------------------------------------------------------------------

// build botree
void ReplayBuildOrder::MakeBoNodeList(BONodeList* bo)
{
	ReplayObjectSequence all(ReplayObjectSequence::OBJECT,ReplayObjectSequence::MAXBO);
	int i;

	// insert buildings
	for(i=0;i<m_buildings.GetCount();i++)
		if(!all.InsertObject(m_buildings.GetTime(i),m_buildings.GetObject(i)))
			break;
	// insert units
	for(i=0;i<m_units.GetCount();i++)
		if(!all.InsertObject(m_units.GetTime(i),m_units.GetObject(i)+256))
			break;
	// insert research
	for(i=0;i<m_research.GetCount();i++)
		if(!all.InsertObject(m_research.GetTime(i),m_research.GetObject(i)+512))
			break;
	// insert upgrades
	for(i=0;i<m_upgrade.GetCount();i++)
		if(!all.InsertObject(m_upgrade.GetTime(i),m_upgrade.GetObject(i)+1024))
			break;				

	// convert bo to node list 
	bo->RemoveAll();
	for(i=0;i<all.GetCount();i++)
	{
		int content = all.GetObject(i);
		int type = BONode::BLD;
		if(content>=1024) {content-=1024; type = BONode::UPG;}
		else if(content>=512) {content-=512; type = BONode::RES;}
		else if(content>=256) {content-=256; type = BONode::UNIT;}
		bo->AddNode(new BONode(0,content,type));
	}
}

//-----------------------------------------------------------------------------------------------------------------

// get build order including buildings, units, research, upgrade in one sequence
void ReplayEvtList::GetFinalBuildOrder(CString& bo)
{
	// build node list
	BONodeList nodelist;
	m_bo.MakeBoNodeList(&nodelist);

	// convert to string
	nodelist.ToString(bo);
}

//-----------------------------------------------------------------------------------------------------------------
