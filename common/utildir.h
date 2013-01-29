//----------------------------------------------------------------------------
// FDR (C) 2002 InVision Technologies
//----------------------------------------------------------------------------
// Design & Implementation by VERTIGO Engineering (http://www.vertigoeng.com)
//----------------------------------------------------------------------------
#ifndef __UTILDIR_H
#define __UTILDIR_H

class UtilDir
{
private:
	// split a path into a dir and a filename
	static const char *_SplitRoot(char *rootup);

public:
	// builds a path from a dir and a file, taking care of the separator inbetween
	static const char *AddFileName(char *path, const char *dir, const char *file);

	// builds a path from a dir and a file, taking care of the separator inbetween
	static const char *AddFileName(char *path, const char *file) {return AddFileName(path, (const char *)path, file);}

	// because we can only create one level of directory at the time
	// we must recursively create each level of a full directory path
	// we go up until we find the existing root dir, and then we go down again
	// to create each level
	//
	static int CreatDirRecursive(const char *root, const char *newdir=0);

	// browse for a directory.
	// returns pointer to selected path if ok, 0 if cancel
	static const char * BrowseDir(const char *title, const char *defpath);

	// Return true if the file exist and is writable OR the file does not exist
	// Try to make it writable if it's not.
	static bool MakeFileWritable( const char *pszFile );
};

#endif

