#include "MachineGui.hpp"

#include <algorithm>

#include "Machine.hpp"
#include "MainFrm.hpp"
#include "MachineView.hpp"
#include "Song.hpp"
#include "WireGui.hpp"


#ifdef _MSC_VER
#undef min
#undef max
#endif


namespace psycle {
	namespace host {

		MachineGui::MachineGui(MachineView* view,
#ifdef use_psycore
							  psy::core::Machine* mac) :	   
#else
				              Machine* mac) :						
#endif

			PsycleCanvas::Group(view->root(), mac->GetPosX(), mac->GetPosY()),
			view_(view),
			mac_(mac),
			dragging_(false)
		{		
			assert(mac_);
			PsycleCanvas::Line::Points pts;
			pts.push_back(std::pair<double,double>(0, 0));
			pts.push_back(std::pair<double,double>(0, 100));
			sel_line_left_top_1.SetPoints(pts);
			sel_line_left_top_1.SetVisible(false);
			Add(&sel_line_left_top_1);
			sel_line_left_top_2.SetPoints(pts);
			sel_line_left_top_2.SetVisible(false);
			Add(&sel_line_left_top_2);
			sel_line_right_top_1.SetPoints(pts);
			sel_line_right_top_1.SetVisible(false);
			Add(&sel_line_right_top_1);
			sel_line_right_top_2.SetPoints(pts);
			sel_line_right_top_2.SetVisible(false);
			Add(&sel_line_right_top_2);
			sel_line_left_bottom_1.SetPoints(pts);
			sel_line_left_bottom_1.SetVisible(false);
			Add(&sel_line_left_bottom_1);
			sel_line_left_bottom_2.SetPoints(pts);
			sel_line_left_bottom_2.SetVisible(false);
			Add(&sel_line_left_bottom_2);
			sel_line_right_bottom_1.SetPoints(pts);
			sel_line_right_bottom_1.SetVisible(false);
			Add(&sel_line_right_bottom_1);
			sel_line_right_bottom_2.SetPoints(pts);
			sel_line_right_bottom_2.SetVisible(false);
			Add(&sel_line_right_bottom_2);
		}

		MachineGui::~MachineGui()
		{
		}

		void MachineGui::AttachWire(WireGui* gui) 
		{
			wire_uis_.push_back(gui);
		}

		void MachineGui::DetachWire(WireGui* wire_gui)
		{
			std::vector<WireGui*>::iterator it;
			it = std::find(wire_uis_.begin(), wire_uis_.end(), wire_gui);
			assert(it != wire_uis_.end());
			wire_uis_.erase(it);			
		}

		void MachineGui::RemoveWires()
		{
			std::vector<WireGui*>::iterator it = wire_uis_.begin();
			for ( ; it != wire_uis_.end(); ++it ) {
				WireGui* wire_ui = (*it);
				if (wire_ui->toGUI() && (wire_ui->toGUI() != this))
					wire_ui->toGUI()->DetachWire(wire_ui);
				if (wire_ui->fromGUI() && (wire_ui->fromGUI() != this))
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

		bool MachineGui::OnEvent(PsycleCanvas::Event* ev)
		{
			if (ev->type == PsycleCanvas::Event::BUTTON_PRESS) {
					new_con_ = false;
				//else
				if ( ev->button == 3 ) {
					dragging_x_ = ev->x;
					dragging_y_ = ev->y;
				}
			} else
			if (ev->type == PsycleCanvas::Event::MOTION_NOTIFY) {
				if (dragging_) {
					DoDragging(ev->x, ev->y);
				} else
				if (ev->button == 3 || (ev->button == 1 && (ev->shift & MK_SHIFT))) {
					if (!new_con_ && (dragging_x_ != ev->x || dragging_y_ != ev->y)) {
						view_->OnNewConnection(this);
						new_con_ = true;
					}
				} else
				if ( ev->button == 1) {
					StartDragging(ev->x, ev->y);
				}
			} else
			if (ev->type == PsycleCanvas::Event::BUTTON_RELEASE) {
				if (ev->button == 3) {
					view()->DoMacPropDialog(mac(), true);
				} else {
					StopDragging();
				}
			}
			if (ev->type == PsycleCanvas::Event::BUTTON_2PRESS) {
				CRect rc;
				view()->parent()->GetWindowRect(rc);
				ShowDialog(rc.left + absx() + ev->x,  rc.top + absy() + ev->y);
			}
			return true;
		}
		
		bool MachineGui::InRect(double x,
								double y,
								double x1,
								double y1,
								double x2,
								double y2) const {
			if (x1 < x2) {
				if (y1 < y2)
					return (x >= x1 && x < x2 && y >= y1 && y < y2) ? true : false;
				else 
					return ( x >= x1 && x < x2 && y >= y2 && y < y1) ? true : false;
			} else {
				if (y1 < y2)
					return (x >= x2 && x < x1 && y >= y1 && y < y2) ? true : false;
				else 
					return (x >= x2 && x < x1 && y >= y2 && y < y1) ? true : false;
			}
		}

		void MachineGui::StartDragging(double x, double y)
		{
			dragging_ = true;			
			dragging_x_ = x;
			dragging_y_ = y;
		}

		void MachineGui::DoDragging(double x, double y)
		{
			double new_x = this->x() + x - dragging_x_;
			double new_y = this->y() + y - dragging_y_;
			// limit to greater/equal 0,0
			new_x = std::max(0.0, new_x);
			new_y = std::max(0.0, new_y);
			// limit to screensize
			double x1, y1, x2, y2;
			GetBounds(x1,y1,x2,y2);
			new_x = std::min(new_x, static_cast<double>(view()->cw() - preferred_width()));
			new_y = std::min(new_y, static_cast<double>(view()->ch() - preferred_height()));
			view()->SetSave(true);
			SetXY(new_x, new_y);
			OnMove(); 
			view()->SetSave(false);
			view()->Flush();
			std::ostringstream str;
			str << mac()->GetEditName()
				<< " (" << static_cast<int>(new_x)
				<< "," << static_cast<int>(new_y)
				<< ")";
			view()->WriteStatusBar(str.str());
		}

		void MachineGui::StopDragging()
		{
			dragging_ = false;
		}
	
		void MachineGui::OnMove()
		{
			std::vector<WireGui*>::iterator it = wire_uis_.begin();
			for ( ; it != wire_uis_.end(); ++it ) {
				(*it)->UpdatePosition();
			}
			mac()->SetPosX(x());
			mac()->SetPosY(y());
		}

		void MachineGui::SetSelected(bool on)
		{
			if ( on && !IsSelected() ) {
				view()->song()->seqBus = view()->song()->FindBusFromIndex(mac()->id());
				view()->main()->UpdateComboGen();
				view()->child_view()->Invalidate(1);
			}	
			int size = 5;
			PsycleCanvas::Line::Points pts;
			pts.push_back(std::pair<double,double>(-size, -size));
			pts.push_back(std::pair<double,double>(-size, size));
			sel_line_left_top_1.SetPoints(pts);
			sel_line_left_top_1.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(-size, -size));
			pts.push_back(std::pair<double,double>(size, -size));
			sel_line_left_top_2.SetPoints(pts);
			sel_line_left_top_2.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(preferred_width()-size, -size));
			pts.push_back(std::pair<double,double>(preferred_width()+size, -size));
			sel_line_right_top_1.SetPoints(pts);
			sel_line_right_top_1.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(preferred_width()+size, -size));
			pts.push_back(std::pair<double,double>(preferred_width()+size, +size));
			sel_line_right_top_2.SetPoints(pts);
			sel_line_right_top_2.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(-size, preferred_height() - size));
			pts.push_back(std::pair<double,double>(-size, preferred_height() + size));
			sel_line_left_bottom_1.SetPoints(pts);
			sel_line_left_bottom_1.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(-size, preferred_height() + size));
			pts.push_back(std::pair<double,double>(+size, preferred_height() + size));
			sel_line_left_bottom_2.SetPoints(pts);
			sel_line_left_bottom_2.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(preferred_width() + size, preferred_height() - size));
			pts.push_back(std::pair<double,double>(preferred_width() + size, preferred_height() + size));
			sel_line_right_bottom_1.SetPoints(pts);
			sel_line_right_bottom_1.SetVisible(on);
			pts.clear();
			pts.push_back(std::pair<double,double>(preferred_width() - size, preferred_height() + size));
			pts.push_back(std::pair<double,double>(preferred_width() + size, preferred_height() + size));
			sel_line_right_bottom_2.SetPoints(pts);
			sel_line_right_bottom_2.SetVisible(on);
		}

		bool MachineGui::IsSelected()
		{			
			return view()->song()->seqBus == mac()->id();
		}

		int MachineGui::preferred_width() const
		{
			return 100;
		}

		int MachineGui::preferred_height() const
		{
			 return 20;
		}

	}  // namespace host
}  // namespace psycle
