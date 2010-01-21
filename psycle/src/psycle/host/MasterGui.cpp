#include "MasterGui.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/machine.h>
#include <psycle/core/song.h>
using namespace psycle::core;
#else
#include "Machine.hpp"
#include "Song.hpp"
#endif

#include "MasterDlg.hpp"
#include "MachineView.hpp"

namespace psycle {
	namespace host {

		MasterGui::MasterGui(MachineView* view, Machine* mac)
			: MachineGui(view, mac),
			  dialog_(0),
			  pixbuf_(this),
			  mute_pixbuf_(this),
			  pan_pixbuf_(this),
			  vu_bg_pixbuf_(this),
			  vu_peak_pixbuf_(this),
			  vu_led_pixbuf_(this),
			  text_(this) {
		}

		MasterGui::~MasterGui() {
			if (dialog_)
				delete dialog_;
		}

		bool MasterGui::OnEvent(PsycleCanvas::Event* ev) {
			if ( ev->type == PsycleCanvas::Event::BUTTON_RELEASE ) {
				if (ev->button == 3) {
					// do not allow for master to be deleted etc..
					return true;
				}
			}
			// else fallback to machinegui events
			return MachineGui::OnEvent(ev);
		}

		void MasterGui::SetSkin(const SMachineCoords& MachineCoords,
								CBitmap* machineskin,
								CBitmap* machineskinmask,
								CBitmap* machinebkg,
								HBITMAP hbmMachineSkin,
								HBITMAP hbmMachineBkg,
								HBITMAP hbmMachineDial,
								const CFont& font,
								COLORREF font_color)
		{			
			pixbuf_.SetImage(machineskin);
			mute_pixbuf_.SetImage(machineskin);
			solo_pixbuf_.SetImage(machineskin);
			vu_bg_pixbuf_.SetImage(machineskin);
			vu_peak_pixbuf_.SetImage(machineskin);
			vu_led_pixbuf_.SetImage(machineskin);
			pixbuf_.SetSize(MachineCoords.sMaster.width, 
							MachineCoords.sMaster.height);
			pixbuf_.SetSource(MachineCoords.sMaster.x, 
							  MachineCoords.sMaster.y);					
			text_.SetVisible(false);
		}

		void MasterGui::ShowDialog(double x, double y)
		{
			if (!dialog_) {
				dialog_ = new CMasterDlg((Master*)mac(), view()->child_view());
				for (int i=0;i<MAX_CONNECTIONS; ++i) {
					if (mac()->_inputCon[i]) {
						if (view()->song()->machine(mac()->_inputMachines[i])) {
							strcpy(dialog_->macname[i],view()->song()->machine(mac()->_inputMachines[i])->GetEditName().c_str());
						}
					}
				}
				dialog_->CenterWindowOnPoint(x, y);
				dialog_->ShowWindow(SW_SHOW);
			} else {
				dialog_->ShowWindow(SW_SHOW);
				dialog_->CenterWindowOnPoint(x, y);
			}
		}
		void MasterGui::UpdateVU(CDC* devc)
		{
			MachineGui::UpdateVU(devc);
			if ( dialog_ ) {
				dialog_->UpdateUI();
			}
		}
		int MasterGui::preferred_width() const
		{
			return pixbuf_.width();
		}

		int MasterGui::preferred_height() const
		{
			 return pixbuf_.height();
		}

	}  // namespace host
}  // namespace psycle
