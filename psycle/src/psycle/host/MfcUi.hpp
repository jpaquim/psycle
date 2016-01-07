#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "Ui.hpp"

namespace psycle {
namespace host {
namespace ui {
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
  
  virtual Region* Clone() const { 
    Region* r = new Region(rgn_);
    r->h_cache_ = h_cache_;
    r->w_cache_ = w_cache_;
    return r;
  }  
  void Offset(double dx, double dy) {
    CPoint pt(dx, dy);
    rgn_.OffsetRgn(pt);
  }
  int Combine(const ui::Region& other, int combinemode) {
    
    return rgn_.CombineRgn(&rgn_, (const CRgn*)other.source(), combinemode);
  }  
  ui::Rect bounds() const {
    CRect rc;
    rgn_.GetRgnBox(&rc);
    ui::Rect bounds; 
    bounds.setxywh(rc.left,
                   rc.top,
                   w_cache_ != -1 ? w_cache_ : rc.Width(),
                   h_cache_ != -1 ? h_cache_ : rc.Height());
    return bounds;
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

class Font : public ui::Font {
 public:
  Font() {
    LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));
    lfLogFont.lfHeight = 12;    
    strcpy(lfLogFont.lfFaceName, "Arial");
    cfont_.CreateFontIndirect(&lfLogFont);  
  }

  Font::Font(const Font& other) {       
    cfont_.DeleteObject();
    LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));
    Font* font = (Font*)(&other);
    font->cfont().GetLogFont(&lfLogFont);
    cfont_.CreateFontIndirect(&lfLogFont);
  }

  Font& operator= (const Font& other) {
    if (this != &other) {
      cfont_.DeleteObject();
      LOGFONT lfLogFont;
      memset(&lfLogFont, 0, sizeof(lfLogFont));
      Font* font = (Font*)(&other);
      font->cfont().GetLogFont(&lfLogFont);
      cfont_.CreateFontIndirect(&lfLogFont);
    }
    return *this;
  }

  virtual ~Font() {
    cfont_.DeleteObject();
  }

  virtual Font* Clone() const { 
    Font* clone = new Font();
    clone->set_info(info());    
    return clone;
  }

  virtual void set_info(const FontInfo& info) {
    LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));
    lfLogFont.lfHeight = info.height;
    strcpy(lfLogFont.lfFaceName, info.name.c_str());
    cfont_.DeleteObject();
    cfont_.CreateFontIndirect(&lfLogFont);
  }

  virtual FontInfo info() const {
    LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));
    cfont_.GetLogFont(&lfLogFont);
    FontInfo info;
    info.height = lfLogFont.lfHeight;
    info.name = lfLogFont.lfFaceName;
    return info;
  }

  CFont& cfont() const { return cfont_; }

 private:
   mutable CFont cfont_;
};

// ==============================================
// mfc::Graphics
// ==============================================

class Graphics : public ui::Graphics {
 public:
  Graphics() 
      : cr_(CDC::FromHandle(GetDC(0))),      
        rgb_color_(0xFFFFFF),
        argb_color_(0xFFFFFF),
        brush(rgb_color_),
        updatepen_(false),
        updatebrush_(false) {
     Init();
  }

  Graphics(CDC* cr) 
      : cr_(cr),
        rgb_color_(0xFFFFFF),
        argb_color_(0xFFFFFF),
        brush(rgb_color_),
        updatepen_(false),
        updatebrush_(false) {
    Init();
  }

  virtual ~Graphics() {
    if (cr_) {
      Dispose();
    }
  }

  virtual void Dispose() {
    cr_->SelectObject(old_pen);
    pen.DeleteObject();
    brush.DeleteObject();
    cr_->SetGraphicsMode(GM_ADVANCED);
    cr_->SetWorldTransform(&rXform);
    cr_->SelectObject(old_font);
    cr_->SetBkMode(OPAQUE);
    cr_->SelectClipRgn(&old_rgn_);
    old_rgn_.DeleteObject();
    clp_rgn_.DeleteObject();
    cr_ = 0;
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
    if (argb_color_ != color) {
      argb_color_ = color;
      rgb_color_ = ToCOLORREF(color);
      updatepen_ = updatebrush_ = true;      
    }
  }

  ARGB color() const { return argb_color_; }

  void DrawArc(const ui::Rect& rect, const Point& start, const Point& end) {
    check_pen_update();
    CRect rc(rect.left(), rect.top(), rect.right(), rect.bottom());    
    cr_->Arc(&rc, CPoint(start.x(), start.y()), CPoint(end.x(), end.y()));
  }

  void DrawLine(double x1, double y1, double x2, double y2) {
    check_pen_update();
    cr_->MoveTo(x1, y1);
    cr_->LineTo(x2, y2);
  }

  void DrawRect(const ui::Rect& rect) {
    check_pen_update();
    check_brush_update();
    CRect rc(rect.left(), rect.top(), rect.right(), rect.bottom());
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
    cr_->FillSolidRect(x, y, x+width, y+height, rgb_color_);
  }

  void FillOval(double x, double y, double width, double height) {
    check_pen_update();
    check_brush_update();
    cr_->Ellipse(x, y, x+width, y+height);
  }

  void FillRegion(const ui::Region& rgn) {    
    check_pen_update();
    check_brush_update();    
    cr_->FillRgn((CRgn*) (&rgn)->source(), &brush);
  }

  void DrawPolygon(const ui::Points& points) {
    check_pen_update();
    cr_->SelectStockObject(NULL_BRUSH);
    std::vector<CPoint> lpPoints;
    ui::Points::const_iterator it = points.begin();
    const int size = points.size();
    for (; it!=points.end(); ++it) {
      lpPoints.push_back(CPoint(it->x(), it->y()));
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
      lpPoints.push_back(CPoint((*it).x(), (*it).y()));
    }
    cr_->Polygon(&lpPoints[0], size);
  }

  void DrawPolyline(const ui::Points& points) {
    check_pen_update();
    std::vector<CPoint> lpPoints;
    ui::Points::const_iterator it = points.begin();
    const int size = points.size();
    for (; it!=points.end(); ++it) {
      lpPoints.push_back(CPoint((*it).x(), (*it).y()));
    }
    cr_->Polyline(&lpPoints[0], size);
  }

  void SetFont(const ui::Font& font) {
    font_ = *dynamic_cast<const ui::mfc::Font*>(&font);
    cr_->SelectObject(font_.cfont());
    cr_->SetTextColor(rgb_color_);
  }

  const Font& font() const { return font_; }

  virtual Size text_size(const std::string& text) const {
    ui::Size size;
    SIZE extents = {0};
    GetTextExtentPoint32(cr_->m_hDC, text.c_str(), text.length(),
      &extents);
    size.set_size(extents.cx, extents.cy);
    return size;
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
      pen.CreatePen(PS_SOLID, 1, rgb_color_);
      cr_->SelectObject(&pen);
      cr_->SetTextColor(rgb_color_);
      updatepen_ = false;
    }
  }

  void check_brush_update() {
    if (updatebrush_) {
      brush.DeleteObject();
      brush.CreateSolidBrush(rgb_color_);
      cr_->SelectObject(&brush);    
      updatebrush_ = false;
    }
  }

  virtual CDC* dc() { return cr_; }
    
  private:
   void Init() {
     assert(cr_);
     pen.CreatePen(PS_SOLID, 1, rgb_color_);
     cr_->SetTextColor(rgb_color_);
     old_pen = cr_->SelectObject(&pen);
     old_brush = cr_->SelectObject(&brush);
     cr_->GetWorldTransform(&rXform);    
     cr_->SetBkMode(TRANSPARENT);
    
     old_font = (CFont*) cr_->SelectObject(&font_.cfont());
     old_rgn_.CreateRectRgn(0, 0, 0, 0);
     clp_rgn_.CreateRectRgn(0, 0, 0, 0);
     ::GetRandomRgn(cr_->m_hDC, old_rgn_, SYSRGN);
     POINT pt = {0,0};
     HWND hwnd = ::WindowFromDC(cr_->m_hDC);
     ::MapWindowPoints(NULL, hwnd, &pt, 1);
     old_rgn_.OffsetRgn(pt);    
   }

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
   mfc::Font font_;
   CFont* old_font;
   ARGB argb_color_;
   RGB rgb_color_;
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

class WindowImp : public CWnd, public ui::WindowImp {
 public:
  WindowImp() : ui::WindowImp() {}
  WindowImp(ui::Window* w) : ui::WindowImp(w) {}
  
  static WindowImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    WindowImp* imp = new WindowImp();
    imp->Create(parent, nID);    
    imp->set_window(w);
    return imp;
  }  

  virtual void dev_set_pos(const ui::Rect& pos) {
    MoveWindow(pos.left(), pos.top(), pos.width(), pos.height());
  }

  virtual ui::Size dev_dim() const {
    CRect rc;            
    GetClientRect(&rc);
    return ui::Size(rc.Width(), rc.Height());
  }
  
  virtual void DevShow() { ShowWindow(SW_SHOW); }  
  virtual void DevHide() { ShowWindow(SW_HIDE); }
  virtual void DevInvalidate() { Invalidate(); }
  virtual void DevInvalidate(ui::Region& rgn) {
    if (m_hWnd) {
      ::RedrawWindow(m_hWnd, NULL, *((CRgn*) rgn.source()), RDW_INVALIDATE | RDW_UPDATENOW);
    }
  }
  virtual void DevSetCapture() { SetCapture(); }  
  virtual void DevReleaseCapture() { ReleaseCapture(); }  
  virtual void DevShowCursor() { while (::ShowCursor(TRUE) < 0); }  
  virtual void DevHideCursor() { while (::ShowCursor(FALSE) >= 0); }
  virtual void DevSetCursorPos(double x, double y) {
    CPoint point(x, y);
    ClientToScreen(&point);
		::SetCursorPos(point.x, point.y);
  }
  virtual void DevSetCursor(CursorStyle style) {}

  virtual void dev_set_parent(Window* window) {
    SetParent(dynamic_cast<CWnd*>(window->imp()));
  }

  BOOL Create(CWnd* pParentWnd, UINT nID) {
   if (!CWnd::Create(NULL, 
                     NULL, 
                     WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		                 CRect(0, 0, 0, 0), 
                     pParentWnd, 
                     nID, 
                     NULL)) {
		  TRACE0("Failed to create window\n");
			return false;
	  }    
    return true;
  }

  virtual void DevDestroy() { DestroyWindow(); }
  
 protected:		
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);   
  virtual BOOL PreTranslateMessage(MSG* pMsg);  
	DECLARE_MESSAGE_MAP()
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
  void OnDestroy();	
  void OnPaint();
  void OnSize(UINT nType, int cx, int cy);  
  // MouseEvents
  void OnLButtonDown(UINT nFlags, CPoint pt) {    
    MouseEvent ev(pt.x, pt.y, 1, nFlags);
    OnDevMouseDown(ev);                  
  }
  void OnRButtonDown(UINT nFlags, CPoint pt) {
    MouseEvent ev(pt.x, pt.y, 2, nFlags);
    OnDevMouseDown(ev);                  
  }
	void OnLButtonDblClk(UINT nFlags, CPoint pt) {
    MouseEvent ev(pt.x, pt.y, 1, nFlags);
    OnDevDblclick(ev);
  }
	void OnMouseMove(UINT nFlags, CPoint pt) {
    MouseEvent ev(pt.x, pt.y, 1, nFlags);
    OnDevMouseMove(ev);                  
  }
	void OnLButtonUp(UINT nFlags, CPoint pt) {
    MouseEvent ev(pt.x, pt.y, 1, nFlags);
    OnDevMouseUp(ev);
  }
	void OnRButtonUp(UINT nFlags, CPoint pt) {
    MouseEvent ev(pt.x, pt.y, 2, nFlags);
    OnDevMouseUp(ev);
  }

 private:
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
  
  static void SetFocusHook();
  static void ReleaseHook();
  static LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam);  
  CBitmap bmpDC;
  static void FilterHook(HWND hwnd);  
  static HHOOK _hook;
  static std::map<HWND, WindowImp*> windows_;
};

class Systems : public ui::Systems {
 public:
  virtual ui::Region* CreateRegion() { return new ui::mfc::Region(); }
  virtual ui::Graphics* CreateGraphics() { return new ui::mfc::Graphics(); }
  virtual ui::Graphics* CreateGraphics(void* dc) { return new ui::mfc::Graphics((CDC*) dc); }
  virtual ui::Image* CreateImage() { return new ui::mfc::Image(); }
  virtual ui::Font* CreateFont() { return new ui::mfc::Font(); }
};


} // namespace mfc
} // namespace ui
} // namespace host
} // namespace psycle