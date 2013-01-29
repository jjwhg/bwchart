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
#define PSTRING(section, name, prefix, defval) \
	if(bLoad) m_##name = AfxGetApp()->GetProfileString(section,#prefix #name,defval);\
	else AfxGetApp()->WriteProfileString(section,#prefix #name,m_##name);

// macro to load/save an int or a BOOL in registry
#define PINT(section, name, prefix, defval) \
	if(bLoad) m_##name = AfxGetApp()->GetProfileInt(section,#prefix #name,defval);\
	else AfxGetApp()->WriteProfileInt(section,#prefix #name,m_##name);

// macro to load/save an int or a Ctime in registry
#define PCTIME(section, name, prefix, defval) \
	if(bLoad) m_##name = VERegistryParameters::GetProfileTime(section,#prefix #name,defval);\
	else VERegistryParameters::WriteProfileTime(section,#prefix #name,m_##name);

// macro to load/save a bool in registry
#define PBOOL(section, name, prefix, defval) \
	if(bLoad) m_##name = AfxGetApp()->GetProfileInt(section,#prefix #name,defval) ? true : false;\
	else AfxGetApp()->WriteProfileInt(section,#prefix #name,m_##name ? 1:0);

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
