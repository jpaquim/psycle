/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.5 $
 */
// GearRackDlg.cpp : implementation file
//

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

#include "GearRackDlg.h"
#include "Song.h"
#include "Instrument.h"
#include "Machine.h"
#include "MainFrm.h"
#include "Instrument.h"
#include ".\gearrackdlg.h"


/////////////////////////////////////////////////////////////////////////////
// CGearRackDlg dialog

int CGearRackDlg::DisplayMode = 0;

CGearRackDlg::CGearRackDlg()
{
	m_pParent = NULL;
	pParentMain = NULL;
}




/////////////////////////////////////////////////////////////////////////////
// CGearRackDlg message handlers

void CGearRackDlg::OnCancel()
{
	pParentMain->pGearRackDialog = NULL;
	DestroyWindow();
}
/*
BOOL CGearRackDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP))
	{
		m_pParent->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
	}
	return CDialog::PreTranslateMessage(pMsg);
}
*/
void CGearRackDlg::RedrawList() 
{	
	TCHAR buffer[64];
	
	m_list.ResetContent();

	int selected;
	int b;

	switch (DisplayMode)
	{
	case 0:
		m_text.SetWindowText(SF::CResourceString(IDS_MSG0028));
		m_props.SetWindowText(SF::CResourceString(IDS_MSG0029));
		m_radio_gen.SetCheck(1);
		m_radio_efx.SetCheck(0);
		m_radio_ins.SetCheck(0);

		selected = Global::_pSong->SeqBus();
		if (selected >= MAX_BUSES)
		{
			selected = 0;
		}
		for (b=0; b<MAX_BUSES; b++) // Check Generators
		{
			if(Global::_pSong->pMachine(b))
			{
				_stprintf(buffer,SF::CResourceString(IDS_MSG0030),b,Global::_pSong->pMachine(b)->_editName);
				m_list.AddString(buffer);
			}
			else
			{
				_stprintf(buffer,SF::CResourceString(IDS_MSG0031),b);
				m_list.AddString(buffer);
			}
		}
		break;
	case 1:
		m_text.SetWindowText(SF::CResourceString(IDS_MSG0032));
		m_props.SetWindowText(SF::CResourceString(IDS_MSG0029));
		m_radio_gen.SetCheck(0);
		m_radio_efx.SetCheck(1);
		m_radio_ins.SetCheck(0);

		selected = Global::_pSong->SeqBus();
		if (selected < MAX_BUSES)
		{
			selected = 0;
		}
		else
		{
			selected -= MAX_BUSES;
		}
		for (b=MAX_BUSES; b<MAX_BUSES*2; b++) // Write Effects Names.
		{
			if(Global::_pSong->pMachine(b))
			{
				_stprintf(buffer,SF::CResourceString(IDS_MSG0030),b,Global::_pSong->pMachine(b)->_editName);
				m_list.AddString(buffer);
			}
			else
			{
				_stprintf(buffer,SF::CResourceString(IDS_MSG0031),b);
				m_list.AddString(buffer);
			}
		}
		break;
	case 2:
		m_text.SetWindowText(SF::CResourceString(IDS_MSG0033));
		m_props.SetWindowText(SF::CResourceString(IDS_MSG0034));
		m_radio_gen.SetCheck(0);
		m_radio_efx.SetCheck(0);
		m_radio_ins.SetCheck(1);

		TCHAR buffer[64];
		for (int b = 0; b < PREV_WAV_INS; b++)
		{
			_stprintf(buffer, SF::CResourceString(IDS_MSG0030), b, Global::_pSong->pInstrument(b)->_sName);
			m_list.AddString(buffer);
		}
		CComboBox cc(pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT));
		if (cc.GetCurSel() == AUX_WAVES)
		{
			selected = Global::_pSong->InstSelected();
		}
		else
		{
			cc.SetCurSel(AUX_WAVES);
			pParentMain->UpdateComboIns(true);
			selected = Global::_pSong->InstSelected();
		}
		break;
	}

	m_list.SetSel(selected);
}

