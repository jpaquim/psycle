#include "WireGui.hpp"
#include "MachineGui.hpp"
#include "MachineView.hpp"
#include "WireDlg.hpp"

namespace psycle {
	namespace host {

		WireGui::WireGui(MachineView* view)	:
			TestCanvas::Line(view->root()),
			view_(view),
			fromGUI_(0),
			toGUI_(0),
			start_(0),
			dragging_(0),
			wire_dlg_(0)
		{
			TestCanvas::Line::Points m_points;
			m_points.push_back(std::pair<double,double>(0, 0));
			m_points.push_back(std::pair<double,double>(100, 100));
			SetPoints( m_points);
		}

		WireGui::~WireGui()
		{
		}

		void WireGui::setGuiConnectors(MachineGui* from,
									   MachineGui* to,
									   MachineGui* start)  {
			fromGUI_ = from;
			toGUI_ = to;
			start_ = start;
		}

		void WireGui::dragging_start(int pickpoint)
		{
			dragging_ = true;
			drag_picker_ = pickpoint;
			GetFocus();
		}

		void WireGui::dragging(double x, double y) {
			TestCanvas::Line::Points points(2);
			if ( drag_picker_ == 1 ) {
				points[0] = PointAt(0);
				points[1] = std::pair<double,double>(x, y);
			} else
			if ( drag_picker_ == 0 ) {
				points[1] = PointAt(1);
				points[0] = std::pair<double,double>(x, y);
			}
			SetPoints(points);
		}

		void WireGui::dragging_stop()
		{
			dragging_ = false;
		}

		void WireGui::BeforeWireDeletion()
		{
			wire_dlg_ = 0;
		}

		bool WireGui::OnEvent(TestCanvas::Event* ev)
		{
			switch(ev->type) {
				case TestCanvas::Event::BUTTON_2PRESS:
					if (!wire_dlg_) {			
						wire_dlg_ = new CWireDlg(fromGUI_->view()->child_view(), this);
						wire_dlg_->this_index = 0;
						wire_dlg_->wireIndex = 0;
						wire_dlg_->isrcMac = fromGUI_->mac()->_macIndex;
						wire_dlg_->_pSrcMachine = fromGUI_->mac();
						wire_dlg_->_pDstMachine = toGUI_->mac();
						wire_dlg_->Create();
						//pParentMain->CenterWindowOnPoint(wdlg, point);
						wire_dlg_->ShowWindow(SW_SHOW);
					}
				break;
				case TestCanvas::Event::MOTION_NOTIFY:
					if(dragging_) {
						dragging(ev->x, ev->y);
					}
				break;
				case TestCanvas::Event::BUTTON_RELEASE:
					if(dragging_) {
 				       dragging_stop();
					   view_->OnRewireEnd(this, ev->x, ev->y, drag_picker_);
					}
				break;
				default:
					;
			}
			return true;
		}

		void WireGui::UpdatePosition()
		{
			TestCanvas::Group* parentGroup = parent();
			double xp1, yp1, xp2, yp2;
			parentGroup->GetBounds(xp1, yp1, xp2, yp2);
			if ( fromGUI_ ) {
				double x1, y1, x2, y2;
				fromGUI_->GetBounds( x1, y1, x2, y2);
				double midW = (x2 - x1) / 2;
				double midH = (y2 - y1) / 2;
				TestCanvas::Group* fromParent = fromGUI_->parent();
				double x3, y3, x4, y4;
				fromParent->GetBounds(x3,y3,x4,y4);
				double x = x1+x3 - xp1;
				double y = y1+y3 - yp1;

				TestCanvas::Line::Points points(2);
				points[0] = std::pair<double,double>(x + midW, y + midH);
				points[1] = PointAt(1);
				SetPoints(points);
			}
			if ( toGUI_ ) {
				double x1, y1, x2, y2;
				toGUI_->GetBounds( x1, y1, x2, y2);
				double midW = (x2 - x1) / 2;
				double midH = (y2 - y1) / 2;
				TestCanvas::Group* toParent = toGUI_->parent();
				double x3, y3, x4, y4;
				toParent->GetBounds(x3,y3,x4,y4);
				double x = x1+x3 - xp1;
				double y = y1+y3 - yp1;
				TestCanvas::Line::Points points(2);
				points[0] = PointAt(0);
				points[1] = std::pair<double,double>(x + midW, y + midH);
				SetPoints(points);
			}
		}

	}  // namespace host
}  // namespace psycle
