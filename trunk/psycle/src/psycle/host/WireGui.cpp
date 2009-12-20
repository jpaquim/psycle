#include "WireGui.hpp"
#include <psycle/helpers/math.hpp>
#include "MachineGui.hpp"
#include "MachineView.hpp"
#include "WireDlg.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/machine.h>
using namespace psycle::core;
#else
#include "Machine.hpp"
#endif 

#include "Configuration.hpp"

namespace psycle { namespace host {

using namespace helpers;

		WireGui::WireGui(MachineView* view)	:
			view_(view),
			fromGUI_(0),
			toGUI_(0),
			start_(0),
			wiresrc_(-1),
			wiredest_(-1),
			newcon_(false),
			dragging_(0),
			wire_dlg_(0),
			// the shaded arrow colors will be multiplied by these values to convert them from grayscale to the
  			// polygon color stated in the config.
			deltaColR(((Global::pConfig->mv_polycolour     & 0xFF) / 510.0) + .45),
			deltaColG(((Global::pConfig->mv_polycolour>>8  & 0xFF) / 510.0) + .45),
			deltaColB(((Global::pConfig->mv_polycolour>>16 & 0xFF) / 510.0) + .45),			
			linepen1( PS_SOLID, Global::pConfig->mv_wirewidth+(Global::pConfig->mv_wireaa*2), Global::pConfig->mv_wireaacolour),
			linepen2( PS_SOLID, Global::pConfig->mv_wirewidth+(Global::pConfig->mv_wireaa), Global::pConfig->mv_wireaacolour2),
			linepen3( PS_SOLID, Global::pConfig->mv_wirewidth, Global::pConfig->mv_wirecolour), 
			polyInnardsPen(PS_SOLID, 0, RGB(192 * deltaColR, 192 * deltaColG, 192 * deltaColB))
		{
			rgn_.CreateRectRgn(0, 0, 0, 0);
			PsycleCanvas::Line::Points m_points;
			m_points.push_back(std::pair<double,double>(0, 0));
			m_points.push_back(std::pair<double,double>(100, 100));
			SetPoints( m_points);
			triangle_size_tall = Global::pConfig->mv_triangle_size+((23*Global::pConfig->mv_wirewidth)/16);
			triangle_size_center = triangle_size_tall/2;
			triangle_size_wide = triangle_size_tall/2;
			triangle_size_indent = triangle_size_tall/6;			
		}

		WireGui::~WireGui()
		{			
			if (wire_dlg_) {
				wire_dlg_->DestroyWindow();
			}			
			if (toGUI_) {
				toGUI_->DetachWire(this);
			}
			if (fromGUI_) {
				fromGUI_->DetachWire(this);
			}
		}

		void WireGui::StartDragging(int pickpoint)
		{
			dragging_ = true;
			drag_picker_ = pickpoint;
			GetFocus();
		}

