#include "machinegui.hpp"
#include "machine.hpp"
#include "machineview.hpp"
#include "wiregui.hpp"

namespace psycle {
	namespace host {

		MachineGui::MachineGui(MachineView* view,
							   Machine* mac) :
			dragging_(false),
			TestCanvas::Group(view->root(), mac->_x, mac->_y),
			mac_(mac)
			//rect_(this, 0, 0, 200, 100)
		{
			Add(&pixbuf_);
			Add(&mute_pixbuf_);
			Add(&solo_pixbuf_);
		}

		MachineGui::~MachineGui()
		{
		}

		bool MachineGui::OnEvent(TestCanvas::Event* ev)
		{
			if ( ev->type == TestCanvas::Event::BUTTON_PRESS ) {
				TestMute(ev->x, ev->y);
				dragging_start(ev->x, ev->y);
			} else
			if ( ev->type == TestCanvas::Event::BUTTON_MOTION ) {
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
		
		void MachineGui::OnMove()
		{
			std::vector< std::pair<WireGui*, int> >::iterator it;
			it = wire_uis_.begin();
			for ( ; it != wire_uis_.end(); ++it ) {
				(*it).first->UpdatePosition();
			}
		}

		void MachineGui::SetSkin(const SMachineCoords& MachineCoords,
								 CBitmap* machineskin,
								 CBitmap* machineskinmask,
								 CBitmap* machinebkg,
								 HBITMAP hbmMachineSkin,
								 HBITMAP hbmMachineBkg,
								 HBITMAP hbmMachineDial)
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
					
				break;
				case MACHMODE_FX:
					pixbuf_.SetSize(MachineCoords.sEffect.width, 
									MachineCoords.sEffect.height);
									pixbuf_.SetSource(MachineCoords.sEffect.x, 
									MachineCoords.sEffect.y);
				break;
				case MACHMODE_MASTER:
					pixbuf_.SetSize(MachineCoords.sMaster.width, 
									MachineCoords.sMaster.height);
									pixbuf_.SetSource(MachineCoords.sMaster.x, 
									MachineCoords.sMaster.y);
			
				break;
				default:
				;
			}
		}
	}
}