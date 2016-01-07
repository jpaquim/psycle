#pragma once
#include <psycle/host/detail/project.hpp>
#include "Ui.hpp"
#include "Psycle.hpp"

#include "MfcUi.hpp"

namespace psycle {
namespace host {
namespace ui {

Systems::Systems() : concrete_factory_(0) {}

Systems& Systems::instance() {
  static Systems instance_;
  if (!instance_.concrete_factory_) {
    instance_.concrete_factory_ = new ui::mfc::Systems();
  }
  return instance_;
} 

ui::Rect Window::dummy_pos_;
boost::shared_ptr<ui::Region> Window::dummy_region_(ui::Systems::instance().CreateRegion());
Window::List Window::dummy_list_;
Window::Ptr Window::nullpointer;

Window::~Window() {
  if (imp_.get()) {
    imp_->DevDestroy(); 
  }
}
void Window::set_pos(const ui::Rect& pos) { imp_->dev_set_pos(pos); }
ui::Size Window::dim() const { return imp_->dev_dim(); }
  
void Window::Show() { 
  if (imp_.get()) {
    imp_->DevShow(); 
  }
}
void Window::Hide() { 
  if (imp_.get()) {
    imp_->DevHide();
  }
}
void Window::Invalidate() { 
  if (imp_.get()) {
    imp_->DevInvalidate();
  }
}

void Window::Invalidate(Region& rgn) { 
  if (imp_.get()) {
    imp_->DevInvalidate(rgn); 
  }
}

void Window::SetCapture() { 
  if (imp_.get()) {
    imp_->DevSetCapture();
  }
}

void Window::ReleaseCapture() {
  if (imp_.get()) {
    imp_->DevReleaseCapture();
  }
}

void Window::ShowCursor() {
  if (imp_.get()) {
    imp_->DevShowCursor();
  }
}

void Window::HideCursor() {
  if (imp_.get()) {
    imp_->DevHideCursor();
  }
}

void Window::SetCursorPos(double x, double y) {
  if (imp_.get()) {
    imp_->DevSetCursorPos(x, y);
  }
}

void Window::SetCursor(CursorStyle style) {
  if (imp_.get()) {
    imp_->DevSetCursor(style);
  }
}

void Window::set_parent(Window* window) {
  if (imp_.get()) {
    imp_->dev_set_parent(window);
  }
} 

} // namespace ui
} // namespace host
} // namespace psycle