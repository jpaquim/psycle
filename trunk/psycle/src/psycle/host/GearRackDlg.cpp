///\file
///\brief implementation file for psycle::host::CGearRackDlg.

#include "GearRackDlg.hpp"
#include "WaveEdFrame.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/song.h>
#include <psycle/core/machine.h>
#include <psycle/core/machinefactory.h>
using namespace psycle::core;
#else
#include "Song.hpp"
#include "Machine.hpp"
#endif
#include "MainFrm.hpp"
#include "PatternView.hpp"
#include "MachineView.hpp"

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		int CGearRackDlg::DisplayMode = 0;

		CGearRackDlg::CGearRackDlg(MachineView* view)
			: CDialog(CGearRackDlg::IDD, view->child_view()),
			  view_(view)
		{
			//{{AFX_DATA_INIT(CGearRackDlg)
			//}}AFX_DATA_INIT
			m_pParent = view->child_view();
			pParentMain = view->main();
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

				selected = Global::song().seqBus;
				if (selected >= MAX_BUSES)
				{
					selected = 0;
				}
				for (b=0; b<MAX_BUSES; b++) // Check Generators
				{
					if(Global::song().machine(b))
					{
						sprintf(buffer,"%.2X: %s",b,Global::song().machine(b)->GetEditName().c_str());
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

				selected = Global::song().seqBus;
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
					if(Global::song().machine(b))
					{
						sprintf(buffer,"%.2X: %s",b,Global::song().machine(b)->GetEditName().c_str());
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
					sprintf(buffer, "%.2X: %s", b, Global::song()._pInstrument[b]->_sName);
					m_list.AddString(buffer);
				}
				CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
				if (cc->GetCurSel() == AUX_WAVES)
				{
					selected = Global::song().instSelected();
				}
				else
				{
					cc->SetCurSel(AUX_WAVES);
					pParentMain->UpdateComboIns(true);
					selected = Global::song().instSelected();
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
				Global::song().seqBus = tmac;
				pParentMain->UpdateComboGen();
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					if (cc->GetCurSel() == AUX_WAVES)
					{
						Global::song().instSelected(tmac);
						Global::song().auxcolSelected=tmac;
						pParentMain->UpdateComboIns(false);
					}
					else
					{
						cc->SetCurSel(AUX_WAVES);
						Global::song().instSelected(tmac);
						Global::song().auxcolSelected=tmac;
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
				//fallthrough
			case 0:
				{
					view_->ShowNewMachineDlg(-1,-1,view_->song()->machine(tmac), false);
				}
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					cc->SetCurSel(AUX_WAVES);
					Global::song().instSelected(tmac);
					Global::song().auxcolSelected=tmac;
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
			case 1:
				tmac+=MAX_BUSES;
				//fallthrough
			case 0:
				if (MessageBox("Are you sure?","Delete Machine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
				{
					if (Global::song().machine(tmac))
					{
						view_->DeleteMachineGui(view_->song()->machine(tmac));
#if PSYCLE__CONFIGURATION__USE_PSYCORE
						Global::song().DeleteMachine(view_->song()->machine(tmac));
#else
						Global::song().DestroyMachine(tmac);
#endif
						pParentMain->UpdateEnvInfo();
						pParentMain->UpdateComboGen();
						if (m_pParent->viewMode==view_modes::machine)
						{
							m_pParent->pattern_view()->Repaint(PatternView::draw_modes::all);
						}
					}
				}
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					cc->SetCurSel(AUX_WAVES);
						Global::song().instSelected(tmac);
						Global::song().auxcolSelected=tmac;
					pParentMain->UpdateComboIns(true);
					pParentMain->m_wndInst.WaveUpdate();
				}
				Global::song().DeleteInstrument(Global::song().instSelected());
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
				if (Global::song().machine(tmac))
				{
					view_->DoMacPropDialog(view_->song()->machine(tmac), false);
				}
				break;
			case 1:
				if (Global::song().machine(tmac+MAX_BUSES))
				{
					pParentMain->UpdateEnvInfo();
					pParentMain->UpdateComboGen();
					if (m_pParent->viewMode==view_modes::machine)
					{
						m_pParent->pattern_view()->Repaint(PatternView::draw_modes::all);
					}
				}
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					cc->SetCurSel(AUX_WAVES);
						Global::song().instSelected(tmac);
						Global::song().auxcolSelected=tmac;
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
				if (view_->song()->machine(tmac))
				{
					view_->ShowDialog(view_->song()->machine(tmac),
									  point.x,
									  point.y);
				}
				break;
			case 1:
				if (view_->song()->machine(tmac+MAX_BUSES))
				{
					view_->ShowDialog(view_->song()->machine(tmac+MAX_BUSES),
									  point.x,
									  point.y);
				}
				break;
			case 2:
				{
					CComboBox *cc=(CComboBox *)pParentMain->m_wndControl2.GetDlgItem(IDC_AUXSELECT);
					cc->SetCurSel(AUX_WAVES);
					Global::song().instSelected(tmac);
					Global::song().auxcolSelected=tmac;
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
			case 1:
				sel[0]+=MAX_BUSES;
				sel[1]+=MAX_BUSES;
				//fallthrough
			case 0:
				m_pParent->AddMacViewUndo();
				Global::song().ExchangeMachines(sel[0],sel[1]);
				pParentMain->UpdateComboGen(true);
				view_->Rebuild();				
				if (m_pParent->viewMode==view_modes::machine)
				{
					m_pParent->pattern_view()->Repaint(PatternView::draw_modes::all);
				}
				break;
			case 2:
				m_pParent->AddMacViewUndo();
				Global::song().IsInvalided(true);
				Global::song().ExchangeInstruments(sel[0],sel[1]);
				
				Global::song().IsInvalided(false);
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
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			case 1:
				tmac1+=MAX_BUSES;
				if (tmac2 >= 0)
				{
					tmac2+=MAX_BUSES;
				}
				//fallthrough
			case 0:
				if (!Global::song().machine(tmac1)) {
					MessageBox("The first selection does not correspond to a valid machine","Gear Rack Dialog");
					return;
				} else if(tmac2 != -1 && Global::song().machine(tmac2)) {
					MessageBox("You cannot clone over an existing machine. Please select an empty slot.","Gear Rack Dialog");
					return;
				}
				{
					Machine* newMac = MachineFactory::getInstance().CloneMachine(*Global::song().machine(tmac1));
					if (!newMac) {
						MessageBox("Cloning process failed","Gear Rack Dialog");
						return;
					}
					Global::song().AddMachine(newMac, tmac2);
					view_->CreateMachineGui(newMac);
				}
				pParentMain->UpdateComboGen(true);
				if (m_pParent->viewMode==view_modes::machine)
				{
					m_pParent->pattern_view()->Repaint(PatternView::draw_modes::all);
				}
				break;
#else
			case 0:
				if (tmac2 < 0)
				{
					tmac2 = Global::song().GetFreeBus();
				}
				if (tmac2 >= 0)
				{
					if (!Global::song().CloneMac(tmac1,tmac2))
					{
						MessageBox("Select 1 active slot (and optionally 1 empty destination slot)","Gear Rack Dialog");
						return;
					}
					else {
						view_->CreateMachineGui(view_->song()->machine(tmac2));
					}
				}
				pParentMain->UpdateComboGen(true);
				if (m_pParent->viewMode==view_modes::machine)
				{
					m_pParent->pattern_view()->Repaint(PatternView::draw_modes::all);
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
					tmac2 = Global::song().GetFreeFxBus();
				}
				if (tmac2 >= 0)
				{
					if (!Global::song().CloneMac(tmac1,tmac2))
					{
						MessageBox("Select 1 active slot (and optionally 1 empty destination slot)","Gear Rack Dialog");
						return;
					}
				}
				pParentMain->UpdateComboGen(true);
				if (m_pParent->viewMode==view_modes::machine)
				{
					m_pParent->pattern_view()->Repaint(PatternView::draw_modes::all);
				}
				break;
#endif
			case 2:
				Global::song().IsInvalided(true);
				if (tmac2 < 0)
				{
					for (int i = 0; i < MAX_INSTRUMENTS; i++)
					{
						if (Global::song()._pInstrument[i]->Empty())
						{
							tmac2 = i;
							break;
						}
					}
				}
				if (tmac2 >=0)
				{
					if (!Global::song().CloneIns(tmac1,tmac2))
					{
						MessageBox("Select 1 active slot (and optionally 1 empty destination slot)","Gear Rack Dialog");
						return;
					}
				}
				
				Global::song().IsInvalided(false);
				pParentMain->UpdateComboIns(true);
				break;
			}
			
			pParentMain->RedrawGearRackList();
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
