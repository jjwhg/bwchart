// gradient.h : implementation file
//

#ifndef __GRADIENT_H
#define __GRADIENT_H

//----------------------------------------------------------------------------------------------------------

class Gradient
{
public:
	static void Fill(CDC *pDC, const CRect& rect, COLORREF rgb, COLORREF rgb2, ULONG mode=GRADIENT_FILL_RECT_H);
};

//----------------------------------------------------------------------------------------------------------

#endif
