// DlgEditComment.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgEditComment.h"
#include "replaydb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgEditComment dialog


DlgEditComment::DlgEditComment(class ReplayInfo *rep, CWnd* pParent /*=NULL*/)
	: CDialog(DlgEditComment::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgEditComment)
	m_comment = _T("");
	//}}AFX_DATA_INIT
	m_rep = rep;
	m_comment = m_rep->m_comment;
}


void DlgEditComment::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgEditComment)
	DDX_Text(pDX, IDC_EDIT1, m_comment);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgEditComment, CDialog)
	//{{AFX_MSG_MAP(DlgEditComment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgEditComment message handlers

void DlgEditComment::OnOK() 
{
	UpdateData(TRUE);
	m_rep->m_comment = m_comment;
	CDialog::OnOK();
}

BOOL DlgEditComment::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// limit comments to 256 characters
	((CEdit*)GetDlgItem(IDC_EDIT1))->SetLimitText(256);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
