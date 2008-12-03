#include "MasterGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"

namespace psycle {
	namespace host {

		MasterGui::MasterGui(class MachineView* view,
							 class Machine* mac)
			: MachineGui(view, mac),
			  dialog_(0)
		{
		}

		MasterGui::~MasterGui()
		{
			if ( dialog_ )
				delete dialog_;
		}

		bool MasterGui::OnEvent(TestCanvas::Event* ev)
		{
			MachineGui::OnEvent(ev);
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog();		
			}
			return true;
		}

		void MasterGui::ShowDialog()
		{
			if ( !dialog_ ) {
				dialog_ = new CMasterDlg(view()->child_view());
				dialog_->_pMachine = (Master*)mac();
				for (int i=0;i<MAX_CONNECTIONS; i++)
				{
					if (mac()->_inputCon[i])
					{
						if (view()->song()->_pMachine[mac()->_inputMachines[i]])
						{
							strcpy(dialog_->macname[i],view()->song()->_pMachine[mac()->_inputMachines[i]]->_editName);
						}
					}
				}
				dialog_->Create();
//				CPoint point(-1,-1);
//				CenterWindowOnPoint(dialog_, point);
				dialog_->ShowWindow(SW_SHOW);
			} else {
				dialog_->ShowWindow(SW_SHOW);
			}
		}

	}  // namespace host
}  // namespace psycle
