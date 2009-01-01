///\file
///\brief implementation file for psycle::host::CMacProp.

#include "MacProp.hpp"
#include "Psycle.hpp"
#include "MainFrm.hpp"
#include "MachineGui.hpp"
#include "MachineView.hpp"

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		extern CPsycleApp theApp;

		CMacProp::CMacProp(CWnd* pParent)
			: CDialog(CMacProp::IDD, pParent)
		{
			m_view=NULL;
		}

		CMacProp::CMacProp(MachineGui* gui)
			: CDialog(CMacProp::IDD, 0),
			  gui_(gui)
		{
			m_view=NULL;
		}

		void CMacProp::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_SOLO, m_soloCheck);
			DDX_Control(pDX, IDC_BYPASS, m_bypassCheck);
			DDX_Control(pDX, IDC_MUTE, m_muteCheck);
			DDX_Control(pDX, IDC_EDIT1, m_macname);
		}

		BEGIN_MESSAGE_MAP(CMacProp, CDialog)
			ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
			ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
			ON_BN_CLICKED(IDC_MUTE, OnMute)
			ON_BN_CLICKED(IDC_BYPASS, OnBypass)
			ON_BN_CLICKED(IDC_SOLO, OnSolo)
			ON_BN_CLICKED(IDC_CLONE, OnClone)
			ON_BN_CLICKED(IDC_REPLACEMAC, OnBnClickedReplacemac)
		END_MESSAGE_MAP()

		BOOL CMacProp::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			deleted=false;
			replaced=false;

			m_macname.SetLimitText(31);
			char buffer[64];
			sprintf(buffer,"%.2X : %s Properties",Global::_pSong->FindBusFromIndex(thisMac),pMachine->_editName);
			SetWindowText(buffer);

			m_macname.SetWindowText(pMachine->_editName);

			m_muteCheck.SetCheck(pMachine->_mute);
			m_soloCheck.SetCheck(pSong->machineSoloed == thisMac);
			m_bypassCheck.SetCheck(pMachine->Bypass());
			if (pMachine->_mode == MACHMODE_GENERATOR ) 
			{
				m_bypassCheck.ShowWindow(SW_HIDE);
			}
			else 
			{
				m_soloCheck.ShowWindow(SW_HIDE);
			}
			return TRUE;
			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
		}

		void CMacProp::OnChangeEdit1() 
		{
			m_macname.GetWindowText(txt, 32);
		}

		void CMacProp::OnButton1() 
		{
			// Delete MACHINE!
			if (MessageBox("Are you sure?","Delete Machine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
			{
				if ( m_view != NULL )
				{
					m_view->AddMacViewUndo();
				}

				deleted = true;
				OnCancel();
			}
		}

		void CMacProp::OnMute() 
		{
			pMachine->_mute = (m_muteCheck.GetCheck() == 1);
			pMachine->_volumeCounter=0.0f;
			pMachine->_volumeDisplay = 0;
			if ( m_view != NULL )
			{
				m_view->AddMacViewUndo();
				m_view->updatePar=thisMac;
				m_view->Repaint(draw_modes::machine);
			}
#ifdef use_test_canvas
			gui_->SetMute(pMachine->_mute);
			gui_->QueueDraw();
#endif
		}
		void CMacProp::OnBypass() 
		{
			pMachine->Bypass(m_bypassCheck.GetCheck() == 1);
			if ( m_view != NULL )
			{
				m_view->AddMacViewUndo();
				m_view->updatePar=thisMac;
				m_view->Repaint(draw_modes::machine);
			}
		}

		void CMacProp::OnSolo() 
		{
#ifdef use_test_canvas
			gui_->view()->SetSolo(pMachine);
			gui_->QueueDraw();
#else
			m_view->AddMacViewUndo();
			if (m_soloCheck.GetCheck() == 1)
			{
				for ( int i=0;i<MAX_MACHINES;i++ )
				{
					if (pSong->_pMachine[i])
					{
						if ( pSong->_pMachine[i]->_mode == MACHMODE_GENERATOR )
						{
							pSong->_pMachine[i]->_mute = true;
							pSong->_pMachine[i]->_volumeCounter=0.0f;
							pSong->_pMachine[i]->_volumeDisplay =0;
						}
					}
				}
				pMachine->_mute = false;
				if ( m_muteCheck.GetCheck() ) m_muteCheck.SetCheck(0);
				pSong->machineSoloed = thisMac;
			}
			else
			{
				pSong->machineSoloed = -1;
				for ( int i=0;i<MAX_BUSES;i++ )
				{
					if (pSong->_pMachine[i])
					{
						pSong->_pMachine[i]->_mute = false;
					}
				}
				if ( m_muteCheck.GetCheck() ) m_muteCheck.SetCheck(0);
			}
			if ( m_view != NULL )
			{
				m_view->Repaint(draw_modes::all_machines);
			}
#endif
		}

		void CMacProp::OnClone() 
		{
			int src = pMachine->_macIndex;
			int dst = -1;

			if ((src < MAX_BUSES) && (src >=0))
			{
				dst = Global::_pSong->GetFreeBus();
			}
			else if ((src < MAX_BUSES*2) && (src >= MAX_BUSES))
			{
				dst = Global::_pSong->GetFreeFxBus();
			}
			if (dst >= 0)
			{
				if (!Global::_pSong->CloneMac(src,dst))
				{
					MessageBox("Cloning failed","Cloning failed");
				}
#ifdef use_test_canvas
				else {
					gui_->view()->CreateMachineGui(gui_->view()->song()->_pMachine[dst]);
				}
#endif
				if ( m_view != NULL )
				{
					((CMainFrame *)theApp.m_pMainWnd)->UpdateComboGen(true);
					if (m_view->viewMode==view_modes::machine)
					{
						m_view->Repaint();
					}
				}
			}
			OnCancel();
		}
		void CMacProp::OnBnClickedReplacemac()
		{
			int index = pMachine->_macIndex;
#ifdef use_test_canvas
			replaced = true;
#else
			m_view->NewMachine(pMachine->_x,pMachine->_y,index);
			strcpy(txt,Global::_pSong->_pMachine[index]->_editName);

			CMainFrame* pParentMain = ((CMainFrame *)theApp.m_pMainWnd);
			pParentMain->UpdateEnvInfo();
			pParentMain->UpdateComboGen();
			if (m_view->viewMode==view_modes::machine)
			{
				m_view->Repaint();
			}
#endif
			OnCancel();
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
