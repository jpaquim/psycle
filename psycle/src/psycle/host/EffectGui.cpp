#include "EffectGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "FrameMachine.hpp"
#include "ChildView.hpp"

namespace psycle {
	namespace host {

		EffectGui::EffectGui(class MachineView* view,
							 class Machine* mac)
			: MachineGui(view, mac),
			  dialog_(0)
		{
		}

		EffectGui::~EffectGui()
		{
		}

		bool EffectGui::OnEvent(TestCanvas::Event* ev)
		{
			MachineGui::OnEvent(ev);
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog();		
			}
			return true;
		}

		void EffectGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void EffectGui::ShowDialog()
		{
			if ( !dialog_ ) {
				dialog_ = new CFrameMachine(mac()->_macIndex, this);
				//CenterWindowOnPoint(m_pWndMac[tmac], point);
			}
		}

	}  // namespace host
}  // namespace psycle
