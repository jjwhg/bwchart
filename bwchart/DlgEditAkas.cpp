// DlgEditAkas.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgEditAkas.h"
#include "Dlgbwchart.h"
#include "aka.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgEditAkas dialog


DlgEditAkas::DlgEditAkas(AkaList *akalist, Aka *player, int type, CWnd* pParent /*=NULL*/)
	: CDialog(DlgEditAkas::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgEditAkas)
	m_mainName = _T("");
	m_akas = _T("");
	m_suggType = -1;
	m_searchFor = _T("");
	//}}AFX_DATA_INIT

	m_bAskForRefresh=true;
	m_akalist=akalist;
	m_player = player;
	m_type = type;
	if(player!=0) m_initialMainName = player->MainName();
}


void DlgEditAkas::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgEditAkas)
	DDX_Control(pDX, IDC_SUGGESTIONS, m_listSugg);
	DDX_Text(pDX, IDC_MAINNAME, m_mainName);
	DDX_Text(pDX, IDC_AKAS, m_akas);
	DDX_Radio(pDX, IDC_RADIO1, m_suggType);
	DDX_Text(pDX, IDC_SEARCHFOR, m_searchFor);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgEditAkas, CDialog)
	//{{AFX_MSG_MAP(DlgEditAkas)
	ON_BN_CLICKED(IDC_ADDAKA, OnAddaka)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio)
	ON_EN_CHANGE(IDC_MAINNAME, OnChangeMainname)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio)
	ON_EN_CHANGE(IDC_SEARCHFOR, OnChangeSearchfor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------

void DlgEditAkas::OnAddaka() 
{
	UpdateData(TRUE);

	// Get the indexes of all the selected items.
	int nCount = m_listSugg.GetSelCount();
	CUIntArray ar;
	ar.SetSize(nCount);
	m_listSugg.GetSelItems(nCount, (int*)ar.GetData()); 

	for(int i=0; i<nCount; i++)
	{
		CString str;
		m_listSugg.GetText(ar[i],str);
		if(!m_akas.IsEmpty()) m_akas+="\r\n";
		m_akas+=str;
	}

	UpdateData(FALSE);
}

//-----------------------------------------------------------------------------
void DlgEditAkas::OnRadio() 
{
	UpdateData(TRUE);
	_FillSugg();
	AfxGetApp()->WriteProfileInt("BWCHART_AKAS","SUGGTYPE",m_suggType);
}

//-----------------------------------------------------------------------------

void DlgEditAkas::OnOK() 
{
	UpdateData(TRUE);

	// build temporary aka
	Aka tmp(m_mainName);

	// main name changed?
	if(m_mainName!= m_initialMainName)
	{
		// make sure new name does not already exist
		if(m_akalist->GetAkaFromMainName(m_mainName)!=0)
		{
			AfxMessageBox(IDS_AKAEXIST);
			return;
		}
	}

	// add aliases
	char *buffer = _strdup(m_akas);
	char *p=strtok(buffer,"\r\n");
	while(p!=0)
	{
		if(*p) tmp.AddAka(p);
		p=strtok(0,"\r\n");
	}

	// remove old aka
	if(m_player!=0) 
		m_akalist->RemovePlayer(m_player);

	// add new one
	m_player = new Aka(tmp);
	m_akalist->AddPlayer(m_player);

	// free temporary buffer
	free(buffer);

	CDialog::OnOK();
}

//-----------------------------------------------------------------------------

void DlgEditAkas::_FillSugg()
{
	// get players/maps list from browser
	const CStringArray *array = m_type==PLAYERS ? MAINWND->pGetBrowser()->GetPlayers() : MAINWND->pGetBrowser()->GetMaps();
	if(array->GetSize()==0 && m_bAskForRefresh && !MAINWND->pGetBrowser()->IsRefreshDone())
	{
		// we must refresh
		CString players; players.LoadString(IDS_PLAYERS);
		CString maps; maps.LoadString(IDS_MAPS);
		CString msg;
		msg.Format(IDS_REFRESHBROWSER,m_type==PLAYERS?(const char*)players:(const char*)maps);
		if(MessageBox(msg,0,MB_YESNO)==IDOK)
			MAINWND->pGetBrowser()->Refresh(false);
	}

	// clear list
	m_listSugg.ResetContent();

	// prepare strings for suggestions
	CString search(m_mainName);
	search.MakeLower();
	CString search2(m_searchFor);
	search2.MakeLower();

	// browse players/maps list
	for(int i=0; i<array->GetSize(); i++)
	{
		CString entry = (CString)array->GetAt(i);
		if(m_suggType==0)
		{
			// is it a correct suggestion?
			CString low(entry);
			low.MakeLower();
			if(low.Find(search)>=0)
				m_listSugg.AddString(entry);
			else if(!search2.IsEmpty() && low.Find(search2)>=0)
				m_listSugg.AddString(entry);
		}
		else
		{
			m_listSugg.AddString(entry);
		}
	}
}

//-----------------------------------------------------------------------------

BOOL DlgEditAkas::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_PLAYERNAME,m_type==PLAYERS?"Player main name":"Map main name");
	
	if(m_player!=0)
	{
		// player name
		m_mainName = m_player->MainName();

		// akas
		m_akas = "";
		for(int i=0;i<m_player->GetAkaCount();i++)
		{
			if(i>0) m_akas+="\r\n";
			m_akas += m_player->GetAka(i);
		}

	}
		 
	// fill suggestion list
	m_suggType = AfxGetApp()->GetProfileInt("BWCHART_AKAS","SUGGTYPE",0);
	_FillSugg();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//-----------------------------------------------------------------------------

void DlgEditAkas::OnChangeMainname() 
{
	OnChangeSearchfor();
}

//-----------------------------------------------------------------------------

void DlgEditAkas::OnChangeSearchfor() 
{
	if(m_suggType==0)
	{
		UpdateData(TRUE);
		_FillSugg();
	}
}

//-----------------------------------------------------------------------------
