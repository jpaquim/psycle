// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include "Canvas.hpp"
#include <algorithm> // for std::transform
#include <cctype> // for std::tolower

namespace psycle {
namespace host  {
namespace ui {

class RectangleBox : public Window {
 public:
  static std::string type() { return "rectanglebox"; }  
  RectangleBox() : Window(), fill_color_(0) { set_auto_size(false, false); }

  void SetColor(ARGB color) { fill_color_ = color; FLS(); }
  void Draw(Graphics* g, Region& draw_region);

 private:
  ARGB fill_color_;
};

class Pic : public Window {
 public:
  Pic() : Window(), image_(0), zoom_factor_(1.0, 1.0) {
    transparent_ = pmdone = false;
  }
  
  static std::string type() { return "canvaspic"; }

  virtual void Draw(Graphics* g, Region& draw_region);  
	virtual ui::Dimension OnCalcAutoDimension() const;
  void OnSize(const ui::Dimension& dimension) {
		view_dimension_ = dimension;    
    FLS();
  }
  void SetSource(const ui::Point& src) { 
		src_ = src;
    FLS(); 
  }
  void SetImage(Image* image);
	const ui::Dimension& view_dimension() const { return view_dimension_; }
	void set_zoom(const ui::Point& zoom_factor) { 
		zoom_factor_ = zoom_factor;		
		FLSEX();
	}

 private:  
	ui::Dimension view_dimension_;
	ui::Point src_;  
	ui::Point zoom_factor_;
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
      color_(0xFFFFFF),
			is_auto_dimension_calculated_(false),
			is_aligned_(false) {
  }
  Text(const std::string& text) : 
      vertical_alignment_(ALTOP),
      justify_(LEFTJUSTIFY),
      color_(0xFFFFFF),
      text_(text),
			is_auto_dimension_calculated_(false),
			is_aligned_(false) { 
  }

  static std::string type() { return "canvastext"; }

  virtual void Draw(Graphics* cr, Region& draw_region);  
  void set_text(const std::string& text);
  const std::string& text() const { return text_; }
  void set_color(ARGB color) { 
    color_ = color;
		FLS();
  }
  ARGB color() const { return color_; }
  void set_font(const Font& font);
  void set_vertical_alignment(AlignStyle vertical_alignment) { 
    vertical_alignment_ = vertical_alignment;
		is_aligned_ = false;
  }
  void set_justify(JustifyStyle justify) { 
		justify_ = justify;
		is_aligned_ = false;
	}
	virtual ui::Dimension OnCalcAutoDimension() const;

 private:  
	void PrepareGraphics(Graphics* g);	
	void CalculateAlignmentAndJustify() const;	
	void CalculateJustify() const;
	void CalculateAutoDimension() const;
	void PrepareAutoDimensionUpdate() const {		
	  is_auto_dimension_calculated_ = is_aligned_ = false;
  }
	void OutputText(Graphics* g);	

  std::string text_;
  AlignStyle vertical_alignment_;
  JustifyStyle justify_;
  ARGB color_;
  ui::Font font_;
	mutable bool is_auto_dimension_calculated_;
	mutable ui::Dimension auto_dimension_cache_;
	mutable bool is_aligned_;
	mutable ui::Point align_cache_;
};

class Splitter : public Window {
 public:
	typedef boost::shared_ptr<Splitter> Ptr;
  typedef boost::shared_ptr<const Splitter> ConstPtr;
  typedef boost::weak_ptr<Splitter> WeakPtr;
  typedef boost::weak_ptr<const Splitter> ConstWeakPtr;
  static std::string type() { return "canvassplitter"; }  
  Splitter();
  Splitter(Orientation orientation);

  void SetColor(ARGB color) { fill_color_ = color; FLS(); }

	void set_orientation(Orientation orientation) {
		orientation_ = orientation; 
		if (orientation == HORZ) {
			set_align(ALBOTTOM);
			set_position(ui::Rect(ui::Point(), ui::Dimension(0, 5)));
		} else 
	  if (orientation == VERT) {
			set_align(ALLEFT);
		  set_position(ui::Rect(ui::Point(), ui::Dimension(5, 0)));
	  }  		
	}

  void Draw(Graphics* g, Region& draw_region);

	virtual void OnMouseDown(MouseEvent& ev);
	virtual void OnMouseUp(MouseEvent& ev);
	virtual void OnMouseMove(MouseEvent& ev);
	virtual void OnMouseOut(MouseEvent& ev);
 private:
   ARGB fill_color_;
	 bool do_split_;
	 Orientation orientation_;
	 double drag_pos_;
	 double parent_abs_pos_;
	 ui::Window* item_;
};

class TerminalView : public Scintilla, public psycle::host::Timer {
 public: 
  TerminalView();
  void output(const std::string& text);   
  virtual void OnTimerViewRefresh() { invokelater.Invoke(); }

 private:
   ui::Commands invokelater;
};

class TerminalFrame : public Frame {
  public:   
   TerminalFrame() { Init(); }
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
   
   static void InitInstance() { instance(); }
   static void ExitInstance() { terminal_frame_.reset(0); }
   static TerminalFrame& instance() {
     if (terminal_frame_.get() == 0) {
       terminal_frame_ = std::auto_ptr<TerminalFrame>(new TerminalFrame());
     }
     return *terminal_frame_;
   }

  private:
   boost::shared_ptr<TerminalView> terminal_view_;
   static std::auto_ptr<TerminalFrame> terminal_frame_;
};

class HeaderGroup : public ui::Group {
 public:
	static std::string type() { return "canvasheadergroup"; }
  
	HeaderGroup();
	HeaderGroup(const std::string& title);

	virtual void Add(const ui::Window::Ptr& item);
	virtual void RemoveAll();
	virtual void UpdateAlign();
	void set_title(const std::string& title) { header_text_->set_text(title); }
	const std::string& title() const { return header_text_->text(); }

	void FlsClient();

 private:
	void Init();
	ui::Ornament::Ptr header_background_;
	ui::Ornament::Ptr border_;
	ui::Text::Ptr header_text_;
	ui::Group::Ptr client_;
};

} // namespace ui
} // namespace host
} // namespace psycle