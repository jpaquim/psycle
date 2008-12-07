#include "MixerGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "FrameMachine.hpp"
#include "ChildView.hpp"

namespace psycle {
	namespace host {

		MixerGui::MixerGui(class MachineView* view,
					       class Machine* mac)
			: MachineGui(view, mac),
			  dialog_(0)
		{
		}

		MixerGui::~MixerGui()
		{		
		}

		bool MixerGui::OnEvent(TestCanvas::Event* ev)
		{
			MachineGui::OnEvent(ev);
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog();		
			}
			return true;
		}

		void MixerGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void MixerGui::ShowDialog()
		{
			if ( !dialog_ ) {
			//	dialog_ = new CFrameMachine(mac()->_macIndex, this);
				//CenterWindowOnPoint(m_pWndMac[tmac], point);
			}
		}

	}  // namespace host
}  // namespace psycle
