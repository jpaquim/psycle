#pragma once

#pragma warning(disable: 4996)
// #include <psycle/host/detail/project.hpp>
#include "atlimage.h"
#include "Ui.hpp"
#include "Scintilla.h"
#include "SciLexer.h"


#include <fstream>
#include <string>
#include <iostream>

namespace psycle {
namespace host {
namespace ui {
namespace mfc {


class Charset {
public:	
#ifdef UNICODE
 static std::wstring utf8_to_win(const std::string& str) {
   std::wstring convertedString;
    int requiredSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);
    if(requiredSize > 0) {
        std::vector<wchar_t> buffer(requiredSize);
        MultiByteToWideChar(CP_UTF8, 0, utf8_str_.c_str(), -1, &buffer[0], requiredSize);
        convertedString.assign(buffer.begin(), buffer.end() - 1);
    }
    return convertedString;
 }
 static std::string win_to_utf8(const std::wstring& wstr) {
   std::string convertedString;
   int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, 0, 0, 0, 0);
   if(requiredSize > 0) {
     std::vector<char> buffer(requiredSize);
     WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &buffer[0], requiredSize, 0, 0);
     convertedString.assign(buffer.begin(), buffer.end() - 1);
    }
   return convertedString;
 }
#else
static std::string utf8_to_win(const std::string& str) {
//	return boost::locale::conv::from_utf(str, std::locale());  
	return str;
}
static std::string win_to_utf8(const std::string& str) { return str; }
#endif
};

class TypeConverter {
 public:
	inline static CRect rect(const ui::Rect& rect) {
		return CRect(static_cast<int>(rect.left()),
			           static_cast<int>(rect.top()),
			           static_cast<int>(rect.right()),
			           static_cast<int>(rect.bottom()));
	}

	inline static CPoint point(const ui::Point& point) {
		return CPoint(static_cast<int>(point.x()), static_cast<int>(point.y()));
	}
	inline static CPoint point(const ui::Dimension& dimension) {
		return CPoint(static_cast<int>(dimension.width()), static_cast<int>(dimension.height()));
	}
};

class SystemMetrics : public ui::SystemMetrics {
 public:    
  SystemMetrics() {}
  virtual ~SystemMetrics() {}

  virtual ui::Dimension screen_dimension() const {
    return ui::Dimension(GetSystemMetrics(SM_CXFULLSCREEN),
                         GetSystemMetrics(SM_CYFULLSCREEN));
  }
  virtual ui::Dimension scrollbar_size() const {
    return ui::Dimension(GetSystemMetrics(SM_CXHSCROLL),
                         GetSystemMetrics(SM_CXVSCROLL));
  }
};

class ImageImp : public ui::ImageImp {
 public:
  ImageImp() : bmp_(0), shared_(false) {}
  ImageImp(CBitmap* bmp) : bmp_(bmp), shared_(true) {}

  ~ImageImp() {
    if (!shared_ && bmp_) {
      bmp_->DeleteObject();
      delete bmp_;
    }    
    if (mask_.m_hObject) {
      mask_.DeleteObject();
    }
  }

  virtual ImageImp* DevClone() const {		
		ImageImp* clone = new ImageImp();
		if (bmp_) {												
			CDC src_dc; 
			src_dc.CreateCompatibleDC(NULL); 
			CBitmap* old_bmp_src = src_dc.SelectObject(bmp_); 
			BITMAP bm; 
			bmp_->GetBitmap(&bm);
			CDC dest_dc; 
			dest_dc.CreateCompatibleDC(NULL);
			clone->bmp_ = new CBitmap();
			clone->bmp_->CreateCompatibleBitmap(&src_dc, bm.bmWidth, bm.bmHeight);
			CBitmap* old_bmp_dest = dest_dc.SelectObject(clone->bmp_); 
			dest_dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &src_dc, 0, 0, SRCCOPY);
			src_dc. SelectObject(old_bmp_src); 
			dest_dc. SelectObject(old_bmp_dest); 			
			::ReleaseDC(NULL, src_dc);
			::ReleaseDC(NULL, dest_dc);
	  }
	  return clone;
  }

  void DevReset(const ui::Dimension& dimension);	

  virtual void DevLoad(const std::string& filename) {
		Dispose();
    CImage image;
    HRESULT r;		
		r = image.Load(Charset::utf8_to_win(filename).c_str());
    if (r!=S_OK) throw std::runtime_error(std::string("Error loading file:") + filename.c_str());    
    bmp_ = new CBitmap();
    bmp_->Attach(image.Detach());    
  }
	virtual void DevSave(const std::string& filename) {
		if (bmp_) {
			CImage image;
			image.Attach(*bmp_);
			HRESULT r;
			r = image.Save(Charset::utf8_to_win(filename).c_str());
			image.Detach();
			if (r != S_OK) throw std::runtime_error(std::string("Error saving file:") + filename.c_str());
		}
  }	
	void SetBitmap(CBitmap* bmp) {
		if (!shared_ && bmp_) {
      bmp_->DeleteObject();
    }
    shared_ = false;		
		bmp_ = bmp;
	}
	void FromHandle(HBITMAP handle) {
		Dispose();				
		bmp_ = new CBitmap();
		bmp_->FromHandle(handle);
	}
  virtual ui::Dimension dev_dim() const {
    ui::Dimension image_dim;
    if (bmp_) {    
      BITMAP bm;
      bmp_->GetBitmap(&bm);
      image_dim.set(bm.bmWidth, bm.bmHeight);
    }    
    return image_dim;
  }
  virtual void DevSetTransparent(bool on, ARGB color) {
    if (mask_.m_hObject) {
      mask_.DeleteObject();      
    }
    if (on) {      
      PrepareMask(bmp_, &mask_, ToCOLORREF(color));
    }
  }
	CBitmap* dev_source() { return bmp_; }
  CBitmap* dev_source() const { return bmp_; }
  CBitmap* dev_mask() { return mask_.m_hObject ? &mask_ : 0; }
  const CBitmap* dev_mask() const { return mask_.m_hObject ? &mask_ : 0; }

	virtual void DevCut(const ui::Rect& bounds) {
		CDC src_dc; 
        src_dc.CreateCompatibleDC(NULL); 
		CBitmap* old_bmp_src = src_dc.SelectObject(bmp_); 
		BITMAP bm; 
		bmp_->GetBitmap(&bm);
		CDC dest_dc; 
		dest_dc.CreateCompatibleDC(NULL);
		CBitmap* clone_bmp = new CBitmap();
		clone_bmp->CreateCompatibleBitmap(&src_dc, static_cast<int>(bounds.width()), static_cast<int>(bounds.height()));
		CBitmap* old_bmp_dest = dest_dc.SelectObject(clone_bmp); 
		dest_dc.BitBlt(0, 0, static_cast<int>(bounds.width()), static_cast<int>(bounds.height()), &src_dc, static_cast<int>(bounds.left()), static_cast<int>(bounds.top()), SRCCOPY);
		src_dc. SelectObject(old_bmp_src); 
		dest_dc. SelectObject(old_bmp_dest); 			
		::ReleaseDC(NULL, src_dc);
		::ReleaseDC(NULL, dest_dc);
		Dispose();
		bmp_ = clone_bmp;
	}

	virtual void DevSetPixel(ui::Point& pt, ARGB color) {
		CDC src_dc;
		src_dc.CreateCompatibleDC(NULL);
		CBitmap* old_bmp_src = src_dc.SelectObject(bmp_);
		CPoint cpt(static_cast<int>(pt.x()), static_cast<int>(pt.y()));
		src_dc.SetPixel(cpt, ToCOLORREF(color));
		src_dc.SelectObject(old_bmp_src);
		::ReleaseDC(NULL, src_dc);
	}

	virtual ARGB DevGetPixel(ui::Point& pt) const {
	  CDC src_dc;
	  src_dc.CreateCompatibleDC(NULL);
	  CBitmap* old_bmp_src = src_dc.SelectObject(bmp_);
	  CPoint cpt(static_cast<int>(pt.x()), static_cast<int>(pt.y()));
	  int color = src_dc.GetPixel(cpt);
	  src_dc.SelectObject(old_bmp_src);
	  ::ReleaseDC(NULL, src_dc);	
	  return ToARGB(color);
	}


	virtual void DevRotate(float radians) {
		CDC src_dc;
		src_dc.CreateCompatibleDC(NULL);
		CBitmap* old_bmp_src = src_dc.SelectObject(bmp_);
		BITMAP bm;
		bmp_->GetBitmap(&bm);
		CDC dest_dc;
		dest_dc.CreateCompatibleDC(NULL);
		

// rotate
		float cosine = (float)cos(radians);
		float sine = (float)sin(radians);

		// Compute dimensions of the resulting bitmap
		// First get the coordinates of the 3 corners other than origin
		int x1 = (int)(bm.bmHeight * sine);
		int y1 = (int)(bm.bmHeight * cosine);
		int x2 = (int)(bm.bmWidth * cosine + bm.bmHeight * sine);
		int y2 = (int)(bm.bmHeight * cosine - bm.bmWidth * sine);
		int x3 = (int)(bm.bmWidth * cosine);
		int y3 = (int)(-bm.bmWidth * sine);

		int minx = (std::min)(0, (std::min)(x1, (std::min)(x2, x3)));
		int miny = (std::min)(0, (std::min)(y1, (std::min)(y2, y3)));
		int maxx = (std::max)(0, (std::max)(x1, (std::max)(x2, x3)));
		int maxy = (std::max)(0, (std::max)(y1, (std::max)(y2, y3)));

		int w = maxx - minx;
		int h = maxy - miny;
				
		CBitmap* clone_bmp = new CBitmap();
		clone_bmp->CreateCompatibleBitmap(&src_dc, w, h);
		CBitmap* old_bmp_dest = dest_dc.SelectObject(clone_bmp);
		
		COLORREF clrBack = 0x00000;
		// Draw the background color before we change mapping mode
		HBRUSH hbrBack = CreateSolidBrush(clrBack);
		HBRUSH hbrOld = (HBRUSH)::SelectObject(dest_dc.m_hDC, hbrBack);
		dest_dc.PatBlt(0, 0, w, h, PATCOPY);
		::DeleteObject(::SelectObject(dest_dc.m_hDC, hbrOld));

		// We will use world transform to rotate the bitmap
		SetGraphicsMode(dest_dc.m_hDC, GM_ADVANCED);
		XFORM xform;
		xform.eM11 = cosine;
		xform.eM12 = -sine;
		xform.eM21 = sine;
		xform.eM22 = cosine;
		xform.eDx = (float)-minx;
		xform.eDy = (float)-miny;

		SetWorldTransform(dest_dc.m_hDC, &xform);

		// Now do the actual rotating - a pixel at a time
		dest_dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &src_dc, 0, 0, SRCCOPY);

// rotate end



		src_dc.SelectObject(old_bmp_src);
		dest_dc.SelectObject(old_bmp_dest);
		::ReleaseDC(NULL, src_dc);
		::ReleaseDC(NULL, dest_dc);
		Dispose();
		bmp_ = clone_bmp;

	}

	virtual void DevResize(const ui::Dimension& dimension) {
		CDC src_dc;
		src_dc.CreateCompatibleDC(NULL);
		CBitmap* old_bmp_src = src_dc.SelectObject(bmp_);
		BITMAP bm;
		bmp_->GetBitmap(&bm);
		CDC dest_dc;
		dest_dc.CreateCompatibleDC(NULL);
		CBitmap* clone_bmp = new CBitmap();
		clone_bmp->CreateCompatibleBitmap(&src_dc, static_cast<int>(dimension.width()), static_cast<int>(dimension.height()));
		CBitmap* old_bmp_dest = dest_dc.SelectObject(clone_bmp);
        // scale
		int w = static_cast<int>(dev_dim().width());
		int h = static_cast<int>(dev_dim().height());
		int w2 = static_cast<int>(dimension.width());
		int h2 = static_cast<int>(dimension.height());

		double x_ratio = (w - 1) / (double)w2;
		double y_ratio = (h - 1) / (double)h2;

		for (int i = 1; i < h2; ++i) {
			for (int j = 1; j < w2; ++j) {
				int x = (int)(x_ratio * j);
				int y = (int)(y_ratio * i);
				double x_diff = (x_ratio * j) - x;
				double y_diff = (y_ratio * i) - y;
				COLORREF p1 = src_dc.GetPixel(x, y);
				COLORREF p2 = src_dc.GetPixel(x + 1, y);
				COLORREF p3 = src_dc.GetPixel(x, y + 1);
				COLORREF p4 = src_dc.GetPixel(x + 1, y + 1);

				byte r1 = GetRValue(p1);
				byte g1 = GetGValue(p1);
				byte b1 = GetBValue(p1);

				byte r2 = GetRValue(p2);
				byte g2 = GetGValue(p2);
				byte b2 = GetBValue(p2);

				byte r3 = GetRValue(p3);
				byte g3 = GetGValue(p3);
				byte b3 = GetBValue(p3);

				byte r4 = GetRValue(p4);
				byte g4 = GetGValue(p4);
				byte b4 = GetBValue(p4);

				double diff1 = (1 - x_diff)*(1 - y_diff);
				double diff2 = (x_diff)*(1 - y_diff);
				double diff3 = (y_diff)*(1 - x_diff);
				double diff4 = (x_diff*y_diff);

				int outr = (int)(r1*diff1 + r2*diff2 + r3*diff3 + r4*diff4);
				int outg = (int)(g1*diff1 + g2*diff2 + g3*diff3 + g4*diff4);
				int outb = (int)(b1*diff1 + b2*diff2 + b3*diff3 + b4*diff4);
				//local outa = (int)(a1*diff1 + a2*diff2 + a3*diff3 + a4*diff4)
				CPoint pt(j, i);
				dest_dc.SetPixel(pt, RGB(outr, outg, outb));
			}
		}

		// end of scale
		src_dc.SelectObject(old_bmp_src);
		dest_dc.SelectObject(old_bmp_dest);
		::ReleaseDC(NULL, src_dc);
		::ReleaseDC(NULL, dest_dc);
		Dispose();
		bmp_ = clone_bmp;
	}

 private:
	void Dispose() {
		if (!shared_ && bmp_) {
      bmp_->DeleteObject();
    }
    shared_ = false;
	}
  void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);    
	virtual ui::Graphics* dev_graphics();

  CBitmap *bmp_, mask_;
	CDC dest_dc_;
  bool shared_;
	std::auto_ptr<ui::Graphics> paint_graphics_;
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

/*
inline void PremultiplyBitmapAlpha(HDC hDC, HBITMAP hBmp) {
  BITMAP bm = { 0 };
  GetObject(hBmp, sizeof(bm), &bm);
  BITMAPINFO* bmi = (BITMAPINFO*) _alloca(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
  ::ZeroMemory(bmi, sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
  bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  BOOL bRes = ::GetDIBits(hDC, hBmp, 0, bm.bmHeight, NULL, bmi, DIB_RGB_COLORS);
  if( !bRes || bmi->bmiHeader.biBitCount != 32 ) return;
  LPBYTE pBitData = (LPBYTE) ::LocalAlloc(LPTR, bm.bmWidth * bm.bmHeight * sizeof(DWORD));
  if( pBitData == NULL ) return;
  LPBYTE pData = pBitData;
  ::GetDIBits(hDC, hBmp, 0, bm.bmHeight, pData, bmi, DIB_RGB_COLORS);
  for( int y = 0; y < bm.bmHeight; y++ ) {
    for( int x = 0; x < bm.bmWidth; x++ ) {
      pData[0] = (BYTE)((DWORD)pData[0] * pData[3] / 255);
      pData[1] = (BYTE)((DWORD)pData[1] * pData[3] / 255);
      pData[2] = (BYTE)((DWORD)pData[2] * pData[3] / 255);
      pData += 4;
    }
  }
  ::SetDIBits(hDC, hBmp, 0, bm.bmHeight, pBitData, bmi, DIB_RGB_COLORS);
  ::LocalFree(pBitData);
}*/

class FontImp : public ui::FontImp {
 public:
  FontImp() {
    LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));
    lfLogFont.lfHeight = 12;    
#ifdef UNICODE
		wcscpy(lfLogFont.lfFaceName, _T("Arial"));
#else
    strcpy(lfLogFont.lfFaceName, "Arial");
#endif
    cfont_.CreateFontIndirect(&lfLogFont);  
  }
  
  virtual ~FontImp() {
    cfont_.DeleteObject();
  }

  virtual FontImp* DevClone() const { 
    FontImp* clone = new FontImp();
    clone->dev_set_info(dev_info());
    return clone;
  }

  virtual void dev_set_info(const FontInfo& info) {
    LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));
    lfLogFont.lfHeight = info.height;
    lfLogFont.lfHeight = info.height;
    if (info.bold) {
      lfLogFont.lfWeight = FW_BOLD;
    }
#ifdef UNICODE
		wcscpy(lfLogFont.lfFaceName, Charset::utf8_to_win(info.name).c_str());
#else
    strcpy(lfLogFont.lfFaceName,Charset:: utf8_to_win(info.name).c_str());
#endif    
    cfont_.DeleteObject();
    cfont_.CreateFontIndirect(&lfLogFont);
  }

  virtual FontInfo dev_info() const {
    LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));
    cfont_.GetLogFont(&lfLogFont);
    FontInfo info;
    info.height = lfLogFont.lfHeight;
    info.name = Charset::win_to_utf8(lfLogFont.lfFaceName);
    info.bold = (lfLogFont.lfWeight == FW_BOLD);
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
        updatebrush_(false),
				pen_width_(1) {       
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
        pen_width_(1) {
    Init();
  }

  virtual ~GraphicsImp() {    
    if (cr_) {
      DevDispose();
    }
    if (hScreenDC) {      
      ReleaseDC(0, hScreenDC);      
    }
  }

  virtual void DevDispose() {
    cr_->SelectObject(old_pen);
    pen.DeleteObject();
    brush.DeleteObject();
    cr_->SetGraphicsMode(GM_ADVANCED);
    cr_->SetWorldTransform(&rXform);
    cr_->SelectObject(old_font);
    cr_->SetBkMode(OPAQUE);
    cr_->SelectClipRgn(&old_rgn_);
		if (old_bpm_) {
			cr_->SelectObject(old_bpm_);
		}
    old_rgn_.DeleteObject();
    clp_rgn_.DeleteObject();
    cr_ = 0;
  }

  void DevDrawString(const std::string& str, double x, double y) {
    check_pen_update();		
		cr_->TextOut(static_cast<int>(x), static_cast<int>(y), Charset::utf8_to_win(str).c_str());
  }

  void DevCopyArea(const ui::Rect& rect, const ui::Point& delta) {    
    cr_->BitBlt(
			static_cast<int>(rect.left() + delta.x()),
			static_cast<int>(rect.top() + delta.y()),
			static_cast<int>(rect.width()),
			static_cast<int>(rect.height()),
      cr_,
			static_cast<int>(delta.x()),
			static_cast<int>(delta.y()),
      SRCCOPY);
  }

  void DevTranslate(double x, double y) {
    rXform.eDx += static_cast<int>(x);
    rXform.eDy += static_cast<int>(y);
    cr_->SetGraphicsMode(GM_ADVANCED);
    cr_->SetWorldTransform(&rXform);
  }

  void DevSetColor(ARGB color) {
    if (argb_color_ != color) {
      argb_color_ = color;
      rgb_color_ = ToCOLORREF(color);
      updatepen_ = updatebrush_ = true;      
    }
  }

  ARGB dev_color() const { return argb_color_; }

  void DevDrawArc(const ui::Rect& rect, const Point& start, const Point& end) {
    check_pen_update();
    CRect rc = TypeConverter::rect(rect);
    cr_->Arc(&rc, TypeConverter::point(start), TypeConverter::point(end));
  }

  void DevDrawLine(const ui::Point& p1, const ui::Point& p2) {
    check_pen_update();
		cr_->MoveTo(TypeConverter::point(p1));
    cr_->LineTo(TypeConverter::point(p2));
  }

  void DevDrawRect(const ui::Rect& rect) {
    check_pen_update();
    cr_->SelectStockObject(NULL_BRUSH);
		cr_->Rectangle(TypeConverter::rect(rect));
		cr_->SelectObject(&brush);
  }
  
  void DevDrawRoundRect(const ui::Rect& rect, const ui::Dimension& arc_dim) {
    check_pen_update();
    check_brush_update();
    cr_->SelectStockObject(NULL_BRUSH);
		cr_->RoundRect(TypeConverter::rect(rect), TypeConverter::point(arc_dim));      
    cr_->SelectObject(&brush);
  }
  
  void DevDrawOval(const ui::Rect& rect) {
    check_pen_update();
    check_brush_update();
    cr_->SelectStockObject(NULL_BRUSH);
    cr_->Ellipse(TypeConverter::rect(rect));
    cr_->SelectObject(&brush);
  }

  void DevFillRoundRect(const ui::Rect& rect, const ui::Dimension& arc_dim) {
    check_pen_update();
    check_brush_update();
    cr_->RoundRect(TypeConverter::rect(rect), TypeConverter::point(arc_dim));
  }

  void DevFillRect(const ui::Rect& rect) {
    cr_->FillSolidRect(TypeConverter::rect(rect), rgb_color_);
  }

  void DevFillOval(const ui::Rect& rect) {
    check_pen_update();
    check_brush_update();
    cr_->Ellipse(TypeConverter::rect(rect));
  }

  void DevFillRegion(const ui::Region& rgn);

  void DevDrawPolygon(const ui::Points& points) {
    check_pen_update();
    cr_->SelectStockObject(NULL_BRUSH);
    std::vector<CPoint> lpPoints;
    ui::Points::const_iterator it = points.begin();
    const int size = points.size();
    for (; it!=points.end(); ++it) {
      lpPoints.push_back(TypeConverter::point(*it));
    }
    cr_->Polygon(&lpPoints[0], size);
    cr_->SelectObject(&brush);
  }

  void DevFillPolygon(const ui::Points& points) {
    check_pen_update();
    check_brush_update();
    std::vector<CPoint> lpPoints;
    ui::Points::const_iterator it = points.begin();
    const int size = points.size();
    for (; it!=points.end(); ++it) {
			lpPoints.push_back(TypeConverter::point(*it));
    }
    cr_->Polygon(&lpPoints[0], size);
  }

  void DevDrawPolyline(const ui::Points& points) {
    check_pen_update();
    std::vector<CPoint> lpPoints;
    ui::Points::const_iterator it = points.begin();
    const int size = points.size();
    for (; it!=points.end(); ++it) {
			lpPoints.push_back(TypeConverter::point(*it));
    }
    cr_->Polyline(&lpPoints[0], size);
  }

  void DevSetFont(const ui::Font& font) {
    font_ = font;
    mfc::FontImp* imp = dynamic_cast<mfc::FontImp*>(font_.imp());
    assert(imp);
    cr_->SelectObject(imp->cfont());
    cr_->SetTextColor(rgb_color_);    
  }

  const Font& dev_font() const { return font_; }

  virtual Dimension dev_text_size(const std::string& text) const {
    ui::Dimension size;
    SIZE extents = {0};
    GetTextExtentPoint32(cr_->m_hDC, Charset::utf8_to_win(text).c_str(), text.length(),
      &extents);
    size.set(extents.cx, extents.cy);
    return size;
  }

	virtual void DevSetPenWidth(double width) {
		pen_width_ = static_cast<int>(width);
		updatepen_ = true;
	}
	      
	void DevDrawImage(ui::Image* image, const ui::Rect& destination_position,
		             const ui::Point& src) {
		if (image) {
			assert(image->imp());
			CDC memDC;
			CBitmap* oldbmp;
			memDC.CreateCompatibleDC(cr_);
			ImageImp* imp = dynamic_cast<ImageImp*>(image->imp());
			assert(imp);
			oldbmp = memDC.SelectObject(imp->dev_source());
			if (!imp->dev_mask()) {
				cr_->BitBlt(static_cast<int>(destination_position.left()), 
					          static_cast<int>(destination_position.top()),
						        static_cast<int>(destination_position.width()),
										static_cast<int>(destination_position.height()),
					          &memDC, 
					          static_cast<int>(src.x()),
					          static_cast<int>(src.y()),
					          SRCCOPY);
			} else {
				TransparentBlt(cr_,
					             static_cast<int>(destination_position.left()),
						           static_cast<int>(destination_position.top()),
							         static_cast<int>(destination_position.width()),
								       static_cast<int>(destination_position.height()),
					             &memDC,
					             imp->dev_mask(),
									     static_cast<int>(src.x()),
										   static_cast<int>(src.y()));
			}
			memDC.SelectObject(oldbmp);
			memDC.DeleteDC();
	  }
  }

	void DevDrawImage(ui::Image* image,
		                const ui::Rect& destination_position,
		                const ui::Point& src,
		                const ui::Point& zoom_factor) {
		if (image) {
			assert(image->imp());
			CDC memDC;
			CBitmap* oldbmp;
			memDC.CreateCompatibleDC(cr_);
			ImageImp* imp = dynamic_cast<ImageImp*>(image->imp());
			assert(imp);
			oldbmp = memDC.SelectObject(imp->dev_source());
			if (!imp->dev_mask()) {
				cr_->StretchBlt(static_cast<int>(destination_position.left()),
					              static_cast<int>(destination_position.top()),
					              static_cast<int>(destination_position.width() * zoom_factor.x()), 
					              static_cast<int>(destination_position.height() * zoom_factor.y()),
					              &memDC, 
					              static_cast<int>(src.x()),
					              static_cast<int>(src.y()),
					              static_cast<int>(destination_position.width()),
					              static_cast<int>(destination_position.height()),
					              SRCCOPY);
			} else {
				/*TransparentBlt(cr_,
					             destination_position.left(), 
					             destination_position.top(), 
					             destination_position.width(), 
					             destination_position.height(),
					             &memDC,
					             imp->dev_mask(),
					             src.x(),
					             src.y());*/
			}
			memDC.SelectObject(oldbmp);
			memDC.DeleteDC();
	  }
  }

  void DevSetClip(const ui::Rect& rect) {
    CRgn rgn;
    rgn.CreateRectRgn(static_cast<int>(rect.left()), static_cast<int>(rect.top()), static_cast<int>(rect.right()), static_cast<int>(rect.bottom()));
    cr_->SelectClipRgn(&rgn);
    rgn.DeleteObject();
  }

  void DevSetClip(ui::Region* rgn);

  CRgn& dev_clip() {
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
      pen.CreatePen(PS_SOLID | PS_COSMETIC, pen_width_, rgb_color_);
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

  virtual CDC* dev_dc() { return cr_; }

	virtual void AttachBitmap(CBitmap* bmp) {
		old_bpm_ = cr_->SelectObject(bmp);		
	}

	virtual void DevAttachImage(ui::Image* image) {
		ui::mfc::ImageImp* imp = dynamic_cast<ImageImp*>(image->imp());
		if (imp) {
			AttachBitmap(imp->dev_source());
		}
	}

  virtual void DevSaveOrigin() {
     saveXform.push(rXform);
   }
   virtual void DevRestoreOrigin() {
     rXform = saveXform.top();
     saveXform.pop();
     cr_->SetGraphicsMode(GM_ADVANCED);
     cr_->SetWorldTransform(&rXform);
   }
    
  private:
   void Init() {
     assert(cr_);
		 old_bpm_ = 0;
     pen.CreatePen(PS_SOLID, 1, rgb_color_);
     cr_->SetTextColor(rgb_color_);
     old_pen = cr_->SelectObject(&pen);
     old_brush = cr_->SelectObject(&brush);
     cr_->GetWorldTransform(&rXform);    
     cr_->SetBkMode(TRANSPARENT);
    
     mfc::FontImp* imp = dynamic_cast<mfc::FontImp*>(font_.imp());
     assert(imp);
     old_font = (CFont*) cr_->SelectObject(&imp->cfont());
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
   ui::Font font_;
   CFont* old_font;
   ARGB argb_color_;
   RGB rgb_color_;
   bool updatepen_, updatebrush_;
   XFORM rXform;
   std::stack<XFORM> saveXform;
   CRgn old_rgn_, clp_rgn_;
	 CBitmap* old_bpm_;
   int pen_width_;
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
  WindowImp* popup_window_;
};

struct WindowID { 
  static int id_counter;
  static int auto_id() { return id_counter++; }
};

struct DummyWindow {
  static CWnd* dummy() {
     if (!dummy_wnd_.m_hWnd) {   
       dummy_wnd_.Create(0, _T("psycleuidummywnd"), 0, CRect(0, 0, 0, 0), ::AfxGetMainWnd(), 0);       
     }
    return &dummy_wnd_;
  }
 private:
  static CWnd dummy_wnd_;
};

template <class T, class I>
class WindowTemplateImp : public T, public I {
 public:  
  WindowTemplateImp() : I(), color_(0xFF000000), mouse_enter_(true), is_double_buffered_(false) {}
  WindowTemplateImp(ui::Window* w) : I(w), color_(0xFF000000), mouse_enter_(true), is_double_buffered_(false) {}
    
  virtual void dev_set_pos(const ui::Rect& pos);
  virtual ui::Rect dev_pos() const;
  virtual ui::Rect dev_abs_pos() const;	
  virtual ui::Rect dev_desktop_pos() const;
  virtual ui::Dimension dev_dim() const {
    CRect rc;            
    GetWindowRect(&rc);
    return ui::Dimension(rc.Width() - padding_.width() - border_space_.width(), rc.Height() - padding_.height() - border_space_.height());
  }
	virtual void dev_set_margin(const BoxSpace& margin) { margin_ = margin; }
	virtual const BoxSpace& dev_margin() const { return margin_; }
	virtual void dev_set_padding(const BoxSpace& padding) {
		padding_ = padding;				
		dev_set_pos(dev_pos());		
	}
	virtual const BoxSpace& dev_padding() const { return padding_; }
	virtual void dev_set_border_space(const BoxSpace& border_space) { 
		border_space_ = border_space;
	}
	virtual const BoxSpace& dev_border_space() const { return border_space_; }
  virtual void DevScrollTo(int offset_x, int offset_y) {
    //::SetScrollPos(m_hWnd, SB_HORZ, offset_x, TRUE);
    //::SetScrollPos(m_hWnd, SB_HORZ, offset_x, TRUE);
    ScrollWindow(-offset_x, -offset_y);
  }
  virtual void DevEnable() { EnableWindow(true); }
  virtual void DevDisable() { EnableWindow(false); }
  virtual void DevShow() { 		
		ShowWindow(SW_SHOWNORMAL); 
	}
  virtual void DevHide() { ShowWindow(SW_HIDE); }
  virtual void DevInvalidate() {    		
		if (window() && window()->root()) {
      CWnd* root = dynamic_cast<CWnd*>(window()->root()->imp());			
			::RedrawWindow(root->m_hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);        
		}    
  }
  virtual void DevInvalidate(const ui::Region& rgn) {
    if (m_hWnd) {
      mfc::RegionImp* imp = dynamic_cast<mfc::RegionImp*>(rgn.imp());
      assert(imp);	
			if (window() && window()->root()) {
        CWnd* root = dynamic_cast<CWnd*>(window()->root()->imp());											
				::RedrawWindow(m_hWnd, NULL, imp->crgn(), RDW_INVALIDATE | RDW_ERASE);
			}
    }
  }
  virtual void DevSetCapture() { SetCapture(); }  
  virtual void DevReleaseCapture() { ReleaseCapture(); }  
  virtual void DevShowCursor() { while (::ShowCursor(TRUE) < 0); }  
  virtual void DevHideCursor() { while (::ShowCursor(FALSE) >= 0); }
  virtual void DevSetCursorPos(double x, double y) {
    CPoint point(static_cast<int>(x), static_cast<int>(y));
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

  virtual void DevViewDoubleBuffered() { is_double_buffered_ = true; }
  virtual void DevViewSingleBuffered() { is_double_buffered_ = false; }
  virtual bool dev_is_double_buffered() const { return is_double_buffered_; }

protected:  
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  DECLARE_MESSAGE_MAP()
  int OnCreate(LPCREATESTRUCT lpCreateStruct);
  void OnDestroy();	
  void OnPaint();
  void OnSize(UINT nType, int cx, int cy); 


  afx_msg void OnKillFocus(CWnd* pNewWnd);		
  BOOL OnEraseBkgnd(CDC* pDC);
  
  virtual bool OnDevUpdateArea(ui::Area& area);   
 protected:

  virtual void OnHScroll(UINT a, UINT b, CScrollBar* pScrollBar) {
    ReflectLastMsg(pScrollBar->GetSafeHwnd());
  }
  virtual void OnVScroll(UINT a, UINT b, CScrollBar* pScrollBar) {
    ReflectLastMsg(pScrollBar->GetSafeHwnd());
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

  void MapPointToRoot(CPoint& pt) const {
    if (window() && window()->root()) {
      CWnd* root = dynamic_cast<CWnd*>(window()->root()->imp());
      MapWindowPoints(root, &pt, 1);
    }
  }

  void MapPointToDesktop(CPoint& pt) const {
    ::MapWindowPoints(m_hWnd, ::GetDesktopWindow(), &pt, 1);
  }

	ui::Rect MapPosToBoxModel(const CRect& rc) const;

  virtual BOOL prevent_propagate_event(ui::Event& ev, MSG* pMsg);

  template <class T, class T1>
  bool WorkEvent(T& ev, void (T1::*ptmember)(T&), Window* window, MSG* msg) {
    if (window) {
      try {
        (window->*ptmember)(ev);        
      } catch (std::exception& e) {
        Alert(e.what());
      }
      if (!::IsWindow(msg->hwnd)) {
        return true;
      }
      return prevent_propagate_event(ev, msg) != 0;          
    } 
    return false;
  }
  
 private:
  CBitmap bmpDC;
  ARGB color_;
  BOOL m_bTracking;
  bool mouse_enter_;  
  bool is_double_buffered_;
	BoxSpace margin_;
	BoxSpace padding_;
	BoxSpace border_space_;
};

class AlertImp : public ui::AlertImp {
 public:
	AlertImp() {}
	virtual ~AlertImp() {}
	virtual void DevAlert(const std::string& text) {
		AfxMessageBox(Charset::utf8_to_win(text).c_str());
	}
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
		                 CRect(0, 0, 100, 20), 
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
  // BOOL OnEraseBkgnd(CDC* pDC);
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
                     CRect(0, 0, static_cast<int>(size.width()), static_cast<int>(size.height())),
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
   virtual BOOL OnChildNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
  
   DECLARE_MESSAGE_MAP()
  void OnPaint() {
	  this->
	  CScrollBar::OnPaint(); 
  }  
};

class ButtonImp : public WindowTemplateImp<CButton, ui::ButtonImp> {
 public:  
  ButtonImp() : WindowTemplateImp<CButton, ui::ButtonImp>() {}

  static ButtonImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    ButtonImp* imp = new ButtonImp();
    if (!imp->Create(_T("Button"),
                     WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | DT_CENTER,
                     CRect(0, 0, 200, 19), 
                     parent, 
                     nID)) {
      TRACE0("Failed to create window\n");
      return 0;
    }
    imp->set_window(w);  
    return imp;
  }

  BOOL OnEraseBkgnd(CDC* pDC) { return CButton::OnEraseBkgnd(pDC); }

  virtual void dev_set_text(const std::string& text) { SetWindowText(Charset::utf8_to_win(text).c_str()); }
  virtual std::string dev_text() const {
    CString s;
    GetWindowText(s);		
		return Charset::win_to_utf8(s.GetString());
  }

	virtual bool dev_checked() const { return GetCheck() != 0; }
	virtual void DevCheck() { SetCheck(true); }
	virtual void DevUnCheck() { SetCheck(false); }

protected:
  DECLARE_MESSAGE_MAP()
  void OnPaint() { CButton::OnPaint(); }
  void OnClick();
};

class CheckBoxImp : public WindowTemplateImp<CButton, ui::CheckBoxImp> {
 public:  
  CheckBoxImp() : WindowTemplateImp<CButton, ui::CheckBoxImp>() {}

  static CheckBoxImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    CheckBoxImp* imp = new CheckBoxImp();
    if (!imp->Create(_T("CheckBox"),
                     WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | DT_CENTER  | BS_CHECKBOX | BS_AUTOCHECKBOX,
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

  virtual void dev_set_text(const std::string& text) { SetWindowText(Charset::utf8_to_win(text).c_str()); }
  virtual std::string dev_text() const {
    CString s;    
    GetWindowText(s);
		return Charset::win_to_utf8(s.GetString());    
  }
  virtual void dev_set_background_color(ARGB color) {     
    // this->S
  }
  virtual bool dev_checked() const { return GetCheck() != 0; }
  virtual void DevCheck()  { SetCheck(true); }
  virtual void DevUnCheck() { SetCheck(false); }

protected:
  DECLARE_MESSAGE_MAP()
  void OnPaint() { CButton::OnPaint(); }
  void OnClick();
};

class RadioButtonImp : public WindowTemplateImp<CButton, ui::RadioButtonImp> {
public:
	RadioButtonImp() : WindowTemplateImp<CButton, ui::RadioButtonImp>() {}

	static RadioButtonImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
		RadioButtonImp* imp = new RadioButtonImp();
		if (!imp->Create(_T("RadioButton"),
			WS_CHILD | WS_TABSTOP | DT_CENTER | BS_AUTORADIOBUTTON,
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

	virtual void dev_set_text(const std::string& text) { SetWindowText(Charset::utf8_to_win(text).c_str()); }
	virtual std::string dev_text() const {
		CString s;
		GetWindowText(s);
		return Charset::win_to_utf8(s.GetString());
	}
	virtual void dev_set_background_color(ARGB color) {
		// this->S
	}
	virtual bool dev_checked() const { return GetCheck() != 0; }
	virtual void DevCheck() { SetCheck(true); }
	virtual void DevUnCheck() { SetCheck(false); }

protected:
	DECLARE_MESSAGE_MAP()
	void OnPaint() { CButton::OnPaint(); }
	void OnClick();
};

class GroupBoxImp : public WindowTemplateImp<CButton, ui::GroupBoxImp> {
public:
	ui::GroupBoxImp() : WindowTemplateImp<CButton, ui::GroupBoxImp>() {}

	static ui::GroupBoxImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
		GroupBoxImp* imp = new GroupBoxImp();
		if (!imp->Create(_T("GroupBox"),
			WS_CHILD | WS_TABSTOP | DT_CENTER | BS_GROUPBOX,
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

	virtual void dev_set_text(const std::string& text) { SetWindowText(Charset::utf8_to_win(text).c_str()); }
	virtual std::string dev_text() const {
		CString s;
		GetWindowText(s);
		return Charset::win_to_utf8(s.GetString());
	}
	virtual void dev_set_background_color(ARGB color) {
		// this->S
	}	

	virtual bool dev_checked() const { return GetCheck() != 0; }
	virtual void DevCheck() { SetCheck(true); }
	virtual void DevUnCheck() { SetCheck(false); }

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
    int style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX |
                WS_MAXIMIZEBOX | WS_SIZEBOX;
		
    if (!imp->Create(NULL, _T("PsycleWindow"), style, CRect(0, 0, 200, 200),  
		             parent)) {					 
      TRACE0("Failed to create frame window\n");
      return 0;
    }
    imp->set_window(w);        
    return imp;
  }

  static FrameImp* MakeMain(ui::Window* w) {
	FrameImp* imp = new FrameImp();    	    
    if (!imp->CreateEx(NULL, AfxRegisterWndClass(0), _T("PsycleWindow"), WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CRect(0, 0, 200, 200), NULL, 0,  
		             NULL)) {					 
      TRACE0("Failed to create frame window\n");
      return 0;
    }
    imp->set_window(w);        
    return imp;
  }

  BOOL PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Ändern Sie hier die Fensterklasse oder die Darstellung, indem Sie
	//  CREATESTRUCT cs modifizieren.

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}
   
  virtual void dev_set_title(const std::string& title) {
    SetWindowText(Charset::utf8_to_win(title).c_str());
  }

  virtual std::string dev_title() const {
    CString s;
    GetWindowText(s);
    return Charset::win_to_utf8(s.GetString());
  }

  virtual void dev_set_viewport(ui::Window::Ptr viewport) {
    CWnd* wnd = dynamic_cast<CWnd*>(viewport->imp());
    if (wnd) {      
      wnd->SetParent(this);      
      viewport_ = viewport;
    }
  }

  virtual void DevShowDecoration();
  virtual void DevHideDecoration();
  virtual void DevPreventResize();
  virtual void DevAllowResize();

  bool DevIsValid() const {
	  return m_hWnd && ::IsWindow(m_hWnd);
  }
  
  void OnSize(UINT nType, int cx, int cy) {
    if (viewport_) {
      CRect rect;
      this->GetClientRect(rect);
      CWnd* wnd = dynamic_cast<CWnd*>(viewport_->imp());
      if (wnd && ::IsWindow(wnd->m_hWnd)) {
        wnd->MoveWindow(&rect);
      }
    }
  }
     
 protected:
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  DECLARE_MESSAGE_MAP()
	afx_msg void OnDynamicMenuItems(UINT nID);  
  afx_msg void OnContextMenu( CWnd* pWnd, CPoint pos );		
  void OnPaint() { CFrameWnd::OnPaint(); }
  BOOL OnEraseBkgnd(CDC* pDC) { return 1; }
  virtual void OnClose() {		
	if (window()) {
	  ((Frame*)window())->OnClose();
	}
  }
  ui::Window::Ptr viewport_;
  afx_msg void OnSetFocus(CWnd* pNewWnd) {
    if (window()) {
      ui::Event ev;
      window()->OnFocus(ev);
    }
  }
  afx_msg void OnKillFocus(CWnd* pNewWnd) {
    if (window()) {
      window()->OnKillFocus();
    }
  }
};

class PopupFrameImp : public FrameImp {
 public:
   PopupFrameImp() : FrameImp(), mouse_hook_(0) {}

  static PopupFrameImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    PopupFrameImp* imp = new PopupFrameImp();    
    if (!imp->Create(NULL, _T("PsyclePopupFrame"), WS_POPUP,
                     CRect(0, 0, 200, 200), 
                     parent, 0, WS_EX_TOPMOST | WS_EX_NOACTIVATE)) {
      TRACE0("Failed to create frame window\n");
      return 0;
    }
    imp->set_window(w);        
    return imp;
  }
 
  virtual void DevShow();
  virtual void DevHide();
  HHOOK mouse_hook_;  
  static ui::FrameImp* popup_frame_;  
};

class MenuImp;

class MenuContainerImp : public ui::MenuContainerImp { 
 public:  
  MenuContainerImp();
 
  virtual void DevUpdate(const Node::Ptr& node, boost::shared_ptr<Node> prev_node);
  virtual void DevErase(boost::shared_ptr<Node> node);
  virtual void DevInvalidate();
  virtual void DevTrack(const ui::Point& pos) {}

  void RegisterMenuEvent(int id, MenuImp* menu_imp);
  MenuImp* FindMenuItemById(int id);
  static MenuContainerImp* MenuContainerImpById(int id);
  void WorkMenuItemEvent(int id);
  void set_menu_window(CWnd* menu_window, const Node::Ptr& root_node);
  void set_cmenu(CMenu* cmenu) { cmenu_ = cmenu; }
  
 private:
   void UpdateNodes(Node::Ptr parent_node, CMenu* parent, int pos_start = 0);
   std::map<int, MenuImp*> menu_item_id_map_;
   static std::map<int, MenuContainerImp*> menu_bar_id_map_;   
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
    parent()->ModifyMenu(pos_, MF_BYPOSITION, 0, Charset::utf8_to_win(text).c_str());    
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
#ifdef UNICODE
  std::wstring text_;  
#else
	std::string text_;
#endif
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
  virtual void DevUpdate(const Node::Ptr& node, boost::shared_ptr<Node> prev_node);
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
	virtual void DevExpandAll();
  
  virtual void dev_set_images(const ui::Images::Ptr& images) {
    ui::Images::iterator it = images->begin();
    for (; it != images->end(); ++it) {
			Image::Ptr image = *it;
			assert(image->imp());			
			ImageImp* imp = dynamic_cast<ImageImp*>(image->imp());
			assert(imp);
      m_imageList.Add(imp->dev_source(),  (COLORREF)0xFFFFFF);    
    }
    SetImageList(&m_imageList, TVSIL_NORMAL);
  }
  CImageList m_imageList;

  virtual ui::Node::Ptr dev_node_at(const ui::Point& pos) const;

 protected:
  virtual BOOL prevent_propagate_event(ui::Event& ev, MSG* pMsg);
  DECLARE_MESSAGE_MAP()
  void OnPaint() { CTreeCtrl::OnPaint(); }
  BOOL OnChange(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnRightClick(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnDblClick(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnBeginLabelEdit(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnEndLabelEdit(NMHDR * pNotifyStruct, LRESULT * result);
  ui::Node* find_selected_node();
  
 private:   
   void UpdateNode(boost::shared_ptr<Node> node, boost::shared_ptr<Node> prev_node);
   bool is_editing_;
};

class ListViewImp;

class ListNodeImp : public NodeImp {  
 public:
  ListNodeImp() { memset(&lvi, 0, sizeof(lvi)); }
  ~ListNodeImp() {}

  void set_pos(int pos) { lvi.iItem = pos; }
  virtual int pos() const { return lvi.iItem; }
      
  // LVITEM DevInsert(ui::mfc::ListViewImp* list, const ui::Node& node, ListNodeImp* prev_imp);
  void DevInsertFirst(ui::mfc::ListViewImp* list, const ui::Node& node, ListNodeImp* node_imp, ListNodeImp* prev_imp, int pos);
  void DevSetSub(ui::mfc::ListViewImp* list, const ui::Node& node, ListNodeImp* node_imp, ListNodeImp* prev_imp, int level);
  void set_text(ui::mfc::ListViewImp* list, const std::string& text);

  LVITEM lvi;
#ifdef UNICODE
  std::wstring text_;  
#else
	std::string text_;
#endif
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
  virtual void DevUpdate(const Node::Ptr& node, Node::Ptr prev_node);
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
			Image::Ptr image = *it;
			assert(image->imp());			
			ImageImp* imp = dynamic_cast<ImageImp*>(image->imp());
			assert(imp);
      m_imageList.Add(imp->dev_source(),  (COLORREF)0xFFFFFF);    
    }
    SetImageList(&m_imageList, TVSIL_NORMAL);
  }
  virtual void DevViewList() { 
	  ::SendMessage(this->m_hWnd, LVM_SETVIEW, (WPARAM)LV_VIEW_LIST, 0);	   
  }
  virtual void DevViewReport() { 
	  this->ModifyStyle(0, LVS_REPORT, 0);	  
  }
  virtual void DevViewIcon() { 
	  ::SendMessage(this->m_hWnd, LVM_SETVIEW, (WPARAM)LV_VIEW_ICON, 0);	   
  }
  virtual void DevViewSmallIcon() { 
	  ::SendMessage(this->m_hWnd, LVM_SETVIEW, (WPARAM)LV_VIEW_SMALLICON, 0);  
  }
  virtual void DevEnableRowSelect() {
    SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT);
  }
  virtual void DevDisableRowSelect() {
    SetExtendedStyle(GetExtendedStyle() & ~ LVS_EX_FULLROWSELECT);
  }
  virtual void DevAddColumn(const std::string& text, int width) {
    InsertColumn(column_pos_++, Charset::utf8_to_win(text).c_str(), LVCFMT_LEFT, width);
  }
  virtual std::vector<ui::Node::Ptr> dev_selected_nodes();

  CImageList m_imageList;

  virtual int dev_top_index() const { return GetTopIndex(); }
  virtual void DevEnsureVisible(int index) { EnsureVisible(index, true); }

 protected:  
  virtual BOOL prevent_propagate_event(ui::Event& ev, MSG* pMsg);

  DECLARE_MESSAGE_MAP()
  void OnPaint() { CListCtrl::OnPaint(); }
  BOOL OnChange(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnRightClick(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnBeginLabelEdit(NMHDR * pNotifyStruct, LRESULT * result);
  BOOL OnEndLabelEdit(NMHDR * pNotifyStruct, LRESULT * result);
  void OnCustomDrawList(NMHDR *pNMHDR, LRESULT *pResult);
  ui::Node* find_selected_node();

 private:
   ListNodeImp* UpdateNode(boost::shared_ptr<Node> node, boost::shared_ptr<Node> prev_node, int pos);
   bool is_editing_;
   int column_pos_;
};

class ComboBoxImp : public WindowTemplateImp<CComboBox, ui::ComboBoxImp> {
 public:  
  ComboBoxImp() : WindowTemplateImp<CComboBox, ui::ComboBoxImp>() {}

  static ComboBoxImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    ComboBoxImp* imp = new ComboBoxImp();
    if (!imp->Create(WS_CHILD|WS_VSCROLL|CBS_DROPDOWNLIST|WS_VISIBLE,
                     CRect(0, 0, 200, 20), 
                     parent, 
                     nID)) {
      TRACE0("Failed to create window\n");
      return 0;
    }
    imp->set_window(w);    
    WindowHook::windows_[imp->GetSafeHwnd()] = imp;
    return imp;
  }  
  
  void dev_add_item(const std::string& item) {    
    AddString(Charset::utf8_to_win(item).c_str());    
  }

  void dev_set_items(const std::vector<std::string>& itemlist) {
    ResetContent();
    std::vector<std::string>::const_iterator it = itemlist.begin();
    for (; it != itemlist.end(); ++it) {      
      AddString(Charset::utf8_to_win((*it)).c_str());
    }
  }

  virtual std::vector<std::string> dev_items() const {
    std::vector<std::string> itemlist;
    for (int i = 0; i < this->GetCount(); ++i) {
      CString str;
      GetLBText(i, str);
      itemlist.push_back(Charset::win_to_utf8(str.GetString()));
    }
    return itemlist;
  }
  
  virtual void dev_set_item_index(int index) { SetCurSel(index); }
  virtual int dev_item_index() const { return GetCurSel(); }

  virtual std::string dev_text() const {		
    CString s;
    GetWindowText(s);		
		return Charset::win_to_utf8(s.GetString());
  }
  
  BOOL OnEraseBkgnd(CDC* pDC) { return CComboBox::OnEraseBkgnd(pDC); }

 protected:
  virtual BOOL prevent_propagate_event(ui::Event& ev, MSG* pMsg);

  DECLARE_MESSAGE_MAP()
  void OnPaint() { CComboBox::OnPaint(); }
  BOOL OnSelect();
};

class EditImp : public WindowTemplateImp<CEdit, ui::EditImp> {
 public:  
  EditImp() : WindowTemplateImp<CEdit, ui::EditImp>() {}

  static EditImp* Make(ui::Window* w, CWnd* parent, UINT nID) {
    EditImp* imp = new EditImp();
    if (!imp->Create(WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_AUTOHSCROLL,
                     CRect(0, 0, 200, 20), 
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

  virtual void dev_set_text(const std::string& text) { 
		SetWindowText(Charset::utf8_to_win(text).c_str());
	}
  virtual std::string dev_text() const {
    CString s;
    GetWindowText(s);		
		return Charset::win_to_utf8(s.GetString());
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
         _T(""), 
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

	void DevClearAll() { f(SCI_CLEARALL, 0, 0); }

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
  void DevReplaceSel(const std::string& text) { f(SCI_REPLACESEL, 0, text.c_str()); }
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
			streamoff size = file.tellg();
      char *contents = new char [static_cast<int>(size)];
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
    BOOL res = file.Open(Charset::utf8_to_win(filename).c_str(), CFile::modeCreate | CFile::modeReadWrite);
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
    return f(SCI_GETOVERTYPE, 0, 0) != 0;
  }
  bool dev_modified() const {
    return f(SCI_GETMODIFY, 0, 0) != 0;
  }
  int dev_add_marker(int line, int id) {
    return f(SCI_MARKERADD, line, id);
  }
  int dev_delete_marker(int line, int id) {
    return f(SCI_MARKERDELETE, line, id);
  }
  void dev_define_marker(int id, int symbol, ARGB foreground_color, ARGB background_color) {
    f(SCI_MARKERDEFINE, id, symbol);
    f(SCI_MARKERSETFORE, id, ToCOLORREF(foreground_color));
    f(SCI_MARKERSETBACK, id, ToCOLORREF(background_color));
  }
  void DevShowCaretLine() {
    f(SCI_SETCARETLINEVISIBLE, true, 0);
    f(SCI_SETCARETLINEVISIBLEALWAYS, true, 0);
  }
  void DevHideCaretLine() { f(SCI_SETCARETLINEVISIBLE, false, 0); }
  void dev_set_caret_line_background_color(ARGB color) { f(SCI_SETCARETLINEBACK, ToCOLORREF(color), 0); }
            
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
  virtual ui::Window* CreateWin() { 
    Window* window = new Window();
    WindowImp* imp = WindowImp::Make(window, DummyWindow::dummy(), WindowID::auto_id());
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

	virtual ui::AlertImp* CreateAlertImp() {
		return new AlertImp();
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
  virtual ui::FrameImp* CreateMainFrameImp() {     
	  return FrameImp::MakeMain(0);
  }
  virtual ui::FrameImp* CreatePopupFrameImp() {
    return PopupFrameImp::Make(0, ::AfxGetMainWnd(), WindowID::auto_id());    
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
  virtual ui::CheckBoxImp* CreateCheckBoxImp() {     
    return CheckBoxImp::Make(0, DummyWindow::dummy(), WindowID::auto_id());    
  }
	virtual ui::RadioButtonImp* CreateRadioButtonImp() {
		return RadioButtonImp::Make(0, DummyWindow::dummy(), WindowID::auto_id());
	}
	virtual ui::GroupBoxImp* CreateGroupBoxImp() {
		return GroupBoxImp::Make(0, DummyWindow::dummy(), WindowID::auto_id());
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
  virtual ui::FontImp* CreateFontImp() {    
    return new ui::mfc::FontImp();
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