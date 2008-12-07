#include "VstFxGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "FrameMachine.hpp"
#include "ChildView.hpp"
#include "VstEffectWnd.hpp"

namespace psycle {
	namespace host {

		VstFxGui::VstFxGui(class MachineView* view,
							       class Machine* mac)
			: MachineGui(view, mac),
			  dialog_(0)
		{
		}

		VstFxGui::~VstFxGui()
		{		
		}

		bool VstFxGui::OnEvent(TestCanvas::Event* ev)
		{
			MachineGui::OnEvent(ev);
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog();		
			}
			return true;
		}

		void VstFxGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void VstFxGui::ShowDialog()
		{
			if ( !dialog_ ) {
				dialog_ = new CVstEffectWnd(reinterpret_cast<vst::plugin*>(mac()), this);
				// newwin->_pActive = &isguiopen[tmac];
				dialog_->LoadFrame(IDR_VSTFRAME, 
//					WS_OVERLAPPEDWINDOW,
					WS_POPUPWINDOW | WS_CAPTION,
					view()->child_view()->pParentFrame);
				std::ostringstream winname;
				winname << std::hex << std::setw(2)
					<< view()->song()->FindBusFromIndex(mac()->_macIndex)
						<< " : " << mac()->_editName;
						dialog_->SetTitleText(winname.str().c_str());
							// C_Tuner.dll crashes if asking size before opening.
//							newwin->ResizeWindow(0);
				dialog_->ShowWindow(SW_SHOWNORMAL);
				dialog_->PostOpenWnd();
//				CenterWindowOnPoint(m_pWndMac[tmac], point);
			}
		}

	}  // namespace host
}  // namespace psycle
