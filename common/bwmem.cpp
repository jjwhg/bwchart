#include "stdafx.h"
#include "bwmem.h"
#include <assert.h>

int BWMem::m_patch = PATCH_UNKNOWN;
unsigned long BWMem::gByteAddress[__BWMEM__MAX];


//--------------------------------------------------------------------------------------------------------------

//get address for a specific info
unsigned long BWMem::Addr(int info) 
{
	if(m_patch==PATCH_UNKNOWN) SetBWPatch(PATCH_110);
	return gByteAddress[info];
}

//--------------------------------------------------------------------------------------------------------------

// set BW engine version
void BWMem::SetBWPatch(int patch)
{
	m_patch=patch;




	// BW bytes for 1.10
	gByteAddress[MINERAL1	]= 0x501370;
	gByteAddress[GameTime	]= 0x5014BC;
	gByteAddress[TICK		]= 0x5014BC;
	gByteAddress[REPLAYFILE	]= 0x501FBC;

	gByteAddress[INGAMEBIS  ]= 0x636DB4; // from superpinguin

	gByteAddress[MineralEnd	]= 0x643C50;

	gByteAddress[MSGTEXT0	]= 0x650CA8; // 218 bytes
	gByteAddress[MSGTICK0	]= 0x6517BC; // 4 bytes
	gByteAddress[MSGINDEX	]= 0x6517F0; // byte

	gByteAddress[LOGIN		]= 0x653048;  
	gByteAddress[PlayerDesc	]= 0x653088;
	gByteAddress[Slot1		]= 0x653093;
	gByteAddress[Slot2		]= 0x6530B7;
	gByteAddress[Slot3		]= 0x6530Db;
	gByteAddress[Slot4		]= 0x6530ff;
	gByteAddress[Slot5		]= 0x653123;
	gByteAddress[Slot6		]= 0x653147;
	gByteAddress[Slot7		]= 0x65316B;
	gByteAddress[Slot8		]= 0x65318F;
	gByteAddress[BNETFLAG	]= 0x6532A0;

	gByteAddress[GAMEBNET	]= 0x6532CC;
	gByteAddress[MAPBNET	]= 0x653315;

	gByteAddress[HPLAYER	]= 0x6538B0;

	
	gByteAddress[INGAME		]= 0x65e368;  // including game lobby on bnet, but not game results
	gByteAddress[GAME		]= 0x65e3A0;
	gByteAddress[MAP		]= 0x65e3E9;

	gByteAddress[REALMINERAL]= 0x69013C;
	gByteAddress[GAS		]= 0x690138;
	gByteAddress[SUPPLY_STR	]= 0x690090;

	
	if(m_patch==PATCH_111)
	{
		for(int i=0;i<__BWMEM__MAX;i++)
			gByteAddress[i]-=0x3ED0;
	}
	else if(m_patch==PATCH_112)
	{
		gByteAddress[MINERAL1	]= 0x508620; //// 0x501370 - 0x3ED0;
		gByteAddress[GameTime	]= 0x50876C; ////
		gByteAddress[TICK		]= 0x50876C; ////
		gByteAddress[REPLAYFILE	]= 0x50926C; ////


	// SP: l'addresse que tu avait noté été pas bonne je pense
	// tu a fait +0x3ED au lieu de -0x3ED : 
	// l'ancienne adresse corigée est : 0x63AC84 (a modifier + bas peu etre)
	// la nouvelle est ici :
		gByteAddress[INGAMEBIS  ]= 0x645E0C; //// from superpinguin (including game result, but not lobby (i think))


	// SP: attention : celui la je l'ai converti mais j'en suis pas sur du tout :
	// y'a pas de références direct dessus, c'est pas une vrai
	// adresse, c'est quelque part au milieu d'un tableau
		gByteAddress[MineralEnd	]= 0x64B610; ////


		gByteAddress[MSGTEXT0	]= 0x65864C; //// 218 bytes
		gByteAddress[MSGTICK0	]= 0x659160; //// 4 bytes
		gByteAddress[MSGINDEX	]= 0x659194; //// byte

		gByteAddress[LOGIN		]= 0x65ADF8; ////
		gByteAddress[PlayerDesc	]= 0x65AE38; ////

		gByteAddress[Slot1		]= 0x65AE43; ////
	// pas testé a fond mais on vas dire que ca vas toujours de 0x24 en 0x24 pour tt les slots
	// si c pas ca faut rechanger
		gByteAddress[Slot2		]= 0x65AE67; ////
		gByteAddress[Slot3		]= 0x65AE8B; ////
		gByteAddress[Slot4		]= 0x65AEAF; ////
		gByteAddress[Slot5		]= 0x65AED3; ////
		gByteAddress[Slot6		]= 0x65AEF7; ////
		gByteAddress[Slot7		]= 0x65AF1B; ////
		gByteAddress[Slot8		]= 0x65AF3F; ////


		gByteAddress[BNETFLAG	]= 0x65B050; ////

		gByteAddress[GAMEBNET	]= 0x65B084; ////
		gByteAddress[MAPBNET	]= 0x65B0CD; ////
		gByteAddress[HPLAYER	]= 0x65B660; ////

		
	// un peu louche pour l'adresse suivante, beaucoup de référence
	// elle est utilisé pour plein de truc différent, a vérifier :
		gByteAddress[INGAME		]= 0x666100; //// including game lobby on bnet, but not game results

	// la c encore moins fiable pour les 2 qui suivent, aucune référence direct...
	// ca me plais pas, c'est de la mémore allouée dynamiquement
	// et t'a du bol pasque ca tombe toujours sur la meme adresse :)
		gByteAddress[GAME		]= 0x666138; ////
		gByteAddress[MAP		]= 0x666181; ////

	// la ca y'a l'air d'avoir des références, mais si jamais je me trompe
	// et que c'est dynamique ca plantera beaucoup
		gByteAddress[REALMINERAL]= 0x697EA4; ////
		gByteAddress[GAS		]= 0x697EA0; ////

	// attention, celle la j'ai pas trouvé donc je l'ai fait par calcul
		gByteAddress[SUPPLY_STR	]= 0x697DF8; ////
	}

}

