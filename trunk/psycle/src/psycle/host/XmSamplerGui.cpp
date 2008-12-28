#include "XmSamplerGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "ChildView.hpp"
#include "XMSamplerUI.hpp"

namespace psycle {
	namespace host {

		XmSamplerGui::XmSamplerGui(class MachineView* view,
							       class Machine* mac)
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

		void XmSamplerGui::ShowDialog(double x, double y)
		{
			if ( !dialog_ ) {
				dialog_ = new XMSamplerUI(mac()->GetEditName(), this, view()->child_view());
				dialog_->Init((XMSampler*)mac());
				dialog_->Create(view()->child_view());
			}
		}

	}  // namespace host
}  // namespace psycle
