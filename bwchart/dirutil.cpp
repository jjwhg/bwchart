// utildir.cpp : implementation file
//

#include "stdafx.h"
#include "dirutil.h"

#include<io.h>
#include<direct.h>
#include<assert.h>
#include <shellapi.h>

//----------------------------------------------------------------------------

// builds a path from a dir and a file, taking care of the separator inbetween
const char *UtilDir::AddFileName(char *path, const char *dir, const char *file)
{
	// get separator
	char szSep[]="?";
	szSep[0] = '\\';

	// no crash in release
	if(path==0 || dir==0 || file==0) return 0;

	// append backslash if necessary
	strcpy(path,dir);
	if(path[0]!=0 && path[strlen(path)-1]!=szSep[0]) 
		strcat(path,szSep);

	// concat file name (without backslash if it had one in the beginning)
	const char *p=(file[0]==szSep[0]) ? file+1 : file;
	if(p[0]!=0) strcat(path,p);
	return path;
}

//----------------------------------------------------------------------------

// split a path into a dir and a filename
const char *UtilDir::_SplitRoot(char *rootup)
{
	// create root dir for upper level
	int lenpath = strlen(rootup)-1;
	if(rootup[lenpath]=='\\') rootup[lenpath]=0;
	char *ptr=strrchr(rootup,'\\');
	if(ptr) *ptr=0;
	return ptr+1;
}

//----------------------------------------------------------------------------

// because we can only create one level of directory at the time
// we must recursively create each level of a full directory path
// we go up until we find the existing root dir, and then we go down again
// to create each level
//
int UtilDir::CreatDirRecursive(const char *root, const char *newdir)
{
	// if root dir dont exist, we must go up to create it
	if(_access(root,0)!=0) 
	{
		// create root dir for upper level
		char rootup[_MAX_PATH+1];
		strcpy(rootup,root);
		const char *dir = _SplitRoot(rootup);
		if(CreatDirRecursive(rootup, dir)!=0)
			return -1;
	}

	if(newdir==0) return 0;
	assert(newdir[0]!='\\');

	// now root dir exists, we can build the new dir
	char path[_MAX_PATH+1];
	AddFileName(path,root,newdir);

	// create new dir
	if(_mkdir(path)!=0) return -1;
	return 0;
}

//-----------------------------------------------------------------------------

static char szPath[_MAX_PATH]="";

int CALLBACK  BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM /*pData*/) 
{
	switch(uMsg) 
	{
		case BFFM_INITIALIZED: 
		{
			// WParam is TRUE since you are passing a path.
			// It would be FALSE if you were passing a pidl.
			SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)szPath);
			break;
		}
		case BFFM_SELCHANGED: 
		{
			// Set the status window to the currently selected path.
			TCHAR szDir[MAX_PATH];
			if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir)) 
				SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
			break;
		}
		default:
			break;
	}
	return 0;
}

//-----------------------------------------------------------------------------

// browse for a directory.
// returns pointer to selected path if ok, 0 if cancel
const char * UtilDir::BrowseDir(const char *title, const char *defpath)
{
	const char *pret=0;
	if(defpath) strcpy(szPath,defpath);
	else szPath[0]=0; 

	// need a special allocator
	LPMALLOC lpMalloc; 
	if (::SHGetMalloc(&lpMalloc) != NOERROR) return 0;

	// init dialog box info
	BROWSEINFO info;
	memset(&info,0,sizeof(BROWSEINFO));
	info.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
	info.pszDisplayName = szPath;
	info.lpszTitle = title;
	info.ulFlags = BIF_RETURNONLYFSDIRS+BIF_DONTGOBELOWDOMAIN+BIF_BROWSEFORCOMPUTER; 
	info.lpfn = BrowseCallbackProc;

	// open dialog box (win 95 required)
	LPITEMIDLIST lpItemIDList = SHBrowseForFolder(&info); 

	// if we selected a path, put it in the edit box
	if(lpItemIDList!=NULL && ::SHGetPathFromIDList(lpItemIDList, szPath))
	{
		pret=szPath;
	}

	// free item list
	lpMalloc->Free(lpItemIDList);

	// free special allocator
	lpMalloc->Release();      

	return pret;
}

//--------------------------------------------------------------------------------

// Return true if the file exist and is writable OR the file does not exist
// Try to make it writable if it's not.
bool UtilDir::MakeFileWritable( const char *pszFile )
{
	if( pszFile == 0 || pszFile[0]==0) return false;

	DWORD dwAttributes = GetFileAttributes( pszFile );
	// on error
	if( dwAttributes == (DWORD)(-1) ) return true;
	// if we already have write permission
	if( (dwAttributes&FILE_ATTRIBUTE_READONLY)==0 ) return true;
	// add write permission
	dwAttributes &= ~FILE_ATTRIBUTE_READONLY;
	return( SetFileAttributes( pszFile, dwAttributes )!=0 );
}

//--------------------------------------------------------------------------------------------------
