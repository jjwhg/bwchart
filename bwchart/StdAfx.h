// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__D1664EEE_75BA_41CB_A094_20BA53AC66A2__INCLUDED_)
#define AFX_STDAFX_H__D1664EEE_75BA_41CB_A094_20BA53AC66A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _CRT_SECURE_NO_DEPRECATE 1

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxinet.h>		// MFC support for http
#endif // _AFX_NO_AFXCMN_SUPPORT

#define DO_NOT_INCLUDE_XCOMBOLIST
#define NO_XLISTCTRL_TOOL_TIPS

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__D1664EEE_75BA_41CB_A094_20BA53AC66A2__INCLUDED_)
