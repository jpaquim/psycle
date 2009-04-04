#include "XmSamplerGui.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/song.h>
#include <psycle/core/xmsampler.h>
using namespace psy::core;
#else
#include "Song.hpp"
#include "XMSampler.hpp"
#endif
#include "ChildView.hpp"
#include "MachineView.hpp"
#include "XMSamplerUI.hpp"

namespace psycle {
	namespace host {

		XmSamplerGui::XmSamplerGui(MachineView* view,
								   Machine* mac)
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
			if ( !dialog_ ) {
				dialog_ = new XMSamplerUI(mac()->GetEditName().c_str(), this, view()->child_view());
				dialog_->Init((XMSampler*)mac());
				dialog_->Create(view()->child_view());
			}
		}

	}  // namespace host
}  // namespace psycle
