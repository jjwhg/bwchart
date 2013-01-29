//----------------------------------------------------------------------------
// FDR (C) 2002 InVision Technologies
//----------------------------------------------------------------------------
// Design & Implementation by VERTIGO Engineering (http://www.vertigoeng.com)
//----------------------------------------------------------------------------
// minilog.cpp : 
//
#include"stdafx.h"
#include"minilog.h"
#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//-----------------------------------------------------------------------------------------------

#ifdef UNIX // added by JP
#define _vsnprintf vsnprintf
#define _MAX_PATH 255
#include <sys/time.h>
#include <unistd.h>

char* _strtime(char* szBuffer)
{
	time_t t;
	struct tm *tms;
	time(&t);
	tms = localtime(&t);
	sprintf(szBuffer,"%02d:%02d:%02d",(int)tms->tm_hour,(int)tms->tm_min,(int)tms->tm_sec) ;
	return szBuffer ;
}

void GetModuleFileName(int,char* path,int nBufferSize)
{
	// /proc/self/exe is a linked to the current prog
	int nPos = readlink("/proc/self/exe",path, nBufferSize);
	assert(nPos>0) ;
	// if NPos<0 can't readlink
	if(nPos<0) return ;
	path[nPos] = 0 ;

	// for compatibility issue (under window programs have exe as extension)
	strcat(path,".exe") ;
}

#endif /* UNIX */

//-----------------------------------------------------------------------------------------------

void VEMiniLog::SetLevel(int level)
{
	if(m_nTraceLevel==0 && level>0) m_nTraceLevel = level;
	LogF(LOG_DEBUG1,"Level set to %d",level);
	m_nTraceLevel = level;
}

//-----------------------------------------------------------------------------------------------

#define ENDOFLINE "\r\n"

// trave level
int VEMiniLog::m_nTraceLevel=0; // default is no trace

// buffer for logging lines
static char gszParam[512+1];

// line = "04:13:01.610|DEB| ogger\logger.c| 353| Log file created (05-09-2001)"
void VEMiniLog::Print(int level, const char *pszModule, int iLineNumber, const char *pszFormat, ...)
{
	// is tracing off?
	if(level > m_nTraceLevel) return;

	// error type
	const char *errType = 
		level==-1 ? "SYS" : 
		level==-2 ? "ERR" : 
		"DEB";

 	// store time, module name & line number
	_strtime(gszParam);
	const char *pmod=strrchr(pszModule,'\\'); 
	pmod=pmod!=0?pmod+1:pszModule;
	sprintf(&gszParam[strlen(gszParam)], ".000|%s|%-13s|%04d|",errType, pmod, iLineNumber);

	// get ... parameters
    va_list	arg_ptr;
    va_start( arg_ptr, pszFormat );
    _vsnprintf( &gszParam[strlen(gszParam)], sizeof(gszParam)-strlen(gszParam), pszFormat, arg_ptr );
    va_end( arg_ptr );

	// write line of log
	FILE *fp=fopen(GetLogFileName(),"ab");
	if(fp==0) return;
	fwrite(gszParam,strlen(gszParam),1,fp);
	fwrite(ENDOFLINE,strlen(ENDOFLINE),1,fp);
	fclose(fp);
}

//-----------------------------------------------------------------------------------------------

const char *VEMiniLog::GetLogFileName()
{
	// build file name
	static char path[_MAX_PATH]="";
	if(path[0]==0)
	{
		GetModuleFileName(0,path,sizeof(path));
		char *p=strrchr(path,'.'); if(p!=0) p[1]=0;
		strcat(path,"log");
	}
	return path;
}

//-----------------------------------------------------------------------------------------------

int VEMiniLog::PrintError(const char *pszModule, int iLineNumber, int error, const char *pszErr, const char *pszFormat, ...)
{
	// is it really an error?
	if(error==0) return 0;

	// get ... parameters
    va_list	arg_ptr;
    va_start( arg_ptr, pszFormat );
	char szParam[512];
    _vsnprintf( szParam, sizeof(szParam), pszFormat, arg_ptr );
    va_end( arg_ptr );

	// add error string	for VLOG
	strcat(szParam," {");
	strcat(szParam,pszErr);
	strcat(szParam,"}");

	// add an error line in the log file
	Print(0,pszModule,iLineNumber,szParam);

	// return error code
	return error;
}

//-----------------------------------------------------------------------------------------------
