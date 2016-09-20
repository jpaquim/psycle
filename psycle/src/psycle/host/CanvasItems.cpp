// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

// #include "stdafx.h"

#include "CanvasItems.hpp"
#include "Ui.hpp"
#include "Scintilla.h"

namespace psycle {
namespace host  {
namespace ui {
  
void RectangleBox::Draw(Graphics* g, Region& draw_region) {
 //if (GetAlpha(fill_color_) != 0xFF) {
    g->SetColor(fill_color_);
    g->FillRect(ui::Rect(ui::Point(), dim()));  
 //}
}

void Line::Draw(Graphics* g, Region& draw_region) {  
  g->SetColor(color());
  ui::Point mp;  
  for (Points::iterator it = pts_.begin(); it != pts_.end(); ++it) {
    Point& pt = (*it);
    if (it != pts_.begin()) {
      g->DrawLine(mp, pt);
    }
    mp = pt;    
  }
}

Window::Ptr Line::Intersect(double x, double y, Event* ev, bool &worked) {
  /*double distance_ = 5;
  Point  p1 = PointAt(0);
  Point  p2 = PointAt(1);
  double  ankathede    = p1.first - p2.first;
  double  gegenkathede = p1.second - p2.second;
  double  hypetenuse   = sqrt(ankathede*ankathede + gegenkathede*gegenkathede);
  if (hypetenuse == 0) return 0;
  double cos = ankathede    / hypetenuse;
  double sin = gegenkathede / hypetenuse;
  int dx = static_cast<int> (-sin*distance_);
  int dy = static_cast<int> (-cos*distance_);
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
  CRgn rgn;
  rgn.CreatePolygonRgn(&pts[0],pts.size(), WINDING);
  Item::Ptr item = rgn.PtInRegion(x-this->x(),y-this->y()) ? this : 0;
  rgn.DeleteObject();*/
  return Window::Ptr();
}

/*bool Line::OnUpdateArea(const ui::Dimension& preferred_dimension) {
  double dist = 5;  
  double zoom = 1.0; // parent() ? parent()->zoom() : 1.0;
  ui::Rect bounds = area().bounds();
  area_->Clear();
  area_->Add(RectShape(ui::Rect(ui::Point((bounds.left()-dist)*zoom, 
                                          (bounds.top()-dist)*zoom),
                                ui::Point((bounds.left() + bounds.width()+2*dist+1)*zoom, 
                                          (bounds.top() + bounds.height()+2*dist+1)*zoom))));
  return true;
}*/


// Text
Text::Text() : Window(), 
    vertical_alignment_(ALTOP),
    justify_(LEFTJUSTIFY),
    color_(0xFFFFFF),
    is_auto_dimension_calculated_(false),
	  is_aligned_(false) {      
}

Text::Text(const std::string& text) : 
  vertical_alignment_(ALTOP),
  justify_(LEFTJUSTIFY),
  color_(0xFFFFFF),
  text_(text),
  is_auto_dimension_calculated_(false),
  is_aligned_(false) { 
}

void Text::set_property(const ConfigurationProperty& configuration_property) {
  if (configuration_property.name() == "color") {
    set_color(configuration_property.int_value());
  } else
  if (configuration_property.name() == "font") {    
    set_font(ui::Font(configuration_property.font_info_value()));
  }
}

ui::Dimension Text::OnCalcAutoDimension() const {
	if (!is_auto_dimension_calculated_) {			
		CalculateAutoDimension();
	}
	return auto_dimension_cache_;
}

void Text::set_text(const std::string& text) {  
  text_ = text;
  if (auto_size()) {
    PrepareAutoDimensionUpdate();
    UpdateAutoDimension();
  }
  FLSEX();
}

void Text::set_font(const Font& font) {  
  font_ = font;
	PrepareAutoDimensionUpdate();
	UpdateAutoDimension();  
}

void Text::Draw(Graphics* g, Region& draw_region) {	
	PrepareGraphics(g);
	CalculateAlignmentAndJustify();
	OutputText(g);
}

void Text::PrepareGraphics(Graphics* g) {
	g->SetFont(font_);
  g->SetColor(color_);
}

void Text::CalculateAlignmentAndJustify() const {	
	CalculateAutoDimension();
	if (!is_aligned_) {
		align_cache_.reset();
		switch (justify_) {
			case LEFTJUSTIFY:
			break;    
			case CENTERJUSTIFY:
				align_cache_.set_x((dim().width() - auto_dimension_cache_.width()) / 2);
			break;
			case RIGHTJUSTIFY:
				align_cache_.set_x(dim().width() - auto_dimension_cache_.width());
			break;
			default:
			break;
		}
		CalculateJustify();
		is_aligned_ = true;
	}	
}

void Text::CalculateAutoDimension() const {
	if (!is_auto_dimension_calculated_) {		
		Graphics g;		
		g.SetFont(font_);
		auto_dimension_cache_ = g.text_size(text_);
		is_auto_dimension_calculated_ = true;
	}
}

void Text::CalculateJustify() const {
	switch (vertical_alignment_) {
			case ALTOP:
			break;
			case ALCENTER:
				align_cache_.set_y((dim().height() - auto_dimension_cache_.height()) / 2);
			break;
			case ALBOTTOM:
				align_cache_.set_y(dim().height() - auto_dimension_cache_.height());
			break;
			default:      
			break;
		}
}

void Text::OutputText(Graphics* g) {
	g->DrawString(text_, align_cache_);  
}

// Pic
void Pic::Draw(Graphics* g, Region& draw_region) {
	if (image_) {
		if (zoom_factor_ == ui::Point(1.0, 1.0)) {
			g->DrawImage(image_, ui::Rect(ui::Point::zero(), view_dimension_), src_);
		} else {
		  g->DrawImage(image_, ui::Rect(ui::Point::zero(), view_dimension_), src_, zoom_factor_);
		}
	}
}

void Pic::SetImage(Image* image) {  
  image_ = image;
  view_dimension_ = image_ ? image_->dim() : ui::Dimension::zero();
  FLSEX();
}

ui::Dimension Pic::OnCalcAutoDimension() const {
	return view_dimension_;
}

Splitter::Splitter() :  
	Window(), 
	fill_color_(0x404040),
	do_split_(false),
	drag_pos_(-1),
	parent_abs_pos_(0),
	item_(0) {
  set_auto_size(false, false);
  set_orientation(HORZ);
  set_align(ui::ALBOTTOM);
  set_position(ui::Rect(ui::Point(), ui::Dimension(0, 5)));
}

Splitter::Splitter(Orientation orientation) :
     Window(), 
	 fill_color_(0x404040),
	 do_split_(false),
	 drag_pos_(-1),
	 parent_abs_pos_(0),
	 item_(0) {
  set_auto_size(false, false);
  set_orientation(orientation);
  if (orientation == ui::HORZ) {
	set_align(ui::ALBOTTOM);
	set_position(ui::Rect(ui::Point(), ui::Dimension(0, 5)));
  } else {
	set_align(ui::ALLEFT);
	set_position(ui::Rect(ui::Point(), ui::Dimension(5, 0)));
  }
}


void Splitter::Draw(Graphics* g, Region& draw_region) {
 //if (GetAlpha(fill_color_) != 0xFF) {
    g->SetColor(fill_color_);
    g->FillRect(ui::Rect(ui::Point(), dim()));  
 //}
}

void Splitter::OnMouseDown(MouseEvent& ev) {	
	ui::Window* last = item_ = 0;
	drag_pos_ = -1;
  for (iterator it = parent()->begin(); it!=parent()->end(); ++it) {
    if ((*it).get() == this) {
			item_ = last;
      break;
    }
		last = (*it).get();
  }
	if (item_) {		
		parent_abs_pos_ = (orientation_ == HORZ) ? parent()->abs_position().top()
                                             : parent()->abs_position().left();
    do_split_ = true;		
    BringWindowToTop();
	  SetCapture();	
	}
}

void Splitter::OnMouseUp(MouseEvent& ev) {
	do_split_ = false;
	ReleaseCapture();
  if (orientation_ == VERT) {   
    if (align() == ALLEFT) {
      item_->set_position(ui::Rect(item_->position().top_left(),
                     ui::Dimension(position().top_left().x() -
                                   item_->position().top_left().x(),
                                   item_->position().height())));      
    }
  } else      
  if (orientation_ == HORZ) {    
    if (align() == ALBOTTOM) {
		  item_->set_position(ui::Rect(item_->position().top_left(),
                     ui::Dimension(item_->position().width(),
                                   item_->position().bottom_right().y() - 
                                   position().top_left().y() - 
                                   position().height())));
    }
  }  
  ((Group*)parent())->FlagNotAligned();
  parent()->UpdateAlign();    
}

void Splitter::OnMouseMove(MouseEvent& ev) {
	if (do_split_) {
		if (orientation_ == HORZ) {
      if (drag_pos_ != ev.cy()) {        			  
         set_position(ui::Point(position().top_left().x(), ev.cy() - parent_abs_pos_));
         drag_pos_ = ev.cy();
      }
		} else {
			if (orientation_ == VERT) {
				if (drag_pos_ != ev.cx()) {													
          set_position(ui::Point(ev.cx() - parent_abs_pos_, position().top_left().y()));
          drag_pos_ = ev.cx();
				}
			}
		}
	} else {
		if (orientation_ == HORZ) {
			SetCursor(ui::ROW_RESIZE);
		} else
		if (orientation_ == VERT) {
			SetCursor(ui::COL_RESIZE);
		}
	}
}

void Splitter::OnMouseOut(MouseEvent& ev) {
	if (!do_split_) {
		SetCursor(ui::DEFAULT);
	}
}

TerminalView::TerminalView() : Scintilla(), Timer() {
  set_background_color(0x232323);
  set_foreground_color(0xFFBF00);      
  StyleClearAll();
  set_linenumber_foreground_color(0x939393);
  set_linenumber_background_color(0x232323);   
  set_margin_background_color(0x232323);  
  f(SCI_SETWRAPMODE, (void*) SC_WRAP_CHAR, 0);
	StartTimer();
}

void TerminalView::output(const std::string& text) {
  struct {
    std::string text;
    TerminalView* that;
    void operator()() const {
      that->AddText(text);
    }
   } f;   
  f.that = this;
  f.text = text;
  invokelater.Add(f);
}

std::auto_ptr<TerminalFrame> TerminalFrame::terminal_frame_(0);

void TerminalFrame::Init() {
  set_title("Psycle Terminal");
  ui::Canvas::Ptr maincanvas = ui::Canvas::Ptr(new ui::Canvas());
  set_viewport(maincanvas);
  maincanvas->SetSave(false);
  terminal_view_ = boost::shared_ptr<TerminalView>(new TerminalView());
  terminal_view_->set_align(ALCLIENT);
  maincanvas->Add(terminal_view_);  
  maincanvas->set_aligner(ui::Aligner::Ptr(new DefaultAligner()));
  set_position(ui::Rect(ui::Point(0, 0), ui::Dimension(500, 400)));
}

HeaderGroup::HeaderGroup() {		
	Init();
}

HeaderGroup::HeaderGroup(const std::string& title) {
	Init();
	header_text_->set_text(title);
}

void HeaderGroup::Init() {
	header_background_.reset(ui::OrnamentFactory::Instance().CreateFill(0x6D799C));
	ui::LineBorder* border = new ui::LineBorder(0x444444);
	border->set_border_radius(ui::BorderRadius(5, 5, 5, 5));
	border_.reset(border);
	add_ornament(border_);
	set_aligner(ui::Aligner::Ptr(new DefaultAligner()));
	set_auto_size(false, false);
	ui::Group::Ptr header(new ui::Group());
	header->set_align(ui::ALTOP);
	header->set_auto_size(false, true);
	header->set_aligner(ui::Aligner::Ptr(new DefaultAligner()));
	header->add_ornament(header_background_);
	header->set_margin(ui::BoxSpace(5));
	Group::Add(header);
	header_text_.reset(new ui::Text());
	header_text_->set_text("Header");
	header->Add(header_text_);
	ui::FontInfo info;
	info.name = "Tahoma";
	info.height = 12;
	info.bold = true;
	ui::Font font(info);
	header_text_->set_font(font);
	header_text_->set_color(0xFFFFFF);
	header_text_->set_align(ui::ALLEFT);
	header_text_->set_auto_size(true, true);
	header_text_->set_margin(ui::BoxSpace(0, 5, 0, 0));

	client_.reset(new ui::Group());
	Group::Add(client_);
	client_->set_align(ui::ALCLIENT);
	client_->set_auto_size(false, false);
	client_->set_aligner(ui::Aligner::Ptr(new DefaultAligner()));
	client_->set_margin(ui::BoxSpace(0, 5, 5, 5));
}

void HeaderGroup::Add(const ui::Window::Ptr& item) {		
	client_->Add(item);
}

void HeaderGroup::RemoveAll() {		
	client_->RemoveAll();
}

void HeaderGroup::UpdateAlign() {		
	client_->UpdateAlign();
}

void HeaderGroup::FlsClient() {
	client_->Invalidate();
}

} // namespace ui
} // namespace host
} // namespace psycle