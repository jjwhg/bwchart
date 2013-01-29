/********************************************************************
*
* Description:	definitions for StormLib - linux port
*		intended to be used in GLdiablo
*		
*	---->	StormLib was originally developed for Windows by
*		Ladislav Zezula (www.hyperlink.cz/ladik), and he did
*		a _great_ job! Thanks Ladislav!
*
*	this is currently a quick and dirty hack to get it working
*	don't expect beauty and/or miracles :)
*
* Author: Marko Friedemann <marko.friedemann@bmx-chemnitz.de>
* Created at: Mon Jan 29 18:26:01 CEST 2001
* Computer: whiplash.flachland-chemnitz.de 
* System: Linux 2.4.0 on i686
*    
* Copyright (c) 2001 BMX-Chemnitz.DE All rights reserved.
*
********************************************************************/

#ifndef _PORT_h_included
#define _PORT_h_included

// for Windows, simply include <windows.h>
#ifdef _WIN32

#include <windows.h>

// assumption: we are not on Windows, so this must be linux *grin*
#else

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// most of this stuff is taken from wine (don't have no MS includes at hand)
#define CHAR char
#define BYTE unsigned char
#define SHORT short
#define WORD unsigned short
#define LONG long
#define DWORD unsigned long
#define BOOL bool

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#define VOID void
#define HANDLE void *
#define WINAPI 

#define FILE_BEGIN SEEK_SET
#define FILE_CURRENT SEEK_CUR
#define FILE_END SEEK_END

#define CREATE_NEW 1
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4

#define FILE_SHARE_READ 0x00000001L
#define GENERIC_WRITE 0x40000000
#define GENERIC_READ 0x80000000

#define ERROR_SUCCESS 0
#define ERROR_INVALID_FUNCTION 1
#define ERROR_FILE_NOT_FOUND 2
#define ERROR_ACCESS_DENIED 5
#define ERROR_NOT_ENOUGH_MEMORY 8
#define ERROR_BAD_FORMAT 11
#define ERROR_NO_MORE_FILES 18
#define ERROR_HANDLE_EOF 38
#define ERROR_HANDLE_DISK_FULL 39
#define ERROR_INVALID_PARAMETER 87
#define ERROR_DISK_FULL 112
#define ERROR_ALREADY_EXISTS 183
#define ERROR_CAN_NOT_COMPLETE 1003

#define INVALID_HANDLE_VALUE ((HANDLE) -1)

#ifndef min
#define min(a, b) (a < b) ? a : b
#endif

#ifndef max
#define max(a, b) (a > b) ? a : b
#endif
	
extern int globalerr;

void SetLastError(int err);
int GetLastError();
char *ErrString(int err);

HANDLE CreateFile(const char *sFileName, DWORD ulMode, DWORD ulSharing, void *pSecAttrib, DWORD, DWORD, HANDLE);
BOOL CloseHandle(HANDLE);

DWORD GetFileSize(HANDLE, DWORD *);
DWORD SetFilePointer(HANDLE, LONG lOffSetLow, LONG *pOffSetHigh, DWORD ulMethod);
BOOL SetEndOfFile(HANDLE);

BOOL ReadFile(HANDLE, void *pBuffer, DWORD ulLen, DWORD *ulRead, void *pOverLapped);
BOOL WriteFile(HANDLE, const void *pBuffer, DWORD ulLen, DWORD *ulWritten, void *pOverLapped);

#endif
// ! _WIN32

#endif

