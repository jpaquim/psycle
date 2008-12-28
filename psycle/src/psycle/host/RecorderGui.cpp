#include "RecorderGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "WaveInMacDlg.hpp"
#include "ChildView.hpp"

namespace psycle {
	namespace host {

		RecorderGui::RecorderGui(class MachineView* view,
							     class Machine* mac)
			: GeneratorGui(view, mac),
			  dialog_(0)
		{
		}

		RecorderGui::~RecorderGui()
		{	
			if (dialog_)
				dialog_->DestroyWindow();
		}

		bool RecorderGui::OnEvent(TestCanvas::Event* ev)
		{			
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog(ev->x, ev->y);
				return true;
			}
			return GeneratorGui::OnEvent(ev);
			
		}

		void RecorderGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void RecorderGui::ShowDialog(double x, double y)
		{
			if ( !dialog_ ) {
				dialog_ = new CWaveInMacDlg(view()->child_view(), this);
				dialog_->pRecorder = (AudioRecorder*)mac();
				dialog_->Create();
				CRect rc;
				view()->parent()->GetWindowRect(rc);
				dialog_->Show(rc.left + absx() + x, rc.top + absy() + y);			
			}
		}

	}  // namespace host
}  // namespace psycle
