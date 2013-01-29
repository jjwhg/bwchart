// DlgFastAka.cpp : implementation file
//

#include "stdafx.h"
#include "bwchart.h"
#include "DlgFastAka.h"
#include "DlgBWChart.h"
#include "replaydb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(DlgFastAka, CDialog)
	//{{AFX_MSG_MAP(DlgFastAka)
	ON_BN_CLICKED(IDC_ASSO1, OnAsso1)
	ON_BN_CLICKED(IDC_ASSO2, OnAsso2)
	ON_BN_CLICKED(IDC_ASSO3, OnAsso3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//----------------------------------------------------------------------------------------------------------

DlgFastAka::DlgFastAka(ReplayInfo *replay, CWnd* pParent /*=NULL*/)
	: CDialog(DlgFastAka::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgFastAka)
	m_player2 = _T("");
	m_player1 = _T("");
	m_map = _T("");
	//}}AFX_DATA_INIT
	m_replay = replay;
	m_bAkasModified=false;
}

//----------------------------------------------------------------------------------------------------------

void DlgFastAka::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgFastAka)
	DDX_Control(pDX, IDC_LISTMAPS, m_maps);
	DDX_Control(pDX, IDC_LISTPLAYERS2, m_players2);
	DDX_Control(pDX, IDC_LISTPLAYERS1, m_players1);
	DDX_Text(pDX, IDC_PLAYER2, m_player2);
	DDX_Text(pDX, IDC_PLAYER1, m_player1);
	DDX_Text(pDX, IDC_MAP, m_map);
	//}}AFX_DATA_MAP
}

//----------------------------------------------------------------------------------------------------------

void DlgFastAka::_AddAka(CComboBox *combo, const char *newaka, AkaList *akalist, UINT btid, UINT txid) 
{
	UpdateData(TRUE);

	// get selection
	int sel = combo->GetCurSel();
	if(sel<0) return;

	// add aka to aka list
	Aka *aka = (Aka *)combo->GetItemData(sel);
	akalist->AddAka(aka,newaka);

	// save new player
	akalist->Save(aka);

	// disable combo
	_DisableCombo(combo, btid, txid);

	// update akas & browser screens when we leave
	m_bAkasModified=true;
}

//----------------------------------------------------------------------------------------------------------

void DlgFastAka::OnAsso1() 
{
	UpdateData(TRUE);
	_AddAka(&m_players1, m_player1, &MAINWND->pGetAkas()->m_akalist,IDC_ASSO1,IDC_TEXT1);
}

void DlgFastAka::OnAsso2() 
{
	UpdateData(TRUE);
	_AddAka(&m_players2, m_player2, &MAINWND->pGetAkas()->m_akalist,IDC_ASSO2,IDC_TEXT2);
}

void DlgFastAka::OnAsso3() 
{
	UpdateData(TRUE);
	_AddAka(&m_maps, m_map, &MAINWND->pGetAkas()->m_akalistMap,IDC_ASSO3,IDC_TEXT3);
}

//----------------------------------------------------------------------------------------------------------

void DlgFastAka::_DisableCombo(CComboBox *combo, UINT btid, UINT txid)
{
	combo->EnableWindow(FALSE);
	GetDlgItem(btid)->ShowWindow(SW_HIDE);
	GetDlgItem(txid)->ShowWindow(SW_SHOW);
}

//----------------------------------------------------------------------------------------------------------

void DlgFastAka::_PrepareCombo(CComboBox *combo, AkaList *akalist, const char *straka, const char *regentry, UINT btid, UINT txid)
{
	const Aka *aka = akalist->GetAkaFromName(straka);
	const char *name = aka==0 ? 0 : aka->MainName();
	akalist->FillCombo(combo,regentry,name);
	if(name!=0) _DisableCombo(combo, btid, txid);
}

//----------------------------------------------------------------------------------------------------------

BOOL DlgFastAka::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// fill replay info
	m_player1 = m_replay->m_player[0];
	m_player2 = m_replay->m_player[1];
	m_map = m_replay->m_map;

	// fill player 1 combo
	AkaList *akalist = &MAINWND->pGetAkas()->m_akalist;
	AkaList *akalistmap = &MAINWND->pGetAkas()->m_akalistMap;
	_PrepareCombo(&m_players1, akalist, m_player1, "SELPLAYER", IDC_ASSO1, IDC_TEXT1);

	// fill player 2 combo
	_PrepareCombo(&m_players2, akalist, m_player2, "SELPLAYER", IDC_ASSO2, IDC_TEXT2);

	// fill map combo
	_PrepareCombo(&m_maps, akalistmap, m_map, "SELMAP", IDC_ASSO3, IDC_TEXT3);
	
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//----------------------------------------------------------------------------------------------------------

void DlgFastAka::OnCancel() 
{
	if(m_bAkasModified)
	{
		// update akas
		MAINWND->pGetAkas()->RefreshAkas();

		// update browser
		MAINWND->pGetBrowser()->RefreshAkas();
	}
	
	CDialog::OnCancel();
}

//----------------------------------------------------------------------------------------------------------

