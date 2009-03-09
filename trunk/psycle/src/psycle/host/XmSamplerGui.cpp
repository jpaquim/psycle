#include "XmSamplerGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "ChildView.hpp"
#include "XMSamplerUI.hpp"

namespace psycle {
	namespace host {

		XmSamplerGui::XmSamplerGui(class MachineView* view,
#ifdef use_psycore
								   class psy::core::Machine* mac)
#else
								   class Machine* mac)
#endif
			: GeneratorGui(view, mac),
			  dialog_(0)
		{
		}

		XmSamplerGui::~XmSamplerGui()
		{
			if (dialog_)
				dialog_->DestroyWindow();
		}

		void XmSamplerGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}
		void XmSamplerGui::UpdateVU(CDC* devc) {
			GeneratorGui::UpdateVU(devc);
			if(dialog_) {
				dialog_->UpdateUI();
			}
		}

		void XmSamplerGui::ShowDialog(double x, double y)
		{
#ifdef use_psycore
			//todo
#else
			if ( !dialog_ ) {
				dialog_ = new XMSamplerUI(mac()->GetEditName(), this, view()->child_view());
				dialog_->Init((XMSampler*)mac());
				dialog_->Create(view()->child_view());
			}
#endif
		}

	}  // namespace host
}  // namespace psycle
