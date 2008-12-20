#include "MasterGui.hpp"
#include "Song.hpp"
#include "MasterDlg.hpp"
#include "MachineView.hpp"

namespace psycle {
	namespace host {

		MasterGui::MasterGui(class MachineView* view,
							 class Machine* mac)
			: MachineGui(view, mac),
			  dialog_(0),
			  pixbuf_(this),
			  mute_pixbuf_(this),
			  pan_pixbuf_(this),
			  vu_bg_pixbuf_(this),
			  vu_peak_pixbuf_(this),
			  vu_led_pixbuf_(this),
			  text_(this)
		{
		}

		MasterGui::~MasterGui()
		{
			if ( dialog_ )
				delete dialog_;
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
			machineskin_ = machineskin;
			machineskinmask_ = machineskinmask;
			machinebkg_ = machinebkg;
			hbmMachineSkin_ = hbmMachineSkin;
			hbmMachineBkg_ = hbmMachineBkg;
			hbmMachineDial = hbmMachineDial;
			MachineCoords_ = MachineCoords;
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

		bool MasterGui::OnEvent(TestCanvas::Event* ev)
		{
			MachineGui::OnEvent(ev);
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				ShowDialog();		
			}
			return true;
		}

		void MasterGui::ShowDialog()
		{
			if ( !dialog_ ) {
				dialog_ = new CMasterDlg(view()->child_view());
				dialog_->_pMachine = (Master*)mac();
				for (int i=0;i<MAX_CONNECTIONS; i++)
				{
					if (mac()->_inputCon[i])
					{
						if (view()->song()->_pMachine[mac()->_inputMachines[i]])
						{
							strcpy(dialog_->macname[i],view()->song()->_pMachine[mac()->_inputMachines[i]]->_editName);
						}
					}
				}
				dialog_->Create();
//				CPoint point(-1,-1);
//				CenterWindowOnPoint(dialog_, point);
				dialog_->ShowWindow(SW_SHOW);
			} else {
				dialog_->ShowWindow(SW_SHOW);
			}
		}

	}  // namespace host
}  // namespace psycle
