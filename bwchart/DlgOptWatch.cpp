// DlgOptWatch.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgOptWatch.h"
#include "regparam.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgOptWatch dialog


DlgOptWatch::DlgOptWatch(CWnd* pParent /*=NULL*/)
	: CDialog(DlgOptWatch::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgOptWatch)
	m_timeWait = 3000;
	m_keyseq = _T("SBORO*O");
	m_timeWait2 = 1500;
	m_bwexe110 = _T("");
	m_keyseq2 = _T("");
	m_bwexe109 = _T("");
	m_autoKeys = FALSE;
	m_bwplayer = _T("");
	m_autoStartRWA = FALSE;
	m_bwexe111 = _T("");
	m_bwexe112 = _T("");
	m_bwexe113 = _T("");
	m_bwexe114 = _T("");
	m_bwexe115 = _T("");
	m_bwexe116 = _T("");
	//}}AFX_DATA_INIT

	_Parameters(true);

	_DefaultPath(m_bwexe109,"starcraft1[1].09b.exe");
	_DefaultPath(m_bwexe110,"starcraft.exe");
	if(m_bwexe111.IsEmpty()) m_bwexe111 = m_bwexe110;
	if(m_bwexe112.IsEmpty()) m_bwexe112 = m_bwexe111;
	if(m_bwexe113.IsEmpty()) m_bwexe113 = m_bwexe112;
	if(m_bwexe114.IsEmpty()) m_bwexe114 = m_bwexe113;
	if(m_bwexe115.IsEmpty()) m_bwexe115 = m_bwexe114;
	if(m_bwexe116.IsEmpty()) m_bwexe116 = m_bwexe115;

	if(m_bwplayer.IsEmpty()) 
	{
		char buf[255];
		GetModuleFileName(0,buf,sizeof(buf));
		char *p=strrchr(buf,'\\'); p[1]=0;
		strcat(buf,"bwplayer.exe");
		m_bwplayer = buf;
	}
	//}}AFX_DATA_INIT
}


//----------------------------------------------------------------------------------------------

void DlgOptWatch::_DefaultPath(CString& path, const char *defexe)
{
	if(path.IsEmpty()) 
	{
		_GetStarcraftPath(path);
		if(path.Right(1)!="\\") path += "\\";
		path += defexe;
	}
}

//----------------------------------------------------------------------------------------------

void DlgOptWatch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgOptWatch)
	DDX_Text(pDX, IDC_TIMEWAIT, m_timeWait);
	DDX_Text(pDX, IDC_KEYSEQ, m_keyseq);
	DDX_Text(pDX, IDC_TIMEWAIT2, m_timeWait2);
	DDX_Text(pDX, IDC_INSTALLPATH, m_bwexe110);
	DDX_Text(pDX, IDC_KEYSEQ2, m_keyseq2);
	DDX_Text(pDX, IDC_BWEXE109, m_bwexe109);
	DDX_Check(pDX, IDC_AUTOKEYS, m_autoKeys);
	DDX_Text(pDX, IDC_BWPLAYER, m_bwplayer);
	DDX_Check(pDX, IDC_AUTOSTARTRWA, m_autoStartRWA);
	DDX_Text(pDX, IDC_BWEXE111, m_bwexe111);
	DDX_Text(pDX, IDC_BWEXE112, m_bwexe112);
	DDX_Text(pDX, IDC_BWEXE113, m_bwexe113);
	DDX_Text(pDX, IDC_BWEXE114, m_bwexe114);
	DDX_Text(pDX, IDC_BWEXE115, m_bwexe115);
	DDX_Text(pDX, IDC_BWEXE116, m_bwexe116);
	//}}AFX_DATA_MAP
}

//----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DlgOptWatch, CDialog)
	//{{AFX_MSG_MAP(DlgOptWatch)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_BROWSEDIR, OnBrowsedir)
	ON_BN_CLICKED(IDC_BROWSEPLAYER, OnBrowseplayer)
	ON_BN_CLICKED(IDC_BROWSEDIRV11, OnBrowsedirv11)
	ON_BN_CLICKED(IDC_BROWSEDIRV12, OnBrowsedirv12)
	ON_BN_CLICKED(IDC_BROWSEDIRV13, OnBrowsedirv13)
	ON_BN_CLICKED(IDC_BROWSEDIRV14, OnBrowsedirv14)
	ON_BN_CLICKED(IDC_BROWSEDIRV15, OnBrowsedirv15)
	ON_BN_CLICKED(IDC_BROWSEDIRV16, OnBrowsedirv16)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_AUTOKEYS, OnUpdateControl)
	ON_BN_CLICKED(IDC_AUTOSTARTRWA, OnUpdateControl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::_GetStarcraftPath(CString& path) 
{
	// open key for control panel
	HKEY hKey;
	if ( ERROR_SUCCESS != RegCreateKey(HKEY_LOCAL_MACHINE, "Software\\Blizzard Entertainment\\Starcraft",&hKey) ) 
		return;

	// get program location
	char buffer[255];
	unsigned long size = sizeof(buffer);
	unsigned long type = REG_SZ;
	RegQueryValueEx(hKey,"Program", 0, &type, (unsigned char*)buffer, &size);
	char *p=strrchr(buffer,'\\'); if(p!=0) p[1]=0;
	path=buffer;

	// close key
	RegCloseKey(hKey);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::InnerSaveValues()
{
	UpdateData(TRUE);
	_Parameters(false);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::OnDestroy() 
{
	InnerSaveValues();
	CDialog::OnDestroy();
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::_BrowseExe(CString& path) 
{
	static char BASED_CODE szFilter[] = "Executable (*.exe)|*.exe|All Files (*.*)|*.*||";

	CString scdir;
	_GetStarcraftPath(scdir);

 	CFileDialog dlg(TRUE,"exe","",0,szFilter,this);
	dlg.m_ofn.lpstrInitialDir = scdir;
	if(dlg.DoModal()==IDOK)
	{
		path = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::OnBrowse() 
{
	_BrowseExe(m_bwexe109);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::OnBrowsedir() 
{
	_BrowseExe(m_bwexe110);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::OnBrowsedirv11() 
{
	_BrowseExe(m_bwexe111);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::OnBrowsedirv12() 
{
	_BrowseExe(m_bwexe112);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::OnBrowsedirv13() 
{
	_BrowseExe(m_bwexe113);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::OnBrowsedirv14() 
{
	_BrowseExe(m_bwexe114);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::OnBrowsedirv15() 
{
	_BrowseExe(m_bwexe115);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::OnBrowsedirv16() 
{
	_BrowseExe(m_bwexe116);
}

//-----------------------------------------------------------------------------------------------------------------

void DlgOptWatch::_Parameters(bool bLoad)
{
	PINT("BWCHART_OPTIONS",timeWait,3000);
	PSTRING("BWCHART_OPTIONS",keyseq,"SEORO*O");
	PSTRING("BWCHART_OPTIONS",keyseq2,"SSORO*O");
	PINT("BWCHART_OPTIONS",timeWait2,1500);
	PSTRING("BWCHART_OPTIONS",bwexe110,"");
	PSTRING("BWCHART_OPTIONS",bwexe109,"");
	PSTRING("BWCHART_OPTIONS",bwexe111,"");
	PSTRING("BWCHART_OPTIONS",bwexe112,"");
	PSTRING("BWCHART_OPTIONS",bwexe113,"");
	PSTRING("BWCHART_OPTIONS",bwexe114,"");
	PSTRING("BWCHART_OPTIONS",bwexe115,"");
	PSTRING("BWCHART_OPTIONS",bwexe116,"");
	PINT("BWCHART_OPTIONS",autoKeys ,1);
	PINT("BWCHART_OPTIONS",autoStartRWA,1);
	PSTRING("BWCHART_OPTIONS",bwplayer,"");
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::OnUpdateControl() 
{
	UpdateData(TRUE);
	
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptWatch::OnBrowseplayer() 
{
	static char BASED_CODE szFilter[] = "Executable (*.exe)|*.exe|All Files (*.*)|*.*||";

 	CFileDialog dlg(TRUE,"exe","",0,szFilter,this);
	dlg.m_ofn.lpstrInitialDir = m_bwplayer;
	if(dlg.DoModal()==IDOK)
	{
		m_bwplayer = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

//--------------------------------------------------------------------------------------------------------------
