// gradient.cpp : implementation file
//

#include "stdafx.h"
#include "gradient.h"


//----------------------------------------------------------------------------------------------------------

void Gradient::Fill(CDC *pDC, const CRect& rect, COLORREF rgb, COLORREF rgb2, ULONG mode)
{
	TRIVERTEX        vert[2] ;

	// right edge
	vert [1] .x      = rect.right;
	vert [1] .y      = rect.bottom; 
	vert [1] .Red    = GetRValue(rgb)<<8;
	vert [1] .Green  = GetGValue(rgb)<<8;
	vert [1] .Blue   = GetBValue(rgb)<<8;
	vert [1] .Alpha  = 0x0000;

	// left edge
	vert [0] .x      = rect.left;
	vert [0] .y      = rect.top;
	vert [0] .Red    = GetRValue(rgb2)<<8;
	vert [0] .Green  = GetGValue(rgb2)<<8;
	vert [0] .Blue   = GetBValue(rgb2)<<8;
	vert [0] .Alpha  = 0x0000;

	// fill rectangle with gradient
	GRADIENT_RECT    gRect;
	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;
	GradientFill(pDC->GetSafeHdc(),vert,2,&gRect,1,mode);	   // REQUIRES msimg32.lib
}

//----------------------------------------------------------------------------------------------------------
