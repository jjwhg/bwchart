// bitmap.cpp : implementation file
//

#include "stdafx.h"
#include "mybitmap.h"

//--------------------------------------------------------------------------------

//ctor
MyBitmap::MyBitmap() : m_hBitmap(0), m_width(0), m_height(0)
{
	m_clrbkg = RGB(0,0,0);
}

//--------------------------------------------------------------------------------

//dtor
MyBitmap::~MyBitmap()
{
	// delete previous bitmap if any
	if(m_hBitmap!=0) ::DeleteObject(m_hBitmap);
}

//--------------------------------------------------------------------------------

// create initial bitmap
void MyBitmap::CreateBitmap(unsigned long *pixels, int width, int height)
{
	_CreateBitmap(pixels, width, height);
}

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

// get pixels from bitmap
unsigned long *MyBitmap::GetBits(HDC hMemDC, HBITMAP hBitmap)
{
	unsigned long *buffer = new unsigned long[m_width*m_height];
	BITMAPINFO Info;
	_InitBitmapInfo(Info, m_width, m_height,32);
	::GetDIBits(hMemDC,hBitmap,0,m_height,buffer,(BITMAPINFO*)&Info,DIB_RGB_COLORS);
	return buffer;
}

//--------------------------------------------------------------------------------

// create initial bitmap
void MyBitmap::UpdatePixels(unsigned long *pixels, int width, int height)
{
	// if bitmap was never built or bitmap dimension changed 
	if(m_hBitmap==0 || width!=m_width || height!=m_height) 
	{
		// create a news bitmap
		_CreateBitmap(pixels, width, height);
		return;
	}

	//  create a DC for the screen and create a memory DC compatible to screen DC
	HDC hScrDC = ::GetDC(0);	// Get a DC on the main display
	HDC hMemDC = ::CreateCompatibleDC(hScrDC);

	// select new bitmap into memory DC
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);

	// set pixels bit 
	BITMAPINFO Info;
	_InitBitmapInfo(Info, width, height,32); // negative height means top-bottom
	::SetDIBits(hMemDC,m_hBitmap,0,height,pixels,&Info,DIB_RGB_COLORS);
	
	//  select old bitmap back into memory DC and get handle to bitmap of the screen
	::SelectObject(hMemDC, hOldBitmap);

	// clean up
	::ReleaseDC(0,hScrDC);
	::DeleteDC(hMemDC);
}

//--------------------------------------------------------------------------------
				  
// pixels = array of 32 bits RGB pixel (order of bytes in memory is BGRA where A should be 0, use the RGB macro)
// width  = width of the image in pixels
// height = heigh of the image in pixels
//
void MyBitmap::_CreateBitmap(unsigned long *pixels, int width, int height)
{
	HBITMAP hOldBitmap;

	// save bitmap dimensions
	m_width=width;
	m_height=height;

	// delete previous bitmap if any
	if(m_hBitmap!=0) ::DeleteObject(m_hBitmap);

	//  create a DC for the screen and create a memory DC compatible to screen DC
	HDC hScrDC = ::GetDC(0);	// Get a DC on the main display
	HDC hMemDC = ::CreateCompatibleDC(hScrDC);

	// create a bitmap compatible with the screen DC (one pixel per map quare)
	m_hBitmap = ::CreateCompatibleBitmap(hScrDC, width,height);
	if (m_hBitmap==0) goto Error;

	// select new bitmap into memory DC
	hOldBitmap = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);

	// set pixels bit 
	BITMAPINFO Info;
	_InitBitmapInfo(Info, width, height,32); // negative height means top-bottom
	::SetDIBits(hMemDC,m_hBitmap,0,height,pixels,&Info,DIB_RGB_COLORS);
	
	//  select old bitmap back into memory DC and get handle to bitmap of the screen
	::SelectObject(hMemDC, hOldBitmap);

Error:
	// clean up
	::ReleaseDC(0,hScrDC);
	::DeleteDC(hMemDC);
}

//--------------------------------------------------------------------------------

