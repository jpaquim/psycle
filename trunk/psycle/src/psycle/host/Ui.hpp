// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#define BOOST_SIGNALS_NO_DEPRECATION_WARNING
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#undef GetGValue
#define GetGValue(rgb) (LOBYTE((rgb) >> 8))
#define ToCOLORREF(argb) RGB((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, (argb >> 0) & 0xFF)
#define GetAlpha(argb) (argb>>24) & 0xFF
#define RGBToARGB(r, g, b) (((ARGB) (b) << 0) | ((ARGB) (g) << 8) |((ARGB) (r) << 16) | ((ARGB) (0) << 24))
#define ToARGB(rgb) RGBToARGB(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb))

namespace psycle {
namespace host {
namespace ui {

typedef uint32_t ARGB;
typedef uint32_t RGB;

struct Point {
  Point() : x_(0), y_(0) {}
  Point(double x, double y) : x_(x), y_(y) {}
  
  void set(double x, double y) { 
    x_ = x; 
    y_ = y; 
  }
  void setx(double x) { x_ = x; }  
  double x() const { return x_; }
  void sety(double y) { y_ = y; }
  double y() const { return y_; }
 private:
  double x_, y_;
};

typedef std::vector<Point> Points;

struct Size {
  Size() : w_(0), h_(0) {}
  Size(double w, double h) : w_(w), h_(h) {}

  inline bool operator==(const Size& rhs) const { 
    return w_ == rhs.width()  && h_ == rhs.height();
  }
  inline bool operator!=(const Size& rhs) const { return !(*this == rhs); }

  void set_size(double w, double h) { 
    w_ = w;
    h_ = h; 
  }
  void set_width(double w) { w_ = w; }
  void set_height(double h) { h_ = h; }
  double width() const { return w_; }
  double height() const { return h_; }

 private:
  double w_, h_;
};

struct Rect {
  Rect() { set(0, 0, 0, 0); }
  Rect(double l, double t, double r, double b) { set(l, t, r, b); }

  inline void set(double l, double t, double r, double b) {
    left_ = l; top_ = t; right_ = r; bottom_ = b;
  }

  inline void setxywh(double x, double y, double w, double h) {
    left_ = x; top_ = y; right_ = x + w; bottom_ = y + h;
  }

  inline void set_left(double val) { left_ = val; }
  inline void set_top(double val) { top_ = val; }
  inline void set_right(double val) { right_ = val; }
  inline void set_bottom(double val) { bottom_ = val; }
  inline void set_width(double val) { right_ = left_ + val; }
  inline void set_height(double val) { bottom_ = top_ + val; }
  inline double left() const { return left_; }
  inline double top() const { return top_; }
  inline double right() const { return right_; }
  inline double bottom() const { return bottom_; }
  inline double width() const { return right_ - left_; }
  inline double height() const { return bottom_ - top_; }
  inline ui::Size size() const { return Size(width(), height()); }
  inline bool empty() const { return left_ == 0 && top_ == 0 && right_ == 0 && bottom_ == 0; }
  
 private:
  double left_, top_, right_, bottom_;
};

enum CursorStyle {
  AUTO, MOVE, NO_DROP, COL_RESIZE, ALL_SCROLL, POINTER, NOT_ALLOWED,
  ROW_RESIZE, CROSSHAIR, PROGRESS, E_RESIZE, NE_RESIZE, DEFAULT, TEXT,
  N_RESIZE, NW_RESIZE, HELP, VERTICAL_TEXT, S_RESIZE, SE_RESIZE, INHERIT,
  WAIT, W_RESIZE, SW_RESIZE
};

struct FontInfo {
  std::string name;
  int height;
};

class Event {
 public:  
  Event() : work_parent_(false), prevent_default_(false) {}
  virtual ~Event() {}

  void WorkParent() { work_parent_ = true; }
  void StopWorkParent() { work_parent_ = false; }
  bool is_work_parent() const { return work_parent_; }
  void PreventDefault() { prevent_default_ = true; }
  bool is_prevent_default() const { return prevent_default_; }
  
 private:  
  bool work_parent_;
  bool prevent_default_;
};

class Window;

class MouseEvent : public Event {
  public:
   MouseEvent(int cx, int cy, int button, unsigned int shift)
      : cx_(cx), 
        cy_(cy), 
        button_(button), 
        shift_(shift) {
   }

  double cx() const { return cx_; }
  double cy() const { return cy_; }
  int button() const { return button_; }
  int shift() const { return shift_; }

 private:  
  double cx_, cy_;
  int button_, shift_;
};

class KeyEvent : public Event {
 public:
  KeyEvent(int keycode, int flags) : keycode_(keycode), flags_(flags) {}

