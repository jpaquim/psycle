#include "wiregui.hpp"
#include "machinegui.hpp"

namespace psycle {
	namespace host {

		WireGui::WireGui()	:
			fromGUI_(0),
			toGUI_(0),
			start_(0)
		{
			TestCanvas::Line::Points m_points;
			m_points.push_back(std::pair<double,double>(0, 0));
			m_points.push_back(std::pair<double,double>(100, 100));
			SetPoints( m_points);
		}

		WireGui::~WireGui()
		{
		}

		MachineGui* WireGui::fromGUI() const
		{
			return fromGUI_;
		}

		MachineGui* WireGui::toGUI() const
		{
			return toGUI_;
		}

		MachineGui* WireGui::start() const
		{
		  return start_;
		}

		void WireGui::setGuiConnectors(MachineGui* from,
									   MachineGui* to,
									   MachineGui* start)  {
			fromGUI_ = from;
			toGUI_ = to;
			start_ = start;
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
