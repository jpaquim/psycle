// This source is free software ; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation ; either version 2, or (at your option) any later 
// version.
// copyright 2015-2016 members of the psycle project 
// http://psycle.sourceforge.net

#pragma once

#include "LockIF.hpp"
#define BOOST_SIGNALS_NO_DEPRECATION_WARNING
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/ptr_container/ptr_list.hpp>

#include <bitset>
#include <stack>

#undef GetGValue
#define GetGValue(rgb) (LOBYTE((rgb) >> 8))
#define ToCOLORREF(argb) RGB((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, (argb >> 0) & 0xFF)
#define GetAlpha(argb) (argb>>24) & 0xFF
#define RGBToARGB(r, g, b) (((ARGB) (b) << 0) | ((ARGB) (g) << 8) |((ARGB) (r) << 16) | ((ARGB) (0) << 24))
#define ToARGB(rgb) RGBToARGB(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb))

namespace psycle {
namespace host {

class LockIF;

namespace ui {

typedef uint32_t ARGB;
typedef uint32_t RGB;

extern class {
 public:
  template<typename T>
  operator boost::shared_ptr<T>() { return boost::shared_ptr<T>(); }
  template<typename T>
  operator boost::weak_ptr<T>() { return boost::weak_ptr<T>(); }
} nullpointer;


struct Point {
  Point() : x_(0), y_(0) {}
  Point(double x, double y) : x_(x), y_(y) {}

  inline bool operator==(const Point& rhs) const { 
    return x_ == rhs.x()  && y_ == rhs.y();
  }
  inline bool operator!=(const Point& rhs) const { return !(*this == rhs); }
  inline Point operator+(const Point& rhs) const { 
    return Point(x_ + rhs.x(), y_ + rhs.y());
  }
  inline Point operator-(const Point& rhs) const {
    return Point(x_ - rhs.x(), y_ - rhs.y());
  }
  Point& operator+=(const Point& rhs) {
    x_ += rhs.x();
    y_ += rhs.y();
    return *this;
  }
  Point& operator-=(const Point& rhs) {
    x_ -= rhs.x();
    y_ -= rhs.y();
    return *this;
  }
  void set_xy(double x, double y) { 
    x_ = x; 
    y_ = y;    
  }
  void set_x(double x) { x_ = x; }  
  double x() const { return x_; }
  void set_y(double y) { y_ = y; }
  double y() const { return y_; }

 private:
  double x_, y_;
};

typedef std::vector<Point> Points;

struct Dimension {
  Dimension() : width_(0), height_(0) {}
  Dimension(double val) : width_(val), height_(val) {}
  Dimension(double width, double height) : width_(width), height_(height) {}

  inline bool operator==(const Dimension& rhs) const { 
    return width_ == rhs.width()  && height_ == rhs.height();
  }
  inline bool operator!=(const Dimension& rhs) const { 
    return !(*this == rhs);
  }
  inline Dimension operator+(const Dimension& rhs) const { 
    return Dimension(width_ + rhs.width(), height_ + rhs.height());
  }
  inline Dimension operator-(const Dimension& rhs) const {
    return Dimension(width_ - rhs.width(), height_ - rhs.height());
  }
  inline Dimension operator/(const Dimension& rhs) const {
    return Dimension(width_/rhs.width(), height_/rhs.height());
  }
  inline Dimension operator*(const Dimension& rhs) const {
    return Dimension(width_*rhs.width(), height_*rhs.height());
  }
  Dimension& operator+=(const Dimension& rhs) {
    width_ += rhs.width();
    height_ += rhs.height();
    return *this;
  }
  Dimension& operator-=(const Dimension& rhs) {
    width_ -= rhs.width();
    height_ -= rhs.height();
    return *this;
  }
  Dimension& operator*=(const Dimension& rhs) {
    width_ *= rhs.width();
    height_ *= rhs.height();
    return *this;
  }
  Dimension& operator/=(const Dimension& rhs) {
    width_ /= rhs.width();
    height_ /= rhs.height();
    return *this;
  } 
  void set(double width, double height) { 
    width_ = width;
    height_ = height;
  }
  void set_width(double width) { width_ = width; }
  void set_height(double height) { height_ = height; }
  double width() const { return width_; }
  double height() const { return height_; }

  ui::Point as_point() const { return ui::Point(width_, height_); }
 private:
  double width_, height_;
};

class Region;

struct Rect {
  Rect() {}  
  Rect(const ui::Point& top_left, const ui::Point& bottom_right) :
     top_left_(top_left),
     bottom_right_(bottom_right) {
  }
  Rect(const ui::Point& top_left, const ui::Dimension& dim) : 
     top_left_(top_left),
     bottom_right_(top_left.x() + dim.width(), top_left.y() + dim.height()) {
  }  

  inline bool operator==(const Rect& rhs) const { 
    return left() == rhs.left() && 
           top() == rhs.top() && 
           right() == rhs.right() && 
           bottom() == rhs.bottom();
  }
  inline bool operator!=(const Rect& rhs) const { return !(*this == rhs); }
    
  inline void set(const ui::Point& top_left, const ui::Point& bottom_right) {
    top_left_ = top_left;
    bottom_right_ = bottom_right;
  }

  inline void set(const ui::Point& top_left, const ui::Dimension& dim) {
    top_left_ = top_left;
    bottom_right_.set_xy(top_left.x() + dim.width(), top_left.y() + dim.height());
  }
  
  inline void set_left(double left) { top_left_.set_x(left); }
  inline void set_top(double top) { top_left_.set_y(top); }
  inline void set_right(double right) { bottom_right_.set_x(right); }
  inline void set_bottom(double bottom) { bottom_right_.set_y(bottom); }
  inline void set_width(double width) {
    bottom_right_.set_x(top_left_.x() + width);
  }
  inline void set_height(double height) {
    bottom_right_.set_y(top_left_.y() + height);
  }
  inline double left() const { return top_left_.x(); }
  inline double top() const { return top_left_.y(); }
  inline double right() const { return bottom_right_.x(); }
  inline double bottom() const { return bottom_right_.y(); }
  inline double width() const { return bottom_right_.x() - top_left_.x(); }
  inline double height() const { return bottom_right_.y() - top_left_.y(); }
  inline const ui::Point& top_left() const { return top_left_; }
  inline const ui::Point top_right() const { return ui::Point(right(), top()); }
  inline const ui::Point& bottom_right() const { return bottom_right_; }
  inline const ui::Point bottom_left() const { return ui::Point(left(), bottom()); }  
  inline void set_dimension(const ui::Dimension& dimension) {
    set_width(dimension.width());
    set_height(dimension.height());
  }
  inline ui::Dimension dimension() const { 
    return Dimension(width(), height());
  }
  inline bool empty() const { 
    return top_left_.x() == 0 && top_left_.y() == 0 && 
           bottom_right_.x() == 0 && bottom_right_.y() == 0;
  }
  
  inline void Offset(double dx, double dy) {
    top_left_.set_x(top_left_.x() + dx);
    top_left_.set_y(top_left_.y() + dy);
    bottom_right_.set_x(bottom_right_.x() + dx);
    bottom_right_.set_y(bottom_right_.y() + dy);    
  }

  inline void Offset(const ui::Point& delta) { Offset(delta.x(), delta.y()); }

  std::auto_ptr<ui::Region> region() const;

  std::string str() const {
    std::stringstream str;
    str << top_left_.x() << " " << top_left_.y() << " " << bottom_right_.x() << " " << bottom_right_.y();
    return str.str();
  }
  
 private:
  ui::Point top_left_, bottom_right_;  
};

class SystemMetrics {
 public:    
  SystemMetrics() {}  
  virtual ~SystemMetrics() = 0;

  virtual ui::Dimension screen_dimension() const = 0;
  virtual ui::Dimension scrollbar_size() const = 0;
};

inline SystemMetrics::~SystemMetrics() {}

class RegionImp;

class Region {
 public:  
  Region();
  Region(ui::RegionImp* imp);
  Region(const ui::Rect& rect);
  Region& operator = (const Region& other);
  Region(const Region& other);
  virtual ~Region() {}
  
  ui::RegionImp* imp() { return imp_.get(); }
  ui::RegionImp* imp() const { return imp_.get(); }

  virtual Region* Clone() const;
  virtual void Offset(double dx, double dy);
  virtual int Combine(const Region& other, int combinemode);
  virtual ui::Rect bounds() const;
  virtual bool Intersect(double x, double y) const;
  virtual bool IntersectRect(const ui::Rect& rect) const;
  virtual void Clear();
  virtual void SetRect(const ui::Rect& rect);  
  
 private:  
  std::auto_ptr<RegionImp> imp_;
};

struct Shape {
  Shape() {}
  virtual ~Shape() {}

  virtual void Offset(double dx, double dy) = 0;
  virtual ui::Rect bounds() const = 0;
};

struct RectShape : public Shape {
  RectShape() {}
  RectShape(const ui::Rect& rect) : rect_(rect) {}
  virtual ~RectShape() {}
  virtual void Offset(double dx, double dy) { rect_.Offset(dx, dy); }
  void SetRect(const ui::Rect& rect) { rect_ = rect; }
  virtual ui::Rect bounds() const{ return rect_; }

 private:
  ui::Rect rect_;
};

struct Area {
  Area();
  Area(const Rect& rect);
  Area* Clone() const;
  ~Area();
  
  void Add(const RectShape& area);
  void Clear();
  void Offset(double dx, double dy);
  int Combine(const Area& other, int combinemode);
  bool Intersect(double x, double y) const;
  const ui::Rect& bounds() const;
  const ui::Region& region() const;

  int size() const { return rect_shapes_.size(); }
 private:
  void Update() const {
    if (needs_update_) {
      ComputeBounds();
      ComputeRegion();
      needs_update_ = false;
    }
  }
  void ComputeBounds() const;
  void ComputeRegion() const;
  typedef std::vector<RectShape>::iterator rect_iterator;
  typedef std::vector<RectShape>::const_iterator rect_const_iterator;
  std::vector<RectShape> rect_shapes_; 
  volatile mutable bool needs_update_;   
  mutable ui::Rect bound_cache_;
  mutable ui::Region region_cache_;   
};

enum Orientation { HORZ = 0, VERT = 1 };

enum CursorStyle {
  AUTO, MOVE, NO_DROP, COL_RESIZE, ALL_SCROLL, POINTER, NOT_ALLOWED,
  ROW_RESIZE, CROSSHAIR, PROGRESS, E_RESIZE, NE_RESIZE, DEFAULT, TEXT,
  N_RESIZE, NW_RESIZE, HELP, VERTICAL_TEXT, S_RESIZE, SE_RESIZE, INHERIT,
  WAIT, W_RESIZE, SW_RESIZE
};

class Event {
 public:  
  Event() : work_parent_(false), default_prevented_(false), stop_propagation_(false) {}
  virtual ~Event() {}

  void WorkParent() { work_parent_ = true; }
  void StopWorkParent() { work_parent_ = false; }
  bool is_work_parent() const { return work_parent_; }
  void PreventDefault() { default_prevented_ = true; }
  bool is_default_prevented() const { return default_prevented_; }  
  void StopPropagation() { stop_propagation_ = true; }
  bool is_propagation_stopped() const { return stop_propagation_; }
  
 private:    
  bool work_parent_;
  bool default_prevented_;
  bool stop_propagation_;
};

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

struct FontInfo {
  FontInfo() : name("Arial"), height(12), bold(false) {}
  std::string name;
  int height;
  bool bold;
};

class FontImp;

class Font {
 public:
  Font();
  Font(const FontInfo& font_info);  
  Font(const Font& other);
  Font& operator= (const Font& other);
  virtual ~Font() {}
  
  ui::FontImp* imp() { return imp_.get(); }
  ui::FontImp* imp() const { return imp_.get(); }

  virtual void set_info(const FontInfo& info);
  virtual FontInfo info() const;

 private:  
  std::auto_ptr<FontImp> imp_;
};

class FontImp {
 public:
  FontImp() {}
  virtual ~FontImp() = 0;
  virtual FontImp* DevClone() const = 0;

  virtual void dev_set_info(const FontInfo& info) = 0;
  virtual FontInfo dev_info() const = 0;
};

inline FontImp::~FontImp() {}

class RegionImp {
 public:
  RegionImp() {}
  virtual ~RegionImp() = 0;
  virtual RegionImp* DevClone() const = 0;
  virtual void DevOffset(double dx, double dy) = 0;
  virtual int DevCombine(const Region& other, int combinemode) = 0;  
  virtual ui::Rect DevBounds() const = 0;
  virtual bool DevIntersect(double x, double y) const = 0;
  virtual bool DevIntersectRect(const ui::Rect& rect) const = 0;
  virtual void DevClear() = 0;
  virtual void DevSetRect(const ui::Rect& rect) = 0;  
};

inline RegionImp::~RegionImp() {}

class ImageImp;

class Image {
 public:
  typedef boost::shared_ptr<Image> Ptr;
  typedef boost::weak_ptr<Image> WeakPtr;

  Image();
  virtual ~Image() {};

  virtual void Load(const std::string& filename);
  virtual void SetTransparent(bool on, ARGB color);
  virtual ui::Dimension dim() const;
  virtual void* source();
  virtual void* mask();
  virtual const void* mask() const;

 private:
  std::auto_ptr<ImageImp> imp_;
};

class ImageImp {
 public:  
  ImageImp() {}
  virtual ~ImageImp() = 0;

  virtual void Load(const std::string& filename) = 0;
  virtual void SetTransparent(bool on, ARGB color) = 0;
  virtual ui::Dimension dim() const = 0;
  virtual void* source() = 0;
  virtual void* mask() = 0;
  virtual const void* mask() const = 0;
};

inline ImageImp::~ImageImp() { }

class Images {
 public:  
  typedef boost::shared_ptr<Images> Ptr;
  typedef boost::shared_ptr<const Images> ConstPtr;
  typedef boost::weak_ptr<Images> WeakPtr;
  typedef boost::weak_ptr<const Images> ConstWeakPtr;
  typedef std::vector<ui::Image::Ptr> Container;
  typedef Images::Container::iterator iterator;

  virtual iterator begin() { return images_.begin(); }
  virtual iterator end() { return images_.end(); }
  virtual bool empty() const { return images_.empty(); }
  virtual int size() const { return images_.size(); }
  
  Images() {}
  virtual ~Images() {}
    
  void Add(const Image::Ptr& image) { images_.push_back(image); }
  void Insert(iterator it, const Image::Ptr& item) { 
    images_.insert(it, item);
  }
  void Remove(iterator it) { images_.erase(it); }
  void RemoveAll() { images_.clear(); }

  private:
   Images::Container images_;
};

namespace canvas { class Group; }

class GraphicsImp;

class Graphics {
friend class canvas::Group;
 public:
  typedef boost::shared_ptr<Graphics> Ptr;
  typedef boost::shared_ptr<const Graphics> ConstPtr;
  typedef boost::weak_ptr<Graphics> WeakPtr;
  typedef boost::weak_ptr<const Graphics> ConstWeakPtr;
  Graphics(); 
  Graphics(CDC* cr);
  virtual ~Graphics() {}
  
  GraphicsImp* imp() { return imp_.get(); };
  GraphicsImp* imp() const { return imp_.get(); };

  virtual void CopyArea(const ui::Rect& rect, const ui::Point& delta);
  virtual void DrawArc(const ui::Rect& rect, const Point& start, const Point& end);
  virtual void DrawLine(const ui::Point& p1, const ui::Point& p2);
  virtual void DrawRect(const ui::Rect& rect);
  virtual void DrawRoundRect(const ui::Rect& rect, const ui::Dimension& arc_dim);
  virtual void DrawOval(const ui::Rect& rect);
  virtual void DrawString(const std::string& str, const ui::Point& point);
  virtual void FillRect(const ui::Rect& rect);
  virtual void FillRoundRect(const ui::Rect& rect, const ui::Dimension& arc_dim);
  virtual void FillOval(const ui::Rect& rect);
  virtual void FillRegion(const ui::Region& rgn);
  virtual void SetColor(ARGB color);
  virtual ARGB color() const;
  virtual void Translate(double x, double y);
  virtual void SetFont(const Font& font);
  virtual const Font& font() const;
  virtual Dimension text_size(const std::string& text) const;
  virtual void DrawPolygon(const ui::Points& podoubles);
  virtual void FillPolygon(const ui::Points& podoubles);
  virtual void DrawPolyline(const Points& podoubles);
  virtual void DrawImage(ui::Image* img, double x, double y);
  virtual void DrawImage(ui::Image* img, double x, double y, double width, double height);
  virtual void DrawImage(ui::Image* img, double x, double y, double width, double height, double xsrc, double ysrc);
  virtual void SetClip(const ui::Rect& rect);
  virtual void SetClip(ui::Region* rgn);
  virtual CRgn& clip();
  virtual void Dispose();
  virtual CDC* dc();  // just for testing right now

 private:
  std::auto_ptr<GraphicsImp> imp_;
  virtual void SaveOrigin();
  virtual void RestoreOrigin();
};

enum WindowMsg { ONWND, SHOW, HIDE, FOCUS };

enum AlignStyle {
  ALNONE = 1,
  ALTOP,
  ALLEFT,
  ALRIGHT,
  ALBOTTOM,   
  ALCLIENT, 
  ALCENTER
};

enum JustifyStyle {
  LEFTJUSTIFY,
  RIGHTJUSTIFY,
  CENTERJUSTIFY
};

class WindowImp;
class Ornament;
class Aligner;

class Commands {
 public:
   Commands();
   ~Commands() {}

   void test() {
     struct {
       void operator()() const
        {
            //some code
        }
     } f;
     functors.push_back(f);     
   }

   template<typename T>
   void Add(T& f) {     
     functors.push_back(f);     
   }
   void Clear();
   void Invoke();

   std::list<boost::function<void(void)>> functors;   
};


class Window;

class WindowShowStrategy {
  public:
   WindowShowStrategy();
   virtual ~WindowShowStrategy() = 0;
   virtual void set_position(Window& window) = 0;
};

inline WindowShowStrategy::~WindowShowStrategy() {}

class ChildPosEvent;

class Window : public boost::enable_shared_from_this<Window> {
 friend class WindowImp;
 public:  
  typedef boost::shared_ptr<Window> Ptr;
  typedef boost::shared_ptr<const Window> ConstPtr;
  typedef boost::weak_ptr<Window> WeakPtr;
  typedef boost::weak_ptr<const Window> ConstWeakPtr;
  typedef std::vector<Window::Ptr> Container;  
  typedef Window::Container::iterator iterator;

  Window();
  Window(WindowImp* imp);
  virtual ~Window();

  void set_imp(WindowImp* imp);
  virtual void release_imp() { imp_.release(); }
  WindowImp* imp() { return imp_.get(); };
  WindowImp* imp() const { return imp_.get(); };

  void set_debug_text(const std::string& text) { debug_text_ = text; }
  const std::string& debug_text() const { return debug_text_; }  

  static std::string type() { return "canvasitem"; }
  virtual std::string GetType() const { return "window"; }
   
  virtual iterator begin() { return dummy_list_.begin(); }
  virtual iterator end() { return dummy_list_.end(); }
  virtual bool empty() const { return true; }
  virtual int size() const { return 0; }  
  bool has_childs() const { return size() != 0; }
  virtual Window* root();
  virtual const Window* root() const;
  virtual bool is_root() const { return 0; }
  virtual bool IsInGroup(Window::WeakPtr group) const;
  virtual bool IsInGroupVisible() const;
  virtual void set_parent(Window* parent);
  virtual Window* parent() { return parent_; }
  virtual Window* parent() const { return parent_; }  
  template <class T, class T1>
  void PreOrderTreeTraverse(T& functor, T1& cond);
  template <class T, class T1>
  void PostOrderTreeTraverse(T& functor, T1& cond);
  Window::Container SubItems();
    
  virtual void Add(const Window::Ptr& item) {}
  virtual void Insert(iterator it, const Window::Ptr& item) {}
  virtual void Remove(const Window::Ptr& item) {}
  virtual void RemoveAll() {}
    
  virtual void set_pos(const ui::Point& pos);
  virtual void set_pos(const ui::Rect& pos);
  virtual void ScrollTo(int offsetx, int offsety);
  virtual ui::Rect pos() const;
  virtual ui::Rect abs_pos() const;
  virtual ui::Rect desktop_pos() const;
  virtual ui::Dimension dim() const;  
   
  virtual void set_aligner(const boost::shared_ptr<Aligner>& aligner) {}
  virtual boost::shared_ptr<Aligner> aligner() const { return nullpointer; }  
  void set_align(AlignStyle align) { align_ = align; }
  AlignStyle align() const { return align_; }
  void set_margin(const ui::Rect& margin) { margin_ = margin; }
  const ui::Rect& margin() const { return margin_; }
  void set_padding(const ui::Rect& padding) { padding_ = padding; }
  const ui::Rect& padding() const { return padding_; }
  
  virtual void Show();
  virtual void Show(const boost::shared_ptr<WindowShowStrategy>& aligner);
  virtual void Hide();
  virtual bool visible() const { return visible_; }  
  virtual void FLS(); // invalidate new region
  virtual void FLSEX(); //  invalidate combine new & old region
  virtual void FLS(const Region& rgn) {} // invalidate region  
  virtual void Invalidate();
  virtual void Invalidate(const Region& rgn);
  virtual void PreventFls();
  virtual void EnableFls();
  virtual bool is_fls_prevented() const;
  virtual void SetCapture();
  virtual void ReleaseCapture();
  virtual void ShowCursor();
  virtual void HideCursor();
  virtual void SetCursorPos(double x, double y);
  virtual void SetCursor(CursorStyle style);  
  CursorStyle cursor() const { return DEFAULT; }  
  
  virtual void needsupdate();
  virtual const Area& area() const;
  virtual std::auto_ptr<Region> draw_region() { return std::auto_ptr<Region>(); }
  virtual bool OnUpdateArea();
  virtual Window::Ptr HitTest(double x, double y) { 
    return area().Intersect(x, y) ? shared_from_this() : nullpointer;    
  }  
  virtual void set_auto_size(bool auto_size_width, bool auto_size_height) {
    auto_size_width_ = auto_size_width;
    auto_size_height_ = auto_size_height;
  }
  virtual bool auto_size_width() const;
  virtual bool auto_size_height() const;
  virtual void SetClip(const ui::Rect& rect) {}
  virtual bool has_clip() const { return false; }
  const Region& clip() const;
  virtual void RemoveClip() {}
  
  virtual void Draw(Graphics* g, Region& draw_region) {}
  virtual void DrawBackground(Graphics* g, Region& draw_region);
  virtual void set_ornament(boost::shared_ptr<ui::Ornament> ornament);
  virtual boost::weak_ptr<ui::Ornament> ornament();
  void set_clip_children();
  void add_style(UINT flag);
  void remove_style(UINT flag);

  virtual void OnMessage(WindowMsg msg, int param = 0) {}
  virtual void OnSize(const ui::Dimension& dimension) {}
  virtual void OnChildPos(ChildPosEvent& ev) {}
  
