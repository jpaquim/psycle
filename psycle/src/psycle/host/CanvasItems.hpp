// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net


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
  Rect() : Window(), fill_color_(0) { set_auto_size(false, false); }

  void SetColor(ARGB color) { fill_color_ = color; FLS(); }

  void Draw(Graphics* g, Region& draw_region);
 private:
   ARGB fill_color_;
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
  void OnSize(const ui::Dimension& dimension) {    
    width_ = dimension.width();
    height_ = dimension.height();
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
  void SetPoints(const Points& pts) { pts_ = pts; FLS(); }
  void SetPoint(int idx, const Point& pt) { pts_[idx] = pt; FLS(); }
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
  typedef boost::shared_ptr<Text> Ptr;
  typedef boost::shared_ptr<const Text> ConstPtr;
  typedef boost::weak_ptr<Text> WeakPtr;
  typedef boost::weak_ptr<const Text> ConstWeakPtr;

  Text() : Window(), 
      vertical_alignment_(ALTOP),
      justify_(LEFTJUSTIFY),
      color_(0xFFFFFF) {
  }
  Text(const std::string& text) : 
      vertical_alignment_(ALTOP),
      justify_(LEFTJUSTIFY),
      color_(0xFFFFFF),
      text_(text) { 
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
  void set_font(const Font& font);
  void set_vertical_alignment(AlignStyle vertical_alignment) { 
    vertical_alignment_ = vertical_alignment;
  }
  void set_justify(JustifyStyle justify) { justify_ = justify; }

 private:
  ui::Point ComputeAlignment(Graphics* g) const;
  std::string text_;
  AlignStyle vertical_alignment_;
  JustifyStyle justify_;
  ARGB color_;
  ui::Font font_;  
};


class TerminalView : public Scintilla, public psycle::host::Timer {
 public: 
  TerminalView();
  void output(const std::string& text);   
  virtual void OnTimerViewRefresh() { 
    invokelater.Invoke();
    invokelater.Clear();
  }

 private:
   ui::Commands invokelater;
};

class TerminalFrame : public Frame {
  public:   
   void Init();
   void output(const std::string& text) {      
     Show();     
     terminal_view_->output(text);
   }
   virtual void OnShow() { terminal_view_->StartTimer(); }
   virtual void OnClose() { Hide(); }

   virtual void release_imp() {
     Frame::release_imp();
     terminal_view_->release_imp();
   }   

  private:
   boost::shared_ptr<TerminalView> terminal_view_;
   boost::shared_ptr<canvas::Aligner> align_;
};

} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle