// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "PsycleConfig.hpp"

#undef GetGValue
#define GetGValue(rgb) (LOBYTE((rgb) >> 8))

namespace psycle {
namespace host {
namespace ui {
typedef int32_t ARGB;
#define ToCOLORREF(argb) RGB((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, (argb >> 0) & 0xFF)
#define GetAlpha(argb) (argb>>24) & 0xFF

typedef std::pair<double, double> Point;
typedef std::vector<std::pair<double, double> > Points;

enum CursorStyle {
  AUTO, MOVE, NO_DROP, COL_RESIZE, ALL_SCROLL, POINTER, NOT_ALLOWED,
  ROW_RESIZE, CROSSHAIR, PROGRESS, E_RESIZE, NE_RESIZE, DEFAULT, TEXT,
  N_RESIZE, NW_RESIZE, HELP, VERTICAL_TEXT, S_RESIZE, SE_RESIZE, INHERIT,
  WAIT, W_RESIZE, SW_RESIZE
};

struct Font {
  Font() : name("Arial"), height(12) {}
  std::string name;
  int height;
};

class Region {
 public:
  Region() {}
  Region(int x, int y, int width, int height) {}
  virtual ~Region() = 0;
  virtual Region* Clone() const = 0;

  virtual void Offset(int dx, int dy) = 0;
  virtual int Combine(const Region& other, int combinemode) = 0;
  virtual void GetBoundRect(double& x, double& y, double& width, double& height) const = 0;
  virtual bool Intersect(int x, int y) const = 0;
  virtual bool IntersectRect(double x, double y, double width, double height) const = 0;
  virtual void Clear() = 0;
  virtual void SetRect(double x, double y, double width, double height) = 0;
  virtual void* source() = 0;
  virtual const void* source() const = 0;
private:
  Region& operator = (const Region& other) {/* do nothing */}
  Region(const Region& other) {/* do nothing */}
};

inline Region::~Region() {}

class Image {
 public:
  Image() {}
  virtual ~Image() = 0;

  virtual void Load(const std::string& filename) = 0;
  virtual void SetTransparent(bool on, ARGB color) = 0;
  virtual double width() const = 0;
  virtual double height() const = 0;
  virtual void* source() = 0;
  virtual void* mask() = 0;
  virtual const void* mask() const = 0;
};

inline Image::~Image() { }

class Graphics {
 public:
  Graphics() {}
  virtual ~Graphics() = 0;

  virtual void CopyArea(int x, int y, int width, int height, int dx, int dy) = 0;
  virtual void DrawLine(int x1, int y1, int x2, int y2) = 0;
  virtual void DrawRect(int x, int y, int width, int height) = 0;
  virtual void DrawRoundRect(int x, int y, int width, int height, int arc_width, int arch_height) = 0;
  virtual void DrawOval(int x, int y, int width, int height) = 0;
  virtual void DrawString(const std::string& str, int x, int y) = 0;
  virtual void FillRect(int x, int y, int width, int height) = 0;
  virtual void FillRoundRect(int x, int y, int width, int height, int arc_width, int arch_height) = 0;
  virtual void FillOval(int x, int y, int width, int height) = 0;
  virtual void SetColor(ARGB color) = 0;
  virtual ARGB color() const = 0;
  virtual void Translate(double x, double y) = 0;
  virtual void SetFont(const Font& font) = 0;
  virtual Font font() const = 0;
  virtual void DrawPolygon(const Points& points) = 0;
  virtual void FillPolygon(const Points& points) = 0;
  virtual void DrawPolyline(const Points& points) = 0;
  virtual void DrawImage(Image* img, int x, int y) = 0;
  virtual void DrawImage(Image* img, int x, int y, int width, int height) = 0;
  virtual void DrawImage(Image* img, int x, int y, int width, int height, int xsrc, int ysrc) = 0;
  virtual void SetClip(int x, int y, int width, int height)=0;
  virtual void SetClip(Region* rgn)=0;
  virtual CRgn& clip() = 0;
};

inline Graphics::~Graphics() { }

class Menu;

class MenuBar {
 public:
  MenuBar() {}
  virtual ~MenuBar() = 0;
  virtual void add(Menu* menu) = 0;
  virtual void append(Menu* menu) = 0;
  virtual void remove(CMenu* menu, int pos) = 0;
  virtual bool needsupdate() const = 0;
  virtual void setupdate(bool update) = 0;
};

inline MenuBar::~MenuBar() {}

class MenuItem {
 public:
  static int id_counter;
  static std::map<std::uint16_t, ui::MenuItem*> menuItemIdMap;
  MenuItem() {}
  virtual ~MenuItem() = 0;
  virtual void set_id(int id) = 0;
  virtual int id() const = 0;
  virtual void set_label(const std::string& label) = 0;
  virtual const std::string& label() const = 0;
  virtual void check() = 0;
  virtual void uncheck() = 0;
  virtual bool checked() const = 0;
  virtual void set_menu(Menu* menu) = 0;
};

inline MenuItem::~MenuItem() {}

class Menu { 
 public:
  Menu() {}  
  virtual ~Menu() = 0; 
  virtual void set_label(const std::string& label) = 0;
  virtual const std::string& label() const = 0; 
  virtual CMenu* cmenu() = 0;
  virtual void setcmenu(CMenu* menu) = 0;
  virtual void set_parent(Menu* parent) = 0;
  virtual Menu* parent() = 0;
  virtual int pos() const = 0;
  virtual void set_pos(int pos) = 0;
  virtual void setbar(MenuBar* bar) = 0;
  virtual MenuBar* bar() = 0;
  virtual void add(Menu* newmenu) = 0;
  virtual void add(MenuItem* item) = 0;
  virtual void addseparator() = 0;
  virtual void remove(MenuItem* item) = 0;
};

inline Menu::~Menu() {}

namespace mfc {
class Region : public ui::Region {
 public:
  Region() { Init(0, 0, 0, 0); }
  Region(int x, int y, int width, int height) { Init(x, y, width, height); }  
  Region(const CRgn& rgn)  {
    assert(rgn.m_hObject);
    Init(0, 0, 0, 0);
    rgn_.CopyRgn(&rgn);
  }
  ~Region() { rgn_.DeleteObject(); }
  virtual Region* Clone() const { return new Region(rgn_); }  
  void Offset(int dx, int dy) {
    CPoint pt(dx, dy);
    rgn_.OffsetRgn(pt);
  }
  int Combine(const ui::Region& other, int combinemode) {
    return rgn_.CombineRgn(&rgn_, (const CRgn*)other.source(), combinemode);
  }
  void GetBoundRect(double& x, double& y, double& width, double& height) const {
    CRect rc;
    rgn_.GetRgnBox(&rc);
    x = rc.left; y = rc.top; width = rc.Width(); height = rc.Height();
  }
  bool Intersect(int x, int y) const { return rgn_.PtInRegion(x, y); }
  bool IntersectRect(double x, double y, double width, double height) const {
    CRect rc(x, y, x+width, y+height);
    return rgn_.RectInRegion(rc);
  }
  virtual void SetRect(double x, double y, double width, double height) {
    rgn_.SetRectRgn(x, y, x+width, y+height);
  }
  void Clear() { SetRect(0, 0, 0, 0); }
  void* source() { return &rgn_; }
  const void* source() const { return &rgn_; };