  int keycode() const { return keycode_; }
  int flags() const { return flags_; }
  bool shiftkey() const { return (MK_SHIFT & flags_) == MK_SHIFT; }
  bool ctrlkey() const { return (MK_CONTROL & flags_) == MK_CONTROL; }    

 private:
   int keycode_, flags_;
};

class Font {
 public:
  Font() {}
  virtual ~Font() = 0;
  virtual Font* Clone() const = 0;

  virtual void set_info(const FontInfo& info) = 0;
  virtual FontInfo info() const = 0;
};

inline Font::~Font() {}

class Region {
 public:
  Region() : w_cache_(-1), h_cache_(-1) {}
  Region(int x, int y, int width, int height) {}
  virtual ~Region() = 0;
  virtual Region* Clone() const = 0;

  virtual void Offset(double dx, double dy) = 0;
  virtual int Combine(const Region& other, int combinemode) = 0;  
  virtual ui::Rect bounds() const = 0;
  virtual bool Intersect(double x, double y) const = 0;
  virtual bool IntersectRect(double x, double y, double width, double height) const = 0;
  virtual void Clear() = 0;
  virtual void SetRect(double x, double y, double width, double height) = 0;

  virtual void* source() = 0;
  virtual const void* source() const = 0;

  double w_cache_, h_cache_;
private:
  Region& operator = (const Region& other) {/* do nothing */}
  Region(const Region& other) {/* do nothing */}
};

inline Region::~Region() {}

class Image {
 public:
  typedef boost::shared_ptr<Image> Ptr;
  typedef boost::weak_ptr<Image> WeakPtr;

  Image() { }
  virtual ~Image() = 0;

  virtual void Load(const std::string& filename) = 0;
  virtual void SetTransparent(bool on, ARGB color) = 0;
  virtual void size(double& width, double& height) const = 0;  
  virtual void* source() = 0;
  virtual void* mask() = 0;
  virtual const void* mask() const = 0;
};

inline Image::~Image() { }

namespace canvas { class Group; }

class Graphics {
friend class canvas::Group;
 public:
  Graphics() {}
  virtual ~Graphics() = 0;

  virtual void CopyArea(double x, double y, double width, double height, double dx, double dy) = 0;
  virtual void DrawArc(const ui::Rect& rect, const Point& start, const Point& end) = 0;
  virtual void DrawLine(double x1, double y1, double x2, double y2) = 0;
  virtual void DrawRect(const ui::Rect& rect) = 0;
  virtual void DrawRoundRect(double x, double y, double width, double height, double arc_width, double arch_height) = 0;  
  virtual void DrawOval(double x, double y, double width, double height) = 0;
  virtual void DrawString(const std::string& str, double x, double y) = 0;
  virtual void FillRect(double x, double y, double width, double height) = 0;
  virtual void FillRoundRect(double x, double y, double width, double height, double arc_width, double arch_height) = 0;
  virtual void FillOval(double x, double y, double width, double height) = 0;
  virtual void FillRegion(const ui::Region& rgn) = 0;
  virtual void SetColor(ARGB color) = 0;
  virtual ARGB color() const = 0;
  virtual void Translate(double x, double y) = 0;  
  virtual void SetFont(const Font& font) = 0;
  virtual const Font& font() const = 0;
  virtual Size text_size(const std::string& text) const = 0;
  virtual void DrawPolygon(const ui::Points& podoubles) = 0;
  virtual void FillPolygon(const ui::Points& podoubles) = 0;
  virtual void DrawPolyline(const Points& podoubles) = 0;
  virtual void DrawImage(ui::Image* img, double x, double y) = 0;
  virtual void DrawImage(ui::Image* img, double x, double y, double width, double height) = 0;
  virtual void DrawImage(ui::Image* img, double x, double y, double width, double height, double xsrc, double ysrc) = 0;
  virtual void SetClip(double x, double y, double width, double height)=0;
  virtual void SetClip(ui::Region* rgn)=0;
  virtual CRgn& clip() = 0;
  virtual void Dispose() = 0;

  virtual CDC* dc() = 0;  // just for testing right now
private:
  virtual void SaveOrigin() = 0;
  virtual void RestoreOrigin() = 0;
};

inline Graphics::~Graphics() {}


class Systems {
 public:
  static Systems& instance();
  virtual ~Systems() {}
  virtual ui::Region* CreateRegion() { 
    assert(concrete_factory_);
    return concrete_factory_->CreateRegion(); 
  }
  virtual ui::Graphics* CreateGraphics() { 
    assert(concrete_factory_);
    return concrete_factory_->CreateGraphics(); 
  }
  virtual ui::Graphics* CreateGraphics(void* dc) { 
    assert(concrete_factory_);
    return concrete_factory_->CreateGraphics(dc); 
  }
  virtual ui::Image* CreateImage() { 
    assert(concrete_factory_);
    return concrete_factory_->CreateImage(); 
  }
  virtual ui::Font* CreateFont() { 
    assert(concrete_factory_);
    return concrete_factory_->CreateFont(); 
  }
  void set_concret_factory(Systems& concrete_factory) {
    concrete_factory_ = &concrete_factory;
  }

 protected:
  Systems();
  Systems(Systems const&) {}             
  Systems& operator=(Systems const&) {}

 private:
  Systems* concrete_factory_;
};

enum WindowMsg {
  ONWND,
  SHOW,
  HIDE,
  FOCUS
};

enum AlignStyle {
  ALNONE = 1,
  ALTOP = 2,
  ALLEFT = 4,
  ALRIGHT = 8,
  ALBOTTOM = 16,   
  ALCLIENT = 32,
  ALFIXED = 64
};

class ItemStyle {
 public:
  typedef boost::shared_ptr<ItemStyle> Ptr;
  ItemStyle() : align_(ALNONE) {}

  AlignStyle align() const { return align_; }
  void set_align(AlignStyle align) { align_ = align; }  
  void set_margin(const ui::Rect& margin) { margin_ = margin; }
  const ui::Rect& margin() const { return margin_; }
  void set_padding(const ui::Rect& padding) { padding_ = padding; }
  const ui::Rect& padding() const { return padding_; }
    
 private:
  AlignStyle align_;
  ui::Rect margin_, padding_;
};

class WindowImp;

namespace canvas {
  class Aligner;
}

class Window : public boost::enable_shared_from_this<Window> {
 friend class WindowImp;
 public:  
  typedef boost::shared_ptr<Window> Ptr;  
  typedef boost::shared_ptr<const Window> ConstPtr;
  typedef boost::weak_ptr<Window> WeakPtr;
  typedef boost::weak_ptr<const Window> ConstWeakPtr;  
  typedef std::vector<Window::Ptr> List;
  static Window::Ptr nullpointer;

  Window() {}
  Window(WindowImp* imp) { imp_.reset(imp); }
  virtual ~Window();

  void set_imp(WindowImp* imp) { imp_.reset(imp); }
  WindowImp* imp() { return imp_.get(); };

  // structure   
  typedef Window::List::iterator iterator;
  virtual iterator begin() { return dummy_list_.begin(); }
  virtual iterator end() { return dummy_list_.end(); }
  virtual bool empty() const { return true; }
  virtual int size() const { return 0; }
  virtual Window* root() { return 0; }
  virtual Window* root() const { return 0; }
  virtual bool is_root() const { return 0; }
  virtual void set_parent(const Window::WeakPtr& parent) { parent_ = parent; }
  virtual Window::WeakPtr parent() { return parent_; }
  virtual Window::ConstWeakPtr parent() const { return parent_; }
  virtual Window::List SubItems() { return Window::List(); }

  virtual void Add(const Window::Ptr& item) {}
  virtual void Insert(iterator it, const Window::Ptr& item) {}
  virtual void Remove(const Window::Ptr& item) {}
  virtual void RemoveAll() {}

  virtual void set_pos(const ui::Point& pos) {}
  virtual void set_pos(const ui::Rect& pos);
  virtual ui::Rect pos() const { return ui::Rect(); }
  virtual ui::Rect abs_pos() const { return ui::Rect(); }
  virtual ui::Size dim() const;
  virtual void set_aligner(const boost::shared_ptr<canvas::Aligner>& aligner) {}
  virtual boost::shared_ptr<canvas::Aligner> aligner() const { return dummy_aligner_; }
  virtual void set_style(const ItemStyle::Ptr& style) {}
  virtual ItemStyle::Ptr style() { return ItemStyle::Ptr(); }
  virtual ItemStyle::Ptr style() const { return ItemStyle::Ptr(); }
  virtual bool has_style() const { return false; }  

  virtual void Show();
  virtual void Hide();
  virtual bool visible() const { return true; }
  virtual void Invalidate();
  virtual void Invalidate(Region& rgn);
  virtual void SetCapture();
  virtual void ReleaseCapture();
  virtual void ShowCursor();
  virtual void HideCursor();
  virtual void SetCursorPos(double x, double y);
  virtual void SetCursor(CursorStyle style);  
  virtual void set_parent(Window* window);

  // Regions
  virtual void needsupdate() {}
  virtual const Region& region() const { return *dummy_region_.get(); }
  virtual bool onupdateregion() { needsupdate(); return true; }
  virtual Window::Ptr HitTest(double x, double y) { return nullpointer; }

  // Events
  virtual void OnMessage(WindowMsg msg, int param = 0) {}

  virtual void Draw(Graphics* g, Region& draw_region) {}
  virtual void DrawBackground(Graphics* g, Region& draw_region) {}
  virtual void OnSize(double width, double height) {}

