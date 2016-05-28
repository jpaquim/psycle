// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "CanvasItems.hpp"
#include "Psycle.hpp"
#include "PsycleConfig.hpp"
#include "Ui.hpp"

namespace psycle {
namespace host  {
namespace ui {
namespace canvas {

  /*
void Rect::Draw(Graphics* g, Region& draw_region) {
  // double z = acczoom();  
  if (GetAlpha(fillcolor_) != 0xFF) {
   g->SetColor(fillcolor_);
   g->FillRect(0, 0, width_, height_);  
  }
/*    CRect rect(x1_*z, y1_*z, x2_*z, y2_*z);
  CPen pen;
  pen.CreatePen(PS_SOLID, 1, ToCOLORREF(strokecolor_));
  CBrush brush(ToCOLORREF(fillcolor_));
  CBrush* pOldBrush = devc->SelectObject(&brush);
  CPen* pOldPen = devc->SelectObject(&pen);
  double alpha = 1; // (GetAlpha(fillcolor_)) / 255.0;
  if (alpha == 1) {
    if (bx_!=0 || by_!=0) {
      CPoint pt(bx_*z, by_*z);
      devc->RoundRect(rect, pt);
    } else {
      devc->Rectangle(rect);
    }
  } else {
    this->paintRect(*devc, rect, ToCOLORREF(strokecolor_),
      ToCOLORREF(fillcolor_), (alpha*127));
  }
  devc->SelectObject(pOldPen);
  devc->SelectObject(pOldBrush);*/
// }

/*bool Rect::paintRect(CDC &hdc, RECT dim, COLORREF penCol, COLORREF brushCol, unsigned int opacity) {
  XFORM rXform;
  hdc.GetWorldTransform(&rXform);
  XFORM rXform_new = rXform;
  rXform_new.eDx = zoomabsx();
  rXform_new.eDy = zoomabsy();
  hdc.SetGraphicsMode(GM_ADVANCED);
  hdc.SetWorldTransform(&rXform_new);
  dim.right -= dim.left;
  dim.bottom -= dim.top;
  dim.left = dim.top = 0;
  HDC tempHdc         = CreateCompatibleDC(hdc);
  BLENDFUNCTION blend = {AC_SRC_OVER, 0, opacity, 0};
  HBITMAP hbitmap;       // bitmap handle
  BITMAPINFO bmi;        // bitmap header
  // zero the memory for the bitmap info
  ZeroMemory(&bmi, sizeof(BITMAPINFO));
  // setup bitmap info
  // set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on, the blending will occur in the center of each of the three areas.
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = dim.right-dim.left;
  bmi.bmiHeader.biHeight = dim.bottom-dim.top;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = (dim.right-dim.left) * (dim.bottom-dim.top) * 4;
  // create our DIB section and select the bitmap into the dc
  hbitmap = CreateDIBSection(tempHdc, &bmi, DIB_RGB_COLORS, NULL, NULL, 0x0);
  SelectObject(tempHdc, hbitmap);
  SetDCPenColor(tempHdc, penCol);
  SetDCBrushColor(tempHdc, brushCol);
  HBRUSH br = CreateSolidBrush(brushCol);
  FillRect(tempHdc, &dim, br);
  AlphaBlend(hdc, dim.left, dim.top, dim.right, dim.bottom, tempHdc, dim.left, dim.top, dim.right, dim.bottom, blend);
  DeleteObject(hbitmap);
  DeleteObject(br);
  DeleteObject(tempHdc);
  hdc.SetGraphicsMode(GM_ADVANCED);
  hdc.SetWorldTransform(&rXform);
  return 0;
}*/


void Line::Draw(Graphics* g, Region& draw_region) {  
  g->SetColor(color());
  ui::Point mp;  
  for (Points::iterator it = pts_.begin(); it != pts_.end(); ++it) {
    Point& pt = (*it);
    if (it != pts_.begin()) {
      g->DrawLine(mp, pt);
    }
    mp = pt;    
  }
}

Window::Ptr Line::Intersect(double x, double y, Event* ev, bool &worked) {
  /*double distance_ = 5;
  Point  p1 = PointAt(0);
  Point  p2 = PointAt(1);
  double  ankathede    = p1.first - p2.first;
  double  gegenkathede = p1.second - p2.second;
  double  hypetenuse   = sqrt(ankathede*ankathede + gegenkathede*gegenkathede);
  if (hypetenuse == 0) return 0;
  double cos = ankathede    / hypetenuse;
  double sin = gegenkathede / hypetenuse;
  int dx = static_cast<int> (-sin*distance_);
  int dy = static_cast<int> (-cos*distance_);
  std::vector<CPoint> pts;
  CPoint p;
  p.x = p1.first + dx; p.y = p1.second - dy;
  pts.push_back(p);
  p.x = p2.first + dx; p.y = p2.second - dy;
  pts.push_back(p);
  p.x = p2.first - dx; p.y = p2.second + dy;
  pts.push_back(p);
  p.x = p1.first - dx; p.y = p1.second + dy;
  pts.push_back(p);
  CRgn rgn;
  rgn.CreatePolygonRgn(&pts[0],pts.size(), WINDING);
  Item::Ptr item = rgn.PtInRegion(x-this->x(),y-this->y()) ? this : 0;
  rgn.DeleteObject();*/
  return Window::Ptr();
}

bool Line::OnUpdateArea() {  
  double dist = 5;  
  double zoom = 1.0; // parent() ? parent()->zoom() : 1.0;
  ui::Rect bounds = area().bounds();
  area_->Clear();
  area_->Add(RectShape(ui::Rect(ui::Point((bounds.left()-dist)*zoom, 
                                          (bounds.top()-dist)*zoom),
                                ui::Point((bounds.left() + bounds.width()+2*dist+1)*zoom, 
                                          (bounds.top() + bounds.height()+2*dist+1)*zoom))));
  return true;
}

bool Text::OnUpdateArea() {   
  area_->Clear();  
  if (!auto_size_width() && !auto_size_height()) {    
    area_->Add(RectShape(ui::Rect(area_->bounds().top_left(), imp()->dev_pos().dimension())));
  } if (auto_size_width() && auto_size_height()) {
    std::auto_ptr<Graphics> g(ui::Systems::instance().CreateGraphics());
    area_->Add(RectShape(ui::Rect(area_->bounds().top_left(), g->text_size(text_))));
  } else {
    std::auto_ptr<Graphics> g(ui::Systems::instance().CreateGraphics());
    double width = auto_size_width() ? g->text_size(text_).width() : imp()->dev_pos().dimension().width();
    double height = auto_size_height() ? g->text_size(text_).height() : imp()->dev_pos().dimension().height();
    area_->Add(RectShape(ui::Rect(area_->bounds().top_left(), Dimension(width, height))));
  }
  return true;  
}

void Text::set_text(const std::string& text) {  
  STR();
  text_ = text; 
  Window::set_pos(pos().top_left());    
  FLS();          
}

void Text::Draw(Graphics* g, Region& draw_region) {   
//  g->SetFont(*font_);
  g->SetColor(color_);
  double xp(0);
  double yp(0);
  if (alignment_ & ALCENTER) {
    ui::Dimension text_dim = g->text_size(text_);    
    xp = (dim().width() - text_dim.width()) / 2;
    yp = (dim().height() - text_dim.height()) / 2;
  }
  g->DrawString(text_, xp, yp);    
}

// Pic
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
}

void Pic::Draw(Graphics* g, Region& draw_region) {
  g->DrawImage(image_, 0, 0, width_, height_, xsrc_, ysrc_);
  // todo zoom  
}

void Pic::SetImage(Image* image) {
  STR();
  image_ = image;
  width_ = image_->dim().width();
  height_ = image_->dim().height();
  FLS();
}

bool Pic::OnUpdateArea() {
  area_->Clear();
  area_->Add(RectShape(ui::Rect(ui::Point(), ui::Point(width_, height_))));  
  return true;
}


} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle