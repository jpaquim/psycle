///\file
///\brief implementation file for psycle::host::CGearRackDlg.

#include "GearRackDlg.hpp"

#include "WaveEdFrame.hpp"
#include "MainFrm.hpp"
#include "ChildView.hpp"
#include "InputHandler.hpp"
#include "Song.hpp"
#include "Machine.hpp"

namespace psycle { namespace host {

		int CGearRackDlg::DisplayMode = 0;

		CGearRackDlg::CGearRackDlg(CMainFrame* pParent, CChildView* pMain, CGearRackDlg** windowVar_)
			: CDialog(CGearRackDlg::IDD, AfxGetMainWnd())
			, mainFrame(pParent)
			, mainView(pMain)
			, windowVar(windowVar_)
		{
			CDialog::Create(IDD, AfxGetMainWnd());
		}

		void CGearRackDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_PROPERTIES, m_props);
			DDX_Control(pDX, IDC_RADIO_INS, m_radio_ins);
			DDX_Control(pDX, IDC_RADIO_GEN, m_radio_gen);
			DDX_Control(pDX, IDC_RADIO_EFX, m_radio_efx);
			DDX_Control(pDX, ID_TEXT, m_text);
			DDX_Control(pDX, IDC_GEARLIST, m_list);
		}

		BEGIN_MESSAGE_MAP(CGearRackDlg, CDialog)
			ON_WM_CLOSE()
			ON_BN_CLICKED(IDC_CREATE, OnCreate)
			ON_BN_CLICKED(IDC_DELETE, OnDelete)
			ON_LBN_DBLCLK(IDC_GEARLIST, OnDblclkGearlist)
			ON_BN_CLICKED(IDC_PROPERTIES, OnProperties)
			ON_BN_CLICKED(IDC_SHOW_MASTER, OnMasterProperties)
			ON_BN_CLICKED(IDC_PARAMETERS, OnParameters)
			ON_LBN_SELCHANGE(IDC_GEARLIST, OnSelchangeGearlist)
			ON_BN_CLICKED(IDC_RADIO_EFX, OnRadioEfx)
			ON_BN_CLICKED(IDC_RADIO_GEN, OnRadioGen)
			ON_BN_CLICKED(IDC_RADIO_INS, OnRadioIns)
			ON_BN_CLICKED(IDC_EXCHANGE, OnExchange)
			ON_BN_CLICKED(IDC_CLONEMACHINE, OnClonemachine)
		END_MESSAGE_MAP()


		BOOL CGearRackDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			// fill our list box and select the currently selected machine
			RedrawList();
			return TRUE;
			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
		}

		void CGearRackDlg::OnClose()
		{
			AfxGetMainWnd()->SetFocus();
			CDialog::OnClose();
			DestroyWindow();
		}
		void CGearRackDlg::PostNcDestroy()
		{
			if(windowVar != NULL) *windowVar = NULL;
			delete this;
		}

		BOOL CGearRackDlg::PreTranslateMessage(MSG* pMsg) 
		{
			if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP)) {
				CmdDef def = Global::pInputHandler->KeyToCmd(pMsg->wParam,0);
				if(def.GetType() == CT_Note) {
					mainView->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
					return true;
				}
			}
			return CDialog::PreTranslateMessage(pMsg);
		}
		void CGearRackDlg::RedrawList() 
		{
			char buffer[64];
			
			m_list.ResetContent();

			int selected=0;
			int b;

			switch (DisplayMode)
			{
			case 0:
				m_text.SetWindowText("Machines: Generators");
				m_props.SetWindowText("Properties");
				m_radio_gen.SetCheck(1);
				m_radio_efx.SetCheck(0);
				m_radio_ins.SetCheck(0);

				selected = Global::_pSong->seqBus;
				if (selected >= MAX_BUSES)
				{
					selected = 0;
				}
				for (b=0; b<MAX_BUSES; b++) // Check Generators
				{
					if(Global::_pSong->_pMachine[b])
					{
						sprintf(buffer,"%.2X: %s",b,Global::_pSong->_pMachine[b]->_editName);
						m_list.AddString(buffer);
					}
					else
					{
						sprintf(buffer,"%.2X: empty",b);
						m_list.AddString(buffer);
					}
				}
				break;
			case 1:
				m_text.SetWindowText("Machines: Effects");
				m_props.SetWindowText("Properties");
				m_radio_gen.SetCheck(0);
				m_radio_efx.SetCheck(1);
				m_radio_ins.SetCheck(0);

				selected = Global::_pSong->seqBus;
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
					if(Global::_pSong->_pMachine[b])
					{
						sprintf(buffer,"%.2X: %s",b,Global::_pSong->_pMachine[b]->_editName);
						m_list.AddString(buffer);
					}
					else
					{
						sprintf(buffer,"%.2X: empty",b);
						m_list.AddString(buffer);
					}
				}
				break;
			case 2:
				m_text.SetWindowText("Sample Instruments");
				m_props.SetWindowText("Wave Editor");
				m_radio_gen.SetCheck(0);
				m_radio_efx.SetCheck(0);
				m_radio_ins.SetCheck(1);

				char buffer[64];
				for (int b=0;b<PREV_WAV_INS;b++)
				{
					sprintf(buffer, "%.2X: %s", b, Global::_pSong->_pInstrument[b]->_sName);
					m_list.AddString(buffer);
				}
				CComboBox *cc=(CComboBox *)mainFrame->m_machineBar.GetDlgItem(IDC_AUXSELECT);
				if (cc->GetCurSel() == AUX_INSTRUMENT)
				{
					selected = Global::_pSong->instSelected;
				}
				else
				{
					cc->SetCurSel(AUX_INSTRUMENT);
					mainFrame->UpdateComboIns(true);
					selected = Global::_pSong->instSelected;
				}
				break;
			}

			m_list.SetCurSel(selected);
		}

		void CGearRackDlg::OnSelchangeGearlist() 
		{
			int tmac = m_list.GetCurSel();
			switch (DisplayMode)
			{
			case 1:
				tmac += MAX_BUSES;
			case 0:
				Global::_pSong->seqBus = tmac;
				mainFrame->UpdateComboGen();
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)mainFrame->m_machineBar.GetDlgItem(IDC_AUXSELECT);
					if (cc->GetCurSel() == AUX_INSTRUMENT)
					{
						Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
						mainFrame->UpdateComboIns(false);
					}
					else
					{
						cc->SetCurSel(AUX_INSTRUMENT);
						Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
						mainFrame->UpdateComboIns(true);
					}
					mainFrame->m_wndInst.WaveUpdate();
				}
				break;
			}
		}

		void CGearRackDlg::OnCreate() 
		{
			int tmac = m_list.GetCurSel();
			switch (DisplayMode)
			{
			case 1:
				tmac += MAX_BUSES;
			case 0:
				{
					mainView->NewMachine(-1,-1,tmac);

					mainFrame->UpdateEnvInfo();
					mainFrame->UpdateComboGen();
					if (mainView->viewMode==view_modes::machine)
					{
						mainView->Repaint();
					}
				}
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)mainFrame->m_machineBar.GetDlgItem(IDC_AUXSELECT);
					cc->SetCurSel(AUX_INSTRUMENT);
					Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
					mainFrame->UpdateComboIns(true);
					mainFrame->m_wndInst.WaveUpdate();
				}
				mainFrame->OnLoadwave();
				mainFrame->UpdateComboIns(true);
				break;
			}
			RedrawList();
		}

		void CGearRackDlg::OnDelete() 
		{
			int tmac = m_list.GetCurSel();
			Global::pInputHandler->AddMacViewUndo();
			switch (DisplayMode)
			{
			case 1:
				tmac+=MAX_BUSES;
				//fallthrough
			case 0:
				if (MessageBox("Are you sure?","Delete Machine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
				{
					if (Global::_pSong->_pMachine[tmac])
					{
						{
							CExclusiveLock lock(&Global::_pSong->semaphore, 2, true);
							mainFrame->CloseMacGui(tmac);
							Global::_pSong->DestroyMachine(tmac);
						}
						mainFrame->UpdateEnvInfo();
						mainFrame->UpdateComboGen();
						if (mainView->viewMode==view_modes::machine)
						{
							mainView->Repaint();
						}
					}
				}
				break;
			case 2:
				if (MessageBox("Are you sure?","Delete Instruments", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
				{
					{
						CExclusiveLock lock(&Global::_pSong->semaphore, 2, true);
						Global::_pSong->DeleteInstrument(Global::_pSong->instSelected);
					}
					mainFrame->UpdateComboIns(true);
					mainFrame->m_wndInst.WaveUpdate();
				}
				break;
			}
			RedrawList();
		}

		void CGearRackDlg::OnDblclkGearlist() 
		{
			OnCreate();	
		}


		void CGearRackDlg::OnProperties() 
		{
			int tmac = m_list.GetCurSel();
			switch (DisplayMode)
			{
			case 0:
				if (Global::_pSong->_pMachine[tmac])
				{
					mainView->DoMacPropDialog(tmac);
					mainFrame->UpdateEnvInfo();
					mainFrame->UpdateComboGen();
					if (mainView->viewMode==view_modes::machine)
					{
						mainView->Repaint();
					}
				}
				break;
			case 1:
				if (Global::_pSong->_pMachine[tmac+MAX_BUSES])
				{
					mainView->DoMacPropDialog(tmac+MAX_BUSES);
					mainFrame->UpdateEnvInfo();
					mainFrame->UpdateComboGen();
					if (mainView->viewMode==view_modes::machine)
					{
						mainView->Repaint();
					}
				}
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)mainFrame->m_machineBar.GetDlgItem(IDC_AUXSELECT);
					cc->SetCurSel(AUX_INSTRUMENT);
					Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
					mainFrame->UpdateComboIns(true);
					mainFrame->m_wndInst.WaveUpdate();
				}

				mainFrame->m_pWndWed->ShowWindow(SW_SHOWNORMAL);
				mainFrame->m_pWndWed->SetActiveWindow();
				break;
			}
			RedrawList();
		}

		void CGearRackDlg::OnParameters() 
		{
			POINT point;
			GetCursorPos(&point);
			int tmac = m_list.GetCurSel();
			switch (DisplayMode)
			{
			case 0:
				if (Global::_pSong->_pMachine[tmac])
				{
					mainFrame->ShowMachineGui(tmac,point);
				}
				break;
			case 1:
				if (Global::_pSong->_pMachine[tmac+MAX_BUSES])
				{
					mainFrame->ShowMachineGui(tmac+MAX_BUSES,point);
				}
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)mainFrame->m_machineBar.GetDlgItem(IDC_AUXSELECT);
					cc->SetCurSel(AUX_INSTRUMENT);
					Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
					mainFrame->UpdateComboIns(true);
					mainFrame->m_wndInst.WaveUpdate();
				}
				mainFrame->ShowInstrumentEditor();
				mainFrame->UpdateComboIns(true);
				break;
			}
			RedrawList();
		}
		void CGearRackDlg::OnMasterProperties()
		{
			POINT point;
			GetCursorPos(&point);
			mainFrame->ShowMachineGui(MASTER_INDEX,point);
		}

		void CGearRackDlg::OnRadioGen() 
		{
			DisplayMode = 0;
			RedrawList();
		}

		void CGearRackDlg::OnRadioEfx() 
		{
			DisplayMode = 1;
			RedrawList();
			
		}

		void CGearRackDlg::OnRadioIns() 
		{
			DisplayMode = 2;
			RedrawList();
			
		}

		void CGearRackDlg::OnExchange() 
		{
			if ( m_list.GetSelCount() != 2 )
			{
				MessageBox("This option requires that you select two entries","Gear Rack Dialog");
				return;
			}

			int sel[2]={0,0},j=0;
			const int maxitems=m_list.GetCount();
			for (int c=0;c<maxitems;c++) 
			{
				if ( m_list.GetSel(c) != 0) sel[j++]=c;
			}

			switch (DisplayMode) // should be necessary to rename opened parameter windows.
			{
			case 1:
				sel[0]+=MAX_BUSES;
				sel[1]+=MAX_BUSES;
				//fallthrough
			case 0:
				Global::pInputHandler->AddMacViewUndo();
				Global::_pSong->ExchangeMachines(sel[0],sel[1]);
				mainFrame->UpdateComboGen(true);
				if (mainView->viewMode==view_modes::machine)
				{
					mainView->Repaint();
				}
				break;
			case 2:
				Global::pInputHandler->AddMacViewUndo();
				Global::_pSong->ExchangeInstruments(sel[0],sel[1]);
				
				mainFrame->UpdateComboIns(true);
				break;
			}
			
			RedrawList();
		}

		void CGearRackDlg::OnClonemachine() 
		{
			int tmac1 = m_list.GetCurSel();
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
				MessageBox("Select 1 active slot (and optionally 1 empty destination slot)","Gear Rack Dialog");
				return;
			}

			// now lets do the actual work...
			switch (DisplayMode) // should be necessary to rename opened parameter windows.
			{
			case 0:
				if (tmac2 < 0)
				{
					tmac2 = Global::_pSong->GetFreeBus();
				}
				if (tmac2 >= 0)
				{
					if (!Global::_pSong->CloneMac(tmac1,tmac2))
					{
						MessageBox("Select 1 active slot (and optionally 1 empty destination slot)","Gear Rack Dialog");
						return;
					}
				}
				mainFrame->UpdateComboGen(true);
				if (mainView->viewMode==view_modes::machine)
				{
					mainView->Repaint();
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
					tmac2 = Global::_pSong->GetFreeFxBus();
				}
				if (tmac2 >= 0)
				{
					if (!Global::_pSong->CloneMac(tmac1,tmac2))
					{
						MessageBox("Select 1 active slot (and optionally 1 empty destination slot)","Gear Rack Dialog");
						return;
					}
				}
				mainFrame->UpdateComboGen(true);
				if (mainView->viewMode==view_modes::machine)
				{
					mainView->Repaint();
				}
				break;
			case 2:
				if (tmac2 < 0)
				{
					for (int i = 0; i < MAX_INSTRUMENTS; i++)
					{
						if (Global::_pSong->_pInstrument[i]->Empty())
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
						MessageBox("Select 1 active slot (and optionally 1 empty destination slot)","Gear Rack Dialog");
						return;
					}
				}
				
				mainFrame->UpdateComboIns(true);
				break;
			}
			
			RedrawList();
		}

	}   // namespace
}   // namespace