  virtual void OnMouseDown(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnMouseUp(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnDblclick(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnMouseMove(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnMouseEnter(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnMouseOut(MouseEvent& ev) { ev.WorkParent(); }
  boost::signal<void (MouseEvent&)> MouseDown;
  boost::signal<void (MouseEvent&)> MouseUp;
  boost::signal<void (MouseEvent&)> DblClick;
  boost::signal<void (MouseEvent&)> MouseMove;
  boost::signal<void (MouseEvent&)> MouseEnter;
  boost::signal<void (MouseEvent&)> MouseOut;
  // Key Events
  virtual void OnKeyDown(KeyEvent& ev) { ev.WorkParent(); }
  virtual void OnKeyUp(KeyEvent& ev) { ev.WorkParent(); }
  boost::signal<void (KeyEvent&)> KeyDown;
  boost::signal<void (KeyEvent&)> KeyUp;
  // Focus Events
  virtual void OnFocus() {}
  virtual void OnKillFocus() {}
  boost::signal<void ()> Focus;
  boost::signal<void ()> KillFocus;

  // for inner window mfc controls
  virtual void OnFocusChange(int id) {}
 protected:  
  virtual void WorkMouseDown(MouseEvent& ev) { OnMouseDown(ev); }
  virtual void WorkMouseUp(MouseEvent& ev) { OnMouseUp(ev); }
  virtual void WorkMouseMove(MouseEvent& ev) { OnMouseMove(ev); }
  virtual void WorkDblClick(MouseEvent& ev) { OnDblclick(ev); }
  virtual void WorkKeyUp(KeyEvent& ev) { OnKeyUp(ev); }
  virtual bool WorkKeyDown(KeyEvent& ev) { OnKeyDown(ev); return true; }

  std::auto_ptr<WindowImp> imp_;
  Window::WeakPtr parent_;
  static List dummy_list_;
  static ui::Rect dummy_pos_;
  boost::shared_ptr<canvas::Aligner> dummy_aligner_;
  static boost::shared_ptr<ui::Region> dummy_region_;
};

class Ornament {
 public:
  Ornament() {}
  virtual ~Ornament() = 0;

  virtual void Draw(Window::Ptr& item, Graphics* g, Region& draw_region) = 0;
  virtual std::auto_ptr<ui::Rect> padding() const { return std::auto_ptr<ui::Rect>(); }
};

inline Ornament::~Ornament() {}

class WindowImp {
 public:
  WindowImp() : window_(0) {}
  WindowImp(Window* window) : window_(window) {}
  virtual ~WindowImp() {}

  void set_window(Window* window) { window_ = window; }
  Window* window() { return window_; }
  Window* window() const { return window_; }

  virtual void dev_set_pos(const ui::Rect& pos) = 0;
  virtual ui::Size dev_dim() const = 0;
  virtual void DevShow() = 0;
  virtual void DevHide() = 0;
  virtual void DevInvalidate() = 0;
  virtual void DevInvalidate(Region& rgn) = 0;
  virtual void DevSetCapture() = 0;
  virtual void DevReleaseCapture() = 0;
  virtual void DevShowCursor() = 0;
  virtual void DevHideCursor() = 0;
  virtual void DevSetCursorPos(double x, double y) = 0;
  virtual void DevSetCursor(CursorStyle style) {}
  virtual void DevDestroy() {}
  virtual void dev_set_parent(Window* window) {}

  // Events
  virtual void OnDevDraw(Graphics* g, Region& draw_region) {
    if (window_) {
      window_->Draw(g, draw_region);
    }
  }

  virtual void OnDevSize(double width, double height) {
    if (window_) {
      window_->OnSize(width, height);
    }
  }
  virtual void OnDevMouseDown(MouseEvent& ev) {
    if (window_) {
      window_->WorkMouseDown(ev);
    }
  }
  virtual void OnDevMouseUp(MouseEvent& ev) {
    if (window_) {
      window_->WorkMouseUp(ev);
    }
  }
  virtual void OnDevDblclick(MouseEvent& ev) {
    if (window_) {
      window_->WorkDblClick(ev);
    }
  }
  virtual void OnDevMouseMove(MouseEvent& ev) {
    if (window_) {
      window_->WorkMouseMove(ev);
    }
  }  

  // Key Events
  virtual void OnDevKeyDown(KeyEvent& ev) { 
    if (window_) {
      window_->WorkKeyDown(ev);
    }
  }
  virtual void OnDevKeyUp(KeyEvent& ev) {
    if (window_) {
      window_->WorkKeyDown(ev);
    }
  }

  virtual void OnDevFocusChange(int id) {
    if (window_) {
      window_->OnFocusChange(id);
    }
  }

 private:
  Window* window_;
};


} // namespace ui
} // namespace host
} // namespace psycle


