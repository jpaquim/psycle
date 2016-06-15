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
  Dimension current_dim;  
  ui::Rect current_pos;    
  for (iterator i = begin(); i != end(); ++i) {
    Window::Ptr item = *i;
    if (!item->visible()) {
      continue;
    }        
    ui::Dimension item_dim = item->aligner() ? item->aligner()->dim() : item->dim();   
    if (item->aligner()) {
      if (!item->auto_size_width()) {
        item_dim.set_width(item->dim().width());
      }
      if (!item->auto_size_height()) {
        item_dim.set_height(item->dim().height());
      }
    }
    if (item->align() != ALNONE) {      
      item_dim.set(item_dim.width(),
                   item_dim.height());
    }
    // width
    {
      double diff = current_pos.width();
      switch (item->align()) {
        case ALLEFT:
          current_pos.set_left(current_pos.left() + item_dim.width());
          if (diff == 0) {          
            current_dim.set_width(current_dim.width() + item_dim.width());
            current_pos.set_right(current_pos.right() + item_dim.width());
          } else {
            double expand = item_dim.width() - diff;
            if (expand > 0) {
              current_dim.set_width(current_dim.width() + expand);
              current_pos.set_right(current_pos.right() + expand);
            }
          }
          current_dim.set_width(current_dim.width() + item->margin().right() + item->margin().left());
        break;
        case ALRIGHT:        
          if (diff == 0) {          
            current_dim.set_width(current_dim.width() + item_dim.width());
            current_pos.set_right(current_pos.right() + item_dim.width());
          } else {
            double expand = item_dim.width() - diff;
            if (expand > 0) {
              current_dim.set_width(current_dim.width() + expand);
              current_pos.set_right(current_pos.right() + expand);
            } else
            if (expand == 0) {
              current_pos.set_right(current_pos.right() - item_dim.width());
            }
          }
        break;  
        case ALTOP:
        case ALBOTTOM:
          if (diff == 0) {
            current_dim.set_width(current_dim.width() + item_dim.width());
            current_pos.set_right(current_pos.right() + item_dim.width());
          } else {
            double expand = item_dim.width() - diff;
            if (expand > 0) {
              current_dim.set_width(current_dim.width() + expand);
              current_pos.set_right(current_pos.right() + expand);
            }
          }
          current_dim.set_width(current_dim.width() + item->margin().right() + item->margin().left());
        break;
            
        default:
          current_dim.set_width(std::max(item_dim.width(), current_dim.width()));          
        break;
      }    
    }

    // height
    {
      double diff = current_pos.height();
      switch (item->align()) {
        case ALTOP:
          current_pos.set_top(current_pos.top() + item_dim.height());
          if (diff == 0) {
            current_dim.set_height(current_dim.height() + item_dim.height());
            current_pos.set_bottom(current_pos.bottom() + item_dim.height());            
          } else {            
            double expand = item_dim.height() - diff;
            current_dim.set_height(current_dim.height() + expand);
            current_pos.set_bottom(current_pos.bottom() + expand); 
          }          
          current_dim.set_height(current_dim.height() + item->margin().bottom() + item->margin().top());
        break;
        case ALLEFT:
        case ALRIGHT:
          if (diff == 0) {
            current_dim.set_height(current_dim.height() + item_dim.height());
            current_pos.set_bottom(current_pos.bottom() + item_dim.height());
          } else {
            double expand = item_dim.height() - diff;
            if (expand > 0) {
              current_dim.set_height(current_dim.height() + expand);
              current_pos.set_bottom(current_pos.bottom() + expand);
            }
          }
          current_dim.set_height(current_dim.height() + item->margin().bottom() + item->margin().top());
        break;
        case ALBOTTOM:
          if (diff == 0) {          
            current_dim.set_height(current_dim.height() + item_dim.height());
            current_pos.set_bottom(current_pos.bottom() + item_dim.height());
          } else {
            double expand = item_dim.height() - diff;
            if (expand > 0) {
              current_dim.set_height(current_dim.height() + expand);
              current_pos.set_bottom(current_pos.bottom() + expand);
            }
          }
        break;            
        default:
          current_dim.set_height(std::max(item_dim.height(), current_dim.height()));          
        break;
      }    
    }
  } // end loop   
  
  dim_ = current_dim;
}

