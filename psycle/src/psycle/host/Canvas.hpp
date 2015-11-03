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

typedef std::vector<std::pair<double,double> > Points;

enum CursorStyle {
  AUTO, MOVE, NO_DROP, COL_RESIZE,
  ALL_SCROLL, POINTER, NOT_ALLOWED,
  ROW_RESIZE, CROSSHAIR, PROGRESS, E_RESIZE,
  NE_RESIZE, DEFAULT, TEXT, N_RESIZE,
  NW_RESIZE, HELP, VERTICAL_TEXT, S_RESIZE,
  SE_RESIZE, INHERIT, WAIT, W_RESIZE, SW_RESIZE
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

  virtual void Offset(int dx, int dy) = 0;
  virtual int Combine(const Region& other, int combinemode) = 0;
  virtual void GetBoundRect(double& x, double& y, double& width, double& height) const = 0;
  virtual bool Intersect(int x, int y) const = 0;
  virtual bool IntersectRect(double x, double y, double width, double height) const = 0;
  virtual void Clear() = 0;
  virtual void SetRect(double x, double y, double width, double height) = 0;
  virtual void* source() = 0;
  virtual const void* source() const = 0;
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

namespace mfc {
class Region : public ui::Region {
 public:
  Region() { Init(0, 0, 0, 0); }
  Region(int x, int y, int width, int height) { Init(x, y, width, height); }
  Region(const ui::Region& other) {
    Init(0, 0, 0, 0);
    rgn_.CopyRgn((const CRgn*) other.source());
  }
  Region(const CRgn& rgn)  {
    assert(rgn.m_hObject);
    Init(0, 0, 0, 0);
    rgn_.CopyRgn(&rgn);
  }
  ~Region() { rgn_.DeleteObject(); }
  Region& operator=(const Region& rhs) {
    rgn_.CopyRgn((CRgn*)rhs.source());
    return *this;
  }
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
    BITMAP bm;
    bmp_->GetBitmap(&bm);
    width_ = bm.bmWidth;
    height_ = bm.bmHeight;
  }
  ~Image() {
    if (!shared_ && bmp_) {
      delete bmp_;
    }
    if (mask_) {
      delete mask_;
    }
  }

  void Load(const std::string& filename) {
    if (!shared_ && bmp_) delete bmp_;
    std::string extlower = boost::filesystem::path(filename).string();
    std::transform(extlower.begin(), extlower.end(), extlower.begin(), std::tolower);
    HRESULT r;
    if (extlower == "png") {
      CPngImage image;
      r = image.Load(_T(filename.c_str()));
      if (r!=S_OK) throw std::runtime_error(std::string("Error loading file:") + filename.c_str());
      bmp_ = new CBitmap();
      bmp_->Attach(image.Detach());
    } else {
      CImage image;
      r = image.Load(_T(filename.c_str()));
      if (r!=S_OK) throw std::runtime_error(std::string("Error loading file:") + filename.c_str());
      bmp_ = new CBitmap();
      bmp_->Attach(image.Detach());
    }
  }

  double width() const { return width_; }
  double height() const { return height_; }
  void SetTransparent(bool on, ARGB color) {
    if (mask_) {
      delete mask_;
      mask_ = 0;
    }
    if (on) {
      mask_ = new CBitmap();
      PrepareMask(bmp_, mask_, ToCOLORREF(color));
    }
  }

 private:
  void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);
  void* source() { return bmp_; }
  void* source() const { return bmp_; }
  void* mask() { return mask_; }
  const void* mask() const { return mask_; }
  void Init() {
    bmp_ = mask_ = 0;
    width_ = height_ = xsrc_ = ysrc_ = 0;
    shared_ = false;
  }
  CBitmap* bmp_, *mask_;
  int width_, height_, xsrc_, ysrc_;
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
    rgn ? cr_->SelectClipRgn((CRgn*) rgn->source())
        : cr_->SelectClipRgn(0);
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
} // namespace mfc

namespace canvas {
using namespace ui;

class Item;
class Canvas;

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
  Event(Item* item, Type type, int cx, int cy, int button, unsigned int shift)
       : item_(item), type_(type), cx_(cx), cy_(cy), button_(button), shift_(shift) {}
  Type type() const { return type_; }
  void setcxy(double x, double y) { cx_ = x; cy_ = y; }
  double cx() const { return cx_; }
  double cy() const { return cy_; }
  unsigned int button() const { return button_; }
  unsigned int shift() const { return shift_; }
  void setitem(Item* item) { item_ = item; }
  Item* item() const { return item_; }
 private:
  Item* item_;
  Type type_;
  double cx_, cy_;
  unsigned int button_, shift_;
};

class Item {
public:
  Item() { Init(); }
  Item(class Group* parent) { Init(parent); }
  Item(class Group* parent, double x, double y) { Init(parent, x, y); }
  virtual ~Item();
  
  typedef std::vector<Item*>::iterator iterator;
  virtual iterator begin() { return dummy.begin(); }
  virtual iterator end() { return dummy.end(); }
  virtual bool empty() const { return true; }

  virtual void Draw(Graphics* g, Region& repaint_region, class Canvas* widget) = 0;
  virtual bool OnEvent(Event* ev) { return 0; }
  void GetFocus();
  void Detach();
  virtual void Show() {STR(); visible_ = true; FLS(); }
  virtual void Hide() {STR(); visible_ = false; FLS(); }
  void STR();  // store old region
  void FLS();  // invalidate combine new & old region
  void FLS(const Region& rgn); // invalidate region
  virtual void needsupdate();

  virtual const Region& region() const { return rgn_; }
  virtual void GetBoundRect(double& x, double& y, double& width, double& height) const {
    region().GetBoundRect(x, y, width, height);
  }
  virtual Item* Intersect(double x, double y, Event* ev, bool& worked) {
    if (update_) { region(); }
    return rgn_.Intersect(x, y) ? this : 0;    
  }

  bool visible() const { return visible_; }
  virtual void SetXY(double x, double y) { STR(); x_ = x; y_ = y; FLS(); }   
  double x() const { return x_; }
  double y() const { return y_; }
  void pos(double& xv, double& yv) const { xv = x(); yv = y(); }
  void clientpos(double& xv, double& yv) const { xv = zoomabsx(); yv = zoomabsy(); }
  virtual double zoomabsx() const;
  virtual double zoomabsy() const;
  virtual double acczoom() const;
  virtual void set_zorder(int z);

  void EnablePointerEvents() { pointer_events_ = true; }
  void DisablePointerEvents() { pointer_events_ = false; }
  bool pointerevents() const { return pointer_events_; }

  Canvas* canvas();
  virtual Canvas* widget() const { return 0; }
  void set_parent(class Group* parent) { parent_ = parent; }
  Group* parent() { return parent_; }
  const Group* parent() const { return parent_; }
                      
  void set_manage(bool on) { managed_ = on; }
  bool managed() const { return managed_; }
    
  void SetClip(int x, int y, int width, int height) {
    clp_rgn_.SetRect(x, y, width, height);
    has_clip_ = true;
  }
  bool has_clip() const { return has_clip_; }
  const Region& clip() const { return clp_rgn_; }
  void RemoveClip() { has_clip_ = false; }

protected:
  void CheckButtonPress();
  void CheckFocusItem();  
  void swap_smallest(double& x1, double& x2) const {
    if (x1 > x2) {
      std::swap(x1, x2);
     }
  }          
  mutable bool update_, has_clip_;
  mutable mfc::Region rgn_, clp_rgn_;
  double x_, y_;

private:
  void Init(Group* parent_ = 0, double x = 0, double y = 0);
  Group* parent_;
  mfc::Region fls_rgn_;
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
  void RemoveAll();
  void Clear();
    
  virtual void Draw(Graphics* g, Region& repaint_region, Canvas* widget);
  virtual Item* Intersect(double x, double y, Event* ev, bool &worked);

  void RaiseToTop(Item* item);
  void set_zorder(Item* item, int z);
  int zorder(Item* item) const;
  void setzoom(double zoom) { zoom_ = zoom; }
  virtual double zoom() const { return zoom_; }
  virtual const Region& region() const;

  bool work_item_event(Item* item, Event* ev);    
  bool is_root() const { return is_root_; }
        
  Canvas* widget() { return widget_; }
  virtual Canvas* widget() const { return widget_; }  
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
  virtual const Region& region() const;  
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
  virtual const Region& region() const;
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
  typedef std::pair<double, double> Point;
  void SetPoints(const Points& pts) {  STR(); pts_ = pts; FLS(); }
  void SetPoint(int idx, const Point& pt) { STR(); pts_[idx] = pt; FLS(); }
  const Points& points() const { return pts_; }
  const Point& PointAt(int index) const { return pts_.at(index); }
  void SetColor(ARGB color) { color_ = color; FLS(); }
  ARGB color() const { return color_; }
  virtual const Region& region() const;
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
  virtual const Region& region() const;
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

class Canvas {
  friend class Item;
  friend class Group;
 public:
  Canvas() : wnd_(0) { Init(); }
  Canvas(CWnd* parent) : wnd_(parent) { Init(); }
  void Init();
  ~Canvas();

  void set_wnd(CWnd* wnd) {wnd_ = wnd;}
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

 protected:
  void Invalidate(Region& rgn);
 private:
  Canvas(const Canvas& other) {}
  Canvas& operator=(Canvas rhs) {}
  Item* DelegateEvent(Event* event, Item* item);
  CWnd* wnd_;
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
} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle