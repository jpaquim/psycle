// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#include <psycle/host/detail/project.private.hpp>
#include "Canvas.hpp"
#include "CanvasItems.hpp"
#include <algorithm> // for std::transform
#include <cctype> // for std::tolower

namespace psycle {
namespace host  {
namespace ui {
namespace canvas {

Item::Ptr Item::nullpointer;

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

void Item::PreventFls() {
  Canvas* c = root();
  if (c) {  
    c->prevent_fls_ = true;
  }
}

void Item::EnableFls() {
  Canvas* c = root();
  if (c) { 
    c->prevent_fls_ = false;
  }
}

void Item::SetSize(double w, double h) {
  STR();
  //PreventFls(); 
  OnSize(w, h);     
  //EnableFls();
  FLS();
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

void Item::UpdateAlign() {
  ui::Dimension dim;  
  CalcDimension(dim);
  Canvas* c = root();
  bool old_save(false);
  if (c) {
    old_save = c->save_;
    c->SetSave(true);
  }
  DoAlign();
  if (c) {    
    c->SetSave(old_save);
    c->Flush();
  }    
}

double Item::zoomabsx() const {
  std::vector<Item::ConstPtr> items;
  Item::ConstWeakPtr p = parent();
  while (!p.expired()) {
    items.push_back(p.lock());
    p = p.lock()->parent();
  }
  double x = x_;
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
  Item::ConstWeakPtr p = parent();
  while (!p.expired()) {
    items.push_back(p.lock());
    p = p.lock()->parent();
  }
  double y = y_;
  // double zoom = 1.0;
  std::vector<Item::ConstPtr>::reverse_iterator rev_it = items.rbegin();
  for ( ; rev_it != items.rend(); ++rev_it) {
    const Item::ConstPtr item = *rev_it;
    //zoom *= item->parent() ? item->parent()->zoom() : 1.0;
    y += item->y();
  }
  return y;
}

bool Item::IsInGroupVisible() const {
  bool res = visible();
  Item::ConstWeakPtr p = parent();
  while (!p.expired()) {
    res = p.lock()->visible();
    if (!res) {
      break;   
    }
    p = p.lock()->parent();
  }
  return res;
}

void Item::Show() { 
  if (!visible_) { 
    STR();     
    visible_ = true;       
    OnMessage(SHOW);        
    FLS();   
  }
  //  Canvas* c = root();
  // if (c) {
  // ui::Dimension dim;
  // c->CalcDimension(dim);
  // c->DoAlign();
  // } 
}

void Item::Hide() { 
  if (visible_) {
    STR(); 
    visible_ = false;    
    OnMessage(HIDE);
    FLS();
  } 
}

void Item::DrawBackground(Graphics* g, Region& draw_region) {
  int alpha = GetAlpha(fill_color());
  if (alpha != 0xFF) {
    g->SetColor(fill_color());
    g->FillRegion(*rgn_);
  }  
}


void Group::CalcDimension(ui::Dimension& dim) {
  double w(0);
  double h(0);  
  double left(0);
  double right(0);
  double top(0);
  double bottom(0);
  int bitmask = ALCLIENT | ALLEFT | ALRIGHT | ALTOP | ALBOTTOM;
  for (iterator i = items_.begin(); i != items_.end(); ++i) {
    Item::Ptr item = *i;
    if (!item->visible()) {
      continue;
    }
    AlignStyle item_style = item->has_style() ? item->style()->align() : ALNONE;
    ui::Dimension idim;    
    item->CalcDimension(idim);    
    // width
    {
      double diff = right - left;
      switch (item_style & bitmask) {
        case ALLEFT:
          left += item->dim_.width();
          if (diff == 0) {          
            w += item->dim_.width();
            right += item->dim_.width();
          } else {
            double expand = item->dim_.width() - diff;
            if (expand > 0) {
              w += expand;
              right += expand;
            }
          }
        break;
        case ALTOP:
        case ALBOTTOM:
          if (diff == 0) {
            w += item->dim_.width();
            right += item->dim_.width();
          } else {
            double expand = item->dim_.width() - diff;
            if (expand > 0) {
              w += expand;
              right += expand;
            }
          }      
        break;
        case ALRIGHT:        
          if (diff == 0) {          
            w += item->dim_.width();
            right += item->dim_.width();
          } else {
            double expand = item->dim_.width() - diff;
            if (expand > 0) {
              w += expand;
              right += expand;
            } else
            if (expand == 0) {
              right -= item->dim_.width();
            }
          }
        break;      
        default:
          // w = std::max(item->dim_.width(), w);
        break;
      }    
    }

    // height
    {
      double diff = bottom - top;
      switch (item_style & bitmask) {
        case ALTOP:
          top += item->dim_.height();
          if (diff == 0) {          
            h += item->dim_.height();
            bottom += item->dim_.height();
          } else {
            double expand = item->dim_.height() - diff;
            h += expand;
            bottom += expand;            
          }
        break;
        case ALLEFT:
        case ALRIGHT:
          if (diff == 0) {
            h += item->dim_.height();
            bottom += item->dim_.height();
          } else {
            double expand = item->dim_.height() - diff;
            if (expand > 0) {
              h += expand;
              bottom += expand;
            }
          }      
        break;
        case ALBOTTOM:
          if (diff == 0) {          
            h += item->dim_.height();
            bottom += item->dim_.height();
          } else {
            double expand = item->dim_.height() - diff;
            if (expand > 0) {
              h += expand;
              bottom += expand;
            }
          }
        break;            
        default:
          h = std::max(item->dim_.height(), h);
        break;
      }    
    }
  } // end loop 
  if (has_style()) {
    const Margin& margin = style()->margin();
    w = w + margin.left() + margin.right();
    h = h + margin.top() + margin.bottom();
  }
  dim.set_size(w, h);  
  dim_ = dim;
}

void Group::DoAlign() {
  Item::Ptr client;
  double left(0);
  double right(width());
  double top(0);
  double bottom(height());
  for (iterator i = items_.begin(); i != items_.end(); ++i) {
    Item::Ptr item = *i;
    if (!item->visible()) continue;
    AlignStyle item_style = item->has_style() ? item->style()->align() : ALNONE;    
    Margin margin;
    if (item->has_style()) {
      margin = item->style()->margin();
    }
    double margin_w = margin.left() + margin.right();
    double margin_h = margin.top() + margin.bottom();
    int bitmask = ALCLIENT | ALLEFT | ALRIGHT | ALTOP | ALBOTTOM;
    switch (item_style & bitmask) {
      case ALCLIENT:
        client = item;
      break;      
      case ALLEFT:
        item->SetPos(left + margin.left(), top + margin.top(), item->dim_.width() - margin_w, bottom - top - margin_h); 
        left += item->dim_.width();
      break;
      case ALRIGHT:
        item->SetPos(right - item->dim_.width() + margin.right(), top + margin.top(), item->dim_.width() - margin_w, bottom - top - margin_h); 
        right -= item->dim_.width();
      break;
      case ALTOP:
        {
          double h = (item_style & ALFIXED) ? item->height() : item->dim_.height();
          item->SetPos(left + margin.left(), top + margin.top(),  right - left - margin_w, h - margin_h); 
          top += h;
        }
      break;
      case ALBOTTOM:
        {
          double h = (item_style & ALFIXED) ? item->height() : item->dim_.height();
          item->SetPos(left + margin.left(), bottom - h + margin.top(), right - left - margin_w, h - margin_h); 
          bottom -= h;
        }
      break;
      default:
      break;
    } // end switch  
    if (item_style != ALCLIENT) {
      item->DoAlign();
    }
  } // end loop 
  if (client) {
    client->SetPos(left, top,  right - left, bottom - top); 
    client->DoAlign();
  }
}

void Group::OnSize(double cw, double ch) {  
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
          item->DrawBackground(g, tmp);
          item->Draw(g, tmp);
          g->SetClip(0);
        }
        item->draw_rgn_ = &tmp;          
        g->SetClip(&tmp);
        item->DrawBackground(g, tmp);
        item->Draw(g, tmp);
        g->SetClip(0);
        item->blit_.reset(0);
        item->draw_rgn_ = 0;
        root()->item_blit_ = false;
      } else {
        item->draw_rgn_ = item_rgn.get();
        item->DrawBackground(g, *item_rgn);        
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

bool Group::onupdateregion() {  
  std::auto_ptr<Region> rgn(new mfc::Region());  
  std::vector<Item::Ptr>::const_iterator it = items_.begin();
  bool first = true;
  for ( ; it != items_.end(); ++it) {
    Item::Ptr item = *it;  
    if (item->visible()) {
      std::auto_ptr<Region> tmp(item->region().Clone());      
      tmp->Offset(item->x(), item->y());        
      int nCombineResult = rgn->Combine(*tmp, RGN_OR);
      if (nCombineResult == NULLREGION) {
        rgn->Clear();
        first = true;
      } else if (first) {
        rgn->Offset(-item->x(), -item->y());
        first = false;
      }     
    }
  }   
  if (w_ != -1) {
    double x, y, w, h;
    rgn->BoundRect(x, y, w, h);
    mfc::Region rgn1(x, y, w_, h); 
    if (w > w_) {
      rgn->Combine(rgn1, RGN_AND);
      rgn->w_cache_ = w_;      
    } else {
      rgn->Combine(rgn1, RGN_OR);
      rgn->w_cache_ = w_;     
    }
  }
  if (h_ != -1) {
    double x, y, w, h;
    rgn->BoundRect(x, y, w, h);
    mfc::Region rgn1(x, y, w, h_); 
    if (h > h_) {
      rgn->Combine(rgn1, RGN_AND);
      rgn->h_cache_ = h_;
    } else {
      rgn->Combine(rgn1, RGN_OR);      
      rgn->h_cache_ = h_;     
    }
  }
  rgn_.reset(rgn->Clone());
  
  return true;
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

Item::Ptr Group::HitTest(double x, double y) {
  Item::Ptr result;
  ItemList::const_reverse_iterator rev_it = items_.rbegin();
  for (; rev_it != items_.rend(); ++rev_it) {
    Item::Ptr item = *rev_it;
    item = item->visible() 
           ? item->HitTest(x-item->x(), y-item->y())
		       : nullpointer; 
    if (item) {
      result = item;
      break;
    }
  }
  return result;
}

void Group::OnMessage(CanvasMsg msg) {
  for (iterator it = items_.begin(); it != items_.end(); ++it ) {
    (*it)->OnMessage(msg);
  }
}

// Canvas
void Canvas::Init() {
  is_root_ = true;
  steal_focus_ = item_blit_ = show_scrollbar = save_ = prevent_fls_ = false;
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



void Canvas::OnSize(double cw, double ch) {  
  cw_ = cw;
  ch_ = ch;
  needsupdate(); 
  UpdateAlign();
  Group::OnSize(cw, ch);
}

void Canvas::StealFocus(const Item::Ptr& item) {
  button_press_item_ = item;
  steal_focus_ = true;
}

void Canvas::SetFocus(const Item::Ptr& item) {
  if (item != focus_item_.lock()) {
    if (!focus_item_.expired() && item != focus_item_.lock()) {
      focus_item_.lock()->OnKillFocus();
    }
    focus_item_ = item;
    if (!focus_item_.expired()) {
      focus_item_.lock()->OnFocus();
    }
  }
}

// Events

bool Canvas::WorkKeyDown(KeyEvent& ev) {
  if (!focus_item_.expired()) {
    Item::Ptr item = focus_item_.lock();   
    WorkEvent(ev, &Item::OnKeyDown, item);    
  } else {
    ev.PreventDefault();
    OnKeyDown(ev);
  }  
  return !ev.is_work_parent();
}

void Canvas::WorkKeyUp(KeyEvent& ev) {
  if (!focus_item_.expired()) {
    Item::Ptr item = focus_item_.lock();
    WorkEvent(ev, &Item::OnKeyDown, item);;
  } else {
    OnKeyUp(ev);  
  }
}

void Canvas::WorkMouseDown(MouseEvent& ev) {
  button_press_item_ = HitTest(ev.cx(), ev.cy());  
  if (!button_press_item_.expired()) {        
    Item::Ptr item = button_press_item_.lock();    
    WorkEvent(ev, &Item::OnMouseDown, item);
    button_press_item_ = item;    
  }
  SetFocus(button_press_item_.lock());
}

void Canvas::WorkDblClick(MouseEvent& ev) {
  button_press_item_ = HitTest(ev.cx(), ev.cy());  
  if (!button_press_item_.expired()) {        
    Item::Ptr item = button_press_item_.lock();    
    WorkEvent(ev, &Item::OnDblclick, item);        
  }
  SetFocus(button_press_item_.lock());
  button_press_item_ = nullpointer;
}

void Canvas::WorkMouseUp(MouseEvent& ev) {
  if (!button_press_item_.expired()) {
    button_press_item_.lock()->OnMouseUp(ev);
    button_press_item_.reset();
  } else {
    Item::Ptr item = HitTest(ev.cx(), ev.cy());  
    WorkEvent(ev, &Item::OnMouseUp, item);
  }
}

void Canvas::WorkMouseMove(MouseEvent& ev) {
  if (!button_press_item_.expired()) {        
    Item::Ptr item = button_press_item_.lock();    
    WorkEvent(ev, &Item::OnMouseMove, item);
  } else {
    Item::Ptr item = HitTest(ev.cx(), ev.cy());    
    while (item) {
      if (!mouse_move_.expired()  &&
           mouse_move_.lock() != item
         ) {
        mouse_move_.lock()->OnMouseOut(ev);
      }       
      item->OnMouseMove(ev);
      if (ev.is_work_parent()) {        
        item = item->parent().lock();  
      } else {
        mouse_move_ = item;
        break;
      }
    }    
    if (!item && !mouse_move_.expired()) {
      mouse_move_.lock()->OnMouseOut(ev);
      mouse_move_.reset();
    }
  }
}

void Canvas::WorkOnSize(int cw, int ch) {
  OnSize(cw, ch);
}

void Canvas::WorkTimer() {
  // OnSize(cw, ch);
}

void Canvas::Invalidate(Region& rgn) { 
  if (!prevent_fls_) {
    if (IsSaving()) {
      save_rgn_.Combine(rgn, RGN_OR);
    } else if (wnd_) {
        //wnd_->InvalidateRgn((CRgn*)rgn.source(), 0);    
      RedrawWindow(wnd_->m_hWnd, NULL, *((CRgn*) rgn.source()), RDW_INVALIDATE | RDW_UPDATENOW);
    }
  }
}

void Canvas::Flush() {
  if (!prevent_fls_) {
    if (wnd_) {
      RedrawWindow(wnd_->m_hWnd, NULL, *((CRgn*) save_rgn_.source()), RDW_INVALIDATE | RDW_UPDATENOW);
    // wnd_->InvalidateRgn((CRgn*) save_rgn_.source(), 0);
    }
    save_rgn_.Clear();
  }
}

void Canvas::Invalidate() {
  if (!prevent_fls_) {
    if (wnd_) {
      wnd_->Invalidate();    
    }
  }
}  

void Canvas::InvalidateSave() {
  if (!prevent_fls_) {
    if (wnd_ &&::IsWindow(wnd_->m_hWnd)) {
      wnd_->InvalidateRgn((CRgn*) save_rgn_.source(), 0);
    }
  }
}

void Canvas::SetCapture() {
  if (wnd_ && ::IsWindow(wnd_->m_hWnd)) {
    ::SetCapture(wnd_->m_hWnd);
  }
}

void Canvas::ReleaseCapture() {
  if (wnd_ && ::IsWindow(wnd_->m_hWnd)) {
    ::ReleaseCapture();
  }
}

void Canvas::ShowCursor() { 
  while (::ShowCursor(TRUE) < 0);
}

void Canvas::HideCursor() { 
  while (::ShowCursor(FALSE) >= 0); 
}

void Canvas::SetCursorPos(int x, int y) {
  if (wnd_) {
    CPoint point(x, y);
    wnd_->ClientToScreen(&point);
		::SetCursorPos(point.x, point.y);
  }
}

void Canvas::SetCursor(CursorStyle style) {
  LPTSTR c = 0;
  int ac = 0;
  switch (style) {
    case AUTO        : c = IDC_IBEAM; break;
    case MOVE        : c = IDC_SIZEALL; break;
    case NO_DROP     : ac = AFX_IDC_NODROPCRSR; break;
    case COL_RESIZE  : c = IDC_SIZEWE; break;
    case ALL_SCROLL  : ac = AFX_IDC_TRACK4WAY; break;
    case POINTER     : c = IDC_HAND; break;
    case NOT_ALLOWED : c = IDC_NO; break;
    case ROW_RESIZE  : c = IDC_SIZENS; break;
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

// CanvasView

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
  ON_COMMAND_RANGE(ID_DYNAMIC_CONTROLS_BEGIN, ID_DYNAMIC_CONTROLS_LAST, OnCtrlCommand) 
END_MESSAGE_MAP()

void View::OnCtrlCommand(UINT id) {}

std::map<HWND, View*> View::views_;
std::map<std::uint16_t, Item::WeakPtr> View::mfc_ctrls_;

HHOOK View::_hook = 0;

LRESULT __stdcall View::HookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    CWPSTRUCT* info = (CWPSTRUCT*) lParam;        
    if (info->message == WM_SETFOCUS) {
      FilterHook(info->hwnd);                     
    }
  }
  return CallNextHookEx(_hook , nCode, wParam, lParam);       
}

void View::FilterHook(HWND hwnd) {  
  typedef std::map<HWND, View*> ViewList;
  ViewList::iterator it = views_.begin();
  for (; it != views_.end(); ++it) {
    View* view = (*it).second;
    if (::IsChild(view->GetSafeHwnd(), hwnd)) {
        view->OnFocusChange(::GetDlgCtrlID(hwnd));
    }
  } 
}

void View::SetFocusHook() {
  if (_hook) {
    return;
  }
  if (!(_hook = SetWindowsHookEx(WH_CALLWNDPROC, 
                                 View::HookCallback,
                                 AfxGetInstanceHandle(),
                                 GetCurrentThreadId()))) {
    TRACE(_T("ui::canvas::View : Failed to install hook!\n"));
  }
}

void View::ReleaseHook() { 
  UnhookWindowsHookEx(_hook);
}

void View::set_canvas(Canvas::WeakPtr canvas) { 
  if (canvas_.lock() == canvas.lock()) {
    return;
  }    
  if (!canvas_.expired()) {
    canvas_.lock()->set_wnd(0);
  }
  canvas_ = canvas;            
  if (!canvas.expired() && m_hWnd) {      
    Canvas* c = canvas.lock().get();
    c->set_wnd(this);
    try {   
      CRect rc;            
      GetClientRect(&rc);
      if (rc.Width() != 0 && rc.Height() != 0) {      
        c->OnSize(rc.Width(), rc.Height());
        c->ClearSave();
        Invalidate();
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }  
  } 
}        

BOOL View::PreTranslateMessage(MSG* pMsg) {
  if (pMsg->message==WM_KEYDOWN ) {
    CWnd* hwndTest = GetFocus();
    if (hwndTest) {
      int id = hwndTest->GetDlgCtrlID();
      Item::WeakPtr item = FindById(id);
      Canvas* c = canvas().lock().get();    
      if (c) {
        UINT nFlags = 0;
        UINT flags = Win32KeyFlags(nFlags);
        try {
          canvas::KeyEvent ev(pMsg->wParam, flags);
          c->WorkKeyDown(ev);
          return ev.is_prevent_default();
        } catch(std::exception& e) {
          AfxMessageBox(e.what());
        }
      }
    }          
  } else
  if (pMsg->message == WM_KEYUP) {      
    CWnd* hwndTest = GetFocus();
    if (hwndTest) {
      int id = hwndTest->GetDlgCtrlID();
      Item::WeakPtr item = FindById(id);
      Canvas* c = canvas().lock().get();    
      if (c) {
        UINT nFlags = 0;
        UINT flags = Win32KeyFlags(nFlags);
        try {
          canvas::KeyEvent ev(pMsg->wParam, flags);
          c->WorkKeyUp(ev);
          return ev.is_prevent_default();
        } catch(std::exception& e) {
          AfxMessageBox(e.what());
        }
      }          
    }
  }
  return CWnd::PreTranslateMessage(pMsg);
}


BOOL View::PreCreateWindow(CREATESTRUCT& cs) {
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
  views_[GetSafeHwnd()] = this;
   // Set the hook
  SetFocusHook();  
	return 0;
}

void View::OnDestroy() {  
  bmpDC.DeleteObject();
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
  if (!canvas_.expired() && m_hWnd && IsWindowVisible()) {
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
      c->WorkTimer();
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
//      DelegateEvent(canvas::Event::SCROLL, 0, 0, pt);
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
      //DelegateEvent(canvas::Event::SCROLL, 0, 0, pt);
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


void View::OnPaint() {
  CRgn rgn;
  rgn.CreateRectRgn(0, 0, 0, 0);
	int result = GetUpdateRgn(&rgn, FALSE);

  if (!result) return; // If no area to update, exit.
	
	CPaintDC dc(this);

  if (!bmpDC.m_hObject) { // buffer creation	
		CRect rc;
		GetClientRect(&rc);		
		bmpDC.CreateCompatibleBitmap(&dc, rc.right - rc.left, rc.bottom - rc.top);
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
  GetClientRect(&rc);
	dc.BitBlt(0, 0, rc.right-rc.left, rc.bottom-rc.top, &bufDC, 0, 0, SRCCOPY);
	bufDC.SelectObject(oldbmp);
	bufDC.DeleteDC();
  rgn.DeleteObject();    
}

void View::OnSize(UINT nType, int cw, int ch) {  
  if (bmpDC.m_hObject != NULL) { // remove old buffer to force recreating it with new size
	  TRACE("CanvasView::OnResize(). Deleted bmpDC\n");
	  bmpDC.DeleteObject();	  
  }
   if (!canvas_.expired()) {      
    try {
      Canvas* c = canvas().lock().get();
      c->WorkOnSize(cw, ch);
      c->ClearSave();
      Invalidate();
    } catch (std::exception& e) {
      AfxMessageBox(e.what());
    }  
  } 
  CWnd::OnSize(nType, cw, ch);
}

IMPLEMENT_DYNAMIC(FrameAdaptee, CFrameWnd)

BEGIN_MESSAGE_MAP(FrameAdaptee, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
  ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()  
END_MESSAGE_MAP()

int FrameAdaptee::OnCreate(LPCREATESTRUCT lpCreateStruct)  {		
  if( CFrameWnd::OnCreate(lpCreateStruct) == 0) {
    pView_ = new View();
    pView_->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL);    
    return 0;
  }       
  return -1;		
}
    
BOOL FrameAdaptee::PreCreateWindow(CREATESTRUCT& cs) {
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
			
  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	return TRUE;
}

void FrameAdaptee::OnClose() { 
 try {
   frame_->OnFrameClose();
  } catch (std::exception& e) {
    AfxMessageBox(e.what());
  }    
}

void FrameAdaptee::OnDestroy() {    
	/*HICON _icon = GetIcon(false);
	DestroyIcon(_icon);*/
  if (pView_ != NULL) { 
    pView_->DestroyWindow();
    delete pView_; pView_ = 0;
  }  
}

} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle