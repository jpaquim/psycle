#include "SamplerGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "FrameMachine.hpp"
#include "ChildView.hpp"
#include "GearTracker.hpp"

namespace psycle {
	namespace host {

		SamplerGui::SamplerGui(class MachineView* view,
						       class Machine* mac)
			: GeneratorGui(view, mac),
			  dialog_(0)
		{			
		}

		SamplerGui::~SamplerGui()
		{
			if (dialog_)
				dialog_->DestroyWindow();
		}

		bool SamplerGui::OnEvent(TestCanvas::Event* ev)
		{			
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog();
				return true;
			}
			return GeneratorGui::OnEvent(ev);			
		}

		void SamplerGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void SamplerGui::ShowDialog()
		{
			if ( !dialog_ ) {
				dialog_ = new CGearTracker(this);
				dialog_->_pMachine = (Sampler*)mac();
				dialog_->Create();
				dialog_->ShowWindow(SW_SHOW);
			}
		}

	}  // namespace host
}  // namespace psycle
