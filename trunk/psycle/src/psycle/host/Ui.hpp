// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2015-2016 members of the psycle project http://psycle.sourceforge.net

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

struct Dimension {
  Dimension() : width_(0), height_(0) {}
  Dimension(double width, double height) : width_(width), height_(height) {}

  inline bool operator==(const Dimension& rhs) const { 
    return width_ == rhs.width()  && height_ == rhs.height();
  }
  inline bool operator!=(const Dimension& rhs) const { return !(*this == rhs); }

  void set(double width, double height) { 
    width_ = width;
    height_ = height;
  }
  void set_width(double width) { width_ = width; }
  void set_height(double height) { height_ = height; }
  double width() const { return width_; }
  double height() const { return height_; }

 private:
  double width_, height_;
};

class Region;

struct Rect {
  Rect() {}  
  Rect(const ui::Point& top_left, const ui::Point& bottom_right) {
    top_left_ = top_left;
    bottom_right_ = bottom_right;    
  }
  Rect(const ui::Point& top_left, const ui::Dimension& dim) {
    top_left_ = top_left;
    bottom_right_.set(top_left.x() + dim.width(), top_left.y() + dim.height());    
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
    bottom_right_.set(top_left.x() + dim.width(), top_left.y() + dim.height());    
  }
  
  inline void set_left(double left) { top_left_.setx(left); }
  inline void set_top(double top) { top_left_.sety(top); }
  inline void set_right(double right) { bottom_right_.setx(right); }
  inline void set_bottom(double bottom) { bottom_right_.sety(bottom); }
  inline void set_width(double width) { bottom_right_.setx(top_left_.x() + width); }
  inline void set_height(double height) { bottom_right_.setx(top_left_.y() + height); }
  inline double left() const { return top_left_.x(); }
  inline double top() const { return top_left_.y(); }
  inline double right() const { return bottom_right_.x(); }
  inline double bottom() const { return bottom_right_.y(); }
  inline double width() const { return bottom_right_.x() - top_left_.x(); }
  inline double height() const { return bottom_right_.y() - top_left_.y(); }
  inline const ui::Point& top_left() const { return top_left_; }
  inline const ui::Point& bottom_right() const { return bottom_right_; }
  inline ui::Dimension size() const { return Dimension(width(), height()); }
  inline bool empty() const { 
    return top_left_.x() == 0 && top_left_.y() == 0 && bottom_right_.x() == 0 && bottom_right_.y() == 0;
  }
  
  inline void Offset(double dx, double dy) {
    top_left_.setx(top_left_.x() + dx);
    top_left_.sety(top_left_.y() + dy);
    bottom_right_.setx(bottom_right_.x() + dx);
    bottom_right_.sety(bottom_right_.y() + dy);    
  }

  std::auto_ptr<ui::Region> region() const;
 private:
  ui::Point top_left_, bottom_right_;  
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
  Area() : update_bound_cache_(true) {}
  Area(const Rect& rect) { Add(RectShape(rect)); }
  ~Area() {}
  
  Area* Clone() const;

  void Add(const RectShape& area);
  void Clear();
  void Offset(double dx, double dy);
  int Combine(const Area& other, int combinemode);
  bool Intersect(double x, double y) const;
  ui::Rect& bounds() const;

  std::auto_ptr<ui::Region> region() const;
 private:
   void Update() const;
   typedef std::vector<RectShape>::iterator rect_iterator;
   typedef std::vector<RectShape>::const_iterator rect_const_iterator;
   std::vector<RectShape> rect_shapes_; 
   mutable bool update_bound_cache_;   
   mutable ui::Rect bound_cache_;
   mutable std::auto_ptr<ui::Region> region_cache_;
};

enum Orientation { HORZ = 0, VERT = 1 };

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
  Region() {}  
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
  void SetRect(const ui::Rect& rect) { 
    SetRect(rect.left(), rect.top(), rect.width(), rect.height());
  }

  virtual void* source() = 0;
  virtual const void* source() const = 0;
    
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
  virtual ui::Dimension dim() const = 0;
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
  virtual Dimension text_size(const std::string& text) const = 0;
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

