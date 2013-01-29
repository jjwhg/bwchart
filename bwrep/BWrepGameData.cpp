#include "stdafx.h"
#include "bwrepgamedata.h"
#include "hsvrgb.h"
#include <assert.h>

_stOBJECTDESC gAllUnits[MAXUNIT];
_stACTIONTYPEDESC gAllActions[MAXACTIONTYPE];
_stACTIONTYPEDESC gOverallActions[MAXACTIONTYPE];
COLORREF gActionColors[BWrepGameData::_CMD_MAX_];

const char* BWrepGameData::g_Races[] = {"Zerg","Terran","Protoss"};

const char* BWrepGameData::g_Commands[] = {
	"!0x00",	//0x00
	"!0x01",
	"!0x02",
	"!0x03",
	"!0x04",
	"!0x05",	//0x05
	"!0x06",
	"!0x07",
	"!0x08",
	"Select",
	"Shift Select",	//0x0a
	"Shift Deselect",
	"Build",
	"Vision",
	"Ally",
	"!0x0f",
	"!0x10",	//0x10
	"!0x11",
	"!0x12",
	"Hotkey",
	"Move",
	"Attack",	//0x15
	"!0x16",
	"!0x17",
	"Cancel",
	"Cancel Hatch",
	"Stop",	//0x1a
	"!0x1b",
	"!0x1c",
	"!0x1d",
	"Return Cargo",
	"Train",
	"Cancel Train",	//cancel train
	"Cloak",	//cloak?
	"Decloak",	//decloak?
	"Hatch",
	"!0x24",
	"Unsiege",	//0x25
	"Siege",
	"Arm (Interceptor/Scarab)",
	"Unload All",
	"Unload",
	"Merge Archon",	//0x2a
	"Hold Position",
	"Burrow",
	"Unburrow",
	"Cancel Nuke",
	"Lift",
	"Research",	//0x30
	"!0x31",
	"Upgrade",
	"!0x33",
	"!0x34",
	"Morph",	//0x35
	"Stim",
	"!0x37",
	"!0x38",
	"!0x39",
	"!0x3a",	//0x3a
	"!0x3b",
	"!0x3c",
	"!0x3d",
	"!0x3e",
	"!0x3f",
	"!0x40",	//0x40
	"!0x41",
	"!0x42",
	"!0x43",
	"!0x44",
	"!0x45",	//0x45
	"!0x46",
	"!0x47",
	"!0x48",
	"!0x49",
	"!0x4a",	//0x4a
	"!0x4b",
	"!0x4c",
	"!0x4d",
	"!0x4e",
	"!0x4f",
	"!0x50",	//0x50
	"!0x51",
	"!0x52",
	"!0x53",
	"!0x54",
	"!0x55",
	"!0x56",
	"Leave Game",
	"Minimap Ping",
	"!0x59",
	"Merge Dark Archon",
	"Deselect",
	"Chat"
};
const int BWrepGameData::g_CommandsSize = sizeof(g_Commands)/sizeof(g_Commands[0]);

const char* BWrepGameData::g_Research[] = {
	"Stim Pack",	//0x00
	"Lockdown",
	"EMP Shockwave",
	"Spider Mines",
	"",
	"Siege Tank", //0x05
	"",
	"Irradiate",
	"Yamato Gun",
	"Cloaking Field", //wraiths
	"Personal Cloaking", //0x0a
	"Burrow",
	"",
	"Spawn Broodling",
	"",
	"Plague",
	"Consume",	//0x10
	"Ensnare",
	"",
	"Psionic Storm",
	"Hallucination", 
	"Recall", //0x15 
	"Stasis Field",
	"",
	"Restoration",
	"Disruption Web",
	"", //0x1a
	"Mind Control",
	"",
	"",
	"Optical Flare",
	"Maelstrom",
	"Lurker Aspect",	//0x20
	"",
	"",
	"",
	"",
	"", //0x25
	"",
	"",
	"",
	"",
	"", //0x2a
	"",
	"",
	"",
	"",
	"",
	"", //0x30
	"",
};
const int BWrepGameData::g_ResearchSize = sizeof(g_Research)/sizeof(g_Research[0]);

const char* BWrepGameData::g_Upgrades[] = {
	"Terran Infantry Armor",	//0x00
	"Terran Vehicle Plating",
	"Terran Ship Plating",
	"Zerg Carapace",
	"Zerg Flyer Carapace",
	"Protoss Ground Armor", //0x05
	"Protoss Air Armor",
	"Terran Infantry Weapons",
	"Terran Vehicle Weapons",
	"Terran Ship Weapons",
	"Zerg Melee Attacks", //0x0a
	"Zerg Missile Attacks",
	"Zerg Flyer Attacks",
	"Protoss Ground Weapons",
	"Protoss Air Weapons",
	"Protoss Plasma Shields",
	"U-238 Shells (Marine Range)",	//0x10
	"Ion Thrusters (Vulture Speed)",
	"",
	"Titan Reactor (Science Vessel Energy)",
	"Ocular Implants (Ghost Sight)",
	"Moebius Reactor (Ghost Energy)", //0x15
	"Apollo Reactor (Wraith Energy)",
	"Colossus Reactor (Battle Cruiser Energy)",
	"Ventral Sacs (Overlord Transport)",
	"Antennae (Overlord Sight)",
	"Pneumatized Carapace (Overlord Speed)", //0x1a
	"Metabolic Boost (Zergling Speed)",
	"Adrenal Glands (Zergling Attack)",
	"Muscular Augments (Hydralisk Speed)",
	"Grooved Spines (Hydralisk Range)",
	"Gamete Meiosis (Queen Energy)",
	"Defiler Energy",	//0x20
	"Singularity Charge (Dragoon Range)",
	"Leg Enhancement (Zealot Speed)",
	"Scarab Damage",
	"Reaver Capacity",
	"Gravitic Drive (Shuttle Speed)", //0x25
	"Sensor Array (Observer Sight)",
	"Gravitic Booster (Observer Speed)",
	"Khaydarin Amulet (Templar Energy)",
	"Apial Sensors (Scout Sight)",
	"Gravitic Thrusters (Scout Speed)", //0x2a
	"Carrier Capacity",
	"Khaydarin Core (Arbiter Energy)",
	"",
	"",
	"Argus Jewel (Corsair Energy)",
	"", //0x30
	"Argus Talisman (Dark Archon Energy)",
	"",
	"Caduceus Reactor (Medic Energy)",
	"Chitinous Plating (Ultralisk Armor)",
	"Anabolic Synthesis (Ultralisk Speed)", //0x35
	"Charon Boosters (Goliath Range)",
	"",
	"",
	"",
	"", //0x3a
};

const int BWrepGameData::g_UpgradesSize = sizeof(g_Upgrades)/sizeof(g_Upgrades[0]);

const char* BWrepGameData::g_BuildingTypes[] = {
	"",	//0x00
	"",
	"",
	"",
	"",
	"", //0x05
	"",
	"",
	"",
	"",
	"", //0x0a
	"",
	"",
	"",
	"",
	"",
	"",	//0x10
	"",
	"",
	"",
	"",
	"", //0x15
	"",
	"",
	"",
	"Morph", //"Zerg Building",
	"", //0x1a
	"",
	"",
	"",
	"Build", //"Terran Building",
	"Warp", //"Protoss Building",
	"",	//0x20
	"",
	"",
	"",
	"Add-On", //"Terran Add-On",
	"", //0x25
	"",
	"",
	"",
	"",
	"", //0x2a
	"",
	"",
	"",
	"Evolve", //"Zerg Add-On",
	"",
	"",	//0x30
	"",
	"",
	"",
	"",
	"", //0x35
	"",
	"",
	"",
	"",
	"", //0x3a
	"",
	"",
	"",
	"",
	"",
	"",	//0x40
	"",
	"",
	"",
	"",
	"", //0x45
	"",
	"Land",	//terran landing building
	"",
	"",
	"", //0x4a
	"",
	"",
	"",
	"",
	"",
};

const int BWrepGameData::g_BuildingTypesSize = sizeof(g_BuildingTypes)/sizeof(g_BuildingTypes[0]);

const char* BWrepGameData::g_Objects[] = {
	"Marine",	//0x00
	"Ghost",
	"Vulture",
	"Goliath",
	"",
	"Siege Tank", //0x05
	"",
	"SCV",
	"Wraith",
	"Science Vessel",
	"", //0x0a
	"Dropship",
	"Battlecruiser",
	"",
	"Nuke",
	"",
	"",	//0x10
	"",
	"",
	"",
	"",
	"", //0x15
	"",
	"",
	"",
	"",
	"", //0x1a
	"",
	"",
	"",
	"",
	"",
	"Firebat",	//0x20
	"",
	"Medic",
	"",
	"",
	"Zergling", //0x25
	"Hydralisk",
	"Ultralisk",
	"",
	"Drone",
	"Overlord", //0x2a
	"Mutalisk",
	"Guardian",
	"Queen",
	"Defiler",
	"Scourge",
	"",	//0x30
	"",
	"Infested Terran",
	"",
	"",
	"", //0x35
	"",
	"",
	"",
	"",
	"Valkyrie", //0x3a
	"",
	"Corsair",
	"Dark Templar",
	"Devourer",
	"",
	"Probe",	//0x40
	"Zealot",
	"Dragoon",
	"High Templar",
	"",
	"Shuttle", //0x45
	"Scout",
	"Arbiter",
	"Carrier",
	"",
	"", //0x4a
	"",
	"",
	"",
	"",
	"",
	"",	//0x50
	"",
	"",
	"Reaver",
	"Observer",
	"", //0x55
	"",
	"",
	"",
	"",
	"", //0x5a
	"",
	"",
	"",
	"",
	"",
	"",	//0x60
	"",
	"",
	"",
	"",
	"", //0x65
	"",
	"Lurker",
	"",
	"",
	"Command Center",	//0x006a
	"ComSat",
	"Nuclear Silo",
	"Supply Depot",
	"Refinery",	//refinery?
	"Barracks",
	"Academy",	//Academy?	//0x0070
	"Factory",
	"Starport",
	"Control Tower",
	"Science Facility",
	"Covert Ops",	//0x0075
	"Physics Lab",
	"",
	"Machine Shop",
	"",
	"Engineering Bay",	//0x007a
	"Armory",
	"Missile Turret",
	"Bunker",
	"",
	"",
	"",	//0x80
	"",
	"Infested CC",
	"Hatchery",
	"Lair",
	"Hive", //0x85
	"Nydus Canal",
	"Hydralisk Den",
	"Defiler Mound",
	"Greater Spire",
	"Queens Nest", //0x8a
	"Evolution Chamber",
	"Ultralisk Cavern",
	"Spire",
	"Spawning Pool",
	"Creep Colony",
	"Spore Colony",	//0x90
	"",
	"Sunken Colony",
	"",
	"",
	"Extractor", //0x95
	"",
	"",
	"",
	"",
	"Nexus", //0x9a
	"Robotics Facility",
	"Pylon",
	"Assimilator",
	"",
	"Observatory",
	"Gateway",	//0xa0
	"",
	"Photon Cannon",
	"Citadel of Adun",
	"Cybernetics Core",
	"Templar Archives", //0xa5
	"Forge",
	"Stargate",
	"",
	"Fleet Beacon",
	"Arbiter Tribunal", //0xaa
	"Robotics Support Bay",
	"Shield Battery",
	"",
	"",
	"",
	"",	//0xb0
	"",
	"",
	"",
	"",
	"", //0xb5
	"",
	"",
	"",
	"",
	"", //0xba
	"",
	"",
	"",
	"",
	"Larva",
	"Rine/Bat",
	"Dark Archon",
	"Archon",
	"Scarab",
	"Interceptor",
	"Interceptor/Scarab"
};

const int BWrepGameData::g_ObjectsSize = sizeof(g_Objects)/sizeof(g_Objects[0]);

const char* BWrepGameData::g_Attacks[] = {
	"Move",	//0x00 move with right click
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown", //0x05
	"Move", // move with key or click on icon (can be attack icon as well)
	"Unknown",
	"Attack",
	"Gather",
	"Unknown", //0x0a
	"Unknown",
	"Unknown",
	"Unknown",
	"Attack Move",
	"Unknown",
	"Unknown",	//0x10
	"Unknown",
	"Unknown",
	"Failed Casting?",
	"Unknown",
	"Unknown", //0x15
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown", //0x1a
	"Infest CC",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",	//0x20
	"Unknown",
	"Repair",
	"Unknown",
	"Unknown",
	"Unknown", //0x25
	"Unknown",
	"Clear Rally",
	"Set Rally",
	"Unknown",
	"Unknown", //0x2a
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",	//0x30
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Attack", //0x35 for carriers it seems
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown", //0x3a
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",	//0x40
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown", //0x45
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown", //0x4a
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Gather",
	"Gather",	//0x50
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown", //0x55
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown", //0x5a
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",	//0x60
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown", //0x65
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",	//0x006a
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unload",	//0x0070
	"Yamato",
	"Unknown",
	"Lockdown",
	"Unknown",
	"Unknown",	//0x0075
	"Unknown",
	"Dark Swarm",
	"Parasite",
	"Spawn Broodling",
	"EMP",	//0x007a
	"Unknown",
	"Unknown",
	"Unknown",
	"Launch Nuke",
	"Unknown",
	"Unknown",	//0x80
	"Unknown",
	"Unknown",
	"Unknown",
	"Lay Mine",
	"Unknown", //0x85
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown", //0x8a
	"ComSat Scan",
	"Unknown",
	"Defense Matrix",
	"Psionic Storm",
	"Recall", // & lockdown
	"Plague",	//0x90
	"Consume",
	"Ensnare",
	"Stasis",
	"Hallucination",
	"Unknown", //0x95
	"Unknown",
	"Unknown",
	"Patrol",
	"Unknown",
	"Unknown", //0x9a
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",	//0xa0
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown", //0xa5
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown", //0xaa
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",	//0xb0
	"Heal",
	"Unknown",
	"Unknown",
	"Restore",
	"Disruption Web", //0xb5
	"Mind Control",
	"Unknown",
	"Feedback",
	"Optic Flare",
	"Maelstrom", //0xba
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Irradiate"
};

const int BWrepGameData::g_AttacksSize = sizeof(g_Attacks)/sizeof(g_Attacks[0]);

const char* BWrepGameData::g_AttackModifiers[] = {
	"",
	"Shift-Queue",
};

const int BWrepGameData::g_AttackModifiersSize = sizeof(g_AttackModifiers)/sizeof(g_AttackModifiers[0]);

const char* BWrepGameData::g_HotKeyModifiers[] = {
	"Assign",
	"Select",
	"Add",
};

const int BWrepGameData::g_HotKeyModifiersSize = sizeof(g_HotKeyModifiers)/sizeof(g_HotKeyModifiers[0]);

//------------------------------------------------------------------------------------------------------------

// get action name from action id
const char *BWrepGameData::GetActionNameFromID(int id)
{
	assert(id>=0 && id<BWrepGameData::g_CommandsSize);
	return BWrepGameData::g_Commands[id];
}

//------------------------------------------------------------------------------------------------------------

// get object name from object id
const char *BWrepGameData::GetObjectNameFromID(int id)
{
	assert(id>=0 && id<BWrepGameData::g_ObjectsSize);
	return BWrepGameData::g_Objects[id];
}

//------------------------------------------------------------------------------------------------------------

// get research name from research id
const char *BWrepGameData::GetResearchNameFromID(int id)
{
	assert(id>=0 && id<BWrepGameData::g_ResearchSize);
	return BWrepGameData::g_Research[id];
}

//------------------------------------------------------------------------------------------------------------

// get upgrade name from upgrade id
const char *BWrepGameData::GetUpgradeNameFromID(int id)
{
	static char buffer[128];
	assert(id>=0 && id<BWrepGameData::g_UpgradesSize);
	strcpy(buffer,BWrepGameData::g_Upgrades[id]);
	char *p=strchr(buffer,'('); if(p!=0) p=strtok(p+1,")"); else p=buffer;
	return p;
}

//------------------------------------------------------------------------------------------------------------

// get attack name from attack id
const char *BWrepGameData::GetAttackNameFromID(int id)
{
	assert(id>=0 && id<BWrepGameData::g_AttacksSize);
	return BWrepGameData::g_Attacks[id];
}

//------------------------------------------------------------------------------------------------------------

// true if id is for a building
bool BWrepGameData::IsBuilding(int id)
{
	if(id>=OBJ_COMMANDCENTER && id<=OBJ_BUNKER)
		return true;
	if(id>=OBJ_INFESTED_COMMANDCENTER && id<=OBJ_EXTRACTOR)
		return true;
	if(id>=OBJ_NEXUS && id<=OBJ_SHIELDBATTERY)
		return true;
	return false;
}

//------------------------------------------------------------------------------------------------------------

// get color 
COLORREF BWrepGameData::GetBarColor(int idx, int maxc)
{
	int h = ((8*359*idx)/maxc)%360;
	float v = 0.6f;
	float s = 0.6f;
	return CHsvRgb::Hsv2Rgb(h,s,v).rgb();
}

//------------------------------------------------------------------------------------------------------------

#define DEFOBJ(n,sup,min,gaz, bwcid) gAllUnits[BWrepGameData::OBJ_##n].Set(sup,min,gaz,0,0,bwcid);
#define DEFOBJ2(n,min,gaz,w,h, bwcid) gAllUnits[BWrepGameData::OBJ_##n].Set(0,min,gaz,w,h,bwcid);
#define DEFTECH(n,min,gaz,mt,bwcid) {n,min,gaz,mt,bwcid},

