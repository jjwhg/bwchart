#include "stdafx.h"
#include "bwmap.h"
#include "hsvrgb.h"
#include "replay.h"

#define MAXCLR 230
#define LOWCLR 60

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// player colors
COLORREF gPlayerColors[]=
{
	RGB(LOWCLR,MAXCLR,LOWCLR),
	RGB(LOWCLR,LOWCLR,MAXCLR),
	RGB(MAXCLR,LOWCLR,LOWCLR),
	RGB(MAXCLR,MAXCLR,0),
	RGB(MAXCLR,0,MAXCLR),
	RGB(0,MAXCLR,MAXCLR),
	RGB(MAXCLR,MAXCLR/2,0),
	RGB(MAXCLR,0,MAXCLR/2),
	RGB(MAXCLR/2,0,MAXCLR),
	RGB(MAXCLR,MAXCLR/2,MAXCLR/2),
	RGB(MAXCLR/2,MAXCLR/2,MAXCLR),
	RGB(MAXCLR,MAXCLR/2,MAXCLR)
};
#define MAXCOLORS (sizeof(gPlayerColors)/sizeof(gPlayerColors[0]))

MapElem MapElem::gEmpty(MAPEMPTY,MAPEMPTY);

//--------------------------------------------------------------------------------

static void _InitBitmapInfo(BITMAPINFO& info, int nWidth, int nHeight, int nBitCount)
{
	memset(&info, 0, sizeof(BITMAPINFO));
	info.bmiHeader.biSize = sizeof(info.bmiHeader);
	info.bmiHeader.biWidth = nWidth;
	info.bmiHeader.biHeight = nHeight;	// must be positive if we want a bottom-up DIB with origin in the lower-left
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = (unsigned short)nBitCount;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biSizeImage = 0;
	info.bmiHeader.biXPelsPerMeter = 0;
	info.bmiHeader.biYPelsPerMeter = 0;
	info.bmiHeader.biClrUsed = 0;
	info.bmiHeader.biClrImportant = 0;
}

//--------------------------------------------------------------------------------

// get player color
COLORREF ReplayMap::GetPlayerColor(int idx)
{
	return gPlayerColors[idx];
}

//--------------------------------------------------------------------------------

COLORREF ReplayMap::_GetColor(int i, int j, int options)	const
{
	COLORREF clr=0;
	bool bIsMove=false;

	// get map element
	MapElem* val = GetSquare(i,j);

	// if we have units
	if(m_useUnits)
	{
		// if we have unit at that position, draw unit instead of building
		MapElem* elem = m_maphi.GetSquare(i,j);
		if(elem->m_buildingid>0) {val=elem; bIsMove=true;}
	}

	// is it a mineral?
	if(val->m_playerid==MAPMINERAL)
	{
		// it's cyan
		if((options&MINERALS_ON)!=0) return RGB(0,255,255);
	}
	// if we have a building or unit on that pixel
	else if(val->m_playerid!=MAPEMPTY)
	{
		// use player color
		clr = gPlayerColors[val->m_playerid%MAXCOLORS];

		// if hilight is on
		if(val->m_buildingid>0)
		{
			// compute current color
			int h; double s,v;
			CHsvRgb::Rgb2Hsv(clr,&h,&s,&v);
			s = 1.0-0.8*((double)val->m_buildingid / (double)MAXHILIGHT);	 // from gray to color
			if(bIsMove || (options&BUILDINGS_ON)==0)
				v = 0.2 + 0.8*((double)val->m_buildingid / (double)MAXHILIGHT);	 // luminosity going down
			else
				v = 1.0;
			clr = CHsvRgb::Hsv2Rgb(h,s,v).rgb();
			return clr;
		}

		// hilight is 0 but building should be on
		if((options&BUILDINGS_ON)!=0 && !bIsMove) return clr;
	}

	// display tile in shades of gray
	const MapElem* tile = m_tileset.GetSquare(i,j);
	if(tile->m_buildingid!=MAPEMPTY) clr=RGB(tile->m_buildingid/2,tile->m_buildingid/2,tile->m_buildingid/2);
	else clr = 0;

	return clr;
}

//--------------------------------------------------------------------------------

HBITMAP ReplayMap::_CreateBitmap(int options) const 
{
	HBITMAP hOldBitmap;

	// invalid sizes?
	if(m_width*m_height==0) return 0;

	// destroy previous bitmap
	if(m_hBitmap!=0) ::DeleteObject(m_hBitmap);

	//  create a DC for the screen and create a memory DC compatible to screen DC
	HDC hScrDC = ::GetDC(0);	// Get a DC on the main display
	HDC hMemDC = ::CreateCompatibleDC(hScrDC);

	// create a bitmap compatible with the screen DC (one pixel per map quare)
	m_hBitmap = ::CreateCompatibleBitmap(hScrDC, m_width,m_height);
	if (m_hBitmap==0) goto Error;

	// select new bitmap into memory DC
	hOldBitmap = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);

	// compute pixel correspondence
	{
	char *pBuf = new char[m_width*m_height*32/sizeof(char)];
	DWORD *pix = (DWORD*)pBuf;
	COLORREF clr;
	for(int j=0;j<m_height;j++)
		for(int i=0;i<m_width;i++)
		{
			// get map element color
			clr = _GetColor(i, j, options);

			// init pixel color
			*(((unsigned char*)pix)+0)= *(((unsigned char*)&clr)+2);
			*(((unsigned char*)pix)+1)= *(((unsigned char*)&clr)+1);
			*(((unsigned char*)pix)+2)= *(((unsigned char*)&clr)+0);
			*(((unsigned char*)pix)+3)= *(((unsigned char*)&clr)+3);
			pix++;
		}

	// set pixels bit 
	BITMAPINFO Info;
	_InitBitmapInfo(Info, m_width, -m_height,32); // negative height means top-bottom
	::SetDIBits(hMemDC,m_hBitmap,0,m_height,pBuf,&Info,DIB_RGB_COLORS);
	delete[] pBuf;
	}
	
	//  select old bitmap back into memory DC and get handle to bitmap of the screen
	::SelectObject(hMemDC, hOldBitmap);

Error:
	// clean up
	::ReleaseDC(0,hScrDC);
	::DeleteDC(hMemDC);
	return m_hBitmap;
}

//--------------------------------------------------------------------------------

// start animation
void ReplayMapAnimated::Start()
{
	// clear map
	Clear();

	// compute action peaks and reset all counters
	_ComputePeaks();

	// reset actions
	m_animidx=0;
	m_animidx2=0;
	m_animidx3=0;
	for(int i=0; i<(int)m_actions.GetCount(); i++)
	{
		ReplayMapAction *act = (ReplayMapAction*)m_actions.GetPtr(i*sizeof(ReplayMapAction));
		act->Reset();
	}
	for(int i=0; i<(int)m_moves.GetCount(); i++)
	{
		ReplayMapAction *act = (ReplayMapAction*)m_moves.GetPtr(i*sizeof(ReplayMapAction));
		act->Reset();
	}
	for(int i=0; i<(int)m_units.GetCount(); i++)
	{
		ReplayMapAction *act = (ReplayMapAction*)m_units.GetPtr(i*sizeof(ReplayMapAction));
		act->Reset();
	}
}

//--------------------------------------------------------------------------------

void ReplayMapAnimated::_ComputePeaks()
{
	// reset building/move count per player
	memset(m_buildingCount,0,sizeof(m_buildingCount));
	memset(m_moveCount,0,sizeof(m_moveCount));
	memset(m_unitCount,0,sizeof(m_moveCount));
	m_peakBuildCount=0;
	m_peakMoveCount=0;
	m_peakUnitCount=0;

	// count buildings
	for(int i=0; i<(int)m_actions.GetCount(); i++)
	{
		// get action
		ReplayMapAction *act = (ReplayMapAction*)m_actions.GetPtr(i*sizeof(ReplayMapAction));
		if(act->GetPlayerID()<MAXPLAYERS)
		{
			m_buildingCount[act->GetPlayerID()]++;
			if(m_buildingCount[act->GetPlayerID()] > m_peakBuildCount) m_peakBuildCount=m_buildingCount[act->GetPlayerID()];
		}
	}

	// count move actions
	for(int i=0; i<(int)m_moves.GetCount(); i++)
	{
		// get action
		ReplayMapAction *act = (ReplayMapAction*)m_moves.GetPtr(i*sizeof(ReplayMapAction));
		if(act->GetPlayerID()<MAXPLAYERS)
		{
			m_moveCount[act->GetPlayerID()]++;
			if(m_moveCount[act->GetPlayerID()] > m_peakMoveCount) m_peakMoveCount=m_moveCount[act->GetPlayerID()];
		}
	}

	// count train units actions
	for(int i=0; i<(int)m_units.GetCount(); i++)
	{
		// get action
		ReplayMapAction *act = (ReplayMapAction*)m_units.GetPtr(i*sizeof(ReplayMapAction));
		if(act->GetPlayerID()<MAXPLAYERS)
		{
			m_unitCount[act->GetPlayerID()]++;
			if(m_unitCount[act->GetPlayerID()] > m_peakUnitCount) m_peakUnitCount=m_unitCount[act->GetPlayerID()];
		}
	}

	// reset building/move count per player
	memset(m_buildingCount,0,sizeof(m_buildingCount));
	memset(m_moveCount,0,sizeof(m_moveCount));
	memset(m_unitCount,0,sizeof(m_unitCount));
}