void MyBitmap::PaintCenter(CDC* pDC, const CRect& rect)
{
	int x = (rect.left+rect.right)/2 - m_width/2;
	int y = (rect.top+rect.bottom)/2 - m_height/2;
	CRect centered(x,y,x+m_width,y+m_height);
	CRect centeredopt;
	centeredopt.IntersectRect(&centered,&rect);
	if(centeredopt.left>rect.left || centeredopt.top>rect.top)
		pDC->FillSolidRect(&rect,m_clrbkg);
	Paint(pDC, centeredopt, false);
}

//--------------------------------------------------------------------------------

void MyBitmap::Paint(CDC* pDC, const CRect& rect, bool stretch, int dx, int dy)
{
	// if no bitmap, fill with black
	if(m_hBitmap==0) 
	{
		pDC->FillSolidRect(&rect,m_clrbkg);
		return;
	}

	// create compatible dc			
	HDC hMemDC = ::CreateCompatibleDC(pDC->GetSafeHdc());

	// select bitmap into memory DC
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);

	if(stretch && (m_width!= rect.Width() || m_height!=rect.Height()))
	{
		// stretch blit image
		//::SetStretchBltMode(pDC->GetSafeHdc(),COLORONCOLOR);
		::SetStretchBltMode(pDC->GetSafeHdc(),HALFTONE); 
		::SetBrushOrgEx(pDC->GetSafeHdc(),0,0,NULL);
		::StretchBlt(pDC->GetSafeHdc(),rect.left,rect.top,rect.Width(),rect.Height(),hMemDC,dx,dy,m_width,m_height,SRCCOPY);
	}
	else
	{
		// blit image
		int width = min(m_width,rect.Width());
		int height = min(m_height,rect.Height());
		::BitBlt(pDC->GetSafeHdc(),rect.left,rect.top,width,height,hMemDC,dx,dy,SRCCOPY);
	}

	//  select old bitmap back into memory DC and get handle to bitmap of the screen
	::SelectObject(hMemDC, hOldBitmap);

	// clean up
	::DeleteDC(hMemDC);
}

//--------------------------------------------------------------------------------

// tile image in bigger rect
void MyBitmap::Tile(CDC* pDC, const CRect& rect)
{
	if(m_width==0 || m_height==0) return;

	int nwidth = 1+rect.Width()/m_width;
	int nheight = 1+rect.Height()/m_height;
	CRect rectPaint;
	for(int i=0;i<nwidth;i++)
	{
		for(int j=0;j<nheight;j++)
		{
			rectPaint.left = rect.left + i*m_width;
			rectPaint.right = rectPaint.left + m_width;
			rectPaint.top = rect.top + j*m_height;
			rectPaint.bottom = rectPaint.top + m_height;
			CRect rectbmp;
			rectbmp.IntersectRect(&rectPaint,&rect);
			Paint(pDC,rectbmp,false);
		}
	}
}

//--------------------------------------------------------------------------------

void MyBitmap::LoadFromResouce(UINT id)
{
	// delete previous bitmap if any
	if(m_hBitmap!=0) ::DeleteObject(m_hBitmap);

	// load from resource
	m_hBitmap = (HBITMAP)LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(id),IMAGE_BITMAP,0,0,LR_DEFAULTCOLOR);

	// if we have a bitmap loaded
	if(m_hBitmap !=0)
	{
		// get its size
		BITMAP info;
		GetObject(m_hBitmap, sizeof(info), &info);
		m_width = info.bmWidth;
		m_height=info.bmHeight;
	}
}

//--------------------------------------------------------------------------------

void MyBitmap::LoadFromFile(const char *path)
{
	// delete previous bitmap if any
	if(m_hBitmap!=0) ::DeleteObject(m_hBitmap);
	m_hBitmap = 0;

	//empty path?
	if(path[0]==0) return;

     // Use LoadImage() to get the image loaded into a DIBSection
     m_hBitmap = (HBITMAP)LoadImage( NULL, path, IMAGE_BITMAP, 0, 0,
                 LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE );

	 	// if we have a bitmap loaded
	if(m_hBitmap !=0)
	{
		// get its size
		BITMAP info;
		GetObject(m_hBitmap, sizeof(info), &info);
		m_width = info.bmWidth;
		m_height=info.bmHeight;
	}
}

//--------------------------------------------------------------------------------
