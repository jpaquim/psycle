// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "PsycleConfig.hpp"
#include "Ui.hpp"
#include "CScintilla.hpp"
#include "DesignPatterns.hpp"

#define BOOST_SIGNALS_NO_DEPRECATION_WARNING

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>
#include <boost/bind.hpp>

namespace psycle {
namespace host {
namespace ui {
namespace canvas {

using namespace ui;

enum CanvasMsg {
  ONWND,
  SHOW,
  HIDE
};

struct BlitInfo {
  double dx, dy;  
};

class Item;


class AlignStrategy {
 public:
  typedef boost::shared_ptr<AlignStrategy> Ptr;
  typedef boost::weak_ptr<const AlignStrategy> ConstPtr;
  typedef boost::weak_ptr<AlignStrategy> WeakPtr;  

  AlignStrategy() {}
  virtual ~AlignStrategy() = 0;
  virtual void Add(boost::shared_ptr<Item>& item) = 0;  
  virtual void DoAlign(bool has_w, bool has_h);
};

inline AlignStrategy::~AlignStrategy() {};

enum AlignStyle {
  ALNONE = 1,
  ALTOP = 2,
  ALLEFT = 4,
  ALRIGHT = 8,
  ALBOTTOM = 16,   
  ALCLIENT = 32,
  ALFIXED = 64
};

enum Orientation { 
  HORZ = 0, 
  VERT = 1
};

typedef psycle::host::ui::Rect Margin;

class ItemStyle {
 public:
  ItemStyle() : align_(ALNONE) {}

  AlignStyle align() const { return align_; }
  void set_align(AlignStyle align) { 
    align_ = align;
    // update();
  }  

  const Margin& margin() const { return margin_; }

  void set_margin(Margin margin) { margin_ = margin; update(); }
  
   boost::signal<void ()> update;
 private:
  AlignStyle align_;
  Margin margin_;
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

class Canvas;

class Item : public boost::enable_shared_from_this<Item> {
  friend class Group;
  friend class Canvas;
public:  
  typedef boost::shared_ptr<Item> Ptr;
  typedef boost::shared_ptr<const Item> ConstPtr;
  typedef boost::weak_ptr<Item> WeakPtr;
  typedef boost::weak_ptr<const Item> ConstWeakPtr;
  typedef boost::shared_ptr<ItemStyle> StylePtr;
  typedef std::vector<Item::Ptr> ItemList;

  static canvas::Item::Ptr nullpointer;

  Item() 
    : rgn_(new mfc::Region),
      clp_rgn_(new mfc::Region),
      fls_rgn_(new mfc::Region),       
      draw_rgn_(0) {
    x_ = y_ = 0;
    visible_ = pointer_events_ = update_= true;
    has_clip_ = has_store_ = false;
    color_ = 0xFF000000;
  }
    
  virtual ~Item() {}
  
  static std::string type() { return "canvasitem"; }

  typedef ItemList::iterator iterator;
  virtual iterator begin() { return dummy.begin(); }
  virtual iterator end() { return dummy.end(); }
  virtual bool empty() const { return true; }
  virtual int size() const { return 0; }
  
  virtual void Add(const Item::Ptr& item) {}
  virtual void Insert(iterator it, const Item::Ptr& item) {}
  virtual void Remove(const Item::Ptr& item) {}
  virtual void RemoveAll() {}
  virtual void Clear() {}

  void set_align_strategy(AlignStrategy::Ptr& strategy) { 
    align_strategy_ = strategy;
  }
  AlignStrategy::Ptr align_strategy() const { return align_strategy_; }

  ItemList SubItems() {
    ItemList allitems;
    iterator it = begin();
    for (; it != end(); ++it) {
      Item::Ptr item = *it;
      allitems.push_back(item);
      ItemList subs = item->SubItems();
      iterator itsub = subs.begin();
      for (; itsub != subs.end(); ++itsub) {
        allitems.push_back(*it);
      }
    }
    return allitems;
  }

  virtual void Draw(Graphics* g, Region& draw_region) {}
  void DrawBackground(Graphics* g, Region& draw_region);

