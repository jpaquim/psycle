///\file
///\brief implementation file for psycle::host::CMacProp.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/host/gui/Psycle.hpp>
#include <psycle/host/gui/MacProp.hpp>
#include <psycle/host/gui/MainFrm.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		extern CPsycleApp theApp;

		CMacProp::CMacProp(CWnd* pParent)
			: CDialog(CMacProp::IDD, pParent)
		{
			m_view=NULL;
			//{{AFX_DATA_INIT(CMacProp)
			//}}AFX_DATA_INIT
		}

		void CMacProp::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CMacProp)
			DDX_Control(pDX, IDC_SOLO, m_soloCheck);
			DDX_Control(pDX, IDC_BYPASS, m_bypassCheck);
			DDX_Control(pDX, IDC_MUTE, m_muteCheck);
			DDX_Control(pDX, IDC_EDIT1, m_macname);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CMacProp, CDialog)
			//{{AFX_MSG_MAP(CMacProp)
			ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
			ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
			ON_BN_CLICKED(IDC_MUTE, OnMute)
			ON_BN_CLICKED(IDC_BYPASS, OnBypass)
			ON_BN_CLICKED(IDC_SOLO, OnSolo)
			ON_BN_CLICKED(IDC_CLONE, OnClone)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CMacProp::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			deleted=false;

			m_macname.SetLimitText(31);
			char buffer[64];
			sprintf(buffer,"%.2X : %s Properties",Global::_pSong->FindBusFromIndex(thisMac),pMachine->_editName);
			SetWindowText(buffer);

			m_macname.SetWindowText(pMachine->_editName);

			m_muteCheck.SetCheck(pMachine->_mute);
			m_soloCheck.SetCheck(pSong->machineSoloed == thisMac);
			m_bypassCheck.SetCheck(pMachine->_bypass);
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
				m_view->Repaint(DMMacRefresh);
			}
		}
		void CMacProp::OnBypass() 
		{
			pMachine->_bypass = (m_bypassCheck.GetCheck() == 1);
			if ( m_view != NULL )
			{
				m_view->AddMacViewUndo();
				m_view->updatePar=thisMac;
				m_view->Repaint(DMMacRefresh);
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
				m_view->Repaint(DMAllMacsRefresh);
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
					if (!Global::_pSong->_pMachine[i])
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
					if (!Global::_pSong->_pMachine[i])
					{
						dst = i;
						break;
					}
				}
			}
			if (dst >= 0)
			{
				if (!Global::_pSong->CloneMac(src,dst))
				{
					MessageBox("Cloning failed","Cloning failed");
				}
				if ( m_view != NULL )
				{
					((CMainFrame *)theApp.m_pMainWnd)->UpdateComboGen(true);
					if (m_view->viewMode==VMMachine)
					{
						m_view->Repaint();
					}
				}
			}
			OnCancel();
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
