///\file
///\brief implementation file for psycle::host::CMacProp.

#include "MacProp.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/song.h>
#include <psycle/core/machine.h>
#include <psycle/core/machinefactory.h>
using namespace psycle::core;
#else
#include "Machine.hpp"
#include "Song.hpp"
#endif

#include "MainFrm.hpp"
#include "MachineView.hpp"
#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		extern CPsycleApp theApp;

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
			sprintf(buffer,"%.2X : %s Properties",Global::song().FindBusFromIndex(thisMac),pMachine->GetEditName().c_str());
			m_macname.SetWindowText(pMachine->GetEditName().c_str());

			SetWindowText(buffer);



			m_muteCheck.SetCheck(pMachine->_mute);
			m_soloCheck.SetCheck(pSong->machineSoloed == thisMac);
			m_bypassCheck.SetCheck(pMachine->Bypass());

			if (pMachine->IsGenerator() )
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
			}
			gui_->SetMute(pMachine->_mute);
			gui_->QueueDraw();
		}
		void CMacProp::OnBypass() 
		{
			pMachine->Bypass(m_bypassCheck.GetCheck() == 1);
			if ( m_view != NULL )
			{
				m_view->AddMacViewUndo();
				m_view->updatePar=thisMac;				
				gui_->QueueDraw();
			}
		}

		void CMacProp::OnSolo() 
		{
			gui_->view()->SetSolo(pMachine);
			gui_->QueueDraw();
		}

		void CMacProp::OnClone() 
		{
			int src = pMachine->id();
			int dst = -1;

			if ((src < MAX_BUSES) && (src >=0))
			{
				dst = gui_->view()->song()->GetFreeBus();
			}
			else if ((src < MAX_BUSES*2) && (src >= MAX_BUSES))
			{
				dst = gui_->view()->song()->GetFreeFxBus();
			}
			if (dst >= 0)
			{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				Machine * newMac = MachineFactory::getInstance().CloneMachine(*pMachine);
				if(!newMac)
				{
					MessageBox("Cloning failed","Cloning failed");
				}
				else {
					gui_->view()->song()->AddMachine(newMac);
					gui_->view()->CreateMachineGui(newMac);
				}
#else
				if (!gui_->view()->song()->CloneMac(src,dst))
				{
					MessageBox("Cloning failed","Cloning failed");
				}
				else {
					gui_->view()->CreateMachineGui(gui_->view()->song()->machine(dst));
				}
#endif

				if ( m_view != NULL )
				{
					((CMainFrame *)theApp.m_pMainWnd)->UpdateComboGen(true);
					if (m_view->viewMode==view_modes::machine)
					{
						m_view->Invalidate();
					}
				}
			}
			OnCancel();
		}
		void CMacProp::OnBnClickedReplacemac()
		{
			replaced = true;
			OnCancel();
		}

	}   // namespace
}   // namespace
