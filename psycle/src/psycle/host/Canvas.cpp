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

void DefaultAligner::CalcDimensions() {
  double w(0);
  double h(0);  
  double left(0);
  double right(0);
  double top(0);
  double bottom(0);
  int bitmask = ALCLIENT | ALLEFT | ALRIGHT | ALTOP | ALBOTTOM;
  for (iterator i = begin(); i != end(); ++i) {
    Window::Ptr item = *i;
    if (!item->visible()) {
      continue;
    }    
    AlignStyle item_style = item->has_style() ? item->style()->align() : ALNONE;
    ui::Dimension item_dim = item->aligner() ? item->aligner()->dim() : item->dim();
    if (item_style != ALNONE) {
      const ui::Rect& margin = item->style()->margin();
      item_dim.set(item_dim.width() + margin.left() + margin.right(),
                   item_dim.height() + margin.top() + margin.bottom());
    }
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
          w = std::max(item_dim.width(), w);
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
  dim_.set(w, h);  
}


void DefaultAligner::SetPositions() {
  if (group_.expired()) {
    return;
  }
  if (group_.lock()->area().bounds().empty()) {
    return;
  }
  Window::Ptr client;
  double left(0);
  double right(group_.lock()->pos().width());
  double top(0);
  double bottom(group_.lock()->pos().height());  

  for (iterator i = begin(); i != end(); ++i) {
    Window::Ptr item = *i;
    if (!item->visible()) continue;
    ui::Dimension item_dim = item->aligner() ? item->aligner()->dim() : item->dim();
    AlignStyle item_style = item->has_style() ? item->style()->align() : ALNONE;    
    ui::Rect margin;    
    if (item->has_style()) {
      margin = item->style()->margin();      
      item_dim.set(item_dim.width() + margin.left() + margin.right(),
                   item_dim.height() + margin.top() + margin.bottom());    
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
  set_auto_size(false, false);  
  steal_focus_ = save_ = prevent_fls_ = false;  
}

void Canvas::OnSize(double cw, double ch) {
  Group::OnSize(cw, ch);
  try {    
    Align();    
  } catch (std::exception& e) {
    AfxMessageBox(e.what());
  }
}

void Canvas::StealFocus(const Window::Ptr& item) {
  button_press_item_ = item;
  steal_focus_ = true;
}

void Canvas::SetFocus(const Window::Ptr& item) {
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
    }
  }
}

void Canvas::Flush() {
  if (!prevent_fls_) {  
    Invalidate(*save_rgn_.get());      
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
    Invalidate(*save_rgn_.get());
  }
}

} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle