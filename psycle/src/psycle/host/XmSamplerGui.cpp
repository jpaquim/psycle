// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#include "XmSamplerGui.hpp"

#include "ChildView.hpp"
#include "MachineView.hpp"
#include "XMSamplerUI.hpp"

#include <psycle/core/song.h>
#include <psycle/core/xmsampler.h>

namespace psycle {
	namespace host {
		using namespace core;

		XmSamplerGui::XmSamplerGui(MachineView* view,
								   Machine* mac)
			: GeneratorGui(view, mac),
			  dialog_(0) {
		}

		XmSamplerGui::~XmSamplerGui() {
			if (dialog_)
				dialog_->DestroyWindow();
		}

		void XmSamplerGui::BeforeDeleteDlg() {
			dialog_ = 0;
		}

		void XmSamplerGui::UpdateVU(CDC* devc) {
			GeneratorGui::UpdateVU(devc);
			if(dialog_) {
				dialog_->UpdateUI();
			}
		}

		void XmSamplerGui::ShowDialog(double x, double y) {
			if (!dialog_) {
				dialog_ = new XMSamplerUI(mac()->GetEditName().c_str(), this, view()->child_view());
				dialog_->Init((XMSampler*)mac());
				dialog_->Create(view()->child_view());
			}
		}

	}  // namespace host
}  // namespace psycle
