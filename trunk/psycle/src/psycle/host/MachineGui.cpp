#include "MachineGui.hpp"
#include "Machine.hpp"
#include "MachineView.hpp"
#include "WireGui.hpp"

namespace psycle {
	namespace host {

		MachineGui::MachineGui(MachineView* view,
							   Machine* mac) :
			dragging_(false),
			TestCanvas::Group(view->root(), mac->_x, mac->_y),
			view_(view),
			mac_(mac)						
		{		
			assert(mac_);
		}

		MachineGui::~MachineGui()
		{
		}

		MachineView* MachineGui::view()
		{
			return view_;
		}

		void MachineGui::SetSelected(bool on)
		{
		}

		bool MachineGui::IsSelected() const
		{
			return false;
		}

		bool MachineGui::OnEvent(TestCanvas::Event* ev)
		{
			if ( ev->type == TestCanvas::Event::BUTTON_PRESS ) {
				if ( ev->button == 1 ) {
//					TestMute(ev->x, ev->y);
					dragging_start(ev->x, ev->y);
				} else
				if ( ev->button == 3 ) {
					view_->OnNewConnection(this);
				}
			} else
			if ( ev->type == TestCanvas::Event::MOTION_NOTIFY ) {
			  if (dragging_)
				dragging(ev->x, ev->y);
			} else
			if ( ev->type == TestCanvas::Event::BUTTON_RELEASE ) {
				dragging_stop();
			}
			return true;
		}

		bool MachineGui::InRect(double x, double y, double x1, double y1, double x2,
			double y2 ) const {
			if ( x1 < x2 ) {
				if ( y1 < y2 )
					return ( x >= x1 && x < x2 && y >= y1 && y < y2 ) ? 1 : 0;
				else 
					return ( x >= x1 && x < x2 && y >= y2 && y < y1 ) ? 1 : 0;
			} else {
				if ( y1 < y2 )
					return ( x >= x2 && x < x1 && y >= y1 && y < y2 ) ? 1 : 0;
				else 
					return ( x >= x2 && x < x1 && y >= y2 && y < y1 ) ? 1 : 0;
			}
		}

		/*bool MachineGui::TestMute(double x, double y)
		{
			if(InRect(x,
					  y,
					  MachineCoords_.dGeneratorMute.x,
					  MachineCoords_.dGeneratorMute.y,
					  MachineCoords_.dGeneratorMute.x + 
					  MachineCoords_.sGeneratorMute.width,
					  MachineCoords_.dGeneratorMute.y + 
					  MachineCoords_.sGeneratorMute.height)) {
			  SetMute(!mac_->_mute);
			  QueueDraw();
			  return true;
			} else {
				return false;
			}
		}

		bool MachineGui::TestSolo(double x, double y)
		{
			if(InRect(x,
					  y,
					  MachineCoords_.dGeneratorSolo.x,
					  MachineCoords_.dGeneratorSolo.y,
					  MachineCoords_.dGeneratorSolo.x + 
					  MachineCoords_.sGeneratorSolo.width,
					  MachineCoords_.dGeneratorSolo.y + 
					  MachineCoords_.sGeneratorSolo.height)) {
			  SetMute(!mac_->_mute);
			  QueueDraw();
			  return true;
			} else {
				return false;
			}
		}*/

		void MachineGui::dragging_start(double x, double y)
		{
			dragging_ = true;
			dragging_x_ = x;
			dragging_y_ = y;
		}

		void MachineGui::dragging(double x, double y)
		{
			double delta_x = x - dragging_x_;
			double delta_y = y - dragging_y_;
			// limit to greater/equal 0,0
			if (absx() + delta_x < 0)
				delta_x = -this->x();
			if (absy() + delta_y < 0)
				delta_y = -this->y();	
			Move(delta_x, delta_y);
			OnMove(); 
		}

		void MachineGui::dragging_stop()
		{
			dragging_ = false;
		}

		void MachineGui::SetMute(bool mute)
		{			
/*			mac_->_mute = mute;
			mute_pixbuf_.SetVisible(mute);
			solo_pixbuf_.SetVisible(false);*/
		}

		void MachineGui::SetSolo(bool mute)
		{
/*			mute_pixbuf_.SetVisible(false);
			solo_pixbuf_.SetVisible(true);*/
		}

		void MachineGui::AttachWire(WireGui* gui, int point) 
		{
			wire_uis_.push_back(std::pair<WireGui*,int>(gui, point));
		}

		void MachineGui::DetachWire(WireGui* wire_gui)
		{
			std::vector<std::pair<WireGui*,int> >::iterator it = wire_uis_.begin();
			for ( ; it != wire_uis_.end(); ++it ) {
				if ((*it).first == wire_gui) {
					wire_uis_.erase(it);
					break;
				}
			}
		}
	
		void MachineGui::OnMove()
		{
			std::vector< std::pair<WireGui*, int> >::iterator it;
			it = wire_uis_.begin();
			for ( ; it != wire_uis_.end(); ++it ) {
				(*it).first->UpdatePosition();
			}
		}

		void MachineGui::UpdateVU()
		{
			mac()->_volumeMaxCounterLife--;
			if ((mac()->_volumeDisplay > mac()->_volumeMaxDisplay)
				||	(mac()->_volumeMaxCounterLife <= 0)) {
				mac()->_volumeMaxDisplay = mac()->_volumeDisplay-1;
				mac()->_volumeMaxCounterLife = 60;
			}
/*			mac_->_volumeMaxCounterLife--;
			if ((mac_->_volumeDisplay > mac_->_volumeMaxDisplay)
				||	(mac_->_volumeMaxCounterLife <= 0)) {
				mac_->_volumeMaxDisplay = mac_->_volumeDisplay-1;
				mac_->_volumeMaxCounterLife = 60;
			}
			int vol = mac_->_volumeDisplay;
			int max = mac_->_volumeMaxDisplay;
			if ( mac_->_mode == MACHMODE_GENERATOR) {
				vol *= MachineCoords_.dGeneratorVu.width;
				vol /= 96;
				max *= MachineCoords_.dGeneratorVu.width;
				max /= 96;
				vu_bg_pixbuf_.SetXY(vol+MachineCoords_.dGeneratorVu.x,
									MachineCoords_.dGeneratorVu.y);
				vu_bg_pixbuf_.SetSize(MachineCoords_.dGeneratorVu.width-vol,
								  MachineCoords_.sGeneratorVu0.height);
				vu_bg_pixbuf_.SetSource(MachineCoords_.sGenerator.x+MachineCoords_.dGeneratorVu.x+vol,
										MachineCoords_.sGenerator.y+MachineCoords_.dGeneratorVu.y);
				vu_bg_pixbuf_.QueueDraw();
				vu_peak_pixbuf_.SetXY(max + MachineCoords_.dGeneratorVu.x, 
									  MachineCoords_.dGeneratorVu.y);
				vu_peak_pixbuf_.SetSize(MachineCoords_.sGeneratorVuPeak.width, 
										MachineCoords_.sGeneratorVuPeak.height);
				vu_peak_pixbuf_.SetSource(MachineCoords_.sGeneratorVuPeak.x, 
										  MachineCoords_.sGeneratorVuPeak.y);
				vu_peak_pixbuf_.QueueDraw();
				vu_led_pixbuf_.SetXY(MachineCoords_.dGeneratorVu.x, 
									 MachineCoords_.dGeneratorVu.y);
				vu_led_pixbuf_.SetSize(vol,
									MachineCoords_.sGeneratorVu0.height);
				vu_led_pixbuf_.SetSource(MachineCoords_.sGeneratorVu0.x, 
									  MachineCoords_.sGeneratorVu0.y);
				vu_led_pixbuf_.QueueDraw();
			} else
			if ( mac_->_mode == MACHMODE_FX) {
				vol *= MachineCoords_.dEffectVu.width;
				vol /= 96;
				max *= MachineCoords_.dEffectVu.width;
				max /= 96;
				vu_bg_pixbuf_.SetXY(vol+MachineCoords_.dEffectVu.x,
									MachineCoords_.dEffectVu.y);
				vu_bg_pixbuf_.SetSize(MachineCoords_.dEffectVu.width-vol,
								  MachineCoords_.sEffectVu0.height);
				vu_bg_pixbuf_.SetSource(MachineCoords_.sEffect.x+MachineCoords_.dGeneratorVu.x+vol,
										MachineCoords_.sEffect.y+MachineCoords_.dGeneratorVu.y);
				vu_bg_pixbuf_.QueueDraw();
				vu_peak_pixbuf_.SetXY(max + MachineCoords_.dEffectVu.x, 
									  MachineCoords_.dEffectVu.y);
				vu_peak_pixbuf_.SetSize(MachineCoords_.sEffectVuPeak.width, 
										MachineCoords_.sEffectVuPeak.height);
				vu_peak_pixbuf_.SetSource(MachineCoords_.sEffectVuPeak.x, 
										  MachineCoords_.sEffectVuPeak.y);
				vu_peak_pixbuf_.QueueDraw();
				vu_led_pixbuf_.SetXY(MachineCoords_.dEffectVu.x, 
									 MachineCoords_.dEffectVu.y);
				vu_led_pixbuf_.SetSize(vol,
									MachineCoords_.sEffectVu0.height);
				vu_led_pixbuf_.SetSource(MachineCoords_.sEffectVu0.x, 
									  MachineCoords_.sEffectVu0.y);
				vu_led_pixbuf_.QueueDraw();

			}*/
		}

		void MachineGui::BeforeDeleteDlg()
		{
		}

		void MachineGui::SetSkin(const SMachineCoords& MachineCoords,
								 CBitmap* machineskin,
								 CBitmap* machineskinmask,
								 CBitmap* machinebkg,
								 HBITMAP hbmMachineSkin,
								 HBITMAP hbmMachineBkg,
								 HBITMAP hbmMachineDial,
								 const CFont& font,
								 COLORREF font_color)
		{
		}
		

	}  // namespace host
}  // namespace psycle
