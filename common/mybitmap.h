#ifndef __MYBITMAP_H
#define __MYBITMAP_H

class MyBitmap
{
private:
	HBITMAP m_hBitmap;
	int m_width;
	int m_height;
	COLORREF m_clrbkg;

	void _CreateBitmap(unsigned long *pixels, int width, int height);

public:
	// ctor
	MyBitmap();
	// dtor
	~MyBitmap();

	// create initial bitmap
	void CreateBitmap(unsigned long *pixels, int width, int height);

	// change bitmap bits
	void UpdatePixels(unsigned long *pixels, int width, int height);

	// paint image 
	void Paint(CDC* pDC, const CRect& rect, bool stretch=false, int dx=0, int dy=0);

	// paint image in center
	void PaintCenter(CDC* pDC, const CRect& rect);

	// tile image in bigger rect
	void Tile(CDC* pDC, const CRect& rect);

	// load from resource
	void LoadFromResouce(UINT id);

	// load from file
	void LoadFromFile(const char *path);

	// get pixels from bitmap
	unsigned long *GetBits(HDC hMemDC, HBITMAP hBitmap);

	// true if bitmap is loaded
	bool HasBitmap() const {return m_hBitmap!=0;} 

	// set background color
	void SetBkgColor(COLORREF clr) {m_clrbkg=clr;}

};

#endif
