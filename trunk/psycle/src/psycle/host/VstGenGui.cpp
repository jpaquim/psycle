#include "VstGenGui.hpp"

#include "ChildView.hpp"
#include "MachineView.hpp"
#include "VstEffectWnd.hpp"

#include <psycle/core/song.h>

namespace psycle { namespace host {

VstGenGui::VstGenGui(MachineView* view, Machine* mac)
:
	GeneratorGui(view, mac),
	dialog_(0)
{}

VstGenGui::~VstGenGui() {
	if(dialog_) dialog_->DestroyWindow();
}

void VstGenGui::BeforeDeleteDlg() {
	dialog_ = 0;
}

void VstGenGui::ShowDialog(double x, double y) {
	if(!dialog_) {
		dialog_ = new CVstEffectWnd(reinterpret_cast<vst::plugin*>(mac()), this);
		// newwin->_pActive = &isguiopen[tmac];
		dialog_->LoadFrame(
			IDR_VSTFRAME, 
			//WS_OVERLAPPEDWINDOW,
			WS_POPUPWINDOW | WS_CAPTION,
			view()->child_view()
		);
		std::ostringstream winname;
		winname << std::hex << std::setw(2)
			<< view()->song()->FindBusFromIndex(mac()->id())
				<< " : " << mac()->GetEditName();
				dialog_->SetTitleText(winname.str().c_str());
		// C_Tuner.dll crashes if asking size before opening.
		//newwin->ResizeWindow(0);*/
		dialog_->ShowWindow(SW_SHOWNORMAL);
		dialog_->PostOpenWnd();
		dialog_->CenterWindowOnPoint(x, y);
	}
}

}}
