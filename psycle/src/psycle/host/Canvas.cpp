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
  BITMAP bm;
  // Get the dimensions of the source bitmap
  pBmpSource->GetObject(sizeof(BITMAP), &bm);
  // Create the mask bitmap
  pBmpMask->DeleteObject();
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
  int i;
  int z = 0;
  
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
void Item::Init(Group* parent, double x, double y) {  
  parent_ = 0;  
  if (parent) parent->Add(this);    
  x_ = x;
  y_ = y;
  managed_ = has_store_ = false;
  visible_ = pointer_events_ = update_=  true;
  has_clip_ = false;
}

Item::~Item() {
  CheckButtonPress();
  CheckFocusItem();
  if (parent_ && !managed_) {
    iterator it = find(parent_->begin(), parent_->end(), this);
    parent_->items_.erase(it);
  }  
}

void Item::Detach() { if (parent_) parent_->Remove(this); }

void Item::CheckButtonPress() {
  Canvas* c = canvas();
  if (c) {
    Item* bp = c->button_press_item_;
    while (bp) {
      if (bp == this) {
        c->button_press_item_ = 0;
        break;
      }
      bp = bp->parent();
    }
  }
}

void Item::CheckFocusItem() {
  Canvas* c = canvas();
  if (c) {
    Item* bp = c->focus_item_;
    while (bp) {
      if (bp == this) {
        c->focus_item_ = 0;
        break;
      }
      bp = bp->parent();
    }
  }
}

double Item::acczoom() const {
  double zoom = 1.0;
  const Group* p = parent();
  while (p) {
    zoom *= p->zoom();
    p = p->parent();
  }
  return zoom;
}

Canvas* Item::canvas() {
  Group* p = this->parent();
  if (!p) { return this->widget(); }
  while (p->parent()) p = p->parent();
  return p->widget();
}

void Item::needsupdate() {
  update_ = true;
  if (parent()) parent()->needsupdate();
}

