#include "RecorderGui.hpp"

#include "MachineView.hpp"
#include "WaveInMacDlg.hpp"
#include "ChildView.hpp"

#include <psycle/core/machine.h>

namespace psycle { namespace host {

using namespace core;

		RecorderGui::RecorderGui(class MachineView* view, Machine* mac)
			: GeneratorGui(view, mac),
			  dialog_(0)
		{
		}

		RecorderGui::~RecorderGui()
		{	
			if (dialog_)
				dialog_->DestroyWindow();
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
				dialog_->Show(x, y);			
			}
		}

	}  // namespace host
}  // namespace psycle
