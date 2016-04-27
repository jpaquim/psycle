// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "PsycleConfig.hpp"
#include "Ui.hpp"

namespace psycle {
namespace host {
namespace ui {
namespace canvas {

using namespace ui;

struct BlitInfo {
  double dx, dy;  
};

class DefaultAligner : public Aligner {
 public:
  virtual void CalcDimensions();
  virtual void SetPositions();
  virtual void Realign();    
};

// canvas
class Canvas : public ui::Group {  
 public:
  typedef boost::shared_ptr<Canvas> Ptr;
  typedef boost::weak_ptr<Canvas> WeakPtr;  
  
  Canvas() : Group(), save_rgn_(ui::Systems::instance().CreateRegion()) {    
    Init();
  }

  Canvas(ui::Window* parent) : 
      Group(),
      save_rgn_(ui::Systems::instance().CreateRegion()) {    
    Init(); 
  }   
    
  static std::string type() { return "canvas"; }
      
  void Flush();  
  void StealFocus(const Window::Ptr& item);  
  virtual void OnSize(double cx, double cy);
  void SetSave(bool on) { save_ = on; }
  bool IsSaving() const { return save_; }    
  void ClearSave() { save_rgn_->Clear(); }
     
  void InvalidateSave();  

  virtual void WorkMouseDown(MouseEvent& ev);
  virtual void WorkMouseUp(MouseEvent& ev);
  virtual void WorkMouseMove(MouseEvent& ev);
  virtual void WorkDblClick(MouseEvent& ev);
  virtual void WorkKeyUp(KeyEvent& ev);
  virtual bool WorkKeyDown(KeyEvent& ev); 
  virtual void WorkOnFocus(Event& ev);

  boost::signal<void (std::exception&)> error;

  void Invalidate();
  void Invalidate(const Region& rgn);
  virtual void PreventFls() { fls_prevented_ = true; }
  virtual void EnableFls() { fls_prevented_ = false; }
  bool is_fls_prevented() const { return fls_prevented_; }

  virtual void OnFocusChange(int id) { OnMessage(FOCUS, id); }

  virtual bool is_root() const { return true; }
  
 private:  
  void Init();  
  Canvas(const Canvas& other) {}
  Canvas& operator=(Canvas rhs) {}
 
  template <class T, class T1>
  void WorkEvent(T& ev, void (T1::*ptmember)(T&), Window::Ptr& item, bool set_focus = false) {
    while (item) {
      // send event to item
      if (set_focus) {
        SetFocus(item);
      }
      (item.get()->*ptmember)(ev);
      if (ev.is_work_parent()) {
        item = item->parent().lock();  
      } else {    
        break;
      }
    }
  }
   
  bool save_, steal_focus_, fls_prevented_;
  Window::WeakPtr button_press_item_, mouse_move_;  
  std::auto_ptr<ui::Region> save_rgn_;  
};

enum LineFormat {
  DOTTED, DASHED, SOLID, DOUBLE, GROOVE, RIDGE, INSET, OUTSET, NONE, HIDDEN
};

struct BorderStyle {
  BorderStyle() : top(SOLID), right(SOLID), bottom(SOLID), left(SOLID) {}
  BorderStyle(LineFormat left, LineFormat top, LineFormat right,
              LineFormat bottom) :
      left(left), top(top), right(right), bottom(bottom) {
  }
  LineFormat left, top, right, bottom;  
};

struct BorderRadius {
  BorderRadius() { left_top = right_top = left_bottom = right_bottom = 0.0; }
  BorderRadius(double left_top, double right_top, double left_bottom, double right_bottom) : 
      left_top(left_top),
      right_top(right_top),
      left_bottom(left_bottom),
      right_bottom(right_bottom) {
  }

  bool empty() const { 
    return left_top == 0 &&  right_top == 0 && left_bottom == 0 
           && right_bottom == 0;
  }
  double left_top;
  double right_top;
  double left_bottom;
  double right_bottom;
};

class LineBorder : public ui::Ornament {
 public:
  LineBorder() : color_(0xFFFFFF) {}
  LineBorder(ARGB color) : color_(color) {}  

  LineBorder* Clone() {
    LineBorder* border = new LineBorder();
    *border = *this;
    return border;
  }
   
  virtual void Draw(Window::Ptr& item, Graphics* g, Region& draw_region) {    
    DrawBorder(item, g, draw_region);
  }
  virtual std::auto_ptr<ui::Rect> padding() const {
    ui::Point pad(1, 1);
    return std::auto_ptr<ui::Rect>(new ui::Rect(pad, pad));
  }

  void set_border_radius(const BorderRadius& radius) { border_radius_ = radius; }
  const BorderRadius& border_radius() const { return border_radius_; }
  void set_border_style(const BorderStyle& style) { border_style_ = style; }
  const BorderStyle& border_style() const { return border_style_; }
  
 private:  
  void DrawBorder(Window::Ptr& item, Graphics* g, Region& draw_region) {
    g->SetColor(color_);  
    ui::Rect rc = item->area().bounds();
    if (border_radius_.empty()) {
      g->DrawRect(rc);
    } else { 
      if (border_style_.top != NONE) {
        g->DrawLine(rc.left() + border_radius_.left_top, rc.top(), rc.right() - border_radius_.right_top - 1, rc.top());
      }
      if (border_style_.bottom != NONE) {
        g->DrawLine(rc.left() + border_radius_.left_bottom, rc.bottom() - 1, rc.right() - border_radius_.right_bottom - 1, rc.bottom() - 1); 
      }
      if (border_style_.left != NONE) {
        g->DrawLine(rc.left(), rc.top() + border_radius_.left_top, rc.left(), rc.bottom() - border_radius_.left_bottom -1);
      }
      if (border_style_.right != NONE) {
        g->DrawLine(rc.right() - 1, rc.top() + border_radius_.right_top, rc.right() - 1, rc.bottom() - border_radius_.right_bottom);  
      }
      if (border_radius_.left_top != 0 && border_style_.top != NONE) {
        g->DrawArc(ui::Rect(rc.top_left(), 
                            ui::Point(rc.left() + 2*border_radius_.left_top, rc.top() + 2*border_radius_.left_top)),
                   Point(rc.left() + border_radius_.left_top, rc.top()),
                   Point(rc.left(), rc.top() + border_radius_.left_top));
      }
      if (border_radius_.right_top != 0 && border_style_.top != NONE) {
        g->DrawArc(ui::Rect(ui::Point(rc.right() - 2*border_radius_.right_top - 1, rc.top()),
                            ui::Point(rc.right() - 1, rc.top() + 2*border_radius_.right_top)), 
                   Point(rc.right() - 1, rc.top() + border_radius_.right_top),
                   Point(rc.right() - border_radius_.right_top - 1, rc.top()));    
      }
      if (border_radius_.left_bottom != 0 && border_style_.bottom != NONE ) {
        g->DrawArc(ui::Rect(ui::Point(rc.left(), rc.bottom() - 2*border_radius_.left_bottom - 1),
                            ui::Point(rc.left() + 2*border_radius_.left_bottom, rc.bottom() - 1)), 
                   Point(rc.left(), rc.bottom() - border_radius_.left_bottom - 1),
                   Point(rc.left() + border_radius_.left_bottom, rc.bottom() - 1));
      }
      if (border_radius_.right_bottom != 0 && border_style_.bottom != NONE) {
        g->DrawArc(ui::Rect(ui::Point(rc.right() - 2*border_radius_.right_bottom,
                                      rc.bottom() - 2*border_radius_.right_bottom - 1),
                            ui::Point(rc.right() - 1, rc.bottom() - 1)), 
                   Point(rc.right() - border_radius_.right_bottom - 1, rc.bottom() - 1),
                   Point(rc.right() - 1, rc.bottom() - border_radius_.right_bottom - 1));
      }
    }
  }  
  
  ARGB color_;
  int thickness_;
  BorderRadius border_radius_;
  BorderStyle border_style_;
};

class Wallpaper : public ui::Ornament {
 public:
  Wallpaper() {}
  Wallpaper(ui::Image::WeakPtr image) : image_(image) {}
 
  Wallpaper* Clone() {
    Wallpaper* paper = new Wallpaper();
    *paper = *this;
    return paper;
  }

  virtual void Draw(Window::Ptr& item, Graphics* g, Region& draw_region) {
    DrawWallpaper(item, g, draw_region);
  }

  virtual bool transparent() const { return false; }

 private:
  void DrawWallpaper(Window::Ptr& item, Graphics* g, Region& draw_region) {
    if (!image_.expired()) {
      ui::Dimension dim = item->dim();
      ui::Dimension image_dim = image_.lock()->dim();
      if ((item->dim().width() > image_dim.width()) || 
         (item->dim().height() > image_dim.height())) {
        for (int cx = 0; cx < item->dim().width(); cx += image_dim.width()) {
          for (int cy = 0; cy < item->dim().height(); cy += image_dim.height()) {
            g->DrawImage(image_.lock().get(), cx, cy, image_dim.width(), image_dim.height());
          }
        }
      }
    }
  }
  
  ui::Image::WeakPtr image_;
};

class Fill : public ui::Ornament {
 public:
  Fill() : color_(0xFF000000), use_bounds_(false) {}
  Fill(ARGB color) : color_(color), use_bounds_(false) {}
  Fill(ARGB color, bool use_bounds) : color_(color), use_bounds_(use_bounds) {}

  Fill* Clone() {
    Fill* fill = new Fill();
    *fill = *this;
    return fill;
  }
   
  virtual void set_color(ARGB color) { color_ = color; }
  virtual ARGB color() const { return color_; }  
  
  virtual void Draw(Window::Ptr& item, Graphics* g, Region& draw_region) {
    DrawFill(item, g, draw_region);
  }
  
  void UseWindowBounds() { use_bounds_ = true; }
  void UseWindowArea() { use_bounds_ = false; }

  virtual bool transparent() const { return false; }

 private:
  void DrawFill(Window::Ptr& item, Graphics* g, Region& draw_region) {
    if (draw_region.bounds().height() > 0) {    
      int alpha = GetAlpha(color_);
      if (alpha != 0xFF) {
        g->SetColor(color_);
        g->FillRegion(use_bounds_ ? *item->area().bounds().region().get() 
                                  : item->area().region());        
      }
    }
  }
  ARGB color_;
  bool use_bounds_;
};

class OrnamentFactory {
 public:
  static OrnamentFactory &Instance() {  // Singleton instance
    static OrnamentFactory instance;
    return instance;
  }

  LineBorder* CreateLineBorder() { return new LineBorder(); }
  LineBorder* CreateLineBorder(ARGB color) { return new LineBorder(color); }
  Fill* CreateFill() { return new Fill(); }
  Fill* CreateFill(ARGB color) { return new Fill(color); }
  Fill* CreateBoundFill(ARGB color) { return new Fill(color, true); }
  Wallpaper* CreateWallpaper(ui::Image::WeakPtr image) { return new Wallpaper(image); }

 private:
   OrnamentFactory() {}    
};


} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle