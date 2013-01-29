#ifndef _BWDB_H
#define _BWDB_H

extern const char * _ConverToHex(const char *str);
extern char * _ConverFromHex(const char *str);

#define TAG_VERSION "__VERSION_"

//------------------------------------------------------------

class BWChartDB
{
private:
	static void _BuildUserDataFileName(CString& rpath, const char *file);
	static void _MoveReplayFile(int nfile, bool tobwchart);

	static bool m_useMyDocuments;
	static char *m_buffer;
	static int m_bufferSize;
	static char *_GetBuffer(int size);

public:
	virtual ~BWChartDB() {};
		
	// removed all unwanted signs in a map name to make it more readable
	static const char *ClarifyMapName(CString& map, const char *mapname);

	// get database file name
	enum {FILE_MAIN, FILE_FAVORITES, FILE_COMMENTS, FILE_AKAS, FILE_MAPAKAS,FILE_BOS,__FILEMAX};
 	static const char *GetDatabaseFileName(CString& path, int file);

	// init/exit instance
	static bool InitInstance(bool useMyDocuments);
	static void ExitInstance();
	static bool ClearDatabase();

	static const char * ConverToHex(const char *str);
	static char * ConverFromHex(const char *str);

	// write entry (section/entry/data in regular format)
	static void WriteEntry(int file, const char *section, const char *entry, const char *data, bool convertToHex=true);

	// section/entry already in HEX format, buffer returned in regular format
	static void ReadEntryBis(int file, const char *section, const char *entry, char *buffer, int bufsize);

	// section/entry in regular format, buffer returned in regular format
	static void ReadEntry(int file, const char *section, const char *entry, char *buffer, int bufsize, bool convertFromHex=true);

	// remove entry (section/entry/data in regular format)
	static void Delete(int nfile, const char *section, const char *entry);

	// load file
	bool LoadFile(int nfile);
	virtual void ProcessEntry(const char * /*section*/, const char * /*entry*/, const char * /*data*/, int /*percentage*/) {}

	// change database directory
	static void UpdateDatabaseDir(bool mydoc);
};

//------------------------------------------------------------

#endif