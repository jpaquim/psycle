// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "PsycleConfig.hpp"
#include "MfcUi.hpp"
#include "CScintilla.hpp"
#include "DesignPatterns.hpp"

namespace psycle {
namespace host {
namespace ui {
namespace canvas {

using namespace ui;

struct BlitInfo {
  double dx, dy;  
};

enum Orientation { 
  HORZ = 0, 
  VERT = 1
};

class Canvas;

class Command {
 public:
  virtual ~Command() {}
  virtual void Execute() {}
 protected:
};

class Item : public Window {
  friend class Group;
  friend class Canvas;
public:  
    

  Item() 
    : rgn_(ui::Systems::instance().CreateRegion()),
      clp_rgn_(ui::Systems::instance().CreateRegion()),
      fls_rgn_(ui::Systems::instance().CreateRegion()),       
      draw_rgn_(0) {
    x_ = y_ = 0;    
    auto_size_width_ = auto_size_height_ = true;
    visible_ = pointer_events_ = update_= true;
    has_clip_ = has_store_ = false;
    color_ = 0xFF000000;
  }
    
  virtual ~Item() {}
  
  static std::string type() { return "canvasitem"; }

  // appearance
  void DrawBackground(Graphics* g, Region& draw_region);    
  virtual ui::Rect bounds() const { return region().bounds(); }
  virtual void Show();
  virtual void Hide();
  virtual bool visible() const { return visible_; }  
  bool IsInGroupVisible() const;
  void set_ornament(boost::shared_ptr<ui::Ornament> ornament);
  // hit detection
  virtual Item::Ptr HitTest(double x, double y) {
    return region().Intersect(x, y) ? shared_from_this() : nullpointer;
  }
  
  Window::List SubItems() {
    Window::List allitems;
    iterator it = begin();
    for (; it != end(); ++it) {
      Window::Ptr item = *it;
      allitems.push_back(item);
      Window::List subs = item->SubItems();
      iterator itsub = subs.begin();
      for (; itsub != subs.end(); ++itsub) {
        allitems.push_back(*it);
      }
    }
    return allitems;
  }   
  Window* root();    
  bool IsInGroup(Item::WeakPtr group) const;  
    
  void GetFocus();
  void Detach();
  
  // regions
  void STR();  // store old region
  void FLS();  // invalidate combine new & old region
  void FLS(const Region& rgn); // invalidate region
  virtual void needsupdate();

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
    irgn->Offset(-abs_pos().left(), -abs_pos().top());
    return irgn;
  }
    
  virtual void set_pos(const ui::Point& pos) {
    STR();
    x_ = pos.x();
    y_ = pos.y();
    FLS();
  }
  void SetBlitXY(double x, double y);

  virtual ui::Rect pos() const {
    return ui::Rect(x_, y_, x_ + bounds().width(), y_ + bounds().height());
  }

  virtual void set_pos(const ui::Rect& rect) {    
    set_pos(Point(rect.left(), rect.top()));
    SetSize(rect.width(), rect.height());
  }
 
  void set_auto_size(bool auto_size_width, bool auto_size_height) {
    auto_size_width_ = auto_size_width;
    auto_size_height_ = auto_size_height;
  }

  bool auto_size_width() const { return auto_size_width_; }
  bool auto_size_height() const { return auto_size_height_; }

  void PreventFls();
  void EnableFls();
 
  void SetSize(double width, double height);
  virtual void SetHeight(double height) { 
    set_pos(ui::Rect(pos().left(), pos().top(), pos().left() + bounds().width(), pos().top() + height));   
  }
  virtual void SetWidth(double width) { 
    set_pos(ui::Rect(pos().left(), pos().top(), pos().left() + width, pos().top() + bounds().height()));   
  } 
  ui::Size dim() const {
    ui::Size d;
    if (has_style()) {
      const ui::Rect& margin = style()->margin();
      d.set_size(pos().width() + margin.left() + margin.right(),
                   pos().height() + margin.top() + margin.bottom());
    } else {
      d.set_size(pos().width(), pos().height());
    }
    return d;
  }

  virtual void OnSize(double w, double h) {}

  virtual ui::Rect abs_pos() const;

  void EnablePointerEvents() { pointer_events_ = true; }
  void DisablePointerEvents() { pointer_events_ = false; }
  bool pointerevents() const { return pointer_events_; }
                              
  void SetClip(double x, double y, double width, double height) {
    STR();            
    clp_rgn_->SetRect(x, y, width, height);
    has_clip_ = true;    
    FLS(); 
  }
  bool has_clip() const { return has_clip_; }
  const Region& clip() const { return *clp_rgn_.get(); }
  void RemoveClip() { has_clip_ = false; }
  void set_style(const ItemStyle::Ptr& style) { 
    style_ = style; 
   //  style_->update.connect(boost::bind(&Item::onalign, this));
  }

  ItemStyle::Ptr style() { 
    if (!style_) {
      style_ = ItemStyle::Ptr(new ItemStyle());
     //  style_->update.connect(boost::bind(&Item::onalign, this));
    }
    return style_;
  }

  ItemStyle::Ptr style() const { 
    if (!style_) {
      style_ = ItemStyle::Ptr(new ItemStyle());
     //  style_->update.connect(boost::bind(&Item::onalign, this));
    }
    return style_;
  }

  bool has_style() const { return style_ != 0; }  
  
  virtual void set_fill_color(ARGB color) { STR(); color_ = color; FLS(); }
  virtual ARGB fill_color() const { return color_; }

 protected:
  void swap_smallest(double& x1, double& x2) const {
    if (x1 > x2) {
      std::swap(x1, x2);
     }
  }
  
  double x_, y_;  
  mutable bool update_, has_clip_;
  mutable std::auto_ptr<Region> rgn_;
  std::auto_ptr<Region> clp_rgn_;
     
 private:        
  std::auto_ptr<Region> fls_rgn_;
  Region* draw_rgn_;
  bool visible_, pointer_events_, has_store_;  
  std::auto_ptr<BlitInfo> blit_;
  mutable ItemStyle::Ptr style_;
  boost::weak_ptr<Ornament> ornament_;
  ARGB color_;
  bool auto_size_width_, auto_size_height_;
  static boost::shared_ptr<Aligner> dummy_aligner;
};

class Canvas;

class Group : public Item {
 public:
  friend Canvas;
  friend Item;
  
  Group() : Item(), zoom_(1), is_root_(false), w_(-1), h_(-1) {}

  static std::string type() { return "canvasgroup"; }

  // structure  
  virtual Window::iterator begin() { return items_.begin(); }
  virtual Window::iterator end() { return items_.end(); }
  virtual bool empty() const { return items_.empty(); }
  virtual int size() const { return items_.size(); }

  void Add(const Item::Ptr& item);
  void Insert(iterator it, const Item::Ptr& item);
  void Remove(const Item::Ptr& item);
  void RemoveAll() { STR(); items_.clear(); FLS(); }
    
  // appearence
  void set_aligner(const boost::shared_ptr<Aligner>& aligner);  
  virtual void Draw(Graphics* g, Region& draw_region);
 
  virtual Window::Ptr HitTest(double x, double y);

  void RaiseToTop(Item::Ptr item) {  Remove(item); Add(item); }
  void set_zorder(Item::Ptr item, int z);
  int zorder(Item::Ptr item) const;
  void setzoom(double zoom) { zoom_ = zoom; }
  virtual double zoom() const { return zoom_; }
  virtual bool onupdateregion();

  bool is_root() const { return is_root_; }
  virtual void OnMessage(WindowMsg msg, int param = 0);
    
  void Align();
  boost::shared_ptr<Aligner> aligner() const { return aligner_; }
  
