// memoryBlock.cpp : implementation of the MemoryBlock class
//

#include "stdafx.h"
#include "memblock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------

unsigned long MemoryBlock::Add(const void *pData, unsigned long size, bool bInvert)
{
	unsigned long off;

	// if block not allocated yet
	if(m_pData==0)
	{
		// allocate initial block
		m_Size = max(m_Resize,size);
		m_pData = (char*)Alloc(m_Size);
		if(m_pData == 0) return INVALID_PTR;
	}
	else
	{
		// if we dont have enough space in current block
		if(m_Used+size > m_Size)
		{
			// resize block
			m_Size = max(m_Size+m_Resize,m_Used+size);
			m_pData = (char*)Realloc(m_pData, m_Size);
			if(m_pData == 0) return INVALID_PTR;
		}
	}
	// offset is at the end of current data
	off = m_Used;
	// increase counter of used space 
	m_Used += size;
	m_Count++;
	// copy data
	if(bInvert)
	{
		// insert before last object
		memcpy(m_pData+off,m_pData+off-size,size);
		memcpy(m_pData+off-size,pData,size);
	}
	else
	{
		// add at the end
		memcpy(m_pData+off,pData,size);
	}
	// return offset on data
	return off;
}

//---------------------------------------------------------------------------------------

void MemoryBlock::RemoveAt(unsigned long off, unsigned long size, unsigned long count)
{
	// move memory
	unsigned long toMove=m_Used-(off+size*count);
	if(toMove>0) memmove(m_pData+off,m_pData+off+size*count,toMove);

	// decrease counter of used space 
	m_Used -= size*count;
	m_Count-= count;
}
