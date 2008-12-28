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

		bool MixerGui::OnEvent(TestCanvas::Event* ev)
		{			
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog(ev->x, ev->y);
				return true;
			}
			return EffectGui::OnEvent(ev);
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
				dialog_->Generate(rc.left + absx() + x, rc.top + absy() + y);			
				char winname[32];
				sprintf(winname,"%.2X : %s",dialog_->MachineIndex
									   ,mac()->_editName);
				dialog_->SetWindowText(winname);
				dialog_->centerWindowOnPoint(rc.left + absx() + x, rc.top + absy() + y);
			}
		}

	}  // namespace host
}  // namespace psycle
