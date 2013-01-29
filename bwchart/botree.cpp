#include"stdafx.h"
#include"botree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BONode * BONode::FindChild(const BONode& node, bool add) 
{
	// find existing node
	for(int i=0;i< GetChildrenCount();i++)
	{
		BONode *child = GetChild(i);
		if(*child == node) 
		{
			if(child->GetType()==TERMINATOR)
				child->IncRef(); 
			return child;
		}
	}

	// if not found and we must add it
	if(add)
	{
		// create new node
		BONode *newchild = new BONode(this,node.GetContent(),node.GetType());
		m_children.Add(newchild);
		return newchild;
	}
	return 0;
}

void BONode::GoDown(IBOLister *lister, BONodeList* bo)
{
	// for each node at current level
	for(int i=0;i<GetChildrenCount();i++)
	{
		BONode *child = GetChild(i);
		
		// if child is a leaf
		if(child->IsLeaf()) 
		{
			// it should be a terminator too
			assert(child->GetType()==TERMINATOR);
			// register another unique bo
			lister->AddBO(bo,child->GetCount());
		}
		else 
		{
			// add node in BO
			bo->AddNode(child);
			// go down one level
			child->GoDown(lister,bo);
			// remove node from BO
			bo->RemoveLastNode();
		}
	}
}


// get object name
const char *BONode::GetName() const 
{
	assert(m_type!=TERMINATOR);
	return (m_type==BLD || m_type==UNIT) ? BWrepGameData::GetObjectNameFromID(m_content):
		   m_type==RES ? BWrepGameData::GetResearchNameFromID(m_content):
			             BWrepGameData::GetUpgradeNameFromID(m_content);
}

void BONode::ToString(CString& str, bool readable) const
{
	assert(m_type!=TERMINATOR);
	if(!readable)
	{
		str.Format("%04X",GetADN());
		assert(GetName()!=0);
	}
	else
	{
		if(m_type==UPG)
			str.Format("U:%s",GetName());
		else if(m_type==RES)
			str.Format("R:%s",GetName());
		else
			str.Format("%s",GetName());
	}
}

void BONode::FromString(const CString& str)
{
	int val;
	sscanf((const char*)str,"%x",&val);
	m_content = val & 0x00FF;
	m_type = ((val & 0xFF00)>>8);
	m_count = 1;
	assert(GetName()!=0);
}

// shall we exclude a node?
bool BONode::Exclude(int options) const
{
	if(options==-1) return false;

	// shall we skip it?
	if((options&BOTree::BUILDING)==0 && GetType()==BONode::BLD) return true;
	if((options&BOTree::UNIT)==0 && GetType()==BONode::UNIT) return true;
	if((options&BOTree::RESEARCH)==0 && GetType()==BONode::RES) return true;
	if((options&BOTree::UPGRADE)==0 && GetType()==BONode::UPG) return true;
	if((options&BOTree::DEPOTS)==0) 
	{
		if(	(GetType()==BONode::BLD && GetContent() == BWrepGameData::OBJ_SUPPLYDEPOT) ||
			(GetType()==BONode::BLD && GetContent() == BWrepGameData::OBJ_PYLON) ||
			(GetType()==BONode::UNIT && GetContent() == BWrepGameData::OBJ_OVERLORD))
			return true;
	}

	return false;
}

//------------------------------------------------------------------------

// convert to string
// format: <node1><node2><node3>
void BONodeList::ToString(CString& str, bool readable, int options)
{
	str="";
	CString strnode;
	for(int i=0;i<GetCount();i++)
	{
		BONode *node = GetNode(i);

		// shall we skip it?
		if(node->Exclude(options)) continue;

		node->ToString(strnode,readable);
		if(readable && !str.IsEmpty()) str+=CString(" + ");
		str+=strnode; 
	}
}

// init from string
void BONodeList::FromString(const CString& str)
{
	RemoveAll();
	int nodecount=str.GetLength()/4;
	for(int i=0;i<nodecount;i++)
	{
		BONode *node = new BONode(str.Mid(4*i,4));
		AddNode(node);
	}
}

// copy operator
BONodeList& BONodeList::operator = (const BONodeList& src)
{
	RemoveAll();
	for(int i=0;i<src.GetCount();i++)
	{
		BONode *node = src.GetNode(i);
		AddNode(new BONode(*node));
	}
	return *this;
}

//------------------------------------------------------------------------

// add a build order to the tree
void BOTree::AddBO(const BONodeList& bo, int options, int maxnode)
{
	// for each node in the BO
	BONode *parent =&m_root;
	for(int i=0,j=0;i<bo.GetCount();i++)
	{
		// get node
		BONode *node =bo.GetNode(i);

		// shall we skip it?
		if(node->Exclude(options)) continue;

		// find it or add it the children list at the current level
		parent = parent->FindChild(*node, true);
		if(maxnode>0 && ++j==maxnode) break;
	}

	// add terminating node
	BONode term(0,0,BONode::TERMINATOR);
	parent->FindChild(term, true);

#ifndef NDEBUG
	#if 0
	ListUniqueBos(this);
	OutputDebugString("-----------------------\r\n");
	#endif
#endif
}

//------------------------------------------------------------------------

void BOTree::ListUniqueBos(IBOLister *lister)
{
	BONodeList bo;
	m_root.GoDown(lister, &bo);
}

// for debugging
void BOTree::AddBO(BONodeList* bo, int count)
{
	CString str;
	bo->ToString(str,true);
	CString str2;
	str2.Format("%d %s\r\n",count,(const char*)str);
	OutputDebugString(str2);
}

//------------------------------------------------------------------------

