#ifndef __bwmap_h
#define __bwmap_h

#include "memblock.h"
#include <assert.h>

class Replay;

//---------------------------------------------------------------------------------------------------

#define MAXPLAYERS 32
#define MAPEMPTY 0xFF
#define MAPMINERAL 0xFE
#define MAPACCESS(x,y) (x+y*m_width)
#define MAXHILIGHT 200
#define BUILD_CONSUMERATE	5
#define MOVE_CONSUMERATE	5
#define ISVALIDPLAYERID(pid) ((unsigned char)pid<MAPMINERAL)

//------=---------------------------------------------------------------------------------------------

class MapElem
{
public:
	static MapElem gEmpty;
	MapElem() {}
	MapElem(int bid, int pid) {m_buildingid=(unsigned char)bid; m_playerid=(unsigned char)pid;}
	MapElem(const MapElem& src) {m_buildingid=src.m_buildingid; m_playerid=src.m_playerid;}
	unsigned char m_buildingid;
	unsigned char m_playerid;
	bool IsEmpty() const {return m_buildingid==MAPEMPTY && m_playerid==MAPEMPTY;}
};

//---------------------------------------------------------------------------------------------------

class MapSurface
{
	MapElem *m_map;
	void _Create(int w,int h) {m_map= new MapElem[w*h]; memset(m_map,0,w*h*sizeof(MapElem));}
	bool _IsValid(int x, int y) const {return (x>=0 && x<m_width && y>=0 && y<m_height);}

protected:
	// map size
	int m_width;
	int m_height;

public:
	MapSurface(int w, int h) {m_width=w; m_height=h;_Create(w,h);}
	~MapSurface() {delete[]m_map;}

	// clear map
	void Clear(int val) {memset(m_map,val,m_width*m_height*sizeof(MapElem));}

	// set square
	void SetSquare(int x, int y, const MapElem& val) {if(_IsValid(x,y)) m_map[MAPACCESS(x,y)]=val;}

	// increment square
	void IncSquare(int x, int y, const MapElem& val) 
	{
		if(_IsValid(x,y)) 
		{
			m_map[MAPACCESS(x,y)].m_buildingid+=val.m_buildingid;
			m_map[MAPACCESS(x,y)].m_playerid+=val.m_playerid;
		}
	}

	// sum square content
	void Sum(MapElem& sum, unsigned __int64& unit) const
	{
		assert(m_width*m_height<=64); 
		unit=0;
		unsigned __int64 mask=1;
		for(int i=0;i<m_width*m_height;i++)
		{
			// count how many squares are "lighted"
			MapElem& elem = m_map[i];
			if(elem.m_buildingid>0) sum.m_buildingid++;
			if(elem.m_playerid>0) 
			{
				sum.m_playerid++; unit|=mask;
			}
			mask<<=1;
		}
	}

	// clear content for units only
	void ClearUnit() const
	{
		for(int i=0;i<m_width*m_height;i++)
		{
			MapElem& elem = m_map[i];
			elem.m_playerid=0;
		}
	}

	// return square at position
	MapElem* GetSquare(int x, int y) const {return _IsValid(x,y) ? &m_map[MAPACCESS(x,y)] : &MapElem::gEmpty;}

	// map size
	int GetWidth() const {return m_width;}
	int GetHeight() const {return m_height;}
};

//---------------------------------------------------------------------------------------------------

class ReplayMap	: public MapSurface
{
private:
	// associated tile map (if any)
	MapSurface m_tileset;

	COLORREF _GetColor(int i, int j, int options) const;

protected:
	// corresponding bitmap
	mutable HBITMAP m_hBitmap;

	// hilight for moves
	MapSurface m_maphi;
	bool m_useUnits;

	HBITMAP _CreateBitmap(int options) const ;

public:
	ReplayMap(int w, int h) :  MapSurface(w,h), m_maphi(w,h), m_tileset(w,h), m_hBitmap(0), m_useUnits(false) 
		{m_tileset.Clear(MAPEMPTY); Clear();}
	virtual ~ReplayMap() {if(m_hBitmap!=0) ::DeleteObject(m_hBitmap);}

	// options for bitmap
	enum {BUILDINGS_ON=1,MINERALS_ON=2};