//--------------------------------------------------------------------------------

// build map at specific time
HBITMAP ReplayMapAnimated::BuildMap(unsigned long time, int options)
{
	// update build actions
	for(int i=m_animidx; i<(int)m_actions.GetCount(); i++)
	{
		// get action
		ReplayMapAction *act = (ReplayMapAction*)m_actions.GetPtr(i*sizeof(ReplayMapAction));

		// if player is disabled, ignore action
		if(ISVALIDPLAYERID(act->GetPlayerID()))
		{
			const ReplayEvtList * list = m_replay->GetEvtListFromPlayerID(act->GetPlayerID());
			if(!list->IsEnabled()) continue;
		}

		// action behind current time?
		if(act->GetTime()>time) break;

		// update map pixels
		SetBuilding(act->GetX(),act->GetY(),act->GetWidth(),act->GetHeight(),MapElem(act->Highlight(),act->GetPlayerID()));

		// action hilight is over?
		if(act->Highlight()==0) m_animidx=i;
		else act->Consume(BUILD_CONSUMERATE);

		// update buidling count
		if(act->Highlight()==MAXHILIGHT-BUILD_CONSUMERATE && act->GetPlayerID()<MAXPLAYERS)
			m_buildingCount[act->GetPlayerID()]++;
	}

	// update move actions
	m_maphi.Clear(0);
	for(int i=m_animidx2; i<(int)m_moves.GetCount(); i++)
	{
		// get action
		ReplayMapAction *act = (ReplayMapAction*)m_moves.GetPtr(i*sizeof(ReplayMapAction));

		// if player is disabled, ignore action
		if(ISVALIDPLAYERID(act->GetPlayerID()))
		{
			const ReplayEvtList * list = m_replay->GetEvtListFromPlayerID(act->GetPlayerID());
			if(!list->IsEnabled()) continue;
		}
		
		// action behind current time?
		if(act->GetTime()>time) break;

		// action hilight is over?
		if(act->Highlight()==0) m_animidx2=i;
		else 
		{
			// update move map pixels
			m_maphi.SetSquare(act->GetX(),act->GetY(), MapElem(act->Highlight(),act->GetPlayerID()));

			// change hilight
			act->Consume(MOVE_CONSUMERATE);

			// update buidling count
			if(act->Highlight()==MAXHILIGHT-MOVE_CONSUMERATE && act->GetPlayerID()<MAXPLAYERS)
				m_moveCount[act->GetPlayerID()]++;
		}
	}

	// update train units actions
	for(int i=m_animidx3; i<(int)m_units.GetCount(); i++)
	{
		// get action
		ReplayMapAction *act = (ReplayMapAction*)m_units.GetPtr(i*sizeof(ReplayMapAction));

		// if player is disabled, ignore action
		if(ISVALIDPLAYERID(act->GetPlayerID()))
		{
			const ReplayEvtList * list = m_replay->GetEvtListFromPlayerID(act->GetPlayerID());
			if(!list->IsEnabled()) continue;
		}
		
		// action behind current time?
		if(act->GetTime()>time) break;

		// action hilight is over?
		if(act->Highlight()==0) m_animidx3=i;
		else 
		{
			// change hilight
			act->Consume(MOVE_CONSUMERATE);

			// update units count
			if(act->Highlight()==MAXHILIGHT-MOVE_CONSUMERATE && act->GetPlayerID()<MAXPLAYERS)
				m_unitCount[act->GetPlayerID()]++;
		}
	}

	// rebuild bitmap
	_CreateBitmap(options);
	return m_hBitmap;
}

//--------------------------------------------------------------------------------

int _compare( const void *arg1, const void *arg2 )
{
	ReplayMapAction *act1 = (ReplayMapAction*)arg1;
	ReplayMapAction *act2 = (ReplayMapAction*)arg2;
	if(act1->GetTime()<act2->GetTime()) return 1;
	if(act1->GetTime()<act2->GetTime()) return -1;
	return 0;
}

// sort actions according to time
void ReplayMapAnimated::Sort()
{
	if(m_actions.GetCount()>0) qsort(m_actions.GetPtr(0),m_actions.GetCount(),sizeof(ReplayMapAction),_compare);
	if(m_moves.GetCount()>0) qsort(m_moves.GetPtr(0),m_moves.GetCount(),sizeof(ReplayMapAction),_compare);
	if(m_units.GetCount()>0) qsort(m_units.GetPtr(0),m_units.GetCount(),sizeof(ReplayMapAction),_compare);
}

//--------------------------------------------------------------------------------