  virtual void set_pos(const ui::Rect& pos) {
    STR();
    x_ = pos.left();
    y_ = pos.top();
    w_ = pos.width();
    h_ = pos.height();
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

 protected:  
  Window::List items_;

 private:
  void Init();   
  double zoom_;
  bool is_root_;
  double w_, h_; 
  boost::shared_ptr<Aligner> aligner_;
};

class Aligner;

struct CalcDim { void operator()(Aligner& aligner) const; };
struct SetPos { void operator()(Aligner& aligner) const; };

class Aligner { 
 public:
  typedef boost::shared_ptr<Aligner> Ptr;
  typedef boost::weak_ptr<const Aligner> ConstPtr;
  typedef boost::weak_ptr<Aligner> WeakPtr;  
  
  Aligner() {}
  virtual ~Aligner() = 0;

  void Align() {
    static CalcDim calc_dim;
    static SetPos set_pos;    
    PostOrderTreeTraverse(calc_dim);
    PreOrderTreeTraverse(set_pos);
    //full_align_ = false;
  }

  virtual void CalcDimensions() = 0;
  virtual void SetPositions() = 0;

  virtual void set_group(const Group::WeakPtr& group) { group_ = group; }  
  const ui::Size& dim() const { return dim_; }
  const ui::Rect& pos() const { return pos_; }     
  template <class T>
  void PreOrderTreeTraverse(T& functor);
  template <class T>
  void PostOrderTreeTraverse(T& functor);
  bool full_align() const { return true; }

 protected:  
  typedef Window::List::iterator iterator;
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
   
  Group::WeakPtr group_;
  ui::Size dim_;
  ui::Rect pos_;
  
 private: 
  static Window::List dummy;
  static bool full_align_;
};

inline Aligner::~Aligner() {};

class DefaultAligner : public Aligner {
 public:
  virtual void CalcDimensions();
  virtual void SetPositions();
  virtual void Realign();
};



// canvas
class Canvas : public Group {
  friend class Item;
  friend class Group;
 public:
  typedef boost::shared_ptr<Canvas> Ptr;
  typedef boost::weak_ptr<Canvas> WeakPtr;
  //Canvas(CWnd* parent, int nID)
    //: Window(ui::mfc::WindowImp::Make(this, parent, nID)) {
  
  Canvas()
      : Group(),     
        save_rgn_(ui::Systems::instance().CreateRegion()) {    
    Init();
  }

  Canvas(CWnd* wnd, CWnd* parent, int nID) 
      : Group(), 
        save_rgn_(ui::Systems::instance().CreateRegion()) {    
    set_imp(ui::mfc::WindowImp::Make(this, parent, AFX_IDW_PANE_FIRST));
    Init();
  }

  Canvas(ui::Window* parent) 
    : Group(),
      save_rgn_(ui::Systems::instance().CreateRegion()) {
   Init(); 
  }    
  
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
    void ClearSave() { save_rgn_->Clear(); }
 
    virtual bool onupdateregion() { 
      rgn_->SetRect(0, 0, cw_, ch_);
      return true;
    }

  virtual void set_pos(const ui::Rect& pos) { Window::set_pos(pos); }

  void InvalidateSave();    
  void SetCursor(CursorStyle style);
  HCURSOR cursor() const { return cursor_; }
  
  bool show_scrollbar;
  int nposv, nposh;

  virtual void WorkMouseDown(MouseEvent& ev);
  virtual void WorkMouseUp(MouseEvent& ev);
  virtual void WorkMouseMove(MouseEvent& ev);
  virtual void WorkDblClick(MouseEvent& ev);
  virtual void WorkKeyUp(KeyEvent& ev);
  virtual bool WorkKeyDown(KeyEvent& ev); 

  boost::signal<void (std::exception&)> error;

  void Invalidate();
  void Invalidate(Region& rgn);

  bool prevent_fls() const { return prevent_fls_; }  
  virtual void OnFocusChange(int id) { OnMessage(FOCUS, id); }
  
 private:  
  void Init();
  void DoDraw(Graphics* g, Region& rgn);
  Canvas(const Canvas& other) {}
  Canvas& operator=(Canvas rhs) {}
 
  template <class T, class T1>
  void WorkEvent(T& ev, void (T1::*ptmember)(T&), Window::Ptr& item) {
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
   
  bool save_, steal_focus_, prevent_fls_;
  Window::WeakPtr button_press_item_, mouse_move_, focus_item_;
  CBitmap* bg_image_;
  int bg_width_, bg_height_;
  int cw_, ch_;
  ARGB color_;
  std::auto_ptr<ui::Region> save_rgn_;
  HCURSOR cursor_;
  bool item_blit_;
};

// class CanvasFrameWnd;

/*class View : public Window {
 public:
  View(CWnd* parent, int nID) 
    : Window(ui::mfc::WindowImp::Make(this, parent, nID)) {
  }

  boost::signal<void (std::exception&)> error;

  void set_canvas(Canvas::WeakPtr canvas);    
  Canvas::WeakPtr canvas() { return canvas_; }

  virtual void Draw(Graphics* g, Region& draw_region) {
    Canvas* c = canvas().lock().get();
    try {     
      c->Draw(g, draw_region);
    } catch (std::exception& e) {
      AfxMessageBox(e.what());
    }
  }

  virtual void OnSize(double width, double height) {
    if (!canvas_.expired()) {      
      try {
        Canvas* c = canvas().lock().get();
        c->WorkOnSize(width, height);
        c->ClearSave();
        Invalidate();
      } catch (std::exception& e) {
        AfxMessageBox(e.what());
      }  
    } 
  }

  virtual void OnMouseDown(MouseEvent& ev) {
    try {    
      if (!canvas_.expired()) {
        canvas().lock()->WorkMouseDown(ev);                  
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }
  }
  virtual void OnMouseUp(MouseEvent& ev) {
    try {    
      if (!canvas_.expired()) {
        canvas().lock()->WorkMouseUp(ev);                  
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }
  }
  virtual void OnDblclick(MouseEvent& ev) {
    try {    
      if (!canvas_.expired()) {
        canvas().lock()->WorkDblClick(ev);                  
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }
  }
  virtual void OnMouseMove(MouseEvent& ev) {
    try {    
      if (!canvas_.expired()) {
        canvas().lock()->WorkMouseMove(ev);                  
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }
  }

  virtual void OnKeyDown(KeyEvent& ev) {
    try {    
      if (!canvas_.expired()) {
        canvas().lock()->WorkKeyDown(ev);                  
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }    
  }
  
  virtual void OnKeyUp(KeyEvent& ev) {
    try {    
      if (!canvas_.expired()) {
        canvas().lock()->WorkKeyUp(ev);                  
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }
  }

  virtual void OnFocusChange(int id) {
    if (!canvas_.expired()) {
      canvas().lock()->OnMessage(FOCUS, id);
    }
  }

 private:
  Canvas::WeakPtr canvas_;
};*/

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
  void set_canvas(Canvas::WeakPtr canvas) {     
    if (!canvas.expired()) {
      canvas_ = canvas;
      ui::mfc::WindowImp* imp = (ui::mfc::WindowImp*) canvas.lock()->imp();
      if (!imp) {
        imp = ui::mfc::WindowImp::Make(canvas.lock().get(), this, AFX_IDW_PANE_FIRST);
      }
      imp->ShowWindow(SW_SHOW);
      canvas.lock()->set_imp(imp);      
      canvas.lock()->OnMessage(ui::ONWND);
    }
  } 
  Canvas::WeakPtr canvas() { return canvas_; } 
  
protected:   
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs); 
  int OnCreate(LPCREATESTRUCT lpCreateStruct);	
  
  BOOL OnEraseBkgnd(CDC* pDC) { return TRUE; }
  void OnClose();
  virtual void OnDestroy();
 private:
  FrameAdaptee(Frame* frame) : frame_(frame) {}
  Frame* frame_;
  Canvas::WeakPtr canvas_;   
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
  virtual void set_pos(const ui::Rect& pos) {
    adaptee_->MoveWindow(pos.left(), pos.top(), pos.width(), pos.height());
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