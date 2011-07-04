/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.5 $
 */
#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "KeyConfigDlg.h"
#include "inputhandler.h"
#include "configuration.h"
#include ".\keyconfigdlg.h"



/////////////////////////////////////////////////////////////////////////////
// CKeyConfigDlg dialog


//IMPLEMENT_DYNCREATE(CKeyConfigDlg, CPropertyPage)

CKeyConfigDlg::CKeyConfigDlg() 
{
	m_prvIdx = 0;
	
	//{{AFX_DATA_INIT(CKeyConfigDlg)
	//}}AFX_DATA_INIT
	bInit = FALSE;
}






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
		if(cmdDefn!=_T("Null") && cmdDefn!=_T("-") && cmdDefn!=_T("") && cmdDefn!=_T("Invalid"))
		    m_lstCmds.AddString(LPCTSTR(cmdDefn));
    }        
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



BOOL CKeyConfigDlg::OnQueryCancel() 
{
	// user cancelled,
	// restore from saved settings
	Global::pInputHandler->ConfigRestore();
	return TRUE;
}

int CKeyConfigDlg::OnApply()
{
	// update last key
	BOOL _b = FALSE;
	OnLbnSelchangeCmdlist(NULL,NULL,NULL,_b);

	Global::pInputHandler->bCtrlPlay  = m_cmdCtrlPlay.GetCheck()?true:false;
	Global::pInputHandler->bFT2HomeBehaviour = m_cmdNewHomeBehaviour.GetCheck()?true:false;
	Global::pInputHandler->bFT2DelBehaviour = m_cmdFT2Del.GetCheck()?true:false;
	Global::pInputHandler->bShiftArrowsDoSelect = m_cmdShiftArrows.GetCheck()?true:false;
	
	// save settings
	Global::pInputHandler->ConfigSave();
	
	Global::pConfig->_wrapAround = m_wrap.GetCheck()?true:false;
	Global::pConfig->_centerCursor = m_centercursor.GetCheck()?true:false;
	Global::pConfig->_cursorAlwaysDown = m_cursordown.GetCheck()?true:false;

	Global::pConfig->bFileSaveReminders = m_save_reminders.GetCheck()?true:false;
	Global::pConfig->_RecordMouseTweaksSmooth = m_tweak_smooth.GetCheck()?true:false;
	Global::pConfig->_RecordUnarmed = m_record_unarmed.GetCheck()?true:false;
	
	Global::pConfig->bShowSongInfoOnLoad = m_show_info.GetCheck()?true:false;
	Global::pConfig->autosaveSong = m_autosave.GetCheck()?true:false;
	
	TCHAR buffer[32];
	m_numlines.GetWindowText(buffer,16);
	
	int nlines = _tstoi(buffer);

	if (nlines < 1)
		{ nlines = 1; }
	else if (nlines > MAX_LINES)
		{ nlines = MAX_LINES; }

	Global::pConfig->defaultPatLines=nlines;

	m_autosave_mins.GetWindowText(buffer,16);
	nlines = _tstoi(buffer);
	if (nlines < 1)
	{ nlines = 1; }
	else if (nlines > 60)
	{ nlines = 60; }
	Global::pConfig->autosaveSongTime = nlines;
	
	return PSNRET_NOERROR;
}


LRESULT CKeyConfigDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_autosave_spin.Attach(GetDlgItem(IDC_AUTOSAVE_MINS_SPIN));
	m_autosave_mins.Attach(GetDlgItem(IDC_AUTOSAVE_MINS));
	m_autosave.Attach(GetDlgItem(IDC_AUTOSAVE_CURRENT_SONG));
	m_save_reminders.Attach(GetDlgItem(IDC_FILE_SAVE_REMINDERS));
	m_tweak_smooth.Attach(GetDlgItem(IDC_TWEAK_SMOOTH));
	m_record_unarmed.Attach(GetDlgItem(IDC_RECORD_UNARMED));
	m_show_info.Attach(GetDlgItem(IDC_SHOW_INFO_ON_LOAD));
	m_cmdShiftArrows.Attach(GetDlgItem(IDC_SHIFTARROWS));
	m_cmdFT2Del.Attach(GetDlgItem(IDC_FT2DEL));
	m_cmdNewHomeBehaviour.Attach(GetDlgItem(IDC_FT2_HOME_END));
	m_cmdCtrlPlay.Attach(GetDlgItem(IDC_PLAY_R_CTRL));
	m_lstCmds.Attach(GetDlgItem(IDC_CMDLIST));
	m_hotkey0.Attach(GetDlgItem(IDC_KEY0));
	m_wrap.Attach(GetDlgItem(IDC_WRAP));
	m_centercursor.Attach(GetDlgItem(IDC_CENTERCURSOR));
	m_cursordown.Attach(GetDlgItem(IDC_CURSORDOWN));
	m_numlines.Attach(GetDlgItem(IDC_EDIT_DEFLINES));
	m_spinlines.Attach(GetDlgItem(IDC_SPIN_DEFLINES));
	m_textlines.Attach(GetDlgItem(IDC_TEXT_DEFLINES));

	m_cmdCtrlPlay.SetCheck(Global::pInputHandler->bCtrlPlay?1:0);
	m_cmdNewHomeBehaviour.SetCheck(Global::pInputHandler->bFT2HomeBehaviour?1:0);
	m_cmdFT2Del.SetCheck(Global::pInputHandler->bFT2DelBehaviour?1:0);
	m_cmdShiftArrows.SetCheck(Global::pInputHandler->bShiftArrowsDoSelect?1:0);

	m_save_reminders.SetCheck(Global::pConfig->bFileSaveReminders?1:0);
	m_tweak_smooth.SetCheck(Global::pConfig->_RecordMouseTweaksSmooth?1:0);
	m_record_unarmed.SetCheck(Global::pConfig->_RecordUnarmed?1:0);
	m_show_info.SetCheck(Global::pConfig->bShowSongInfoOnLoad?1:0);
	m_autosave.SetCheck(Global::pConfig->autosaveSong?1:0);
	
	// prevent ALT in hotkey	

//	TRACE(_T("%d %d %d %d %d %d\n"),HKCOMB_A,HKCOMB_C,HKCOMB_S,HKCOMB_CA,HKCOMB_SA,HKCOMB_SCA);

	WORD rules=HKCOMB_A|HKCOMB_CA|HKCOMB_SA|HKCOMB_SCA;
	WORD subst=0;

	m_hotkey0.SetRules(rules,subst);

	m_wrap.SetCheck(Global::pConfig->_wrapAround?1:0);
	m_centercursor.SetCheck(Global::pConfig->_centerCursor?1:0);
	m_cursordown.SetCheck(Global::pConfig->_cursorAlwaysDown?1:0);

	m_spinlines.SetRange(1,MAX_LINES);
	m_autosave_spin.SetRange(1,60);
	
	TCHAR buffer[16];
	_itot(Global::pConfig->defaultPatLines,buffer,10);
	m_numlines.SetWindowText(buffer);
	_itot(Global::pConfig->autosaveSongTime,buffer,10);
	m_autosave_mins.SetWindowText(buffer);
	
	UDACCEL acc;
	acc.nSec = 4;
	acc.nInc = 16;
	m_spinlines.SetAccel(1, &acc);
	
	bInit = TRUE;
	BOOL bt_ = FALSE;
	OnEnChangeEditDeflines(NULL,NULL,NULL,bt_);

	FillCmdList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE

}

LRESULT CKeyConfigDlg::OnBnClickedNone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{

	// TODO: Add your control notification handler code here

	WORD key = 0;
	WORD mods = 0;	
	
	// save key settings for key we've just moved from
	FindKey(m_prvIdx,key,mods);

	UINT nKey=key;
	UINT nMod=0;
	if(mods&HOTKEYF_SHIFT)
		nMod|=MOD_S;
	if(mods&HOTKEYF_CONTROL)
		nMod|=MOD_C;
	if(mods&HOTKEYF_EXT)
		nMod|=MOD_E;

	Global::pInputHandler->SetCmd(cdefNull,nKey,nMod);
	
	m_hotkey0.SetHotKey(0,0);

	return 0;
}

LRESULT CKeyConfigDlg::OnBnClickedDefaults3(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO : ここにコントロール通知ハンドラ コードを追加します。

	return 0;
}

LRESULT CKeyConfigDlg::OnBnClickedDefaults(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	Global::pInputHandler->BuildCmdLUT();

	WORD key = 0;
	WORD mods = 0;	
	
	// update display for new key
	m_prvIdx = m_lstCmds.GetCurSel();
	FindKey(m_prvIdx,key,mods);
	m_hotkey0.SetHotKey(key,mods);

	return 0;
}

LRESULT CKeyConfigDlg::OnBnClickedImportreg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	
	// restore key config data
	// returns true on success, false on fail 
	// TODO: キーコンフィグデータのリードライトをチェックする
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[_MAX_PATH];       // buffer for file name
	
	szFile[0] = _T('\0');
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetParent();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("Psycle Keymap Presets\0*.psk\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrInitialDir = _skinPathBuf;
	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn)==TRUE)
	{
		FILE* hfile;
		if ((hfile=_tfopen(szFile,_T("rwt"))) == NULL )
		{
			MessageBox(SF::CResourceString(IDS_ERR_MSG0070)
				,SF::CResourceString(IDS_ERR_MSG0071),MB_OK);
			return FALSE;
		}
		TCHAR buf[512];
		while (_fgetts(buf, 512, hfile))
		{
			UINT i,j;
			if (_tcsstr(buf,_T("Key[")))
			{
				TCHAR *q = _tcschr(buf,91); // [
				if (q)
				{
					q++;
					j = _tstoi(q);
					TCHAR *p = _tcschr(q,93); // ]
					if (p)
					{
						p++;
						i = _tstoi(p);
						q = _tcschr(p,61); // =
						if (q)
						{
							q++;
							int cmddata = _tstoi(q);
							CmdSet ID = CmdSet(cmddata);
							Global::pInputHandler->SetCmd(ID,i,j);
						}
					}
				}
			}
		}
		fclose(hfile);
	}
//	FillCmdList();
//	OnSelchangeCmdlist();
	WORD key = 0;
	WORD mods = 0;	
	
	// update display for new key
	m_prvIdx = m_lstCmds.GetCurSel();
	FindKey(m_prvIdx,key,mods);
	m_hotkey0.SetHotKey(key,mods);
	return 0;
}

LRESULT CKeyConfigDlg::OnBnClickedExportreg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
// save key config data
// returns true on success, false on fail
	// TODO: Add your control notification handler code here
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[_MAX_PATH];       // buffer for file name
	szFile[0]=_T('\0');
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)GetParent();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("Psycle Keymap Presets\0*.psk\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;	
	ofn.lpstrInitialDir = _skinPathBuf;

	if (GetSaveFileName(&ofn)==TRUE)
	{
		FILE* hfile;

		CString str = szFile;
		CString str2 = str.Right(4);
		if ( str2.CompareNoCase(_T(".psk")) != 0 ) str.Insert(str.GetLength(),_T(".psk"));
		_stprintf(szFile,str);
		DeleteFile(szFile);

		if ((hfile = _tfopen(szFile,_T("wa"))) == NULL ) // file does not exist.
		{
			MessageBox(SF::CResourceString(IDS_ERR_MSG0074)
				,SF::CResourceString(IDS_ERR_MSG0075),MB_OK);
			return FALSE;
		}

		_ftprintf(hfile,_T("[Psycle Keymap Presets v1.0]\n\n"));
		for(UINT j=0;j<MOD_MAX;j++)
		{
			for(UINT i=0;i<256;i++)
			{
				if(Global::pInputHandler->cmdLUT[j][i].IsValid())
				{
					_ftprintf(hfile,_T("Key[%d]%03d=%03d     ; cmd = '%s'\n"),j,i,Global::pInputHandler->cmdLUT[j][i].ID,Global::pInputHandler->cmdLUT[j][i].GetName());
				}
			}
		}
		fclose(hfile);
	}	return 0;
}

//LRESULT CKeyConfigDlg::OnStnClickedTextDeflines(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//{
//	// TODO : ここにコントロール通知ハンドラ コードを追加します。
//
//	return 0;
//}

LRESULT CKeyConfigDlg::OnEnChangeEditDeflines(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO :  これが RICHEDIT コントロールの場合、まず、CDialogImpl<CKeyConfigDlg>::OnInitDialog() 関数をオーバーライドして、
	// OR 状態の ENM_CORRECTTEXT フラグをマスクに入れて、
	// CRichEditCtrl().SetEventMask() を呼び出さない限り、
	// コントロールは、この通知を送信しません。

	// TODO: Add your control notification handler code here
	TCHAR buffer[256];
	if (bInit)
	{
		m_numlines.GetWindowText(buffer,16);
		int val=_tstoi(buffer);

		if (val < 0)
		{
			val = 0;
		}
		else if(val > MAX_LINES)
		{
			val = MAX_LINES-1;
		}
		_stprintf(buffer,_T("HEX: %x"),val);
		m_textlines.SetWindowText(buffer);
	}
	return 0;
}

LRESULT CKeyConfigDlg::OnLbnSelchangeCmdlist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

	return 0;
}
