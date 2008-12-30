#include "MachineGui.hpp"
#include "Machine.hpp"
#include "MachineView.hpp"
#include "MacProp.hpp"
#include "WireGui.hpp"
#include "MainFrm.hpp"

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

		void MachineGui::UpdateVU()
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
					DoMacPropDialog();
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

		void MachineGui::DoMacPropDialog()
		{
			int propMac = mac()->_macIndex;
			CMacProp dlg(this);
			//dlg.m_view=this;
			dlg.pMachine = Global::_pSong->_pMachine[propMac];
			dlg.pSong = view()->song();
			dlg.thisMac = propMac;
			if(dlg.DoModal() == IDOK)
			{
				sprintf(dlg.pMachine->_editName, dlg.txt);
				view()->main()->StatusBarText(dlg.txt);
				view()->main()->UpdateEnvInfo();
				view()->main()->UpdateComboGen();
				if (view()->main()->pGearRackDialog)
				{
					view()->main()->RedrawGearRackList();
				}	
				UpdateText();
			}
			if(dlg.deleted)
			{
				view()->song()->DestroyMachine(propMac);
				view()->main()->UpdateEnvInfo();
				view()->main()->UpdateComboGen();
				if (view()->main()->pGearRackDialog)
				{
					view()->main()->RedrawGearRackList();
				}
				view()->SetDeleteMachineGui(this);
			}
			view()->child_view()->Invalidate(1);
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
	
		void MachineGui::OnMove()
		{
			std::vector< std::pair<WireGui*, int> >::iterator it;
			it = wire_uis_.begin();
			for ( ; it != wire_uis_.end(); ++it ) {
				(*it).first->UpdatePosition();
			}
		}

	}  // namespace host
}  // namespace psycle
