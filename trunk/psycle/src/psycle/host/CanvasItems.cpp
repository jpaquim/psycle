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
  g->set_color(fill_color_);
  g->FillRect(ui::Rect(ui::Point(), dim())); 
}

void Line::Draw(Graphics* g, Region& draw_region) {  
  g->set_color(color());
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
    vertical_alignment_(AlignStyle::ALTOP),
    justify_(JustifyStyle::LEFTJUSTIFY),
    color_(0xFFFFFFFF),    
	  is_aligned_(false) {      
}

Text::Text(const std::string& text) : 
  vertical_alignment_(AlignStyle::ALTOP),
  justify_(JustifyStyle::LEFTJUSTIFY),
  color_(0xFFFFFFFF),
  text_(text),  
  is_aligned_(false) { 
}

void Text::set_property(const ConfigurationProperty& configuration_property) {
  if (configuration_property.name() == "color") {
    set_color(configuration_property.int_value());
  } else
  if (configuration_property.name() == "font") {
    if (configuration_property.int_value() != -0) {
      FontInfo info;
      info.set_stock_id(configuration_property.int_value());
      set_font(ui::Font(info));
    } else {
      set_font(ui::Font(configuration_property.font_info_value()));
    }
  }
}

void Text::set_text(const std::string& text) {
  if (text_ != text) {
    text_ = text;
    UpdateTextAlignment();   
    UpdateAutoDimension();  
    FLSEX();
  }
}

void Text::set_font(const Font& font) {  
  font_ = font;
  UpdateTextAlignment();
	UpdateAutoDimension();
  FLSEX();
}

void Text::Draw(Graphics* g, Region& draw_region) {  	
	g->SetFont(font_);  
  g->set_color(color_);
	g->DrawString(text_, text_alignment_position());  
}

Dimension Text::text_dimension() const {
  Graphics g;		
	g.SetFont(font_); 
	return g.text_dimension(text_);
}

const Point& Text::text_alignment_position() {
  if (!is_aligned_) { 
    ui::Dimension text_dim = text_dimension();
    alignment_position_.set_xy(justify_offset(text_dim),
                               vertical_alignment_offset(text_dim));
		is_aligned_ = true;
  }	
  return alignment_position_;
}

double Text::justify_offset(const Dimension& text_dimension) {
  double result(0);
  switch (justify_) {	  
		case JustifyStyle::CENTERJUSTIFY:
			result = (dim().width() - text_dimension.width())/2;
		break;
		case JustifyStyle::RIGHTJUSTIFY:
			result = dim().width() - text_dimension.width();
		break;
		default:
		break;
  }
  return result;
}

double Text::vertical_alignment_offset(const Dimension& text_dimension) {
  double result(0);
	switch (vertical_alignment_) {	  
		case AlignStyle::ALCENTER:        
      result = (dim().height() - text_dimension.height())/2;
		break;
		case AlignStyle::ALBOTTOM:
		  result = dim().height() - text_dimension.height();
		break;
		default:      
		break;
	}
  return result;
}

// Pic
void Pic::Draw(Graphics* g, Region& draw_region) {
	if (image_) {
		if (zoom_factor_ == Point(1.0, 1.0)) {
			g->DrawImage(image_, ui::Rect(ui::Point::zero(), view_dimension_), src_);
		} else {
		  g->DrawImage(image_, ui::Rect(ui::Point::zero(), view_dimension_), src_, zoom_factor_);
		}
	}
}

void Pic::SetImage(Image* image) {  
  image_ = image;
  view_dimension_ = image_ ? image_->dim() : Dimension::zero();
  FLSEX();
}

Dimension Pic::OnCalcAutoDimension() const {
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
  set_orientation(Orientation::HORZ);
  set_align(AlignStyle::ALBOTTOM);
  set_position(Rect(Point(), Dimension(0, 5)));
}

Splitter::Splitter(Orientation::Type orientation) :
   Window(), 
	 fill_color_(0x404040),
	 do_split_(false),
	 drag_pos_(-1),
	 parent_abs_pos_(0),
	 item_(0) {
  set_auto_size(false, false);
  set_orientation(orientation);
  if (orientation == Orientation::HORZ) {
	  set_align(AlignStyle::ALBOTTOM);
	  set_position(Rect(Point(), Dimension(0, 5)));
  } else {
	  set_align(AlignStyle::ALLEFT);
	  set_position(Rect(Point(), Dimension(5, 0)));
  }
}


void Splitter::Draw(Graphics* g, Region& draw_region) { 
  g->set_color(fill_color_);
  g->FillRect(ui::Rect(Point(), dim()));   
}

void Splitter::OnMouseDown(MouseEvent& ev) {	
	Window* last = item_ = 0;
	drag_pos_ = -1;
  for (iterator it = parent()->begin(); it!=parent()->end(); ++it) {
    if ((*it).get() == this) {
			item_ = last;
      break;
    }
		last = (*it).get();
  }
	if (item_) {		
		parent_abs_pos_ = (orientation_ == Orientation::HORZ) 
                      ? parent()->absolute_position().top()
                      : parent()->absolute_position().left();
    do_split_ = true;		
    BringToTop();
	  SetCapture();	
	}
}

void Splitter::OnMouseUp(MouseEvent& ev) {
	do_split_ = false;
	ReleaseCapture();
  if (orientation_ == Orientation::VERT) {   
    if (align() == AlignStyle::ALLEFT) {
      item_->set_position(Rect(item_->position().top_left(),
                               Dimension(position().top_left().x() -
                               item_->position().top_left().x(),
                               item_->position().height())));      
    }
  } else      
  if (orientation_ == Orientation::HORZ) {    
    if (align() == AlignStyle::ALBOTTOM) {
		  item_->set_position(Rect(item_->position().top_left(),
                               Dimension(item_->position().width(),
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
		if (orientation_ == Orientation::HORZ) {
      if (drag_pos_ != ev.client_pos().y()) {        			  
         drag_pos_ = (std::max)(0.0, ev.client_pos().y());
         set_position(Point(position().top_left().x(),
                            drag_pos_ - parent_abs_pos_));
      }
		} else {
			if (orientation_ == Orientation::VERT) {
				if (drag_pos_ != ev.client_pos().x()) {
          drag_pos_ = (std::max)(0.0, ev.client_pos().x());
          set_position(Point(drag_pos_ - parent_abs_pos_,
                             position().top_left().y())); 
				}
			}
		}
	} else {
		if (orientation_ == Orientation::HORZ) {
			SetCursor(CursorStyle::ROW_RESIZE);
		} else
		if (orientation_ == Orientation::VERT) {
			SetCursor(CursorStyle::COL_RESIZE);
		}
	}
}

void Splitter::OnMouseOut(MouseEvent& ev) {
	if (!do_split_) {
		SetCursor(CursorStyle::DEFAULT);
	}
}

TerminalView::TerminalView() : Scintilla(), Timer() {
  set_background_color(0xFF232323);
  set_foreground_color(0xFFFFBF00);      
  StyleClearAll();
  set_linenumber_foreground_color(0xFF939393);
  set_linenumber_background_color(0xFF232323);     
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
  Canvas::Ptr view_port = Canvas::Ptr(new Canvas());
  view_port->set_aligner(Aligner::Ptr(new DefaultAligner()));
  set_viewport(view_port);
  view_port->SetSave(false);
  terminal_view_ = boost::shared_ptr<TerminalView>(new TerminalView());
  terminal_view_->set_auto_size(false, false);
  terminal_view_->set_align(AlignStyle::ALCLIENT);
  view_port->Add(terminal_view_);      
  Group::Ptr option_panel(new Group());
  option_panel->set_aligner(Aligner::Ptr(new DefaultAligner()));
  option_panel->set_auto_size(false, true);
  option_panel->set_align(AlignStyle::ALBOTTOM);  
  option_background_.reset(OrnamentFactory::Instance().CreateFill(0xFFCACACA));
  option_panel->add_ornament(option_background_);  
  view_port->Add(option_panel);  
  CheckBox::Ptr autoscroll_checkbox(new CheckBox());
  autoscroll_checkbox->set_auto_size(false, false);
  autoscroll_checkbox->set_align(AlignStyle::ALLEFT);
  autoscroll_checkbox->set_text("Autoscroll");
  autoscroll_checkbox->set_position(ui::Rect(ui::Point(), ui::Dimension(200, 20)));  
  option_panel->Add(autoscroll_checkbox);
  set_position(Rect(Point(), Dimension(500, 400)));
}

HeaderGroup::HeaderGroup() {		
	Init();
}

HeaderGroup::HeaderGroup(const std::string& title) {
	Init();
	header_text_->set_text(title);
}

void HeaderGroup::Init() {
	header_background_.reset(OrnamentFactory::Instance().CreateFill(0xFF6D799C));
	LineBorder* border = new LineBorder(0xFF444444);
	border->set_border_radius(BorderRadius(5));
	border_.reset(border);
	add_ornament(border_);
	set_aligner(Aligner::Ptr(new DefaultAligner()));
	set_auto_size(false, false);
	Group::Ptr header(new Group());
	header->set_align(AlignStyle::ALTOP);
	header->set_auto_size(false, true);
	header->set_aligner(Aligner::Ptr(new DefaultAligner()));
	header->add_ornament(header_background_);
	header->set_margin(BoxSpace(5));
	Group::Add(header);
	header_text_.reset(new Text());
	header_text_->set_text("Header");
	header->Add(header_text_);	
	header_text_->set_font(Font(FontInfo("Tahoma", 12, 500, FontStyle::ITALIC)));
	header_text_->set_color(0xFFFFFFFF);
	header_text_->set_align(AlignStyle::ALLEFT);
	header_text_->set_auto_size(true, true);
	header_text_->set_margin(BoxSpace(0, 5, 0, 0));

	client_.reset(new Group());
	Group::Add(client_);
	client_->set_align(AlignStyle::ALCLIENT);
	client_->set_auto_size(false, false);
	client_->set_aligner(Aligner::Ptr(new DefaultAligner()));
	client_->set_margin(BoxSpace(0, 5, 5, 5));
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