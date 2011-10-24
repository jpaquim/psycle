///\file
///\brief implementation file for psycle::host::CMacProp.

#include <psycle/host/detail/project.private.hpp>
#include "MacProp.hpp"

#include "MachineGui.hpp"
#include "MachineView.hpp"
#include "MainFrm.hpp"

#include <psycle/core/machine.h>
#include <psycle/core/machinefactory.h>
#include <psycle/core/song.h>

namespace psycle { namespace host {

		BEGIN_MESSAGE_MAP(CMacProp, CDialog)
			ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
			ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
			ON_BN_CLICKED(IDC_MUTE, OnMute)
			ON_BN_CLICKED(IDC_BYPASS, OnBypass)
			ON_BN_CLICKED(IDC_SOLO, OnSolo)
			ON_BN_CLICKED(IDC_CLONE, OnClone)
			ON_BN_CLICKED(IDC_REPLACEMAC, OnBnClickedReplacemac)
		END_MESSAGE_MAP()

		CMacProp::CMacProp(MachineGui* gui)
			: CDialog(CMacProp::IDD, 0),
			  gui_(gui) {
		}

		void CMacProp::DoDataExchange(CDataExchange* pDX) {
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_SOLO, m_soloCheck);
			DDX_Control(pDX, IDC_BYPASS, m_bypassCheck);
			DDX_Control(pDX, IDC_MUTE, m_muteCheck);
			DDX_Control(pDX, IDC_EDIT1, m_macname);
		}

		BOOL CMacProp::OnInitDialog() {
			CDialog::OnInitDialog();

			deleted=false;
			replaced=false;

			m_macname.SetLimitText(31);
			char buffer[64];
			sprintf(buffer,"%.2X : %s Properties",gui_->view()->song()->FindBusFromIndex(gui_->mac()->id()), gui_->mac()->GetEditName().c_str());
			m_macname.SetWindowText(gui_->mac()->GetEditName().c_str());
			SetWindowText(buffer);
			m_muteCheck.SetCheck(gui_->mac()->_mute);			
			m_soloCheck.SetCheck(gui_->view()->song()->machineSoloed == gui_->mac()->id());
			m_bypassCheck.SetCheck(gui_->mac()->Bypass());

			if (gui_->mac()->IsGenerator()) {
				m_bypassCheck.ShowWindow(SW_HIDE);
			}
			else {
				m_soloCheck.ShowWindow(SW_HIDE);
			}
			return TRUE;
			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
		}

		void CMacProp::OnChangeEdit1() {
			m_macname.GetWindowText(txt, 32);
		}

		void CMacProp::OnButton1() {
			// Delete MACHINE!
			if (MessageBox("Are you sure?","Delete Machine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES) {
				deleted = true;
				OnCancel();
			}
		}

		void CMacProp::OnMute() {
			gui_->mac()->_mute = (m_muteCheck.GetCheck() == 1);
			gui_->mac()->_volumeCounter=0.0f;
			gui_->mac()->_volumeDisplay = 0;
			gui_->view()->child_view()->updatePar=gui_->mac()->id();
			gui_->SetMute(gui_->mac()->_mute);
			gui_->QueueDraw();
		}

		void CMacProp::OnBypass() {
			gui_->mac()->Bypass(m_bypassCheck.GetCheck() == 1);
			gui_->view()->child_view()->updatePar=gui_->mac()->id();				
			gui_->QueueDraw();
		}

		void CMacProp::OnSolo() {
			gui_->view()->SetSolo(gui_->mac());
			gui_->QueueDraw();
		}

		void CMacProp::OnClone() {
			int src = gui_->mac()->id();
			int dst = -1;

			if ((src < MAX_BUSES) && (src >=0)) {
				dst = gui_->view()->song()->GetFreeBus();
			}
			else if ((src < MAX_BUSES*2) && (src >= MAX_BUSES)) {
				dst = gui_->view()->song()->GetFreeFxBus();
			}
			if (dst >= 0) {
				Machine * newMac = MachineFactory::getInstance().CloneMachine(*gui_->mac());
				if(!newMac)
				{
					MessageBox("Cloning failed","Cloning failed");
				}
				else {
					gui_->view()->song()->AddMachine(newMac);
					gui_->view()->CreateMachineGui(newMac);
				}
				gui_->view()->main()->UpdateComboGen(true);
				if (gui_->view()->child_view()->viewMode==view_modes::machine) {
					gui_->view()->child_view()->Invalidate();
				}
			}
			OnCancel();
		}

		void CMacProp::OnBnClickedReplacemac() {
			replaced = true;
			OnCancel();
		}

	}   // namespace
}   // namespace
