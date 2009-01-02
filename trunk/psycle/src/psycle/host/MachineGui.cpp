#include "MachineGui.hpp"
#include "Machine.hpp"
#include "MachineView.hpp"
#include "WireGui.hpp"
#include "MainFrm.hpp"

#ifdef _MSC_VER
#undef min
#undef max
#endif


namespace psycle {
	namespace host {

		MachineGui::MachineGui(MachineView* view,
							   Machine* mac) :
			TestCanvas::Group(view->root(), mac->_x, mac->_y),
			view_(view),
			mac_(mac),
			dragging_(false)
		{		
			assert(mac_);
		}

		MachineGui::~MachineGui()
		{
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

		void MachineGui::RemoveWires()
		{
			std::vector<std::pair<WireGui*,int> >::iterator it = wire_uis_.begin();
			for ( ; it != wire_uis_.end(); ++it ) {
				WireGui* wire_ui = (*it).first;
				if (wire_ui->toGUI() && (wire_ui->toGUI() != this ) )
					wire_ui->toGUI()->DetachWire(wire_ui);
				if (wire_ui->fromGUI() && (wire_ui->fromGUI() != this ))
					wire_ui->fromGUI()->DetachWire(wire_ui);
				wire_ui->set_manage(false);
				wire_ui->SetGuiConnectors(0,0,0);
				delete wire_ui;
			}
			wire_uis_.clear();
		}		

		void MachineGui::UpdateVU(CDC* devc)
		{
			mac()->_volumeMaxCounterLife--;
			if ((mac()->_volumeDisplay > mac()->_volumeMaxDisplay)
				||	(mac()->_volumeMaxCounterLife <= 0)) {
					mac()->_volumeMaxDisplay = mac()->_volumeDisplay-1;
					mac()->_volumeMaxCounterLife = 60;
			}
		}

		void MachineGui::UpdateText()
		{
		}

		void MachineGui::BeforeDeleteDlg()
		{
		}

		bool MachineGui::OnEvent(TestCanvas::Event* ev)
		{
			if ( ev->type == TestCanvas::Event::BUTTON_PRESS ) {
				if ( ev->button == 1 ) {
					dragging_start(ev->x, ev->y);
				} else
				if ( ev->button == 3 ) {
					new_con_ = false;
					dragging_x_ = ev->x;
					dragging_y_ = ev->y;
				}
			} else
			if ( ev->type == TestCanvas::Event::MOTION_NOTIFY ) {
				if (dragging_) {
					dragging(ev->x, ev->y);
				} else if (ev->button == 3) {
					if (!new_con_ && (dragging_x_ != ev->x || dragging_y_ != ev->y)) {
						view_->OnNewConnection(this);
						new_con_ = true;
					}
				}
			} else
			if ( ev->type == TestCanvas::Event::BUTTON_RELEASE ) {
				if (ev->button == 3) {
					view()->DoMacPropDialog(mac(), true);
				} else {
					dragging_stop();
				}
			}
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS ) {
				CRect rc;
				view()->parent()->GetWindowRect(rc);
				ShowDialog(rc.left + absx() + ev->x,  rc.top + absy() + ev->y);
			}
			return true;
		}

		
		bool MachineGui::InRect(double x, double y, double x1, double y1, double x2,
			double y2 ) const {
			if ( x1 < x2 ) {
				if ( y1 < y2 )
					return ( x >= x1 && x < x2 && y >= y1 && y < y2 ) ? true : false;
				else 
					return ( x >= x1 && x < x2 && y >= y2 && y < y1 ) ? true : false;
			} else {
				if ( y1 < y2 )
					return ( x >= x2 && x < x1 && y >= y1 && y < y2 ) ? true : false;
				else 
					return ( x >= x2 && x < x1 && y >= y2 && y < y1 ) ? true : false;
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
			double new_x = this->x() + x - dragging_x_;
			double new_y = this->y() + y - dragging_y_;
			// limit to greater/equal 0,0
			new_x = std::max(0.0, new_x);
			new_y = std::max(0.0, new_y);
			// limit to screensize
			double x1, y1, x2, y2;
			GetBounds(x1,y1,x2,y2);
			new_x = std::min(new_x, view()->cw() - (x2 - x1));
			new_y = std::min(new_y, view()->ch() - (y2 - y1));
			SetXY(new_x, new_y);
			OnMove(); 
		}

		void MachineGui::dragging_stop()
		{
			dragging_ = false;
		}
	
		void MachineGui::OnMove()
		{
			std::vector< std::pair<WireGui*, int> >::iterator it;
			it = wire_uis_.begin();
			for ( ; it != wire_uis_.end(); ++it ) {
				(*it).first->UpdatePosition();
			}
			mac()->_x = x();
			mac()->_y = y();
		}

	}  // namespace host
}  // namespace psycle
