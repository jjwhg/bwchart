#include"stdafx.h"
#include"bwdb.h"
#include"resource.h"
#include"DlgBWChart.h"
#include"dirutil.h"
#include<io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool BWChartDB::m_useMyDocuments = true;

char *BWChartDB::m_buffer=0;
int BWChartDB::m_bufferSize=0;

//-----------------------------------------------------------------------------------------------------------------

char * BWChartDB::_GetBuffer(int size)
{
	// make sure conversion buffer is big enough
	if(size > m_bufferSize)
	{
		m_bufferSize = size;
		m_buffer = (char*)realloc(m_buffer,m_bufferSize);
	}

	return m_buffer;
}

//-----------------------------------------------------------------------------------------------------------------

const char * BWChartDB::ConverToHex(const char *str)
{
	// make sure conversion buffer is big enough
	int lenstr =  (int)strlen(str);
	_GetBuffer((lenstr+1)*2);

	// convert chars to Hex values
	m_buffer[0]=0;
	for(int i=0;i<lenstr; i++)
		sprintf(&m_buffer[strlen(m_buffer)],"%02X",(unsigned char)str[i]);
	
	return m_buffer;
}

//-----------------------------------------------------------------------------------------------------------------

// removed all unwanted signs in a map name to make it more readable
const char *BWChartDB::ClarifyMapName(CString& map, const char *mapname)
{
	map = mapname;
	map.Replace("WGTour","");
	map.Replace("WCG","");
	map.Replace("(2)","");
	map.Replace("(3)","");
	map.Replace("(4)","");
	map.Replace("(5)","");
	map.Replace("(6)","");
	map.Replace("(7)","");
	map.Replace("(8)","");
	map.Replace("[2])","");
	map.Replace("[3])","");
	map.Replace("[4])","");
	map.Replace("[5])","");
	map.Replace("[6])","");
	map.Replace("[7])","");
	map.Replace("[8])","");
	map.Replace("WGT9 - ","");
	map.Replace("WGT10 - ","");
	map.Replace("WGT11 - ","");
	map.Replace("WGT12 - ","");
	map.Replace("WGT13 - ","");
	map.Replace("WGT14 - ","");
	map.Replace("PGT-","");
	return map;
}

//-----------------------------------------------------------------------------------------------------------------

// str = 08FFACDDEE  <- series of hexadecimal 8bits numbers
// buffer = the corresponding ascii letters
//
char * BWChartDB::ConverFromHex(const char *str)
{
	// make sure conversion buffer is big enough
	int len = (int)strlen(str)/2;
	_GetBuffer((len+1)*2);

	// convert Hex to chars
	m_buffer[0]=0;
	int val;
	for(int i=0,j=0;i<len; i++)
	{
		char c = str[j];
		int digit = (c>='0' && c<='9') ? (int)(c-'0') : (int)(10+c-'A');
		val = digit<<4;
		j++;
		c = str[j];
		digit = (c>='0' && c<='9') ? (int)(c-'0') : (int)(10+c-'A');
		val |= digit;
		j++;
		//sscanf(str+2*i,"%02X",&buffer[i]);
		m_buffer[i] = (unsigned char)val;
	}
	m_buffer[len]=0;
	return m_buffer;
}

//--------------------------------------------------------------------------------------------------------------

void BWChartDB::_BuildUserDataFileName(CString& rpath, const char *file)
{
	// use default (in My Documents)
	char path[MAX_PATH]="";

	if(m_useMyDocuments)
	{
		SHGetSpecialFolderPath(AfxGetMainWnd()->GetSafeHwnd(),path,CSIDL_PERSONAL,FALSE);
		if(path[strlen(path)-1]!='\\') strcat(path,"\\");
		strcat(path,"bwchart\\");

		// directory exist?
		UtilDir::CreatDirRecursive(path);
	}
	else
	{
		// use executable path
		GetModuleFileName(0,path,sizeof(path));
		char *p=strrchr(path,'\\'); p[1]=0;
	}

	// file path
	strcat(path,file);
	rpath = path;
}

//--------------------------------------------------------------------------------------------------------------

const char *BWChartDB::GetDatabaseFileName(CString& path, int file)
{
	char *files[]={
		"replays.txt",
		"favorites.txt",
		"comments.txt",
		"akas.txt",
		"mapakas.txt",
		"buildorders.txt"
	};
	// build rep list file name
	_BuildUserDataFileName(path,files[file]);
	return path;
}

//-----------------------------------------------------------------------------------------------------------------

static void CreateDefaultAka(const char *filename, UINT resID)
{
	HRSRC hres= FindResource(0,MAKEINTRESOURCE(resID),"TextFile");
	if(hres==0) return;
	HGLOBAL hg = LoadResource(0,hres);
	if(hg==0) return;
	void *pData = LockResource(hg);
	if(pData==0) return;
	FILE *fp=fopen(filename,"wb");
	if(fp==0) return;
	fwrite(pData,1,SizeofResource(0,hres),fp);
	fclose(fp);
}

//-----------------------------------------------------------------------------------------------------------------

// if file doesnt exist, create it with version number in it
static void _WriteVersion(int nfile)
{
	// if file doesnt exist
	CString repFile;
	if(_access(BWChartDB::GetDatabaseFileName(repFile, nfile),0)!=0)
	{
		// create default file
		if(nfile==BWChartDB::FILE_AKAS) CreateDefaultAka(repFile, IDR_TEXTAKAS);
		else if(nfile==BWChartDB::FILE_MAPAKAS) CreateDefaultAka(repFile, IDR_TEXTMAPAKAS);

		// add version
		::WritePrivateProfileString(TAG_VERSION,"ver",NVERSION,repFile);
	}
}

//-----------------------------------------------------------------------------------------------------------------

static void _ReadVersion(int nfile, CString& version)
{
	CString repFile;
	version="";

	// if file exists
	if(_access(BWChartDB::GetDatabaseFileName(repFile, nfile),0)==0)
	{
		// read version
		char buffer[64];
		::GetPrivateProfileString(TAG_VERSION,"ver","",buffer,sizeof(buffer),repFile);
		version=buffer;
	}
}

//-----------------------------------------------------------------------------------------------------------------

static void _DeleteFile(const char *path)
{
	UtilDir::MakeFileWritable(path);
	::DeleteFile(path);
}

//-----------------------------------------------------------------------------------------------------------------

void BWChartDB::_MoveReplayFile(int nfile, bool tobwchart)
{	
	CString fileFrom;
	CString fileTo;

	// save flag
	bool mydoc = m_useMyDocuments;

	// build file names
	m_useMyDocuments=tobwchart;
	GetDatabaseFileName(fileFrom, nfile);
	m_useMyDocuments=!tobwchart;
	GetDatabaseFileName(fileTo, nfile);

	// move file
	MoveFile(fileFrom,fileTo);

	// restore flag
	m_useMyDocuments = mydoc;
}

//-----------------------------------------------------------------------------------------------------------------

// change database directory
void BWChartDB::UpdateDatabaseDir(bool mydoc)
{	
	if(m_useMyDocuments==mydoc) return;

	m_useMyDocuments=mydoc;
	for(int i=0;i<__FILEMAX;i++)
		_MoveReplayFile(i,!m_useMyDocuments);
}

//-----------------------------------------------------------------------------------------------------------------

bool BWChartDB::ClearDatabase()
{
	if(AfxMessageBox(IDS_DELETEALL,MB_YESNO)==IDNO) return false;

	CString file;
	_DeleteFile(GetDatabaseFileName(file, FILE_MAIN));
	_DeleteFile(GetDatabaseFileName(file, FILE_FAVORITES));
	_DeleteFile(GetDatabaseFileName(file, FILE_BOS));
	InitInstance(m_useMyDocuments);
	return true;
}

//-----------------------------------------------------------------------------------------------------------------

// returns false if the BOs file is missing
bool BWChartDB::InitInstance(bool useMyDocuments)
{
	// where is the database
	m_useMyDocuments = useMyDocuments;

	// read version from main file
	CString version;
	_ReadVersion(FILE_MAIN, version);

	// if it's missing, it's an old file, so delete it
	if(version.IsEmpty()) 
		_DeleteFile(GetDatabaseFileName(version, FILE_MAIN));

	// if it's an old version, delete it
	version.MakeUpper();
	if(version.Compare("1.01H")<0) 
	{
		AfxMessageBox(IDS_FORMATCHG,MB_OK|MB_ICONINFORMATION);
		_DeleteFile(GetDatabaseFileName(version, FILE_MAIN));
		_DeleteFile(GetDatabaseFileName(version, FILE_FAVORITES));
	}

	// do we have a BO file?
	bool boExist=(_access(GetDatabaseFileName(version, FILE_BOS),0)==0);

	_WriteVersion(FILE_MAIN);
	_WriteVersion(FILE_FAVORITES);
	_WriteVersion(FILE_COMMENTS);
	_WriteVersion(FILE_AKAS);
	_WriteVersion(FILE_MAPAKAS);
	_WriteVersion(FILE_BOS);
	return boExist;
}

//-----------------------------------------------------------------------------------------------------------------

void BWChartDB::ExitInstance()
{
	if(m_buffer!=0) free(m_buffer);
}

//-----------------------------------------------------------------------------------------------------------------

#define SRCEOL "\r\n"
#define TOKENEOL "_%#$%_"

void BWChartDB::WriteEntry(int nfile, const char *section, const char *entry, const char *data, bool convertToHex)
{
	CString repFile(section);
	if(section[strlen(section)-1]=='\\') repFile=repFile.Left(repFile.GetLength()-1);

	// write to file
	CString cvnDir = ConverToHex(repFile);
	CString cvnName = ConverToHex(entry);
	if(convertToHex)
		::WritePrivateProfileString(cvnDir,cvnName,ConverToHex(data),GetDatabaseFileName(repFile, nfile));
	else
		::WritePrivateProfileString(cvnDir,cvnName,data,GetDatabaseFileName(repFile, nfile));

#ifndef NDEBUG
	//if(nfile==0)
	//	::WritePrivateProfileString(section,entry,data,"e:\\data\\vertigo\\runtime\\debug\\replays.txt");
#endif
}

//-----------------------------------------------------------------------------------------------------------------

// section/entry already in HEX format
void BWChartDB::ReadEntryBis(int nfile, const char *section, const char *entry, char *buffer, int bufsize)
{
	CString sectionNoSlash(section);
	if(section[strlen(section)-1]=='\\') sectionNoSlash=sectionNoSlash.Left(sectionNoSlash.GetLength()-1);
	buffer[0]=0;
	CString repFile;
	::GetPrivateProfileString(sectionNoSlash,entry,"",buffer,bufsize,GetDatabaseFileName(repFile, nfile));
}

//-----------------------------------------------------------------------------------------------------------------

// section/entry in regular format
void BWChartDB::ReadEntry(int nfile, const char *section, const char *entry, char *buffer, int bufsize, bool convertFromHex)
{
	CString repFile(section);
	buffer[0]=0;
	if(section[strlen(section)-1]=='\\') repFile=repFile.Left(repFile.GetLength()-1);
	CString cvnSec = ConverToHex(repFile);
	CString cvnEnt = ConverToHex(entry);
	::GetPrivateProfileString(cvnSec,cvnEnt,"",buffer,bufsize,GetDatabaseFileName(repFile, nfile));
	// buffer is converted back to regular chars
	if(convertFromHex) strcpy(buffer,ConverFromHex(buffer));
}

//-----------------------------------------------------------------------------------------------------------------

void BWChartDB::Delete(int nfile, const char *section, const char *entry)
{
	CString repFile(section);
	if(section[strlen(section)-1]=='\\') repFile=repFile.Left(repFile.GetLength()-1);
	CString cnvDir = ConverToHex(repFile);
	CString cnvName = ConverToHex(entry);
	::WritePrivateProfileString(cnvDir,cnvName,0,GetDatabaseFileName(repFile, nfile));
	::WritePrivateProfileString(0,0,0,repFile);
}

//--------------------------------------------------------------------------------------------------------------

// load file
bool BWChartDB::LoadFile(int nfile)
{
	// make file writable for future modifications
	CString repFile;
	BWChartDB::GetDatabaseFileName(repFile, nfile);
	UtilDir::MakeFileWritable(repFile);

	// get section list
	FILE *fp=fopen(repFile,"rb");
	if(fp==0) return false;

	// get file size
	fseek(fp,0,SEEK_END);
	unsigned long fsize=(unsigned long)ftell(fp);
	fseek(fp,0,SEEK_SET);

	// read line by line
	char line[2408];
	char reppath[512];
	char repname[512];
	reppath[0]=0;
	repname[0]=0;
	unsigned long sizeRead=0;
	while(fgets(line,sizeof(line),fp)!=0)
	{
		// update size read
		sizeRead+=strlen(line);
		unsigned long percentage = (100UL*sizeRead)/fsize;

		// remove end of line
		char *text=strtok(line,"\r\n");
		if(text==0) continue;

		// skip blanks
		while(*text==' ') text++;

		// comment?
		if(text[0]==';' || text[0]==0) continue;

		// if it is a section
		if(text[0]=='[')
		{
			text++;
			char *p=strrchr(text,']');
			if(p!=0)
			{
				*p=0; 
				if(strcmp(text,TAG_VERSION)!=0)
				{
					strcpy(reppath,BWChartDB::ConverFromHex(text));
					continue;
				}
			}
			reppath[0]=0;
		}
		else
		{
			// must be an entry, extract entry name
			char *p=strtok(text,"=");
			if(p==0) continue;
			strcpy(repname,BWChartDB::ConverFromHex(p));
			char *data=p+strlen(p)+1;
			// process entry
			if(reppath[0]!=0 && data) ProcessEntry(reppath, repname, data, (int)percentage);
		}
	}

	//close file
	fclose(fp);
	return true;
}

//-----------------------------------------------------------------------------------------------------------------