void BWrepGameData::InitUnits()
{
	static bool bNeedInit=true;
	if(!bNeedInit) return;

	memset(gAllUnits,0,sizeof(gAllUnits));
	memset(gAllActions,0,sizeof(gAllActions));
	memset(gActionColors,0,sizeof(gActionColors));

	DEFOBJ(ARBITER	,	8	,	100	,	350,	70	)
	DEFOBJ(CARRIER	,	12	,	350	,	250,	71	)
	DEFOBJ(CORSAIR	,	4	,	150	,	100,	67	)
	DEFOBJ(DARKTEMPLAR,	4	,	125	,	100,	59	)
	DEFOBJ(DRAGOON	,	4	,	125	,	50,		63	)
	DEFOBJ(HIGHTEMPLAR,	4	,	50	,	150,	64	)
	DEFOBJ(OBSERVER	,	2	,	25	,	75	,	72)
	DEFOBJ(PROBE	,	2	,	50	,	0	,	61)
	DEFOBJ(REAVER	,	8	,	200	,	100	,	66)
	DEFOBJ(SCOUT	,	6	,	275	,	125	,	69)
	DEFOBJ(ZEALOT	,	4	,	100	,	0	,	62)
	DEFOBJ(SHUTTLE	,	4	,	200	,	0	,	68)

	DEFOBJ(BATTLECRUISER,12	,	400	,	300	,	12)
	DEFOBJ(DROPSHIP	,	4	,	100	,	100	,	11)
	DEFOBJ(FIREBAT	,	2	,	50	,	25	,	7)
	DEFOBJ(GHOST	,	2	,	25	,	75	,	2)
	DEFOBJ(GOLIATH	,	4	,	100	,	50	,	4)
	DEFOBJ(MARINE	,	2	,	50	,	0	,	1)
	DEFOBJ(MEDIC	,	2	,	50	,	25	,	8)
	DEFOBJ(SCIENCEVESSEL,4	,	100	,	225	,	10)
	DEFOBJ(SCV	,	2	,	50	,	0		,	6)
	DEFOBJ(SIEGETANK,	4	,	150	,	100	,	5)
	DEFOBJ(VALKYRIE	,	6	,	250	,	125	,	13)
	DEFOBJ(VULTURE	,	4	,	75	,	0	,	3)
	DEFOBJ(WRAITH	,	4	,	150	,	100	,	9)
	DEFOBJ(NUKE	,	16	,	200	,	200		,	58)

	DEFOBJ(DEFILER	,	4	,	50	,	150	,	119)
	DEFOBJ(DEVOURER	,	4	,	250	,	150	,	126)
	DEFOBJ(DRONE	,	2	,	50	,	0	,	118)
	DEFOBJ(GUARDIAN	,	4	,	150	,	200	,	123)
	DEFOBJ(HYDRALISK,	2	,	75	,	25	,	116)
	DEFOBJ(LURKER	,	4	,	125	,	125	,	120)
	DEFOBJ(MUTALISK	,	4	,	100	,	100	,	122)
	DEFOBJ(OVERLORD	,	0	,	100	,	0	,	121)
	DEFOBJ(QUEEN	,	4	,	100	,	100	,	124)
	DEFOBJ(SCOURGE	,	1	,	12	,	38	,	125)
	DEFOBJ(ULTRALISK,	8	,	200	,	200	,	117)
	DEFOBJ(ZERGLING	,	1	,	25	,	0	,	115)
	DEFOBJ(INFESTEDTERRAN,2	,	100	,	50	,	000)

	DEFOBJ2(SUPPLYDEPOT, 100, 0, 3, 2		,	15)
	DEFOBJ2(COMMANDCENTER, 400, 0,4,3		,	14)
	DEFOBJ2(COMSAT, 50, 50 ,2,2				,	26)
	DEFOBJ2(NUCLEARSILO,100,100,2,2			,	27)
	DEFOBJ2(ENGINEERINGBAY, 125, 0,4,3		,	22)
	DEFOBJ2(BARRACKS, 150, 0,4,3			,	17)
	DEFOBJ2(REFINERY, 100, 0,4,2			,	16)
	DEFOBJ2(MISSILETURRET, 75,0,2,2			,	24)
	DEFOBJ2(ACADEMY, 150,0,3,2				,	18)
	DEFOBJ2(BUNKER, 100,0,2,2				,	25)
	DEFOBJ2(FACTORY,200,100,4,3				,	19)
	DEFOBJ2(MACHINESHOP ,50,50,2,2			,	31)
	DEFOBJ2(ARMORY ,100,50,3,2				,	23)
	DEFOBJ2(STARPORT,150,100,4,3			,	20)
	DEFOBJ2(CONTROLTOWER ,50,50,2,2			,	28)
	DEFOBJ2(SCIENCEFACILITY, 100,150,4,3	,	21)
	DEFOBJ2(PHYSICSLAB , 50, 50,2,2			,	30)
	DEFOBJ2(COVERTOPS ,50, 50,2,2			,	29)

	DEFOBJ2(ASSIMILATOR,100,0,4,2			,	76)
	DEFOBJ2(NEXUS,400,0,4,3					,	73)
	DEFOBJ2(PYLON,100,0,2,2					,	75)
	DEFOBJ2(GATEWAY,150,0,4,3				,	78)
	DEFOBJ2(FORGE,150,0,3,2					,	83)
	DEFOBJ2(SHIELDBATTERY,100,0,3,2			,	88)	//AC
	DEFOBJ2(CYBERNETICSCORE,200,0,3,2		,	80)
	DEFOBJ2(PHOTONCANNON,200,0,2,2			,	79)
	DEFOBJ2(ROBOTICSFACILITY,200,200,3,2	,	74)
	DEFOBJ2(ROBOTICSSUPPORTBAY,150,100,3,2	,	87) //AB
	DEFOBJ2(STARGATE,150,150,4,3			,	84)
	DEFOBJ2(CITADELOFADUN,150,100,3,2		,	81)
	DEFOBJ2(FLEETBEACON,300,200,3,2			,	85)
	DEFOBJ2(TEMPLARARCHIVES,150,200,3,2		,	82)
	DEFOBJ2(OBSERVATORY,50,100,3,2			,	77)
	DEFOBJ2(ARBITERTRIBUNAL,200,150,3,2		,	86)	//AA

	DEFOBJ2(EXTRACTOR,50,0,4,2				,	142)
	DEFOBJ2(HATCHERY,300,0,4,3				,	127)
	DEFOBJ2(EVOLUTIONCHAMBER,75,0,3,2		,	135)
	DEFOBJ2(CREEPCOLONY,75,0,2,2			,	139)
	DEFOBJ2(SPAWNINGPOOL,200,0,3,2			,	138)
	DEFOBJ2(SPORECOLONY,50,0,2,2			,	140)
	DEFOBJ2(SUNKENCOLONY,50,0,2,2			,	141)
	DEFOBJ2(HYDRALISKDEN,100,50,3,2			,	131)
	DEFOBJ2(LAIR, 150,100,-3,-3				,	128)
	DEFOBJ2(SPIRE , 200, 150,2,2			,	137)
	DEFOBJ2(QUEENSNEST ,	150,100,3,2		,	134)      
	DEFOBJ2(GREATERSPIRE , 100, 150,-4,-3	,	133)
	DEFOBJ2(HIVE , 200, 150,-4,-3			,	129)
	DEFOBJ2(NYDUSCANAL,150,0,2,2			,	130)
	DEFOBJ2(DEFILERMOUND ,100,100,4,2		,	132)
	DEFOBJ2(ULTRALISKCAVERN, 150,200,3,2	,	136)

	DEFOBJ(DARKARCHON,	4,	250,200			,	60)
	DEFOBJ(ARCHON,	4,	100,300				,	65)
	DEFOBJ(SCARAB,	0,	25,	0				,	165)
	DEFOBJ(INTERCEPTOR,	0,	25,	0			,	166)
	DEFOBJ(SCARABORINTERCEPTOR,	0,	25,	0	,	000)

	// count valid slots for units
	int i,j,valid=0;
	for(i=0; i<MAXUNIT; i++)
		if(!gAllUnits[i].IsEmpty()) 
			valid++;

	// compute colors for units
	for(i=0,j=0; i<MAXUNIT; i++)
		if(!gAllUnits[i].IsEmpty()) 
			gAllUnits[i].clr = GetBarColor(j++, valid);

	// count valid slots for actions
	valid=0;
	for(i=0; i<MAXACTIONTYPE; i++)
		if(!gAllActions[i].IsEmpty(i)) 
			valid++;

	// compute colors for actions
	for(i=0,j=0; i<MAXACTIONTYPE; i++)
		if(!gAllActions[i].IsEmpty(i)) 
			gAllActions[i].clr = GetBarColor(j++, valid);

	// default action colors
	gActionColors[BWrepGameData::CMD_BUILD]=RGB(20,20,210);
	gActionColors[BWrepGameData::CMD_MORPH]=RGB(20,20,210);
	gActionColors[BWrepGameData::CMD_TRAIN]=RGB(20,180,20);
	gActionColors[BWrepGameData::CMD_HATCH]=RGB(20,180,20);
	gActionColors[BWrepGameData::CMD_HOTKEY]=RGB(140,20,140);
	gActionColors[BWrepGameData::CMD_LEAVEGAME]=RGB(255,0,0);
	gActionColors[BWrepGameData::CMD_RESEARCH]=RGB(180,160,20);
	gActionColors[BWrepGameData::CMD_UPGRADE]=RGB(180,160,20);
	gActionColors[BWrepGameData::CMD_MERGEARCHON]=RGB(160,10,10);
	gActionColors[BWrepGameData::CMD_MERGEDARKARCHON]=RGB(140,10,10);
	gAllActions[BWrepGameData::CMD_MESSAGE].clr =gActionColors[BWrepGameData::CMD_MESSAGE]=RGB(140,40,40);

	bNeedInit=false;
}

