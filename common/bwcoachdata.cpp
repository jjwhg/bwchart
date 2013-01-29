// BWCoach (C) 2004 jca				
// This file contains a list of all possible training/building/researching/upgrading actions.
// One action per line defined like this: <action id>",<action description>",<action duration>	
// Eg: "1,"Train Marine",20" means it will take 20s for a terran marine to get trained.
//				
// ALL DURATIONS FOR FASTEST MODE ONLY				

#include"stdafx.h"
#include"bwcoachdata.h"

static stBWCOACH_ACTION_DESC gBWCoachActions[]=
{
//----TERRAN----------------------				
{1	,"Train Marine",	20},
{2	,"Train Ghost",	43},
{3	,"Train Vulture",	26},
{4	,"Train Goliath",	34},
{5	,"Train Tank",	43},
{6	,"Train SCV",	17},
{7	,"Train Firebat",	20},
{8	,"Train Medic",	26},
{9	,"Train Wraith",	51},
{10	,"Train Science Vessel",	68},
{11	,"Train Dropship",	43},
{12	,"Train BC",	113},
{13	,"Train Valkyrie",	43},
			
{14	,"Build CC",	102},
{15	,"Build Supply Depot",	34},
{16	,"Build Refinery",	34},
{17	,"Build Barracks",	68},
{18	,"Build Academy",	68},
{19	,"Build Factory",	68},
{20	,"Build Starport",	60},
{21	,"Build Science Facility",	51},
{22	,"Build EBay",	51},
{23	,"Build Armory",	68},
{24	,"Build Turret",	26},
{25	,"Build Bunker",	26},
			
{26	,"Add Comsat Station",	34},
{27	,"Add Nuclear Silo",	34},
{28	,"Add Control Tower",	68},
{29	,"Add Covert Ops",	34},
{30	,"Add Physics Lab",	34},
{31	,"Add Machine Shop",	34},
			
{32	,"Research Stim Packs",	68},
{33	,"Research Lockdown",	85},
{34	,"Research EMP Shockwave",	102},
{35	,"Research Spider Mines",	68},
{36	,"Research Siege Mode",	68},
{37	,"Research Irradiate",	68},
{38	,"Research Yamato Gun",	102},
{39	,"Research Cloaking Field",	85},
{40	,"Research Personnel Cloaking",	68},
{41	,"Research Restoration",	68},
{42	,"Research Optical Flare",	102},
{43	,"Research Marine Range",	85},
{44	,"Research Vulture Speed",	85},
{45	,"Research SciVessel Energy",	141},
{46	,"Research Ghost Sight",	141},
{47	,"Research Ghost Energy",	141},
{48	,"Research Wraith Energy",	141},
{49	,"Research BC Energy",	141},
{50	,"Research Medic Energy",	141},
{51	,"Research Goliath Range",	113},
			
{52	,"Upgrade Infantry Armor",	226},
{53	,"Upgrade Infantry Weapons",	226},
{54	,"Upgrade Vehicle Plating",	226},
{55	,"Upgrade Vehicle Weapons",	226},
{56	,"Upgrade Ship Plating",	226},
{57	,"Upgrade Ship Weapons",	226},
{58	,"Make Nuclear Warhead",	85},
				
//----PROTOSS----------------------
{59	,"Train Dark Templar",	43},
{60	,"Train Dark Archon",	17},
{61	,"Train Probe",	17},
{62	,"Train Zealot",	34},
{63	,"Train Dragoon",	43},
{64	,"Train High Templar",	43},
{65	,"Train Archon",	17},
{66	,"Train Reaver",	60},
{67	,"Train Corsair",	34},
{68	,"Train Shuttle",	51},
{69	,"Train Scout",	68},
{70	,"Train Arbiter",	136},
{71	,"Train Carrier",	119},
{72	,"Train Observer",	34},
			
{73	,"Build Nexus",	102},
{74	,"Build Robotics Facility",	68},
{75	,"Build Pylon",	26},
{76	,"Build Assimilator",	34},
{77	,"Build Observatory",	26},
{78	,"Build Gateway",	51},
{79	,"Build Photon Cannon",	43},
{80	,"Build Cybernetics Core",	51},
{81	,"Build Citadel Of Adun",	51},
{82	,"Build Templar Archives",	51},
{83	,"Build Forge",	34},
{84	,"Build Stargate",	60},
{85	,"Build Fleet Beacon",	51},
{86	,"Build Arbiter Tribunal",	51},
{87	,"Build Robotics Support Bay",	26},
{88	,"Build Shield Battery",	26},
			
{89	,"Research Storm",	102},
{90	,"Research Hallucination",	68},
{91	,"Research Recall",	102},
{92	,"Research Stasis",	85},
{93	,"Research Disruption",	68},
{94	,"Research Mind Control",	102},
{95	,"Research Maelstrom",	85},
{96	,"Research Goon Range",	141},
{97	,"Research Zealot Speed",	113},
{98	,"Research Scarab Damage",	141},
{99	,"Research Reaver Capacity",	141},
{100,"Research Shuttle Speed",	141},
{101,"Research Obs Sight",	113},
{102,"Research Obs Speed",	113},
{103,"Research Templar Energy",	141},
{104,"Research Scout Sight",	141},
{105,"Research Scout Speed",	141},
{106,"Research Arbiter Energy",	141},
{107,"Research Corsair Energy",	141},
{108,"Research DA Energy",	141},
{109,"Research Carrier Capacity",	85},
		
{110,"Upgrade Armor",	226},
{111,"Upgrade Plating",	226},
{112,"Upgrade Ground Weapons",	226},
{113,"Upgrade Air Weapons",	226},
{114,"Upgrade Plasma Shields",	226},
				
//----ZERG----------------------
{115	,"Hatch Zergling",	24},
{116	,"Hatch Hydralisk",	24},
{117	,"Hatch Ultralisk",	51},
{118	,"Hatch Drone",	20},
{119	,"Hatch Defiler",	43},
{120	,"Morph Lurker",	34},
{121	,"Hatch Overlord",	34},
{122	,"Hatch Mutalisk",	34},
{123	,"Morph Guardian",	34},
{124	,"Hatch Queen",	43},
{125	,"Hatch Scourge",	26},
{126	,"Morph Devourer",	34},
			
{127	,"Build Hatchery",	102},
{128	,"Evolve to Lair",	85},
{129	,"Evolve to Hive",	102},
{130	,"Build Nydus Canal",	34},
{131	,"Build Hydralisk den",	34},
{132	,"Build Defiler mound",	51},
{133	,"Evolve to Greater Spire",	102},
{134	,"Build Queen's Nest",	51},
{135	,"Build Evolution Chamber",	34},
{136	,"Build Ultralisk Cavern",	68},
{137	,"Build Spire",	102},
{138	,"Build Spawning Pool",	68},
{139	,"Build Creep Colony",	17},
{140	,"Evolve to Spore Colony",	17},
{141	,"Evolve to Sunken Colony",	17},
{142	,"Build Extractor",	34},
			
{143	,"Research OL Transport",	136},
{144	,"Research OL Sight",	113},
{145	,"Research OL Speed",	113},
{146	,"Research ling Speed",	85},
{147	,"Research ling Attack",	85},
{148	,"Research Hydra Speed",	85},
{149	,"Research Hydra Range",	85},
{150	,"Research Queen Energy",	141},
{151	,"Research Defiler Energy",	141},
{152	,"Research Ultra Armor",	113},
{153	,"Research Ultra Speed",	113},
{154	,"Research Burrowing",	68},
{155	,"Research Spawn Broodling",	68},
{156	,"Research Plague",	85},
{157	,"Research Consume",	85},
{158	,"Research Ensnare",	68},
{159	,"Research Lurker",	102},
	
{160	,"Upgrade Carapace",	226},
{161	,"Upgrade Flyer Carapace",	226},
{162	,"Upgrade Melee Attacks",	226},
{163	,"Upgrade Missile Attacks",	226},
{164	,"Upgrade Flyer Attacks",	226},

{165	,"Build Scarab",	0},
{166	,"Build Interceptor",	0}
};

int BWCoachData::GetCount()
{
	return sizeof(gBWCoachActions)/sizeof(gBWCoachActions[0]);
}

int BWCoachData::GetDuration(int i)
{
	if(i==0) return 0;
	return gBWCoachActions[i-1].duration;
}

const char *BWCoachData::GetMessage(int i)
{
	if(i==0) return 0;
	return gBWCoachActions[i-1].desc;
}