void CGearRackDlg::ExchangeMacs(int one,int two)
{
	Machine * tmp1 = Global::_pSong->pMachine(one);
	Machine * tmp2 = Global::_pSong->pMachine(two);
	
	// if they are both valid
	
	if (tmp1 && tmp2)
	{
		m_pParent->AddMacViewUndo();
		
		// gotta exchange positions
		
		int temp = tmp1->_x;
		tmp1->_x = tmp2->_x;
		tmp2->_x = temp;
		
		temp = tmp1->_y;
		tmp1->_y = tmp2->_y;
		tmp2->_y = temp;
		
		// gotta exchange all connections
		
		float tmp1ivol[MAX_CONNECTIONS],tmp2ivol[MAX_CONNECTIONS], tmp1ovol[MAX_CONNECTIONS],tmp2ovol[MAX_CONNECTIONS];
		
		for (int i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (tmp1->_connection[i])
			{
				tmp1->GetDestWireVolume(one,i,tmp1ovol[i]); //OutputConVol
			}
			if (tmp2->_connection[i])
			{
				tmp2->GetDestWireVolume(two,i,tmp2ovol[i]); //OutputConVol
			}				
			tmp1->GetWireVolume(i,tmp1ivol[i]); // InputConVol
			tmp2->GetWireVolume(i,tmp2ivol[i]); // InputConVol

			temp = tmp1->_outputMachines[i];
			tmp1->_outputMachines[i] = tmp2->_outputMachines[i];
			tmp2->_outputMachines[i] = temp;
			
			temp = tmp1->_inputMachines[i];
			tmp1->_inputMachines[i] = tmp2->_inputMachines[i];
			tmp2->_inputMachines[i] = temp;
			
			bool btemp = tmp1->_connection[i];
			tmp1->_connection[i] = tmp2->_connection[i];
			tmp2->_connection[i] = btemp;
			
			btemp = tmp1->_inputCon[i];
			tmp1->_inputCon[i] = tmp2->_inputCon[i];
			tmp2->_inputCon[i] = btemp;
			
		}
		
		temp = tmp1->_numOutputs;
		tmp1->_numOutputs = tmp2->_numOutputs;
		tmp2->_numOutputs = temp;
		
		temp = tmp1->_numInputs;
		tmp1->_numInputs = tmp2->_numInputs;
		tmp2->_numInputs = temp;

		Global::_pSong->pMachine(one,tmp2);
		Global::_pSong->pMachine(two,tmp1);
		
		tmp1->_macIndex = two;
		tmp2->_macIndex = one;

		for (i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (tmp1->_inputCon[i])
			{
				tmp1->InitWireVolume(Global::_pSong->pMachine(tmp1->_inputMachines[i])->_type,i,tmp2ivol[i]);
			}
//			else tmp1->SetWireVolume(i,0);
			if (tmp2->_inputCon[i])
			{
				tmp2->InitWireVolume(Global::_pSong->pMachine(tmp2->_inputMachines[i])->_type,i,tmp1ivol[i]);
			}
//			else tmp2->SetWireVolume(i,0);

			if (tmp1->_connection[i])
			{
				Global::_pSong->pMachine(tmp1->_outputMachines[i])->InitWireVolume(tmp1->_type,tmp1->FindOutputWire(tmp1->_outputMachines[i]),tmp2ovol[i]);
			}
			if (tmp2->_connection[i])
			{
				Global::_pSong->pMachine(tmp2->_outputMachines[i])->InitWireVolume(tmp2->_type,tmp2->FindOutputWire(tmp2->_outputMachines[i]),tmp1ovol[i]);
			}
			
		}
		return;
	}
	if (tmp1)
	{
		m_pParent->AddMacViewUndo();
		// ok we gotta swap this one for a null one
		Global::_pSong->pMachine(one,NULL);
		Global::_pSong->pMachine(two,tmp1);

		tmp1->_macIndex = two;

		// and any machine that pointed to this one needs to be swapped
		for (int i = 0; i < MAX_MACHINES; i++)
		{
			Machine* cmp = Global::_pSong->pMachine(i);
			if (cmp)
			{
				for (int j=0; j < MAX_CONNECTIONS; j++)
				{
					if (cmp->_inputMachines[j] == one)
					{
						cmp->_inputMachines[j] = two;
					}

					if (cmp->_outputMachines[j] == one)
					{
						cmp->_outputMachines[j] = two;
					}
				}
			}
		}
		return;
	}
	if (tmp2)
	{
		m_pParent->AddMacViewUndo();
		// ok we gotta swap this one for a null one
		Global::_pSong->pMachine(two,NULL);
		Global::_pSong->pMachine(one,tmp2);

		tmp2->_macIndex = one;

		// and any machine that pointed to this one needs to be swapped
		for (int i = 0; i < MAX_MACHINES; i++)
		{
			Machine*cmp = Global::_pSong->pMachine(i);
			if (cmp)
			{
				for (int j=0; j < MAX_CONNECTIONS; j++)
				{
					if (cmp->_inputMachines[j] == two)
					{
						cmp->_inputMachines[j] = one;
					}

					if (cmp->_outputMachines[j] == two)
					{
						cmp->_outputMachines[j] = one;
					}
				}
			}
		}
		return;
	}
}

void CGearRackDlg::ExchangeIns(int one,int two)
{
	Song* pSong = Global::_pSong;
	Instrument * tmpins;

	m_pParent->AddMacViewUndo();
	
	tmpins = pSong->pInstrument(one);
	pSong->pInstrument(one,pSong->pInstrument(two));
	pSong->pInstrument(two,tmpins);
	//The above works because we are not creating new objects, just swaping them.
	//this means that no new data is generated/deleted,and the information is just
	//copied. If not, we would have had to define the operator=() function and take
	//care of it.

}

LRESULT CGearRackDlg::OnBnClickedCreate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int tmac = m_list.GetAnchorIndex();//GetCurSel();
	switch (DisplayMode)
	{
	case 1:
		tmac += MAX_BUSES;
	case 0:
		{
			Global::_pSong->SeqBus(tmac);

			Machine * mac = Global::_pSong->pMachine(tmac);
			if (mac)
			{
				int x = mac->_x;
				int y = mac->_y;

				// buffer all the connection info

				int outputMachines[MAX_CONNECTIONS];
				int inputMachines[MAX_CONNECTIONS];
				float inputConVol[MAX_CONNECTIONS];
				float outputConVol[MAX_CONNECTIONS];
				bool connection[MAX_CONNECTIONS];
				bool inputCon[MAX_CONNECTIONS];

				int numOutputs = mac->_numOutputs;
				int numInputs = mac->_numInputs;

				for (int i = 0; i < MAX_CONNECTIONS; i++)
				{
					outputMachines[i] = mac->_outputMachines[i];
					inputMachines[i] = mac->_inputMachines[i];
					inputConVol[i] = mac->_inputConVol[i]*mac->_wireMultiplier[i];
					connection[i] = mac->_connection[i];
					inputCon[i] = mac->_inputCon[i];
					// store volumes coming back this way, they get destroyed by new machine
					if (connection[i])
					{
						int j = Global::_pSong->pMachine(outputMachines[i])->FindInputWire(tmac);
						if (j >= 0)
						{
							Global::_pSong->pMachine(outputMachines[i])->GetWireVolume(j, outputConVol[i]);
						}
					}
				}

				m_pParent->NewMachine(x,y,tmac);
				// replace all the connection info

				Machine * mac = Global::_pSong->pMachine(tmac);
				if (mac)
				{
					mac->_numOutputs = numOutputs;
					mac->_numInputs = numInputs;

					for (i = 0; i < MAX_CONNECTIONS; i++)
					{
						// restore input connections
						if (inputCon[i])
						{
							Global::_pSong->InsertConnection(inputMachines[i], tmac, inputConVol[i]);
						}
						// restore output connections
						if (connection[i])
						{
							Global::_pSong->InsertConnection(tmac, outputMachines[i], outputConVol[i]);
						}
					}
				}
			}
			else
			{
				m_pParent->NewMachine(-1,-1,tmac);
			}

			pParentMain->UpdateEnvInfo();
			pParentMain->UpdateComboGen();
			if (m_pParent->viewMode==VMMachine)
			{
				m_pParent->Repaint();
			}
		}
		break;
	case 2:
		{
			CComboBox cc(pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT));
			cc.SetCurSel(AUX_WAVES);
			Global::_pSong->AuxcolSelected(tmac);
			Global::_pSong->InstSelected(tmac);
			pParentMain->UpdateComboIns(true);
			pParentMain->m_wndInst.WaveUpdate();
		}
		BOOL bDummy;
		pParentMain->OnBnClickedLoadwave(0,0,NULL,bDummy);
		pParentMain->UpdateComboIns(true);
		break;
	}
	RedrawList();
	return 0;
}