enum WindowMsg { ONWND, SHOW, HIDE, FOCUS };

enum AlignStyle {
  ALNONE = 1,
  ALTOP = 2,
  ALLEFT = 4,
  ALRIGHT = 8,
  ALBOTTOM = 16,   
  ALCLIENT = 32 
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
class Ornament;
class Aligner;

class Window : public boost::enable_shared_from_this<Window> {
 friend class WindowImp;
 public:  
  typedef boost::shared_ptr<Window> Ptr;  
  typedef boost::shared_ptr<const Window> ConstPtr;
  typedef boost::weak_ptr<Window> WeakPtr;
  typedef boost::weak_ptr<const Window> ConstWeakPtr;  
  typedef std::vector<Window::Ptr> Container;
  static Window::Ptr nullpointer;

  Window();
  Window(WindowImp* imp);
  virtual ~Window();

  void set_imp(WindowImp* imp);
  WindowImp* imp() { return imp_.get(); };
  WindowImp* imp() const { return imp_.get(); };

  void set_debug_text(const std::string& text) { debug_text_ = text; }
  const std::string& debug_text() const { return debug_text_; }  

  static std::string type() { return "canvasitem"; }
  virtual std::string GetType() const { return "window"; }
 
  typedef Window::Container::iterator iterator;
  virtual iterator begin() { return dummy_list_.begin(); }
  virtual iterator end() { return dummy_list_.end(); }
  virtual bool empty() const { return true; }
  virtual int size() const { return 0; }  
  bool has_childs() const { return size() != 0; }
  virtual Window* root();
  virtual bool is_root() const { return 0; }
  virtual bool IsInGroup(Window::WeakPtr group) const;
  virtual bool IsInGroupVisible() const;
  virtual void set_parent(const Window::WeakPtr& parent);
  virtual Window::WeakPtr parent() { return parent_; }
  virtual Window::ConstWeakPtr parent() const { return parent_; }  
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
  virtual ui::Rect pos() const;
  virtual ui::Rect abs_pos() const;  
  virtual ui::Dimension dim() const;  
   
  virtual void set_aligner(const boost::shared_ptr<Aligner>& aligner) {}
  virtual boost::shared_ptr<Aligner> aligner() const { return dummy_aligner_; }
  virtual void set_style(const ItemStyle::Ptr& style) {}  
  virtual ItemStyle::Ptr style() { 
    if (!style_) {
      style_ = ItemStyle::Ptr(new ItemStyle());     
    }
    return style_;
  }
  virtual ItemStyle::Ptr style() const { 
    if (!style_) {
      style_ = ItemStyle::Ptr(new ItemStyle());     
    }
    return style_;
  }
  virtual bool has_style() const { return style_ != 0; }  

  virtual void Show();
  virtual void Hide();
  virtual bool visible() const { return visible_; }
  virtual void FLS();  // invalidate combine new & old region
  virtual void FLS(const Region& rgn) {} // invalidate region
  virtual void STR(); // store old region
  virtual void Invalidate();
  virtual void Invalidate(Region& rgn);
  virtual void PreventFls() {}
  virtual void EnableFls() {}
  virtual void SetCapture();
  virtual void ReleaseCapture();
  virtual void ShowCursor();
  virtual void HideCursor();
  virtual void SetCursorPos(double x, double y);
  virtual void SetCursor(CursorStyle style);  
  CursorStyle cursor() const { return DEFAULT; }
  virtual void set_parent(Window* window);
  
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
  virtual void SetClip(double x, double y, double width, double height) {}
  virtual bool has_clip() const { return false; }
  const Region& clip() const { return *dummy_region_.get(); }
  virtual void RemoveClip() {}
  
  virtual void Draw(Graphics* g, Region& draw_region) {}
  virtual void DrawBackground(Graphics* g, Region& draw_region);
  virtual void set_ornament(boost::shared_ptr<ui::Ornament> ornament);
  virtual boost::weak_ptr<ui::Ornament> ornament();
  void set_clip_children();
  void add_style(UINT flag);
  void remove_style(UINT flag);

  virtual void OnMessage(WindowMsg msg, int param = 0) {}
  virtual void OnSize(double width, double height) {}
  virtual void OnChildPos(ui::Window& child) {}
  
  virtual void EnablePointerEvents() { pointer_events_ = true; }
  virtual void DisablePointerEvents() { pointer_events_ = false; }
  virtual bool pointerevents() const { return pointer_events_; }
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
  
  virtual void OnKeyDown(KeyEvent& ev) { ev.WorkParent(); }
  virtual void OnKeyUp(KeyEvent& ev) { ev.WorkParent(); }
  boost::signal<void (KeyEvent&)> KeyDown;
  boost::signal<void (KeyEvent&)> KeyUp;
   
  virtual void GetFocus() {}
  virtual void OnFocus() {}
  virtual void OnKillFocus() {}
  boost::signal<void ()> Focus;
  boost::signal<void ()> KillFocus;
  ui::Rect pos_old_;  
  virtual void OnFocusChange(int id) {}

 protected:  
  virtual void WorkMouseDown(MouseEvent& ev) { OnMouseDown(ev); }
  virtual void WorkMouseUp(MouseEvent& ev) { OnMouseUp(ev); }
  virtual void WorkMouseMove(MouseEvent& ev) { OnMouseMove(ev); }
  virtual void WorkDblClick(MouseEvent& ev) { OnDblclick(ev); }
  virtual void WorkKeyUp(KeyEvent& ev) { OnKeyUp(ev); }
  virtual bool WorkKeyDown(KeyEvent& ev) { OnKeyDown(ev); return true; }
  void WorkChildPos();

  mutable bool update_;
  mutable std::auto_ptr<Area> area_;  
  mutable std::auto_ptr<Area> fls_area_;
 private:  
  mutable ItemStyle::Ptr style_;
  std::auto_ptr<WindowImp> imp_;
  Window::WeakPtr parent_;  
  static Container dummy_list_;  
  std::string debug_text_;
  boost::shared_ptr<Aligner> dummy_aligner_;
  static boost::shared_ptr<ui::Region> dummy_region_;
  bool auto_size_width_, auto_size_height_;
  boost::weak_ptr<Ornament> ornament_;  
  bool visible_, pointer_events_;
  ui::Point pos_; 
  ui::Point Window::CalcAbsPos(const Point& pos) const;  
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
  void RemoveAll() { STR(); items_.clear(); FLS(); }
    
  // appearence
  void set_aligner(const boost::shared_ptr<Aligner>& aligner); 
  virtual Window::Ptr HitTest(double x, double y);
  void RaiseToTop(Window::Ptr item) {  Remove(item); Add(item); }
  void set_zorder(Window::Ptr item, int z);
  int zorder(Window::Ptr item) const;  
  virtual bool OnUpdateArea();  
  virtual void OnMessage(WindowMsg msg, int param = 0);    
  void Align();
  void FlagNotAligned();
  boost::shared_ptr<Aligner> aligner() const { return aligner_; }  
  virtual void OnChildPos(Window& window) {
    if (auto_size_width() || auto_size_height()) {
      window.needsupdate();
      Window::set_pos(pos());
    }
  }
  
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

class Frame : public Window {
 public:  
  typedef boost::weak_ptr<Frame> WeakPtr;
  static std::string type() { return "canvastreeitem"; }  

  Frame();
  Frame(FrameImp* imp);

  FrameImp* imp() { return (FrameImp*) Window::imp(); };
  FrameImp* imp() const { return (FrameImp*) Window::imp(); };

  virtual void set_view(ui::Window::Ptr view);
  virtual void set_title(const std::string& title);

  virtual void OnClose() {}
};

class Ornament {
 public:
  Ornament() {}
  virtual ~Ornament() = 0;

  virtual bool transparent() const { return true; }

  virtual void Draw(Window::Ptr& item, Graphics* g, Region& draw_region) = 0;
  virtual std::auto_ptr<ui::Rect> padding() const { return std::auto_ptr<ui::Rect>(); }
};

inline Ornament::~Ornament() {}

// Controls

class TreeNodeImp;
class Tree;

class TreeNode : public boost::enable_shared_from_this<TreeNode> {
 public:
  static std::string type() { return "canvastreenode"; }
  virtual void set_text(const std::string& text);
  virtual std::string text() const;

  virtual void Add(boost::shared_ptr<TreeNode> node);

  void set_imp(TreeNodeImp* imp);
  TreeNodeImp* imp() { return imp_.get(); };
  TreeNodeImp* imp() const { return imp_.get(); };

  void set_tree(boost::weak_ptr<Tree> tree);
  void set_parent(const boost::weak_ptr<TreeNode>& parent);
  boost::weak_ptr<TreeNode> parent() const { return parent_; }

  virtual void OnClick() {}

  void WorkClick();
private:  
  std::list<boost::shared_ptr<TreeNode> > children_;
  std::auto_ptr<TreeNodeImp> imp_;
  boost::weak_ptr<Tree> tree_;
  boost::weak_ptr<TreeNode> parent_;
};

class TreeImp;

class Tree : public Window {
 public:  
  typedef boost::weak_ptr<Tree> WeakPtr;
  static std::string type() { return "canvastreeitem"; }  

  Tree() { set_auto_size(false, false); }
  Tree(TreeImp* imp);

  TreeImp* imp() { return (TreeImp*) Window::imp(); };
  TreeImp* imp() const { return (TreeImp*) Window::imp(); };

  void test();

  void AddNode(boost::shared_ptr<TreeNode> node);
  void Clear();    

  virtual void build(TreeNode& node);

  virtual void set_background_color(ARGB color);
  virtual ARGB background_color() const;

  virtual void set_text_color(ARGB color);
  virtual ARGB text_color() const;

  virtual void OnClick(TreeNode* node) {}

 private: 
  std::list<boost::shared_ptr<TreeNode> > children_;
};

class TableItemImp;
class Table;

class TableItem : public boost::enable_shared_from_this<TableItem> {
 public:
  static std::string type() { return "canvastableitem"; }
  virtual void set_text(const std::string& text);
  virtual std::string text() const;

  //virtual void Add(boost::shared_ptr<TableItem> item);

  void set_imp(TableItemImp* imp);
  TableItemImp* imp() { return imp_.get(); };
  TableItemImp* imp() const { return imp_.get(); };

  void set_table(boost::weak_ptr<Table> table);
  
  virtual void OnClick() {}

  

  void WorkClick();
private:  
  //std::list<boost::shared_ptr<TreeNode> > children_;
  std::auto_ptr<TableItemImp> imp_;
  boost::weak_ptr<Table> table_;  
};

class TableImp;

class Table : public Window {
 public:    
  static std::string type() { return "canvastableitem"; }  

  Table() { set_auto_size(false, false); }
  Table(TableImp* imp);

  TableImp* imp() { return (TableImp*) Window::imp(); };
  TableImp* imp() const { return (TableImp*) Window::imp(); };
  
  virtual void OnClick() {}   

  void InsertColumn(int col, const std::string& text);
  void InsertRow();
  int InsertText(int nItem, const std::string& text);
  void SetText(int nItem, int nSubItem, const std::string& text);
  void AutoSize(int cols);
  virtual void set_background_color(ARGB color);
  virtual ARGB background_color() const;

  virtual void set_text_color(ARGB color);
  virtual ARGB text_color() const;
};

class ScrollBarImp;

class ScrollBar : public Window {
 public:
  static std::string type() { return "canvasscrollbaritem"; }
  ScrollBar() {}
  ScrollBar(ScrollBarImp* imp);

  ScrollBarImp* imp() { return (ScrollBarImp*) Window::imp(); };
  ScrollBarImp* imp() const { return (ScrollBarImp*) Window::imp(); };

  virtual void OnScroll(int pos) {}
  
  void set_scroll_range(int minpos, int maxpos);
  void scroll_range(int& minpos, int& maxpos); 
  void set_scroll_pos(int pos);
  int scroll_pos() const;
  void system_size(int& width, int& height) const;
};

class ComboBoxImp;

class ComboBox : public Window {
 public:
  static std::string type() { return "canvascomboboxitem"; }
  ComboBox() {}
  ComboBox(ComboBoxImp* imp);

  ComboBoxImp* imp() { return (ComboBoxImp*) Window::imp(); };
  ComboBoxImp* imp() const { return (ComboBoxImp*) Window::imp(); };
};

class EditImp;

class Edit : public Window {
 public:
  static std::string type() { return "canvaseditboxitem"; }
  Edit() { set_auto_size(false, false); }
  Edit(EditImp* imp);

  EditImp* imp() { return (EditImp*) Window::imp(); };
  EditImp* imp() const { return (EditImp*) Window::imp(); };
  virtual void set_text(const std::string& text);
  virtual std::string text() const;
};

class ButtonImp;

class Button : public Window {
 public:
  static std::string type() { return "canvaseditboxitem"; }
  Button() {}
  Button(ButtonImp* imp);

  ButtonImp* imp() { return (ButtonImp*) Window::imp(); };
  ButtonImp* imp() const { return (ButtonImp*) Window::imp(); };
  virtual void set_text(const std::string& text);
  virtual std::string text() const;
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
  Scintilla() { set_auto_size(false, false); }
  Scintilla(ScintillaImp* imp);

  ScintillaImp* imp() { return (ScintillaImp*) Window::imp(); };
  ScintillaImp* imp() const { return (ScintillaImp*) Window::imp(); };
  
  virtual std::string GetType() const { return "scintilla"; }

  int f(int sci_cmd, void* lparam, void* wparam);
  void AddText(const std::string& text);
  void FindText(const std::string& text, int cpmin, int cpmax, int& pos, int& cpselstart, int& cpselend) const;
  void GotoLine(int pos);
  int length() const;
  int selectionstart();
  int selectionend();
  void SetSel(int cpmin, int cpmax);
  bool has_selection() const;
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
private:
  static std::string dummy_str_;
};

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
  virtual ui::ComboBox* CreateComboBox();
  virtual ui::Edit* CreateEdit();
  virtual ui::Button* CreateButton();
  virtual ui::ScrollBar* CreateScrollBar();
  virtual ui::Tree* CreateTree();

 protected:
  Systems() {}
  Systems(Systems const&) {}             
  Systems& operator=(Systems const&) {}

 private:
  std::auto_ptr<Systems> concrete_factory_;
};

// Imp Interfaces
class WindowImp {
 public:
  WindowImp() : window_(0) {}
  WindowImp(Window* window) : window_(window) {}
  virtual ~WindowImp() {}

  void set_window(Window* window) { window_ = window; }
  Window* window() { return window_; }
  Window* window() const { return window_; }

  virtual void dev_set_pos(const ui::Rect& pos) = 0;
  virtual ui::Rect dev_pos() const = 0;
  virtual ui::Dimension dev_dim() const = 0;
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
  virtual bool DevDestroy() { return false; }  
  virtual void dev_set_parent(Window* window) {} 
  virtual void dev_set_clip_children() {}
 
  virtual bool OnDevUpdateArea(ui::Area& rgn) { return true; }
  // Events raised by implementation
  virtual void OnDevDraw(Graphics* g, Region& draw_region);  
  virtual void OnDevSize(double width, double height);
  virtual void OnDevMouseDown(MouseEvent& ev);    
  virtual void OnDevMouseUp(MouseEvent& ev);    
  virtual void OnDevDblclick(MouseEvent& ev);   
  virtual void OnDevMouseMove(MouseEvent& ev);  
  virtual void OnDevKeyDown(KeyEvent& ev);   
  virtual void OnDevKeyUp(KeyEvent& ev);
  virtual void OnDevFocusChange(int id);

  virtual void dev_add_style(UINT flag) {}
  virtual void dev_remove_style(UINT flag) {}

 private:
  Window* window_;
};

class FrameImp : public WindowImp {
 public:  
  FrameImp() : WindowImp() {}
  FrameImp(Window* window) : WindowImp(window) {}  

  virtual void dev_set_title(const std::string& title) = 0;
  virtual void dev_set_view(ui::Window::Ptr view) = 0;

