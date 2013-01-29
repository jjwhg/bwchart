#ifndef __BOTREE_H
#define __BOTREE_H

#include"memblock.h"
#include"bwrepgamedata.h"
#include<assert.h>

class BONodeList;

//------------------------------------------------------------------------

class IBOLister
{
public:
	virtual void AddBO(BONodeList* bo, int count)=0;
};

//------------------------------------------------------------------------

class BONode : public CObject
{
private:
	// object/upg/res id
	unsigned char m_content;
	unsigned char m_type;

	//---information used in the tree

	// leaf count
	unsigned short m_count;
	// parent
	BONode *m_parent;
	// children	(array of BONode*)
	XObArray m_children;

public:
	//ctor
	BONode(BONode *parent, int content, int type) : 
		m_count(1), m_parent(parent), m_content((unsigned char)content), m_type((unsigned char )type) 
		{}
	BONode(const CString& str) :m_parent(0), m_count(1) {FromString(str);}
	BONode(const BONode& src) : m_count(1), m_parent(0)
	{
		m_content = src.m_content;
		m_type = src.m_type;
		assert(GetName()!=0);
	}

	// shall we exclude a node?
	bool Exclude(int options) const;

	bool operator ==(const BONode& src) const {return src.GetADN() == GetADN();}

	unsigned short GetADN() const {unsigned short adn=m_type; adn<<=8; adn|=m_content; return adn;}
	int GetContent() const {return (int)m_content;}
	enum {BLD,UNIT,UPG,RES,TERMINATOR};
	int GetType() const {return (int)m_type;}
	int GetCount() const {return (int)m_count;}
	BONode * GetParent() const {return m_parent;}
	int GetChildrenCount() const {return m_children.GetSize();}
	BONode * GetChild(int i) {return (BONode*)m_children[i];}
	bool IsLeaf() const {return GetChildrenCount()==0;}
	void IncRef() {m_count++;}

	BONode * FindChild(const BONode& node, bool add); 

	void GoDown(IBOLister *lister, BONodeList* bo);

	// get object name
	const char *GetName() const; 

	void ToString(CString& str, bool readable=false) const;
	void FromString(const CString& str);

};

//------------------------------------------------------------------------

class BONodeList : public CObject
{
private:
	XObArray m_bo;

public:
	//ctor
	BONodeList() {}
	BONodeList(const CString& str) {FromString(str);}

	BONodeList& operator = (const BONodeList& src);

	void AddNode(BONode *node) {m_bo.Add(node);}
	int GetCount() const {return m_bo.GetSize();}
	BONode *GetNode(int i) const {return (BONode *)m_bo[i];}
	void RemoveLastNode() {m_bo.RemoveAt(m_bo.GetSize()-1);}
	void RemoveAll() {m_bo.RemoveAll();}

	// convert to string
	void ToString(CString& str, bool readable=false, int options=-1);
	void FromString(const CString& str);
};

//------------------------------------------------------------------------

class BOTree : public IBOLister
{
private:
	BONode m_root;

public:
	BOTree() : m_root(0,0,0) {}

	// add build order to the tree
	enum {BUILDING=1,UNIT=2,RESEARCH=4,UPGRADE=8,DEPOTS=16,ALL=255};
	void AddBO(const BONodeList& bo, int options=ALL, int maxnode=-1);

	// list all unique build orders
	void ListUniqueBos(IBOLister *lister);

	// for debugging
	virtual void AddBO(BONodeList* bo, int count);
};

#endif

