// DlgHelp.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgHelp dialog


DlgHelp::DlgHelp(CWnd* pParent /*=NULL*/)
	: CDialog(DlgHelp::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgHelp)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void DlgHelp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgHelp)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgHelp, CDialog)
	//{{AFX_MSG_MAP(DlgHelp)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgHelp message handlers
