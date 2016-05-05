// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#include <psycle/host/detail/project.private.hpp>
#include "Canvas.hpp"
#include <algorithm> // for std::transform
#include <cctype> // for std::tolower

namespace psycle {
namespace host  {
namespace ui {
namespace canvas {

class Rect : public Window {
 public:
  static std::string type() { return "canvasrect"; }  
  Rect() : Window() { set_auto_size(false, false); }            
};

class Pic : public Window {
 public:
  Pic() : Window() {
    image_ = 0;
    width_ = height_ = xsrc_ = ysrc_ = 0;
    transparent_ = pmdone = false;
  }
  
  static std::string type() { return "canvaspic"; }

  virtual void Draw(Graphics* g, Region& draw_region);
  virtual bool OnUpdateArea();
  void OnSize(double width, double height) {
    STR();
    width_ = width;
    height_ = height;
    FLS();
  }
  void SetSource(double xsrc, double ysrc) { 
    xsrc_ = xsrc;
    ysrc_ = ysrc;
    FLS(); 
  }
  void SetImage(Image* image);
  void Size(double& width, double& height) {
    width = width_;
    height = height_;
  }  

 private:  
  double width_, height_, xsrc_, ysrc_;  
  bool transparent_, pmdone;  
  Image* image_;
};

class Line : public Window {
 public:
  Line() : Window(), color_(0) { }    
  
  static std::string type() { return "canvasline"; }

  virtual void Draw(Graphics* g, Region& draw_region);
  virtual Window::Ptr Intersect(double x, double y, Event* ev, bool &worked);  
  void SetPoints(const Points& pts) {  STR(); pts_ = pts; FLS(); }
  void SetPoint(int idx, const Point& pt) { STR(); pts_[idx] = pt; FLS(); }
  const Points& points() const { return pts_; }
  const Point& PointAt(int index) const { return pts_.at(index); }
  void SetColor(ARGB color) { color_ = color; FLS(); }
  ARGB color() const { return color_; }
  virtual bool OnUpdateArea();
 private:
  Points pts_;
  ARGB color_;
};

class Text : public Window {
 public:
  Text() : Window(), 
      color_(0xFFFFFF),
      font_(ui::Systems::instance().CreateFont()) {
  }
  Text(const std::string& text) : 
      color_(0xFFFFFF),
      text_(text),
      font_(ui::Systems::instance().CreateFont()) { 
  }
  static std::string type() { return "canvastext"; }

  virtual void Draw(Graphics* cr, Region& draw_region);
  virtual bool OnUpdateArea();
  void set_text(const std::string& text);
  const std::string& text() const { return text_; }
  void set_color(ARGB color) { 
    color_ = color;
    Invalidate();
  }
  ARGB color() const { return color_; }
  void set_font(const Font& font) { font_.reset(font.Clone()); }
  void set_alignment(AlignStyle alignment) { alignment_ = alignment; }

 private:
  std::string text_;
  AlignStyle alignment_;
  ARGB color_;
  std::auto_ptr<Font> font_;  
};


} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle