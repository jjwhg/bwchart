#include"stdafx.h"
#include"resource.h"
#include"DlgAbout.h"
#include"DlgBWChart.h"
#include"gradient.h"
#include"hsvrgb.h"
				 
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------------------------------

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

//-----------------------------------------------------------------------------------------------------------------

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_URL, m_url);
	//}}AFX_DATA_MAP
}

//-----------------------------------------------------------------------------------------------------------------

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetDlgItemText(IDC_VERSION,"BWChart version " NVERSION " " NBUILD);

	CString url("http://www.bwchart.com");
	m_url.SetWindowText(url);
	m_url.SetURL(url);
	m_url.SetUnderline(FALSE);

	//m_ctrlEmail.SetLinkCursor(AfxGetApp()->LoadCursor(IDC_ABOUT_HYPERLINK));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//-----------------------------------------------------------------------------------------------------------------

BOOL CAboutDlg::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	GetClientRect(&rect);
	COLORREF clr=RGB(255,255,255);
	pDC->FillSolidRect(&rect,clr);
	CRect rectb=rect;
	rectb.right=rectb.left+30;
	COLORREF clr2=RGB(50,50,150); //CHsvRgb::Darker(clr,0.85)
	Gradient::Fill(pDC,rectb,clr,clr2,GRADIENT_FILL_RECT_H);
	return TRUE;
}

//-----------------------------------------------------------------------------------------------------------------

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	pDC->SetBkColor(RGB(255,255,255));
	return (HBRUSH)GetStockObject(WHITE_BRUSH);
}