	// clear map
	void Clear() {MapSurface::Clear(MAPEMPTY); if(m_hBitmap!=0) ::DeleteObject(m_hBitmap); m_hBitmap=0;}

	// set building squares
	void SetBuilding(int x, int y, int w, int h, const MapElem& val)
	{
		for(int j=0;j<h;j++)
			for(int i=0;i<w;i++)
				SetSquare(x+i,y+j,val);
	}

	// return building id at position
	const MapElem* GetBuilding(int x, int y) const {return GetSquare(x,y);}

	// associated tile map (if any)
	MapSurface* GetTileSet() {return &m_tileset;}

	// get player color
	static COLORREF GetPlayerColor(int idx);
};

//------------------------------------------------------------------------------------------------------------

class ReplayMapAction
{
private:
	unsigned char m_playerid;
	unsigned char m_highlight;
	unsigned char m_x;
	unsigned char m_y;
	unsigned char m_width;
	unsigned char m_height;
	unsigned long m_time;
public:
	ReplayMapAction(int x, int y, int w, int h, int pid, unsigned long time) :
	  m_x((unsigned char)x), m_y((unsigned char)y), m_playerid((unsigned char)pid), m_width((unsigned char)w), 
		  m_height((unsigned char)h), m_time(time) {Reset();}

	unsigned char Highlight() const {return m_highlight;}
	void Consume(int val) {if(m_highlight>0) {m_highlight-=min(m_highlight,(unsigned char)val);}}
	void Reset() {m_highlight=MAXHILIGHT;}

	unsigned long GetTime() const {return m_time;}
	unsigned char GetPlayerID() const {return m_playerid;}
	unsigned char GetX() const {return m_x;}
	unsigned char GetY() const {return m_y;}
	unsigned char GetWidth() const {return m_width;}
	unsigned char GetHeight() const {return m_height;}
};

//------------------------------------------------------------------------------------------------------------

class ReplayMapAnimated : public ReplayMap
{
private:
	// parent replay
	Replay *m_replay;

	// build actions
	MemoryBlock m_actions; // block with all ReplayMapAction
	int m_animidx;

	// building count per player
	int m_buildingCount[MAXPLAYERS];
	int m_peakBuildCount;

	// move actions
	MemoryBlock m_moves; // block with all ReplayMapAction
	int m_animidx2;

	// move count per player
	int m_moveCount[MAXPLAYERS];
	int m_peakMoveCount;

	// train units actions
	MemoryBlock m_units; // block with all ReplayMapAction
	int m_animidx3;

	// move count per player
	int m_unitCount[MAXPLAYERS];
	int m_peakUnitCount;

	// compute all peaks
	void _ComputePeaks();

public:
	ReplayMapAnimated(Replay *replay, int w, int h) : ReplayMap(w,h), m_replay(replay), 
		m_animidx(0), m_animidx2(0), m_animidx3(0) {m_useUnits=true;}
	~ReplayMapAnimated() {}

	// start animation
	void Start();

	// add build action
	void AddBuild(const ReplayMapAction *act) {m_actions.Add(act,sizeof(ReplayMapAction));}

	// add move action
	void AddMove(const ReplayMapAction *act) {m_moves.Add(act,sizeof(ReplayMapAction));}

	// add train unit action
	void AddUnit(const ReplayMapAction *act) {m_units.Add(act,sizeof(ReplayMapAction));}

	// build map at specific time
	HBITMAP BuildMap(unsigned long time, int options=BUILDINGS_ON|MINERALS_ON);

	// sort actions according to time
	void Sort();

	// move count per player
	int GetMoveCount(int i) {return i<MAXPLAYERS ? m_moveCount[i] : 0;}
	int GetPeakMoveCount() {return m_peakMoveCount;}

	// build count per player
	int GetBuildCount(int i) {return i<MAXPLAYERS ? m_buildingCount[i] : 0;}
	int GetPeakBuildCount() {return m_peakBuildCount;}

	// train units count per player
	int GetUnitCount(int i) {return i<MAXPLAYERS ? m_unitCount[i] : 0;}
	int GetPeakUnitCount() {return m_peakUnitCount;}
};

//------------------------------------------------------------------------------------------------------------

#endif
