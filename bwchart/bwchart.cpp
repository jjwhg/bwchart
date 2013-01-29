// bwchart.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "bwchart.h"
#include "Dlgbwchart.h"
#include "replay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CBwchartApp

BEGIN_MESSAGE_MAP(CBwchartApp, CWinApp)
	//{{AFX_MSG_MAP(CBwchartApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBwchartApp construction

CBwchartApp::CBwchartApp() : m_exitCode(0)
{

}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBwchartApp object

CBwchartApp theApp;


//------------------------------------------------------------------------------------

BOOL CBwchartApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.


	SetRegistryKey("bwchart");

	// Enable drag/drop open
	//m_pMainWnd->DragAcceptFiles();

	
	if(strstr(AfxGetApp()->m_lpCmdLine,"-korean")!=0)
		SetThreadLocale(MAKELCID(MAKELANGID(LANG_KOREAN,SUBLANG_KOREAN),SORT_DEFAULT ));
	else if(strstr(AfxGetApp()->m_lpCmdLine,"-english")!=0)
		SetThreadLocale(MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT ));
	else if(strstr(AfxGetApp()->m_lpCmdLine,"-chinese")!=0)
		SetThreadLocale(MAKELCID(MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED),SORT_DEFAULT ));
	else
		DlgOptions::SetLanguage();
	
	// corrupted file
	CString corrupted = AfxGetApp()->GetProfileString("LOG","LASTREP","");
	if(!corrupted.IsEmpty()) 
	{
		CString msg;
		msg.Format(IDS_CORRUPTED3,(const char*)corrupted);
		AfxMessageBox(msg,MB_OK|MB_ICONEXCLAMATION);
		AfxGetApp()->WriteProfileString("LOG","LASTREP","");
		AfxGetApp()->WriteProfileString("MAIN","LASTREPLAY","");
	}

	// command line mode?
	if(strstr(m_lpCmdLine,"/rep")!=0)
	{
		m_exitCode = /*m_msgCur.wParam =*/ _CommandLineMode();
	}
	else
	{
		// windowed mode
		DlgBWChart dlg(m_lpCmdLine);
		m_pMainWnd = &dlg;
		dlg.DoModal();
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

//------------------------------------------------------------------------------------

// Cmd line format : /rep="replay file" [/sec] [/sep=,] [/txt="text file"]
//
// /sec               : use seconds instead of ticks
// /sep=X             : specifies X as the column separator (don't use ',')
// /rep="replay file" : replay to analyse (use full path unless bwchart.exe is located in same directory)
// /txt="text file"   : output text file
//
// default separator is TAB
// default text file is rep file with .txt extension instead of .rep extension
// default time measurement is ticks
//
// Always put replay file name and output text file name between double quotes (eg:"c:\dir\gg.rep").
//
// Return code: 0 if ok, 1 for invalid parameter, -1 for output file cant be created, -2 for replay cant be loaded
//
int CBwchartApp::_CommandLineMode()
{
	char *argv[4];
	int argc=0;
	int err = 0;

	// extract arguments
	char *p = m_lpCmdLine;
	bool inString =false;
	while(*p!=0)
	{
		argv[argc] = p;
		while(true)
		{
			p++;
			if(*p=='\"') inString=!inString;
			else if(*p==' ' && !inString) {*p=0; p++; argc++; break;}
		}
		while(*p==' ') p++;
	}

	// analyse arguments
	CString repfile;
	CString txtfile;
	bool useSeconds = false;
	char cSep = '\t';
	for(int i=0; i<argc; i++)
	{
		// extract option and value
		char *option = strtok(argv[i],"=");
		char *val = strtok(0,"=");

		// lowercase option
		_strlwr(option);
		if(strcmp(option,"/sec")==0)
			useSeconds = true;
		else if(strcmp(option,"/sep")==0)
		{
			if(val==0) err = 1;
			cSep = val[0];
		}
		else if(strcmp(option,"/rep")==0)
		{
			if(val==0) err = 1;
			repfile = val;
			repfile.Replace('\"',' ');
			repfile.TrimLeft();
			repfile.TrimRight();
		}
		else if(strcmp(option,"/txt")==0)
		{
			if(val==0) err = 1;
			txtfile = val;
			txtfile.Replace('\"',' ');
			txtfile.TrimLeft();
			txtfile.TrimRight();
		}
	}

	// if text file is missing
	if(err == 0 && txtfile.IsEmpty())
	{
		// use replay file name with txt extension
		txtfile = repfile;
		txtfile.Replace(".rep",".txt");
		txtfile.Replace(".REP",".txt");
	}

	// if we have a replay to export
	if(!repfile.IsEmpty())
	{
		Replay replay;
		if(replay.Load(repfile,true,0,true)!=0)
			err = -2;
		else
			err = replay.ExportToText(txtfile,useSeconds,cSep);
	}

	return err;
}

//------------------------------------------------------------------------------------

int CBwchartApp::ExitInstance() 
{
	DESTROY_LOCAL_HEAP(BWElement);
	DESTROY_LOCAL_HEAP(ReplayEvt);

	CWinApp::ExitInstance();
	return m_exitCode;
}

//------------------------------------------------------------------------------------