		void WireGui::DoDragging(double x, double y) {
			PsycleCanvas::Line::Points points(2);
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

		void WireGui::StopDragging()
		{
			dragging_ = false;
		}

		void WireGui::BeforeWireDlgDeletion()
		{
			wire_dlg_ = 0;
		}

		void WireGui::RemoveWire()
		{
			set_manage(false); // this causes an unparent
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			fromGUI_->mac()->Disconnect(*toGUI_->mac()) ;
#endif
			wire_dlg_ = 0;
			delete this;			
		}

		void WireGui::Draw(CDC* devc,
					  	   const CRgn& repaint_region,
						   PsycleCanvas::Canvas* widget) {

			if (dragging_) {
				PsycleCanvas::Line::Draw(devc, repaint_region, widget);
			} else {
				int oriX = points().at(0).first;
				int oriY = points().at(0).second;

				int desX = points().at(1).first;
				int desY = points().at(1).second;

				int const f1 = (desX+oriX)/2;
				int const f2 = (desY+oriY)/2;

				if ( !Global::pConfig->mv_wireaa) {
					AmosDraw(devc, oriX, oriY, desX, desY);
				}

				double modX = double(desX-oriX);
				double modY = double(desY-oriY);
				double modT = sqrt(modX*modX+modY*modY);
								
				modX = modX/modT;
				modY = modY/modT;
				double slope = atan2(modY, modX);
				double altslope;
										
				int rtcol = 140+abs(math::rounded(slope*32));

				altslope=slope;
				if(altslope<-1.05)  altslope -= 2 * (altslope + 1.05);
				if(altslope>2.10) altslope -= 2 * (altslope - 2.10);
				int ltcol = 140 + abs(math::rounded((altslope - 2.10) * 32));

				altslope=slope;
				if(altslope>0.79)  altslope -= 2 * (altslope - 0.79);
				if(altslope<-2.36)  altslope -= 2 * (altslope + 2.36);
				int btcol = 240 - abs(math::rounded((altslope-0.79) * 32));

				// brushes for the right side, left side, and bottom of the arrow (when pointed straight up).
				CBrush rtBrush(RGB(rtcol * deltaColR,
								   rtcol * deltaColG,
								   rtcol * deltaColB));
				CBrush ltBrush(RGB(
								   ltcol * deltaColR,
								   ltcol * deltaColG,
								   ltcol * deltaColB));
				CBrush btBrush(RGB(
								   btcol * deltaColR,
								   btcol * deltaColG,
								   btcol * deltaColB));

				CPen *oldpen = 0;
				if (Global::pConfig->mv_wireaa) {				
					oldpen = devc->SelectObject(&linepen1);
				} else {
					oldpen = devc->SelectObject(&linepen3);				
				}
				CBrush *oldbrush = static_cast<CBrush*>(devc->SelectStockObject(NULL_BRUSH));

				CPoint pol[5];
				CPoint fillpoly[7];
								
				pol[0].x = f1 - math::rounded(modX*triangle_size_center);
				pol[0].y = f2 - math::rounded(modY*triangle_size_center);
				pol[1].x = pol[0].x + math::rounded(modX*triangle_size_tall);
				pol[1].y = pol[0].y + math::rounded(modY*triangle_size_tall);
				pol[2].x = pol[0].x - math::rounded(modY*triangle_size_wide);
				pol[2].y = pol[0].y + math::rounded(modX*triangle_size_wide);
				pol[3].x = pol[0].x + math::rounded(modX*triangle_size_indent);
				pol[3].y = pol[0].y + math::rounded(modY*triangle_size_indent);
				pol[4].x = pol[0].x + math::rounded(modY*triangle_size_wide);
				pol[4].y = pol[0].y - math::rounded(modX*triangle_size_wide);
				if (Global::pConfig->mv_wireaa)
				{				
					devc->SelectObject(&linepen1);
					AmosDraw(devc, oriX, oriY, desX, desY);
					devc->Polygon(&pol[1], 4);
					devc->SelectObject(&linepen2);
					AmosDraw(devc, oriX, oriY, desX, desY);
					devc->Polygon(&pol[1], 4);
					devc->SelectObject(&linepen3);
					AmosDraw(devc, oriX, oriY, desX, desY);
					devc->Polygon(&pol[1], 4);
				} else {
					devc->Polygon(&pol[1], 4);
				}
				fillpoly[2].x = pol[0].x + math::rounded(2*modX*triangle_size_indent);
				fillpoly[2].y = pol[0].y + math::rounded(2*modY*triangle_size_indent);
				fillpoly[6].x = fillpoly[2].x;    
				fillpoly[6].y = fillpoly[2].y;    
				fillpoly[1].x = pol[1].x;         
				fillpoly[1].y = pol[1].y;         
				fillpoly[0].x = pol[2].x;
				fillpoly[0].y = pol[2].y; 
				fillpoly[5].x = pol[2].x;
				fillpoly[5].y = pol[2].y;
				fillpoly[4].x = pol[3].x;
				fillpoly[4].y = pol[3].y;
				fillpoly[3].x = pol[4].x;
				fillpoly[3].y = pol[4].y;
				// fillpoly: (when pointed straight up)
				// top - [1]
				// bottom right corner - [0] and [5]
				// center - [2] and [6] <-- where the three colors meet
				// bottom left corner - [3]
							
				// so the three sides are defined as 0-1-2 (rt), 1-2-3 (lt), and 3-4-5-6 (bt)

				devc->SelectObject(&polyInnardsPen);
				devc->SelectObject(&rtBrush);
				devc->Polygon(fillpoly, 3);
				devc->SelectObject(&ltBrush);
				devc->Polygon(&fillpoly[1], 3);
				devc->SelectObject(&btBrush);
				devc->Polygon(&fillpoly[3], 4);

				devc->SelectObject(GetStockObject(NULL_BRUSH));
				devc->SelectObject(&linepen3);
				devc->Polygon(&pol[1], 4);

				rtBrush.DeleteObject();
				ltBrush.DeleteObject();
				btBrush.DeleteObject();
			}
		}

		void WireGui::AmosDraw(CDC *devc, int oX, int oY, int dX, int dY)
		{
			if (oX == dX) {
				++oX;
			}
			if (oY == dY) {
				++oY;
			}
			devc->MoveTo(oX, oY);
			devc->LineTo(dX, dY);	
		}

		const CRgn& WireGui::region() const
		{
			if ( !points().size() )
				return rgn_;
			double distance_ = 20;
			std::pair<double, double>  p1 = PointAt(0);
			std::pair<double, double>  p2 = PointAt(1);

			double  ankathede    = p1.first - p2.first;
			double  gegenkathede = p1.second - p2.second;
			double  hypetenuse   = sqrt( ankathede*ankathede + gegenkathede*gegenkathede);

			if (hypetenuse == 0)
				return rgn_;

			double cos = ankathede    / hypetenuse;
			double sin = gegenkathede / hypetenuse;

			int dx = static_cast<int> ( -sin*distance_);
			int dy = static_cast<int> ( -cos*distance_);
 
			std::vector<CPoint> pts;
			CPoint p;
			p.x = p1.first + dx; p.y = p1.second - dy;
			pts.push_back(p);
			p.x = p2.first + dx; p.y = p2.second - dy;
			pts.push_back(p);
			p.x = p2.first - dx; p.y = p2.second + dy;
			pts.push_back(p);
			p.x = p1.first - dx; p.y = p1.second + dy;
			pts.push_back(p);
			
			rgn_.DeleteObject();
			/*int err =*/ rgn_.CreatePolygonRgn(&pts[0],pts.size(), WINDING);
			return rgn_;
		}

     	bool WireGui::OnEvent(PsycleCanvas::Event* ev)
		{
			switch(ev->type) {
				case PsycleCanvas::Event::BUTTON_PRESS:
					if ((ev->button == 1 || ev->button == 3) && (ev->shift & MK_SHIFT)) {
						view_->OnWireRewire(this, 0);
						newcon_ = true;
					} else
					if (ev->shift & MK_CONTROL) {
						view_->OnWireRewire(this, 1);
						newcon_ = true;
					} else 
					if (ev->button == 3) {
						newcon_ = false;
					}
				break;
				case PsycleCanvas::Event::BUTTON_2PRESS:
					ShowDialog(ev->x, ev->y);				
				break;
				case PsycleCanvas::Event::MOTION_NOTIFY:
					if(dragging_) {
						DoDragging(ev->x, ev->y);
					} else if (ev->button == 3 && !newcon_) {
						view_->OnWireRewire(this, 0);
						newcon_ = true;
					}
				break;
				case PsycleCanvas::Event::BUTTON_RELEASE:
					if(dragging_) {
 				       StopDragging();
					   view_->OnRewireEnd(this, ev->x, ev->y, drag_picker_);
					}
					else if (ev->button == 3 && !newcon_) {
						ShowDialog(ev->x, ev->y);
					}
					break;
				default:
					;
			}
			return true;
		}
/*
		TestCanvas::Item* WireGui::intersect(double x, double y)
		{
			int oriX = points().at(0).first;
			int oriY = points().at(0).second;

			int desX = points().at(1).first;
			int desY = points().at(1).second;

			int const f1 = (desX+oriX)/2;
			int const f2 = (desY+oriY)/2;
			CPoint pol[5];

			pol[0].x = f1 - 10;
			pol[0].y = f2 - 10;
			pol[1].x = pol[0].x + 20;
			pol[1].y = pol[0].y;
			pol[2].x = pol[0].x + 20;
			pol[2].y = pol[0].y + 20;
			pol[3].x = pol[0].x ;
			pol[3].y = pol[0].y + 20;
			pol[4].x = pol[0].x;
			pol[4].y = pol[0].y;
			CRgn rgn;
			int err = rgn.CreatePolygonRgn(&pol[0],5, WINDING);
			return rgn.PtInRegion(x,y) ? this : 0;
		}
*/
		void WireGui::ShowDialog(double x, double y)
		{
			if (!wire_dlg_) {			
				wire_dlg_ = new CWireDlg(fromGUI_->view()->child_view(), this);
				wire_dlg_->this_index = 0;
				wire_dlg_->wireIndex = 0;
				wire_dlg_->isrcMac = fromGUI_->mac()->id();
				wire_dlg_->_pSrcMachine = fromGUI_->mac();
				wire_dlg_->_pDstMachine = toGUI_->mac();
				wire_dlg_->Create();
				//pParentMain->CenterWindowOnPoint(wdlg, point);
				wire_dlg_->ShowWindow(SW_SHOW);
			}
		}

		void WireGui::UpdatePosition()
		{
			//PsycleCanvas::Group* parentGroup = parent();
			double xp1, xp2;
			xp1 = 0;
			xp2 = 0; //parentGroup->GetBounds(xp1, yp1, xp2, yp2);
			if ( fromGUI_ ) {				
				double x1, y1, x2, y2;
				fromGUI_->GetBounds( x1, y1, x2, y2);			
				double midW = (x2 - x1) / 2;
				double midH = (y2 - y1) / 2;
				/*PsycleCanvas::Group* fromParent = fromGUI_->parent();
				double x3, y3, x4, y4;
				fromParent->GetBounds(x3,y3,x4,y4);*/
				double x = x1; //+x3 - xp1;
				double y = y1; //+y3 - yp1;

				PsycleCanvas::Line::Points points(2);
				points[0] = std::pair<double,double>(x + midW, y + midH);
				points[1] = PointAt(1);
				SetPoints(points);
			}
			if ( toGUI_ ) {
				double x1, y1, x2, y2;
				toGUI_->GetBounds( x1, y1, x2, y2);
				double midW = (x2 - x1) / 2;
				double midH = (y2 - y1) / 2;
				/*PsycleCanvas::Group* toParent = toGUI_->parent();
				double x3, y3, x4, y4;
				toParent->GetBounds(x3,y3,x4,y4);*/
				double x = x1; //+x3 - xp1;
				double y = y1; // +y3 - yp1;
				PsycleCanvas::Line::Points points(2);
				points[0] = PointAt(0);
				points[1] = std::pair<double,double>(x + midW, y + midH);
				SetPoints(points);
			}
		}

	}  // namespace host
}  // namespace psycle