//--------------------------------------------------------------------------------------------------------------

void BWMem::ReadBytes(HANDLE hProcess, char *adr, char *buffer, int size)
{
	DWORD read=0;
	if(!ReadProcessMemory(hProcess,adr,buffer,size,&read))
		memset(buffer,0,size);
}

//--------------------------------------------------------------------------------------------------------------

void BWMem::WriteBytes(HANDLE hProcess, char *adr, const char *buffer, int size)
{
	DWORD written=0;
	if(!WriteProcessMemory(hProcess,adr,(void*)buffer,size,&written))
		written=0;
}

//--------------------------------------------------------------------------------------------------------------

void BWMem::ReadPlayerDesc(HANDLE hProcess, BWrepPlayerInfo *player, int idx)
{
	ReadBytes(hProcess, (char*)(Addr(PlayerDesc)+idx*36), (char*)player, sizeof(BWrepPlayerInfo));
}

//--------------------------------------------------------------------------------------------------------------

static BWrepPlayerInfo player;

BWrepPlayerInfo *BWMem::_GetAnyPlayerDesc(HANDLE hProcess, const char *playerName)
{
	for(int i=0;i<8;i++)
	{
		ReadPlayerDesc(hProcess, &player, i);
		if(strcmp(playerName,player.m_name)==0)
			return &player;
	}
	return 0;
}

//--------------------------------------------------------------------------------------------------------------

BWrepPlayerInfo *BWMem::GetPlayerDesc(HANDLE hProcess)
{
	// read login
	char name[32];
	BWMem::ReadBytes(hProcess, (char*)Addr(LOGIN), name, sizeof(name));
	if(name[0]!=0)
		return _GetAnyPlayerDesc(hProcess, name);
	return 0;
}

//------------------------------------------------------------------------------------

// get game time (in tick unit, divide by 24 to get seconds)
unsigned long BWMem::GetTick(HANDLE hProcess)
{
	// read current tick
	unsigned long tick;
	ReadBytes(hProcess, (char*)Addr(TICK), (char*)&tick, sizeof(tick));
	return tick;
}

