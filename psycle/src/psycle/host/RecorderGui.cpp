#include "RecorderGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"
#include "WaveInMacDlg.hpp"
#include "ChildView.hpp"

namespace psycle {
	namespace host {

		RecorderGui::RecorderGui(class MachineView* view,
							     class Machine* mac)
			: MachineGui(view, mac),
			  dialog_(0)
		{
		}

		RecorderGui::~RecorderGui()
		{		
		}

		bool RecorderGui::OnEvent(TestCanvas::Event* ev)
		{
			MachineGui::OnEvent(ev);
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog();		
			}
			return true;
		}

		void RecorderGui::BeforeDeleteDlg()
		{
			dialog_ = 0;
		}

		void RecorderGui::ShowDialog()
		{
			if ( !dialog_ ) {
				dialog_ = new CWaveInMacDlg(view()->child_view(), this);
				dialog_->pRecorder = (AudioRecorder*)mac();
				dialog_->Create();
				//CenterWindowOnPoint(m_wndView.WaveInMachineDialog, point);
				//dialog_ = new CFrameMachine(mac()->_macIndex, this);
				//CenterWindowOnPoint(m_pWndMac[tmac], point);
			}
		}

	}  // namespace host
}  // namespace psycle
