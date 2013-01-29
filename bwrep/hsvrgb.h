//-----------------------------------------------------------------------------
// File: hsvrgb.h
//-----------------------------------------------------------------------------

#ifndef __HSVRGB_H
#define __HSVRGB_H

#include<windows.h>

class CRgbQuad : public	RGBQUAD
{
public:
	COLORREF rgb() const {return RGB(rgbRed,rgbGreen,rgbBlue);}
};

class CHsvRgb
{ 
public:
	// converts HSV components to RGb components
	// In: h = Hue, is supposed to be between 0 and 360 not included
	//     s = Saturation, is between 0 and 1 included
	//     v = Luminosity, is between 0 and 1 included
	// Out: CRgbQuad structure with r,g,b components between 0 and 255 included
	//
	static CRgbQuad Hsv2Rgb(int h, double s, double v);

	// converts RGB components to HSV components
	// In: RGBQUAD structure qith r,g,b components between 0 and 255 included
	// Out: h = Hue, is supposed to be between 0 and 360 not included
	//      s = Saturation, is between 0 and 1 included
	//      v = Luminosity, is between 0 and 1 included
	//
	// Sophie - 12/29/99
	//
	static void Rgb2Hsv(const RGBQUAD* pRGB, int* pH, double* pS, double* pV);
	static void Rgb2Hsv(const COLORREF rgb, int* pH, double* pS, double* pV);

	// return darker color, same hue and saturation
	static COLORREF Darker(COLORREF rgb, double ratio);
};

//----------------------------------------------------------------------------------------------------------------------

#endif

