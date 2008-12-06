#include "GeneratorGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "FrameMachine.hpp"
#include "ChildView.hpp"

namespace psycle {
	namespace host {

		GeneratorGui::GeneratorGui(class MachineView* view,
							       class Machine* mac)
			: MachineGui(view, mac),
			  dialog_(0)
		{
		}

		GeneratorGui::~GeneratorGui()
		{		
		}

		bool GeneratorGui::OnEvent(TestCanvas::Event* ev)
		{
			MachineGui::OnEvent(ev);
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog();		
			}
			return true;
		}

		void GeneratorGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void GeneratorGui::ShowDialog()
		{
			if ( !dialog_ ) {
				dialog_ = new CFrameMachine(mac()->_macIndex, this);
				//CenterWindowOnPoint(m_pWndMac[tmac], point);
			}
		}

	}  // namespace host
}  // namespace psycle