 private:
   void Init(int x, int y, int width, int height) {
   rgn_.CreateRectRgn(x, y, x+width, y+height);
 }
 CRgn rgn_;
};

class Image : public ui::Image {
 public:
  Image() { Init(); }
  Image(CBitmap* bmp) {
    Init();
    bmp_ = bmp;
    shared_ = true;      
  }
  ~Image() {
    if (!shared_ && bmp_) {
      bmp_->DeleteObject();
      delete bmp_;
    }    
    if (mask_.m_hObject) {
      mask_.DeleteObject();
    }
  }

  void Load(const std::string& filename) {
    if (!shared_ && bmp_) bmp_->DeleteObject();
    shared_ = false;
    CImage image;
    HRESULT r;
    r = image.Load(_T(filename.c_str()));
    if (r!=S_OK) throw std::runtime_error(std::string("Error loading file:") + filename.c_str());    
    bmp_ = new CBitmap();
    bmp_->Attach(image.Detach());    
  }
  double width() const { 
    if (bmp_) {
      BITMAP bm;
      bmp_->GetBitmap(&bm);
      return bm.bmWidth;
    }
    return 0;
  }
  double height() const { 
    if (bmp_) {
      BITMAP bm;
      bmp_->GetBitmap(&bm);      
      return bm.bmHeight;
    }
    return 0;
  }
  void SetTransparent(bool on, ARGB color) {
    if (mask_.m_hObject) {
      mask_.DeleteObject();      
    }
    if (on) {      
      PrepareMask(bmp_, &mask_, ToCOLORREF(color));
    }
  }

 private:
  void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);
  void* source() { return bmp_; }
  void* source() const { return bmp_; }
  void* mask() { return mask_.m_hObject ? &mask_ : 0; }
  const void* mask() const { return mask_.m_hObject ? &mask_ : 0; }
  void Init() {
    bmp_ = 0;    
    shared_ = false;
  }
  CBitmap *bmp_, mask_;
  bool shared_;
};

class Graphics : public ui::Graphics {
 public:
  Graphics(CDC* cr) : cr_(cr), color_(200), brush(ToCOLORREF(200)) {
    assert(cr);
    pen.CreatePen(PS_SOLID, 1, ToCOLORREF(color_));
    cr->SetTextColor(ToCOLORREF(color_));
    old_pen = cr->SelectObject(&pen);
    old_brush = cr->SelectObject(&brush);
    cr->GetWorldTransform(&rXform);
    cr->SetBkMode(TRANSPARENT);
    LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));
    lfLogFont.lfHeight = 12;
    strcpy(lfLogFont.lfFaceName, "Arial");
    cfont.CreateFontIndirect(&lfLogFont);
    old_font = (CFont*) cr->SelectObject(&cfont);
    old_rgn_.CreateRectRgn(0, 0, 0, 0);
    clp_rgn_.CreateRectRgn(0, 0, 0, 0);
    ::GetRandomRgn(cr->m_hDC, old_rgn_, SYSRGN);
    POINT pt = {0,0};
    HWND hwnd = ::WindowFromDC(cr->m_hDC);
    ::MapWindowPoints(NULL, hwnd, &pt, 1);
    old_rgn_.OffsetRgn(pt);
  }

  virtual ~Graphics() {
    cr_->SelectObject(old_pen);
    pen.DeleteObject();
    brush.DeleteObject();
    cfont.DeleteObject();
    cr_->SetGraphicsMode(GM_ADVANCED);
    cr_->SetWorldTransform(&rXform);
    cr_->SelectObject(old_font);
    cr_->SetBkMode(OPAQUE);
    cr_->SelectClipRgn(&old_rgn_);
    old_rgn_.DeleteObject();
    clp_rgn_.DeleteObject();
  }

  void DrawString(const std::string& str, int x, int y) {
    cr_->TextOut(x, y, str.c_str());
  }

