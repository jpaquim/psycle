///\file
///\brief implementation file for psycle::host::CGearRackDlg.
#include <psycle/project.private.hpp>
#include "GearRackDlg.hpp"
#include "Psycle.hpp"
#include "WaveEdFrame.hpp"
#include "Song.hpp"
#include "Machine.hpp"
#include "MainFrm.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		int CGearRackDlg::DisplayMode = 0;

		CGearRackDlg::CGearRackDlg(CChildView* pParent, CMainFrame* pMain)
			: CDialog(CGearRackDlg::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CGearRackDlg)
			//}}AFX_DATA_INIT
			m_pParent = pParent;
			pParentMain = pMain;
		}

		void CGearRackDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CGearRackDlg)
			DDX_Control(pDX, IDC_PROPERTIES, m_props);
			DDX_Control(pDX, IDC_RADIO_INS, m_radio_ins);
			DDX_Control(pDX, IDC_RADIO_GEN, m_radio_gen);
			DDX_Control(pDX, IDC_RADIO_EFX, m_radio_efx);
			DDX_Control(pDX, ID_TEXT, m_text);
			DDX_Control(pDX, IDC_GEARLIST, m_list);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CGearRackDlg, CDialog)
			//{{AFX_MSG_MAP(CGearRackDlg)
			ON_BN_CLICKED(IDC_CREATE, OnCreate)
			ON_BN_CLICKED(IDC_DELETE, OnDelete)
			ON_LBN_DBLCLK(IDC_GEARLIST, OnDblclkGearlist)
			ON_BN_CLICKED(IDC_PROPERTIES, OnProperties)
			ON_BN_CLICKED(IDC_PARAMETERS, OnParameters)
			ON_LBN_SELCHANGE(IDC_GEARLIST, OnSelchangeGearlist)
			ON_BN_CLICKED(IDC_RADIO_EFX, OnRadioEfx)
			ON_BN_CLICKED(IDC_RADIO_GEN, OnRadioGen)
			ON_BN_CLICKED(IDC_RADIO_INS, OnRadioIns)
			ON_BN_CLICKED(IDC_EXCHANGE, OnExchange)
			ON_BN_CLICKED(IDC_CLONEMACHINE, OnClonemachine)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CGearRackDlg::Create()
		{
			return CDialog::Create(IDD, m_pParent);
		}

		BOOL CGearRackDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			// fill our list box and select the currently selected machine
			RedrawList();
			return TRUE;
			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
		}

		void CGearRackDlg::OnCancel()
		{
			pParentMain->pGearRackDialog = NULL;
			DestroyWindow();
			delete this;
		}

		BOOL CGearRackDlg::PreTranslateMessage(MSG* pMsg) 
		{
			if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP))
			{
				m_pParent->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
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
				CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
				if (cc->GetCurSel() == AUX_WAVES)
				{
					selected = Global::_pSong->instSelected;
				}
				else
				{
					cc->SetCurSel(AUX_WAVES);
					pParentMain->UpdateComboIns(true);
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
				pParentMain->UpdateComboGen();
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					if (cc->GetCurSel() == AUX_WAVES)
					{
						Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
						pParentMain->UpdateComboIns(false);
					}
					else
					{
						cc->SetCurSel(AUX_WAVES);
						Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
						pParentMain->UpdateComboIns(true);
					}
					pParentMain->m_wndInst.WaveUpdate();
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
					m_pParent->NewMachine(-1,-1,tmac);

					pParentMain->UpdateEnvInfo();
					pParentMain->UpdateComboGen();
					if (m_pParent->viewMode==view_modes::machine)
					{
						m_pParent->Repaint();
					}
				}
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					cc->SetCurSel(AUX_WAVES);
					Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
					pParentMain->UpdateComboIns(true);
					pParentMain->m_wndInst.WaveUpdate();
				}
				pParentMain->OnLoadwave();
				pParentMain->UpdateComboIns(true);
				break;
			}
			RedrawList();
		}

		void CGearRackDlg::OnDelete() 
		{
			int tmac = m_list.GetCurSel();
			m_pParent->AddMacViewUndo();
			switch (DisplayMode)
			{
			case 0:
				if (MessageBox("Are you sure?","Delete Machine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
				{
					if (Global::_pSong->_pMachine[tmac])
					{
						pParentMain->CloseMacGui(tmac);
						Global::_pSong->DestroyMachine(tmac);
						pParentMain->UpdateEnvInfo();
						pParentMain->UpdateComboGen();
						if (m_pParent->viewMode==view_modes::machine)
						{
							m_pParent->Repaint();
						}
					}
				}
				break;
			case 1:
				if (MessageBox("Are you sure?","Delete Machine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
				{
					if (Global::_pSong->_pMachine[tmac+MAX_BUSES])
					{
						pParentMain->CloseMacGui(tmac+MAX_BUSES);
						Global::_pSong->DestroyMachine(tmac+MAX_BUSES);
						pParentMain->UpdateEnvInfo();
						pParentMain->UpdateComboGen();
						if (m_pParent->viewMode==view_modes::machine)
						{
							m_pParent->Repaint();
						}
					}
				}
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					cc->SetCurSel(AUX_WAVES);
					Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
					pParentMain->UpdateComboIns(true);
					pParentMain->m_wndInst.WaveUpdate();
				}
				Global::_pSong->DeleteInstrument(Global::_pSong->instSelected);
				pParentMain->UpdateComboIns(true);
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
					m_pParent->DoMacPropDialog(tmac);
					pParentMain->UpdateEnvInfo();
					pParentMain->UpdateComboGen();
					if (m_pParent->viewMode==view_modes::machine)
					{
						m_pParent->Repaint();
					}
				}
				break;
			case 1:
				if (Global::_pSong->_pMachine[tmac+MAX_BUSES])
				{
					m_pParent->DoMacPropDialog(tmac+MAX_BUSES);
					pParentMain->UpdateEnvInfo();
					pParentMain->UpdateComboGen();
					if (m_pParent->viewMode==view_modes::machine)
					{
						m_pParent->Repaint();
					}
				}
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					cc->SetCurSel(AUX_WAVES);
					Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
					pParentMain->UpdateComboIns(true);
					pParentMain->m_wndInst.WaveUpdate();
				}

				pParentMain->m_pWndWed->ShowWindow(SW_SHOWNORMAL);
				pParentMain->m_pWndWed->SetActiveWindow();
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
					pParentMain->ShowMachineGui(tmac,point);
				}
				break;
			case 1:
				if (Global::_pSong->_pMachine[tmac+MAX_BUSES])
				{
					pParentMain->ShowMachineGui(tmac+MAX_BUSES,point);
				}
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					cc->SetCurSel(AUX_WAVES);
					Global::_pSong->instSelected = Global::_pSong->auxcolSelected=tmac;
					pParentMain->UpdateComboIns(true);
					pParentMain->m_wndInst.WaveUpdate();
				}
				pParentMain->ShowInstrumentEditor();
				pParentMain->UpdateComboIns(true);
				break;
			}
			RedrawList();
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
			case 0:
				m_pParent->AddMacViewUndo();
				Global::_pSong->ExchangeMachines(sel[0],sel[1]);
				pParentMain->UpdateComboGen(true);
				if (m_pParent->viewMode==view_modes::machine)
				{
					m_pParent->Repaint();
				}
				break;
			case 1:
				m_pParent->AddMacViewUndo();
				Global::_pSong->ExchangeMachines(sel[0]+MAX_BUSES,sel[1]+MAX_BUSES);
				pParentMain->UpdateComboGen(true);
				if (m_pParent->viewMode==view_modes::machine)
				{
					m_pParent->Repaint();
				}
				break;
			case 2:
				m_pParent->AddMacViewUndo();
				Global::_pSong->Invalided=true;
				Global::_pSong->ExchangeInstruments(sel[0],sel[1]);
				
				Global::_pSong->Invalided=false;
				pParentMain->UpdateComboIns(true);
				break;
			}
			
			pParentMain->RedrawGearRackList();
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
				pParentMain->UpdateComboGen(true);
				if (m_pParent->viewMode==view_modes::machine)
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
				pParentMain->UpdateComboGen(true);
				if (m_pParent->viewMode==view_modes::machine)
				{
					m_pParent->Repaint();
				}
				break;
			case 2:
				Global::_pSong->Invalided=true;
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
				
				Global::_pSong->Invalided=false;
				pParentMain->UpdateComboIns(true);
				break;
			}
			
			pParentMain->RedrawGearRackList();
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