//------------------------------------------------------------------------------------

// get player mineral resources
unsigned long BWMem::GetMineral(HANDLE hProcess)
{
	unsigned long mineral;
	ReadBytes(hProcess, (char*)Addr(REALMINERAL), (char*)&mineral, sizeof(mineral));
	return mineral;
}

//------------------------------------------------------------------------------------

// get player gas resources
unsigned long BWMem::GetGas(HANDLE hProcess)
{
	unsigned long gas;
	ReadBytes(hProcess, (char*)Addr(GAS), (char*)&gas, sizeof(gas));
	return gas;
}

//------------------------------------------------------------------------------------

// get player supply
void BWMem::GetSupply(HANDLE hProcess, int *used, int *available)
{
	char sup[16];
	ReadBytes(hProcess, (char*)Addr(SUPPLY_STR), (char*)&sup, sizeof(sup));
	*used = atoi(&sup[1]);
	char *p=&sup[1];
	while(*p!='/' && p<&sup[16]) p++;
	*available = atoi(&p[1]);
}

//------------------------------------------------------------------------------------

// get replay file (if any)
const char *BWMem::GetReplay(HANDLE hProcess, char *repfile)
{
	// read replay file name if any
	repfile[0]=0;
	ReadBytes(hProcess, (char*)Addr(REPLAYFILE), repfile, 255);
	if(strlen(repfile)<=4 || stricmp(&repfile[strlen(repfile)-4],".rep")!=0) repfile[0]=0;
	return repfile;
}

//------------------------------------------------------------------------------------

static void _Invalidate(HWND hWndBW)
{
	CRect rect(10,200,200,290);

	::InvalidateRect(hWndBW,&rect,FALSE);

	/*
	HDC hdc = ::GetDC(0);
	CDC dc;
	dc.Attach(hdc);
	int oldmode=::SetBkMode(hdc,OPAQUE);
	COLORREF oldclr=::SetTextColor(hdc,RGB(255,255,255));
	COLORREF oldbkclr=::SetBkColor(hdc,RGB(0,0,0));
	dc.FillSolidRect(&rect,RGB(0,0,0));
	::SetBkColor(hdc,oldbkclr);
	::SetBkMode(hdc,oldmode);
	::SetTextColor(hdc,oldclr);
	dc.Detach();
	::ReleaseDC(0,hdc);
	*/
}

//------------------------------------------------------------------------------------

// display player message
void BWMem::DisplayPlayerMessage(HANDLE hProcess, HWND hWndBW, const char *msgtext, int durationS)
{
	// read current message index
	unsigned char index;
	ReadBytes(hProcess, (char*)Addr(MSGINDEX), (char*)&index, sizeof(index));

	// real index
	int realindex = (int)index;
	DWORD adr = Addr(MSGTEXT0)+realindex*218;
	DWORD adrt = Addr(MSGTICK0)+realindex*4;

	// write message time
	DWORD time = GetTickCount() + durationS*1000;
	WriteBytes(hProcess, (char*)adrt, (char*)&time, sizeof(time));

	// write message text
	WriteBytes(hProcess, (char*)adr, msgtext, strlen(msgtext)+1);

	// increment message index
	index = (index+1)%11;
	WriteBytes(hProcess, (char*)Addr(MSGINDEX), (char*)&index, sizeof(index));

	// invalidate screen
	Sleep(100);
	_Invalidate(hWndBW);
}

//------------------------------------------------------------------------------------

bool BWGameDetection::IsReplayInProgress(HANDLE hProcess)
{
	unsigned char flag=0;
	BWMem::ReadBytes(hProcess, (char*)BWMem::Addr(BWMem::INGAME), (char*)&flag, sizeof(flag));
	return m_bIsReplay && flag==1;
}

//------------------------------------------------------------------------------------

bool BWGameDetection::IsGameInProgress(HANDLE hProcess)
{
	return m_bGameInProgress;
}

//------------------------------------------------------------------------------------

// called every 250 ms
bool BWGameDetection::CheckEndOfGame(HANDLE hProcess)
{
	// for bnet games we can detect end of game from when total resources summary is displayed
	bool gameover=false;
	if(m_bIsBNET || m_bIsReplay)
	{
		// if total mineral has changed, game is over
		unsigned long mineral=0;
		BWMem::ReadBytes(hProcess, (char*)BWMem::Addr(BWMem::MineralEnd), (char*)&mineral, sizeof(mineral));
		if(m_lastGameMineralEnd!=mineral) {m_lastGameMineralEnd=mineral; gameover=true;m_bGameInProgress=false;}
	}

	// end of replay?
	if(m_bIsReplay && !IsReplayInProgress(hProcess))
		gameover=true;

	// F11 key, or game mineral end detected
	if(GetAsyncKeyState(VK_F11)&0x8000 || gameover)
	{
		// reset mineral
		unsigned long mineral=0;
		BWMem::WriteBytes(hProcess, (char*)BWMem::Addr(BWMem::REALMINERAL), (char*)&mineral, sizeof(mineral));

		// reset game info so we dont restart right away
		ResetGameInfo(hProcess);
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------------------------------

void BWGameDetection::ResetGameInfo(HANDLE hProcess)
{
	memset(m_startConditions,0,sizeof(m_startConditions));

	m_bGameInProgress = false;

	// read game time
	BWMem::ReadBytes(hProcess, (char*)BWMem::Addr(BWMem::GameTime), (char*)&m_gameTime, sizeof(m_gameTime));
}

//--------------------------------------------------------------------------------------------------------------

bool BWGameDetection::_bGameStarted() const
{
	for(int i=0;i<sizeof(m_startConditions)/sizeof(m_startConditions[0]);i++)
		if(!m_startConditions[i]) return false;
	return true;
}

//--------------------------------------------------------------------------------------------------------------

// returns true if game or replay is started
bool BWGameDetection::IsGameStarted(HANDLE hProcess)
{
	// read login
	BWMem::ReadBytes(hProcess, (char*)BWMem::Addr(BWMem::LOGIN), m_playerName, sizeof(m_playerName));
	if(m_playerName[0]!=0) m_startConditions[0]=true;

	// read game time
	unsigned long time=0;
	BWMem::ReadBytes(hProcess, (char*)BWMem::Addr(BWMem::GameTime), (char*)&time, sizeof(time));
	if(time!=m_gameTime && time<150) {m_gameTime=time; m_startConditions[1]=true;}

	// if game is started, try to figure out if it's a replay or not
	if(_bGameStarted())
	{
		// read bnet flag
		char flag[4];
		BWMem::ReadBytes(hProcess, (char*)BWMem::Addr(BWMem::BNETFLAG), flag, 4);
		m_bIsBNET = (strncmp(flag,"TENB",4)==0);

		// is it a replay ?
		unsigned char isReplay=0;
		BWMem::ReadBytes(hProcess, (char*)BWMem::Addr(BWMem::INGAME), (char*)&isReplay, sizeof(isReplay));
		m_bIsReplay = isReplay!=0;

		/*
		// get replay file (if any)
		char repfile[255];
		BWMem::GetReplay(hProcess, repfile);
		if(repfile[0]!=0)  // could be an earlier replay
		{
			// read player mineral several times
			unsigned long prevmineral = BWMem::GetMineral(hProcess);
			int count=0;
			for(int i=0;i<5 && count<3;i++)
			{
				unsigned long min = BWMem::GetMineral(hProcess);
				if(min==prevmineral) count++; else prevmineral=min;
				Sleep(50);
			}
			// if player mineral didnt change in 150 ms, it's a replay
			m_bIsReplay = (count==3);
		}  */

		// if this is not a replay, then it's a game
		m_bGameInProgress = !m_bIsReplay;

		// read current mineral end
		BWMem::ReadBytes(hProcess, (char*)BWMem::Addr(BWMem::MineralEnd), (char*)&m_lastGameMineralEnd, sizeof(m_lastGameMineralEnd));
	}

	return _bGameStarted();
}


//--------------------------------------------------------------------------------------------------------------