void Item::GetFocus() {
  Canvas* c = canvas();
  if (c) c->StealFocus(this);
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
  if (parent()) {
    Canvas* c = this->canvas();
    if (c) {
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
  std::auto_ptr<Region> tmp(region().Clone());
  tmp->Offset(zoomabsx(), zoomabsy());
  STR(); has_store_ = false;
  fls_rgn_->Combine(*tmp, RGN_AND);
  if (parent()) {
    Canvas* c = this->canvas();
    if (c) {
      if (has_clip()) {
        std::auto_ptr<Region> tmp(clip().Clone());
        tmp->Offset(zoomabsx(), zoomabsy());
        fls_rgn_->Combine(*tmp, RGN_AND);              
      }
      c->Invalidate(*fls_rgn_);
    }
  }
}

double Item::zoomabsx() const {
  std::vector<const Item*> items;
  items.push_back(this);
  const Group* p = parent();
  while (p) {
    items.push_back(p);
    p = p->parent();
  }
  double x = 0.0;
 // double zoom = 1.0;
  std::vector<const Item*>::const_reverse_iterator rev_it = items.rbegin();
  for ( ; rev_it != items.rend(); ++rev_it) {
    const Item* item = *rev_it;
    // zoom *= item->parent() ? item->parent()->zoom() : 1.0;
    x += item->x();
  }
  return x;
}

double Item::zoomabsy() const {
  std::vector<const Item*> items;
  items.push_back(this);
  const Group* p = parent();
  while (p) {
    items.push_back(p);
    p = p->parent();
  }
  double y = 0.0;
  // double zoom = 1.0;
  std::vector<const Item*>::const_reverse_iterator rev_it = items.rbegin();
  for ( ; rev_it != items.rend(); ++rev_it) {
    const Item* item = *rev_it;
    //zoom *= item->parent() ? item->parent()->zoom() : 1.0;
    y += item->y();
  }
  return y;
}

bool Item::IsInGroup(Group* group) const {  
  const Group* p = parent();
  while (p) {
    if (group == p) return true;
    p = p->parent();
  }
  return false;
}

Group::Group() : Item(),  widget_(0) { Init(); }
Group::Group(Canvas* widget) : Item(), widget_(widget) { Init(); }
Group::Group(Group* parent, double x, double y) : Item(parent), widget_(0) {
  Init();
}

Group::~Group() {  
  if (widget_ && is_root_ && !managed()) {
    widget_->root_ = 0;
  }
  for (iterator it = items_.begin(); it != items_.end(); ++it) {
    Item* item = *it;
    if (item->managed()) {
      delete item;
    } else {
      item->set_parent(0);
    }
  }
}

void Group::Init() {
  zoom_ = 1;
  is_root_ = false;  
}

void Group::Draw(Graphics* g, Region& repaint_rgn, Canvas* canvas) {
  for (iterator it = items_.begin(); it != items_.end(); ++it ) {
    Item* item = *it;
    if (!item->visible()) continue;
    std::auto_ptr<Region> item_rgn(item->region().Clone());    
    double dx, dy;
    dx = dy = 0;
    if (item->parent()) {
      dx = item->zoomabsx();
      dy = item->zoomabsy();
      item_rgn->Offset(dx, dy);
    }
    int erg = item_rgn->Combine(repaint_rgn, RGN_AND);
    if (erg != NULLREGION) {
      g->Translate(dx, dy);
      if (item->has_clip()) {
        std::auto_ptr<Region> tmp(item->clip().Clone());        
        tmp->Offset(dx, dy);
        item_rgn->Combine(*tmp, RGN_AND);
        g->SetClip(tmp.get());
      }           
      item->Draw(g, repaint_rgn, canvas);      
      if (item->has_clip()) {
        g->SetClip(0);
      }
      g->Translate(-dx, -dy);
    }
  }
}

void Group::Add(Item* item) {
  assert(item);
  if (item->parent()) {
    throw std::runtime_error("Item already child of a group.");
  }
  STR();
  item->set_parent(this);
  items_.push_back(item);  
  FLS();
  item->needsupdate();
}

void Group::Insert(iterator it, Item* item) {
  assert(item);
  assert(!item->parent());
  STR();
  item->set_parent(this);
  items_.insert(it, item);
  FLS();
}

void Group::Remove(Item* item) {
  assert(item);
  iterator it = find(items_.begin(), items_.end(), item);
  if (it == items_.end()) {
    throw std::runtime_error("Item is no child of the group");
  }
  STR();
  items_.erase(it);
  CheckButtonPress();
  CheckFocusItem();
  Canvas* c = canvas();
  if (c) {
    Item* out = c->out_item_;
    while (out) {
      if (out == item) {
        c->out_item_ = 0;
        break;
      }
      out = out->parent();
    }
  }
  item->set_parent(0);
  FLS();
}

void Group::Clear() {
  STR();
  for (iterator it = items_.begin(); it != items_.end(); ++it ) {
    Item* item = *it;
    if (item->managed()) delete item;
  }
  items_.clear();
  FLS();
}

void Group::onupdateregion() {  
  std::auto_ptr<Region> rgn(new mfc::Region());  
  std::vector<Item*>::const_iterator it = items_.begin();
  for ( ; it != items_.end(); ++it) {
    Item* item = *it;
    if (item->visible()) {
      std::auto_ptr<Region> tmp(item->region().Clone());      
      std::stringstream str;
      double x, y, w, h;
      item->GetBoundRect(x, y, w, h);
      str << "x" << x << ", y" << y << ",w" << w << ",h" << h;
      OutputDebugString(str.str().c_str());
      tmp->Offset(item->x(), item->y());        
      int nCombineResult = rgn->Combine(*tmp, RGN_OR);
      if (nCombineResult == NULLREGION) {
        rgn->Clear();
      }
    }
  }    
  rgn_.reset(rgn->Clone());
}

void Group::set_zorder(Item* item, int z) {
  assert(item);
  if (z<0 || z>=items_.size()) return;
  iterator it = find(items_.begin(), items_.end(), item);
  assert(it != items_.end());
  STR();
  items_.erase(it);
  items_.insert(begin()+z, item);
  FLS();
}

int Group::zorder(Item* item) const {
  int z = -1;
  for (int k = 0; k < items_.size(); k++) {
    if (items_[k] == item) {
      z = k;
      break;
    }
  }
  return z;
}

Item* Group::Intersect(double x, double y, Event* ev, bool& worked) {
  Item* found = 0;
  Item* found_noevent = 0;
  std::vector<Item*>::const_reverse_iterator rev_it = items_.rbegin();
  for ( ; rev_it != items_.rend(); ++rev_it) {
    Item* item = *rev_it;
    item = item->visible() ? item->Intersect(x-item->x(), y-item->y(), ev, worked) : 0;
    if (worked) return item;
    if (item) {
      if (work_item_event(item, ev)) {
        worked = true;
        return item;
      } else {
        if (!found_noevent) {
            found_noevent = item;
        }
        }
      }
    }
  if (found_noevent) {    
    Canvas* c = canvas();
    if (c) {
      found = c->DelegateEvent(ev, found_noevent);
      if (found) {
        worked = true;
        return found;
      }
    }
  } else if (ev->type() == Event::MOTION_NOTIFY) {
    Canvas* c = canvas();
    if (c && c->out_item_ && !c->button_press_item_ && c->out_item_->IsInGroup(this)) {        
      Event event(c->out_item_, Event::MOTION_OUT, ev->cx(), ev->cy(),ev->button(), ev->shift());
      c->DelegateEvent(&event, c->out_item_);      
      c->out_item_ = 0;      
      worked = true;      
      return 0;
    }
  }
  return found;
}

bool Group::work_item_event(Item* item, Event* ev) {
  Canvas* c = canvas();
  if (c) {
    if (ev->type() == Event::BUTTON_PRESS || ev->type() == Event::BUTTON_2PRESS) {
        c->button_press_item_ = item;
        c->focus_item_ = item;
    } else {
        if (ev->type() == Event::MOTION_NOTIFY ) {
          if (!c->button_press_item_ && c->out_item_ && item!=c->out_item_) { // mouseout
            OutputDebugString("out");
            Event event(c->out_item_, Event::MOTION_OUT, ev->cx(), ev->cy(),ev->button(), ev->shift());
            c->DelegateEvent(&event, c->out_item_);
          }
          c->out_item_ = item;
        }
    }
    Event e(item, ev->type(), ev->cx(), ev->cy(), ev->button(), ev->shift());
    bool erg = item->OnEvent(&e);
    return erg;
  }
  return 0;
}

void Group::OnMessage(CanvasMsg msg) {
  for (iterator it = items_.begin(); it != items_.end(); ++it ) {
    (*it)->OnMessage(msg);
  }
}


// Rect class
Rect::Rect(Group* parent, double x, double y, double width, double height)
  : Item(parent, x, y), width_(width), height_(height) {
    fillcolor_ = strokecolor_ = 0;
    bx_ = by_ = 0;
}

void Rect::Init() {
  width_ = height_ = bx_ = by_ = 0;
  fillcolor_ = strokecolor_ = 0;
}

void Rect::Draw(Graphics* g, Region& repaint_rgn, class Canvas* widget) {
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

void Line::Draw(Graphics* g, Region& repaint_region, Canvas* widget) {  
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

Item* Line::Intersect(double x, double y, Event* ev, bool &worked) {
  double distance_ = 5;
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
  Item* item = rgn.PtInRegion(x-this->x(),y-this->y()) ? this : 0;
  rgn.DeleteObject();
  return item;
}

void Line::onupdateregion() {  
  double x1, y1, x2, y2;
  double dist = 5;
  GetBoundRect(x1, y1, x2, y2);
  double zoom = parent() ? parent()->zoom() : 1.0;
  rgn_->SetRect((x1-dist)*zoom, (y1-dist)*zoom, (x2+2*dist+1)*zoom, (y2+2*dist+1)*zoom);    
}

Text::Text(Group* parent) : Item(parent) {
  Init(parent ? parent->zoom() : 1.0);
}

Text::Text(Group* parent, const std::string& text) : Item(parent), text_(text) {
  Init(parent ? parent->zoom() : 1.0);
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
    Canvas* c =  const_cast<Text*>(this)->canvas();
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

void Text::Draw(Graphics* g, Region& repaint_region, Canvas* widget) {    
    g->SetColor(color_);
    g->DrawString(text_, 0, 0);    
}

// PixBuf
Pic::Pic(Group* parent, double x, double y, Image* image)
  : Item(parent, x, y),
    image_(image) {
  Init();
  width_ = image->width();
  height_ = image->height();
  region();
}

void Pic::Init() {
  image_ = 0;
  width_ = height_ = xsrc_ = ysrc_ = 0;
  transparent_ = pmdone = false;
}

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

void Pic::Draw(Graphics* g, Region& repaint_region, Canvas* widget) {
  g->DrawImage(image_, 0, 0, width_, height_, xsrc_, ysrc_);
  // todo zoom  
}

void Pic::SetImage(Image* image) {
  STR();
  image_ = image;
  width_ = image_->width();
  height_ = image_->height();
  FLS();
}

void Pic::onupdateregion() {
  rgn_->SetRect(0, 0, width_, height_);    
}

// Canvas
void Canvas::Init() {
  save_ = true;
  steal_focus_ = managed_ = false;
  bg_image_ = 0;
  button_press_item_ = out_item_ = focus_item_ = 0;
  bg_width_ = bg_height_ = 0;
  cw_ = pw_ = ch_ = ph_ = 300;
  color_ = 0;
  save_rgn_.SetRect(0, 0, cw_, ch_);
  root_ = new Group(this);
  root_->is_root_ = true;
  root_->set_manage(false);
  cursor_ = LoadCursor(0, IDC_ARROW);
  old_wnd_ = wnd_;
  show_scrollbar = false;  
  nposv = nposh = 0;
}

Canvas::~Canvas() {
  if (managed_) {
    delete root_;
  } else if (root_) {
    root_->widget_ = 0;    
  }
}

void Canvas::DrawFlush(Graphics *g, Region& rgn) {
    save_rgn_.Combine(rgn, RGN_OR);
    Draw(g, save_rgn_);
    save_rgn_.Clear();
}

void Canvas::Draw(Graphics *g, Region& rgn) {
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
    double x, y, width, height;
    rgn.GetBoundRect(x, y, width, height);
    ARGB old_color = color();
    g->SetColor(color_);
    g->FillRect(x, y, width, height);
    g->SetColor(old_color);
  }
  root_->Draw(g, rgn, this);
}

void Canvas::StealFocus(Item* item) {
  button_press_item_ = item;
  steal_focus_ = true;
}

Item* Canvas::OnEvent(Event* ev) {
  if (ev->type() == Event::ONTIMER || ev->type() == Event::SCROLL || ev->type() == Event::ONSIZE) {
    return 0;
  }
  Item* item = button_press_item_;  
  if (item && ev->type() == Event::KEY_DOWN || item && ev->type() == Event::KEY_UP) {
    if (focus_item_) {
      Event e(focus_item_, ev->type(), ev->cx(), ev->cy(), ev->button(), ev->shift());
      DelegateEvent(&e, focus_item_);
      item = e.item();
    }
  } else
  if (item && ev->type() == Event::MOTION_NOTIFY ) {
      Event e(item, ev->type(), ev->cx(), ev->cy(), ev->button(), ev->shift());
      DelegateEvent(&e, item);
      item = e.item();
  } else
  if (ev->type() != Event::BUTTON_RELEASE) {
      if (ev->type() == Event::BUTTON_PRESS) {
         int fordebugonly = 0;
      }
      bool worked = false;
      item = root_->Intersect(ev->cx(), ev->cy(), ev, worked);
  }
  if (ev->type() == Event::BUTTON_RELEASE || ev->type() == Event::BUTTON_2PRESS) {
      ev->setitem(item);
      button_press_item_ = 0;
      if (item) item->OnEvent(ev);
  }
  return item;
}

Item* Canvas::DelegateEvent(Event* ev, Item* item) {
  bool erg = item->pointerevents() && item->OnEvent(ev);
  while (!erg && item->parent()) {
    item = item->parent();  // redirect event to parent
    if (button_press_item_) {
      button_press_item_ = item;
    }
    ev->setitem(item);
    erg = item->pointerevents() && item->OnEvent(ev);
    if (steal_focus_) {
      steal_focus_ = 0;
      return item;
    }
  }
  return erg ? item : 0;
}

void Canvas::Invalidate(Region& rgn) { 
  if (IsSaving()) {
    save_rgn_.Combine(rgn, RGN_OR);
  } else
    if (wnd_) {
      wnd_->InvalidateRgn((CRgn*)rgn.source(), 0);
    }
}

void Canvas::Flush() {
  if (wnd_) {
    RedrawWindow(wnd_->m_hWnd, NULL, *((CRgn*) save_rgn_.source()), RDW_INVALIDATE | RDW_UPDATENOW);
    // wnd_->InvalidateRgn((CRgn*) save_rgn_.source(), 0);
  }
  save_rgn_.Clear();
}


// CanvasView

bool BaseView::DelegateEvent(int type, int button, UINT nFlags, CPoint pt) {
  if (canvas()) {     
    try {             
      Event ev(0, (Event::Type)type, pt.x, pt.y, button, nFlags);
      return canvas()->OnEvent(&ev);
    } catch (std::exception& e) {
      AfxMessageBox(e.what());
    }    
  }
  return false;
}

void BaseView::Draw() {  
    CRgn pRgn;
	  pRgn.CreateRectRgn(0, 0, 0, 0);
	  wnd_->GetUpdateRgn(&pRgn, FALSE);
    CPaintDC dc(wnd_);
	  CRect rc;
	  wnd_->GetClientRect(&rc);
	  CDC bufferDC;
	  CBitmap bufferBmp;
	  bufferDC.CreateCompatibleDC(&dc);
	  bufferBmp.CreateCompatibleBitmap(&dc, rc.right-rc.left, rc.bottom-rc.top);
    CBitmap* oldbmp = bufferDC.SelectObject(&bufferBmp);		
    if (canvas()) {
      try {
        ui::mfc::Graphics g(&bufferDC);
        ui::mfc::Region canvas_rgn(pRgn);
        canvas()->DrawFlush(&g, canvas_rgn);
      } catch (std::exception& e) {
        AfxMessageBox(e.what());
      }
    }
    dc.BitBlt(0, 0, rc.right, rc.bottom, &bufferDC, 0, 0, SRCCOPY);
    bufferDC.SelectObject(oldbmp);	
	  bufferDC.DeleteDC();  
}

BEGIN_MESSAGE_MAP(View, CWnd)
  ON_WM_TIMER()
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
  ON_CONTROL_RANGE(BN_CLICKED, ID_DYNAMIC_CONTROLS_BEGIN, 
            ID_DYNAMIC_CONTROLS_LAST, OnCtrlBtnClick)
END_MESSAGE_MAP()


BOOL View::PreCreateWindow(CREATESTRUCT& cs) 
		{
			if (!CWnd::PreCreateWindow(cs))
				return FALSE;
			
			cs.dwExStyle |= WS_EX_CLIENTEDGE;
			cs.style &= ~WS_BORDER;
			cs.lpszClass = AfxRegisterWndClass
				(
					CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
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
	return 0;
}

void View::OnDestroy() { StopTimer(); }

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

void View::OnPaint() {
  Draw();
}

void View::OnTimer(UINT_PTR nIDEvent) {
  if (canvas() && IsWindowVisible()) {
    try {    
      if (canvas()->show_scrollbar) {
        ShowScrollBar(SB_BOTH, TRUE);
        SCROLLINFO si;
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_PAGE | SIF_RANGE;
				si.nMin = 0;
				si.nMax = canvas()->nposv; // -VISLINES;
				si.nPage = 1;
				SetScrollInfo(SB_VERT,&si);
        si.nMax = canvas()->nposh; // -VISLINES;
        SetScrollInfo(SB_HORZ, &si);
        canvas()->show_scrollbar = false;
      }
      canvas()->set_wnd(this);
      canvas()->InvalidateSave();
      this->DelegateEvent(Event::ONTIMER, 0, 0, CPoint(0, 0));
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
   if (canvas()) {
    SetCursor(canvas()->cursor());
    return TRUE;
   }
  return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void View::OnSize(UINT nType, int cx, int cy) {  
  if (canvas()) {
    try {
      canvas()->OnSize(cx, cy);
      canvas()->ClearSave();
      Invalidate();
    } catch (std::exception& e) {
      AfxMessageBox(e.what());
    }  
  }
  CWnd::OnSize(nType, cx, cy);
}




// CanvasFrame
IMPLEMENT_DYNAMIC(CanvasFrameWnd, CFrameWnd)

BEGIN_MESSAGE_MAP(CanvasFrameWnd, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()  
END_MESSAGE_MAP()

int CanvasFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)  {		
  if( CFrameWnd::OnCreate(lpCreateStruct) == 0) {
    pView_ = new View();
    pView_->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
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

void CanvasFrameWnd::OnClose() {      			
  OnEvent(1);
	// CFrameWnd::OnClose();
}

void CanvasFrameWnd::OnDestroy() {    
	/*HICON _icon = GetIcon(false);
	DestroyIcon(_icon);*/
  if (pView_ != NULL) { 
    pView_->DestroyWindow();
    delete pView_;
  }  
}

void CanvasFrameWnd::PostNcDestroy() {			
	CFrameWnd::PostNcDestroy();
}

void CanvasFrameWnd::PostOpenWnd() {
	pView_->Open();
	ResizeWindow(0);
}

void CanvasFrameWnd::GetWindowSize(CRect &rcFrame, CRect &rcClient, CRect *pRect) {
  rcClient.top = 0; rcClient.left = 0;
	rcClient.right = 400; rcClient.bottom = 300;
	rcFrame = rcClient;			
}

void CanvasFrameWnd::ResizeWindow(CRect *pRect) {
	CRect rcEffFrame,rcEffClient,rcTemp,tbRect;
	GetWindowSize(rcEffFrame, rcEffClient, pRect);
	SetWindowPos(NULL,0,0,rcEffFrame.right-rcEffFrame.left,rcEffFrame.bottom-rcEffFrame.top,SWP_NOZORDER | SWP_NOMOVE);
	pView_->SetWindowPos(NULL,rcEffClient.left, rcEffClient.top, rcEffClient.right,rcEffClient.bottom,SWP_NOZORDER);	
}

// Messages
void CanvasFrameWnd::OnSetFocus(CWnd* pOldWnd) {
	CFrameWnd::OnSetFocus(pOldWnd);			
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