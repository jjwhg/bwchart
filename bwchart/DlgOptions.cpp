// DlgOptions.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgOptions.h"
#include "Dlgbwchart.h"
#include "dirutil.h"
#include "bwdb.h"
#include "chkupdate.h"
#include "regparam.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern unsigned long gSuspectLimit;

//----------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DlgOptions, CDialog)
	//{{AFX_MSG_MAP(DlgOptions)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_BN_CLICKED(IDC_CHECKUPDATE, OnCheckupdate)
	ON_BN_CLICKED(IDC_FILEASSO, OnFileasso)
	ON_BN_CLICKED(IDC_RADIOSAVE, OnRadiosave)
	ON_EN_CHANGE(IDC_SUSPECT_LIMIT, OnChangeSuspectLimit)
	ON_BN_CLICKED(IDC_RADIOSAVE2, OnRadiosave)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_AUTOADD, OnUpdateControl)
	ON_BN_CLICKED(IDC_AUTOCHECK, OnUpdateControl)
	ON_BN_CLICKED(IDC_AUTOKEYS, OnUpdateControl)
	ON_BN_CLICKED(IDC_AUTOSTARTRWA, OnUpdateControl)
	ON_BN_CLICKED(IDC_AUTOLOAD, OnUpdateControl)
	ON_BN_CLICKED(IDC_RADIO1, OnLanguageChange)
	ON_BN_CLICKED(IDC_RADIO2, OnLanguageChange)
	ON_BN_CLICKED(IDC_RADIO3, OnLanguageChange)
	ON_BN_CLICKED(IDC_RADIO4, OnLanguageChange)
END_MESSAGE_MAP()

//----------------------------------------------------------------------------------------------

void DlgOptions::_DefaultPath(CString& path, const char *defexe)
{
	if(path.IsEmpty()) 
	{
		_GetStarcraftPath(path);
		if(path.Right(1)!="\\") path += "\\";
		path += defexe;
	}
}

//-----------------------------------------------------------------------------------------------------------------

void DlgOptions::_Parameters(bool bLoad)
{
	PINT("BWCHART_OPTIONS",autoCheck ,1);
	PINT("BWCHART_OPTIONS",fileasso,1);
	PINT("BWCHART_OPTIONS",savein, 0);
	PINT("BWCHART_OPTIONS",autoadd, 1);
	PINT("BWCHART_OPTIONS",suspectLimit2, 2);
	PINT("BWCHART_OPTIONS",autoLoad,1);
	PINT("BWCHART_OPTIONS",autoLoadDB,1);
	PINT("BWCHART_OPTIONS",language,0);
	if(bLoad) gSuspectLimit = m_suspectLimit2;
}

//----------------------------------------------------------------------------------------------

DlgOptions::DlgOptions(CWnd* pParent /*=NULL*/)
	: CDialog(DlgOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgOptions)
	m_autoCheck = FALSE;
	m_savein = -1;
	m_fileasso = FALSE;
	m_autoadd = FALSE;
	m_suspectLimit2 = 0;
	m_autoLoad = FALSE;
	m_autoLoadDB = FALSE;
	m_language = -1;
	//}}AFX_DATA_INIT

	_Parameters(true);

}


void DlgOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgOptions)
	DDX_Check(pDX, IDC_AUTOCHECK, m_autoCheck);
	DDX_Radio(pDX, IDC_RADIOSAVE, m_savein);
	DDX_Check(pDX, IDC_FILEASSO, m_fileasso);
	DDX_Check(pDX, IDC_AUTOADD, m_autoadd);
	DDX_Text(pDX, IDC_SUSPECT_LIMIT, m_suspectLimit2);
	DDX_Check(pDX, IDC_AUTOLOAD, m_autoLoad);
	DDX_Check(pDX, IDC_AUTOLOADDB, m_autoLoadDB);
	DDX_Radio(pDX, IDC_RADIO1, m_language);
	//}}AFX_DATA_MAP
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptions::_GetStarcraftPath(CString& path) 
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

void DlgOptions::InnerSaveValues()
{
	UpdateData(TRUE);
	_Parameters(false);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptions::OnDestroy() 
{
	InnerSaveValues();
	CDialog::OnDestroy();
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptions::OnClear() 
{
	if(BWChartDB::ClearDatabase())
		MAINWND->pGetBrowser()->PostMessage(WM_AUTOREFRESH);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptions::OnCheckupdate() 
{
	CheckForUpdate(false);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptions::CheckForUpdate(bool automatic)
{
	Updater upd(NVERSION);
	upd.CheckForUpdateBW(automatic);
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptions::OnFileasso() 
{
	UpdateData(TRUE);
	MAINWND->UpdateFileAssociation();
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptions::OnRadiosave() 
{
	UpdateData(TRUE);	
	if(AfxMessageBox(IDS_MOVEREP,MB_YESNO)==IDYES)
	{
		BWChartDB::UpdateDatabaseDir(m_savein==0);
	}
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptions::OnUpdateControl() 
{
	UpdateData(TRUE);
	
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptions::OnChangeSuspectLimit() 
{
	UpdateData(TRUE);
	gSuspectLimit = m_suspectLimit2;
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptions::OnLanguageChange() 
{
	UpdateData(TRUE);
	MessageBox("Please restart BWChart for the language change to take effect");
}

//--------------------------------------------------------------------------------------------------------------

void DlgOptions::SetLanguage() 
{
	int langage=AfxGetApp()->GetProfileInt("BWCHART_OPTIONS","language",0);

	if(langage==1)
		SetThreadLocale(MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT ));
	else if(langage==2)
		SetThreadLocale(MAKELCID(MAKELANGID(LANG_KOREAN,SUBLANG_KOREAN),SORT_DEFAULT ));
	else if(langage==3)
		SetThreadLocale(MAKELCID(MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED),SORT_DEFAULT ));
	
}

//--------------------------------------------------------------------------------------------------------------

HWND list1=0;
HWND list2=0;
HWND edit=0;
HWND button=0;

int childCount=0;

BOOL CALLBACK EnumChildProc(
  HWND hwnd,      // handle to child window
  LPARAM lParam   // application-defined value
)
{
	childCount++;

	char classname[255];
	GetClassName(hwnd,classname,sizeof(classname));

	if(strcmp(classname,"ListBox")==0)
	{
		if(list1==0) list1 = hwnd;
		else list2=hwnd;
	}
	else if(strcmp(classname,"Edit")==0)
	{
		edit = hwnd;
	}
	else if(strcmp(classname,"Button")==0)
	{
		char text[128];
		GetWindowText(hwnd,text,sizeof(text));
		if(_stricmp(text,"&send")==0)
			button=hwnd;
	}

	return TRUE;
}
 

BOOL CALLBACK EnumWindowsProc(
  HWND hwnd,      // handle to parent window
  LPARAM lParam   // application-defined value
)
{
	char classname[255];
	GetClassName(hwnd,classname,sizeof(classname));

	if(strcmp(classname,"SDlgDialog")==0 && list1==0)
	{
		childCount=0;
		EnumChildWindows(hwnd,EnumChildProc,0);
		if(childCount>1)
		{
			return FALSE;
		}
	}

	return TRUE;
}

void DlgOptions::OnButton1() 
{
	edit=0;
	list1=0;
	list2=0;
	button = 0;
	EnumWindows( EnumWindowsProc, 0);

	if(list1!=0)
	{
		CListBox list;
		list.Attach(list1);
		int sel=list.GetCurSel();
		if(sel!=CB_ERR)
		{
			CString str;
			list.GetText(sel,str);

			char *p=(char*)strchr(str,0x09);
			if(p!=0) *p=0;
			p=(char*)strchr(str,'#');
			if(p!=0) *p=0;

			str = "/squelch "+str;

			CEdit medit;
			medit.Attach(edit);
			//medit.SetWindowText(str);
			medit.Detach();

			::PostMessage(edit,WM_KEYDOWN,VK_DIVIDE,0);
			//::PostMessage(edit,WM_KEYUP,VK_DIVIDE,0);
			char c;
			str.MakeUpper();
			for(int i=0;i<(int)strlen(str);i++)
			{
				c = str[i];
				if(c=='/') c=VK_DIVIDE;
				::PostMessage(edit,WM_KEYDOWN,c,0);
				//::PostMessage(edit,WM_KEYUP,c,0);
			}

			//::PostMessage(button,WM_LBUTTONDOWN,0,0);
			//::PostMessage(button,WM_LBUTTONUP,0,0);
		}

		list.Detach();
	}
	
}

//--------------------------------------------------------------------------------------------------------------

