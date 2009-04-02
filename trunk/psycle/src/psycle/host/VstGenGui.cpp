#include "VstGenGui.hpp"
#ifdef use_psycore
#include <psycle/core/song.h>
using namespace psy::core;
#else
#include "Song.hpp"
#include "Machine.hpp"
#endif


#include "ChildView.hpp"
#include "MachineView.hpp"

#include "VstEffectWnd.hpp"

namespace psycle {
	namespace host {

		VstGenGui::VstGenGui(MachineView* view,
							 Machine* mac)
			: GeneratorGui(view, mac),
			  dialog_(0)
		{
		}

		VstGenGui::~VstGenGui()
		{
			if (dialog_) {
				dialog_->DestroyWindow();
			}
		}

		void VstGenGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void VstGenGui::ShowDialog(double x, double y)
		{
			if ( !dialog_ ) {
				dialog_ = new CVstEffectWnd(reinterpret_cast<vst::plugin*>(mac()), this);
				// newwin->_pActive = &isguiopen[tmac];
				dialog_->LoadFrame(IDR_VSTFRAME, 
//					WS_OVERLAPPEDWINDOW,
					WS_POPUPWINDOW | WS_CAPTION,
					view()->child_view());
				std::ostringstream winname;
				winname << std::hex << std::setw(2)
					<< view()->song()->FindBusFromIndex(mac()->id())
						<< " : " << mac()->GetEditName();
						dialog_->SetTitleText(winname.str().c_str());
							// C_Tuner.dll crashes if asking size before opening.
//							newwin->ResizeWindow(0);*/
				dialog_->ShowWindow(SW_SHOWNORMAL);
				dialog_->PostOpenWnd();
//				CenterWindowOnPoint(m_pWndMac[tmac], point);
			}
		}

	}  // namespace host
}  // namespace psycle
