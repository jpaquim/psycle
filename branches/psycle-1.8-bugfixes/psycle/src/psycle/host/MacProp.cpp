///\file
///\brief implementation file for psycle::host::CMacProp.
#include <psycle/project.private.hpp>
#include "MacProp.hpp"
#include "Psycle.hpp"
#include "MainFrm.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		extern CPsycleApp theApp;

		CMacProp::CMacProp(CWnd* pParent)
			: CDialog(CMacProp::IDD, pParent)
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
				m_view->Repaint(DMMacRefresh);
			}
		}
		void CMacProp::OnBypass() 
		{
			pMachine->Bypass(m_bypassCheck.GetCheck() == 1);
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
		void CMacProp::OnBnClickedReplacemac()
		{
			Global::_pSong->seqBus = pMachine->_macIndex;
			int tmac = pMachine->_macIndex;
			CMainFrame* pParentMain = ((CMainFrame *)theApp.m_pMainWnd);

			Machine * mac = Global::_pSong->_pMachine[tmac];
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
						int j = Global::_pSong->_pMachine[outputMachines[i]]->FindInputWire(tmac);
						if (j >= 0)
						{
							Global::_pSong->_pMachine[outputMachines[i]]->GetWireVolume(j, outputConVol[i]);
						}
					}
				}

				m_view->NewMachine(x,y,tmac);
				// replace all the connection info

				mac = Global::_pSong->_pMachine[tmac];
				if (mac)
				{
					mac->_numOutputs = numOutputs;
					mac->_numInputs = numInputs;

					for (int i = 0; i < MAX_CONNECTIONS; i++)
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
				m_view->NewMachine(-1,-1,tmac);
			}
			strcpy(txt,Global::_pSong->_pMachine[tmac]->_editName);

			pParentMain->UpdateEnvInfo();
			pParentMain->UpdateComboGen();
			if (m_view->viewMode==VMMachine)
			{
				m_view->Repaint();
			}
			OnCancel();
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