  virtual void OnDevClose();
};


class TreeNodeImp {
 public:
  TreeNodeImp() : tree_node_(0) {}
  virtual ~TreeNodeImp() {}
  
  void set_tree_node(TreeNode* tree_node) { tree_node_ = tree_node; }
  TreeNode* tree_node() const { return tree_node_; }

  virtual void dev_set_tree(const boost::weak_ptr<ui::TreeNode>& parent,
                            boost::weak_ptr<Tree> tree) = 0;  
  virtual void dev_set_text(const std::string& text) =  0;
  virtual std::string dev_text() const = 0;

  virtual void OnDevClick() {
    tree_node_->WorkClick();
  }

 private:
  TreeNode* tree_node_;
};

class TreeImp : public WindowImp {
 public:  
  TreeImp() : WindowImp() {}
  TreeImp(Window* window) : WindowImp(window) {}
      
  virtual void dev_set_background_color(ARGB color) = 0;
  virtual ARGB dev_background_color() const = 0;  
  virtual void dev_set_text_color(ARGB color) = 0;
  virtual ARGB dev_text_color() const = 0;  
};

class TableItemImp {
 public:
  TableItemImp() {} // : tree_node_(0) {}
  virtual ~TableItemImp() {}
  
  //void set_tree_node(TreeNode* tree_node) { tree_node_ = tree_node; }
  //TreeNode* tree_node() const { return tree_node_; }

  virtual void dev_set_table(boost::weak_ptr<Table> table) = 0;  
  virtual void dev_set_text(const std::string& text) =  0;
  virtual std::string dev_text() const = 0;

  
  /*virtual void OnDevClick() {
    tree_node_->WorkClick();
  }*/

 private:
  //TareeNode* tree_node_;
};

class TableImp : public WindowImp {
 public:
  TableImp() : WindowImp() {}
  TableImp(Window* window) : WindowImp(window) {}  

  virtual void DevInsertColumn(int col, const std::string& text) = 0;
  virtual void DevInsertRow() = 0;
  virtual int DevInsertText(int nItem, const std::string& text) = 0;
  virtual void DevSetText(int nItem, int nSubItem, const std::string& text) = 0;
  virtual void DevAutoSize(int cols) = 0;
  virtual void dev_set_background_color(ARGB color) = 0;
  virtual ARGB dev_background_color() const = 0;  
  virtual void dev_set_text_color(ARGB color) = 0;
  virtual ARGB dev_text_color() const = 0;
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
};

class ComboBoxImp : public WindowImp {
 public:
  ComboBoxImp() : WindowImp() {}
  ComboBoxImp(Window* window) : WindowImp(window) {}
};

class ButtonImp : public WindowImp {
 public:
  ButtonImp() : WindowImp() {}
  ButtonImp(Window* window) : WindowImp(window) {}

  virtual void dev_set_text(const std::string& text)  = 0;
  virtual std::string dev_text() const = 0;
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
  virtual int dev_selectionstart() { return 0; }
  virtual int dev_selectionend() { return 0; }
  virtual void DevSetSel(int cpmin, int cpmax) {}
  virtual bool dev_has_selection() const { return false; }
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
  virtual bool dev_is_modified() const { return false; }
  virtual void OnDevFirstModified() {}  
};

class ImpFactory {
 public:
  static ImpFactory& instance();
  virtual ~ImpFactory() {}

  void set_concret_factory(ImpFactory& concrete_factory);
  virtual ui::WindowImp* CreateWindowImp();
  virtual ui::WindowImp* CreateWindowCompositedImp();
  virtual ui::FrameImp* CreateFrameImp();
  virtual ui::ScrollBarImp* CreateScrollBarImp(ui::Orientation orientation);
  virtual ui::ComboBoxImp* CreateComboBoxImp();
  virtual ui::EditImp* CreateEditImp();
  virtual ui::TreeImp* CreateTreeImp();
  virtual ui::TableImp* CreateTableImp();
  virtual ui::TreeNodeImp* CreateTreeNodeImp();
  virtual ui::ButtonImp* CreateButtonImp();
  virtual ui::ScintillaImp* CreateScintillaImp();

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
