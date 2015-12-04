// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#include <psycle/host/detail/project.private.hpp>
#include "Canvas.hpp"
#include <algorithm> // for std::transform
#include <cctype> // for std::tolower

namespace psycle {
namespace host  {
namespace ui {

int ui::MenuItem::id_counter = 0;
std::map<std::uint16_t, ui::MenuItem*> ui::MenuItem::menuItemIdMap;

namespace mfc {

void Image::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans) {
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

void Graphics::TransparentBlt(CDC* pDC,
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

void MenuBar::append(ui::Menu* menu) {
  int pos = menu->cmenu()->GetMenuItemCount();
  std::vector<ui::Menu*>::iterator it = items.begin();
  for ( ; it != items.end(); ++it) {
    ui::Menu* m = *it;
    menu->cmenu()->AppendMenu(MF_POPUP, (UINT_PTR)m->cmenu()->m_hMenu, m->label().c_str());
    m->set_parent(menu);
    m->set_pos(pos++);
  }
  menu->setbar(this);
}

void MenuBar::remove(CMenu* menu, int pos) {
    std::vector<ui::Menu*>::iterator it = items.begin();
    for ( ; it != items.end(); ++it) {
      menu->RemoveMenu(pos++, MF_BYPOSITION);
    }
}

// Menu
void Menu::set_label(const std::string& label) {
  label_ = label;
  if (parent()) {
    parent()->cmenu()->ModifyMenu(pos_, MF_BYPOSITION, 0, label.c_str());
    ui::MenuBar* b = bar();
    b->setupdate(b!=0);
  }
}

void Menu::add(ui::Menu* newmenu) {
  cmenu_->AppendMenu(MF_POPUP | MF_ENABLED, (UINT_PTR)newmenu->cmenu()->m_hMenu, newmenu->label().c_str());
  newmenu->set_parent(this);
  newmenu->set_pos(cmenu_->GetMenuItemCount()-1);
}

void Menu::add(ui::MenuItem* item) {
  items.push_back(item);
  item->set_menu(this);
  const int id = ID_DYNAMIC_MENUS_START+item->id_counter;
  item->set_id(id);
  MenuItem::menuItemIdMap[item->id()] = item;
  cmenu_->AppendMenu(MF_STRING, id, item->label().c_str());
  if (item->checked()) {
		cmenu_->CheckMenuItem(id, MF_CHECKED | MF_BYCOMMAND);
  }
}

void Menu::addseparator() {
  cmenu_->AppendMenu(MF_SEPARATOR, 0, "-");
}

void Menu::remove(ui::MenuItem* item) {
  std::vector<ui::MenuItem*>::iterator it;
  it = std::find(items.begin(), items.end(), item);
  if (it != items.end()) {
    items.erase(it);
  }
  cmenu_->RemoveMenu(item->id(), MF_BYCOMMAND);
}

// menuitem  

void MenuItem::set_label(const std::string& label) {
  label_ = label;
  if (menu_) {
      menu_->cmenu()->ModifyMenu(id(), MF_BYCOMMAND, id(), label.c_str());
  }
}

void MenuItem::check() {
  check_ = true;
  if (menu_) {
    menu_->cmenu()->CheckMenuItem(id(), MF_CHECKED | MF_BYCOMMAND);
  }
}

void MenuItem::uncheck() {
  check_ = false;
  if (menu_) {
      menu_->cmenu()->CheckMenuItem(id(), MF_UNCHECKED | MF_BYCOMMAND);
  }
}

}  // namespace mfc

namespace canvas {

void Item::Detach() { 
  if (!parent_.expired()) {
    parent_.lock()->Remove(shared_from_this());
  }
}

double Item::acczoom() const {
  double zoom = 1.0;
  /*Item::ConstWeakPtr p = parent();
  while (!p.expired()) {
    zoom *= p.lock()->zoom();
    p = p.lock()->parent();
  }*/
  return zoom;
}

Canvas* Item::root() {  
  Item::Ptr item = shared_from_this();
  do {
    if (item->is_root()) return (Canvas*) item.get();
    item = item->parent().lock();
  } while (item);  
  return 0;
}

void Item::needsupdate() {
  update_ = true;
  if (!parent().expired()) {
    parent().lock()->needsupdate();
  }
}

void Item::GetFocus() {
  Canvas* c = root();
  if (c) {
    c->StealFocus(shared_from_this());
  }
}

void Item::STR() {  
  fls_rgn_.reset(region().Clone());
  fls_rgn_->Offset(zoomabsx(), zoomabsy());
  has_store_ = true;
}

void Item::FLS() {
  if (has_store_) {
    needsupdate();
    std::auto_ptr<Region> tmp(region().Clone());
    tmp->Offset(zoomabsx(), zoomabsy());
    fls_rgn_->Combine(*tmp, RGN_OR);
    has_store_ = false;    
  } else {
    STR();
    has_store_ = false;
  }
  if (visible() && !parent().expired()) {
    Canvas* c = this->root();
    if (c && c->wnd()) {
      if (has_clip()) {
        std::auto_ptr<Region> tmp(clip().Clone());
        tmp->Offset(zoomabsx(), zoomabsy());
        fls_rgn_->Combine(*tmp, RGN_AND);        
      }
      c->Invalidate(*fls_rgn_);
    }
  }
}

void Item::FLS(const Region& rgn) {
  std::auto_ptr<Region> tmp(rgn.Clone());
  tmp->Combine(region(), RGN_AND);
  tmp->Offset(zoomabsx(), zoomabsy());
  STR(); has_store_ = false;
  fls_rgn_->Combine(*tmp, RGN_AND);
  if (!parent().expired()) {
    Canvas* c = this->root();
    if (c) {      
      c->Invalidate(*fls_rgn_);
    }
  }
}

void Item::SetBlitXY(double x, double y) {    
  BlitInfo* bi = new BlitInfo();    
  bi->dx = x - x_;
  bi->dy = y - y_;    
  blit_.reset(bi);
  double xr, yr, w, h;
  region().BoundRect(xr, yr, w, h);    
  x_ = x; y_ = y;
  Canvas* c = root();
  if (c) {
    c->item_blit_ = true;
  }
  FLS();
}

double Item::zoomabsx() const {
  std::vector<Item::ConstPtr> items;
  items.push_back(shared_from_this());
  Item::ConstWeakPtr p = parent();
  while (!p.expired()) {
    items.push_back(p.lock());
    p = p.lock()->parent();
  }
  double x = 0.0;
 // double zoom = 1.0;
  std::vector<Item::ConstPtr>::const_reverse_iterator rev_it = items.rbegin();
  for ( ; rev_it != items.rend(); ++rev_it) {
    Item::ConstPtr item = *rev_it;
    // zoom *= item->parent() ? item->parent()->zoom() : 1.0;
    x += item->x();
  }
  return x;
}

double Item::zoomabsy() const {
  std::vector<Item::ConstPtr> items;
  items.push_back(shared_from_this());
  Item::ConstWeakPtr p = parent();
  while (!p.expired()) {
    items.push_back(p.lock());
    p = p.lock()->parent();
  }
  double y = 0.0;
  // double zoom = 1.0;
  std::vector<Item::ConstPtr>::reverse_iterator rev_it = items.rbegin();
  for ( ; rev_it != items.rend(); ++rev_it) {
    const Item::ConstPtr item = *rev_it;
    //zoom *= item->parent() ? item->parent()->zoom() : 1.0;
    y += item->y();
  }
  return y;
}

bool Item::IsInGroup(Item::WeakPtr group) const {  
  Item::ConstWeakPtr p = parent();
  while (!p.expired()) {
    if (group.lock() == p.lock()) {
      return true;
    }
    p = p.lock()->parent();
  }
  return false;
}


void Group::Draw(Graphics* g, Region& draw_region) {
  for (iterator it = items_.begin(); it != items_.end(); ++it ) {
    Item::Ptr item = *it;
    if (!item->visible()) continue;
    std::auto_ptr<Region> item_rgn(item->region().Clone());
    double dx(0), dy(0);
    if (!item->parent().expired()) {
      dx = item->zoomabsx();
      dy = item->zoomabsy();
      item_rgn->Offset(dx, dy);
    }
    int erg = item_rgn->Combine(draw_region, RGN_AND);    
    if (erg != NULLREGION) {      
      g->SaveOrigin();
      g->Translate(item->x(), item->y());
      if (item->has_clip()) {        
        g->SetClip(item_rgn.get());
      }
      if (item->blit_.get()) {        
        double x, y, w, h;
        item_rgn->BoundRect(x, y, w, h);                    
        mfc::Region tmp;
        if (item->blit_->dy != 0) {
          g->CopyArea(-item->x(), -item->y(), w, h, 0, item->blit_->dy);
          if (item->blit_->dy > 0) {
            tmp.SetRect(x, y, w, item->blit_->dy);              
          } else {            
            tmp.SetRect(x, y+h+item->blit_->dy, w, -item->blit_->dy);              
          }          
          item->draw_rgn_ = &tmp;          
          g->SetClip(&tmp);
          item->Draw(g, tmp);
          g->SetClip(0);
        }
        if (item->blit_->dx != 0) {
          g->CopyArea(-item->x(), -item->y(), w, h, item->blit_->dx, 0);
          if (item->blit_->dx > 0) {
            tmp.SetRect(x, y, item->blit_->dx, h);              
          } else {            
            tmp.SetRect(x+w+item->blit_->dx, y, -item->blit_->dx, h);              
          }          
          item->draw_rgn_ = &tmp;          
          g->SetClip(&tmp);
          item->Draw(g, tmp);
          g->SetClip(0);
        }
        item->draw_rgn_ = &tmp;          
        g->SetClip(&tmp);
        item->Draw(g, tmp);
        g->SetClip(0);
        item->blit_.reset(0);
        item->draw_rgn_ = 0;
        root()->item_blit_ = false;
      } else {
        item->draw_rgn_ = item_rgn.get();
        item->Draw(g, *item_rgn);
        item->draw_rgn_ = 0;
      }
      if (item->has_clip()) {
        g->SetClip(0);
      }
      g->RestoreOrigin();
    }
  }
}

void Group::Add(const Item::Ptr& item) {  
  if (!item->parent().expired()) {
    throw std::runtime_error("Item already child of a group.");
  }
  STR();
  item->set_parent(shared_from_this());
  items_.push_back(item);  
  FLS();
  item->needsupdate();
}

void Group::Insert(iterator it, const Item::Ptr& item) {
  assert(item);
  assert(item->parent().expired());
  STR();
  item->set_parent(shared_from_this());
  items_.insert(it, item);
  FLS();
}

void Group::Remove(const Item::Ptr& item) {
  assert(item);
  iterator it = find(items_.begin(), items_.end(), item);
  if (it == items_.end()) {
    throw std::runtime_error("Item is no child of the group");
  }
  STR();
  items_.erase(it);  
  item->set_parent(Item::WeakPtr());
  FLS();
}

void Group::Clear() {
  STR();  
  items_.clear();
  FLS();
}

void Group::onupdateregion() {  
  std::auto_ptr<Region> rgn(new mfc::Region());  
  std::vector<Item::Ptr>::const_iterator it = items_.begin();
  for ( ; it != items_.end(); ++it) {
    Item::Ptr item = *it;
    if (item->visible()) {
      std::auto_ptr<Region> tmp(item->region().Clone());      
      tmp->Offset(item->x(), item->y());        
      int nCombineResult = rgn->Combine(*tmp, RGN_OR);
      if (nCombineResult == NULLREGION) {
        rgn->Clear();
      }
    }
  }    
  rgn_.reset(rgn->Clone());
}

void Group::set_zorder(Item::Ptr item, int z) {
  assert(item);
  if (z<0 || z>=items_.size()) return;
  iterator it = find(items_.begin(), items_.end(), item);
  assert(it != items_.end());
  STR();
  items_.erase(it);
  items_.insert(begin()+z, item);
  FLS();
}

int Group::zorder(Item::Ptr item) const {
  int z = -1;
  for (int k = 0; k < items_.size(); k++) {
    if (items_[k] == item) {
      z = k;
      break;
    }
  }
  return z;
}

Item::Ptr Group::Intersect(double x, double y, Event* ev, bool& worked) {  
  if (!Item::Intersect(x, y, ev,worked)) {
    return 0;
  }
  if (ev->type() == Event::BUTTON_PRESS) {
    int fordebugonly = 0;
  }
  Item::WeakPtr found;
  Item::WeakPtr found_noevent;
  ItemList::const_reverse_iterator rev_it = items_.rbegin();
  for ( ; rev_it != items_.rend(); ++rev_it) {
    Item::Ptr item = *rev_it;
    item = item->visible() 
           ? item->Intersect(x-item->x(), y-item->y(), ev, worked)
           : 0; 
    if (worked) {
      return item;
    }
    if (item) {
      if (WorkItemEvent(item, ev)) {
        worked = true;
        return item;
      } else {
        if (found_noevent.expired()) {
            found_noevent = item;
        }
      }
    }
  }
  if (!found_noevent.expired()) {    
    Canvas* c = root();
    if (c) {
      found = c->DelegateEvent(ev, found_noevent);
      if (!found.expired()) {
        worked = true;
        return found.lock();
      }
    }  
  }
  return found.lock();
}

bool Group::WorkItemEvent(Item::WeakPtr item, Event* ev) {
  bool erg = false;
  Canvas* c = root();
  if (c) {    
    Event::Type type = ev->type();
    Event e(item, type, ev->cx(), ev->cy(), ev->button(), ev->shift());
    if (type == Event::MOTION_NOTIFY && !c->mouse_move_.expired()) {
      if (c->mouse_move_.lock() != item.lock() ) { // && !item.lock()->IsInGroup(c->mouse_move_)) {
        Event evout(c->mouse_move_, Event::MOTION_OUT, ev->cx(), ev->cy(),ev->button(), ev->shift());
        c->mouse_move_.lock()->OnEvent(&evout);
        c->mouse_move_.reset();
      }
    }
    erg = !item.lock()->OnEvent(&e).expired();
    if (erg) {
      if (type == Event::BUTTON_PRESS || 
        type == Event::BUTTON_2PRESS) {
        c->button_press_item_ = c->focus_item_ = item;        
      } else
      if (type == Event::MOTION_NOTIFY) {
        c->mouse_move_ = item;
      }
    }
  }
  return erg;
}

void Group::OnMessage(CanvasMsg msg) {
  for (iterator it = items_.begin(); it != items_.end(); ++it ) {
    (*it)->OnMessage(msg);
  }
}


void Rect::Draw(Graphics* g, Region& draw_region) {
  // double z = acczoom();  
  g->SetColor(fillcolor_);
  g->FillRect(0, 0, width_, height_);  
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
}

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

void Rect::onupdateregion() {
  rgn_->SetRect(0, 0, width_+1, height_+1);  
}

void Line::Draw(Graphics* g, Region& draw_region) {  
  g->SetColor(color());
  double mx, my;
  mx = my = 0;
  for (Points::iterator it = pts_.begin(); it != pts_.end(); ++it) {
    Point& pt = (*it);
    if (it != pts_.begin()) {
      g->DrawLine(mx, my, pt.first, pt.second);
    }
    mx = pt.first;
    my = pt.second;
  }  
}

Item::Ptr Line::Intersect(double x, double y, Event* ev, bool &worked) {
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
  return Item::Ptr();
}

void Line::onupdateregion() {  
  double x1, y1, x2, y2;
  double dist = 5;
  BoundRect(x1, y1, x2, y2);
  double zoom = 1.0; // parent() ? parent()->zoom() : 1.0;
  rgn_->SetRect((x1-dist)*zoom, (y1-dist)*zoom, (x2+2*dist+1)*zoom, (y2+2*dist+1)*zoom);    
}

void Text::Init(double zoom) {
  color_ = 0;
  LOGFONT lfLogFont;
  memset(&lfLogFont, 0, sizeof(lfLogFont));
  lfLogFont.lfHeight = 12*zoom;
  strcpy(lfLogFont.lfFaceName, "Arial");
  font_.CreateFontIndirect(&lfLogFont);
}

void Text::onupdateregion() {  
    Canvas* c =  const_cast<Text*>(this)->root();
    if (c) {
      HDC dc = GetDC(0);
      SIZE extents = {0};
      HFONT old_font =
        reinterpret_cast<HFONT>(SelectObject(dc, font_));
      GetTextExtentPoint32(dc, text_.c_str(), text_.length(),
        &extents);
      SelectObject(dc, old_font);
      ReleaseDC(0, dc);
      text_w = extents.cx;
      text_h = extents.cy+10;
      rgn_->SetRect(0, 0, text_w +1, text_h+1);
      update_ = false;
    }    
}

void Text::Draw(Graphics* g, Region& draw_region) {    
    g->SetColor(color_);
    g->DrawString(text_, 0, 0);    
}

// Pic
inline void PremultiplyBitmapAlpha(HDC hDC, HBITMAP hBmp)
{
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
  image_->Size(width_, height_);  
  FLS();
}

void Pic::onupdateregion() {
  rgn_->SetRect(0, 0, width_, height_);    
}

// Canvas
void Canvas::Init() {
  is_root_ = true;
  steal_focus_ = item_blit_ = show_scrollbar = save_ = false;
  bg_image_ = 0;  
  bg_width_ = bg_height_ = 0;
  cw_ = pw_ = ch_ = ph_ = 300;
  nposv = nposh = 0;
  color_ = 0;
  save_rgn_.SetRect(0, 0, cw_, ch_);    
  cursor_ = LoadCursor(0, IDC_ARROW);
  old_wnd_ = wnd_;    
}

void Canvas::Draw(Graphics *g, Region& rgn) {
  if (IsSaving()) {
    save_rgn_.Combine(rgn, RGN_OR);
    DoDraw(g, save_rgn_);
    save_rgn_.Clear();
  } else {
    DoDraw(g, rgn);
  }
}

void Canvas::DoDraw(Graphics *g, Region& rgn) {
  if (bg_image_)	{
    /*CDC memDC;
    memDC.CreateCompatibleDC(devc);
    CBitmap* oldbmp = memDC.SelectObject(bg_image_);
    if ((cw_ > bg_width_) || (ch_ > bg_height_)) {
      for (int cx=0; cx<cw_; cx+=bg_width_) {
        for (int cy=0; cy<ch_; cy+=bg_height_) {
          devc->BitBlt(cx, cy, bg_width_, bg_height_, &memDC, 0, 0, SRCCOPY);
        }
      }
    }
    else {
      devc->BitBlt(0, 0, cw_, ch_, &memDC, 0, 0, SRCCOPY);
    }
    memDC.SelectObject(oldbmp);
    memDC.DeleteDC();*/
  } else {    
    if (!item_blit_) {
      double x, y, width, height;
      rgn.BoundRect(x, y, width, height);      
      g->SetColor(color_);    
      g->FillRect(x, y, width, height);
    }    
  }
  Group::Draw(g, rgn);
}

void Canvas::StealFocus(Item::Ptr item) {
  button_press_item_ = item;
  steal_focus_ = true;
}

Item::WeakPtr Canvas::OnEvent(Event* ev) {
  if (!ev->item().expired()) {
    return Item::WeakPtr();
  }
  if (ev->type() == Event::ONTIMER || ev->type() == Event::SCROLL || ev->type() == Event::ONSIZE) {
    return Item::WeakPtr();
  }
  Item::WeakPtr item = button_press_item_;  
  if (!item.expired() && ev->type() == Event::KEY_DOWN || !item.expired() && ev->type() == Event::KEY_UP) {
    if (!focus_item_.expired()) {
      Event e(focus_item_, ev->type(), ev->cx(), ev->cy(), ev->button(), ev->shift());
      DelegateEvent(&e, focus_item_);
      item = e.item();
    }
  } else
  if (!item.expired() && ev->type() == Event::MOTION_NOTIFY ) {
      Event e(item, ev->type(), ev->cx(), ev->cy(), ev->button(), ev->shift());
      DelegateEvent(&e, item);
      item = e.item();
  } else
  if (ev->type() != Event::BUTTON_RELEASE) {
      if (ev->type() == Event::BUTTON_PRESS) {
         int fordebugonly = 0;
      }
      bool worked = false;
      item = Group::Intersect(ev->cx(), ev->cy(), ev, worked);
      if (ev->type() == Event::MOTION_NOTIFY && 
          button_press_item_.expired() &&
          !mouse_move_.expired() &&
          (item.expired() || (item.lock() != mouse_move_.lock())))
      {
        Event evout(mouse_move_, Event::MOTION_OUT, ev->cx(), ev->cy(),ev->button(), ev->shift());
        mouse_move_.lock()->OnEvent(&evout);
        mouse_move_.reset();    
      }      
  }
  if (ev->type() == Event::BUTTON_RELEASE || ev->type() == Event::BUTTON_2PRESS) {
      ev->setitem(item);
      button_press_item_.reset();
      if (!item.expired()) {
        item.lock()->OnEvent(ev);
      }
  }
  return item;
}

Item::WeakPtr Canvas::DelegateEvent(Event* ev, Item::WeakPtr item) {
  bool erg = item.lock()->pointerevents() && !item.lock()->OnEvent(ev).expired();
  while (!erg && !item.lock()->parent().expired()) {
    item = item.lock()->parent();  // redirect event to parent
    if (!button_press_item_.expired()) {
      button_press_item_ = item;
    }
    ev->setitem(item);
    erg = item.lock()->pointerevents() && !item.lock()->OnEvent(ev).expired();
    if (steal_focus_) {
      steal_focus_ = 0;
      return item;
    }
  }
  return item;
}

void Canvas::Invalidate(Region& rgn) { 
  if (IsSaving()) {
    save_rgn_.Combine(rgn, RGN_OR);
  } else if (wnd_) {
      //wnd_->InvalidateRgn((CRgn*)rgn.source(), 0);
    RedrawWindow(wnd_->m_hWnd, NULL, *((CRgn*) rgn.source()), RDW_INVALIDATE | RDW_UPDATENOW);
  }
}

void Canvas::Flush() {
  if (wnd_) {
    RedrawWindow(wnd_->m_hWnd, NULL, *((CRgn*) save_rgn_.source()), RDW_INVALIDATE | RDW_UPDATENOW);
    // wnd_->InvalidateRgn((CRgn*) save_rgn_.source(), 0);
  }
  save_rgn_.Clear();
}


IMPLEMENT_DYNAMIC(CScintilla, CWnd)

BEGIN_MESSAGE_MAP(CScintilla, CWnd)
ON_NOTIFY_REFLECT_EX(SCN_CHARADDED, OnModified)
END_MESSAGE_MAP()

// CanvasView
bool BaseView::DelegateEvent(int type, int button, UINT nFlags, CPoint pt) {
  try {    
    if (!canvas_.expired()) {
      Event ev(Item::WeakPtr(), (Event::Type)type, pt.x, pt.y, button, nFlags);
      Canvas* c = canvas().lock().get();
      return !c->OnEvent(&ev).expired() ? true : false;
    }
  } catch (std::exception& e) {
    AfxMessageBox(e.what());
  }  
  return false;
}

void BaseView::Draw() { 
  CRgn rgn;
  rgn.CreateRectRgn(0, 0, 0, 0);
	int result = wnd_->GetUpdateRgn(&rgn, FALSE);

  if (!result) return; // If no area to update, exit.
	
	CPaintDC dc(wnd_);

  if (!bmpDC.m_hObject) {// buffer creation	
		CRect rc;
		wnd_->GetClientRect(&rc);		
		bmpDC.CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);
		char buf[128];
		sprintf(buf,"CanvasView::OnPaint(). Initialized bmpDC to 0x%p\n",(void*)bmpDC);
		TRACE(buf);
	}
  CDC bufDC;
	bufDC.CreateCompatibleDC(&dc);
	CBitmap* oldbmp = bufDC.SelectObject(&bmpDC);	    
  if (!canvas_.expired()) {
    Canvas* c = canvas().lock().get();
    try {
      ui::mfc::Graphics g(&bufDC);
      ui::mfc::Region canvas_rgn(rgn);
      c->Draw(&g, canvas_rgn);
    } catch (std::exception& e) {
      AfxMessageBox(e.what());
    }
  }

  CRect rc;
  wnd_->GetClientRect(&rc);
	dc.BitBlt(0, 0, rc.right-rc.left, rc.bottom-rc.top, &bufDC, 0, 0, SRCCOPY);
	bufDC.SelectObject(oldbmp);
	bufDC.DeleteDC();
  rgn.DeleteObject();    
}

BEGIN_MESSAGE_MAP(View, CWnd)  
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_WM_SETFOCUS()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
  ON_WM_KEYDOWN()
	ON_WM_KEYUP()
  ON_WM_SETCURSOR()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_SIZE()
  ON_WM_KEYDOWN()
  ON_WM_KEYUP()
  ON_CONTROL_RANGE(BN_CLICKED, ID_DYNAMIC_CONTROLS_BEGIN, 
            ID_DYNAMIC_CONTROLS_LAST, OnCtrlBtnClick)
END_MESSAGE_MAP()


BOOL View::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;
	// CS_HREDRAW | CS_VREDRAW |
	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass
		(
				CS_DBLCLKS,
			//::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);
			::LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)GetStockObject( HOLLOW_BRUSH ),
			NULL
		);

	return TRUE;
}

int View::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}
  cw_ = 0; ch_ = 0;  
	return 0;
}

void View::OnDestroy() {  
  BaseView::OnDestroy();
}

/* BOOL View::PreCreateWindow(CREATESTRUCT& cs) {
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;
			
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	return TRUE;
}*/

void View::OnSetFocus(CWnd* pOldWnd) {
	CWnd::OnSetFocus(pOldWnd);
	GetParent()->SetFocus();
}

void View::OnTimerViewRefresh() {    
  if (!canvas_.expired() && IsWindowVisible()) {
    try {          
      Canvas* c = canvas().lock().get();
      if (c->show_scrollbar) {
        ShowScrollBar(SB_BOTH, TRUE);
        SCROLLINFO si;
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_PAGE | SIF_RANGE;
				si.nMin = 0;
				si.nMax = c->nposv; // -VISLINES;
				si.nPage = 1;
				SetScrollInfo(SB_VERT,&si);
        si.nMax = c->nposh; // -VISLINES;
        SetScrollInfo(SB_HORZ, &si);
        c->show_scrollbar = false;
      }
      if (c->IsSaving()) {
        c->set_wnd(this);
        c->InvalidateSave();
      }
      DelegateEvent(Event::ONTIMER, 0, 0, CPoint(0, 0));
    } catch(std::exception& e) {
      e;
    }    
  }
}

void View::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {   
  if (nSBCode == SB_THUMBTRACK) {    
    if (pScrollBar) {
     ScrollBar* scrbar = (ScrollBar*) ScrollBar::FindById(pScrollBar->GetDlgCtrlID());
     pScrollBar->SetScrollPos(nPos);     
     try {
       scrbar->OnScroll(nPos);
     } catch (std::exception& e) {
       AfxMessageBox(e.what());
     }
    } else {
      CPoint pt(nPos, 0);
      DelegateEvent(canvas::Event::SCROLL, 0, 0, pt);
      SetScrollPos(SB_HORZ, nPos, false);
    }    
  }
  CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}


void View::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {  
  if (nSBCode == SB_THUMBTRACK) {    
    if (pScrollBar) {
      ScrollBar* scrbar = (ScrollBar*) ScrollBar::FindById(pScrollBar->GetDlgCtrlID());
      pScrollBar->SetScrollPos(nPos);
      try {
       scrbar->OnScroll(nPos);
     } catch (std::exception& e) {
       AfxMessageBox(e.what());
     }
    } else {
      CPoint pt(nPos, 0);
      DelegateEvent(canvas::Event::SCROLL, 0, 0, pt);
      SetScrollPos(SB_VERT, nPos, false);
    }    
  }
  CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL View::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {    
   Canvas* c = canvas().lock().get();    
   if (c) {
    SetCursor(c->cursor());
    return TRUE;
   }
  return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void View::OnSize(UINT nType, int cw, int ch) {  
  if (bmpDC.m_hObject != NULL) { // remove old buffer to force recreating it with new size
	  TRACE("CanvasView::OnResize(). Deleted bmpDC\n");
	  bmpDC.DeleteObject();	  
  }
  Resize(cw, ch);
  cw_ = cw;
  ch_ = ch;
  CWnd::OnSize(nType, cw, ch);
}


// CanvasFrame
IMPLEMENT_DYNAMIC(CanvasFrameWnd, CFrameWnd)

BEGIN_MESSAGE_MAP(CanvasFrameWnd, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
  ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()  
END_MESSAGE_MAP()

int CanvasFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)  {		
  if( CFrameWnd::OnCreate(lpCreateStruct) == 0) {
    pView_ = new View();
    pView_->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL);    
    return 0;
  }       
  return -1;		
}
    
BOOL CanvasFrameWnd::PreCreateWindow(CREATESTRUCT& cs) {
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
			
  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	return TRUE;
}

void CanvasFrameWnd::OnDestroy() {    
	/*HICON _icon = GetIcon(false);
	DestroyIcon(_icon);*/
  if (pView_ != NULL) { 
    pView_->DestroyWindow();
    delete pView_; pView_ = 0;
  }  
}

void CanvasFrameWnd::PostNcDestroy() {			
	CFrameWnd::PostNcDestroy();
}

// Messages
void CanvasFrameWnd::OnSetFocus(CWnd* pOldWnd) {
	CFrameWnd::OnSetFocus(pOldWnd);			
}

BOOL CanvasFrameWnd::OnEraseBkgnd(CDC* dc)  {
  return TRUE;
}

void CanvasFrameWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {          	
	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);	
}

void CanvasFrameWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {              			
	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);  
}


} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle