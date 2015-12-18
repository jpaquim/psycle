// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

typedef int32_t ARGB;
#undef GetGValue
#define GetGValue(rgb) (LOBYTE((rgb) >> 8))
#define ToCOLORREF(argb) RGB((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, (argb >> 0) & 0xFF)
#define GetAlpha(argb) (argb>>24) & 0xFF
#define ToARGB(r, g, b) (((ARGB) (b) << 0) | ((ARGB) (g) << 8) |((ARGB) (r) << 16) | ((ARGB) (0) << 24))

namespace psycle {
namespace host {
namespace ui {

typedef std::pair<double, double> Point;
typedef std::vector<std::pair<double, double> > Points;

struct Rect {
  Rect() { set(0, 0, 0, 0); }
  Rect(double l, double t, double r, double b) { set(l, t, r, b); }

  inline void set(double l, double t, double r, double b) {
    left_ = l; top_ = t; right_ = r; bottom_ = b;
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

 private:
  double left_, top_, right_, bottom_;
};

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

namespace mfc
{

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
    x = rc.left; y = rc.top; 
    width = w_cache_ != -1 ? w_cache_ : rc.Width(); 
    height = h_cache_ != -1 ? h_cache_ : rc.Height();
  }
  bool Intersect(double x, double y) const { return rgn_.PtInRegion(x, y); }
  bool IntersectRect(double x, double y, double width, double height) const {
    CRect rc(x, y, x+width, y+height);
    return rgn_.RectInRegion(rc);
  }
  virtual void SetRect(double x, double y, double width, double height) {
    if (width == 0 || height == 0) {
      w_cache_ = width;
      h_cache_ = height;
    } else {
      w_cache_ = h_cache_ = -1;
    }
    rgn_.SetRectRgn(x, y, x+width, y+height);
  }
  void Clear() { SetRect(0, 0, 0, 0); }
  void* source() { return &rgn_; }
  const void* source() const { return &rgn_; };

 private:
  void Init(int x, int y, int width, int height) {
    w_cache_ = h_cache_ = -1;
    rgn_.CreateRectRgn(x, y, x+width, y+height);
  }
  
  CRgn rgn_;
  double w_cache_, h_cache_;
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

  void size(double& width, double& height) const {
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

inline void Image::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans) {
  //assert(pBmpSource && pBmpSource->m_hObject);
  BITMAP bm;
  // Get the dimensions of the source bitmap
  pBmpSource->GetObject(sizeof(BITMAP), &bm);
  // Create the mask bitmap
  if (pBmpMask->m_hObject) {
    pBmpMask->DeleteObject();
  }
  pBmpMask->CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL);
  // We will need two DCs to work with. One to hold the Image
  // (the source), and one to hold the mask (destination).
  // When blitting onto a monochrome bitmap from a color, pixels
  // in the source color bitmap that are equal to the background
  // color are blitted as white. All the remaining pixels are
  // blitted as black.
  CDC hdcSrc, hdcDst;
  hdcSrc.CreateCompatibleDC(NULL);
  hdcDst.CreateCompatibleDC(NULL);
  // Load the bitmaps into memory DC
  CBitmap* hbmSrcT = (CBitmap*) hdcSrc.SelectObject(pBmpSource);
  CBitmap* hbmDstT = (CBitmap*) hdcDst.SelectObject(pBmpMask);
  // Change the background to trans color
  hdcSrc.SetBkColor(clrTrans);
  // This call sets up the mask bitmap.
  hdcDst.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcSrc,0,0,SRCCOPY);
  // Now, we need to paint onto the original image, making
  // sure that the "transparent" area is set to black. What
  // we do is AND the monochrome image onto the color Image
  // first. When blitting from mono to color, the monochrome
  // pixel is first transformed as follows:
  // if  1 (black) it is mapped to the color set by SetTextColor().
  // if  0 (white) is is mapped to the color set by SetBkColor().
  // Only then is the raster operation performed.
  hdcSrc.SetTextColor(RGB(255,255,255));
  hdcSrc.SetBkColor(RGB(0,0,0));
  hdcSrc.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcDst,0,0,SRCAND);
  // Clean up by deselecting any objects, and delete the
  // DC's.
  hdcSrc.SelectObject(hbmSrcT);
  hdcDst.SelectObject(hbmDstT);
  hdcSrc.DeleteDC();
  hdcDst.DeleteDC();
}

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
    img->size(w, h);
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

inline void Graphics::TransparentBlt(CDC* pDC,
  int xStart,
  int yStart,
  int wWidth,
  int wHeight,
  CDC* pTmpDC,
  CBitmap* bmpMask,
  int xSource, // = 0
  int ySource) // = 0)
{
  // We are going to paint the two DDB's in sequence to the destination.
  // 1st the monochrome bitmap will be blitted using an AND operation to
  // cut a hole in the destination. The color image will then be ORed
  // with the destination, filling it into the hole, but leaving the
  // surrounding area untouched.
  CDC hdcMem;
  hdcMem.CreateCompatibleDC(pDC);
  CBitmap* hbmT = hdcMem.SelectObject(bmpMask);
  pDC->SetTextColor(RGB(0,0,0));
  pDC->SetBkColor(RGB(255,255,255));
  if (!pDC->BitBlt( xStart, yStart, wWidth, wHeight, &hdcMem, xSource, ySource,
    SRCAND)) {
      TRACE("Transparent Blit failure SRCAND");
  }
  // Also note the use of SRCPAINT rather than SRCCOPY.
  if (!pDC->BitBlt(xStart, yStart, wWidth, wHeight, pTmpDC, xSource, ySource,
    SRCPAINT)) {
      TRACE("Transparent Blit failure SRCPAINT");
  }
  // Now, clean up.
  hdcMem.SelectObject(hbmT);
  hdcMem.DeleteDC();
}

} // namespace mfc
} // namespace ui
} // namespace host
} // namespace psycle