LRESULT CGearRackDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_props.Attach(GetDlgItem(IDC_PROPERTIES));
	m_radio_ins.Attach(GetDlgItem(IDC_RADIO_INS));
	m_radio_gen.Attach(GetDlgItem(IDC_RADIO_GEN));
	m_radio_efx.Attach(GetDlgItem(IDC_RADIO_EFX));
	m_text.Attach(GetDlgItem(ID_TEXT));
	m_list.Attach(GetDlgItem(IDC_GEARLIST));
	// fill our list box and select the currently selected machine
	RedrawList();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CGearRackDlg::OnBnClickedDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
// TODO: Add your control notification handler code here
	int tmac = m_list.GetCurSel();
	m_pParent->AddMacViewUndo();
	switch (DisplayMode)
	{
	case 0:
		if (MessageBox(SF::CResourceString(IDS_MSG0052),SF::CResourceString(IDS_MSG0053), MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
		{
			if (Global::_pSong->pMachine(tmac))
			{
				pParentMain->CloseMacGui(tmac);
				Global::_pSong->DestroyMachine(tmac);
				pParentMain->UpdateEnvInfo();
				pParentMain->UpdateComboGen();
				if (m_pParent->viewMode==VMMachine)
				{
					m_pParent->Repaint();
				}
			}
		}
		break;
	case 1:
		if (MessageBox(SF::CResourceString(IDS_MSG0052),SF::CResourceString(IDS_MSG0053), MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
		{
			if (Global::_pSong->pMachine(tmac+MAX_BUSES))
			{
				pParentMain->CloseMacGui(tmac+MAX_BUSES);
				Global::_pSong->DestroyMachine(tmac+MAX_BUSES);
				pParentMain->UpdateEnvInfo();
				pParentMain->UpdateComboGen();
				if (m_pParent->viewMode==VMMachine)
				{
					m_pParent->Repaint();
				}
			}
		}
		break;
	case 2:
		{
			CComboBox cc(pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT));
			cc.SetCurSel(AUX_WAVES);
			Global::_pSong->InstSelected(tmac);
			Global::_pSong->AuxcolSelected(tmac);
			pParentMain->UpdateComboIns(true);
			pParentMain->m_wndInst.WaveUpdate();
		}
		Global::_pSong->DeleteInstrument(Global::_pSong->InstSelected());
		pParentMain->UpdateComboIns(true);
		break;
	}
	RedrawList();
	return 0;
}

LRESULT CGearRackDlg::OnBnClickedParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
		// TODO: Add your control notification handler code here
	POINT point;
	GetCursorPos(&point);
	int tmac = m_list.GetAnchorIndex();//
	switch (DisplayMode)
	{
	case 0:
		if (Global::_pSong->pMachine(tmac))
		{
			pParentMain->ShowMachineGui(tmac,point);
		}
		break;
	case 1:
		if (Global::_pSong->pMachine(tmac+MAX_BUSES))
		{
			pParentMain->ShowMachineGui(tmac+MAX_BUSES,point);
		}
		break;
	case 2:
		{
			CComboBox cc(pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT));
			cc.SetCurSel(AUX_WAVES);
			Global::_pSong->InstSelected(tmac);
			Global::_pSong->AuxcolSelected(tmac);
			pParentMain->UpdateComboIns(true);
			pParentMain->m_wndInst.WaveUpdate();
		}
		pParentMain->ShowInstrumentEditor();
		pParentMain->UpdateComboIns(true);
		break;
	}
	RedrawList();
	return 0;
}

LRESULT CGearRackDlg::OnBnClickedProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	int tmac = m_list.GetAnchorIndex();
	switch (DisplayMode)
	{
	case 0:
		if (Global::_pSong->pMachine(tmac))
		{
			m_pParent->DoMacPropDialog(tmac);
			pParentMain->UpdateEnvInfo();
			pParentMain->UpdateComboGen();
			if (m_pParent->viewMode==VMMachine)
			{
				m_pParent->Repaint();
			}
		}
		break;
	case 1:
		if (Global::_pSong->pMachine(tmac+MAX_BUSES))
		{
			m_pParent->DoMacPropDialog(tmac+MAX_BUSES);
			pParentMain->UpdateEnvInfo();
			pParentMain->UpdateComboGen();
			if (m_pParent->viewMode==VMMachine)
			{
				m_pParent->Repaint();
			}
		}
		break;
	case 2:
		{
			CComboBox cc(pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT));
			cc.SetCurSel(AUX_WAVES);
			Global::_pSong->InstSelected(tmac);
			Global::_pSong->AuxcolSelected(tmac);
			pParentMain->UpdateComboIns(true);
			pParentMain->m_wndInst.WaveUpdate();
		}

		pParentMain->m_pWndWed->ShowWindow(SW_SHOWNORMAL);
		pParentMain->m_pWndWed->SetActiveWindow();
		break;
	}
	RedrawList();
	return 0;
}

LRESULT CGearRackDlg::OnBnClickedExchange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if ( m_list.GetSelCount() != 2 )
	{
		MessageBox(SF::CResourceString(IDS_ERR_MSG0108),SF::CResourceString(IDS_ERR_MSG0109));
		return 0;
	}

	int sel[2]={0,0},j=0;
	const int maxitems=m_list.GetCount();
	for (int c=0;c<maxitems;c++) 
	{
		if ( m_list.GetSel(c) != 0) sel[j++]=c;
	}

	switch (DisplayMode) // should be necessary to rename opened parameter windows.
	{
	case 0:
		ExchangeMacs(sel[0],sel[1]);
		pParentMain->UpdateComboGen(true);
		if (m_pParent->viewMode==VMMachine)
		{
			m_pParent->Repaint();
		}
		break;
	case 1:
		ExchangeMacs(sel[0]+MAX_BUSES,sel[1]+MAX_BUSES);
		pParentMain->UpdateComboGen(true);
		if (m_pParent->viewMode==VMMachine)
		{
			m_pParent->Repaint();
		}
		break;
	case 2:
		Global::_pSong->IsInvalided(true);
		ExchangeIns(sel[0],sel[1]);
		
		Global::_pSong->IsInvalided(false);
		pParentMain->UpdateComboIns(true);
		break;
	}
	
	pParentMain->RedrawGearRackList();
	return 0;
}

LRESULT CGearRackDlg::OnBnClickedClonemachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
		// TODO: Add your control notification handler code here
	int tmac1 = m_list.GetAnchorIndex();
	int tmac2 = -1;

	if ( m_list.GetSelCount() == 2 )
	{
		int sel[2]={0,0},j=0;
		const int maxitems=m_list.GetCount();
		for (int c=0;c<maxitems;c++) 
		{
			if ( m_list.GetSel(c) != 0) sel[j++]=c;
		}

		tmac1 = sel[0];
		tmac2 = sel[1];
	}
	else if ( m_list.GetSelCount() != 1 )
	{
		MessageBox(SF::CResourceString(IDS_ERR_MSG0110),SF::CResourceString(IDS_ERR_MSG0109));
		return 0;
	}

	// now lets do the actual work...
	switch (DisplayMode) // should be necessary to rename opened parameter windows.
	{
	case 0:
		if (tmac2 < 0)
		{
			// we need to find an empty slot
			for (int i = 0; i < MAX_BUSES; i++)
			{
				if (!Global::_pSong->pMachine(i))
				{
					tmac2 = i;
					break;
				}
			}
		}
		if (tmac2 >= 0)
		{
			if (!Global::_pSong->CloneMac(tmac1,tmac2))
			{
				MessageBox(SF::CResourceString(IDS_ERR_MSG0110),SF::CResourceString(IDS_ERR_MSG0109));
				return 0;
			}
		}
		pParentMain->UpdateComboGen(true);
		if (m_pParent->viewMode==VMMachine)
		{
			m_pParent->Repaint();
		}
		break;
	case 1:
		tmac1+=MAX_BUSES;
		if (tmac2 >= 0)
		{
			tmac2+=MAX_BUSES;
		}
		else
		{
			// we need to find an empty slot
			for (int i = MAX_BUSES; i < MAX_BUSES*2; i++)
			{
				if (!Global::_pSong->pMachine(i))
				{
					tmac2 = i;
					break;
				}
			}
		}
		if (tmac2 >= 0)
		{
			if (!Global::_pSong->CloneMac(tmac1,tmac2))
			{
				MessageBox(SF::CResourceString(IDS_ERR_MSG0110),SF::CResourceString(IDS_ERR_MSG0109));
				return 0;
			}
		}
		pParentMain->UpdateComboGen(true);
		if (m_pParent->viewMode==VMMachine)
		{
			m_pParent->Repaint();
		}
		break;
	case 2:
		Global::_pSong->IsInvalided(true);
		if (tmac2 < 0)
		{
			for (int i = 0; i < MAX_INSTRUMENTS; i++)
			{
				if (Global::_pSong->pInstrument(i)->Empty())
				{
					tmac2 = i;
					break;
				}
			}
		}
		if (tmac2 >=0)
		{
			if (!Global::_pSong->CloneIns(tmac1,tmac2))
			{
				MessageBox(SF::CResourceString(IDS_ERR_MSG0110),SF::CResourceString(IDS_ERR_MSG0109));
				return 0;
			}
		}
		
		Global::_pSong->IsInvalided(false);
		pParentMain->UpdateComboIns(true);
		break;
	}
	
	pParentMain->RedrawGearRackList();
	return 0;
}

LRESULT CGearRackDlg::OnLbnSelchangeGearlist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	int tmac = m_list.GetAnchorIndex();
	switch (DisplayMode)
	{
	case 1:
		tmac += MAX_BUSES;
	case 0:
		Global::_pSong->SeqBus(tmac);
		pParentMain->UpdateComboGen();
		break;
	case 2:
		{
			CComboBox cc(pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT));
			if (cc.GetCurSel() == AUX_WAVES)
			{
				Global::_pSong->InstSelected(tmac);
				Global::_pSong->AuxcolSelected(tmac);
				pParentMain->UpdateComboIns(false);
			}
			else
			{
				cc.SetCurSel(AUX_WAVES);
				Global::_pSong->InstSelected(tmac);
				Global::_pSong->AuxcolSelected(tmac);
				pParentMain->UpdateComboIns(true);
			}
			pParentMain->m_wndInst.WaveUpdate();
		}
		break;
	}
	return 0;
}

LRESULT CGearRackDlg::OnBnClickedRadioGen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DisplayMode = 0;
	RedrawList();
	return 0;
}

LRESULT CGearRackDlg::OnBnClickedRadioEfx(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	DisplayMode = 1;
	RedrawList();
	return 0;
}

LRESULT CGearRackDlg::OnBnClickedRadioIns(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	DisplayMode = 2;
	RedrawList();
	return 0;
}

LRESULT CGearRackDlg::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
// TODO: Add your control notification handler code here

	return 0;
}

LRESULT CGearRackDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO : ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
	pParentMain->pGearRackDialog = NULL;
	DestroyWindow();
	return 0;
}

void CGearRackDlg::OnFinalMessage(HWND hWnd)
{
	// TODO : ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	CDialogImpl<CGearRackDlg>::OnFinalMessage(hWnd);
	delete this;
}
