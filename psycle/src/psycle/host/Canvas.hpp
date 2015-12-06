// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "PsycleConfig.hpp"
#include "Scintilla.h"
#include "SciLexer.h"

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>


#undef GetGValue
#define GetGValue(rgb) (LOBYTE((rgb) >> 8))

namespace psycle {
namespace host {
namespace ui {
typedef int32_t ARGB;
#define ToCOLORREF(argb) RGB((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, (argb >> 0) & 0xFF)
#define GetAlpha(argb) (argb>>24) & 0xFF
#define ToARGB(r, g, b) (((ARGB) (b) << 0) | ((ARGB) (g) << 8) |((ARGB) (r) << 16) | ((ARGB) (0) << 24))


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

  virtual void Offset(double dx, double dy) = 0;
  virtual int Combine(const Region& other, int combinemode) = 0;  
  virtual void BoundRect(double& x, double& y, double& width, double& height) const = 0;
  virtual bool Intersect(double x, double y) const = 0;
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
  typedef boost::shared_ptr<Image> Ptr;
  typedef boost::weak_ptr<Image> WeakPtr;

  Image() { }
  virtual ~Image() = 0;

  virtual void Load(const std::string& filename) = 0;
  virtual void SetTransparent(bool on, ARGB color) = 0;
  virtual void Size(double& width, double& height) const = 0;  
  virtual void* source() = 0;
  virtual void* mask() = 0;
  virtual const void* mask() const = 0;
};

inline Image::~Image() { }

namespace canvas
{
class Group;
};

class Graphics {
friend class canvas::Group;
 public:
  Graphics() {}
  virtual ~Graphics() = 0;

  virtual void CopyArea(double x, double y, double width, double height, double dx, double dy) = 0;
  virtual void DrawLine(double x1, double y1, double x2, double y2) = 0;
  virtual void DrawRect(double x, double y, double width, double height) = 0;
  virtual void DrawRoundRect(double x, double y, double width, double height, double arc_width, double arch_height) = 0;
  virtual void DrawOval(double x, double y, double width, double height) = 0;
  virtual void DrawString(const std::string& str, double x, double y) = 0;
  virtual void FillRect(double x, double y, double width, double height) = 0;
  virtual void FillRoundRect(double x, double y, double width, double height, double arc_width, double arch_height) = 0;
  virtual void FillOval(double x, double y, double width, double height) = 0;
  virtual void SetColor(ARGB color) = 0;
  virtual ARGB color() const = 0;
  virtual void Translate(double x, double y) = 0;  
  virtual void SetFont(const Font& font) = 0;
  virtual Font font() const = 0;
  virtual void DrawPolygon(const Points& podoubles) = 0;
  virtual void FillPolygon(const Points& podoubles) = 0;
  virtual void DrawPolyline(const Points& podoubles) = 0;
  virtual void DrawImage(Image* img, double x, double y) = 0;
  virtual void DrawImage(Image* img, double x, double y, double width, double height) = 0;
  virtual void DrawImage(Image* img, double x, double y, double width, double height, double xsrc, double ysrc) = 0;
  virtual void SetClip(double x, double y, double width, double height)=0;
  virtual void SetClip(Region* rgn)=0;
  virtual CRgn& clip() = 0;

  virtual CDC* dc() = 0;  // just for testing right now
private:
  virtual void SaveOrigin() = 0;
  virtual void RestoreOrigin() = 0;
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
  void Offset(double dx, double dy) {
    CPoint pt(dx, dy);
    rgn_.OffsetRgn(pt);
  }
  int Combine(const ui::Region& other, int combinemode) {
    return rgn_.CombineRgn(&rgn_, (const CRgn*)other.source(), combinemode);
  }  
  void BoundRect(double& x, double& y, double& width, double& height) const {
    CRect rc;
    rgn_.GetRgnBox(&rc);
    x = rc.left; y = rc.top; width = rc.Width(); height = rc.Height();
  }
  bool Intersect(double x, double y) const { return rgn_.PtInRegion(x, y); }
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
    if (!shared_ && bmp_) {
      bmp_->DeleteObject();
    }
    shared_ = false;
    CImage image;
    HRESULT r;
    r = image.Load(_T(filename.c_str()));
    if (r!=S_OK) throw std::runtime_error(std::string("Error loading file:") + filename.c_str());    
    bmp_ = new CBitmap();
    bmp_->Attach(image.Detach());    
  }

  virtual void Size(double& width, double& height) const {
    if (bmp_) {    
      BITMAP bm;
      bmp_->GetBitmap(&bm);
      width = bm.bmWidth;
      height = bm.bmHeight;
    }    
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
  Graphics(CDC* cr) 
      : cr_(cr),
        color_(200),
        brush(ToCOLORREF(200)),
        updatepen_(false),
        updatebrush_(false) {
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

  void DrawString(const std::string& str, double x, double y) {
    check_pen_update();
    cr_->TextOut(x, y, str.c_str());
  }

  void CopyArea(double x, double y, double width, double height, double dx, double dy) {
    cr_->BitBlt(x+dx, y+dy, width, height, cr_, x, y, SRCCOPY);
  }

  void Translate(double x, double y) {
    rXform.eDx += x;
    rXform.eDy += y;
    cr_->SetGraphicsMode(GM_ADVANCED);
    cr_->SetWorldTransform(&rXform);
  }

  void SetColor(ARGB color) {
    if (color_ != color) {
      color_ = color;
      updatepen_ = updatebrush_ = true;      
    }
  }

  ARGB color() const { return color_; }

  void DrawLine(double x1, double y1, double x2, double y2) {
    check_pen_update();
    cr_->MoveTo(x1, y1);
    cr_->LineTo(x2, y2);
  }

  void DrawRect(double x, double y, double width, double height) {
    check_pen_update();
    check_brush_update();
    CRect rc(x, y, x+width, y+height);
    cr_->FrameRect(rc, &brush);
  }

  void DrawRoundRect(double x, double y, double width, double height, double arc_width, double arc_height) {
    check_pen_update();
    check_brush_update();
    cr_->SelectStockObject(NULL_BRUSH);
    cr_->RoundRect(x, y, x+width, y+height, arc_width, arc_height);
    cr_->SelectObject(&brush);
  }

  void DrawOval(double x, double y, double width, double height) {
    check_pen_update();
    check_brush_update();
    cr_->SelectStockObject(NULL_BRUSH);
    cr_->Ellipse(x, y, x+width, y+height);
    cr_->SelectObject(&brush);
  }

  void FillRoundRect(double x, double y, double width, double height, double arc_width, double arc_height) {
    check_pen_update();
    check_brush_update();
    cr_->RoundRect(x, y, x+width, y+height, arc_width, arc_height);
  }

  void FillRect(double x, double y, double width, double height) {
    check_pen_update();
    check_brush_update();
    cr_->Rectangle(x, y, x+width, y+height);
  }

  void FillOval(double x, double y, double width, double height) {
    check_pen_update();
    check_brush_update();
    cr_->Ellipse(x, y, x+width, y+height);
  }

  void DrawPolygon(const ui::Points& points) {
    check_pen_update();
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
    check_pen_update();
    check_brush_update();
    std::vector<CPoint> lpPoints;
    ui::Points::const_iterator it = points.begin();
    const int size = points.size();
    for (; it!=points.end(); ++it) {
      lpPoints.push_back(CPoint(it->first, it->second));
    }
    cr_->Polygon(&lpPoints[0], size);
  }

  void DrawPolyline(const ui::Points& points) {
    check_pen_update();
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

  void DrawImage(ui::Image* img, double x, double y) {
    double w, h;
    img->Size(w, h);
    DrawImage(img, x, y, w, h, 0, 0);
  }

  void DrawImage(ui::Image* img, double x, double y, double width, double height) {
    DrawImage(img, x, y, width, height, 0, 0);
  }

  void DrawImage(ui::Image* img, double x, double y, double width, double height, double xsrc,
       double ysrc) {
    CDC memDC;
    CBitmap* oldbmp;
    memDC.CreateCompatibleDC(cr_);
    oldbmp = memDC.SelectObject((CBitmap*)img->source());
    if (!img->mask()) {
      cr_->BitBlt(x, y, width, height, &memDC, xsrc, ysrc, SRCCOPY);
    } else {
      TransparentBlt(cr_, x,  y, width, height, &memDC, (CBitmap*) img->mask(),
        xsrc, ysrc);
    }
    memDC.SelectObject(oldbmp);
    memDC.DeleteDC();
  }

  void SetClip(double x, double y, double width, double height) {
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

  void check_pen_update() {
    if (updatepen_) { 
      pen.DeleteObject();
      pen.CreatePen(PS_SOLID, 1, ToCOLORREF(color_));
      cr_->SelectObject(&pen);
      cr_->SetTextColor(ToCOLORREF(color_));
      updatepen_ = false;
    }
  }

  void check_brush_update() {
    if (updatebrush_) {
      brush.DeleteObject();
      brush.CreateSolidBrush(ToCOLORREF(color_));
      cr_->SelectObject(&brush);    
      updatebrush_ = false;
    }
  }

  virtual CDC* dc() {
     return cr_;
  }
    
  private:
   virtual void SaveOrigin() {
     saveXform.push(rXform);
   }
   virtual void RestoreOrigin() {
     rXform = saveXform.top();
     saveXform.pop();
     cr_->SetGraphicsMode(GM_ADVANCED);
     cr_->SetWorldTransform(&rXform);
   }
   
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
   bool updatepen_, updatebrush_;
   XFORM rXform;
   std::stack<XFORM> saveXform;
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

enum CanvasMsg {
  ONWND
};

struct BlitInfo {
  double dx, dy;  
};

class Item : public boost::enable_shared_from_this<Item> {
  friend class Group;
public:  
  typedef boost::shared_ptr<Item> Ptr;
  typedef boost::shared_ptr<const Item> ConstPtr;
  typedef boost::weak_ptr<Item> WeakPtr;
  typedef boost::weak_ptr<const Item> ConstWeakPtr;
  typedef std::vector<Item::Ptr> ItemList;

  Item() 
    : rgn_(new mfc::Region),
      clp_rgn_(new mfc::Region),
      fls_rgn_(new mfc::Region),       
      draw_rgn_(0),
      has_store_(false) {
    x_ = y_ = 0;
    visible_ = pointer_events_ = update_=  true;
    has_clip_ = has_store_ = false;   
  }
    
  virtual ~Item() { }
  
  typedef ItemList::iterator iterator;
  virtual iterator begin() { return dummy.begin(); }
  virtual iterator end() { return dummy.end(); }
  virtual bool empty() const { return true; }
  
  virtual void Add(const Item::Ptr& item) { }
  virtual void Insert(iterator it, const Item::Ptr& item) { }
  virtual void Remove(const Item::Ptr& item) { }
  virtual void RemoveAll() { }
  virtual void Clear() { }

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
  virtual void Draw(Graphics* g, Region& draw_region) {};
  virtual Item::WeakPtr OnEvent(class Event* ev) { return Item::WeakPtr(); }
  void GetFocus();
  void Detach();
  virtual void Show() { 
    if (!visible_) { 
      STR(); visible_ = true; FLS();
    }
  }
  virtual void Hide() { 
    if (visible_) {
      STR(); visible_ = false; FLS();
    } 
  }
  // regions
  void STR();  // store old region
  void FLS();  // invalidate combine new & old region
  void FLS(const Region& rgn); // invalidate region
  virtual void needsupdate();
  virtual const Region& region() const { 
    if (update_) {    
      update_ = false;        
      const_cast<Item*>(this)->onupdateregion();
      if (has_clip_) {
        rgn_->Combine(*clp_rgn_, RGN_AND);
      }
    }            
    return *rgn_.get();
  }
  virtual void onupdateregion() { needsupdate(); }
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
  virtual Item::Ptr Intersect(double x, double y, Event* ev, bool& worked) {
    if (update_) { region(); }
    return rgn_->Intersect(x, y) ? shared_from_this() : Item::Ptr();    
  }
  virtual bool visible() const { return visible_; }  
  void SetBlitXY(double x, double y);
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

  Canvas* root();
  virtual Canvas* root() const { return 0; }
  virtual void set_parent(const Item::WeakPtr& parent) { parent_ = parent; }
  Item::WeakPtr parent() { return parent_; }
  Item::ConstWeakPtr parent() const { return parent_; }
  bool IsInGroup(Item::WeakPtr group) const;
  virtual bool is_root() const { return 0; }
                            
  void SetClip(double x, double y, double width, double height) {
    clp_rgn_->SetRect(x, y, width, height);
    has_clip_ = true;
    needsupdate();
  }
  bool has_clip() const { return has_clip_; }
  const Region& clip() const { return *clp_rgn_.get(); }
  void RemoveClip() { has_clip_ = false; }
  virtual void OnMessage(CanvasMsg msg) {};
    
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
  
  boost::shared_ptr<Item> this_ptr() {
    return shared_from_this();
  }

private:  
  Item::WeakPtr parent_;
  std::auto_ptr<Region> fls_rgn_;
  Region* draw_rgn_;
  bool visible_, pointer_events_, has_store_;
  ItemList dummy;
  std::auto_ptr<BlitInfo> blit_;
};

class Event {  
 public:
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
  Event(const Item::WeakPtr& item, Event::Type type, int cx, int cy, int button, unsigned int shift)
      : item_(item), type_(type), cx_(cx), cy_(cy), button_(button), shift_(shift) { }
  Event::Type type() const { return type_; }
  void setcxy(double x, double y) { cx_ = x; cy_ = y; }
  double cx() const { return cx_; }
  double cy() const { return cy_; }
  unsigned int button() const { return button_; }
  unsigned int shift() const { return shift_; }
  void setitem(Item::WeakPtr item) { item_ = item; }
  Item::WeakPtr item() const { return item_; }
 private:
  Item::WeakPtr item_;
  Event::Type type_;
  double cx_, cy_;
  unsigned int button_, shift_;
};

class Canvas;

class Group : public Item {
 public:
  friend Canvas;
  friend Item;
  
  Group() : Item(), zoom_(1), is_root_(false) { }

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
  virtual Item::Ptr Intersect(double x, double y, Event* ev, bool &worked);

  void RaiseToTop(Item::Ptr item) {  Remove(item); Add(item); }
  void set_zorder(Item::Ptr item, int z);
  int zorder(Item::Ptr item) const;
  void setzoom(double zoom) { zoom_ = zoom; }
  virtual double zoom() const { return zoom_; }
  virtual void onupdateregion();

  bool WorkItemEvent(Item::WeakPtr item, Event* ev);
  bool is_root() const { return is_root_; }
  virtual void OnMessage(CanvasMsg msg);
  
 protected:
  ItemList items_;
 private:
  void Init();  
  double x_, y_, zoom_;
  bool is_root_;  
};

class Rect : public Item {
 public:
  Rect::Rect() : Item() {
    width_ = height_ = bx_ = by_ = 0;
    fillcolor_ = strokecolor_ = 0;
  }    

  virtual void Draw(Graphics* g, Region& draw_region);

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
  Pic() : Item() {
    image_ = 0;
    width_ = height_ = xsrc_ = ysrc_ = 0;
    transparent_ = pmdone = false;
  }
  
  virtual void Draw(Graphics* g, Region& draw_region);
  virtual void onupdateregion();
  void SetSize(double width, double height) {
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

class Line : public Item {
 public:
  Line() : Item(), color_(0) { }    

  virtual void Draw(Graphics* g, Region& draw_region);
  virtual Item::Ptr Intersect(double x, double y, Event* ev, bool &worked);  
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
  Text(const std::string& text) : text_(text) { Init(1.0); }

  virtual void Draw(Graphics* cr, Region& draw_region);
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
  CWndItem() :
      ui::canvas::Item(),
      id_(newid()),      
      p_wnd_(dummy_wnd()),
      w_(200),
      h_(20) {
    id_map_[id_] = this;
    needsupdate();  
  }  
  virtual ~CWndItem() { control_.DestroyWindow(); }

  virtual void Draw(ui::Graphics* g, ui::Region& draw_region) {                    
    double cx = zoomabsx();
    double cy = zoomabsy();          
    CRect rc;
    control_.GetWindowRect(&rc);
    control_.GetParent()->ScreenToClient(&rc);                    
    if (rc.left != cx || rc.top != cy) {      
      //CRgn rgn;
      //rgn.CreateRectRgn(0, 0, 10, 10);
      //control_.SetWindowRgn(*(CRgn*)(draw_region.source()), true);      
      /*control_.ModifyStyle(0, WS_CLIPSIBLINGS); 
      control_.ShowWindow(SW_HIDE);
      control_.SetWindowRgn(rgn, true);
      control_.ShowWindow(SW_SHOW);
      control_.Invalidate();*/
      // control_.MoveWindow(cx, cy, rc.Width(), rc.Height());
      //::SetWindowRgn(control_.m_hWnd, NULL, false);
      //control_.SetWindowRgn(*(CRgn*)(draw_region.source()), true);
    }    
  }     
  virtual void set_parent(const Item::WeakPtr& parent) { 
    canvas::Item::set_parent(parent);
    canvas::Canvas* c = root();
    if (parent.expired()) {
      p_wnd_ = dummy_wnd();
      control_.SetParent(p_wnd_);
    } else if (c && c->wnd() && c->wnd() != p_wnd_) {       
       p_wnd_ = c->wnd();
       control_.SetParent(p_wnd_);      
    }
  }
  virtual void onupdateregion() { rgn_->SetRect(0, 0, w_, h_); }
  virtual void OnMessage(ui::canvas::CanvasMsg msg) {
    if (msg == ui::canvas::ONWND) {
      CWnd* wnd = root()->wnd();      
      if (!wnd) {
        p_wnd_ = dummy_wnd();
        control_.SetParent(p_wnd_);        
      } else 
      if (p_wnd_!= wnd) {
        p_wnd_ = wnd;
        control_.SetParent(p_wnd_);
        // todo correct clipping
        /*control_.ModifyStyle(0, WS_CLIPSIBLINGS); 
         CRgn rgn;
         rgn.CreateRectRgn(0, 0, 10, 10);  
         control_.SetWindowRgn(rgn, true);*/        
        control_.ShowWindow(SW_SHOW);        
      }
    }
  }  
  virtual void SetXY(double x, double y) {
    x_ = x;
    y_ = y;          
    int cx = zoomabsx();
    int cy = zoomabsy();
    CRect rc;
    control_.GetClientRect(&rc);
    control_.MoveWindow(cx, cy, rc.Width(), rc.Height());
  }  

  virtual void SetPos(double x, double y, double w, double h) {
    x_ = x;
    y_ = y;          
    int cx = zoomabsx();
    int cy = zoomabsy();
    ctrl().MoveWindow(cx, cy, w, h);
  }
  virtual void Show() { 
    if (!ctrl().IsWindowVisible()) {
      ctrl().ShowWindow(SW_SHOW);
    }
  }
  virtual void Hide() {
    if (ctrl().IsWindowVisible()) {
      ctrl().ShowWindow(SW_HIDE);
    }
  }
  virtual bool visible() const { return ctrl().IsWindowVisible(); }    
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
   static CWnd* dummy_wnd() {
     if (!dummy_wnd_.m_hWnd) {   
       dummy_wnd_.Create(0, "psycleuidummywnd", 0, CRect(0, 0, 0, 0), ::AfxGetMainWnd(), 0);       
     }
     return &dummy_wnd_;
   }
  int w_, h_, id_;
  T control_;  
  static int id_counter;
  static std::map<std::uint16_t, CWndItem<T>*> id_map_;
  static CWnd dummy_wnd_;
  CWnd* p_wnd_;
};

template <typename T>
CWnd CWndItem<T>::dummy_wnd_;

template <typename T>
int CWndItem<T>::id_counter = ID_DYNAMIC_CONTROLS_BEGIN;

template <typename T>
std::map<std::uint16_t, CWndItem<T>*> CWndItem<T>::id_map_;

class Button : public CWndItem<CButton> {
 public:
  static std::string type() { return "buttonitem"; }
  Button() : CWndItem<CButton>() { 
    ctrl().Create("btn", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | DT_CENTER,
      CRect(0, 0, 55, 19), p_wnd(), id());    
  }  
  virtual void OnClick() {}  
};


class CSListener {
public:
  virtual ~CSListener() {}
  virtual void OnModified() = 0;
};

class CScintilla : public CWnd {
	public:
    CScintilla() : l_(0) {  }
    bool Create(CWnd* pParentWnd, UINT nID) {
      if (!CreateEx(0, 
                    _T("scintilla"),
                    "", 
                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN
                    , CRect(0, 0, 0, 0),
                    pParentWnd,
                    nID,
                    0)) {
          TRACE0("Failed to create scintilla window\n");				
          return false;
      }
      
      fn = (int (__cdecl *)(void *,int,int,int))
           SendMessage(SCI_GETDIRECTFUNCTION, 0, 0);
      ptr = (void *)SendMessage(SCI_GETDIRECTPOINTER, 0, 0);	 
      
      return true;
    }

    template<class T, class T1>
    int f(int sci_cmd, T lparam, T1 wparam) {     
      return fn(ptr, sci_cmd, (WPARAM) lparam, (LPARAM) wparam);
    }
    
    void setlistener(CSListener* l) { l_ = l; }
            
   protected:
     DECLARE_DYNAMIC(CScintilla)     
     int (*fn)(void*,int,int,int);
	   void * ptr;
     
     BOOL OnModified(NMHDR *,LRESULT *) {
       if (l_) { 
         l_->OnModified();
       }
       return false;
     }

   private:     
     CSListener* l_;
     DECLARE_MESSAGE_MAP();     
};


class Scintilla : public CWndItem<CScintilla>, public CSListener {
 public:
  typedef CScintilla CTRL;
  static std::string type() { return "scintillaitem"; }
  Scintilla() : CWndItem<CScintilla>(), modified_(false), has_file_(false) {
    ctrl().setlistener(this);
    ctrl().Create(p_wnd(), id());    
    f(SCI_SETMARGINWIDTHN, 0, 32);
  }  
  
  template<class T, class T1>
  int f(int sci_cmd, T lparam, T1 wparam) {     
    return ctrl().f(sci_cmd, (WPARAM) lparam, (LPARAM) wparam);
  }

  void LoadFile(const std::string& filename) {
    using namespace std;    
    #if __cplusplus >= 201103L
      ifstream file (filename, ios::in|ios::binary|ios::ate);
    #else
      ifstream file (filename.c_str(), ios::in|ios::binary|ios::ate);
    #endif
    if (file.is_open()) {
      modified_ = true;
      f(SCI_CANCEL, 0, 0);
      f(SCI_SETUNDOCOLLECTION, 0, 0);      
      file.seekg(0, ios::end);
      size_t size = file.tellg();
      char *contents = new char [size];
      file.seekg (0, ios::beg);
      file.read (contents, size);
      file.close();
      f(SCI_ADDTEXT, size, contents);      
      f(SCI_SETUNDOCOLLECTION, true, 0);
      f(SCI_EMPTYUNDOBUFFER, 0, 0);      
      f(SCI_SETSAVEPOINT, 0, 0);      
      f(SCI_GOTOPOS, 0, 0);      
      delete [] contents;
      fname_ = filename;
      modified_ = false;
      has_file_ = true;
    } else {
      throw std::runtime_error("File Not Open Error!");
    }
  }

  void SaveFile(const std::string& filename) {
    //Get the length of the document
    int nDocLength = f(SCI_GETLENGTH, 0, 0);
    //Write the data in blocks to disk
    CFile file;
    BOOL res = file.Open(_T(filename.c_str()), CFile::modeCreate | CFile::modeReadWrite);
    if (res) {
      modified_ = true;
      for (int i=0; i<nDocLength; i += 4095) //4095 because data will be returned NULL terminated
      {
        int nGrabSize = nDocLength - i;
        if (nGrabSize > 4095)
          nGrabSize = 4095;
        //Get the data from the control
        TextRange tr;
        tr.chrg.cpMin = i;
        tr.chrg.cpMax = i + nGrabSize;
        char Buffer[4096];
        tr.lpstrText = Buffer;
        f(SCI_GETTEXTRANGE, 0, &tr);
        //Write it to disk
        file.Write(Buffer, nGrabSize);
      }
    file.Close();
    has_file_ = true;
    fname_ = filename;
    modified_ = false;
    } else {
      throw std::runtime_error("File Save Error");
    }
  }  
  bool has_file() const { return has_file_; }
  const std::string& filename() const { return fname_; }
  bool modified() const { return modified_; }
  virtual void OnModified() { 
    if (!modified_) {
      OnFirstModified();
    }
    modified_ = true;
  }
  virtual void OnFirstModified() { }

private:
  std::string fname_;
  bool modified_, has_file_;
};

class Edit : public CWndItem<CEdit> {
 public:
  static std::string type() { return "edititem"; }
  Edit() : CWndItem<CEdit>() {
    ctrl().Create(WS_CHILD | WS_TABSTOP, CRect(0, 0, 55, 19), p_wnd(), id());        
  }  
  void SetText(const std::string& text) { ctrl().SetWindowText(text.c_str()); }
  std::string GetText() const {    
    CString s;    
    ctrl().GetWindowTextA(s);
    return s.GetString();
  }  
};

enum Orientation { HORZ = 0, VERT = 1 };

class ScrollBar : public CWndItem<CScrollBar> {
 public:
  static std::string type() { return "scrollbaritem"; }
  ScrollBar() : CWndItem<CScrollBar>() {
    ctrl().Create(SBS_VERT | WS_CHILD, CRect(0 , 0, 100, 30), p_wnd(), id());
  }  
  virtual void OnScroll(int pos) {}  
  void SetScrollRange(int minpos, int maxpos) { 
    ctrl().SetScrollRange(minpos, maxpos);
  }
  void scroll_range(int& minpos, int& maxpos) const { 
    ctrl().GetScrollRange(&minpos, &maxpos);
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
  int scroll_pos() const { return ctrl().GetScrollPos(); }

  void SystemSize(int& width, int& height) const {
    width = GetSystemMetrics(SM_CXVSCROLL);
    height = GetSystemMetrics(SM_CXHSCROLL);
  }  
};

// canvas
class Canvas : public Group {
  friend class Item;
  friend class Group;
 public:
  typedef boost::shared_ptr<Canvas> Ptr;
  typedef boost::weak_ptr<Canvas> WeakPtr;

  Canvas() : Group(), wnd_(0) { Init(); }
  Canvas(CWnd* parent) : Group(), wnd_(parent) { Init(); }    
    
  void Draw(Graphics* g, Region& rgn);  
  void Flush();
  void SetColor(ARGB color) { color_ = color; }
  ARGB color() const { return color_; }
  void set_bg_image(CBitmap* bg_image, int width, int height) {
    bg_image_ = bg_image;
    bg_width_ = width;
    bg_height_ = width;
  }
  void StealFocus(Item::Ptr item);
  canvas::Item::WeakPtr OnEvent(Event* ev);
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
        Group::OnMessage(ONWND);        
      }
    }
    CWnd* wnd() { return wnd_; }
    int cw() const { return cw_; }
    int ch() const { return ch_; }
    void ClientSize(int& cw, int& ch) {
      cw = cw_;
      ch = ch_;
    }
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
      case ROW_RESIZE  : c = IDC_SIZENS;; break;
      case CROSSHAIR   : c = IDC_CROSS; break;
      case PROGRESS    : c = IDC_APPSTARTING; break;
      case E_RESIZE    : c = IDC_SIZEWE; break;
      case NE_RESIZE   : c = IDC_SIZENWSE; break;
      case DEFAULT     : c = IDC_ARROW; break;
      case TEXT        : c = IDC_IBEAM; break;
      case N_RESIZE    : c = IDC_SIZENS; break;
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
  void DoDraw(Graphics* g, Region& rgn);
  Canvas(const Canvas& other) {}
  Canvas& operator=(Canvas rhs) {}
  Item::WeakPtr DelegateEvent(Event* event, Item::WeakPtr item);  
  CWnd* wnd_, *old_wnd_;  
  bool save_, steal_focus_;
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

class BaseView {
 public:
  BaseView(CWnd* wnd) : wnd_(wnd) {
    assert(wnd);
  }
  virtual ~BaseView() { }
  virtual void set_canvas(Canvas::WeakPtr canvas) {
    if (canvas_.lock() == canvas.lock()) {
      return;
    }    
    if (!canvas_.expired()) {
       canvas_.lock()->set_wnd(0);
    }
    canvas_ = canvas;
    if (!canvas.expired()) {        
      canvas.lock()->set_wnd(wnd_);
    }    
  }
  Canvas::WeakPtr canvas() { return canvas_; }  
  CWnd* wnd() { return wnd_; }  
protected:
  void Draw();  
  void Resize(int cw, int ch) {    
    if (!canvas_.expired()) {      
      try {
        Canvas* c = canvas().lock().get();
        c->OnSize(cw, ch);
        c->ClearSave();
        wnd_->Invalidate();
      } catch (std::exception& e) {
        AfxMessageBox(e.what());
      }  
    }
  }
  void OnDestroy() { bmpDC.DeleteObject(); }
  bool DelegateEvent(int type, int button, UINT nFlags, CPoint pt);  
  int FlagsToButton(UINT nflags) {
    int button = (MK_LBUTTON & nflags) ? 1 : 
                 (MK_MBUTTON & nflags) ? 3 : 
                 (MK_RBUTTON & nflags) ? 2 : 
                                         0;
    return button;
  }
protected:
  CBitmap bmpDC;
  Canvas::WeakPtr canvas_;
private:  
  CWnd* wnd_;  
};

class View : public CWnd, public BaseView, public Timer {
	public:
    View() : BaseView(this) {}
		virtual void Open() {};		
    void set_canvas(Canvas::WeakPtr canvas) { 
      BaseView::set_canvas(canvas);      
      if (!canvas.expired()) {      
        Canvas* c = canvas.lock().get();
        try {        
          c->OnSize(cw_, ch_);
          c->ClearSave();
          Invalidate();
        } catch (std::exception& e) {
          AfxMessageBox(e.what());
        }  
      } 
    }    
	protected:		
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);    
	DECLARE_MESSAGE_MAP()
		int OnCreate(LPCREATESTRUCT lpCreateStruct);
    void OnDestroy();
		void OnSetFocus(CWnd* pOldWnd);
    void OnPaint() { Draw(); }
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
      DelegateEvent(Event::MOTION_NOTIFY, FlagsToButton(nFlags), nFlags, pt);
    }
		void OnLButtonUp(UINT nFlags, CPoint pt) {
      DelegateEvent(Event::BUTTON_RELEASE, 1, nFlags, pt);
    }
		void OnRButtonUp(UINT nFlags, CPoint pt) {
      DelegateEvent(Event::BUTTON_RELEASE, 2, nFlags, pt);
    }
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
      Canvas* c = canvas().lock().get();    
      if (c) {
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
        DelegateEvent(Event::KEY_DOWN,  nChar, flags, CPoint(0, 0));
      }
      CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
    }
		void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
      Canvas* c = canvas().lock().get();    
      if (c) {
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
        DelegateEvent(Event::KEY_UP,  nChar, flags, CPoint(0, 0));        
      }
      CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
    }
    void OnTimerViewRefresh();
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
    int cw_, ch_;    
};

class CanvasFrameWnd : public CFrameWnd {			
 DECLARE_DYNAMIC(CanvasFrameWnd)
 public:   
  virtual ~CanvasFrameWnd() {} // Use OnDestroy
  void set_canvas(Canvas::WeakPtr canvas) { assert(pView_); pView_->set_canvas(canvas); }
  Canvas::WeakPtr canvas() { return pView_ ? pView_->canvas() : null_wptr; }  
  void SetTitle(const std::string& title) { SetWindowTextA(_T(title.c_str())); }
  void SetPos(int x, int y) { }
 protected:
  CanvasFrameWnd() : CFrameWnd(), pView_(0) { }; // protected constructor used 
                                                 // by dynamic creation,
                                                 // Use OnCreate.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();  
  
  virtual int OnFrameClose() { return 0; }  
  virtual void OnFrameTimer() {}

 DECLARE_MESSAGE_MAP()
	int OnCreate(LPCREATESTRUCT lpCreateStruct);	
  void OnDestroy();
	void OnSetFocus(CWnd* pOldWnd);
  BOOL OnEraseBkgnd(CDC* pDC);
  void OnClose() { 
    try {
      OnFrameClose();
    } catch (std::exception& e) {
      AfxMessageBox(e.what());
    }    
  }
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);	
  virtual void OnTimerTick() {}
 private:
  View* pView_;  
  Canvas::WeakPtr null_wptr;
};

} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle