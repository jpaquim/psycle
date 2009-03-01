#include "MixerGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "FrameMixerMachine.hpp"
#include "ChildView.hpp"

namespace psycle {
	namespace host {

		MixerGui::MixerGui(class MachineView* view,
					       class Machine* mac)
			: EffectGui(view, mac),
			  dialog_(0)
		{
		}

		MixerGui::~MixerGui()
		{
			if (dialog_)
				dialog_->DestroyWindow();
		}

		void MixerGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void MixerGui::ShowDialog(double x, double y)
		{
			if ( !dialog_ ) {
				CRect rc;
				view()->parent()->GetWindowRect(rc);
				dialog_ = new CFrameMixerMachine(this);
				dialog_->LoadFrame(
								IDR_MACHINEFRAME, 
								WS_POPUPWINDOW | WS_CAPTION,
								view()->child_view());
				dialog_->SelectMachine(mac());
				dialog_->Generate(x, y);			
				dialog_->centerWindowOnPoint( x, y);
			}
		}
		void MixerGui::UpdateVU(CDC* devc)
		{
			EffectGui::UpdateVU(devc);
			if ( dialog_ ) {
				dialog_->UpdateUI();
			}
		}	

	}  // namespace host
}  // namespace psycle
