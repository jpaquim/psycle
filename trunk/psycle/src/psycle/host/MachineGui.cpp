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
			mac_(mac),
			pixbuf_(this),
			mute_pixbuf_(this),
			pan_pixbuf_(this),
			vu_bg_pixbuf_(this),
			vu_peak_pixbuf_(this),
			vu_led_pixbuf_(this),
			text_(this)
			
			//rect_(this, 0, 0, 200, 100)
		{					
			UpdateText();
		}

		MachineGui::~MachineGui()
		{
		}

		MachineView* MachineGui::view()
		{
			return view_;
		}

		bool MachineGui::OnEvent(TestCanvas::Event* ev)
		{
			if ( ev->type == TestCanvas::Event::BUTTON_PRESS ) {
				if ( ev->button == 1 ) {
					TestMute(ev->x, ev->y);
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

		bool MachineGui::TestMute(double x, double y)
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
		}

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
			mac_->_mute = mute;
			mute_pixbuf_.SetVisible(mute);
			solo_pixbuf_.SetVisible(false);
		}

		void MachineGui::SetSolo(bool mute)
		{
			mute_pixbuf_.SetVisible(false);
			solo_pixbuf_.SetVisible(true);
		}

		void MachineGui::AttachWire(WireGui* gui, int point) 
		{
			wire_uis_.push_back(std::pair<WireGui*,int>(gui, point));
		}

		void MachineGui::UpdatePan()
		{
			if ( mac_->_mode == MACHMODE_GENERATOR ) {
				int panning = mac_->_panning*MachineCoords_.dGeneratorPan.width;
				panning /= 128;
				pan_pixbuf_.SetXY(panning+MachineCoords_.dGeneratorPan.x, 
							  MachineCoords_.dGeneratorPan.y);
			} else
			if ( mac_->_mode == MACHMODE_FX ) {
				int panning = mac_->_panning*MachineCoords_.dEffectPan.width;
				panning /= 128;
				pan_pixbuf_.SetXY(panning+MachineCoords_.dEffectPan.x, 
							  MachineCoords_.dEffectPan.y);
			}
		}

		void MachineGui::UpdateText()
		{
			char name[sizeof(mac_->_editName)+6+3];
			sprintf(name,"%.2X:%s",mac_->_macIndex,mac_->_editName);
			text_.SetText(name);
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
			mac_->_volumeMaxCounterLife--;
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

			}
		}

		void MachineGui::SetSkin(const SMachineCoords& MachineCoords,
								 CBitmap* machineskin,
								 CBitmap* machineskinmask,
								 CBitmap* machinebkg,
								 HBITMAP hbmMachineSkin,
								 HBITMAP hbmMachineBkg,
								 HBITMAP hbmMachineDial,
								 const CFont& generator_font,
								 COLORREF generator_font_color,
								 const CFont& effect_font,
								 COLORREF effect_font_color)
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
			switch (mac_->_mode) {
				case MACHMODE_GENERATOR: 
					pixbuf_.SetSize(MachineCoords.sGenerator.width, 
									MachineCoords.sGenerator.height);
									pixbuf_.SetSource(MachineCoords.sGenerator.x, 
									MachineCoords.sGenerator.y);
					mute_pixbuf_.SetXY(MachineCoords.dGeneratorMute.x, 
									   MachineCoords.dGeneratorMute.y);
					mute_pixbuf_.SetSize(MachineCoords.sGeneratorMute.width, 
									     MachineCoords.sGeneratorMute.height);
					mute_pixbuf_.SetSource(MachineCoords.sGeneratorMute.x, 
										   MachineCoords.sGeneratorMute.y);
					mute_pixbuf_.SetVisible(false);
					solo_pixbuf_.SetXY(MachineCoords.dGeneratorSolo.x,
									   MachineCoords.dGeneratorSolo.y);
					solo_pixbuf_.SetSize(MachineCoords.sGeneratorSolo.width, 
										 MachineCoords.sGeneratorSolo.height);
					solo_pixbuf_.SetSource(MachineCoords.sGeneratorSolo.x, 
										   MachineCoords.sGeneratorSolo.y);
					solo_pixbuf_.SetVisible(false);
					UpdatePan();
					pan_pixbuf_.SetSize(MachineCoords.sGeneratorPan.width, 
										MachineCoords.sGeneratorPan.height);
					pan_pixbuf_.SetSource(MachineCoords.sGeneratorPan.x, 
										  MachineCoords.sGeneratorPan.y);
					text_.SetXY(MachineCoords.dGeneratorName.x,
							    MachineCoords.dGeneratorName.y);			
					text_.SetFont(generator_font);
					text_.SetColor(GetRValue(generator_font_color) / 255.0,
								   GetGValue(generator_font_color) / 255.0,
								   GetBValue(generator_font_color) / 255.0,
								   1.0);
				break;
				case MACHMODE_FX:
					pixbuf_.SetSize(MachineCoords.sEffect.width, 
									MachineCoords.sEffect.height);
									pixbuf_.SetSource(MachineCoords.sEffect.x, 
									MachineCoords.sEffect.y);
					UpdatePan();
					pan_pixbuf_.SetSize(MachineCoords.sEffectPan.width, 
										MachineCoords.sEffectPan.height);
					pan_pixbuf_.SetSource(MachineCoords.sEffectPan.x, 
										  MachineCoords.sEffectPan.y);
					text_.SetXY(MachineCoords.dEffectName.x,
							    MachineCoords.dEffectName.y);
					text_.SetFont(effect_font);
					text_.SetColor(GetRValue(effect_font_color) / 255.0,
								   GetGValue(effect_font_color) / 255.0,
								   GetBValue(effect_font_color) / 255.0,
								   1.0);
				break;
				case MACHMODE_MASTER:
					pixbuf_.SetSize(MachineCoords.sMaster.width, 
									MachineCoords.sMaster.height);
									pixbuf_.SetSource(MachineCoords.sMaster.x, 
									MachineCoords.sMaster.y);					
					text_.SetVisible(false);
			
				break;
				default:
				;
			}
		}
		

	}  // namespace host
}  // namespace psycle
