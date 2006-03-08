// KeyConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "psycle2.h"
#include "KeyConfigDlg.h"
#include "inputhandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyConfigDlg dialog


CKeyConfigDlg::CKeyConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKeyConfigDlg::IDD, pParent)
{
	m_prvIdx = 0;
	
	//{{AFX_DATA_INIT(CKeyConfigDlg)
	//}}AFX_DATA_INIT

}


void CKeyConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeyConfigDlg)
	DDX_Control(pDX, IDC_SHIFTARROWS, m_cmdShiftArrows);
	DDX_Control(pDX, IDC_FT2DEL, m_cmdFT2Del);
	DDX_Control(pDX, IDC_FT2_HOME_END, m_cmdNewHomeBehaviour);
	DDX_Control(pDX, IDC_PLAY_R_CTRL, m_cmdCtrlPlay);
	DDX_Control(pDX, IDC_CMDLIST, m_lstCmds);
	DDX_Control(pDX, IDC_KEY0, m_hotkey0);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKeyConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CKeyConfigDlg)
	ON_LBN_SELCHANGE(IDC_CMDLIST, OnSelchangeCmdlist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyConfigDlg message handlers

void CKeyConfigDlg::DoCommandList()
{

}

void CKeyConfigDlg::FillCmdList()
{
    // clear list box
    m_lstCmds.ResetContent();

    // add command definitions
    CString cmdDefn;
    int i;
	CmdDef cmd;
    for(i=0;i<1023;i++)
    {
		cmd.ID = CmdSet(i);
		cmdDefn = cmd.GetName();
		if(cmdDefn!="Null" && cmdDefn!="-" && cmdDefn!="" && cmdDefn!="Invalid")
	    m_lstCmds.AddString(LPCTSTR(cmdDefn));
    }        
}

BOOL CKeyConfigDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_cmdCtrlPlay.SetCheck(Global::pInputHandler->bCtrlPlay?1:0);
	m_cmdNewHomeBehaviour.SetCheck(Global::pInputHandler->bFT2HomeBehaviour?1:0);
	m_cmdFT2Del.SetCheck(Global::pInputHandler->bFT2DelBehaviour?1:0);
	m_cmdShiftArrows.SetCheck(Global::pInputHandler->bShiftArrowsDoSelect?1:0);

	// prevent ALT in hotkey	

//	TRACE("%d %d %d %d %d %d\n",HKCOMB_A,HKCOMB_C,HKCOMB_S,HKCOMB_CA,HKCOMB_SA,HKCOMB_SCA);

	WORD rules=HKCOMB_A|HKCOMB_CA|HKCOMB_SA|HKCOMB_SCA;
	WORD subst=0;

	m_hotkey0.SetRules(rules,subst);

	FillCmdList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



// update key on show

void CKeyConfigDlg::OnSelchangeCmdlist() 
{	
	
	WORD key = 0;
	WORD mods = 0;	
	
	// save key settings for key we've just moved from
	FindKey(m_prvIdx,key,mods);
	SaveHotKey(m_prvIdx,key,mods);
	
	// update display for new key
	m_prvIdx = m_lstCmds.GetCurSel();
	FindKey(m_prvIdx,key,mods);
	m_hotkey0.SetHotKey(key,mods);
}

void CKeyConfigDlg::SaveHotKey(long idx,WORD&key,WORD&mods)
{
	// what key is selected in the hotkey?
	WORD hotkey_key = 0;
	WORD hotkey_mods = 0;
	m_hotkey0.GetHotKey(hotkey_key,hotkey_mods);

	// nothing selected? abort
	if(!hotkey_key)
		return;		
	
	UINT nKey=hotkey_key;
	UINT nMod=0;
	if(hotkey_mods&HOTKEYF_SHIFT)
		nMod|=MOD_S;
	if(hotkey_mods&HOTKEYF_CONTROL)
		nMod|=MOD_C;
	if(hotkey_mods&HOTKEYF_EXT)
		nMod|=MOD_E;

	// same as previous? do nothing
	if(nKey==key && nMod==mods)
		return;

	// what command is selected?
	CmdDef cmd = FindCmd(idx);
	
	// save key definition
	if(cmd.IsValid())
		Global::pInputHandler->SetCmd(cmd,nKey,nMod);
}

void CKeyConfigDlg::FindKey(long idx,WORD&key,WORD&mods)
{
	// what command is selected?
	CmdDef cmd = FindCmd(idx);
	
	// locate which key is that command
	if(cmd.IsValid())
		Global::pInputHandler->CmdToKey(cmd,key,mods);
}

CmdDef CKeyConfigDlg::FindCmd(long idx)
{	
	// init
	CmdDef cmd;

	// sanity check
	if(idx<0) 
		return cmd;				

	// get command string
	CString cmdDesc;
	m_lstCmds.GetText(idx,cmdDesc);

	// convert string to cmd
	cmd = Global::pInputHandler->StringToCmd(cmdDesc);
	return cmd;
}



void CKeyConfigDlg::OnCancel() 
{
	// user cancelled,
	// restore from saved settings
	Global::pInputHandler->ConfigRestore();
	CDialog::OnCancel();
}

void CKeyConfigDlg::OnOK() 
{
	// update last key
	OnSelchangeCmdlist();

	Global::pInputHandler->bCtrlPlay  = m_cmdCtrlPlay.GetCheck()?true:false;
	Global::pInputHandler->bFT2HomeBehaviour = m_cmdNewHomeBehaviour.GetCheck()?true:false;
	Global::pInputHandler->bFT2DelBehaviour = m_cmdFT2Del.GetCheck()?true:false;
	Global::pInputHandler->bShiftArrowsDoSelect = m_cmdShiftArrows.GetCheck()?true:false;
	
	// save settings
	Global::pInputHandler->ConfigSave();
	

	CDialog::OnOK();
}