//------------------------------------------------------------------------------------------------------------

_stTECHNICDESC gAllTechs[]={

DEFTECH("Spider Mines",100,100,1				,35)
DEFTECH("Goliath Range",100,100,1				,51)
DEFTECH("Terran Vehicle Weapons",100,100,3		,55)
DEFTECH("Terran Vehicle Plating",100,100,3		,54)
DEFTECH("Terran Ship Weapons",100,100,3			,57)
DEFTECH("Terran Ship Plating",100,100,3			,56)
DEFTECH("Terran Infantry Weapons",100,100,3		,53)
DEFTECH("Terran Infantry Armor",100,100,3		,52)
DEFTECH("Siege Tank",100,100,1					,36)
DEFTECH("Marine Range",100,100,1				,43)
DEFTECH("Restoration",100,100,1					,41)
DEFTECH("Optical Flare",100,100,1				,42)
DEFTECH("Ghost Sight",100,100,1					,46)
DEFTECH("Ghost Energy",150,150,1				,47)
DEFTECH("Lockdown",200,200,1					,33)
DEFTECH("Personal Cloaking",100,100,1			,40)
DEFTECH("Vulture Speed",100,100,1				,44)
DEFTECH("Wraith Energy",200,200,1				,48)
DEFTECH("Cloaking Field",150,150,1				,39)
DEFTECH("Battle Cruiser Energy",150,150,1		,49)
DEFTECH("Science Vessel Energy",150,150,1		,45)
DEFTECH("Yamato Gun",100,100,1					,38)
DEFTECH("EMP Shockwave",200,200,1				,34)
DEFTECH("Irradiate",200,200,1					,37)
DEFTECH("Medic Energy",150,150,1				,50)
DEFTECH("Stim Pack",100,100,1					,32)
 
DEFTECH("Dragoon Range",150,150,1				,96)
DEFTECH("Protoss Ground Weapons",100,100,3		,112)
DEFTECH("Protoss Ground Armor",100,100,3		,110)
DEFTECH("Protoss Air Armor",150,150,3			,110)
DEFTECH("Protoss Air Weapons",100,100,3			,113)
DEFTECH("Zealot Speed",150,150,1				,97)
DEFTECH("Protoss Plasma Shields",200,200,3		,114) //300,300  et 400,400
DEFTECH("Templar Energy",150,150,1				,103)
DEFTECH("Arbiter Energy",150,150,1				,106)
DEFTECH("Scarab Damage",200,200,1				,98)
DEFTECH("Reaver Capacity",200,200,1				,99)
DEFTECH("Observer Sight",150,150,1				,101)
DEFTECH("Scout Sight",100,100,1					,004)
DEFTECH("Observer Speed",150,150,1				,102)
DEFTECH("Scout Speed",200,200,1					,105)
DEFTECH("Shuttle Speed",200,200,1				,100) 
DEFTECH("Disruption Web",200,200,1				,93)
DEFTECH("Corsair Energy",100,100,1				,107)
DEFTECH("Dark Archon Energy",150,150,1			,108)
DEFTECH("Carrier Capacity",100,100,1			,109)
DEFTECH("Maelstrom",100,100,1					,95)	      
DEFTECH("Psionic Storm",200,200,1				,89)	      
DEFTECH("Recall",150,150,1						,91)	     
DEFTECH("Stasis Field",150,150,1				,92)	   
DEFTECH("Hallucination",150,150,1				,90)
DEFTECH("Mind Control",200,200,1				,94)	
						
DEFTECH("Hydralisk Speed",150,150,1				,148)
DEFTECH("Hydralisk Range",150,150,1				,149)
DEFTECH("Zergling Speed",100,100,1				,146)
DEFTECH("Zergling Attack",200,200,1				,147)
DEFTECH("Ultralisk Speed",200,200,1				,153)
DEFTECH("Ultralisk Armor",150,150,1				,152)
DEFTECH("Burrow",100,100,1						,154)  
DEFTECH("Zerg Flyer Attacks",100,100,3			,164)  //Level 2 - 150  150  //Level 3 - 200  200  
DEFTECH("Zerg Melee Attacks",100,100,3			,162)  //Level 2 - 150  150  //Level 3 - 200  200  
DEFTECH("Zerg Carapace",150,150,3				,160)  //Level 2 - 225  225  //Level 3 - 300  300  
DEFTECH("Zerg Flyer Carapace",150,150,3			,161)  //Level 2 - 225  225  //Level 3 - 300  300  
DEFTECH("Zerg Missile Attacks",100,100,3		,163) //Level 2 - 150  150  //Level 3 - 200  200  
DEFTECH("Overlord Transport", 200, 200,1		,143)  
DEFTECH("Overlord Sight",150, 150,1				,144)  
DEFTECH("Overlord Speed",150,150,1				,145)   
DEFTECH("Queen Energy",150,150,1				,150)
DEFTECH("Defiler Energy",150,150,1				,151)
DEFTECH("Ensnare",100,100,1						,158)
DEFTECH("Spawn Broodling",100,100,1				,155)
DEFTECH("Consume",100,100,1						,157)
DEFTECH("Plague",200,200,1						,156)
DEFTECH("Lurker Aspect",200,200,1				,159) //????????

DEFTECH("11",100,100,1,000)
DEFTECH("17",100,100,1,000)
DEFTECH("14",100,100,1,000)
DEFTECH("09",100,100,1,000)
DEFTECH("01",100,100,1,000)
DEFTECH("0A",100,100,1,000)
DEFTECH("08",100,100,1,000)
DEFTECH("18",100,100,1,000)
DEFTECH("02",100,100,1,000)

};

int MAXTECHNIC() {return (sizeof(gAllTechs)/sizeof(gAllTechs[0]));}

//------------------------------------------------------------------------------------------------------------
