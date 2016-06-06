#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "Ui.hpp"
#include "Scintilla.h"
#include "SciLexer.h"

namespace psycle {
namespace host {
namespace ui {
namespace mfc {

class SystemMetrics : public ui::SystemMetrics {
 public:    
  SystemMetrics() {}
  virtual ~SystemMetrics() {}

  virtual ui::Dimension screen_dimension() const {
    return ui::Dimension(GetSystemMetrics(SM_CXFULLSCREEN),
                         GetSystemMetrics(SM_CYFULLSCREEN));
  }
};

class ImageImp : public ui::ImageImp {
 public:
  ImageImp() { Init(); }
  ImageImp(CBitmap* bmp) {
    Init();
    bmp_ = bmp;
    shared_ = true;      
  }
  ~ImageImp() {
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

  virtual ui::Dimension dim() const {
    ui::Dimension image_dim;
    if (bmp_) {    
      BITMAP bm;
      bmp_->GetBitmap(&bm);
      image_dim.set(bm.bmWidth, bm.bmHeight);
    }    
    return image_dim;
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

inline void ImageImp::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans) {
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

class GraphicsImp : public ui::GraphicsImp {
 public:
  GraphicsImp() 
      : hScreenDC(::GetDC(0)),
        cr_(CDC::FromHandle(hScreenDC)),      
        rgb_color_(0xFFFFFF),
        argb_color_(0xFFFFFF),
        brush(rgb_color_),
        updatepen_(false),
        updatebrush_(false) {       
    Init();     
  }

  GraphicsImp(CDC* cr) 
      : hScreenDC(0),
        cr_(cr),
        rgb_color_(0xFFFFFF),
        argb_color_(0xFFFFFF),
        brush(rgb_color_),
        updatepen_(false),
        updatebrush_(false),
        test(false) {
    Init();
  }

  virtual ~GraphicsImp() {    
    if (cr_) {
      Dispose();
    }
    if (hScreenDC) {      
      ReleaseDC(0, hScreenDC);      
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

  void CopyArea(const ui::Rect& rect, const ui::Point& delta) {    
    cr_->BitBlt(
      rect.left() + delta.x(), 
      rect.top() + delta.y(),
      rect.width(),
      rect.height(),
      cr_,
      delta.x(),
      delta.y(),
      SRCCOPY);
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

  void DrawLine(const ui::Point& p1, const ui::Point& p2) {
    check_pen_update();
    cr_->MoveTo(p1.x(), p1.y());
    cr_->LineTo(p2.x(), p2.y());
  }

  void DrawRect(const ui::Rect& rect) {
    check_pen_update();
    check_brush_update();
    CRect rc(rect.left(), rect.top(), rect.right(), rect.bottom());
    cr_->FrameRect(rc, &brush);
  }
  
  void DrawRoundRect(const ui::Rect& rect, const ui::Dimension& arc_dim) {
    check_pen_update();
    check_brush_update();
    cr_->SelectStockObject(NULL_BRUSH);
    cr_->RoundRect(
      rect.left(),
      rect.top(),
      rect.right(),
      rect.bottom(),
      arc_dim.width(), arc_dim.height());
    cr_->SelectObject(&brush);
  }
  
  void DrawOval(const ui::Rect& rect) {
    check_pen_update();
    check_brush_update();
    cr_->SelectStockObject(NULL_BRUSH);
    cr_->Ellipse(rect.left(), rect.top(), rect.right(), rect.bottom());
    cr_->SelectObject(&brush);
  }

  void FillRoundRect(const ui::Rect& rect, const ui::Dimension& arc_dim) {
    check_pen_update();
    check_brush_update();
    cr_->RoundRect(
      rect.left(),
      rect.top(),
      rect.right(),
      rect.bottom(),
      arc_dim.width(),
      arc_dim.height());
  }

  void FillRect(const ui::Rect& rect) {
    cr_->FillSolidRect(rect.left(), rect.top(), rect.right(), rect.bottom(), rgb_color_);
  }

  void FillOval(const ui::Rect& rect) {
    check_pen_update();
    check_brush_update();
    cr_->Ellipse(rect.left(), rect.top(), rect.right(), rect.bottom());
  }

  void FillRegion(const ui::Region& rgn);

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

  virtual Dimension text_size(const std::string& text) const {
    ui::Dimension size;
    SIZE extents = {0};
    GetTextExtentPoint32(cr_->m_hDC, text.c_str(), text.length(),
      &extents);
    size.set(extents.cx, extents.cy);
    return size;
  }

  void DrawImage(ui::Image* img, double x, double y) {    
    DrawImage(img, x, y, img->dim().width(), img->dim().height(), 0, 0);
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

  void SetClip(const ui::Rect& rect) {
    CRgn rgn;
    rgn.CreateRectRgn(rect.left(), rect.top(), rect.right(), rect.bottom());
    cr_->SelectClipRgn(&rgn);
    rgn.DeleteObject();
  }

  void SetClip(ui::Region* rgn);

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

  virtual void SaveOrigin() {
     saveXform.push(rXform);
   }
   virtual void RestoreOrigin() {
     rXform = saveXform.top();
     saveXform.pop();
     cr_->SetGraphicsMode(GM_ADVANCED);
     cr_->SetWorldTransform(&rXform);
   }
    
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
   HDC hScreenDC;
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
   bool test;
};

inline void GraphicsImp::TransparentBlt(CDC* pDC,
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

class WindowHook {
 public:
  static void SetFocusHook();
  static void ReleaseHook();
  static LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam);    
  static void FilterHook(HWND hwnd);  
  static HHOOK _hook;
  static std::map<HWND, ui::WindowImp*> windows_;
};

struct WindowID { 
  static int id_counter;
  static int auto_id() { return id_counter++; }
};

struct DummyWindow {
  static CWnd* dummy() {
     if (!dummy_wnd_.m_hWnd) {   
       dummy_wnd_.Create(0, "psycleuidummywnd", 0, CRect(0, 0, 0, 0), ::AfxGetMainWnd(), 0);       
     }
    return &dummy_wnd_;
  }
 private:
  static CWnd dummy_wnd_;
};

template <class T, class I>
class WindowTemplateImp : public T, public I {
 public:  
  WindowTemplateImp() : I(), color_(0xFF000000), mouse_enter_(true) {}
  WindowTemplateImp(ui::Window* w) : I(w), color_(0xFF000000), mouse_enter_(true) {}
    
  virtual void dev_set_pos(const ui::Rect& pos);
  virtual ui::Rect dev_pos() const;
  virtual ui::Rect dev_abs_pos() const;
  virtual ui::Rect dev_desktop_pos() const;
  virtual ui::Dimension dev_dim() const {
    CRect rc;            
    GetClientRect(&rc);
    return ui::Dimension(rc.Width(), rc.Height());
  }
  
  
  virtual void DevShow() { ShowWindow(SW_SHOW); }
  virtual void DevHide() { ShowWindow(SW_HIDE); }
  virtual void DevInvalidate() {    
    ::RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    //::RedrawWindow(m_hWnd, NULL, imp->crgn(), RDW_INVALIDATE | RDW_UPDATENOW);
    //Invalidate();
  }
  virtual void DevInvalidate(const ui::Region& rgn) {
    if (m_hWnd) {
      mfc::RegionImp* imp = dynamic_cast<mfc::RegionImp*>(rgn.imp());
      assert(imp);
      ::RedrawWindow(m_hWnd, NULL, imp->crgn(), RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
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
  virtual void dev_set_parent(Window* window);  
  virtual void dev_set_fill_color(ARGB color) { color_ = color; }
  virtual ARGB dev_fill_color() const { return color_; }

  virtual ui::Window* dev_focus_window();
  virtual void DevSetFocus() {    
    if (GetFocus() != this) {
      SetFocus();
    }
  }

protected:  
  virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
  void OnDestroy();	
  void OnPaint();
  void OnSize(UINT nType, int cx, int cy); 

  BOOL OnEraseBkgnd(CDC* pDC) { return 1; }
  
  virtual bool OnDevUpdateArea(ui::Area& area);
  
 protected:
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

  void MapPointToRoot(CPoint& pt) const {
    if (window()->root()) {
      CWnd* root = dynamic_cast<CWnd*>(window()->root()->imp());
      MapWindowPoints(root, &pt, 1);
    }
  }

  void MapPointToDesktop(CPoint& pt) const {
    ::MapWindowPoints(m_hWnd, ::GetDesktopWindow(), &pt, 1);
  }

  BOOL prevent_propagate_event(const ui::Event& ev, MSG* pMsg);
  
 private:
  CBitmap bmpDC;
  ARGB color_;
  BOOL m_bTracking;
  bool mouse_enter_;
  ui::Point dev_pos_;
};

class WindowImp : public WindowTemplateImp<CWnd, ui::WindowImp> {
 public:
  WindowImp() : 
      WindowTemplateImp<CWnd, ui::WindowImp>(),
      cursor_(LoadCursor(0, IDC_ARROW)) {
  }
  WindowImp(ui::Window* w) :
      WindowTemplateImp<CWnd, ui::WindowImp>(w),
      cursor_(LoadCursor(0, IDC_ARROW)) {
  }

  static WindowImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    WindowImp* imp = new WindowImp();
    if (!imp->Create(NULL, 
                     NULL, 
                     WS_CHILD, //  | WS_EX_COMPOSITED,
		                 CRect(0, 0, 0, 0), 
                     parent, 
                     nID, 
                     NULL)) {
		  TRACE0("Failed to create window\n");
			return 0;
	  }            
    imp->set_window(w);    
    return imp;
  } 

  static WindowImp* MakeComposited(ui::Window* w, CWnd* parent, UINT nID) {
    WindowImp* imp = new WindowImp();
    if (!imp->Create(NULL, 
                     NULL, 
                     WS_CHILD | WS_EX_COMPOSITED,
		                 CRect(0, 0, 0, 0), 
                     parent, 
                     nID, 
                     NULL)) {
		  TRACE0("Failed to create window\n");
			return 0;
	  }            
    imp->set_window(w);
    return imp;
  }
  
  virtual void dev_set_clip_children() {
    ModifyStyle(0, WS_CLIPCHILDREN);
  }

  virtual void dev_add_style(UINT flag) { ModifyStyle(0, flag); }
  virtual void dev_remove_style(UINT flag) { ModifyStyle(flag, 0); }

 protected:
  virtual void DevSetCursor(CursorStyle style);
  BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {   
    ::SetCursor(cursor_);
    return TRUE;   
  }  
  DECLARE_MESSAGE_MAP()
 private:
   HCURSOR cursor_;
};

class ScrollBarImp : public WindowTemplateImp<CScrollBar, ui::ScrollBarImp> {
 public:  
  ScrollBarImp() : WindowTemplateImp<CScrollBar, ui::ScrollBarImp>() {}

  static ScrollBarImp* Make(ui::Window* w, CWnd* parent, UINT nID, Orientation orientation) {
    ScrollBarImp* imp = new ScrollBarImp();
    int orientation_flag;
    ui::Dimension size;
    if (orientation == HORZ) {
      size.set(100, GetSystemMetrics(SM_CXHSCROLL));
      orientation_flag = SBS_HORZ;
    } else {
      size.set(GetSystemMetrics(SM_CXVSCROLL), 100);
      orientation_flag = SBS_VERT;
    }
  
    if (!imp->Create(orientation_flag | WS_CHILD | WS_VISIBLE,
                     CRect(0, 0, size.width(), size.height()), 
                     parent, 
                     nID)) {
      TRACE0("Failed to create window\n");
      return 0;
    }
    imp->set_window(w);
    return imp;
  }  

  virtual void dev_set_scroll_range(int minpos, int maxpos) {
    SetScrollRange(minpos, maxpos);
  }

  virtual void dev_scroll_range(int& minpos, int& maxpos) {
    GetScrollRange(&minpos, &maxpos);
  }

  virtual void dev_set_scroll_pos(int pos) { SetScrollPos(pos); }
  virtual int dev_scroll_pos() const { return GetScrollPos(); }
  virtual ui::Dimension dev_system_size() const {
    return ui::Dimension(GetSystemMetrics(SM_CXVSCROLL), 
                    GetSystemMetrics(SM_CXHSCROLL));    
  }
 protected:
   DECLARE_MESSAGE_MAP()
  void OnPaint() { CScrollBar::OnPaint(); }
};

class ButtonImp : public WindowTemplateImp<CButton, ui::ButtonImp> {
 public:  
  ButtonImp() : WindowTemplateImp<CButton, ui::ButtonImp>() {}

  static ButtonImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    ButtonImp* imp = new ButtonImp();
    if (!imp->Create("Button", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | DT_CENTER,
                     CRect(0, 0, 55, 19), 
                     parent, 
                     nID)) {
      TRACE0("Failed to create window\n");
      return 0;
    }
    imp->set_window(w);
    return imp;
  }

  BOOL OnEraseBkgnd(CDC* pDC) { return CButton::OnEraseBkgnd(pDC); }

  virtual void dev_set_text(const std::string& text) { SetWindowText(text.c_str()); }
  virtual std::string dev_text() const {
    CString s;    
    GetWindowTextA(s);
    return s.GetString();
  }

protected:
  DECLARE_MESSAGE_MAP()
  void OnPaint() { CButton::OnPaint(); }
  void OnClick();
};

class FrameImp : public WindowTemplateImp<CFrameWnd, ui::FrameImp> {
 public:  
  FrameImp() : WindowTemplateImp<CFrameWnd, ui::FrameImp>() {}

  static FrameImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    FrameImp* imp = new FrameImp();    
    if (!imp->Create(NULL, "PsycleWindow", WS_OVERLAPPEDWINDOW,
                     CRect(0, 0, 200, 200), 
                     parent)) {
      TRACE0("Failed to create frame window\n");
      return 0;
    }
    imp->set_window(w);        
    return imp;
  }
    
  virtual void dev_set_title(const std::string& title) {
    SetWindowTextA(_T(title.c_str()));
  }

  virtual void dev_set_view(ui::Window::Ptr view) {
    CWnd* wnd = dynamic_cast<CWnd*>(view->imp());
    if (wnd) {      
      wnd->SetParent(this);      
      view_ = view;
    }
  }

  virtual void DevShowDecoration();
  virtual void DevHideDecoration();

  void OnSize(UINT nType, int cx, int cy) {
    if (view_) {
      CRect rect;
      this->GetClientRect(rect);
      CWnd* wnd = dynamic_cast<CWnd*>(view_->imp());
      if (wnd) {
        wnd->MoveWindow(&rect);
      }
    }
  }
     
 protected:
  DECLARE_MESSAGE_MAP()
  void OnPaint() { CFrameWnd::OnPaint(); }
  BOOL OnEraseBkgnd(CDC* pDC) { return 1; }
  virtual void OnClose() { OnDevClose(); }
  ui::Window::Ptr view_;
};

class MenuImp;

class MenuContainerImp : public ui::MenuContainerImp { 
 public:  
  MenuContainerImp();
 
  virtual void DevUpdate(boost::shared_ptr<Node> node, boost::shared_ptr<Node> prev_node);
  virtual void DevErase(boost::shared_ptr<Node> node);
  virtual void DevInvalidate();
  virtual void DevTrack(const ui::Point& pos) {}

  void RegisterMenuEvent(std::uint16_t id, MenuImp* menu_imp);
  MenuImp* FindMenuItemById(int id);
  static MenuContainerImp* MenuContainerImpById(int id);
  void WorkMenuItemEvent(int id);
  void set_menu_window(CWnd* menu_window, const Node::Ptr& root_node);
  void set_cmenu(CMenu* cmenu) { cmenu_ = cmenu; }
  
 private:
   void UpdateNodes(Node::Ptr parent_node, CMenu* parent, int pos_start = 0);
   std::map<std::uint16_t, MenuImp*> menu_item_id_map_;
   static std::map<std::uint16_t, MenuContainerImp*> menu_bar_id_map_;   
   CWnd* menu_window_;
   CMenu* cmenu_;
};

class PopupMenuImp : public MenuContainerImp { 
 public:  
  PopupMenuImp();
  virtual ~PopupMenuImp() { popup_menu_->DestroyMenu(); }
 
  virtual void DevTrack(const ui::Point& pos);
 private:
   std::auto_ptr<CMenu> popup_menu_;
};

class MenuImp : public ui::MenuImp {  
 public:
  MenuImp() : cmenu_(0), parent_(0), pos_(0), id_(0) {}
  MenuImp(CMenu* parent) : cmenu_(0), parent_(parent), pos_(0), id_(0) {}
  virtual ~MenuImp() {
    if (parent_ && ::IsMenu(parent()->m_hMenu)) {      
      parent()->RemoveMenu(pos_, MF_BYPOSITION);    
    } else {
      delete cmenu_;
    }
  }
    
  void set_parent(CMenu* parent) { parent_ = parent; }
  CMenu* parent() { return parent_; }
  virtual void dev_set_text(const std::string& text) {    
    parent()->ModifyMenu(pos_, MF_BYPOSITION, 0, text.c_str());    
  }
  void dev_set_pos(int pos) { pos_ = pos; }
  int dev_pos() const { return pos_; }
  void CreateMenu(const std::string& text);
  void CreateMenuItem(const std::string& text, ui::Image* image);

  CMenu* cmenu() { return cmenu_; }
  int id() const { return id_; }
  
 private:
    CMenu* cmenu_;
    CMenu* parent_;    
    int pos_;
    int id_;
};

class TreeViewImp;

class TreeNodeImp : public NodeImp {  
 public:
  TreeNodeImp() : hItem(0) {}
  ~TreeNodeImp() {}
      
  HTREEITEM DevInsert(ui::mfc::TreeViewImp* tree, const ui::Node& node, TreeNodeImp* prev_imp);
    
  HTREEITEM hItem;  
  std::string text_;
};

class TreeViewImp : public WindowTemplateImp<CTreeCtrl, ui::TreeViewImp> {
 public:  
  TreeViewImp() : 
      WindowTemplateImp<CTreeCtrl, ui::TreeViewImp>(), 
      is_editing_(false) {
    m_imageList.Create(22, 22, ILC_COLOR32, 1, 1);
  }
  static TreeViewImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    TreeViewImp* imp = new TreeViewImp();
    if (!imp->Create(WS_CHILD | TVS_EDITLABELS, 
                     CRect(0, 0, 200, 200), 
                     parent, 
                     nID)) {
      TRACE0("Failed to create window\n");
      return 0;
    }
    imp->set_window(w);
    WindowHook::windows_[imp->GetSafeHwnd()] = imp;
    imp->SetLineColor(0xFFFFFF);
    return imp;
  }

  ui::TreeView* tree_view() { 
    ui::TreeView* result = dynamic_cast<ui::TreeView*>(window());
    assert(result);
    return result;
  }  
  virtual void DevClear();  
  virtual void DevUpdate(boost::shared_ptr<Node> node, boost::shared_ptr<Node> prev_node);
  virtual void DevErase(boost::shared_ptr<Node> node);

  // virtual void DevUpdate(boost::shared_ptr<Node> node, boost::shared_ptr<Node> nodeafter);
  virtual void DevEditNode(boost::shared_ptr<ui::Node> node);
  virtual void dev_set_background_color(ARGB color) { 
    SetBkColor(ToCOLORREF(color));
  }  
  virtual ARGB dev_background_color() const { return ToARGB(GetBkColor()); }  
  virtual void dev_set_text_color(ARGB color) {
    SetTextColor(ToCOLORREF(color));
  }
  virtual ARGB dev_text_color() const { return ToARGB(GetTextColor()); }
  BOOL OnEraseBkgnd(CDC* pDC) { return CTreeCtrl::OnEraseBkgnd(pDC); }
  
  void dev_select_node(const boost::shared_ptr<ui::Node>& node);
  virtual boost::weak_ptr<Node> dev_selected();  
  void OnNodeChanged(Node& node);
  bool dev_is_editing() const { return is_editing_; }
  std::map<HTREEITEM, boost::weak_ptr<ui::Node> > htreeitem_node_map_;
  void dev_add_item(boost::shared_ptr<Node> node);
  virtual void DevShowLines();
  virtual void DevHideLines();
  virtual void DevShowButtons();
  virtual void DevHideButtons();
  
  virtual void dev_set_images(const ui::Images::Ptr& images) {
    ui::Images::iterator it = images->begin();
    for (; it != images->end(); ++it) {
      m_imageList.Add((CBitmap*)(*it)->source(),  (COLORREF)0xFFFFFF);    
    }
    SetImageList(&m_imageList, TVSIL_NORMAL);
  }
  CImageList m_imageList;

  virtual ui::Node::Ptr dev_node_at(const ui::Point& pos) const;

 protected:  
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  DECLARE_MESSAGE_MAP()
  void OnPaint() { CTreeCtrl::OnPaint(); }
  BOOL OnChange(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnRightClick(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnDblClick(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnBeginLabelEdit(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnEndLabelEdit(NMHDR * pNotifyStruct, LRESULT * result);
  ui::Node* find_selected_node();

 private:
   bool is_editing_;   
};

class ListViewImp;

class ListNodeImp : public NodeImp {  
 public:
  ListNodeImp() : pos_(0) {} // : hItem(0) {}
  ~ListNodeImp() {}

  void set_pos(int pos) { pos_ = pos; }
  int pos() const { return pos_; }
      
  // LVITEM DevInsert(ui::mfc::ListViewImp* list, const ui::Node& node, ListNodeImp* prev_imp);
  void DevInsertFirst(ui::mfc::ListViewImp* list, const ui::Node& node, ListNodeImp* node_imp, ListNodeImp* prev_imp);
  void DevSetSub(ui::mfc::ListViewImp* list, const ui::Node& node, ListNodeImp* node_imp, ListNodeImp* prev_imp, int level);
  // LVITEM hItem;  
  std::string text_;
  int pos_;
};

class ListViewImp : public WindowTemplateImp<CListCtrl, ui::ListViewImp> {
 public:  
  ListViewImp() : 
      WindowTemplateImp<CListCtrl, ui::ListViewImp>(), 
      is_editing_(false),
      column_pos_(0) {
    m_imageList.Create(22, 22, ILC_COLOR32, 1, 1);
  }
  static ListViewImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    ListViewImp* imp = new ListViewImp();
    if (!imp->Create(WS_CHILD | TVS_EDITLABELS, 
                     CRect(0, 0, 200, 200), 
                     parent, 
                     nID)) {
      TRACE0("Failed to create window\n");
      return 0;
    }
    imp->set_window(w);
    WindowHook::windows_[imp->GetSafeHwnd()] = imp;
    // imp->SetLineColor(0xFFFFFF);
    imp->DevViewList();
    return imp;
  }

  ui::ListView* list_view() { 
    ui::ListView* result = dynamic_cast<ui::ListView*>(window());
    assert(result);
    return result;
  }  
  virtual void DevClear();  
  virtual void DevUpdate(boost::shared_ptr<Node> node, boost::shared_ptr<Node> prev_node);
  virtual void DevErase(boost::shared_ptr<Node> node);  
  virtual void DevEditNode(boost::shared_ptr<ui::Node> node);
  virtual void dev_set_background_color(ARGB color) {     
    SetBkColor(ToCOLORREF(color));
  }  
  virtual ARGB dev_background_color() const { return ToARGB(GetBkColor()); }  
  virtual void dev_set_text_color(ARGB color) {
    SetTextColor(ToCOLORREF(color));
  }
  virtual ARGB dev_text_color() const { return ToARGB(GetTextColor()); }
  BOOL OnEraseBkgnd(CDC* pDC) { return CListCtrl::OnEraseBkgnd(pDC); }
  
  void dev_select_node(const boost::shared_ptr<ui::Node>& node);
  virtual boost::weak_ptr<Node> dev_selected();  
  void OnNodeChanged(Node& node);
  bool dev_is_editing() const { return is_editing_; }
  std::map<HTREEITEM, boost::weak_ptr<ui::Node> > htreeitem_node_map_;
  void dev_add_item(boost::shared_ptr<Node> node);
  virtual void dev_set_images(const ui::Images::Ptr& images) {
    ui::Images::iterator it = images->begin();
    for (; it != images->end(); ++it) {
      m_imageList.Add((CBitmap*)(*it)->source(),  (COLORREF)0xFFFFFF);    
    }
    SetImageList(&m_imageList, TVSIL_NORMAL);
  }

  virtual void DevViewList() { SetView(LV_VIEW_LIST); }
  virtual void DevViewReport() { SetView(LV_VIEW_DETAILS); }
  virtual void DevViewIcon() { SetView(LV_VIEW_ICON); }
  virtual void DevViewSmallIcon() { SetView(LV_VIEW_SMALLICON); }

  virtual void DevAddColumn(const std::string& text, int width) {
    InsertColumn(column_pos_++, _T(text.c_str()), LVCFMT_LEFT, width); //, nColInterval*3);
  }

  CImageList m_imageList;

 protected:  
  DECLARE_MESSAGE_MAP()
  void OnPaint() { CListCtrl::OnPaint(); }
  BOOL OnChange(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnRightClick(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnBeginLabelEdit(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnEndLabelEdit(NMHDR * pNotifyStruct, LRESULT * result);
  void OnCustomDrawList(NMHDR *pNMHDR, LRESULT *pResult);
  ui::Node* find_selected_node();

 private:
   bool is_editing_;
   int column_pos_;
};

class ComboBoxImp : public WindowTemplateImp<CComboBox, ui::ComboBoxImp> {
 public:  
  ComboBoxImp() : WindowTemplateImp<CComboBox, ui::ComboBoxImp>() {}

  static ComboBoxImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    ComboBoxImp* imp = new ComboBoxImp();
    if (!imp->Create(WS_CHILD|WS_VSCROLL|CBS_DROPDOWNLIST|WS_VISIBLE,
                     CRect(10,10,200,100), 
                     parent, 
                     nID)) {
      TRACE0("Failed to create window\n");
      return 0;
    }
    imp->set_window(w);    
    WindowHook::windows_[imp->GetSafeHwnd()] = imp;
    return imp;
  }  
  
  void dev_set_items(const std::vector<std::string>& itemlist) {
    ResetContent();
    std::vector<std::string>::const_iterator it = itemlist.begin();
    for (; it != itemlist.end(); ++it) {      
      AddString((*it).c_str());
    }
  }

  virtual std::vector<std::string> dev_items() const {
    std::vector<std::string> itemlist;
    for (int i = 0; i < this->GetCount(); ++i) {
      CString str;
      GetLBText(i, str);
      itemlist.push_back(str.GetString());
    }
    return itemlist;
  }
  
  virtual void dev_set_item_index(int index) { SetCurSel(index); }
  virtual int dev_item_index() const { return GetCurSel(); }
  
  BOOL OnEraseBkgnd(CDC* pDC) { return CComboBox::OnEraseBkgnd(pDC); }

 protected:
  DECLARE_MESSAGE_MAP()
  void OnPaint() { CComboBox::OnPaint(); }
  BOOL OnSelect();
};

class EditImp : public WindowTemplateImp<CEdit, ui::EditImp> {
 public:  
  EditImp() : WindowTemplateImp<CEdit, ui::EditImp>() {}

  static EditImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    EditImp* imp = new EditImp();
    if (!imp->Create(WS_CHILD | WS_TABSTOP | WS_VISIBLE,
                     CRect(10,10,200,100), 
                     parent, 
                     nID)) {
      TRACE0("Failed to create window\n");
      return 0;
    }    
    imp->set_window(w);
    WindowHook::windows_[imp->GetSafeHwnd()] = imp;
    // Set the hook
    WindowHook::SetFocusHook();
    return imp;
  }

  BOOL OnEraseBkgnd(CDC* pDC) { return CEdit::OnEraseBkgnd(pDC); }

  virtual void dev_set_text(const std::string& text) { SetWindowText(text.c_str()); }
  virtual std::string dev_text() const {
    CString s;    
    GetWindowTextA(s);
    return s.GetString();
  }

protected:
  DECLARE_MESSAGE_MAP()
  void OnPaint() { CEdit::OnPaint(); }
};


class ScintillaImp : public WindowTemplateImp<CWnd, ui::ScintillaImp> {
 public:  
  static std::string type() { return "canvasscintillaitem"; }
  
  ScintillaImp() : 
      WindowTemplateImp<CWnd, ui::ScintillaImp>(),
      find_flags_(0),      
      has_file_(false) {    
    //ctrl().modified.connect(boost::bind(&Scintilla::OnFirstModified, this));
    //ctrl().Create(p_wnd(), id());       
  }

  static ScintillaImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    ScintillaImp* imp = new ScintillaImp();
    if (!imp->Create(parent, nID)) {
      TRACE0("Failed to create window\n");
      return 0;
    }
    imp->set_window(w);
    WindowHook::windows_[imp->GetSafeHwnd()] = imp;
    return imp;
  }
          
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

    f(SCI_SETMARGINWIDTHN, 0, 32);
    return true;
  }

  virtual int dev_f(int sci_cmd, void* lparam, void* wparam) {
    return f(sci_cmd, lparam, wparam);
  }

  template<class T, class T1>
  int f(int sci_cmd, T lparam, T1 wparam) {     
    return fn(ptr, sci_cmd, (WPARAM) lparam, (LPARAM) wparam);
  }

  template<class T, class T1>
  int f(int sci_cmd, T lparam, T1 wparam) const {     
    return fn(ptr, sci_cmd, (WPARAM) lparam, (LPARAM) wparam);
  }
    
  void DevAddText(const std::string& text) {       
    f(SCI_ADDTEXT, text.size(), text.c_str());
  }

  void DevFindText(const std::string& text, int cpmin, int cpmax, int& pos, int& cpselstart, int& cpselend) const {
    TextToFind txt;
    txt.chrg.cpMin = cpmin;      
    txt.chrg.cpMax = cpmax;
    txt.lpstrText = text.c_str();
    pos = const_cast<ScintillaImp*>(this)->f(SCI_FINDTEXT, find_flags_, &txt);
    cpselstart = txt.chrgText.cpMin;
    cpselend = txt.chrgText.cpMax;
  }

  void DevGotoLine(int pos) { f(SCI_GOTOLINE, pos, 0); }
  int dev_length() const {
    return const_cast<ScintillaImp*>(this)->f(SCI_GETLENGTH, 0, 0);
  }
  int dev_selectionstart() const {
    return const_cast<ScintillaImp*>(this)->f(SCI_GETSELECTIONSTART, 0, 0);     
  }
  int dev_selectionend() const {
    return const_cast<ScintillaImp*>(this)->f(SCI_GETSELECTIONEND, 0, 0);     
  }
  void DevSetSel(int cpmin, int cpmax) { f(SCI_SETSEL, cpmin, cpmax); }
  bool dev_has_selection() const { return dev_selectionstart() != dev_selectionend(); }
 
  void dev_set_find_match_case(bool on) {     
   if (on) {
      find_flags_ = find_flags_ | SCFIND_MATCHCASE;
    } else {
      find_flags_ = find_flags_ & ~SCFIND_MATCHCASE;
    }
  }

  void dev_set_find_whole_word(bool on) {
    if (on) {
      find_flags_ = find_flags_ | SCFIND_WHOLEWORD;
    } else {
      find_flags_ = find_flags_ & ~SCFIND_WHOLEWORD;
    }
  }

  void dev_set_find_regexp(bool on) {     
    find_flags_ = (on) ? (find_flags_ | SCFIND_REGEXP) : (find_flags_ & ~SCFIND_REGEXP);    
  }

  void DevLoadFile(const std::string& filename) {
    using namespace std;    
    #if __cplusplus >= 201103L
      ifstream file (filename, ios::in|ios::binary|ios::ate);
    #else
      ifstream file (filename.c_str(), ios::in|ios::binary|ios::ate);
    #endif
    if (file.is_open()) {      
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
      has_file_ = true;
    } else {
      throw std::runtime_error("File Not Open Error!");
    }    
  }

  virtual void dev_set_lexer(const Lexer& lexer);

  void dev_set_foreground_color(ARGB color) { f(SCI_STYLESETFORE, STYLE_DEFAULT, ToCOLORREF(color)); }
  ARGB dev_foreground_color() const { return ToARGB(f(SCI_STYLEGETFORE, STYLE_DEFAULT, 0)); }
  void dev_set_background_color(ARGB color) { f(SCI_STYLESETBACK, STYLE_DEFAULT, ToCOLORREF(color)); }
  ARGB dev_background_color() const { return ToARGB(f(SCI_STYLEGETBACK, STYLE_DEFAULT, 0)); }

  void dev_set_linenumber_foreground_color(ARGB color) { f(SCI_STYLESETFORE, STYLE_LINENUMBER, ToCOLORREF(color)); }
  ARGB dev_linenumber_foreground_color() const { return ToARGB(f(SCI_STYLEGETFORE, STYLE_LINENUMBER, 0)); }
  void dev_set_linenumber_background_color(ARGB color) { f(SCI_STYLESETBACK, STYLE_LINENUMBER, ToCOLORREF(color)); }
  ARGB dev_linenumber_background_color() const { return ToARGB(f(SCI_STYLEGETBACK, STYLE_LINENUMBER, 0)); }

  void dev_set_margin_background_color(ARGB color) {
    f(SCI_SETFOLDMARGINCOLOUR,1, color);
    f(SCI_SETFOLDMARGINHICOLOUR,1, color);
  }
  ARGB dev_margin_background_color() const { return 0; }
  
  void dev_set_sel_foreground_color(ARGB color) { f(SCI_SETSELFORE, 1, ToCOLORREF(color)); }
 // COLORREF sel_foreground_color() const { return f(SCI_GETSELFORE, 0, 0); }
  void dev_set_sel_background_color(ARGB color) { f(SCI_SETSELBACK, 1, ToCOLORREF(color)); }
 // COLORREF sel_background_color() const { return f(SCI_STYLEGETBACK, STYLE_sel, 0); }
  void dev_set_sel_alpha(int alpha) { ToARGB(f(SCI_SETSELALPHA, alpha, 0)); }

  void dev_set_ident_color(ARGB color) { f(SCI_STYLESETFORE, STYLE_INDENTGUIDE, ToCOLORREF(color)); }
  void dev_set_caret_color(ARGB color) { f(SCI_SETCARETFORE, ToCOLORREF(color), 0); }
  ARGB dev_caret_color() const { return ToARGB(f(SCI_GETCARETFORE, 0, 0)); }
  void DevStyleClearAll() { f(SCI_STYLECLEARALL, 0, 0); }

  void DevSaveFile(const std::string& filename) {
    //Get the length of the document
    int nDocLength = f(SCI_GETLENGTH, 0, 0);
    //Write the data in blocks to disk
    CFile file;
    BOOL res = file.Open(_T(filename.c_str()), CFile::modeCreate | CFile::modeReadWrite);
    if (res) {      
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
    f(SCI_SETSAVEPOINT, 0, 0);
    } else {
      throw std::runtime_error("File Save Error");
    }
  }  
  bool dev_has_file() const { return has_file_; }
  virtual const std::string& dev_filename() const { return fname_; }  
  virtual void dev_set_font(const FontInfo& font_info) {
    f(SCI_STYLESETSIZE, STYLE_DEFAULT, font_info.height);
    f(SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM) (font_info.name.c_str())); 
  }    
  boost::signal<void ()> modified;
  int dev_column() const {
    Sci_Position pos = f(SCI_GETCURRENTPOS, 0, 0);
    return f(SCI_GETCOLUMN, pos, 0);    
  }
  int dev_line() const {  
    Sci_Position pos = f(SCI_GETCURRENTPOS, 0, 0);    
    return f(SCI_LINEFROMPOSITION, pos, 0);
  }
  bool dev_ovr_type() const {
    return f(SCI_GETOVERTYPE, 0, 0);
  }
  bool dev_modified() const {
    return f(SCI_GETMODIFY, 0, 0);
  }
            
 protected:
  DECLARE_DYNAMIC(ScintillaImp)     
  int (*fn)(void*,int,int,int);
  void * ptr;
   
  BOOL OnModified(NMHDR *,LRESULT *) {
    modified();
    return false;
  }
 
  BOOL OnFolder(NMHDR * nhmdr,LRESULT *);  

  DECLARE_MESSAGE_MAP(); 

 private:         
  DWORD find_flags_;
  std::string fname_;
  bool has_file_;

  void SetupHighlighting(const Lexer& lexer);
  void SetupFolding(const Lexer& lexer);
  void SetFoldingBasics();
  void SetFoldingColors(const Lexer& lexer);
  void SetFoldingMarkers();
  void SetupLexerType();
};

class RegionImp : public ui::RegionImp {
 public:
  RegionImp();   
  RegionImp(const CRgn& rgn);
  virtual ~RegionImp();
  
  virtual RegionImp* DevClone() const;
  void DevOffset(double dx, double dy);
  int DevCombine(const ui::Region& other, int combinemode);     
  ui::Rect DevBounds() const;
  bool DevIntersect(double x, double y) const;
  bool DevIntersectRect(const ui::Rect& rect) const;
  virtual void DevSetRect(const ui::Rect& rect);
  void DevClear();
  CRgn& crgn() { return rgn_; }

 private:  
  CRgn rgn_;
};

class GameControllersImp : public ui::GameControllersImp {
 public:
   GameControllersImp() {}
   virtual ~GameControllersImp() {}
   
   virtual void DevScanPluggedControllers(std::vector<int>& plugged_controller_ids);
   virtual void DevUpdateController(ui::GameController& controller);
};

class Systems : public ui::Systems {
 public:
  virtual ui::Region* CreateRegion() { return new ui::Region(); }
  virtual ui::Graphics* CreateGraphics() { return new ui::Graphics(); }
  virtual ui::Graphics* CreateGraphics(void* dc) { return new ui::Graphics((CDC*) dc); }
  virtual ui::Image* CreateImage() { return new ui::Image(); }
  virtual ui::Font* CreateFont() { return new ui::mfc::Font(); }
  virtual ui::Window* CreateWin() { 
    Window* window = new Window();
    WindowImp* imp = WindowImp::Make(window, DummyWindow::dummy(), WindowID::auto_id());
    imp->set_window(window);
    return window;
  }
  virtual ui::ScrollBar* CreateScrollBar() { 
    ScrollBar* window = new ScrollBar();
    ScrollBarImp* imp = ScrollBarImp::Make(window, DummyWindow::dummy(), WindowID::auto_id(), ui::VERT);
    imp->set_window(window);
    return window;
  }
  virtual ui::ComboBox* CreateComboBox() { 
    ComboBox* window = new ComboBox();
    ComboBoxImp* imp = ComboBoxImp::Make(window, DummyWindow::dummy(), WindowID::auto_id());
    imp->set_window(window);
    return window;
  }
  virtual ui::Edit* CreateEdit() { 
    Edit* window = new Edit();
    EditImp* imp = EditImp::Make(window, DummyWindow::dummy(), WindowID::auto_id());
    imp->set_window(window);
    return window;
  }
  virtual ui::Button* CreateButton() { 
    Button* window = new Button();
    ButtonImp* imp = ButtonImp::Make(window, DummyWindow::dummy(), WindowID::auto_id());
    imp->set_window(window);
    return window;
  }
  virtual ui::MenuContainer* CreateMenuContainer() { 
    MenuContainer* bar = new MenuContainer();    
    return bar;
  }
};

class ImpFactory : public ui::ImpFactory {
 public:
  virtual bool DestroyWindowImp(ui::WindowImp* imp) {
    if (imp) {    
      CWnd* wnd = (CWnd*) imp;
      if (::IsWindow(wnd->m_hWnd)) { 
        wnd->DestroyWindow();
        return true;
      }
    }
    return false;     
  }

  virtual ui::WindowImp* CreateWindowImp() {
    return WindowImp::Make(0, DummyWindow::dummy(), WindowID::auto_id());    
  }
  virtual ui::WindowImp* CreateWindowCompositedImp() {
    return WindowImp::MakeComposited(0, DummyWindow::dummy(), WindowID::auto_id());    
  }
  virtual ui::FrameImp* CreateFrameImp() {     
    return FrameImp::Make(0, ::AfxGetMainWnd(), WindowID::auto_id());    
  }
  virtual ui::ScrollBarImp* CreateScrollBarImp(Orientation orientation) {     
    return ScrollBarImp::Make(0, DummyWindow::dummy(), WindowID::auto_id(), orientation);    
  }
  virtual ui::ComboBoxImp* CreateComboBoxImp() {     
    return ComboBoxImp::Make(0, DummyWindow::dummy(), WindowID::auto_id());    
  }
  virtual ui::ButtonImp* CreateButtonImp() {     
    return ButtonImp::Make(0, DummyWindow::dummy(), WindowID::auto_id());    
  }  
  virtual ui::EditImp* CreateEditImp() {     
    return EditImp::Make(0, DummyWindow::dummy(), WindowID::auto_id());    
  }  
  virtual ui::ScintillaImp* CreateScintillaImp() {     
    return ScintillaImp::Make(0, DummyWindow::dummy(), WindowID::auto_id());    
  }  
  virtual ui::TreeViewImp* CreateTreeViewImp() {     
    return TreeViewImp::Make(0, DummyWindow::dummy(), WindowID::auto_id());    
  }
  virtual ui::ListViewImp* CreateListViewImp() {     
    return ListViewImp::Make(0, DummyWindow::dummy(), WindowID::auto_id());    
  }
  virtual ui::RegionImp* CreateRegionImp() {     
    return new ui::mfc::RegionImp();
  }
  virtual ui::GraphicsImp* CreateGraphicsImp() {     
    return new ui::mfc::GraphicsImp();
  }  
  virtual ui::GraphicsImp* CreateGraphicsImp(CDC* cr) {     
    return new ui::mfc::GraphicsImp(cr);
  }
  virtual ui::ImageImp* CreateImageImp() {     
    return new ui::mfc::ImageImp();
  }
  virtual ui::MenuContainerImp* CreateMenuContainerImp() {     
    return new MenuContainerImp();
  }
  virtual ui::MenuContainerImp* CreatePopupMenuImp() {     
    return new PopupMenuImp();
  } 
  virtual ui::MenuImp* CreateMenuImp() {
    return new MenuImp(0);
  }  
  virtual ui::GameControllersImp* CreateGameControllersImp() {
    return new GameControllersImp();
  }
};


} // namespace mfc
} // namespace ui
} // namespace host
} // namespace psycle