  void CopyArea(int x, int y, int width, int height, int dx, int dy) {
    cr_->BitBlt(x+dx, y+dy, width, height, cr_, x, y, SRCCOPY);
  }

  void Translate(double x, double y) {
    XFORM rXform_new = rXform;
    rXform_new.eDx = rXform.eDx + x;
    rXform_new.eDy = rXform.eDy + y;
    cr_->SetGraphicsMode(GM_ADVANCED);
    cr_->SetWorldTransform(&rXform_new);
  }

  void SetColor(ARGB color) {
    color_ = color;
    pen.DeleteObject();
    pen.CreatePen(PS_SOLID, 1, ToCOLORREF(color));
    cr_->SelectObject(&pen);
    brush.DeleteObject();
    brush.CreateSolidBrush(ToCOLORREF(color));
    cr_->SelectObject(&brush);
    cr_->SetTextColor(ToCOLORREF(color_));
  }

  ARGB color() const { return color_; }

  void DrawLine(int x1, int y1, int x2, int y2) {
    cr_->MoveTo(x1, y1);
    cr_->LineTo(x2, y2);
  }

  void DrawRect(int x, int y, int width, int height) {
    CRect rc(x, y, x+width, y+height);
    cr_->FrameRect(rc, &brush);
  }

  void DrawRoundRect(int x, int y, int width, int height, int arc_width, int arc_height) {
    cr_->SelectStockObject(NULL_BRUSH);
    cr_->RoundRect(x, y, x+width, y+height, arc_width, arc_height);
    cr_->SelectObject(&brush);
  }

  void DrawOval(int x, int y, int width, int height) {
    cr_->SelectStockObject(NULL_BRUSH);
    cr_->Ellipse(x, y, x+width, y+height);
    cr_->SelectObject(&brush);
  }

  void FillRoundRect(int x, int y, int width, int height, int arc_width, int arc_height) {
    cr_->RoundRect(x, y, x+width, y+height, arc_width, arc_height);
  }

  void FillRect(int x, int y, int width, int height) {
    cr_->Rectangle(x, y, x+width, y+height);
  }

  void FillOval(int x, int y, int width, int height) {
    cr_->Ellipse(x, y, x+width, y+height);
  }

  void DrawPolygon(const ui::Points& points) {
    cr_->SelectStockObject(NULL_BRUSH);
    std::vector<CPoint> lpPoints;
    ui::Points::const_iterator it = points.begin();
    const int size = points.size();
    for (; it!=points.end(); ++it) {
      lpPoints.push_back(CPoint(it->first, it->second));
    }
    cr_->Polygon(&lpPoints[0], size);
    cr_->SelectObject(&brush);
  }

  void FillPolygon(const ui::Points& points) {
    std::vector<CPoint> lpPoints;
    ui::Points::const_iterator it = points.begin();
    const int size = points.size();
    for (; it!=points.end(); ++it) {
      lpPoints.push_back(CPoint(it->first, it->second));
    }
    cr_->Polygon(&lpPoints[0], size);
  }

  void DrawPolyline(const ui::Points& points) {
    std::vector<CPoint> lpPoints;
    ui::Points::const_iterator it = points.begin();
    const int size = points.size();
    for (; it!=points.end(); ++it) {
      lpPoints.push_back(CPoint(it->first, it->second));
    }
    cr_->Polyline(&lpPoints[0], size);
  }

  void SetFont(const Font& fnt) {
    LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));
    lfLogFont.lfHeight = fnt.height;
    strcpy(lfLogFont.lfFaceName, fnt.name.c_str());
    cfont.DeleteObject();
    cfont.CreateFontIndirect(&lfLogFont);
    cr_->SelectObject(&cfont);
  }

  Font font() const {
    LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));
    cfont.GetLogFont(&lfLogFont);
    Font fnt;
    fnt.name = lfLogFont.lfFaceName;
    fnt.height = lfLogFont.lfHeight;
    return fnt;
  }

  void DrawImage(ui::Image* img, int x, int y) {
    DrawImage(img, x, y, img->width(), img->height(), 0, 0);
  }

  void DrawImage(ui::Image* img, int x, int y, int width, int height) {
    DrawImage(img, x, y, width, height, 0, 0);
  }

  void DrawImage(ui::Image* img, int x, int y, int width, int height, int xsrc, int ysrc) {
    CDC memDC;
    CBitmap* oldbmp;
    memDC.CreateCompatibleDC(cr_);
    oldbmp = memDC.SelectObject((CBitmap*)img->source());
    if (!img->mask()) {
      cr_->BitBlt(x, y, width, height, &memDC, xsrc, ysrc, SRCCOPY);
    } else {
      TransparentBlt(cr_, x,  y, width, height, &memDC, (CBitmap*) img->mask(), xsrc, ysrc);
    }
    memDC.SelectObject(oldbmp);
    memDC.DeleteDC();
  }

  void SetClip(int x, int y, int width, int height) {
    CRgn rgn;
    rgn.CreateRectRgn(x, y, x+width, y+height);
    cr_->SelectClipRgn(&rgn);
    rgn.DeleteObject();
  }

  void SetClip(ui::Region* rgn) {
    rgn ? cr_->SelectClipRgn((CRgn*) rgn->source()) : cr_->SelectClipRgn(0);
  }

  CRgn& clip() {
    HDC hdc = cr_->m_hDC;
    ::GetRandomRgn(hdc, clp_rgn_, SYSRGN);
    POINT pt = {0,0};
    // todo test this on different windows versions
    HWND hwnd = ::WindowFromDC(hdc);
    ::MapWindowPoints(NULL, hwnd, &pt, 1);
    clp_rgn_.OffsetRgn(pt.x, pt.y);
    return clp_rgn_;
  }

  private:
   void TransparentBlt(CDC* pDC,
     int xStart,
     int yStart,
     int wWidth,
     int wHeight,
     CDC* pTmpDC,
     CBitmap* bmpMask,
     int xSource = 0,
     int ySource = 0
   );
   CDC* cr_;
   CPen pen, *old_pen;
   CBrush brush, *old_brush;
   mutable CFont cfont;
   CFont* old_font;
   ARGB color_;
   XFORM rXform;
   CRgn old_rgn_, clp_rgn_;
};

class Menu;

class MenuBar : public ui::MenuBar {
 public:
  MenuBar() : update_(false) {}
  ~MenuBar() {}
  void add(ui::Menu* menu) { items.push_back(menu); }
  void append(ui::Menu* menu);
  void remove(CMenu* menu, int pos);
  bool needsupdate() const { return update_; }
  void setupdate(bool update) { update_ = update; }
 private:
  std::vector<ui::Menu*> items;
  bool update_;
};

class MenuItem : public ui::MenuItem {
 public:
  MenuItem() : menu_(0), id_(-1), check_(false) {}
  ~MenuItem() {}
  void set_id(int id) { id_ = id; }
  int id() const { return id_; }
  void set_label(const std::string& label);
  const std::string& label() const { return label_; }
  void check();
  void uncheck();
  bool checked() const { return check_; }
  void set_menu(ui::Menu* menu) { menu_ = menu; }  
 private:
  ui::Menu* menu_;
  int id_;
  bool check_;
  std::string label_;
};

class Menu : public ui::Menu {
 public:
  Menu() : parent_(0), pos_(-1), bar_(0), owner_(true) {
    cmenu_ = new CMenu();
  }
  Menu(CMenu* menu) : cmenu_(menu), parent_(0), pos_(-1), bar_(0), owner_(false) {}
  ~Menu() {
    if (owner_) delete cmenu_;
  }
  void set_label(const std::string& label);
  const std::string& label() const { return label_; }
  CMenu* cmenu() { return cmenu_; }
  void setcmenu(CMenu* menu) { cmenu_ = menu; }
  void set_parent(ui::Menu* parent) { parent_ = parent; }
  ui::Menu* parent() { return parent_; }
  int pos() const { return pos_; }
  void set_pos(int pos) { pos_ = pos; }
  void setbar(ui::MenuBar* bar) { bar_ = bar; }
  ui::MenuBar* bar() { return parent() ? parent()->bar() : bar_; }
  void add(ui::Menu* newmenu);
  void add(ui::MenuItem* item);
  void addseparator();
  void remove(ui::MenuItem* item);

 private:
  std::vector<ui::MenuItem*> items;
  std::string label_;
  CMenu* cmenu_;
  ui::Menu* parent_;
  ui::MenuBar* bar_;
  bool owner_;
  int pos_;
};

} // namespace mfc

namespace canvas {
using namespace ui;

class Item;
class Canvas;

enum CanvasMsg { ONWND };

struct Event {  
  enum Type {
    BUTTON_PRESS,
    BUTTON_2PRESS,
    BUTTON_RELEASE,
    MOTION_NOTIFY,
    MOTION_OUT,
    KEY_DOWN,
    KEY_UP,
    SCROLL,
    ONSIZE,
    ONTIMER
  };
  Event(Item* item, Event::Type type, int cx, int cy, int button, unsigned int shift)
       : item_(item), type_(type), cx_(cx), cy_(cy), button_(button), shift_(shift) {}
  Event::Type type() const { return type_; }
  void setcxy(double x, double y) { cx_ = x; cy_ = y; }
  double cx() const { return cx_; }
  double cy() const { return cy_; }
  unsigned int button() const { return button_; }
  unsigned int shift() const { return shift_; }
  void setitem(Item* item) { item_ = item; }
  Item* item() const { return item_; }
 private:
  Item* item_;
  Event::Type type_;
  double cx_, cy_;
  unsigned int button_, shift_;
};

class Item {
public:
  Item() 
    : rgn_(new mfc::Region),
      clp_rgn_(new mfc::Region),
      fls_rgn_(new mfc::Region) { 
        Init();
  }
  Item(class Group* parent) 
    : rgn_(new mfc::Region),
      clp_rgn_(new mfc::Region),
      fls_rgn_(new mfc::Region) {
        Init(parent);
  }
  Item(class Group* parent, double x, double y) 
    : rgn_(new mfc::Region),
      clp_rgn_(new mfc::Region),
      fls_rgn_(new mfc::Region)  { 
        Init(parent, x, y);
  }
  virtual ~Item();
  
  typedef std::vector<Item*>::iterator iterator;
  virtual iterator begin() { return dummy.begin(); }
  virtual iterator end() { return dummy.end(); }
  virtual bool empty() const { return true; }

  virtual void Draw(Graphics* g, Region& repaint_region, class Canvas* widget) {};
  virtual bool OnEvent(Event* ev) { return 0; }
  void GetFocus();
  void Detach();
  virtual void Show() { if (!visible_) { STR(); visible_ = true; FLS(); } }
  virtual void Hide() { if (visible_) { STR(); visible_ = false; FLS(); } }
  // regions
  void STR();  // store old region
  void FLS();  // invalidate combine new & old region
  void FLS(const Region& rgn); // invalidate region
  virtual void needsupdate();
  virtual const Region& region() const { 
    if (update_) {    
      update_ = false;
      const_cast<Item*>(this)->onupdateregion();    
    }            
    return *rgn_.get();
  }
  virtual void onupdateregion() { needsupdate(); }
  virtual void GetBoundRect(double& x, double& y, double& width, double& height) const {
    region().GetBoundRect(x, y, width, height);
  }
  virtual Item* Intersect(double x, double y, Event* ev, bool& worked) {
    if (update_) { region(); }
    return rgn_->Intersect(x, y) ? this : 0;    
  }
  bool visible() const { return visible_; }
  virtual void SetXY(double x, double y) {
    if (x_!=x || y_!=y) { 
      STR(); x_ = x; y_ = y; FLS();
    }
  }