void DefaultAligner::SetPositions() {
  if (group_.expired()) {
    return;
  }
  if (group_.lock()->area().bounds().empty()) {
    return;
  }
  Window::Ptr client;
  ui::Rect current_pos(ui::Point(0, 0), group_.lock()->pos().dimension());
  for (iterator i = begin(); i != end(); ++i) {
    Window::Ptr item = *i;
    if (!item->visible()) continue;
    ui::Dimension item_dim = item->aligner() ? item->aligner()->dim() : item->dim();
    if (item->aligner()) {
      if (!item->auto_size_width()) {
        item_dim.set_width(item->dim().width());        
      }
      if (!item->auto_size_height()) {
        item_dim.set_height(item->dim().height());        
      }
    }                        
    switch (item->align()) {
      case ALCLIENT:
        client = item;
      break;      
      case ALLEFT:
        { 
          ui::Point top_left = ui::Point(current_pos.left() +  item->margin().left(), current_pos.top() + item->margin().top());          
          current_pos.set_left(current_pos.left() + item_dim.width() + item->margin().left());          
          ui::Rect new_pos = ui::Rect(top_left, current_pos.bottom_left());                                 
          if (new_pos != item->pos()) {
            item->set_pos(new_pos);
          }
          current_pos.set_left(current_pos.left() + item->margin().right());
        }
      break;
      case ALRIGHT:
        item->set_pos(ui::Rect(ui::Point(current_pos.right() - item_dim.width(), current_pos.top()), 
                               ui::Point(current_pos.right(), current_pos.bottom())));
        current_pos.set_right(current_pos.right() - item_dim.width());
      break;
      case ALTOP:
        {                   
          ui::Point top_left = ui::Point(current_pos.left() + item->margin().left(), current_pos.top() + item->margin().top());
          current_pos.set_top(current_pos.top() + item_dim.height() + item->margin().top());
          ui::Rect new_pos = ui::Rect(top_left, current_pos.top_right());
          if (new_pos != item->pos()) {            
            item->set_pos(new_pos);           
          }          
          current_pos.set_top(current_pos.top() + item->margin().bottom());
        }
      break;
      case ALBOTTOM:
        {                      
          current_pos.set_bottom(current_pos.bottom() - item->margin().bottom());
          ui::Point bottom_right = current_pos.bottom_right();
          current_pos.set_bottom(current_pos.bottom() - item_dim.height());
          ui::Rect new_pos = ui::Rect(current_pos.bottom_left(), bottom_right);
          if (new_pos != item->pos()) {
            item->set_pos(new_pos);
          }          
        }
      break;
      default:
      break;
    } // end switch    
  } // end loop 
  
  if (client) {    
    client->set_pos(current_pos);    
  }
}

void DefaultAligner::Realign() {}

// GridAligner
void GridAligner::CalcDimensions() {  
  Dimension itemmax;
  for (iterator i = begin(); i != end(); ++i) {
    Window::Ptr item = *i;
    if (!item->visible()) {
      continue;
    }        
    ui::Dimension item_dim = item->aligner() ? item->aligner()->dim() : item->dim();   
    if (item->aligner()) {
      if (!item->auto_size_width()) {
        item_dim.set_width(item->dim().width());
      }
      if (!item->auto_size_height()) {
        item_dim.set_height(item->dim().height());
      }
    }
    if (item->align() != ALNONE) {      
      item_dim.set(item_dim.width(), item_dim.height());
    }       
    itemmax.set_width(std::max(itemmax.width(), item_dim.width()));
    itemmax.set_height(std::max(itemmax.height(), item_dim.height()));    
  } // end loop   
  
  Dimension current_dim(itemmax.width()*col_num_, itemmax.height()*row_num_);  
  dim_ = current_dim;
}

void GridAligner::SetPositions() {
  if (group_.expired()) {
    return;
  }
  if (group_.lock()->area().bounds().empty()) {
    return;
  }
  Window::Ptr client;
  ui::Rect current_pos(ui::Point(0, 0), group_.lock()->aligner()->dim());

  int cell_width = current_pos.width() / col_num_;
  int cell_height = current_pos.height() / row_num_;

  int pos = 0;
  for (iterator i = begin(); i != end(); ++i) {
    Window::Ptr item = *i;
    if (!item->visible()) {
      continue;
    }    
    item->set_pos(ui::Rect(ui::Point(current_pos.left(), current_pos.top()), 
                           ui::Dimension(cell_width, cell_height)));    
    ++pos;
    if (pos < col_num_) {            
      current_pos.Offset(cell_width, 0);      
    } else {      
      current_pos.Offset(0, cell_height);
      current_pos.set_left(0);
      pos = 0;
    }  
  } // end loop     
}

void GridAligner::Realign() {}


// Canvas
void Canvas::Init() {
  set_auto_size(false, false);
  set_pos(ui::Rect(ui::Point(0, 0), ui::Dimension(500, 500)));
  steal_focus_ = fls_prevented_ = false;
  save_ = true;  
}

void Canvas::OnSize(double cw, double ch) {
  Group::OnSize(cw, ch);
  try {    
    UpdateAlign();    
  } catch (std::exception& e) {
    AfxMessageBox(e.what());
  }
}

void Canvas::StealFocus(const Window::Ptr& item) {  
  steal_focus_ = true;
}

// Events
void Canvas::WorkOnFocus(Event& ev) {
  try {
    if (!focus().expired()) {
      Window::Ptr item = focus().lock();
      WorkEvent(ev, &Window::OnFocus, item);
    } else {
      OnFocus(ev);  
    }
  } catch (std::exception& e) {
    AfxMessageBox(e.what());
    error(e);    
  }
}


void Canvas::Invalidate(const Region& rgn) {   
  if (!fls_prevented_) {
    if (IsSaving()) {
      save_rgn_->Combine(rgn, RGN_OR);
    } else { 
      Window::Invalidate(rgn);      
    }
  }  
}

void Canvas::Flush() {    
  if (!fls_prevented_) {  
    Window::Invalidate(*save_rgn_.get());    
  }
  save_rgn_->Clear();    
}

void Canvas::Invalidate() {  
  if (!fls_prevented_) {
    Window::Invalidate();    
  }  
}  

void Canvas::InvalidateSave() {
  if (!fls_prevented_) {     
    Invalidate(*save_rgn_.get());
  }
}

} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle