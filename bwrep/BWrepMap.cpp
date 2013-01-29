//----------------------------------------------------------------------------------------------------
// Replay Map - jca (May 2003)
//----------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "BWrepMap.h"

#include "unpack.h"
#include <assert.h>

//------------------------------------------------------------------------------------------------------------

//
bool BWrepMap::DecodeMap(const unsigned char *buffer, int mapSize, int w, int h)
{
	const unsigned char *current = buffer;
	int read=0;

	// map dimensions
	m_mapWidth=w;
	m_mapHeight=h;

	// clear previous infos
	_Clear();

	// read block chain
	while(read<mapSize && m_sectionCount<MAXSECTION)
	{
		// extract block title
		char blockTitle[5];
		memcpy(blockTitle,current,4);
		blockTitle[4]=0;
		current+=4;

		// check for corruption
		if(blockTitle[0]==0) break;

		// extract block size
		unsigned long bsize = *((unsigned long*)current);
		current+=4;

		// check for corruption
		if(bsize>(unsigned long)mapSize) break;

		// init section block
		read+=8;
		m_sections[m_sectionCount].Init(blockTitle,bsize,current);

		// next block
		m_sectionCount++;
		current+=bsize;
		read+=bsize;
	}

	// keep pointer on buffer
	m_data = buffer;
	m_datasize = mapSize;

	return true;
}

//------------------------------------------------------------------------------------------------------------

void BWrepMap::_Clear() 
{
	m_sectionCount=0;
	m_datasize=0;

	// free data buffer
	if(m_data!=0) free((void*)m_data);
	m_data=0;
}

//------------------------------------------------------------------------------------------------------------

BWrepMap::~BWrepMap() 
{
	_Clear();
}

//------------------------------------------------------------------------------------------------------------

// find section by name
const BWrepMapSection* BWrepMap::GetSection(const char *name) const
{
	for(int i=0; i<m_sectionCount; i++)
	{
		if(_stricmp(name,m_sections[i].GetTitle())==0)
			return &m_sections[i];
	}
	return 0;
}

//------------------------------------------------------------------------------------------------------------

// get tile section info (2 bytes per map square)
const BWrepMapSection* BWrepMap::GetTileSection() const
{
	// depends on the replay file format
	const BWrepMapSection *tile= GetSection(SECTION_TILE);
	if(tile==0) tile = GetSection(SECTION_MTXM);
	if(tile==0) tile = GetSection(SECTION_MASK);
	return tile;
}

//------------------------------------------------------------------------------------------------------------
