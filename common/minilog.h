//----------------------------------------------------------------------------
// FDR (C) 2002 InVision Technologies
//----------------------------------------------------------------------------
// Design & Implementation by VERTIGO Engineering (http://www.vertigoeng.com)
//----------------------------------------------------------------------------
// minilog.h : 
//
#ifndef __MINILOG_H
#define __MINILOG_H

// logging levels
#define LOG_SYSTEM  -1,__FILE__,__LINE__
#define LOG_ERROR  -2,__FILE__,__LINE__
#define LOG_DEBUG1 1,__FILE__,__LINE__
#define LOG_DEBUG2 2,__FILE__,__LINE__

class VEMiniLog
{
	static int m_nTraceLevel;  // 0 = no trace
public:
	static int PrintError(const char *pszModule, int iLineNumber, int error, const char *pszError, const char *pszFormat, ...);
	static void Print(int level, const char *pszModule, int iLineNumber, const char *pszFormat, ...);
	static const char *GetLogFileName();
	static void SetLevel(int level);
};

// normal logging
#define LogF VEMiniLog::Print

// error logging
#define _E(errnum) __FILE__,__LINE__, errnum, #errnum
#define LogE VEMiniLog::PrintError

// usage:
// LogF(LOG_DEBUG,"text %d",param);
// LogE(_E(error), "text %d",param);

#endif