  virtual void EnablePointerEvents() { pointer_events_ = true; }
  virtual void DisablePointerEvents() { pointer_events_ = false; }
  virtual bool pointerevents() const { return pointer_events_; }
  virtual void OnMouseDown(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnMouseUp(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnDblclick(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnMouseMove(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnMouseEnter(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnMouseOut(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnShow() {}
  boost::signal<void (MouseEvent&)> MouseDown;
  boost::signal<void (MouseEvent&)> MouseUp;
  boost::signal<void (MouseEvent&)> DblClick;
  boost::signal<void (MouseEvent&)> MouseMove;
  boost::signal<void (MouseEvent&)> MouseEnter;
  boost::signal<void (MouseEvent&)> MouseOut;
  
  virtual void OnKeyDown(KeyEvent& ev) { ev.WorkParent(); }
  virtual void OnKeyUp(KeyEvent& ev) { ev.WorkParent(); }
  boost::signal<void (KeyEvent&)> KeyDown;
  boost::signal<void (KeyEvent&)> KeyUp;
  
  Window::WeakPtr focus();
  virtual void SetFocus();  
  virtual void OnFocus(Event& ev) { ev.WorkParent(); }
  virtual void OnKillFocus() {}
  boost::signal<void ()> Focus;
  boost::signal<void ()> KillFocus;
  ui::Rect pos_old_;  

  void lock() const;   
  void unlock() const;
  virtual void Enable();
  virtual void Disable();
  
 protected:  
  virtual void WorkMouseDown(MouseEvent& ev) { OnMouseDown(ev); }
  virtual void WorkMouseUp(MouseEvent& ev) { OnMouseUp(ev); }
  virtual void WorkMouseMove(MouseEvent& ev) { OnMouseMove(ev); }
  virtual void WorkMouseLeave(MouseEvent& ev) { OnMouseOut(ev); }
  virtual void WorkDblClick(MouseEvent& ev) { OnDblclick(ev); }
  virtual void WorkKeyUp(KeyEvent& ev) { OnKeyUp(ev); }
  virtual bool WorkKeyDown(KeyEvent& ev) { OnKeyDown(ev); return true; }  
  virtual void WorkFocus(Event& ev) { OnFocus(ev); }
  void WorkChildPos();
  
  mutable bool update_;
  mutable std::auto_ptr<Area> area_;  
  mutable std::auto_ptr<Area> fls_area_;
 private:
  std::auto_ptr<WindowImp> imp_;
  Window* parent_;  
  static Container dummy_list_;  
  std::string debug_text_;  
  bool auto_size_width_, auto_size_height_;
  boost::weak_ptr<Ornament> ornament_;  
  bool visible_, pointer_events_;  
  AlignStyle align_;
  ui::Rect margin_, padding_;  
  boost::weak_ptr<Window> root_cache_;
};

class ChildPosEvent : public Event {
 public:  
  ChildPosEvent(ui::Window::Ptr window) : window_(window) {}

  Window::Ptr window() { return !window_.expired() ? window_.lock() : nullpointer; }

 private:
  Window::WeakPtr window_;
};

class Group : public Window {
 public:  
  static std::string type() { return "canvasgroup"; }

  Group();  
  Group(WindowImp* imp);
  
  // structure  
  virtual Window::iterator begin() { return items_.begin(); }
  virtual Window::iterator end() { return items_.end(); }
  virtual bool empty() const { return items_.empty(); }
  virtual int size() const { return items_.size(); }
  
  void Add(const Window::Ptr& window);
  void Insert(iterator it, const Window::Ptr& item);
  void Remove(const Window::Ptr& item);
  void RemoveAll() {    
    items_.clear(); 
    FLS();
  }
    
  // appearence
  void set_aligner(const boost::shared_ptr<Aligner>& aligner); 
  virtual Window::Ptr HitTest(double x, double y);
  void RaiseToTop(Window::Ptr item) {  Remove(item); Add(item); }
  void set_zorder(Window::Ptr item, int z);
  int zorder(Window::Ptr item) const;  
  virtual bool OnUpdateArea();  
  virtual void OnMessage(WindowMsg msg, int param = 0);    
  void UpdateAlign();
  void FlagNotAligned();
  boost::shared_ptr<Aligner> aligner() const { return aligner_; }  
  virtual void OnChildPos(ChildPosEvent& ev);
  
 protected:  
  Window::Container items_;

 private:
  void Init();  
  boost::shared_ptr<Aligner> aligner_;
};

struct CalcDim { void operator()(Window& window) const; };
struct SetPos { bool operator()(Window& window) const; };
struct Visible { bool operator()(Window& window) const; };
struct AbortNone { bool operator()(Window& window) const { return false; }};
struct AbortDefault { bool operator()(Window& window) const { return !window.visible(); } };
struct AbortPos { bool operator()(Window& window) const; };
struct SetUnaligned { bool operator()(Window& window) const; };

class Aligner { 
 public:
  typedef boost::shared_ptr<Aligner> Ptr;
  typedef boost::weak_ptr<const Aligner> ConstPtr;
  typedef boost::weak_ptr<Aligner> WeakPtr;  
  
  Aligner();
  virtual ~Aligner() = 0;

  void Align() {
    static CalcDim calc_dim;
    static AbortPos pos_abort;
    static SetPos set_pos;    
    static AbortDefault abort;
        
    group_.lock()->PostOrderTreeTraverse(calc_dim, abort);
    group_.lock()->PreOrderTreeTraverse(set_pos, pos_abort);
    //full_align_ = false;
  }

  virtual void CalcDimensions() = 0;
  virtual void SetPositions() = 0;

  virtual void set_group(const ui::Group::WeakPtr& group) { group_ = group; }  
  const ui::Dimension& dim() const { return dim_; }
  const ui::Rect& pos() const { return pos_; }   

  bool aligned() const { return aligned_; }
  bool full_align() const { return true; }

  void CachePos(const ui::Rect& pos) { pos_ = pos; }
  
  ui::Group::WeakPtr group_;
  bool aligned_;  

 protected:  
  typedef Window::Container::iterator iterator;
  iterator begin() { 
    return !group_.expired() ? group_.lock()->begin() : dummy.begin();
  }  
  iterator end() { 
    return !group_.expired() ? group_.lock()->end() : dummy.end();   
  }
  bool empty() const { 
    return !group_.expired() ? group_.lock()->empty() : false; 
  }
  int size() const { 
    return !group_.expired() ? group_.lock()->size() : 0;
  }  
     
  ui::Dimension dim_;
  ui::Rect pos_;
   
 private: 
  static Window::Container dummy;
  static bool full_align_;
};

inline Aligner::~Aligner() {};

class FrameImp;

class WindowCenterToScreen {
  public:   
   WindowCenterToScreen() : width_perc_(-1), height_perc_(-1) {}   
   virtual ~WindowCenterToScreen() {}
   virtual void set_position(Window& window);

   void SizeToScreen(double width_perc, double height_perc) {
     width_perc_ = width_perc;
     height_perc_ = height_perc;
   }

  private:    
    double width_perc_, height_perc_;
};

class PopupMenu;

class Frame : public Window {
 friend FrameImp;
 public:    
  typedef boost::shared_ptr<Frame> Ptr;
  typedef boost::shared_ptr<const Frame> ConstPtr;
  typedef boost::weak_ptr<Frame> WeakPtr;
  typedef boost::weak_ptr<const Frame> ConstWeakPtr;

  static std::string type() { return "canvasframe"; }

  Frame();
  Frame(FrameImp* imp);

  FrameImp* imp() { return (FrameImp*) Window::imp(); };
  FrameImp* imp() const { return (FrameImp*) Window::imp(); };

  virtual void set_view(ui::Window::Ptr view);
  ui::Window::Ptr view() { return view_.lock(); }
  virtual void set_title(const std::string& title);
  virtual std::string title() const;
  void set_popup_menu(const boost::shared_ptr<PopupMenu>& popup_menu) { 
    popup_menu_ = popup_menu;
  }
  boost::weak_ptr<PopupMenu> popup_menu() { return popup_menu_; }
  virtual void ShowDecoration();
  virtual void HideDecoration();
  virtual void PreventResize();
  virtual void AllowResize();

  virtual void OnClose() {}
  virtual void OnShow() {}  
  virtual void OnContextPopup(ui::Event&, const ui::Point& mouse_point) {}
  virtual void WorkOnContextPopup(ui::Event& ev, const ui::Point& mouse_point);
  
  boost::signal<void (Frame&)> close;

 private:  
  ui::Window::WeakPtr view_;
  boost::weak_ptr<PopupMenu> popup_menu_;  
};

class PopupFrameImp;

class PopupFrame : public Frame {
 public:
   PopupFrame();
   PopupFrame(FrameImp* imp);

   FrameImp* imp() { return (FrameImp*) Window::imp(); };
   FrameImp* imp() const { return (FrameImp*) Window::imp(); };

   static std::string type() { return "canvaspopupframe"; }  
};

class Ornament {
 public:
  typedef boost::shared_ptr<Ornament> Ptr;
  typedef boost::weak_ptr<Ornament> WeakPtr;

  Ornament() {}
  virtual ~Ornament() = 0;
  virtual Ornament* Clone() = 0;

  virtual bool transparent() const { return true; }

  virtual void Draw(Window::Ptr& item, Graphics* g, Region& draw_region) = 0;
  virtual std::auto_ptr<ui::Rect> padding() const { return std::auto_ptr<ui::Rect>(); }
};

inline Ornament::~Ornament() {}

// Controls

class TreeView;

class TreeViewImp;
class ListViewImp;
class Node;
class NodeOwnerImp;

class NodeImp { 
 public: 
  NodeImp() : owner_(0) {}
  virtual ~NodeImp() {}

  virtual int pos() const { return -1; }
  virtual void set_text(const std::string& text) {}
  void set_owner(NodeOwnerImp* owner) { owner_ = owner; }
  NodeOwnerImp* owner() { return owner_; }
  const NodeOwnerImp* owner() const { return owner_; }
  
 private:
  NodeOwnerImp* owner_;
};

class recursive_node_iterator;

class Node : public boost::enable_shared_from_this<Node> {
 public:
  static std::string type() { return "node"; }   
  typedef boost::shared_ptr<Node> Ptr;
  typedef boost::weak_ptr<Node> WeakPtr;
  typedef std::vector<Node::Ptr> Container;
  typedef Container::iterator iterator;
   
  Node::Node() : image_index_(0), selected_image_index_(0) {}
  Node::Node(const std::string& text) : 
      text_(text),
      image_index_(0),
      selected_image_index_(0) {
  }
  virtual ~Node() {}
      
  virtual void set_text(const std::string& text) { 
    text_ = text;
    changed(*this);
  }
  virtual std::string text() const { return text_; }
  virtual void set_image(const Image::WeakPtr& image) { image_ = image; }
  virtual Image::WeakPtr image() { return image_; }

  virtual void set_image_index(int index) { image_index_ = index; }
  virtual int image_index() const { return image_index_; }
  virtual void set_selected_image_index(int index) { selected_image_index_ = index; }
  virtual int selected_image_index() const { return selected_image_index_; }
      
  iterator begin() { return children_.begin(); }
  iterator end() { return children_.end(); }
  bool empty() const { return children_.empty(); }
  int size() const { return children_.size(); }
  Ptr back() const { return children_.back(); }

  recursive_node_iterator recursive_begin();
  recursive_node_iterator recursive_end();
    
  int level() const { return (!parent().expired()) ? parent().lock()->level() + 1 : 0; }    
  void AddImp(NodeImp* imp) { imps.push_back(imp); }
  void erase_imps(NodeOwnerImp* owner);
  void erase_imp(NodeOwnerImp* owner);

  void AddNode(const Node::Ptr& node);
  void insert(iterator it, const Node::Ptr& node);
  void erase(iterator it);
  void clear();

  template<class T>
  void traverse(T& func, Node::Ptr prev_node = nullpointer) {
    func(this->shared_from_this(), prev_node);
    ui::Node::Ptr prev = nullpointer;
    for (iterator it = begin(); it != end(); ++it) {        
      (*it)->traverse(func, prev);
      prev = *it;
    }
  }

  void set_parent(const Node::WeakPtr& parent) { parent_ = parent; }
  Node::WeakPtr parent() const { return parent_; }
  boost::signal<void (Node&)> changed;

  boost::ptr_list<NodeImp> imps;

  NodeImp* imp(NodeOwnerImp& imp);

 private:
  std::string text_;
  ui::Image::WeakPtr image_;
  int image_index_, selected_image_index_;
  Container children_;
  Node::WeakPtr parent_;
};

class recursive_node_iterator {
 friend class Node;
 public:
  recursive_node_iterator(ui::Node::iterator& it) { i.push(it); }    

  bool operator==(const recursive_node_iterator& rhs) const {
    return i.size() == rhs.i.size() && i.top() == rhs.i.top();
  }
  inline bool operator!=(const recursive_node_iterator& rhs) const { 
    return !(*this == rhs);
  }
  ui::Node::Ptr& operator*() { return *(i.top()); }
  recursive_node_iterator& operator++() {    
    Node::Ptr parent = (*i.top())->parent().lock();
    if (i.top() != parent->end()) {
      if ((*i.top())->size() > 0) {        
        i.push((*i.top())->begin());        
      } else {
        ++i.top();        
        while (i.size() > 1 && i.top() == parent->end()) {
          i.pop();
          parent = (*i.top())->parent().lock();
          ++i.top();                    
        }
      }
    } else 
    if (i.size() > 0) {
      i.pop();      
    }
    return *this;
  }
  recursive_node_iterator operator++(int) {
    recursive_node_iterator clone(*this);
    ++(*this);
    return clone;
  }
  int level() const { return i.size(); }

 private:  
  std::stack<ui::Node::iterator> i;  
};

class NodeOwnerImp {
 public:
  virtual ~NodeOwnerImp() {}
  
  virtual void DevUpdate(const Node::Ptr&, Node::Ptr prev_node = nullpointer) = 0;
  virtual void DevErase(Node::Ptr node) = 0;
};

class MenuContainer;

class MenuContainerImp : public NodeOwnerImp {
 public:
  MenuContainerImp() : bar_(0) {}
  virtual ~MenuContainerImp() = 0;
  
  void set_menu_bar(MenuContainer* bar) { bar_ = bar; }
  MenuContainer* menu_bar() { return bar_; }
  
  virtual void DevTrack(const ui::Point& pos) = 0;
  virtual void DevInvalidate() = 0;
   
 private:
  MenuContainer* bar_;
};

inline MenuContainerImp::~MenuContainerImp() {}

class PopupMenu;

class MenuImp : public NodeImp {
  public:
    MenuImp() {}
    virtual ~MenuImp() = 0;

    virtual void dev_set_text(const std::string& text) = 0;
    virtual void dev_set_pos(int pos) = 0;
    virtual int dev_pos() const = 0;
};

inline  MenuImp::~MenuImp() {}

class MenuContainer {
 public:
  MenuContainer();
  MenuContainer(MenuContainerImp* imp);
  virtual ~MenuContainer() {}

  virtual MenuContainerImp* imp() { return imp_.get(); }
  virtual MenuContainerImp* imp() const { return imp_.get(); }

  virtual void Update();
  virtual void Invalidate();
    
  void set_root_node(Node::Ptr& root_node) { 
    if (root_node_.use_count() > 1) {
      root_node_.lock()->erase_imps(imp());
    }
    root_node_ = root_node;  
  }
  Node::WeakPtr root_node() { return root_node_; }

  virtual void OnMenuItemClick(boost::shared_ptr<Node> node) {}

  static int id_counter;

 private:
  std::auto_ptr<ui::MenuContainerImp> imp_;
  Node::WeakPtr root_node_;  
};

class MenuBar : public MenuContainer {
};

class PopupMenu : public MenuContainer {
 public:
  typedef boost::shared_ptr<PopupMenu> Ptr;
  typedef boost::weak_ptr<PopupMenu> WeakPtr;

  PopupMenu();

  virtual void Track(const ui::Point& pos); 
};      

class TreeView : public Window {
 public:
  typedef boost::weak_ptr<TreeView> WeakPtr;
  static std::string type() { return "canvastreeview"; }  

  TreeView();
  TreeView(TreeViewImp* imp);

  TreeViewImp* imp() { return (TreeViewImp*) Window::imp(); };
  TreeViewImp* imp() const { return (TreeViewImp*) Window::imp(); };      
  void UpdateTree();
  void set_root_node(const Node::Ptr& root_node) { root_node_ = root_node; }
  boost::weak_ptr<Node> root_node() { return root_node_; }      
  void set_images(const Images::Ptr& images);
  boost::weak_ptr<Images> images() { return images_; }
  void set_popup_menu(const PopupMenu::Ptr& popup_menu) { 
    popup_menu_ = popup_menu;
  }
  void Clear();      
  virtual void set_background_color(ARGB color);
  virtual ARGB background_color() const;
  virtual void set_text_color(ARGB color);
  virtual ARGB text_color() const;
  virtual void EditNode(const Node::Ptr& node);
  bool is_editing() const;

  void ShowLines();
  void HideLines();
  void ShowButtons();
  void HideButtons();

  virtual void select_node(const Node::Ptr& node);
  virtual boost::weak_ptr<Node> selected();

  virtual void OnChange(const Node::Ptr& node) {}
  virtual void OnRightClick(const Node::Ptr& node) {}
  virtual void OnEditing(const Node::Ptr& node, const std::string& text) {}
  virtual void OnEdited(const Node::Ptr& node, const std::string& text) {}  
  virtual void OnContextPopup(ui::Event&, const ui::Point& mouse_point, const ui::Node::Ptr& node) {}
  virtual void WorkOnContextPopup(ui::Event& ev, const ui::Point& mouse_point, const ui::Node::Ptr& node) {
    OnContextPopup(ev, mouse_point, node);
    if (!ev.is_default_prevented() && !popup_menu_.expired()) {
      popup_menu_.lock()->Track(mouse_point);
    }
  }
 private:     
  Node::WeakPtr root_node_;
  Images::WeakPtr images_;
  PopupMenu::WeakPtr popup_menu_;
};

class ListView : public Window {
 public:  
  static std::string type() { return "canvaslistview"; }  

  typedef boost::shared_ptr<ListView> Ptr;
  typedef boost::shared_ptr<const ListView> ConstPtr;
  typedef boost::weak_ptr<ListView> WeakPtr;
  typedef boost::weak_ptr<const ListView> ConstWeakPtr;

  ListView();
  ListView(ListViewImp* imp);

  ListViewImp* imp() { return (ListViewImp*) Window::imp(); };
  ListViewImp* imp() const { return (ListViewImp*) Window::imp(); };      
  void UpdateList();
  void set_root_node(const Node::Ptr& root_node) { root_node_ = root_node; }
  void AddColumn(const std::string& text, int width);
  boost::weak_ptr<Node> root_node() { return root_node_; }      
  void set_images(const Images::Ptr& images);
  boost::weak_ptr<Images> images() { return images_; }
  void Clear();      
  virtual void set_background_color(ARGB color);
  virtual ARGB background_color() const;
  virtual void set_text_color(ARGB color);
  virtual ARGB text_color() const;
  virtual void EditNode(const Node::Ptr& node);
  bool is_editing() const;

  void ViewList();
  void ViewReport();  
  void ViewIcon();
  void ViewSmallIcon();

  void EnableRowSelect();
  void DisableRowSelect();
  
  virtual void select_node(const Node::Ptr& node);
  virtual boost::weak_ptr<Node> selected();
  virtual std::vector<Node::Ptr> selected_nodes();
  int top_index() const;
  void EnsureVisible(int index);

  virtual void OnChange(const Node::Ptr& node) {}
  virtual void OnRightClick(const Node::Ptr& node) {}
  virtual void OnEditing(const Node::Ptr& node, const std::string& text) {}
  virtual void OnEdited(const Node::Ptr& node, const std::string& text) {}

  boost::signal<void (ListView&, const Node::Ptr& node)> change;

 private:     
  Node::WeakPtr root_node_;
  Images::WeakPtr images_;
};

class ScrollBarImp;

class ScrollBar : public Window {
 public:
  typedef boost::shared_ptr<ScrollBar> Ptr;
  typedef boost::shared_ptr<const ScrollBar> ConstPtr;
  typedef boost::weak_ptr<ScrollBar> WeakPtr;
  typedef boost::weak_ptr<const ScrollBar> ConstWeakPtr;

  static std::string type() { return "canvasscrollbaritem"; }
  ScrollBar();
  ScrollBar(const ui::Orientation& orientation);
  ScrollBar(ScrollBarImp* imp);

  ScrollBarImp* imp() { return (ScrollBarImp*) Window::imp(); };
  ScrollBarImp* imp() const { return (ScrollBarImp*) Window::imp(); };

  virtual void OnScroll(int pos) { scroll(*this); }
  boost::signal<void (ScrollBar&)> scroll;

  void set_scroll_range(int minpos, int maxpos);
  void scroll_range(int& minpos, int& maxpos); 
  void set_scroll_pos(int pos);
  int scroll_pos() const;
  void system_size(int& width, int& height) const;  
};

class ScrollBox : public Group {
 public:   
   ScrollBox();
   static std::string type() { return "canvasscrollbox"; }

   virtual void ScrollBy(double dx, double dy);
   virtual void OnSize(const ui::Dimension& dimension);
   virtual void Add(const Window::Ptr& item) { client_->Add(item); }

  private:
    void Init();
    void OnHScroll(ui::ScrollBar&);
    void OnVScroll(ui::ScrollBar&);
    ui::ScrollBar::Ptr hscrollbar_, vscrollbar_;
    ui::Window::Ptr client_;
    ui::Ornament::Ptr client_background_;
};

class ComboBoxImp;

class ComboBox : public Window {
 public:
  static std::string type() { return "canvascomboboxitem"; }
  
  typedef boost::shared_ptr<ComboBox> Ptr;
  typedef boost::shared_ptr<const ComboBox> ConstPtr;
  typedef boost::weak_ptr<ComboBox> WeakPtr;
  typedef boost::weak_ptr<const ComboBox> ConstWeakPtr;

  ComboBox();
  ComboBox(ComboBoxImp* imp);

  ComboBoxImp* imp() { return (ComboBoxImp*) Window::imp(); };
  ComboBoxImp* imp() const { return (ComboBoxImp*) Window::imp(); };

  virtual void add_item(const std::string& item);
  virtual void set_items(const std::vector<std::string>& itemlist);
  virtual std::vector<std::string> items() const;

  void set_item_index(int index);
  int item_index() const;

  std::string text() const;
  virtual void OnSelect() {}

  boost::signal<void (ComboBox&)> select;
};

class EditImp;

class Edit : public Window {
 public:
  static std::string type() { return "canvaseditboxitem"; }
  Edit();
  Edit(EditImp* imp);

  EditImp* imp() { return (EditImp*) Window::imp(); };
  EditImp* imp() const { return (EditImp*) Window::imp(); };
  virtual void set_text(const std::string& text);
  virtual std::string text() const;  
};

class ButtonImp;

class Button : public Window {
 public:
  static std::string type() { return "canvasbuttonitem"; }

  typedef boost::shared_ptr<Button> Ptr;
  typedef boost::shared_ptr<const Button> ConstPtr;
  typedef boost::weak_ptr<Button> WeakPtr;
  typedef boost::weak_ptr<const Button> ConstWeakPtr;

  Button();
  Button(const std::string& text);
  Button(ButtonImp* imp);

  ButtonImp* imp() { return (ButtonImp*) Window::imp(); };
  ButtonImp* imp() const { return (ButtonImp*) Window::imp(); };
  virtual void set_text(const std::string& text);
  virtual std::string text() const;

  virtual void OnClick() {}

  bool OnUpdateArea();

  boost::signal<void (Button&)> click;
};

class CheckBoxImp;

class CheckBox : public Button {
 public:
  static std::string type() { return "canvascomboboxitem"; }
  
  typedef boost::shared_ptr<CheckBox> Ptr;
  typedef boost::shared_ptr<const CheckBox> ConstPtr;
  typedef boost::weak_ptr<CheckBox> WeakPtr;
  typedef boost::weak_ptr<const CheckBox> ConstWeakPtr;

  CheckBox();
  CheckBox(const std::string& text);
  CheckBox(CheckBoxImp* imp);

  CheckBoxImp* imp() { return (CheckBoxImp*) Window::imp(); };
  CheckBoxImp* imp() const { return (CheckBoxImp*) Window::imp(); };

  virtual void set_background_color(ARGB color);
  virtual bool checked() const;

  void Check();
  void UnCheck();
  
  boost::signal<void (CheckBox&)> click;
};

struct Lexer {
  Lexer() : 
      comment_color_(0), 
      comment_line_color_(0),
      comment_doc_color_(0),
      identifier_color_(0),
      folding_color_(0) {    
  }

  void set_keywords(const std::string& keywords) { keywords_ = keywords; }
  const std::string& keywords() const { return keywords_; }
  void set_comment_color(ARGB color) { comment_color_ = color; }
  ARGB comment_color() const { return comment_color_; }
  void set_comment_line_color(ARGB color) { comment_line_color_ = color; }
  ARGB comment_line_color() const { return comment_line_color_; }
  void set_comment_doc_color(ARGB color) { comment_doc_color_ = color; }
  ARGB comment_doc_color() const { return comment_doc_color_; }
  void set_identifier_color(ARGB color) { identifier_color_ = color; }
  ARGB identifier_color() const { return  identifier_color_; }
  void set_number_color(ARGB color) { number_color_ = color; }
  ARGB number_color() const { return number_color_; }
  void set_word_color(ARGB color) { word_color_ = color; }
  ARGB word_color() const { return word_color_; }
  void set_string_color(ARGB color) { string_color_ = color; }
  ARGB string_color() const { return string_color_; }
  void set_operator_color(ARGB color) { operator_color_ = color; }
  ARGB operator_color() const { return operator_color_; }
  void set_character_code_color(ARGB color) { character_code_color_ = color; }
  ARGB character_code_color() const { return  character_code_color_; }
  void set_preprocessor_color(ARGB color) { preprocessor_color_ = color; }
  ARGB preprocessor_color() const { return  preprocessor_color_; }
  void set_folding_color(ARGB color) { folding_color_ = color; }
  ARGB folding_color() const { return  folding_color_; }
  
private:
  std::string keywords_;
  ARGB comment_color_, comment_line_color_, comment_doc_color_,
       identifier_color_, number_color_, string_color_, word_color_,       
       operator_color_, character_code_color_, preprocessor_color_,
       folding_color_;
};

class ScintillaImp;

class Scintilla : public Window {
 public:
  static std::string type() { return "canvasscintillaitem"; }
  Scintilla();
  Scintilla(ScintillaImp* imp);

  ScintillaImp* imp() { return (ScintillaImp*) Window::imp(); };
  ScintillaImp* imp() const { return (ScintillaImp*) Window::imp(); };
  
  virtual std::string GetType() const { return "scintilla"; }

  int f(int sci_cmd, void* lparam, void* wparam);
  void AddText(const std::string& text);
  void FindText(const std::string& text, int cpmin, int cpmax, int& pos, int& cpselstart, int& cpselend) const;
  void GotoLine(int pos);
  int length() const;
  int selectionstart() const;
  int selectionend() const;
  void SetSel(int cpmin, int cpmax);
  bool has_selection() const;
  void ReplaceSel(const std::string& text);
  void set_find_match_case(bool on);
  void set_find_whole_word(bool on);
  void set_find_regexp(bool on);
  void LoadFile(const std::string& filename);
  void SaveFile(const std::string& filename);
  bool has_file() const;
  void set_lexer(const Lexer& lexer);
  void set_foreground_color(ARGB color);
  ARGB foreground_color() const;
  void set_background_color(ARGB color);
  ARGB background_color() const;
  void set_linenumber_foreground_color(ARGB color);
  ARGB linenumber_foreground_color() const;
  void set_linenumber_background_color(ARGB color);
  ARGB linenumber_background_color() const;  
  void set_margin_background_color(ARGB color);
  ARGB margin_background_color() const;
  void set_sel_foreground_color(ARGB color);
  //ARGB sel_foreground_color() const { return ToARGB(ctrl().sel_foreground_color()); }  
  void set_sel_background_color(ARGB color);
  //ARGB sel_background_color() const { return ToARGB(ctrl().sel_background_color()); }
  void set_sel_alpha(int alpha);
  void set_ident_color(ARGB color);
  void set_caret_color(ARGB color);
  ARGB caret_color() const;
  void StyleClearAll();
  const std::string& filename() const;
  bool is_modified() const;
  virtual void OnFirstModified() {}
  void set_font(const FontInfo& font_info);
  int column() const;    
  int line() const;
  bool ovr_type() const;
  bool modified() const;
  int add_marker(int line, int id);
  int delete_marker(int line, int id);
  void define_marker(int id, int symbol, ARGB foreground_color, ARGB background_color);
  void ShowCaretLine();
  void HideCaretLine();
  void set_caret_line_background_color(ARGB color);

 private:
  static std::string dummy_str_;
};

class GameController  { 
 public:
  GameController();

  void set_id(int id) { id_ = id; }
  int id() const { return id_; }  
  void set(int xpos, int ypos, int zpos, std::bitset<32> buttons) {
    xpos_ = xpos;
    ypos_ = ypos;
    zpos_ = zpos;
    buttons_ = buttons;
  }
  int xpos() const { return xpos_; }
  int ypos() const  { return ypos_; }
  int zpos() const { return zpos_; }  
  const std::bitset<32> buttons() const { return buttons_; }
  
  virtual void OnButtonDown(int button) {}
  virtual void OnButtonUp(int button) {}
  virtual void OnXAxis(int pos, int old_pos) {}
  virtual void OnYAxis(int pos, int old_pos) {}
  virtual void OnZAxis(int pos, int old_pos) {}
    
  void AfterUpdate(const GameController& old_state);

 private:  
  int id_;  
  int xpos_, ypos_, zpos_;  
  std::bitset<32> buttons_;
};

class GameControllersImp;

template <class T>
class GameControllers : public psycle::host::Timer {
 public:
   GameControllers() {
     imp_.reset(ImpFactory::instance().CreateGameControllersImp());
     ScanPluggedControllers();
     StartTimer();
   }
   virtual ~GameControllers() {}

   typedef typename  std::vector<boost::shared_ptr<T> > Container;
   typedef typename  std::vector<boost::shared_ptr<T> >::iterator iterator;

   virtual iterator begin() { return plugged_controllers_.begin(); }
   virtual iterator end() { return plugged_controllers_.end(); }
   virtual bool empty() const { return plugged_controllers_.empty(); }
   virtual int size() const { return plugged_controllers_.size(); }
   
   void set_imp(GameControllersImp* imp) { imp_.reset(imp); }
   void release_imp() { imp_.release(); }
   GameControllersImp* imp() { return imp_.get(); };
   GameControllersImp* imp() const { return imp_.get(); };

   void ScanPluggedControllers() {
     std::vector<int> controller_ids;
     imp_.get()->DevScanPluggedControllers(controller_ids);
     std::vector<int>::iterator it = controller_ids.begin();
     for ( ; it != controller_ids.end(); ++it) {       
       boost::shared_ptr<T> game_controller_info(new T());
       game_controller_info->set_id(*it);
       plugged_controllers_.push_back(game_controller_info);
     }
   }

   void Update() {
     iterator it = plugged_controllers_.begin();
     for ( ; it != plugged_controllers_.end(); ++it) {
       boost::shared_ptr<T> controller = *it;
       GameController old_state = *controller;
       imp_->DevUpdateController(*controller.get()); 
       controller->AfterUpdate(old_state);
     }
   }

   virtual void OnTimerViewRefresh() {
     Update();
   }

 private:
   std::auto_ptr<GameControllersImp> imp_;
   Container plugged_controllers_;   
};

template class GameControllers<GameController>;

class GameControllersImp {
 public:
   GameControllersImp() {}
   virtual ~GameControllersImp() = 0;

   virtual void DevScanPluggedControllers(std::vector<int>& plugged_controller_ids) = 0;   
   virtual void DevUpdateController(ui::GameController& controller) = 0;
};

inline GameControllersImp::~GameControllersImp() {}


class MenuContainer;

// Ui Factory
class Systems {
 public:
  static Systems& instance();
  virtual ~Systems() {}

  void set_concret_factory(Systems& concrete_factory);

  virtual ui::Region* CreateRegion();
  virtual ui::Graphics* CreateGraphics();
  virtual ui::Graphics* CreateGraphics(void* dc);
  virtual ui::Image* CreateImage();
  virtual ui::Font* CreateFont();
  virtual ui::Window* CreateWin();
  virtual ui::Frame* CreateFrame();
  virtual ui::PopupFrame* CreatePopupFrame();
  virtual ui::ComboBox* CreateComboBox();
  virtual ui::Edit* CreateEdit();
  virtual ui::Button* CreateButton();
  virtual ui::ScrollBar* CreateScrollBar(Orientation orientation = VERT);
  virtual ui::TreeView* CreateTreeView();
  virtual ui::ListView* CreateListView();
  virtual ui::MenuContainer* CreateMenuBar();
  virtual ui::PopupMenu* CreatePopupMenu();

  SystemMetrics& metrics();

 protected:
  Systems() {}
  Systems(Systems const&) {}             
  Systems& operator=(Systems const&) {}

 private:
  std::auto_ptr<Systems> concrete_factory_;  
};

// Imp Interfaces
class GraphicsImp {
 public:
  GraphicsImp() {}
  virtual ~GraphicsImp() {}

  virtual void CopyArea(const ui::Rect& rect, const ui::Point& delta) = 0;
  virtual void DrawArc(const ui::Rect& rect, const Point& start, const Point& end) = 0;
  virtual void DrawLine(const ui::Point& p1, const ui::Point& p2) = 0;
  virtual void DrawRect(const ui::Rect& rect) = 0;
  virtual void DrawRoundRect(const ui::Rect& rect, const ui::Dimension& arc_dim) = 0;  
  virtual void DrawOval(const ui::Rect& rect) = 0;
  virtual void DrawString(const std::string& str, double x, double y) = 0;
  virtual void FillRect(const ui::Rect& rect) = 0;
  virtual void FillRoundRect(const ui::Rect& rect, const ui::Dimension& arc_dim) = 0;
  virtual void FillOval(const ui::Rect& rect) = 0;
  virtual void FillRegion(const ui::Region& rgn) = 0;
  virtual void SetColor(ARGB color) = 0;
  virtual ARGB color() const = 0;
  virtual void Translate(double x, double y) = 0;  
  virtual void SetFont(const Font& font) = 0;
  virtual const Font& font() const = 0;
  virtual Dimension text_size(const std::string& text) const = 0;
  virtual void DrawPolygon(const ui::Points& pts) = 0;
  virtual void FillPolygon(const ui::Points& pts) = 0;
  virtual void DrawPolyline(const Points& podoubles) = 0;
  virtual void DrawImage(ui::Image* img, double x, double y) = 0;
  virtual void DrawImage(ui::Image* img, double x, double y, double width, double height) = 0;
  virtual void DrawImage(ui::Image* img, double x, double y, double width, double height, double xsrc, double ysrc) = 0;
  virtual void SetClip(const ui::Rect& rect) = 0;
  virtual void SetClip(ui::Region* rgn) = 0;
  virtual CRgn& clip() = 0;
  virtual void Dispose() = 0;
  virtual CDC* dc() = 0;
  virtual void SaveOrigin() = 0;
  virtual void RestoreOrigin() = 0;
};

class WindowImp {
 public:
  WindowImp() : window_(0) {}
  WindowImp(Window* window) : window_(window) {}
  virtual ~WindowImp() {}

  void set_window(Window* window) { window_ = window; }
  Window* window() { return window_; }
  Window* window() const { return window_; }

  virtual void dev_set_pos(const ui::Rect& pos) = 0;
  virtual void DevScrollTo(int offsetx, int offsety) = 0;
  virtual ui::Rect dev_pos() const = 0;
  virtual ui::Rect dev_abs_pos() const = 0;
  virtual ui::Rect dev_desktop_pos() const = 0;
  virtual ui::Dimension dev_dim() const = 0;
  virtual void DevShow() = 0;
  virtual void DevHide() = 0;
  virtual void DevInvalidate() = 0;
  virtual void DevInvalidate(const Region& rgn) = 0;
  virtual void DevSetCapture() = 0;
  virtual void DevReleaseCapture() = 0;
  virtual void DevShowCursor() = 0;
  virtual void DevHideCursor() = 0;
  virtual void DevSetCursorPos(double x, double y) = 0;
  virtual void DevSetCursor(CursorStyle style) {}  
  virtual void dev_set_parent(Window* window) {} 
  virtual void dev_set_clip_children() {}
  virtual ui::Window* dev_focus_window() = 0;
  virtual void DevSetFocus() = 0;
 
  virtual bool OnDevUpdateArea(ui::Area& rgn) { return true; }
  // Events raised by implementation
  virtual void OnDevDraw(Graphics* g, Region& draw_region);  
  virtual void OnDevSize(const ui::Dimension& dimension);
  
  virtual void dev_add_style(UINT flag) {}
  virtual void dev_remove_style(UINT flag) {}
  virtual void DevEnable() = 0;
  virtual void DevDisable() = 0;

 private:
  Window* window_;
};

class FrameImp : public WindowImp {
 public:  
  FrameImp() {}
  FrameImp(Window* window) : WindowImp(window) {}

  virtual void dev_set_title(const std::string& title) = 0;
  virtual std::string dev_title() const = 0;
  virtual void dev_set_view(ui::Window::Ptr view) = 0;
  virtual void DevShowDecoration() = 0;
  virtual void DevHideDecoration() = 0;
  virtual void DevPreventResize() = 0;
  virtual void DevAllowResize() = 0;

  virtual void OnDevClose();
};

class TreeViewImp : public WindowImp, public NodeOwnerImp {
 public:  
  TreeViewImp() : WindowImp() {}
  TreeViewImp(Window* window) : WindowImp(window) {}
      
  virtual void dev_set_background_color(ARGB color) = 0;
  virtual ARGB dev_background_color() const = 0;  
  virtual void dev_set_text_color(ARGB color) = 0;
  virtual ARGB dev_text_color() const = 0;
  virtual void DevClear() = 0;  
  virtual void dev_select_node(const Node::Ptr& node) = 0;
  virtual Node::WeakPtr dev_selected() = 0;
  virtual void DevEditNode(Node::Ptr node) = 0;
  virtual bool dev_is_editing() const = 0;
  virtual void DevShowLines() = 0;
  virtual void DevHideLines() = 0;
  virtual void DevShowButtons() = 0;
  virtual void DevHideButtons() = 0;
  virtual void dev_set_images(const ui::Images::Ptr& images) = 0;
};

class ListViewImp : public WindowImp, public NodeOwnerImp {
 public:  
  ListViewImp() : WindowImp() {}
  ListViewImp(Window* window) : WindowImp(window) {}
      
  virtual void dev_set_background_color(ARGB color) = 0;
  virtual ARGB dev_background_color() const = 0;  
  virtual void dev_set_text_color(ARGB color) = 0;
  virtual ARGB dev_text_color() const = 0;
  virtual void DevClear() = 0;  
  virtual void dev_select_node(const Node::Ptr& node) = 0;
  virtual std::vector<Node::Ptr> dev_selected_nodes() = 0;
  virtual Node::WeakPtr dev_selected() = 0;
  virtual void DevEditNode(Node::Ptr node) = 0;
  virtual bool dev_is_editing() const = 0;
  virtual void DevAddColumn(const std::string& text, int width) = 0;
  virtual void DevViewList() = 0;
  virtual void DevViewReport() = 0;
  virtual void DevViewIcon() = 0;
  virtual void DevViewSmallIcon() = 0;
  virtual void DevEnableRowSelect() = 0;  
  virtual void DevDisableRowSelect() = 0;
  virtual void dev_set_images(const ui::Images::Ptr& images) = 0;
  virtual int dev_top_index() const = 0;
  virtual void DevEnsureVisible(int index) = 0;
};

class ScrollBarImp : public WindowImp {
 public:
  ScrollBarImp() : WindowImp() {}
  ScrollBarImp(Window* window) : WindowImp(window) {}

  virtual void OnScroll(int pos) {}  
  virtual void dev_set_scroll_range(int minpos, int maxpos) = 0;
  virtual void dev_scroll_range(int& minpos, int& maxpos) = 0;   
  virtual void dev_set_scroll_pos(int pos) = 0;
  virtual int dev_scroll_pos() const = 0;
  virtual ui::Dimension dev_system_size() const = 0;

  virtual void OnDevScroll(int pos);
};

class ComboBoxImp : public WindowImp {
 public:
  ComboBoxImp() : WindowImp() {}
  ComboBoxImp(Window* window) : WindowImp(window) {}

  virtual void dev_add_item(const std::string& item) = 0;
  virtual std::vector<std::string> dev_items() const = 0;
  virtual void dev_set_items(const std::vector<std::string>& itemlist) = 0;
  virtual void dev_set_item_index(int index) = 0;
  virtual int dev_item_index() const = 0;
  virtual std::string dev_text() const = 0;
};

class ButtonImp : public WindowImp {
 public:
  ButtonImp() : WindowImp() {}
  ButtonImp(Window* window) : WindowImp(window) {}

  virtual void dev_set_text(const std::string& text)  = 0;
  virtual std::string dev_text() const = 0;

  virtual void OnDevClick();
};

class CheckBoxImp : public ui::ButtonImp {
 public:
  CheckBoxImp() : ButtonImp() {}  
  CheckBoxImp(Window* window) : ButtonImp(window) {}

  virtual void dev_set_background_color(ARGB color) = 0;
  virtual bool dev_checked() const = 0;
  virtual void DevCheck() = 0;
  virtual void DevUnCheck() = 0;
};

class EditImp : public WindowImp {
 public:
  EditImp() : WindowImp() {}
  EditImp(Window* window) : WindowImp(window) {}

  virtual void dev_set_text(const std::string& text) = 0;
  virtual std::string dev_text() const = 0;  
};

class ScintillaImp : public WindowImp {
 public: 
  ScintillaImp() : WindowImp() {}
  ScintillaImp(Window* window) : WindowImp(window) {}
    
  virtual int dev_f(int sci_cmd, void* lparam, void* wparam) { return 0; }
  virtual void DevAddText(const std::string& text) {}
  virtual void DevFindText(const std::string& text, int cpmin, int cpmax, int& pos, int& cpselstart, int& cpselend) const {}
  virtual void DevGotoLine(int pos) {}
  virtual int dev_length() const { return 0; }
  virtual int dev_selectionstart() const = 0;
  virtual int dev_selectionend() const = 0;
  virtual void DevSetSel(int cpmin, int cpmax) {}
  virtual bool dev_has_selection() const = 0;
  virtual void DevReplaceSel(const std::string& text) = 0; 
  virtual void dev_set_find_match_case(bool on) {}
  virtual void dev_set_find_whole_word(bool on) {}
  virtual void dev_set_find_regexp(bool on) {}
  virtual void DevLoadFile(const std::string& filename) {}
  virtual void DevSaveFile(const std::string& filename) {}
  virtual bool dev_has_file() const { return false; }
  virtual void dev_set_lexer(const Lexer& lexer) {}
  virtual void dev_set_foreground_color(ARGB color) {}
  virtual ARGB dev_foreground_color() const { return 0; }
  virtual void dev_set_background_color(ARGB color) {}
  virtual ARGB dev_background_color() const { return 0; }
  virtual void dev_set_linenumber_foreground_color(ARGB color) {}
  virtual ARGB dev_linenumber_foreground_color() const { return 0; }
  virtual void dev_set_linenumber_background_color(ARGB color) {}
  virtual ARGB dev_linenumber_background_color() const { return 0; }
  virtual void dev_set_margin_background_color(ARGB color) {}
  virtual ARGB dev_margin_background_color() const { return 0; }
  virtual void dev_set_sel_foreground_color(ARGB color) {}
  //ARGB sel_foreground_color() const { return ToARGB(ctrl().sel_foreground_color()); }  
  virtual void dev_set_sel_background_color(ARGB color) {}
  //ARGB sel_background_color() const { return ToARGB(ctrl().sel_background_color()); }
  virtual void dev_set_sel_alpha(int alpha) {}
  virtual void dev_set_ident_color(ARGB color) {}
  virtual void dev_set_caret_color(ARGB color) {}
  virtual ARGB dev_caret_color() const { return 0; }
  virtual void DevStyleClearAll() {}
  virtual const std::string& dev_filename() const = 0;  
  virtual void dev_set_font(const FontInfo& font_info) = 0;
  virtual int dev_column() const = 0;    
  virtual int dev_line() const = 0;
  virtual bool dev_ovr_type() const = 0;
  virtual bool dev_modified() const = 0;
  virtual int dev_add_marker(int line, int id) = 0;
  virtual int dev_delete_marker(int line, int id) = 0;
  virtual void dev_define_marker(int id, int symbol, ARGB foreground_color, ARGB background_color) = 0;
  virtual void DevShowCaretLine() = 0;
  virtual void DevHideCaretLine() = 0;
  virtual void dev_set_caret_line_background_color(ARGB color)  = 0;
};

class MenuContainerImp;
class PopupMenuImp;
class MenuItemImp;

class ImpFactory {
 public:
  static ImpFactory& instance();
  virtual ~ImpFactory() {}

  void set_concret_factory(ImpFactory& concrete_factory);
  virtual ui::WindowImp* CreateWindowImp();
  virtual bool DestroyWindowImp(ui::WindowImp* imp);
  virtual ui::WindowImp* CreateWindowCompositedImp();
  virtual ui::FrameImp* CreateFrameImp();
  virtual ui::FrameImp* CreatePopupFrameImp();
  virtual ui::ScrollBarImp* CreateScrollBarImp(ui::Orientation orientation);
  virtual ui::ComboBoxImp* CreateComboBoxImp();
  virtual ui::EditImp* CreateEditImp();
  virtual ui::TreeViewImp* CreateTreeViewImp();
  virtual ui::ListViewImp* CreateListViewImp();
  virtual ui::MenuContainerImp* CreateMenuContainerImp();
  virtual ui::MenuContainerImp* CreatePopupMenuImp();
  virtual ui::MenuImp* CreateMenuImp();
  virtual ui::ButtonImp* CreateButtonImp();
  virtual ui::CheckBoxImp* CreateCheckBoxImp();
  virtual ui::ScintillaImp* CreateScintillaImp();
  virtual ui::RegionImp* CreateRegionImp();
  virtual ui::FontImp* CreateFontImp();
  virtual ui::GraphicsImp* CreateGraphicsImp();
  virtual ui::GraphicsImp* CreateGraphicsImp(CDC* cr);
  virtual ui::ImageImp* CreateImageImp();
  virtual ui::GameControllersImp* CreateGameControllersImp();  

 protected:
  ImpFactory() {}
  ImpFactory(ImpFactory const&) {}             
  ImpFactory& operator=(ImpFactory const&) {}

 private:
  std::auto_ptr<ImpFactory> concrete_factory_;
};

} // namespace ui
} // namespace host
} // namespace psycle