#include "stdafx.h"
#include "aka.h"
#include "bwdb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//------------------------------------------------------------

void Aka::Save(int nfile)
{
	m_bModified=false;

	// build comma separated list of akas
	CString akas;
	for(int i=0;i<m_akas.GetSize();i++)
	{
		if(i>0) akas+=",";
		akas+=m_akas.GetAt(i);
	}

	// write file entry
	BWChartDB::WriteEntry(nfile,m_mainName,"akas",akas);
}

//------------------------------------------------------------

static unsigned long _ComputeHashCode(const char *name)
{
	unsigned int sum=0;
	int len = min(4,(int)strlen(name));
	for(int i=0;i<len;i++)
	{
		char c = name[i];
		if(c!=' ') 
			sum+=tolower((unsigned char)c)*i;
	}
	return sum%MAXHASH;
}

//------------------------------------------------------------

// add player
void AkaList::AddPlayer(Aka *player)
{
	// add player in main list
	m_akas.Add(player);

	// update hash table with every name
	for(int i=0;i<player->GetAkaCount();i++)
	{
		AkaElement *elem = &m_hash[_ComputeHashCode(player->GetAka(i))];
		elem->AddElem(player, player->GetAka(i));
	}
}

//------------------------------------------------------------

// add player aka
void AkaList::AddAka(Aka *player, const char *newaka)
{
	// add aka into player's aka list
	if(!player->AddAka(newaka)) return;

	// update hash table
	AkaElement *elem = &m_hash[_ComputeHashCode(newaka)];
	elem->AddElem(player, newaka);
}


//------------------------------------------------------------

// find player
Aka* AkaList::FindPlayer(const char *mname, int *idx) const
{
	for(int i=0;i<GetPlayerCount();i++)
	{
		Aka *paka = GetPlayer(i);
		if(_stricmp(paka->MainName(),mname)==0)
		{
			if(idx!=0) *idx = i;
			return paka;
		}
	}
	if(idx!=0) *idx = -1;
	return 0;
}

//------------------------------------------------------------

// remove player
void AkaList::RemovePlayer(Aka *player)
{
	// find player
	int idx;
	AkaList::FindPlayer(player->MainName(), &idx);
	if(idx>=0) 
	{
		// remove from list
		m_akas.RemoveAt(idx);

		// remove from file
		BWChartDB::Delete(m_nfile,player->MainName(),"akas");

		// remove from hash table
		for(int i=0;i<player->GetAkaCount();i++)
		{
			AkaElement *elem = &m_hash[_ComputeHashCode(player->GetAka(i))];
			elem->RemoveElem(player,player->GetAka(i));
			//assert(GetAkaFromName(player->GetAka(i))==0);
		}
		// delete player
		delete player;
	}
}

//------------------------------------------------------------

// get aka from main name
const Aka* AkaList::GetAkaFromMainName(const char *mname) const
{
	Aka *ak = FindPlayer(mname, 0);
	return ak==0 ? 0 : ak;
}
								  
//------------------------------------------------------------

// get aka from any name
const Aka* AkaList::GetAkaFromName(const char *name) const
{
//	ASSERT(strcmp(name,"GG1-ElkY")!=0);

	const AkaElement *elem = &m_hash[_ComputeHashCode(name)];
	const Aka *aka = elem->Find(name);

	/*
	if(aka==0)
	{
		for(int i=0;i<m_akas.GetSize();i++)
		{
			Aka *ak = (Aka *)m_akas.GetAt(i);
			for(int j=0; j<ak->GetAkaCount(); j++)
			{
				if(strcmp(name,ak->GetAka(j))==0)
				{
					ASSERT(0);
					return aka;
				}
			}
		}
	} */

	return aka;
}

//------------------------------------------------------------

void AkaList::ProcessEntry(const char * section, const char *entry, const char *data, int percentage)
{
	// create new player
	Aka *player = new Aka(section);

	// add akas
	char buffer[2048];
	strcpy(buffer,BWChartDB::ConverFromHex(data));
	char *p=strtok(buffer,",");
	while(p!=0)
	{
		player->AddAka(p);
		p=strtok(0,",");
	}

	// add player in list
	AddPlayer(player);
}

//------------------------------------------------------------

// load list
void AkaList::Load()
{
	// clear current list
	m_akas.RemoveAll();

	// clear hash table
	for(int i=0;i<MAXHASH;i++)
		m_hash[i].DeleteElem();

	// load list from file
	BWChartDB::LoadFile(m_nfile);
}

//------------------------------------------------------------

bool Aka::AddAka(const char *name) 
{
	// make sure we dont already have that aka in the list
	for(int i=0;i<m_akas.GetSize();i++)
		if(m_akas.GetAt(i).CompareNoCase(name)==0) 
			return false;

	m_akas.Add(name); 
	m_bModified=true;
	return true;
}

//-------------------------------------------------------------------------------------------------------

// fill a combo with akalist content
void AkaList::FillCombo(CComboBox *combo, const char *regentry, const char *name)
{
	// fill combo
	combo->ResetContent();
	for(int i=0; i<GetPlayerCount(); i++)
	{
	 	// add player
		Aka *aka = GetPlayer(i);
		int idx = combo->AddString(aka->MainName());
		combo->SetItemData(idx,(DWORD)aka);
	}

	// select 
	int sel = name==0 ? AfxGetApp()->GetProfileInt("BWCHART_AKA",regentry,0) : combo->FindStringExact(0,name);

	// select player
	combo->SetCurSel(sel);
}

//------------------------------------------------------------
