#ifndef __AKA_H
#define __AKA_H

#include "bwdb.h"
#include "memblock.h"
#include <assert.h>

#define MAXHASH 256

//------------------------------------------------------------

class Aka : public CObject
{
private:
	CString m_mainName;
	CStringArray m_akas;
	bool m_bModified;
public:
	Aka(const char *name) : m_mainName(name), m_bModified(false) {}
	Aka(const Aka& src)
	{
		m_mainName = src.m_mainName;
		m_bModified = false;
		m_akas.Copy(src.m_akas);
	}
	const char *MainName() const {return m_mainName;}
	void Clear() {m_akas.RemoveAll();}
	bool AddAka(const char *name);
	int GetAkaCount() const {return m_akas.GetSize();}
	const char *GetAka(int i) const {return m_akas.GetAt(i);}
	void Save(int nfile);
};

//------------------------------------------------------------

class AkaElement
{
private:
	CString m_name;
	Aka *m_pAka;
	AkaElement *m_pNext;
public:
	AkaElement() : m_pAka(0), m_pNext(0) {}
	~AkaElement() {if(m_pNext) delete m_pNext;}
	void Clear() {m_pAka=0, m_pNext=0;}

	void AddElem(Aka *aka, const char *name)
	{
		if(m_pAka==0) {m_pAka=aka; m_name=name; return;}
		if(m_pNext==0) m_pNext=new AkaElement();
		m_pNext->AddElem(aka,name);
	}
	void RemoveElem(Aka *player, const char *name)
	{
		if(m_name.CompareNoCase(name)==0 && player==m_pAka)
		{
			if(m_pNext==0) {m_pAka=0; m_name=""; return;}
			m_pAka = m_pNext->m_pAka;
			m_name = m_pNext->m_name;
			AkaElement *oldNext = m_pNext;
			m_pNext = m_pNext->m_pNext;
			oldNext->m_pNext=0;
			delete oldNext;
			return;
		}
		else if(m_pNext!=0)
		{
			m_pNext->RemoveElem(player,name);
			return;
		}
		assert(0);
	}
	void DeleteElem()
	{
		if(m_pNext!=0) m_pNext->DeleteElem();
		m_pAka=0;
		m_pNext=0;
	}
	Aka* Find(const char *name) const
	{
		if(m_name.CompareNoCase(name)==0) return m_pAka;
		if(m_pNext!=0) return m_pNext->Find(name);
		return 0;
	}
};

//------------------------------------------------------------

class AkaList : public BWChartDB
{
private:
	// akas list
	XObArray m_akas;

	// hash-table
	AkaElement m_hash[MAXHASH];

	// data file
	int m_nfile;

public:
	AkaList(int nfile) : m_nfile(nfile) {for(int i=0;i<MAXHASH;i++) m_hash[i].Clear();}

	// get player count
	int GetPlayerCount() const {return m_akas.GetSize();}

	// get player
	Aka *GetPlayer(int i) const {return (Aka *)m_akas.GetAt(i);}

	// find player
	Aka* FindPlayer(const char *mname, int *idx) const;

	// add player
	void AddPlayer(Aka *player);

	// add player aka
	void AddAka(Aka *player, const char *newaka);

	// remove player
	void RemovePlayer(Aka *player);

	// get aka from main name
	const Aka* GetAkaFromMainName(const char *mname) const;

	// get aka from any name
	const Aka* GetAkaFromName(const char *name) const;

	// load list
	void Load();
	void ProcessEntry(const char * section, const char *entry, const char *data, int percentage);

	// sake player info
	void Save(Aka *player) {player->Save(m_nfile);}

	// fill a combo with akalist content
	void FillCombo(CComboBox *combo, const char *regentry, const char *name=0);
};

#endif