  double x() const { return x_; }
  double y() const { return y_; }
  void pos(double& xv, double& yv) const { xv = x(); yv = y(); }
  void clientpos(double& xv, double& yv) const { xv = zoomabsx(); yv = zoomabsy(); }
  virtual double zoomabsx() const;
  virtual double zoomabsy() const;
  virtual double acczoom() const;  
  virtual void SetSize(double w, double h) {}

  void EnablePointerEvents() { pointer_events_ = true; }
  void DisablePointerEvents() { pointer_events_ = false; }
  bool pointerevents() const { return pointer_events_; }

  Canvas* canvas();
  virtual Canvas* widget() const { return 0; }
  void set_parent(class Group* parent) { parent_ = parent; }
  Group* parent() { return parent_; }
  const Group* parent() const { return parent_; }
  bool IsInGroup(Group* group) const;
                      
  void set_manage(bool on) { managed_ = on; }
  bool managed() const { return managed_; }
    
  void SetClip(int x, int y, int width, int height) {
    clp_rgn_->SetRect(x, y, width, height);
    has_clip_ = true;
  }
  bool has_clip() const { return has_clip_; }
  const Region& clip() const { return *clp_rgn_.get(); }
  void RemoveClip() { has_clip_ = false; }
  virtual void OnMessage(CanvasMsg msg) {};
  
protected:
  void CheckButtonPress();
  void CheckFocusItem();  
  void swap_smallest(double& x1, double& x2) const {
    if (x1 > x2) {
      std::swap(x1, x2);
     }
  }          
  mutable bool update_, has_clip_;
  mutable std::auto_ptr<Region> rgn_;
  std::auto_ptr<Region> clp_rgn_;  
  double x_, y_;

private:
  void Init(Group* parent_ = 0, double x = 0, double y = 0);
  Group* parent_;
  std::auto_ptr<Region> fls_rgn_;
  bool managed_, visible_, pointer_events_, has_store_;
  std::vector<Item*> dummy;
};

class Canvas;

class Group : public Item {
 public:
  friend Canvas;
  friend Item;
  Group();
  Group(Canvas* widget);
  Group(Group* parent, double x, double y);
  ~Group();

  typedef std::vector<Item*>::iterator iterator;
  virtual iterator begin() { return items_.begin(); }
  virtual iterator end() { return items_.end(); }
  virtual bool empty() const { return items_.empty(); }
  virtual bool size() const { return items_.size(); }

  void Add(Item* item);
  void Insert(iterator it, Item* item);
  void Remove(Item* item);
  void RemoveAll() { STR(); items_.clear(); FLS(); }
  void Clear();  
    
  virtual void Draw(Graphics* g, Region& repaint_region, Canvas* widget);
  virtual Item* Intersect(double x, double y, Event* ev, bool &worked);

  void RaiseToTop(Item* item) {  Remove(item); Add(item); }
  void set_zorder(Item* item, int z);
  int zorder(Item* item) const;
  void setzoom(double zoom) { zoom_ = zoom; }
  virtual double zoom() const { return zoom_; }
  virtual void onupdateregion();

  bool work_item_event(Item* item, Event* ev);    
  bool is_root() const { return is_root_; }
        
  Canvas* widget() { return widget_; }
  virtual Canvas* widget() const { return widget_; } 
  virtual void OnMessage(CanvasMsg msg);
 protected:
  std::vector<Item*> items_;
 private:
  void Init();
  Canvas* widget_;
  double x_, y_, zoom_;
  bool is_root_;  
};

class Rect : public Item {
 public:
  Rect::Rect() : Item() { Init(); }
  Rect::Rect(Group* parent) : Item(parent) { Init(); }
  Rect(Group* parent, double x1, double y1, double x2, double y2 );
  virtual ~Rect() {}

  virtual void Draw(Graphics* g, Region& repaint_region, Canvas* widget);

  void SetPos(double x1, double y1, double width, double height) {
    STR();
    x_ = x1;
    y_ = y1;
    width_ = width;
    height_ = height;
    FLS();
  }
  double width() const { return width_; }
  double height() const { return height_; }
  // fill colors
  void SetFillColor(ARGB color) { fillcolor_ = color; FLS(); }
  ARGB fillcolor() const { return fillcolor_; }
  // stroke colors
  void SetStrokeColor(ARGB color) { strokecolor_ = color; FLS(); }
  ARGB strokecolor() const { return strokecolor_; }
  void SetBorder(double bx, double by) { STR(); bx_ = bx; by_ = by; FLS(); }
  void border(double &bx, double &by) const { bx = bx_; by = by_; }  
  virtual void onupdateregion();
 private:
  void Init();
  // bool paintRect(CDC &hdc, RECT dim, COLORREF penCol, COLORREF brushCol, unsigned int opacity);
  double width_, height_, bx_, by_;
  ARGB fillcolor_, strokecolor_;
};

class Pic : public Item {
 public:
  Pic() : Item() {}
  Pic(Group* parent) : Item(parent) { Init(); }
  Pic(Group* parent, double x, double y, Image* image);
  ~Pic() {}

  virtual void Draw(Graphics* g, Region& repaint_region, Canvas* widget);
  virtual void onupdateregion();
  void SetSize(int width, int height) {
    STR();
    width_ = width;
    height_ = height;
    FLS();
  }
  void SetSource(int xsrc, int ysrc) { xsrc_ = xsrc; ysrc_ = ysrc; FLS(); }
  void SetImage(Image* image);
  int width() const { return width_; }
  int height() const { return height_; }  

 private:
  void Init();  
  int width_, height_, xsrc_, ysrc_;  
  bool transparent_, pmdone;  
  Image* image_;
};

class Line : public Item {
 public:
  Line() : Item(), color_(0) {}
  Line(Group* parent) : Item(parent), color_(0) {}
  virtual ~Line() {}

  virtual void Draw(Graphics* g, Region& repaint_region, Canvas* widget);
  virtual Item* Intersect(double x, double y, Event* ev, bool &worked);  
  void SetPoints(const Points& pts) {  STR(); pts_ = pts; FLS(); }
  void SetPoint(int idx, const Point& pt) { STR(); pts_[idx] = pt; FLS(); }
  const Points& points() const { return pts_; }
  const Point& PointAt(int index) const { return pts_.at(index); }
  void SetColor(ARGB color) { color_ = color; FLS(); }
  ARGB color() const { return color_; }
  virtual void onupdateregion();
 private:
  Points pts_;
  ARGB color_;
};

class Text : public Item {
 public:
  Text() : Item() { Init(1.0); }
  Text(Group* parent);
  Text(Group* parent, const std::string& text);
  virtual ~Text() { }

  virtual void Draw(Graphics* cr, Region& repaint_region, Canvas* widget);
  virtual void onupdateregion();
  void SetText(const std::string& text) { STR(); text_ = text; FLS(); }
  const std::string& text() const { return text_; }
  void SetColor(ARGB color) { color_ = color; FLS(); }
  ARGB color() const { return color_; }
  void SetFont(const CFont& font) {
    LOGFONT lf;
    const_cast<CFont&>(font).GetLogFont(&lf);
    font_.DeleteObject();
    font_.CreateFontIndirect(&lf);
  }
  
 private:
  void Init(double zoom);
  std::string text_;
  ARGB color_;
  CFont font_;
  mutable int text_w, text_h;
};

// mfc wraper
template <class T>
class CWndItem : public ui::canvas::Item {
 public:  
  CWndItem() : ui::canvas::Item() { Init(); }
  CWndItem(ui::canvas::Group* parent) : ui::canvas::Item(parent) { Init(); }
  virtual ~CWndItem() { control_.DestroyWindow(); }

  virtual void Draw(ui::Graphics* g, ui::Region& repaint_region,
        ui::canvas::Canvas* widget) {
/*    if (control_) {
      double cx = zoomabsx();
      double cy = zoomabsy();      
      CRect rc;
      control_->GetWindowRect(&rc);
      control_->GetParent()->ScreenToClient(&rc);
      if (rc.left != cx || rc.top != cy) {
        control_->SetWindowPos(control_->GetParent(), cx, cy, cx, cy, SWP_NOSIZE | SWP_NOZORDER);
      }
    }*/
  }     
  virtual void onupdateregion() { rgn_->SetRect(0, 0, w_+1, h_+1); }
  virtual void OnMessage(ui::canvas::CanvasMsg msg) {
    if (msg == ui::canvas::ONWND) {
      CWnd* wnd = canvas()->wnd();
      if (p_wnd_!= wnd) {
        p_wnd_ = wnd;
        int cx = zoomabsx();
        int cy = zoomabsy();        
        control_.SetParent(p_wnd_);
        SetXY(cx, cy);        
        control_.ShowWindow(SW_SHOW);
      }
    }
  }  
  virtual void SetXY(double x, double y) {
    x_ = x;
    y_ = y;          
    int cx = zoomabsx();
    int cy = zoomabsy();
    control_.SetWindowPos(p_wnd_, cx, cy, cx, cy, SWP_NOSIZE | SWP_NOZORDER);        
  }  
  CWnd* p_wnd() { return p_wnd_; }
  const CWnd* p_wnd() const { return p_wnd_; }    
  int id() const { return id_; }    
  static CWndItem<T>* FindById(int id) {
    std::map<std::uint16_t, CWndItem<T>*>::const_iterator it 
      = id_map_.find(id);
    return it!=id_map_.end() ? it->second : 0;
  }
 protected:
   static int newid() { return id_counter++; }
   T& ctrl() { return control_; }
   const T& ctrl() const { return control_; }
 private:
  void Init() {
    id_ = newid();
    id_map_[id_] = this;      
    p_wnd_ = ::AfxGetMainWnd();
    w_= 200; h_= 20;
    needsupdate();
  }
  int w_, h_, id_;
  T control_;
  CWnd *p_wnd_;
  static int id_counter;
  static std::map<std::uint16_t, CWndItem<T>*> id_map_;
};

template <typename T>
int CWndItem<T>::id_counter = ID_DYNAMIC_CONTROLS_BEGIN;

template <typename T>
std::map<std::uint16_t, CWndItem<T>*> CWndItem<T>::id_map_;

class Button : public CWndItem<CButton> {
 public:
  static std::string type() { return "buttonitem"; }
  Button() : CWndItem<CButton>() { Init(); }
  Button(ui::canvas::Group* parent) : CWndItem<CButton>(parent) { Init(); }  
  virtual void OnClick() {}
private:
  void Init() {        
    ctrl().Create("btn", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | DT_CENTER,
      CRect(0, 0, 55, 19), p_wnd(), id());
  }
};

class Edit : public CWndItem<CEdit> {
 public:
  static std::string type() { return "edititem"; }
  Edit() : CWndItem<CEdit>() { Init(); }
  Edit(ui::canvas::Group* parent) : CWndItem<CEdit>(parent) { Init(); }
  void SetText(const std::string& text) { ctrl().SetWindowText(text.c_str()); }
  std::string GetText() const {    
    CString s;
    ctrl().GetWindowTextA(s);
    return s.GetString();
  }  
private:
  void Init() {
    ctrl().Create(WS_CHILD | WS_TABSTOP, CRect(0, 0, 55, 19), p_wnd(), id());
  }  
};

enum Orientation { HORZ = 0, VERT = 1 };

class ScrollBar : public CWndItem<CScrollBar> {
 public:
  static std::string type() { return "scrollbaritem"; }
  ScrollBar() : CWndItem<CScrollBar>() { Init(); }
  ScrollBar(ui::canvas::Group* parent) : CWndItem<CScrollBar>(parent) { Init(); }  
  virtual void OnScroll(int pos) {}
  virtual void SetPos(double x, double y, double w, double h) {
    x_ = x;
    y_ = y;          
    int cx = zoomabsx();
    int cy = zoomabsy();
    ctrl().SetWindowPos(p_wnd(), cx, cy, w, h, SWP_NOZORDER);        
  }
  void SetRange(int minpos, int maxpos) { 
    ctrl().SetScrollRange(minpos, maxpos);
  }
  void SetOrientation(Orientation orientation) {    
    DWORD cs = (ctrl().GetStyle() & !SB_HORZ & !SB_VERT) | orientation;
    SCROLLINFO si;
    ctrl().GetScrollInfo(&si);
    CRect rc;
    ctrl().GetClientRect(&rc);
    ctrl().DestroyWindow();        
    ctrl().Create(cs, rc, p_wnd(), id());        
  }
  void SetScrollPos(int pos) { ctrl().SetScrollPos(pos); }
  
private:
  void Init() {        
    ctrl().Create(SBS_VERT | WS_CHILD, CRect(0 , 0, 100, 30), p_wnd(), id());
  }
};

// canvas
class Canvas {
  friend class Item;
  friend class Group;
 public:
  Canvas() : wnd_(0) { Init(); }
  Canvas(CWnd* parent) : wnd_(parent) { Init(); }  
  ~Canvas();
  
  Group* root() { return root_; }
  void Draw(Graphics* g, Region& rgn);
  void DrawFlush(Graphics* g, Region& rgn);
  void Flush();
  void SetColor(ARGB color) { color_ = color; }
  ARGB color() const { return color_; }
  void set_bg_image(CBitmap* bg_image, int width, int height) {
    bg_image_ = bg_image;
    bg_width_ = width;
    bg_height_ = width;
  }
  void StealFocus(Item* item);
  virtual canvas::Item* OnEvent(Event* ev);
  virtual void OnSize(int cx, int cy) {
       cw_ = cx;
       ch_ = cy;
    }
    void SetSave(bool on) { save_ = on; }
    bool IsSaving() const { return save_; }    
    void ClearSave() { save_rgn_.Clear(); }
    void set_wnd(CWnd* wnd) {
      if (old_wnd_ != wnd) {
        old_wnd_ = wnd_ = wnd;        
        root_->OnMessage(ONWND);        
      }
    }
    CWnd* wnd() { return wnd_; }
    int cw() const { return cw_; }
    int ch() const { return ch_; }
    void setpreferredsize(double width, double height) {
      pw_ = width;
      ph_ = height;
    }
    void preferredsize(double& width, double& height) const {
      width = pw_;
      height = ph_;
    }
    void InvalidateSave() {
      if (wnd_ &&::IsWindow(wnd_->m_hWnd)) {
          wnd_->InvalidateRgn((CRgn*) save_rgn_.source(), 0);
      }
    }
  void SetCapture() {
    if (wnd_ && ::IsWindow(wnd_->m_hWnd)) {
      ::SetCapture(wnd_->m_hWnd);
    }
  }
  void ReleaseCapture() {
    if (wnd_ && ::IsWindow(wnd_->m_hWnd)) {
      ::ReleaseCapture();
    }
  }
  void ShowCursor() { while (::ShowCursor(TRUE) < 0); }
  void HideCursor() { while (::ShowCursor(FALSE) >= 0); }
  void SetCursorPos(int x, int y) {
    if (wnd_) {
      CPoint point(x, y);
      wnd_->ClientToScreen(&point);
		  ::SetCursorPos(point.x, point.y);
    }
  }
  void SetCursor(CursorStyle style) {
    LPTSTR c = 0;
    int ac = 0;
    switch (style) {
      case AUTO        : c = IDC_IBEAM; break;
      case MOVE        : c = IDC_SIZEALL; break;
      case NO_DROP     : ac = AFX_IDC_NODROPCRSR; break;
      case COL_RESIZE  : ac = AFX_IDC_HSPLITBAR; break;
      case ALL_SCROLL  : ac = AFX_IDC_TRACK4WAY; break;
      case POINTER     : c = IDC_HAND; break;
      case NOT_ALLOWED : c = IDC_NO; break;
      case ROW_RESIZE  : ac = AFX_IDC_VSPLITBAR; break;
      case CROSSHAIR   : c = IDC_CROSS; break;
      case PROGRESS    : c = IDC_APPSTARTING; break;
      case E_RESIZE    : c = IDC_SIZEWE; break;
      case NE_RESIZE   : c = IDC_SIZENWSE; break;
      case DEFAULT     : c = IDC_ARROW; break;
      case TEXT        : c = IDC_IBEAM; break;
      case S_RESIZE    : c = IDC_SIZENS; break;
      case SE_RESIZE   : c = IDC_SIZENWSE; break;
      case INHERIT     : c = IDC_IBEAM; break;
      case WAIT        : c = IDC_WAIT; break;
      case W_RESIZE    : c = IDC_SIZEWE; break;
      case SW_RESIZE   : c = IDC_SIZENESW; break;
      default          : c = IDC_ARROW; break;
    }
    cursor_ = (c!=0) ? LoadCursor(0, c) : ::LoadCursor(0, MAKEINTRESOURCE(ac));
  }

  HCURSOR cursor() const { return cursor_; }

  double zoomabsx() const { return 0; }
  double zoomabsy() const { return 0; }
  double x() const { return 0; }
  double y() const { return 0; }
  bool show_scrollbar;
  int nposv, nposh;
 protected:
  void Invalidate(Region& rgn);
 private:
  void Init();
  Canvas(const Canvas& other) {}
  Canvas& operator=(Canvas rhs) {}
  Item* DelegateEvent(Event* event, Item* item);  
  CWnd* wnd_, *old_wnd_;
  Group* root_;
  bool save_, steal_focus_, managed_;
  Item *button_press_item_,  *out_item_, *focus_item_;
  CBitmap* bg_image_;
  int bg_width_, bg_height_;
  int cw_, ch_, pw_, ph_;
  ARGB color_;
  mfc::Region save_rgn_;
  HCURSOR cursor_;
};

class CanvasFrameWnd;

int const ID_TIMER_VIEW_REFRESH = 40;

class BaseView {
 public:
  BaseView(CWnd* wnd) : canvas_(0), wnd_(wnd) { assert(wnd); } 
  virtual ~BaseView() {}
  void set_canvas(Canvas* canvas) { canvas_ = canvas; canvas_->set_wnd(wnd_); }
  Canvas* canvas() { return canvas_; }  
  void InitTimer() {
    if (!wnd_->SetTimer(ID_TIMER_VIEW_REFRESH, 10, NULL)) {
			AfxMessageBox(IDS_COULDNT_INITIALIZE_TIMER, MB_ICONERROR);
		}
  }
  void StopTimer() { wnd_->KillTimer(ID_TIMER_VIEW_REFRESH); }
  CWnd* wnd() { return wnd_; }
protected:
  void Draw();  
  bool DelegateEvent(int type, int button, UINT nFlags, CPoint pt);
private:
  Canvas* canvas_;
  CWnd* wnd_;
};

class View : public CWnd, public BaseView {
	public:
    View() : BaseView(this) {}
		virtual void Open() {};		
	protected:		
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);    
	DECLARE_MESSAGE_MAP()
		int OnCreate(LPCREATESTRUCT lpCreateStruct);
    void OnDestroy();
		void OnSetFocus(CWnd* pOldWnd);
		void OnPaint();
		void OnLButtonDown(UINT nFlags, CPoint pt) { 
      DelegateEvent(Event::BUTTON_PRESS, 1, nFlags, pt);
    }
    void OnRButtonDown(UINT nFlags, CPoint pt) {
      DelegateEvent(Event::BUTTON_PRESS, 2, nFlags, pt);
    }
		void OnLButtonDblClk(UINT nFlags, CPoint pt) {
      DelegateEvent(Event::BUTTON_2PRESS, 1, nFlags, pt);
    }
		void OnMouseMove(UINT nFlags, CPoint pt) {
      DelegateEvent(Event::MOTION_NOTIFY, 0, nFlags, pt);
    }
		void OnLButtonUp(UINT nFlags, CPoint pt) {
      DelegateEvent(Event::BUTTON_RELEASE, 1, nFlags, pt);
    }
		void OnRButtonUp(UINT nFlags, CPoint pt) {
      DelegateEvent(Event::BUTTON_RELEASE, 2, nFlags, pt);
    }
    void OnTimer(UINT_PTR nIDEvent);
    BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);         
		void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);    
    void OnCtrlBtnClick(UINT nID) {
      Button* btn = (Button*) Button::FindById(nID);
      if (btn) {
        btn->OnClick();
      }
    }
   public:
    void OnSize(UINT nType, int cx, int cy);
};

class CanvasFrameWnd : public CFrameWnd {			
 DECLARE_DYNAMIC(CanvasFrameWnd)
 public:   
  virtual ~CanvasFrameWnd() {} // Use OnDestroy
  void set_canvas(Canvas* canvas) { assert(pView_); pView_->set_canvas(canvas); }
  Canvas* canvas() { return pView_ ? pView_->canvas() : 0; }
  virtual void PostOpenWnd();
  void SetTitle(const std::string& title) { SetWindowTextA(title.c_str()); }
  void SetPos(int x, int y) { }
 protected:
  CanvasFrameWnd() : CFrameWnd(), pView_(0) {}; // protected constructor used by dynamic creation
                                                // Use OnCreate.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();
  virtual void ResizeWindow(CRect* pRect);
	virtual void GetWindowSize(CRect &rcFrame, CRect &rcClient, CRect *pRect = NULL);

  virtual void OnEvent(int ev) {}
  
 DECLARE_MESSAGE_MAP()
	int OnCreate(LPCREATESTRUCT lpCreateStruct);      
	void OnClose();
  void OnDestroy();
	void OnSetFocus(CWnd* pOldWnd);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);	
 private:
  View* pView_;
};

} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle