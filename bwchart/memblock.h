// memblock.h : interface of the MemoryBlock class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __MEMBLOCK_H
#define __MEMBLOCK_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "heapperclass.h"

//--------------------------------------------------------------------------------------

#define INVALID_PTR 0xFFFFFFFF

class MemoryBlock : public CHeapPerInstance {
public:
	MemoryBlock(unsigned long res=65536) : m_pData(0), m_Size(0), m_Used(0), m_Resize(res), m_Count(0) {}
	~MemoryBlock() {if(m_pData) Free(m_pData);}
	unsigned long Add(const void *pData, unsigned long size, bool bInvert=false);
	void *GetPtr(unsigned long off) const {return m_pData+off;}
	void Clear() {if(m_pData) Free(m_pData); m_pData=0;m_Size=0, m_Used=0; m_Count=0;}
	unsigned long GetCount() const {return m_Count;}
	void RemoveAt(unsigned long off, unsigned long size, unsigned long count=1);
private:
	char *m_pData;
	unsigned long m_Size;
	unsigned long m_Used;
	unsigned long m_Resize;
	unsigned long m_Count;
};

//--------------------------------------------------------------------------------------

class XObArray : public CObArray {
public:
	~XObArray() {RemoveAll();}
	void RemoveAll() {for(int i=0; i<GetSize(); i++) delete GetAt(i); CObArray::RemoveAll();}
};

#endif 
