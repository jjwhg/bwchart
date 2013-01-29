#ifndef __REGPARAM__H
#define __REGPARAM__H

//--------------------------------------------------------------------------------------------------

class VERegistryParameters
{
public:
	static void WriteProfileTime(const char *section, const char *entry, const CTime& time)
	{
		AfxGetApp()->WriteProfileString(section,entry,time.Format("%m/%d/%Y/%H:%M:%S"));
	}

	static CTime GetProfileTime(const char *section, const char *entry, const CTime* deftime=0)
	{
		CString buf = AfxGetApp()->GetProfileString(section,entry,"");
		if(buf.IsEmpty()) return deftime==0?CTime::GetCurrentTime():*deftime;
		// format is:dd/mm/yyyy/hh:mm:ss
		const char *pBuf=buf;
		int month= atoi(pBuf);
		int day= atoi(pBuf+3);
		int year= atoi(pBuf+6);
		int hour= atoi(pBuf+11);
		int min= atoi(pBuf+14);
		int sec= atoi(pBuf+17);
		return CTime(year,month,day,hour,min,sec);
	}
};

// macro to load/save a string in registry
#define PSTRING(section, name, defval) \
	if(bLoad) m_##name = AfxGetApp()->GetProfileString(section,#name,defval);\
	else AfxGetApp()->WriteProfileString(section,#name,m_##name);

// macro to load/save an int or a BOOL in registry
#define PINT(section, name, defval) \
{CString tmpName(#name); tmpName.Replace("[","_"); tmpName.Replace("]","_");\
	if(bLoad) m_##name = AfxGetApp()->GetProfileInt(section,tmpName,defval);\
		else AfxGetApp()->WriteProfileInt(section,tmpName,m_##name);}

// macro to load/save an a bool in registry
#define PBOOL(section, name, defval) \
{CString tmpName(#name); tmpName.Replace("[","_"); tmpName.Replace("]","_");\
	if(bLoad) m_##name = AfxGetApp()->GetProfileInt(section,tmpName,defval)?true:false;\
		else AfxGetApp()->WriteProfileInt(section,tmpName,(int)m_##name);}

// macro to load/save an int or a Ctime in registry
#define PCTIME(section, name, defval) \
	if(bLoad) m_##name = VERegistryParameters::GetProfileTime(section,#name,defval);\
	else VERegistryParameters::WriteProfileTime(section,#name,m_##name);

// macro to load/save an array of int registry
#define PINTARRAY(section, name, size, defvals) \
	{for(int i=0;i<size;i++) {char entry[64]; \
	sprintf(entry,"%s%02d",#name,i);\
	if(bLoad) m_##name [i] = AfxGetApp()->GetProfileInt(section, entry,defvals[i]);\
	else AfxGetApp()->WriteProfileInt(section, entry,m_##name [i]);}}


// SAMPLE : load or save parameters from registry
/*
void CFdrxDlg::_Parameters(bool bLoad)
{
	PSTRING("MAIN", rootDir, "c:\\fdr");
	PSTRING("MAIN", mediaDir, "c:\\fdr\\extract");
	PINT("MAIN", bFilter, 0);
	PINT("MAIN", bMark, 1);
	PINT("MAIN", destFormat, 0);
	PINT("MAIN", destMedia, 0);
	PCTIME("MAIN", dateFrom, 0);
	PCTIME("MAIN", dateTo, 0);
}
*/

//--------------------------------------------------------------------------------------------------

#endif
