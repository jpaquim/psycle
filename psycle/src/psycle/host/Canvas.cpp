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

void Item::Detach() { 
  if (!parent_.expired()) {
    parent_.lock()->Remove(shared_from_this());
  }
}

Window* Item::root() {  
  Window::Ptr item = shared_from_this();
  do {
    if (item->is_root()) return item.get();
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
  Canvas* c = (Canvas*) root();
  if (c) {  
    c->prevent_fls_ = true;
  }
}

void Item::EnableFls() {
  Canvas* c = (Canvas*) root();
  if (c) { 
    c->prevent_fls_ = false;
  }
}

void Item::SetSize(double w, double h) {
  STR();
  OnSize(w, h);
  FLS();
}

void Item::set_ornament(boost::shared_ptr<ui::Ornament> ornament) {
  ornament_ = ornament;
  if (ornament) {
    std::auto_ptr<ui::Rect> opad = ornament->padding();
    ui::Rect pad = style()->padding();
    pad.set(pad.left() + opad->left(),
            pad.top() + opad->top(),
            pad.right() + opad->right(),
            pad.bottom() + opad->bottom());
    style()->set_padding(pad);
  } else if (has_style()) { 
    style()->set_padding(ui::Rect());
  }
}

void Item::GetFocus() {
  Canvas* c = (Canvas*) root();
  if (c) {
    c->StealFocus(shared_from_this());
  }
}

void Item::STR() {  
  fls_rgn_.reset(region().Clone());
  fls_rgn_->Offset(pos().left(), pos().top());
  has_store_ = true;
}

void Item::FLS() {
  if (has_store_) {
    needsupdate();
    std::auto_ptr<Region> tmp(region().Clone());
    tmp->Offset(pos().left(), pos().top());
    fls_rgn_->Combine(*tmp, RGN_OR);
    has_store_ = false;    
  } else {
    STR();
    has_store_ = false;
  }
  if (visible() && !parent().expired()) {
    Window* canvas = root();
    if (canvas) {
      canvas->Invalidate(*fls_rgn_);
    }
  }
}

void Item::FLS(const Region& rgn) {
  std::auto_ptr<Region> tmp(rgn.Clone());
  tmp->Combine(region(), RGN_AND);
  tmp->Offset(pos().left(), pos().top());
  STR(); has_store_ = false;
  fls_rgn_->Combine(*tmp, RGN_AND);
  if (!parent().expired()) {
    Window* canvas = root();
    if (canvas) {
      canvas->Invalidate(*fls_rgn_);
    }
  }  
}

void Item::SetBlitXY(double x, double y) {    
  BlitInfo* bi = new BlitInfo();    
  bi->dx = x - x_;
  bi->dy = y - y_;    
  blit_.reset(bi);  
  x_ = x; y_ = y;
  Canvas* c = (Canvas*) root();
  if (c) {
    c->item_blit_ = true;
  }
  FLS();
}

ui::Rect Item::abs_pos() const {    
  std::vector<Window::ConstPtr> items;
  Window::ConstWeakPtr p = parent();
  while (!p.expired()) {
    items.push_back(p.lock());
    p = p.lock()->parent();
  }
  double x = x_;
  double y = y_; 
  std::vector<Window::ConstPtr>::const_reverse_iterator rev_it = items.rbegin();
  for ( ; rev_it != items.rend(); ++rev_it) {
    Item::ConstPtr item = *rev_it;    
    x += item->pos().left();
    y += item->pos().top();
  }
  return ui::Rect(x, y, x + dim().width(), y + dim().height());
}

bool Item::IsInGroupVisible() const {
  bool res = visible();
  Window::ConstWeakPtr p = parent();
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
  if (draw_region.bounds().height() > 0) {
    ARGB color = fill_color();
    int alpha = GetAlpha(color);
    if (alpha != 0xFF) {
      g->SetColor(color);
      g->FillRegion(*rgn_);
    } 
    if (!ornament_.expired()) { 
      Item::Ptr t = shared_from_this();
      ornament_.lock()->Draw(t, g, draw_region);
    }
  }
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
      dx = item->abs_pos().left();
      dy = item->abs_pos().top();
      item_rgn->Offset(dx, dy);      
    }
    int erg = item_rgn->Combine(draw_region, RGN_AND);    
    if (erg != NULLREGION) {      
      g->SaveOrigin();
      ui::Rect pad;
/*      if (item->has_style()) {
        pad = item->style()->padding();
      }*/
      g->Translate(item->pos().left(), item->pos().top());
/*      if (item->has_clip()) {        
        g->SetClip(item_rgn.get());
      }*/
      /*if (item->blit_.get()) {
        ui::Rect bounds = item_rgn->bounds();          
        std::auto_ptr<ui::Region> tmp(ui::Systems::instance().CreateRegion());
        if (item->blit_->dy != 0) {
          g->CopyArea(-item->x(), -item->y(), bounds.width(), bounds.height(), 0, item->blit_->dy);
          if (item->blit_->dy > 0) {
            tmp->SetRect(bounds.left(), bounds.top(), bounds.width(), item->blit_->dy);
          } else {            
            tmp->SetRect(bounds.left(), bounds.top()+item->blit_->dy, bounds.width(), -item->blit_->dy);
          }          
          item->draw_rgn_ = tmp.get();          
          g->SetClip(tmp.get());
          item->Draw(g, *tmp);
          g->SetClip(0);
        }
        if (item->blit_->dx != 0) {
          g->CopyArea(-item->x(), -item->y(), bounds.width(), bounds.height(), item->blit_->dx, 0);
          if (item->blit_->dx > 0) {
            tmp->SetRect(bounds.left(), bounds.top(), item->blit_->dx, bounds.height());              
          } else {            
            tmp->SetRect(bounds.left()+bounds.width()+item->blit_->dx, bounds.left(), -item->blit_->dx, bounds.height());              
          }          
          item->draw_rgn_ = tmp.get();          
          g->SetClip(tmp.get());
          item->DrawBackground(g, *tmp);
          item->Draw(g, *tmp);
          g->SetClip(0);
        }
        item->draw_rgn_ = tmp.get();          
        g->SetClip(tmp.get());
        item->DrawBackground(g, *tmp);        
        item->Draw(g, *tmp);       
        g->SetClip(0);
        item->blit_.reset(0);
        item->draw_rgn_ = 0;
        root()->item_blit_ = false;
      } else {       */
      //  item->draw_rgn_ = item_rgn.get();
        item->DrawBackground(g, *item_rgn);        
        item->Draw(g, *item_rgn);
      //  item->draw_rgn_ = 0;
      //}
      /*if (item->has_clip()) {
        g->SetClip(0);
      }*/
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

bool Group::onupdateregion() {  
  std::auto_ptr<Region> rgn(ui::Systems::instance().CreateRegion());
  std::vector<Item::Ptr>::const_iterator it = items_.begin();
  bool first = true;
  for ( ; it != items_.end(); ++it) {
    Item::Ptr item = *it;  
    if (item->visible()) {
      std::auto_ptr<Region> tmp(item->region().Clone());      
      tmp->Offset(item->pos().left(), item->pos().top());        
      int nCombineResult = rgn->Combine(*tmp, RGN_OR);
      if (nCombineResult == NULLREGION) {
        rgn->Clear();
        first = true;
      } else if (first) {
        rgn->Offset(-item->pos().left(), -item->pos().top());
        first = false;
      }     
    }
  }      
  if (auto_size_width()) {
    ui::Rect bounds = rgn->bounds();
    std::auto_ptr<Region> rgn1(ui::Systems::instance().CreateRegion());
    rgn1->SetRect(bounds.left(), bounds.top(), w_, bounds.height()); 
    if (bounds.width() > w_) {
      rgn->Combine(*rgn1, RGN_AND);
      rgn->w_cache_ = w_;      
    } else {
      rgn->Combine(*rgn1, RGN_OR);
      rgn->w_cache_ = w_;     
    }
  }
  if (auto_size_height()) {
    ui::Rect bounds = rgn->bounds();
    std::auto_ptr<Region> rgn1(ui::Systems::instance().CreateRegion());
    rgn1->SetRect(bounds.left(), bounds.top(), bounds.width(), h_); 
    if (bounds.height() > h_) {
      rgn->Combine(*rgn1, RGN_AND);
      rgn->h_cache_ = h_;
    } else {
      rgn->Combine(*rgn1, RGN_OR);      
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

Window::Ptr Group::HitTest(double x, double y) {
  Window::Ptr result;
  Window::List::const_reverse_iterator rev_it = items_.rbegin();
  for (; rev_it != items_.rend(); ++rev_it) {
    Window::Ptr item = *rev_it;
    item = item->visible() 
           ? item->HitTest(x-item->pos().left(), y-item->pos().top())
		       : nullpointer; 
    if (item) {
      result = item;
      break;
    }
  }
  return result;
}

void Group::OnMessage(WindowMsg msg, int param) {
  for (iterator it = items_.begin(); it != items_.end(); ++it ) {
    (*it)->OnMessage(msg, param);
  }
}

void Group::set_aligner(const boost::shared_ptr<Aligner>& aligner) { 
  aligner_ = aligner;
  aligner_->set_group(boost::dynamic_pointer_cast<Group>(shared_from_this()));
}
  
Window::List Aligner::dummy;

void Group::Align() { 
  Canvas* c = (Canvas*) root();
  bool old_save(false);
  if (c) {
    old_save = c->save_;
    c->SetSave(true);
  }
  if (aligner_) {
    aligner_->Align();
  }
  if (c) {    
    c->SetSave(old_save);
    c->Flush();
  }
}
// Aligner

bool Aligner::full_align_ = true;

void CalcDim::operator()(Aligner& aligner) const { aligner.CalcDimensions(); };
void SetPos::operator()(Aligner& aligner) const { aligner.SetPositions(); };

template<class T>
void Aligner::PreOrderTreeTraverse(T& functor) {
  Window* g = group_.lock().get();  
  // cache pos information  
  pos_ = g->pos(); //.set(g->pos().left(), g->y(), g->x() + g->width(), g->y() + g->height());
  functor(*this);
  for (iterator i = begin(); i != end(); ++i) {
    Window::Ptr item = *i;
    if (item->visible() && item->aligner()) {   
      Aligner::Ptr al = item->aligner();
      if (!item->aligner()->full_align() &&
          al->pos_.left() == item->pos().left() &&
          al->pos_.top() == item->pos().top() &&
          al->pos_.width() == item->pos().width() &&
          al->pos_.height() == item->pos().height()) {
        continue;
      }      
      al->PreOrderTreeTraverse(functor);    
    }
  }  
}

template<class T>
void Aligner::PostOrderTreeTraverse(T& functor) {  
  for (iterator i = begin(); i != end(); ++i) {
    Window::Ptr item = *i;
    if (item->visible() && item->aligner()) { 
      Aligner::Ptr al = item->aligner();
      if (!al->full_align() &&
          al->pos_.width() == item->pos().width() &&
          al->pos_.height() == item->pos().height()) {
        continue;
      }  
      al->PostOrderTreeTraverse(functor);
    }
  }
  functor(*this);  
}

// DefaultAligner
void DefaultAligner::CalcDimensions() {
  double w(0);
  double h(0);  
  double left(0);
  double right(0);
  double top(0);
  double bottom(0);
  int bitmask = ALCLIENT | ALLEFT | ALRIGHT | ALTOP | ALBOTTOM;
  for (iterator i = begin(); i != end(); ++i) {
    Item::Ptr item = *i;
    if (!item->visible()) {
      continue;
    }
    ui::Size item_dim = item->aligner() ? item->aligner()->dim() : item->dim();
    AlignStyle item_style = item->has_style() ? item->style()->align() : ALNONE;
    // width
    {
      double diff = right - left;
      switch (item_style & bitmask) {
        case ALLEFT:
          left += item_dim.width();
          if (diff == 0) {          
            w += item_dim.width();
            right += item_dim.width();
          } else {
            double expand = item_dim.width() - diff;
            if (expand > 0) {
              w += expand;
              right += expand;
            }
          }
        break;
        case ALTOP:
        case ALBOTTOM:
          if (diff == 0) {
            w += item_dim.width();
            right += item_dim.width();
          } else {
            double expand = item_dim.width() - diff;
            if (expand > 0) {
              w += expand;
              right += expand;
            }
          }      
        break;
        case ALRIGHT:        
          if (diff == 0) {          
            w += item_dim.width();
            right += item_dim.width();
          } else {
            double expand = item_dim.width() - diff;
            if (expand > 0) {
              w += expand;
              right += expand;
            } else
            if (expand == 0) {
              right -= item_dim.width();
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
          top += item_dim.height();
          if (diff == 0) {          
            h += item_dim.height();
            bottom += item_dim.height();
          } else {
            double expand = item_dim.height() - diff;
            h += expand;
            bottom += expand;            
          }
        break;
        case ALLEFT:
        case ALRIGHT:
          if (diff == 0) {
            h += item_dim.height();
            bottom += item_dim.height();
          } else {
            double expand = item_dim.height() - diff;
            if (expand > 0) {
              h += expand;
              bottom += expand;
            }
          }      
        break;
        case ALBOTTOM:
          if (diff == 0) {          
            h += item_dim.height();
            bottom += item_dim.height();
          } else {
            double expand = item_dim.height() - diff;
            if (expand > 0) {
              h += expand;
              bottom += expand;
            }
          }
        break;            
        default:
          h = std::max(item_dim.height(), h);
        break;
      }    
    }
  } // end loop 
  if (group_.lock()->has_style()) {
    const ui::Rect& margin = group_.lock()->style()->margin();    
    const ui::Rect& pad = group_.lock()->style()->padding();
    if (w > 0 && h > 0) {
      w += pad.left() + pad.right();
      h += pad.top() + pad.bottom();       
    }
    w = w + margin.left() + margin.right();    
    h = h + margin.top() + margin.bottom();     
  }
  dim_.set_size(w, h);  
}


void DefaultAligner::SetPositions() {
  if (group_.expired()) {
    return;
  }
  if (group_.lock()->region().bounds().empty()) {
    return;
  }
  Item::Ptr client;
  double left(0);
  double right(group_.lock()->pos().width());
  double top(0);
  double bottom(group_.lock()->pos().height());  

  for (iterator i = begin(); i != end(); ++i) {
    Item::Ptr item = *i;
    if (!item->visible()) continue;
    ui::Size item_dim = item->aligner() ? item->aligner()->dim() : item->dim();
    AlignStyle item_style = item->has_style() ? item->style()->align() : ALNONE;    
    ui::Rect margin;    
    if (item->has_style()) {
      margin = item->style()->margin();
    }
    ui::Rect pad;
    if (group_.lock()->has_style()) {
      pad = group_.lock()->style()->padding();
      margin.set(margin.left() + pad.left(),                 
                 margin.top() + pad.top(),
                 margin.right() - pad.right(),
                 margin.bottom() - pad.bottom());
    }
    double margin_w = margin.left() + margin.right();
    double margin_h = margin.top() + margin.bottom();
    int bitmask = ALCLIENT | ALLEFT | ALRIGHT | ALTOP | ALBOTTOM;
    switch (item_style & bitmask) {
      case ALCLIENT:
        client = item;
      break;      
      case ALLEFT:
        {
          double w = (item_style & ALFIXED) ? item->pos().width() + margin_w: item_dim.width();
          item->set_pos(ui::Rect(left + margin.left(), 
                                 top + margin.top(), 
                                 left + margin.left() + w - margin_w, 
                                 top + margin.top() + bottom - top - margin_h)); 
          left += w;
        }
      break;
      case ALRIGHT:
        item->set_pos(ui::Rect(right - item_dim.width() + margin.right(), 
                               top + margin.top(), 
                               right - item_dim.width() + margin.right() + item_dim.width() - margin_w, 
                               top + margin.top() + bottom - top - margin_h)); 
        right -= item_dim.width();
      break;
      case ALTOP:
        {
          double h = (item_style & ALFIXED) ? item->pos().height() : item_dim.height();
          item->set_pos(ui::Rect(left + margin.left(), 
                                 top + margin.top(),  
                                 left + margin.left() + right - left - margin_w, 
                                 top + margin.top() + h - margin_h)); 
          top += h;
        }
      break;
      case ALBOTTOM:
        {
          double h = (item_style & ALFIXED) ? item->pos().height() : item_dim.height();
          item->set_pos(ui::Rect(left + margin.left(), 
                                 bottom - h + margin.top(), 
                                 left + margin.left() + right - left - margin_w, 
                                 bottom - h + margin.top() + h - margin_h)); 
          bottom -= h;
        }
      break;
      default:
      break;
    } // end switch      
  } // end loop 
  
  if (client) {
    client->set_pos(ui::Rect(left, top, right, bottom)); 
  }
}

void DefaultAligner::Realign() {}

// Canvas
void Canvas::Init() {    
  is_root_ = true;
  steal_focus_ = item_blit_ = show_scrollbar = save_ = prevent_fls_ = false;
  bg_image_ = 0;  
  bg_width_ = bg_height_ = 0;
  cw_ = ch_ = 300;
  nposv = nposh = 0;
  color_ = 0;
  save_rgn_->SetRect(0, 0, cw_, ch_);    
  cursor_ = LoadCursor(0, IDC_ARROW); 
}

void Canvas::Draw(Graphics *g, Region& rgn) {
  if (IsSaving()) {
    save_rgn_->Combine(rgn, RGN_OR);
    DoDraw(g, *save_rgn_);
    save_rgn_->Clear();
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
      g->SetColor(color_);
      ui::Rect bounds = rgn.bounds();    
      g->FillRect(bounds.left(), bounds.top(), bounds.width(), bounds.height());
    }    
  }
  Group::Draw(g, rgn);
}

void Canvas::OnSize(double cw, double ch) {  
  try {
    cw_ = cw;
    ch_ = ch;
    needsupdate();    
    Align();  
    Group::OnSize(cw, ch);
  } catch (std::exception& e) {
    AfxMessageBox(e.what());
  }
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
  try {
    if (!focus_item_.expired()) {
      Window::Ptr item = focus_item_.lock();   
      WorkEvent(ev, &Window::OnKeyDown, item);    
    } else {
      ev.PreventDefault();
      OnKeyDown(ev);
    }
  } catch (std::exception& e) {
    AfxMessageBox(e.what());
    error(e);    
  }
  return !ev.is_work_parent();
}

void Canvas::WorkKeyUp(KeyEvent& ev) {
  try {
    if (!focus_item_.expired()) {
      Window::Ptr item = focus_item_.lock();
      WorkEvent(ev, &Window::OnKeyDown, item);;
    } else {
      OnKeyUp(ev);  
    }
  } catch (std::exception& e) {
    AfxMessageBox(e.what());
    error(e);    
  }
}

void Canvas::WorkMouseDown(MouseEvent& ev) {
  try { 
    button_press_item_ = HitTest(ev.cx(), ev.cy());  
    if (!button_press_item_.expired()) {        
      Window::Ptr item = button_press_item_.lock();    
      WorkEvent(ev, &Window::OnMouseDown, item);
      button_press_item_ = item;    
    }
    SetFocus(button_press_item_.lock());
  } catch (std::exception& e) {
    AfxMessageBox(e.what());
    error(e);    
  }
}

void Canvas::WorkDblClick(MouseEvent& ev) {
  try {
    button_press_item_ = HitTest(ev.cx(), ev.cy());  
    if (!button_press_item_.expired()) {        
      Window::Ptr item = button_press_item_.lock();    
      WorkEvent(ev, &Window::OnDblclick, item);        
    }
    SetFocus(button_press_item_.lock());
    button_press_item_ = nullpointer;
  } catch (std::exception& e) {
    AfxMessageBox(e.what());
    error(e);    
  }
}

void Canvas::WorkMouseUp(MouseEvent& ev) {
  try {
    if (!button_press_item_.expired()) {
      button_press_item_.lock()->OnMouseUp(ev);
      button_press_item_.reset();
    } else {
      Window::Ptr item = HitTest(ev.cx(), ev.cy());  
      WorkEvent(ev, &Window::OnMouseUp, item);
    }
  } catch (std::exception& e) {
    AfxMessageBox(e.what());
    error(e);    
  }
}

void Canvas::WorkMouseMove(MouseEvent& ev) {
  try {
    if (!button_press_item_.expired()) {        
      Window::Ptr item = button_press_item_.lock();    
      WorkEvent(ev, &Window::OnMouseMove, item);
    } else {
      Window::Ptr item = HitTest(ev.cx(), ev.cy());    
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
  } catch (std::exception& e) {
    AfxMessageBox(e.what());
    error(e);    
  }
}

void Canvas::Invalidate(Region& rgn) { 
  if (!prevent_fls_) {
    if (IsSaving()) {
      save_rgn_->Combine(rgn, RGN_OR);
    } else { 
      Window::Invalidate(rgn);
      //RedrawWindow(wnd_->m_hWnd, NULL, *((CRgn*) rgn.source()), RDW_INVALIDATE | RDW_UPDATENOW);
    }
  }
}

void Canvas::Flush() {
  if (!prevent_fls_) {  
      Invalidate(*save_rgn_.get());
      //RedrawWindow(wnd_->m_hWnd, NULL, *((CRgn*) save_rgn_->source()), RDW_INVALIDATE | RDW_UPDATENOW);
    // wnd_->InvalidateRgn((CRgn*) save_rgn_.source(), 0);
  }
  save_rgn_->Clear();  
}

void Canvas::Invalidate() {
  if (!prevent_fls_) {
    Invalidate();    
  }
}  

void Canvas::InvalidateSave() {
  if (!prevent_fls_) {
     // &&::IsWindow(wnd_->m_hWnd)) {
    Invalidate(*save_rgn_.get()); // Rgn((CRgn*) save_rgn_->source(), 0);    
  }
}

void Canvas::SetCursor(CursorStyle style) {
  /*LPTSTR c = 0;
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
  cursor_ = (c!=0) ? LoadCursor(0, c) : ::LoadCursor(0, MAKEINTRESOURCE(ac));*/
}

// CanvasView

/*
void View::set_canvas(Canvas::WeakPtr canvas) { 
  if (canvas_.lock() == canvas.lock()) {
    return;
  }    
  if (!canvas_.expired()) {
    canvas_.lock()->set_wnd(0);
  }
  canvas_ = canvas;            
  if (!canvas.expired()) {      
  //if (!canvas.expired() && m_hWnd) {      
    Canvas* c = canvas.lock().get();
    c->set_wnd(this);
    try {   
      ui::Size size = dim();
      if (size.width() != 0 && size.height() != 0) {      
        c->OnSize(size.width(), size.height());
        c->ClearSave();
        Invalidate();
      }
    } catch (std::exception& e) {
      error(e);
      AfxMessageBox(e.what());
    }  
  } 
} */  

/*
void MFCView::OnTimerViewRefresh() {    
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
//        c->set_wnd(this);
        c->InvalidateSave();
      }
      c->WorkTimer();
    } catch(std::exception& e) {
      e;
    }    
  }
}

void MFCView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {   
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

void MFCView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {  
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

BOOL MFCView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {    
   Canvas* c = canvas().lock().get();    
   if (c) {
    ::SetCursor(c->cursor());
    return TRUE;
   }
  return CWnd::OnSetCursor(pWnd, nHitTest, message);
} */

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
  // pView_ = 0;    
}

} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle