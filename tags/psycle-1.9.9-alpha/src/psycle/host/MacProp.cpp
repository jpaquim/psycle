///\file
///\brief implementation file for psycle::host::CMacProp.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/Psycle.hpp>
#include <psycle/host/MacProp.hpp>
#include <psycle/host/MainFrm.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		extern CPsycleApp theApp;

		CMacProp::CMacProp(CWnd* pParent)
		:
			CDialog(CMacProp::IDD, pParent),
			m_view(0)
		{}

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
		END_MESSAGE_MAP()

		BOOL CMacProp::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			deleted=false;

			{
				std::ostringstream s;
				s	<< std::hex << std::setfill('0') << std::setw(2)
					<< Global::song().FindBusFromIndex(thisMac) << ": " << pMachine->GetEditName() << "  Properties";
				SetWindowText(s.str().c_str());
			}

			m_macname.SetWindowText(pMachine->GetEditName().c_str());

			m_muteCheck.SetCheck(pMachine->_mute);
			m_soloCheck.SetCheck(pSong->machineSoloed == thisMac);
			m_bypassCheck.SetCheck(pMachine->_bypass);
			if (pMachine->_mode == MACHMODE_GENERATOR ) 
				m_bypassCheck.ShowWindow(SW_HIDE);
			else 
				m_soloCheck.ShowWindow(SW_HIDE);
			return true;
		}

		void CMacProp::OnChangeEdit1() 
		{
			char value[32];
			 m_macname.GetWindowText(value,32);
			 txt = value;
		}

		void CMacProp::OnButton1() 
		{
			// Delete MACHINE!
			if (MessageBox("Are you sure?","Delete Machine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
			{
				if(m_view) m_view->AddMacViewUndo();
				deleted = true;
				OnCancel();
			}
		}

		void CMacProp::OnMute() 
		{
			pMachine->_mute = (m_muteCheck.GetCheck() == 1);
			pMachine->_volumeCounter=0.0f;
			pMachine->_volumeDisplay = 0;
			if(m_view)
			{
				m_view->AddMacViewUndo();
				m_view->updatePar=thisMac;
				m_view->Repaint(draw_modes::machine);
			}
		}
		void CMacProp::OnBypass() 
		{
			pMachine->_bypass = (m_bypassCheck.GetCheck() == 1);
			if(m_view)
			{
				m_view->AddMacViewUndo();
				m_view->updatePar=thisMac;
				m_view->Repaint(draw_modes::machine);
			}
		}

		void CMacProp::OnSolo() 
		{
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
		}

		void CMacProp::OnClone() 
		{
			Machine::id_type src(pMachine->id());
			Machine::id_type dst(-1);

			if ((src < MAX_BUSES) && (src >=0))
			{
				// we need to find an empty slot
				for (Machine::id_type i(0); i < MAX_BUSES; i++)
				{
					if (!Global::song()._pMachine[i])
					{
						dst = i;
						break;
					}
				}
			}
			else if ((src < MAX_BUSES*2) && (src >= MAX_BUSES))
			{
				for (Machine::id_type i(MAX_BUSES); i < MAX_BUSES*2; i++)
				{
					if (!Global::song()._pMachine[i])
					{
						dst = i;
						break;
					}
				}
			}
			if (dst >= 0)
			{
				((CMainFrame *)theApp.m_pMainWnd)->m_wndView.AddMacViewUndo();
				if (!Global::song().CloneMac(src,dst))
				{
					MessageBox("Cloning failed","Cloning failed");
				}
				if(m_view)
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
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
