#include "SamplerGui.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/sampler.h>
using namespace psy::core;
#else
#include "Sampler.hpp"
#endif
#include "ChildView.hpp"
#include "MachineView.hpp"
#include "GearTracker.hpp"

namespace psycle {
	namespace host {

		SamplerGui::SamplerGui(MachineView* view,
							   Machine* mac)
			: GeneratorGui(view, mac),
			  dialog_(0)
		{			
		}

		SamplerGui::~SamplerGui()
		{
			if (dialog_)
				dialog_->DestroyWindow();
		}

		void SamplerGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void SamplerGui::ShowDialog(double x, double y)
		{
			if ( !dialog_ ) {
				dialog_ = new CGearTracker(this);
				dialog_->Create();
				dialog_->ShowWindow(SW_SHOW);
			}
		}

	}  // namespace host
}  // namespace psycle