  void GetFocus();
  void Detach();
  virtual void Show();
  virtual void Hide();
  // regions
  void STR();  // store old region
  void FLS();  // invalidate combine new & old region
  void FLS(const Region& rgn); // invalidate region
  void needsupdate();
  virtual const Region& region() const {
    if (update_) {          
      update_ = !const_cast<Item*>(this)->onupdateregion();
      if (has_clip_) {
        rgn_->Combine(*clp_rgn_, RGN_AND);
      }      
    }            
    return *rgn_.get();
  }
  virtual bool onupdateregion() { needsupdate(); return true; }
  std::auto_ptr<Region> draw_region() { 
    return draw_rgn_ ? ClientToItemRegion(*draw_rgn_)
                     : std::auto_ptr<Region>();    
  }
  std::auto_ptr<Region> ClientToItemRegion(const Region& rgn) {
    std::auto_ptr<Region> irgn(rgn.Clone());
    irgn->Offset(-zoomabsx(), -zoomabsy());
    return irgn;
  }
  virtual void BoundRect(double& x, double& y, double& width, double& height) const {
    region().BoundRect(x, y, width, height);    
  }

  virtual void BoundRect(Rect& pos) const {
    double x, y, w, h;
    BoundRect(x, y, w, h);
    pos.set(x, y, x + w, y + h);
  }

  virtual Item::Ptr HitTest(double x, double y) {
    return region().Intersect(x, y) ? shared_from_this() : nullpointer;
  }
  
  virtual bool visible() const { return visible_; }  
  bool IsInGroupVisible() const;
  void SetBlitXY(double x, double y);

  virtual void SetPos(double x, double y, double width, double height) {
    SetXY(x, y);
    SetSize(width, height);
  }

  virtual void SetX(double x) {
    if (x_ != x) {      
      STR(); x_ = x; FLS();
    }      
  } 

  virtual void SetY(double y) {
    if (y_ != y) {      
      STR(); y_ = y; FLS();
    }      
  } 

  virtual void SetXY(double x, double y) {
    if (x_!=x || y_!=y) {      
      STR(); x_ = x; y_ = y; FLS();
    }      
  }  
  void SetSize(double w, double h);

  virtual void SetWidth(double width) { 
    double x1, y1, w, h;
    BoundRect(x1, y1, w, h);
    SetPos(x(), y(), width, h);   
  }

  void PreventFls();
  void EnableFls();

  virtual void SetHeight(double height) { 
    double x1, y1, w, h;
    BoundRect(x1, y1, w, h);
    SetPos(x(), y(), w, height);   
  }

  virtual double width() const {
    double x, y, w, h;
    region().BoundRect(x, y, w, h);
    return w;
  }
  virtual double height() const { 
    double x, y, w, h;
    region().BoundRect(x, y, w, h);
    return h;
  }

  virtual void CalcDimension(ui::Dimension& dim) {
    if (has_style()) {
      const Margin& margin = style()->margin();
      dim.set_size(width() + margin.left() + margin.right(),
                   height() + margin.top() + margin.bottom());
    } else {
      dim.set_size(width(), height());
    }
    dim_ = dim;
  }

  virtual void DoAlign() {}

  void UpdateAlign();
  
  ui::Dimension dim_;

  virtual void OnSize(double w, double h) {}
  double x() const { return x_; }
  double y() const { return y_; }
  void pos(double& xv, double& yv) const { xv = x(); yv = y(); }
  void clientpos(double& xv, double& yv) const { xv = zoomabsx(); yv = zoomabsy(); }
  virtual double zoomabsx() const;
  virtual double zoomabsy() const;
  virtual double acczoom() const;  
 
  void EnablePointerEvents() { pointer_events_ = true; }
  void DisablePointerEvents() { pointer_events_ = false; }
  bool pointerevents() const { return pointer_events_; }

  Canvas* root();
  virtual Canvas* root() const { return 0; }
  virtual void set_parent(const Item::WeakPtr& parent) { parent_ = parent; }
  Item::WeakPtr parent() { return parent_; }
  Item::ConstWeakPtr parent() const { return parent_; }
  bool IsInGroup(Item::WeakPtr group) const;
  virtual bool is_root() const { return 0; }
                            
  void SetClip(double x, double y, double width, double height) {
    STR();            
    clp_rgn_->SetRect(x, y, width, height);
    has_clip_ = true;    
    FLS(); 
  }
  bool has_clip() const { return has_clip_; }
  const Region& clip() const { return *clp_rgn_.get(); }
  void RemoveClip() { has_clip_ = false; }
  virtual void OnMessage(CanvasMsg msg) { }
  void set_style(StylePtr& style) { 
    style_ = style; 
   //  style_->update.connect(boost::bind(&Item::onalign, this));
  }

  StylePtr style() { 
    if (!style_) {
      style_ = Item::StylePtr(new ItemStyle());
     //  style_->update.connect(boost::bind(&Item::onalign, this));
    }
    return style_;
  }
  bool has_style() const { return style_ != 0; }  
  
  void set_fill_color(ARGB color) { color_ = color; }
  ARGB fill_color() const { return color_; }

 protected:
  void swap_smallest(double& x1, double& x2) const {
    if (x1 > x2) {
      std::swap(x1, x2);
     }
  }
  boost::shared_ptr<Item> this_ptr() { return shared_from_this(); }
  double x_, y_;
  mutable bool update_, has_clip_;
  mutable std::auto_ptr<Region> rgn_;
  std::auto_ptr<Region> clp_rgn_;
   
  // Mouse Events
  virtual void OnMouseDown(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnMouseUp(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnDblclick(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnMouseMove(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnMouseEnter(MouseEvent& ev) { ev.WorkParent(); }
  virtual void OnMouseOut(MouseEvent& ev) { ev.WorkParent(); }
  // signals
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

 private:  
  Item::WeakPtr parent_;
  std::auto_ptr<Region> fls_rgn_;
  Region* draw_rgn_;
  bool visible_, pointer_events_, has_store_;
  ItemList dummy;
  std::auto_ptr<BlitInfo> blit_;
  StylePtr style_;
  AlignStrategy::Ptr align_strategy_;
  ARGB color_;
};

class DefaultAlignStrategy : public AlignStrategy {
  virtual void Add(Item::Ptr& item) {}
  virtual void DoAlignStrategy() {}
};


class Canvas;

class Group : public Item {
 public:
  friend Canvas;
  friend Item;
  
  Group() : Item(), zoom_(1), is_root_(false), w_(-1), h_(-1) { }

  static std::string type() { return "canvasgroup"; }

  typedef ItemList::iterator iterator;
  virtual iterator begin() { return items_.begin(); }
  virtual iterator end() { return items_.end(); }
  virtual bool empty() const { return items_.empty(); }
  virtual int size() const { return items_.size(); }

  void Add(const Item::Ptr& item);
  void Insert(iterator it, const Item::Ptr& item);
  void Remove(const Item::Ptr& item);
  void RemoveAll() { STR(); items_.clear(); FLS(); }
  void Clear();  
    
  virtual void Draw(Graphics* g, Region& draw_region);
 
  virtual Item::Ptr HitTest(double x, double y);

  void RaiseToTop(Item::Ptr item) {  Remove(item); Add(item); }
  void set_zorder(Item::Ptr item, int z);
  int zorder(Item::Ptr item) const;
  void setzoom(double zoom) { zoom_ = zoom; }
  virtual double zoom() const { return zoom_; }
  virtual bool onupdateregion();

  bool is_root() const { return is_root_; }
  virtual void OnMessage(CanvasMsg msg);
   
  virtual void CalcDimension(ui::Dimension& dim);
  virtual void DoAlign();
  
  virtual void SetPos(double x, double y, double width, double height) {
    STR();
    x_ = x;
    y_ = y;
    w_ = width;
    h_ = height;
    FLS();
  }

  virtual void SetWidth(double width) {    
    w_ = width;
    needsupdate();
  }

  virtual void SetHeight(double height) {    
    h_ = height;
    needsupdate();
  }

  virtual void OnSize(double w, double h);
 

 protected:  
  ItemList items_;

 private:
  void Init(); 

  double zoom_;
  bool is_root_;
  double w_, h_;
  ARGB color_;
};


class View;

// canvas
class Canvas : public Group {
  friend class Item;
  friend class Group;
 public:
  typedef boost::shared_ptr<Canvas> Ptr;
  typedef boost::weak_ptr<Canvas> WeakPtr;

  Canvas() : Group(), wnd_(0) { Init(); }
  Canvas(View* parent) : Group(), wnd_(parent) { Init(); }    
  
  static std::string type() { return "canvas"; }
    
  void Draw(Graphics* g, Region& rgn);  
  void Flush();
  void SetColor(ARGB color) { color_ = color; }
  ARGB color() const { return color_; }
  void set_bg_image(CBitmap* bg_image, int width, int height) {
    bg_image_ = bg_image;
    bg_width_ = width;
    bg_height_ = width;
  }
  void StealFocus(const Item::Ptr& item);
  void SetFocus(const Item::Ptr& item);
  virtual void OnSize(double cx, double cy);
    void SetSave(bool on) { save_ = on; }
    bool IsSaving() const { return save_; }    
    void ClearSave() { save_rgn_.Clear(); }
    void set_wnd(View* wnd) {
      if (old_wnd_ != wnd) {
        old_wnd_ = wnd_ = wnd;        
        Group::OnMessage(ONWND);        
      }
    }
    View* wnd() { return wnd_; }
    int cw() const { return cw_; }
    int ch() const { return ch_; }
    void ClientSize(int& cw, int& ch) {
      cw = cw_;
      ch = ch_;
    }

    virtual bool onupdateregion() { 
      rgn_->SetRect(0, 0, cw_, ch_);
      return true;
    }

    void setpreferredsize(double width, double height) {
      pw_ = width;
      ph_ = height;
    }
    void preferredsize(double& width, double& height) const {
      width = pw_;
      height = ph_;
    }
    
  void InvalidateSave();
  void SetCapture();  
  void ReleaseCapture();
  void ShowCursor();
  void HideCursor();
  void SetCursorPos(int x, int y);      
  void SetCursor(CursorStyle style);
  
  HCURSOR cursor() const { return cursor_; }
  double zoomabsx() const { return 0; }
  double zoomabsy() const { return 0; }
  double x() const { return 0; }
  double y() const { return 0; }
  bool show_scrollbar;
  int nposv, nposh;

  void WorkMouseDown(MouseEvent& ev);
  void WorkMouseUp(MouseEvent& ev);
  void WorkMouseMove(MouseEvent& ev);
  void WorkDblClick(MouseEvent& ev);
  void WorkKeyUp(KeyEvent& ev);
  bool WorkKeyDown(KeyEvent& ev);
  void WorkOnSize(int cw, int ch);
  void WorkTimer();

  void Invalidate(); // full invalidate
 protected:
  void Invalidate(Region& rgn);

 private:  
  void Init();
  void DoDraw(Graphics* g, Region& rgn);
  Canvas(const Canvas& other) {}
  Canvas& operator=(Canvas rhs) {}
 
  template <class T, class T1>
  void WorkEvent(T& ev, void (T1::*ptmember)(T&), Item::Ptr& item) {
    while (item) {
      // send event to item
      (item.get()->*ptmember)(ev);
      if (ev.is_work_parent()) {
        item = item->parent().lock();  
      } else {    
        break;
      }
    }
  }

  View* wnd_, *old_wnd_;  
  bool save_, steal_focus_, prevent_fls_;
  Item::WeakPtr button_press_item_, mouse_move_, focus_item_;
  CBitmap* bg_image_;
  int bg_width_, bg_height_;
  int cw_, ch_, pw_, ph_;
  ARGB color_;
  mfc::Region save_rgn_;
  HCURSOR cursor_;
  bool item_blit_;
};


class CanvasFrameWnd;

class View : public CWnd, public Timer {
 public:
  View() {}

  boost::signal<void (std::exception&)> error;
  
  void set_canvas(Canvas::WeakPtr canvas);    
  Canvas::WeakPtr canvas() { return canvas_; }     
  void RegisterMfcCtrl(const Item::Ptr& ctrl, int id) { 
    mfc_ctrls_[id] = ctrl; 
  }
  
 protected:		
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);   
  virtual BOOL PreTranslateMessage(MSG* pMsg);  
	DECLARE_MESSAGE_MAP()
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
  void OnDestroy();
	void OnSetFocus(CWnd* pOldWnd);
  void OnPaint();
  void OnSize(UINT nType, int cx, int cy);
  // MouseEvents
	void OnLButtonDown(UINT nFlags, CPoint pt) {
    try {    
      if (!canvas_.expired()) {
        MouseEvent ev(pt.x, pt.y, 1, nFlags);
        canvas().lock()->WorkMouseDown(ev);                  
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }
  }
  void OnRButtonDown(UINT nFlags, CPoint pt) {
    try {    
      if (!canvas_.expired()) {
        MouseEvent ev(pt.x, pt.y, 2, nFlags);
        canvas().lock()->WorkMouseDown(ev);                  
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }
  }
	void OnLButtonDblClk(UINT nFlags, CPoint pt) {
    try {    
      if (!canvas_.expired()) {
        MouseEvent ev(pt.x, pt.y, 1, nFlags);
        canvas().lock()->WorkDblClick(ev);                  
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }
  }
	void OnMouseMove(UINT nFlags, CPoint pt) {
    try {    
      if (!canvas_.expired()) {
        MouseEvent ev(pt.x, pt.y, 1, nFlags);
        canvas().lock()->WorkMouseMove(ev);                  
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }
  }
	void OnLButtonUp(UINT nFlags, CPoint pt) {
    try {    
      if (!canvas_.expired()) {
        MouseEvent ev(pt.x, pt.y, 1, nFlags);
        canvas().lock()->WorkMouseUp(ev);                  
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }
  }
	void OnRButtonUp(UINT nFlags, CPoint pt) {
    try {    
      if (!canvas_.expired()) {
        MouseEvent ev(pt.x, pt.y, 2, nFlags);
        canvas().lock()->WorkMouseUp(ev);                  
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }
  }
    
  void OnTimerViewRefresh();
  BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
  void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);         
	void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);        
   
  void OnCtrlCommand(UINT id);

  void OnCtrlBtnClick(UINT nID) {
/*      Button* btn = (Button*) Button::FindById(nID);
    if (btn) {
      btn->OnClick();
    }*/
  }

  int Win32FlagsToButton(UINT nflags) {
    int button = (MK_LBUTTON & nflags) ? 1 : 
                  (MK_MBUTTON & nflags) ? 3 : 
                  (MK_RBUTTON & nflags) ? 2 : 
                                        0;
  return button;
}

int Win32KeyFlags(UINT nFlags) {
  UINT flags = 0;
  if (GetKeyState(VK_SHIFT) & 0x8000) {
    flags |= MK_SHIFT;
  }
  if (GetKeyState(VK_CONTROL) & 0x8000) {
    flags |= MK_CONTROL;
  }
  if (nFlags == 13) {
    flags |= MK_ALT;
  }
  return flags;
}

  void OnFocusChange(UINT nID) {
    Item::WeakPtr item = FindById(nID);      
    if (!item.expired()) {
      Canvas* c = canvas().lock().get();            
      if (c) {
        c->SetFocus(item.lock());
      }
    }      
  }
             
 private:     
  static void SetFocusHook();
  static void ReleaseHook();
  static LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam);    
  static Item::WeakPtr FindById(int id) {
    std::map<std::uint16_t, Item::WeakPtr>::const_iterator it 
       = mfc_ctrls_.find(id);
    return it!=mfc_ctrls_.end() ? it->second : Item::nullpointer;
  }
  static void FilterHook(HWND hwnd);

  Canvas::WeakPtr canvas_;
  CBitmap bmpDC;
  static HHOOK _hook;
  static std::map<HWND, View*> views_;
  static std::map<std::uint16_t, Item::WeakPtr> mfc_ctrls_; 
};


class Frame;

class FrameAdaptee : public CFrameWnd {
 DECLARE_DYNAMIC(FrameAdaptee)
 public:
  typedef boost::shared_ptr<FrameAdaptee> Ptr;
  static FrameAdaptee* CreateInstance(Frame* frame) {
    FrameAdaptee* wnd(new FrameAdaptee(frame));
    wnd->Create(NULL, "PsycleLuaPlugin", WS_OVERLAPPEDWINDOW,
    CRect(120, 100, 700, 480), ::AfxGetMainWnd());
    return wnd;
  }  
  static void DestroyInstance(FrameAdaptee* adaptee) {
    adaptee->DestroyWindow();
  }
  virtual ~FrameAdaptee() {} // Use OnDestroy
  void set_canvas(Canvas::WeakPtr canvas) { assert(pView_); pView_->set_canvas(canvas); } 
  Canvas::WeakPtr canvas() { return pView_ ? pView_->canvas() : null_wptr; } 
  
protected:   
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs); 
  int OnCreate(LPCREATESTRUCT lpCreateStruct);	
  
  BOOL OnEraseBkgnd(CDC* pDC) { return TRUE; }
  void OnClose();
  virtual void OnDestroy();
 private:
  FrameAdaptee(Frame* frame) : frame_(frame), pView_(0) {}
  Frame* frame_;
  View* pView_; 
  Canvas::WeakPtr null_wptr;
 DECLARE_MESSAGE_MAP()	         
};

class Frame : public AdapterWithCreate<Frame, Item, FrameAdaptee> {
 public:
  typedef boost::shared_ptr<Frame> Ptr;
  void set_canvas(Canvas::WeakPtr canvas) { adaptee_->set_canvas(canvas); }
  Canvas::WeakPtr canvas() { return adaptee_->canvas(); } 
  void SetTitle(const std::string& title) { adaptee_->SetWindowTextA(_T(title.c_str())); }    
  virtual void Show() { adaptee_->ShowWindow(SW_SHOW); }
  virtual void Hide() { adaptee_->ShowWindow(SW_HIDE); }
  virtual void SetPos(double x, double y, double width, double height) {
    adaptee_->MoveWindow(x, y, width, height);
  }
  // Events
  virtual void OnTimerTick() {}
  virtual int OnFrameClose() { return 0; }  
  virtual void OnFrameTimer() {}
};


} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle