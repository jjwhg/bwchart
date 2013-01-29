//-----------------------------------------------------------------------------
// File: hsvrgb.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "hsvrgb.h"
#include <windows.h>
#include <math.h>

//-----------------------------------------------------------------------------

static void hsv2rgb(double h, double s, double v, double *r, double *g, double *b)
{
     int i;
     double f,p,q,t;

     if (s <= 0.0) { // achromatic 
             *r = v;
             *g = v;
             *b = v;
     }
     else {
             if (h >= 1.0) h = 0.0;
             h = 6.0 * h;
             i = (int)h;
             f = h - (double)i;
             p = v * (1 - s);
             q = v * (1 - (s * f));
             t = v * ( 1 - (s * (1 - f)));
             switch (i) {
                     case 0: *r = v; *g = t; *b = p; break;
                     case 1: *r = q; *g = v; *b = p; break;
                     case 2: *r = p; *g = v; *b = t; break;
                     case 3: *r = p; *g = q; *b = v; break;
                     case 4: *r = t; *g = p; *b = v; break;
                     case 5: *r = v; *g = p; *b = q; break;
             }
     }
}

static void rgb2hsv(double r, double g, double b, double *h, double *s, double *v)
{

     double          rgbmin,rgbmax;
     double          rc,bc,gc;
     double          ht=0.0,st=0.0;

     rgbmin = min(r,min(g,b));
     rgbmax = max(r,max(g,b));

     if (rgbmax > 0.0)
             st = (rgbmax - rgbmin) / rgbmax;

     if (st > 0.0) {
             rc = (rgbmax - r) / (rgbmax - rgbmin);
             gc = (rgbmax - g) / (rgbmax - rgbmin);
             bc = (rgbmax - b) / (rgbmax - rgbmin);
             if (r == rgbmax) ht = bc - gc;
             else if (g == rgbmax) ht = 2 + rc - bc;
             else if (b == rgbmax) ht = 4 + gc - rc;
             ht = ht * 60.0;
             if (ht < 0.0) ht += 360.0;
     }
     *h = ht / 360.0;
     *v = rgbmax;
     *s = st;
}

//----------------------------------------------------------------------------------------------------------

// return darker color, same hue and saturation
COLORREF CHsvRgb::Darker(COLORREF rgb, double ratio)
{
	// get darker value
	double sl,vl;
	int hl;
	CHsvRgb::Rgb2Hsv(rgb,&hl,&sl,&vl);
	vl*=ratio;
	CRgbQuad quad=CHsvRgb::Hsv2Rgb(hl,sl,vl);
	return quad.rgb();
}

//----------------------------------------------------------------------------------------------------------------------

// converts HSV components to RGb components
// In: h = Hue, is supposed to be between 0 and 360 not included
//     s = Saturation, is between 0 and 1 included
//     v = Luminosity, is between 0 and 1 included
// Out: RGBQUAD structure qith r,g,b components between 0 and 255 included
//
CRgbQuad CHsvRgb::Hsv2Rgb(int h, double s, double v)
{
	if(s>1.0) s=1.0;
	if(v>1.0) v=1.0;
	if(s<0.0) s=0.0;
	if(v<0.0) v=0.0;

	double r=0.0;
	double g=0.0;
	double b=0.0;
	double dh=(double)(h%360)/360.0;
	hsv2rgb(dh, s, v, &r, &g, &b);

	CRgbQuad c;
	c.rgbRed = (BYTE)floor(r * 255);
	c.rgbGreen = (BYTE)floor(g * 255);
	c.rgbBlue = (BYTE)floor(b * 255);

	return c;
}

//----------------------------------------------------------------------------------------------------------------------

// converts RGB components to HSV components
// In: RGBQUAD structure qith r,g,b components between 0 and 255 included
// Out: h = Hue, is supposed to be between 0 and 360 not included
//      s = Saturation, is between 0 and 1 included
//      v = Luminosity, is between 0 and 1 included
//
// Sophie - 12/29/99
//
void CHsvRgb::Rgb2Hsv(const RGBQUAD* pRGB, int* pH, double* pS, double* pV)
{
	double r=(float)pRGB->rgbRed/255.0;
	double g=(float)pRGB->rgbGreen/255.0;
	double b=(float)pRGB->rgbBlue/255.0;
	double dh;
	rgb2hsv(r, g, b, &dh, pS, pV);
	*pH = (int)(dh*360.0);
}

void CHsvRgb::Rgb2Hsv(const COLORREF rgb, int* pH, double* pS, double* pV)
{
	double r=(float)(GetRValue(rgb))/255.0;
	double g=(float)(GetGValue(rgb))/255.0;
	double b=(float)(GetBValue(rgb))/255.0;
	double dh;
	rgb2hsv(r, g, b, &dh, pS, pV);
	*pH = (int)(dh*360.0);
}

//----------------------------------------------------------------------------------------------------------------------

