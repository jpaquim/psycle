#pragma once
#include <psycle/host/detail/project.hpp>
#include "Ui.hpp"
#include "Psycle.hpp"

#include "MfcUi.hpp"
#include "LuaGui.hpp"

namespace psycle {
namespace host {
namespace ui {

boost::shared_ptr<ui::Region> Window::dummy_region_(ui::Systems::instance().CreateRegion());
Window::Container Window::dummy_list_;

int MenuContainer::id_counter = 0;

MenuContainer::MenuContainer() : imp_(ui::ImpFactory::instance().CreateMenuContainerImp()) {
  imp_->set_menu_bar(this);
}

MenuContainer::MenuContainer(ui::MenuContainerImp* imp) : imp_(imp) {
  imp_->set_menu_bar(this);
}

void MenuContainer::Update() {
  if (!root_node_.expired()) {
    assert(imp_.get());
    imp_->DevUpdate(root_node_.lock());  
  }
}

void MenuContainer::Invalidate() {
  assert(imp_.get());
  imp_->DevInvalidate();
}

PopupMenu::PopupMenu() 
  : MenuContainer(ui::ImpFactory::instance().CreatePopupMenuImp()) {
}

void PopupMenu::Track(const ui::Point& pos) {
  assert(imp());
  imp()->DevTrack(pos);
}

Region::Region() : imp_(ui::ImpFactory::instance().CreateRegionImp()) {
}

Region::Region(const ui::Rect& rect) : imp_(ui::ImpFactory::instance().CreateRegionImp()) {
  imp_->DevSetRect(rect);
}

Region::Region(ui::RegionImp* imp) : imp_(imp) {
}

Region& Region::operator = (const Region& other) {
  imp_.reset(other.imp()->DevClone());
  return *this;
}

Region::Region(const Region& other) {
  imp_.reset(other.imp()->DevClone());
}

Region* Region::Clone() const {
  Region* region = 0;
  if (imp_.get()) {
    region = new Region(imp_->DevClone());
  }
  return region;
}

void Region::Offset(double dx, double dy) {
  if (imp_.get()) {
    imp_.get()->DevOffset(dx, dy);
  }
}

int Region::Combine(const Region& other, int combinemode) {
  int result(0);
  if (imp_.get()) {
    result = imp_.get()->DevCombine(other, combinemode);
  }
  return result;
}

ui::Rect Region::bounds() const {
  ui::Rect result;
  if (imp_.get()) {
    result = imp_.get()->DevBounds();
  }
  return result;
}

bool Region::Intersect(double x, double y) const {
  bool result = false;
  if (imp_.get()) {
    result = imp_.get()->DevIntersect(x, y);
  }
  return result;
}

bool Region::IntersectRect(const ui::Rect& rect) const {
  bool result = false;
  if (imp_.get()) {
    result = imp_.get()->DevIntersectRect(rect);
  }
  return result;
}

void Region::Clear() {  
  if (imp_.get()) {
    imp_.get()->DevClear();
  }  
}

void Region::SetRect(const ui::Rect& rect) {
  if (imp_.get()) {
    imp_.get()->DevSetRect(rect);
  }
}

std::auto_ptr<ui::Region> Rect::region() const {
  std::auto_ptr<ui::Region> result(ui::Systems::instance().CreateRegion());
  result->SetRect(*this);
  return result;  
}

Area::Area() : needs_update_(true) {}

Area::Area(const Rect& rect) : needs_update_(true) {
  Add(RectShape(rect));
}

Area::~Area() {}

Area* Area::Clone() const {
  Area* area = new Area();  
  area->rect_shapes_.insert(area->rect_shapes_.end(), rect_shapes_.begin(),
                            rect_shapes_.end());
  return area;
}

const ui::Region& Area::region() const {     
  Update();  
  return region_cache_;
}

const ui::Rect& Area::bounds() const {    
  Update();    
  return bound_cache_;
}

void Area::Offset(double dx, double dy) {  
  rect_iterator i = rect_shapes_.begin();
  for (; i != rect_shapes_.end(); ++i) {
    i->Offset(dx, dy);
  }
  needs_update_ = true;
}

void Area::Clear() {  
  rect_shapes_.clear();
  region_cache_.Clear();
  bound_cache_.set(ui::Point(0, 0), ui::Point(0, 0));
  needs_update_ = false;
}

bool Area::Intersect(double x, double y) const {  
  Update();
  return region_cache_.Intersect(x, y);
}

void Area::Add(const RectShape& rect) {  
  rect_shapes_.push_back(rect);
  needs_update_ = true;
}

int Area::Combine(const Area& other, int combinemode) {  
  if (combinemode == RGN_OR) {    
    rect_shapes_.insert(rect_shapes_.end(), other.rect_shapes_.begin(),
                        other.rect_shapes_.end());
    needs_update_ = true;
    return COMPLEXREGION;
  }  
  return ERROR;
}

void Area::ComputeRegion() const {
  region_cache_.Clear();
  rect_const_iterator i = rect_shapes_.begin();
  for (; i != rect_shapes_.end(); ++i) {    
    region_cache_.Combine(ui::Region((*i).bounds()), RGN_OR);
  }
}

void Area::ComputeBounds() const {
  if (rect_shapes_.empty()) {
    bound_cache_.set(ui::Point(), ui::Point());
  } else {
    bound_cache_.set(ui::Point(std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::max()),
                      ui::Point(std::numeric_limits<double>::min(),
                                std::numeric_limits<double>::min()));
    rect_const_iterator i = rect_shapes_.begin();
    for (; i != rect_shapes_.end(); ++i) {
      ui::Rect bounds = (*i).bounds();
      if (bounds.left() < bound_cache_.left()) {
        bound_cache_.set_left(bounds.left());
      }
      if (bounds.top() < bound_cache_.top()) {
        bound_cache_.set_top(bounds.top());
      }
      if (bounds.right() > bound_cache_.right()) {
        bound_cache_.set_right(bounds.right());
      }
      if (bounds.bottom() > bound_cache_.bottom()) {
        bound_cache_.set_bottom(bounds.bottom());
      }
    }
  }  
}

Commands::Commands() {}

void Commands::Clear() {  
  functors.clear();  
}

void Commands::Invoke() {       
  std::list<boost::function<void(void)>>::iterator it = functors.begin();
  while (it != functors.end()) {
    (*it)();       
    it++;
  }  
}

//Image
Image::Image() : imp_(ui::ImpFactory::instance().CreateImageImp()) {
}

void Image::Load(const std::string& filename) {
  assert(imp_.get());
  imp_->Load(filename);
}

void Image::SetTransparent(bool on, ARGB color) {
  assert(imp_.get());
  imp_->SetTransparent(on, color);
}

ui::Dimension Image::dim() const {
  assert(imp_.get());
  return imp_->dim();
}

void* Image::source() {
  assert(imp_.get());
  return imp_->source();
}

void* Image::mask() {
  assert(imp_.get());
  return imp_->mask();
}

const void* Image::mask() const {
  assert(imp_.get());
  return imp_->mask();
}


// Graphics
Graphics::Graphics() : imp_(ui::ImpFactory::instance().CreateGraphicsImp()) {
}

Graphics::Graphics(CDC* cr) : imp_(ui::ImpFactory::instance().CreateGraphicsImp(cr)) {
}

void Graphics::CopyArea(const ui::Rect& rect, const ui::Point& delta) {
  assert(imp_.get());
  imp_->CopyArea(rect, delta);  
}

void Graphics::DrawArc(const ui::Rect& rect, const Point& start, const Point& end) {
  assert(imp_.get());
  imp_->DrawArc(rect, start, end);  
}

void Graphics::DrawLine(const ui::Point& p1, const ui::Point& p2) {
  assert(imp_.get());  
  imp_->DrawLine(p1, p2);  
}

void Graphics::DrawRect(const ui::Rect& rect) {
  assert(imp_.get());
  imp_->DrawRect(rect);  
}

void Graphics::DrawRoundRect(const ui::Rect& rect, const ui::Dimension& arc_dim) {
  assert(imp_.get());
  imp_->DrawRoundRect(rect, arc_dim);  
}

void Graphics::DrawOval(const ui::Rect& rect) {
  assert(imp_.get());
  imp_->DrawOval(rect); 
}

void Graphics::DrawString(const std::string& str, double x, double y) {
  assert(imp_.get());
  imp_->DrawString(str, x, y);  
}

void Graphics::FillRect(const ui::Rect& rect) {
  assert(imp_.get());
  imp_->FillRect(rect);  
}

void Graphics::FillRoundRect(const ui::Rect& rect, const ui::Dimension& arc_dim) {  
  assert(imp_.get());
  imp_->FillRoundRect(rect, arc_dim);
}

void Graphics::FillOval(const ui::Rect& rect) {  
  assert(imp_.get());
  imp_->FillOval(rect);
}

void Graphics::FillRegion(const ui::Region& rgn) {  
  assert(imp_.get());
  imp_->FillRegion(rgn);
}

void Graphics::SetColor(ARGB color) {
  assert(imp_.get());
  imp_->SetColor(color);  
}

ARGB Graphics::color() const {
  assert(imp_.get());
  return imp_->color();
}

void Graphics::Translate(double x, double y) {
  assert(imp_.get()); 
  imp_->Translate(x, y);
}

void Graphics::SetFont(const Font& font) {
  assert(imp_.get());
  imp_->SetFont(font);  
}

const Font& Graphics::font() const {
  assert(imp_.get());
  return imp_->font();
}

Dimension Graphics::text_size(const std::string& text) const {
  assert(imp_.get());
  return imp_->text_size(text);  
}

void Graphics::DrawPolygon(const ui::Points& points) {
  assert(imp_.get());
  imp_->DrawPolygon(points);  
}

void Graphics::FillPolygon(const ui::Points& points) {
  assert(imp_.get());
  imp_->DrawPolygon(points);  
}

void Graphics::DrawPolyline(const Points& points) {
  assert(imp_.get());
  imp_->DrawPolyline(points);  
}

void Graphics::DrawImage(ui::Image* img, double x, double y) {
  assert(imp_.get());
  imp_->DrawImage(img, x, y);  
}

void Graphics::DrawImage(ui::Image* img, double x, double y, double width, double height) {
  assert(imp_.get());
  imp_->DrawImage(img, x, y, width, height);  
}

void Graphics::DrawImage(ui::Image* img, double x, double y, double width, double height, double xsrc, double ysrc) {
  assert(imp_.get());
  imp_->DrawImage(img, x, y, width, height, xsrc, ysrc);  
}

void Graphics::SetClip(const ui::Rect& rect) {
  assert(imp_.get());
  imp_->SetClip(rect);  
}

void Graphics::SetClip(ui::Region* rgn) {
  assert(imp_.get());
  imp_->SetClip(rgn);  
}

CRgn& Graphics::clip() {  
  assert(imp_.get());
  return imp_->clip();
}

void Graphics::Dispose() {
  assert(imp_.get());
  imp_->Dispose();  
}

CDC* Graphics::dc() {
  assert(imp_.get());
  return imp_->dc();
}

void Graphics::SaveOrigin() {
  assert(imp_.get());
  imp_->SaveOrigin();
}

void Graphics::RestoreOrigin() {
  assert(imp_.get());
  imp_->RestoreOrigin();
}


Window::Window() :
    update_(true),
    area_(new Area()),
    auto_size_width_(true),
    auto_size_height_(true),
    visible_(true),
    align_(ALNONE),
    pointer_events_(true),
    has_store_(false) {    
}

Window::Window(WindowImp* imp) : 
    update_(true),
    area_(new Area()),
    fls_area_(new Area()),
    auto_size_width_(true),
    auto_size_height_(true),
    visible_(true),
    align_(ALNONE),
    pointer_events_(true),
    has_store_(false) {
  imp_.reset(imp);  
  if (imp) {
    imp->set_window(this);
  }
}

Window::~Window() {   
  if (ImpFactory::instance().DestroyWindowImp(imp_.get())) {              
    imp_.release();         
  }  
}

void Window::set_imp(WindowImp* imp) { 
  imp_.reset(imp);
  if (imp) {
    imp->set_window(this);
  }
}

void Window::lock() const {
  psycle::host::mfc::WinLock::Instance().lock();
}

void Window::unlock() const {
  psycle::host::mfc::WinLock::Instance().unlock();
}

void Window::set_parent(const Window::WeakPtr& parent) {  
  parent_ = parent;  
  if (imp_.get()) {
    imp_->dev_set_parent(parent.lock().get());
  }  
}

Window* Window::root() {  
  if (!parent().expired()) {
    //if (root_cache_.expired()) {
      Window::Ptr window = shared_from_this();
      do {
        if (window->is_root()) {
    //      root_cache_ = window;
          return window.get();
        }
        window = window->parent().lock();
      } while (window);        
    //} else {
      //return root_cache_.lock().get();
    //}
  }
  return 0;
}

Window* Window::root() const {  
  if (!parent().expired()) {
    Window::ConstPtr window = shared_from_this();
    do {
      if (window->is_root()) return (Window*) window.get();
      window = window->parent().lock();
    } while (window);  
  }
  return 0;
}

bool Window::IsInGroup(Window::WeakPtr group) const {  
  Window::ConstWeakPtr p = parent();
  while (!p.expired()) {
    if (group.lock() == p.lock()) {
      return true;
    }
    p = p.lock()->parent();
  }
  return false;
}

void Window::STR() {    
  fls_area_.reset(new Area(area().bounds()));
  fls_area_->Offset(abs_pos().left(), abs_pos().top());
  has_store_ = true;    
}

void Window::FLS() {    
  if (has_store_) {
    needsupdate();
    std::auto_ptr<Area> tmp(new Area(area().bounds()));
    tmp->Offset(abs_pos().left(), abs_pos().top());
    fls_area_->Combine(*tmp, RGN_OR);    
    has_store_ = false;    
  } else {
    STR();
    has_store_ = false;
  }
  if (visible() && root()) {
    root()->Invalidate(fls_area_->region());
  }    
}

bool Window::IsInGroupVisible() const {
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

void Window::set_ornament(boost::shared_ptr<Ornament> ornament) {  
  if (!ornament.get() && ornament_.expired()) {
    return;
  }
  ornament_ = ornament;
  if (ornament) {
    std::auto_ptr<ui::Rect> opad = ornament->padding();
    if (opad.get()) {
      ui::Rect pad = padding();
      pad.set(ui::Point(pad.left() + opad->left(),
                        pad.top() + opad->top()),
              ui::Point(pad.right() + opad->right(),
                        pad.bottom() + opad->bottom()));
      set_padding(pad);
    }
  } else { 
    set_padding(ui::Rect());
  }
  FLS();
}

boost::weak_ptr<ui::Ornament> Window::ornament() {
  return ornament_;
}

const Area& Window::area() const { 
  if (update_) {          
    update_ = !const_cast<Window*>(this)->OnUpdateArea();          
  }            
  return *area_.get();
}  

bool Window::OnUpdateArea() { 
  bool result = true;
  if (imp_.get()) {
    result = imp_->OnDevUpdateArea(*area_.get());
  }
  return result;
}

void Window::set_pos(const ui::Point& pos) {
  STR();
  set_pos(ui::Rect(pos, area().bounds().dimension()));
  FLS();
}

void Window::set_pos(const ui::Rect& pos) {  
  bool size_changed = pos.width() != area().bounds().width() || 
                      pos.height() != area().bounds().height();  
  if (imp_.get()) {    
    ui::Point bottom_right(pos.left() + (auto_size_width() ? dim().width() : pos.width()),
                           pos.top() + (auto_size_height() ? dim().height() : pos.height()));
    STR();
    imp_->dev_set_pos(ui::Rect(pos.top_left(), bottom_right));
    FLS();
  }
  if (size_changed) {
    OnSize(pos.width(), pos.height());
  }
  WorkChildPos();  
}

ui::Rect Window::pos() const { 
  return imp() ? Rect(imp_->dev_pos().top_left(), dim()) : Rect();
}

ui::Rect Window::abs_pos() const {
  return imp() ? Rect(imp_->dev_abs_pos().top_left(), dim()) : Rect();
}

ui::Rect Window::desktop_pos() const {
  return imp() ? Rect(imp_->dev_desktop_pos().top_left(), dim()) : Rect();
}

ui::Dimension Window::dim() const {
  if (imp_.get()) {
    return ui::Dimension((auto_size_width() ? area().bounds().width() : imp_->dev_pos().width()),
                    (auto_size_height() ? area().bounds().height() : imp_->dev_pos().height()));
  } else {
    return ui::Dimension(area().bounds().width(), area().bounds().height());
  }
}

bool Window::auto_size_width() const { 
  return auto_size_width_;
}

bool Window::auto_size_height() const {  
  return auto_size_height_;  
} 

void Window::needsupdate() {
  update_ = true;
  if (!parent().expired()) {
    parent().lock()->needsupdate();
  }
}

void Window::WorkChildPos() {
  needsupdate();
  std::vector<Window::Ptr> items;
  Window::WeakPtr p = parent();
  while (!p.expired()) {
    items.push_back(p.lock());
    p = p.lock()->parent();
  }  
  std::vector<Window::Ptr>::reverse_iterator rev_it = items.rbegin();
  for (; rev_it != items.rend(); ++rev_it) {
    Window::Ptr item = *rev_it;    
    item->OnChildPos(*this);    
  } 
}


Window::WeakPtr Window::focus() {
  if (imp()) {
    ui::Window* win = imp()->dev_focus_window();
    if (win) {
      return win->shared_from_this();
    }
  }
  return nullpointer;
}

void Window::SetFocus() {
  if (imp()) {
    imp()->DevSetFocus();      
  }
}

void Window::PreventFls() {
  if (root()) {
    root()->PreventFls();
  }
}

void Window::EnableFls() {
  if (root()) {
    root()->EnableFls();
  }
}

bool Window::is_fls_prevented() const { 
  return root() ? root()->is_fls_prevented() : false; 
}

void Window::Show() {  
  if (imp_.get()) {
    imp_->DevShow();
  }  
  visible_ = true;
  OnShow();
  FLS();
}

void Window::Show(const boost::shared_ptr<WindowShowStrategy>& aligner) {  
  aligner->set_position(*this);
  Show();  
}

void Window::Hide() {   
  if (imp_.get()) {
    imp_->DevHide();  
  }  
  visible_ = false;
  FLS();
}

void Window::Invalidate() { 
  if (imp_.get()) {
    imp_->DevInvalidate();
  }
}

void Window::Invalidate(const Region& rgn) { 
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

void Window::set_clip_children() {
  if (imp_.get()) {
    imp_->dev_set_clip_children();
  }
}

void Window::add_style(UINT flag) {
  if (imp_.get()) {
    imp_->dev_add_style(flag);
  }
}

void Window::remove_style(UINT flag) {
  if (imp_.get()) {
    imp_->dev_remove_style(flag);
  }
}

void Window::DrawBackground(Graphics* g, Region& draw_region) {
 if (draw_region.bounds().height() > 0) {    
    if (!ornament().expired()) { 
      Window::Ptr shared_this = shared_from_this();      
      ornament().lock()->Draw(shared_this, g, draw_region);
    }
  }
}

template<class T, class T1>
void Window::PreOrderTreeTraverse(T& functor, T1& cond) {  
  if (!functor(*this)) {
    for (iterator i = begin(); i != end(); ++i) {
      Window::Ptr window = *i;      
      if (!cond(*window.get())) {
        window->PreOrderTreeTraverse(functor, cond);
      }
    }  
  }
}

template<class T, class T1>
void Window::PostOrderTreeTraverse(T& functor, T1& cond) {      
  for (iterator i = begin(); i != end(); ++i) {
    Window::Ptr window = *i;  
    if (!cond(*window.get())) {      
      window->PostOrderTreeTraverse(functor, cond);
    }
  }
  functor(*this);
}

// Aligner
Aligner::Aligner() : aligned_(false) {}

bool Aligner::full_align_ = true;

bool Visible::operator()(Window& window) const {
  return window.visible();
}

void CalcDim::operator()(Window& window) const {
 if (window.aligner()) {
   window.aligner()->CalcDimensions();    
 } 
};

bool AbortPos::operator()(Window& window) const { 
    bool abort_tree_walk = true;
    if (window.visible() && window.has_childs() && window.aligner()) {      
      //std::stringstream str;
      //str << "---- " << window.debug_text() << "----" << std::endl;
      //str << "wo-pos" << window.pos_old_.left() << "," << window.pos_old_.top() << std::endl;
      //str << "w-pos" << window.pos().left() << "," << window.pos().top() << std::endl;
		 // TRACE(str.str().c_str());
      if (!window.aligner()->aligned() || 
        window.pos() != window.pos_old_) {
        abort_tree_walk = false;
        /*std::stringstream str;
        str << "wo-pos" << window.pos_old_.left() << "," << window.pos_old_.top() << std::endl;
        str << "wo-size" << window.pos_old_.width() << "," << window.pos_old_.height() << std::endl;
        str << "w-pos" << window.pos().left() << "," << window.pos().top() << std::endl;
        str << "w-size" << window.pos().width() << "," << window.pos().height() << std::endl;
        str << "---- not abort: " << window.debug_text() << "----" << std::endl;
        TRACE(str.str().c_str());*/
        window.aligner()->aligned_ = true;
      }
    }    
    return abort_tree_walk;
  }

bool SetUnaligned::operator()(Window& window) const {
  if (window.aligner()) {
    window.aligner()->aligned_ = false;    
  }
  return false;
}

bool SetPos::operator()(Window& window) const {    
  for (Window::iterator it = window.begin(); it != window.end(); ++it) {
    Window::Ptr child = *it;
    child->pos_old_ = child->pos();
  }
  if (window.visible() && window.has_childs()) {    
    window.aligner()->SetPositions();      
  }  
  return false;
};

Window::Container Window::SubItems() {
    Window::Container allitems;
    iterator it = begin();
    for (; it != end(); ++it) {
      Window::Ptr item = *it;
      allitems.push_back(item);
      Window::Container subs = item->SubItems();
      iterator itsub = subs.begin();
      for (; itsub != subs.end(); ++itsub) {
        allitems.push_back(*it);
      }
    }
    return allitems;
  }     

// Group

Group::Group() {
  set_imp(ui::ImpFactory::instance().CreateWindowImp());
  set_auto_size(false, false);
}

Group::Group(WindowImp* imp) {
  set_imp(imp);
  set_auto_size(false, false);
}  

void Group::Add(const Window::Ptr& window) {  
  if (!window->parent().expired()) {
    throw std::runtime_error("Item already child of a group.");
  }  
  items_.push_back(window);
  window->set_parent(shared_from_this());  
  window->needsupdate();
}

void Group::Insert(iterator it, const Window::Ptr& item) {
  assert(item);
  assert(item->parent().expired());  
  item->set_parent(shared_from_this());
  items_.insert(it, item);  
}

void Group::Remove(const Window::Ptr& item) {
  assert(item);
  iterator it = find(items_.begin(), items_.end(), item);
  if (it == items_.end()) {
    throw std::runtime_error("Item is no child of the group");
  }  
  items_.erase(it);  
  item->set_parent(Window::WeakPtr());  
}

bool Group::OnUpdateArea() {  
  if (!auto_size_width() && !auto_size_height()) {
    area_->Clear();
    area_->Add(RectShape(ui::Rect(area_->bounds().top_left(),
                                  imp()->dev_pos().dimension())));
    return true;
  }

  std::auto_ptr<Area> area(new Area());
  std::vector<Window::Ptr>::const_iterator it = items_.begin();  
  for ( ; it != items_.end(); ++it) {
    Window::Ptr item = *it;  
    if (item->visible()) {
      std::auto_ptr<Area> tmp(item->area().Clone());      
      tmp->Offset(item->pos().left(), item->pos().top());        
      // int nCombineResult = 
      area->Combine(*tmp, RGN_OR);           
    }
  }
  area_.reset(area->Clone()); 
  if (!auto_size_width()) {
    ui::Rect bounds = area_->bounds();
    area_->Clear();
    area_->Add(RectShape(ui::Rect(bounds.top_left(),
                                  ui::Point(imp()->dev_pos().width(),
                                            bounds.height()))));
  }
  if (!auto_size_height()) {        
    ui::Rect bounds = area_->bounds();
    area_->Clear();
    area_->Add(RectShape(ui::Rect(bounds.top_left(),                                  
                                  ui::Point(bounds.width(),
                                            imp()->dev_pos().height()))));
  }
  return true;
}

void Group::set_zorder(Window::Ptr item, int z) {
  assert(item);
  if (z<0 || z>=items_.size()) return;
  iterator it = find(items_.begin(), items_.end(), item);
  assert(it != items_.end());  
  items_.erase(it);
  items_.insert(begin()+z, item);  
}

int Group::zorder(Window::Ptr item) const {
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
  Window::Container::const_reverse_iterator rev_it = items_.rbegin();
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
  
Window::Container Aligner::dummy;

void Group::UpdateAlign() {
  bool is_prevented = is_fls_prevented();
  PreventFls();  
  if (aligner_) {
    aligner_->Align();
  }
  if (!is_prevented) {
    EnableFls();    
  }
  Invalidate();
}

void Group::FlagNotAligned() {
  static AbortNone abort_none;
  static SetUnaligned set_unaligned;
  PreOrderTreeTraverse(set_unaligned, abort_none);  
}

// ScrollBar
ScrollBar::ScrollBar() 
  : Window(ui::ImpFactory::instance().CreateScrollBarImp(HORZ)) {
}

ScrollBar::ScrollBar(const ui::Orientation& orientation) 
  : Window(ui::ImpFactory::instance().CreateScrollBarImp(orientation)) {  
}

ScrollBar::ScrollBar(ScrollBarImp* imp) : Window(imp) {
}

void ScrollBar::set_scroll_range(int minpos, int maxpos) {
  if (imp()) {
    imp()->dev_set_scroll_range(minpos, maxpos);
  }
}

void ScrollBar::scroll_range(int& minpos, int& maxpos) {
  if (imp()) {
    imp()->dev_scroll_range(minpos, maxpos);
  }
}

void ScrollBar::set_scroll_pos(int pos) {
  if (imp()) {
    imp()->dev_set_scroll_pos(pos);
  }
}

int ScrollBar::scroll_pos() const {
  return imp() ? imp()->dev_scroll_pos() : 0;  
}

void ScrollBar::system_size(int& width, int& height) const {
  if (imp()) {
    ui::Dimension dim = imp()->dev_system_size();
    width = dim.width();
    height = dim.height();
  }
}

void WindowCenterToScreen::set_position(Window& window) {    
  ui::Dimension win_dim(
    (width_perc_ < 0)  ? window.dim().width()
                       : Systems::instance().metrics().screen_dimension().width() * width_perc_,
    (height_perc_ < 0) ? window.dim().height() 
                       : Systems::instance().metrics().screen_dimension().height() * height_perc_
  );
  window.set_pos(ui::Rect(    
    ((Systems::instance().metrics().screen_dimension() - win_dim) / 2.0)
    .as_point(),
    win_dim));
}

Frame::Frame() : Window(ui::ImpFactory::instance().CreateFrameImp()) {
  set_auto_size(false, false); 
}

Frame::Frame(FrameImp* imp) : Window(imp) {
  set_auto_size(false, false);
}

void Frame::set_title(const std::string& title) {
  if (imp()) {
    imp()->dev_set_title(title);
  }
}

void Frame::set_view(ui::Window::Ptr view) {
  view_ = view;
  if (imp()) {
    imp()->dev_set_view(view);
    if (view) {
      view->Show();
    }
  }
}

void Frame::ShowDecoration() {
  if (imp()) {
    imp()->DevShowDecoration();
  }
}

void Frame::HideDecoration() {
  if (imp()) {
    imp()->DevHideDecoration();
  }
}

void Node::erase(iterator it) {       
  boost::ptr_list<NodeImp>::iterator imp_it = imps.begin();
  for ( ; imp_it != imps.end(); ++imp_it) {
    imp_it->owner()->DevErase(*it);
  }
  children_.erase(it);
}

void Node::erase_imps(NodeOwnerImp* owner) {
  struct {    
    NodeOwnerImp* that;
      void operator()(Node::Ptr node, Node::Ptr prev_node) {
        boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
        while (it != node->imps.end()) {          
          if (it->owner() == that) {
            it = node->imps.erase(it);            
          } else {
            ++it;
          }
        }
      }
  } imp_eraser;
  imp_eraser.that = owner;
  traverse(imp_eraser, nullpointer);
}

void Node::AddNode(const Node::Ptr& node) {
  children_.push_back(node);
  node->set_parent(shared_from_this());
  boost::ptr_list<NodeImp>::iterator imp_it = imps.begin();
  for ( ;imp_it != imps.end(); ++imp_it) {
    imp_it->owner()->DevUpdate(node, boost::shared_ptr<ui::Node>());
  }
}

void Node::insert(iterator it, const Node::Ptr& node) { 
  Node::Ptr insert_after;
  if (it != begin() && it != end()) {    
    insert_after = *(--iterator(it));
  }      
  children_.insert(it, node);
  node->set_parent(shared_from_this());
  boost::ptr_list<NodeImp>::iterator imp_it = imps.begin();
  for ( ; imp_it != imps.end(); ++imp_it) {
    imp_it->owner()->DevUpdate(node, insert_after);
  }
}

TreeView::TreeView() : Window(ui::ImpFactory::instance().CreateTreeViewImp()) {
  set_auto_size(false, false);
}

TreeView::TreeView(TreeViewImp* imp) : Window(imp) { 
  set_auto_size(false, false);
}

void TreeView::Clear() {
  if (!root_node_.expired()) {    
    root_node_.reset();
    if (imp()) {
      imp()->DevClear();
    }
  }
}

void TreeView::UpdateTree() {
  if (imp() && !root_node_.expired()) {
    imp()->DevClear();  
    imp()->DevUpdate(root_node_.lock());
  }  
}

boost::weak_ptr<Node> TreeView::selected() {
  return imp() ? imp()->dev_selected() : boost::weak_ptr<Node>();  
}

void TreeView::select_node(const Node::Ptr& node) {
  if (imp()) {
    imp()->dev_select_node(node);
  }
}

void TreeView::set_background_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_background_color(color);
  }
}

ARGB TreeView::background_color() const {
  return imp() ? imp()->dev_background_color() : 0xFF00000;
}

void TreeView::set_text_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_text_color(color);
  }
}

ARGB TreeView::text_color() const {
  return imp() ? imp()->dev_text_color() : 0xFF00000;
}

void TreeView::EditNode(const Node::Ptr& node) {
  if (imp()) {
    imp()->DevEditNode(node);
  }
}

bool TreeView::is_editing() const {
  return imp() ? imp()->dev_is_editing() : false;  
}

void TreeView::ShowLines() {
  if (imp()) {
    imp()->DevShowLines();
  }
}

void TreeView::HideLines() {
  if (imp()) {
    imp()->DevHideLines();
  }
}

void TreeView::ShowButtons() {
  if (imp()) {
    imp()->DevShowButtons();
  }
}

void TreeView::HideButtons() {
  if (imp()) {
    imp()->DevHideButtons();
  }
}

void TreeView::set_images(const Images::Ptr& images) { 
  images_ = images;
  if (imp()) {
    imp()->dev_set_images(images);
  }
}

// Table
Table::Table() : Window(ui::ImpFactory::instance().CreateTableImp()) {
  set_auto_size(false, false);
}

Table::Table(TableImp* imp) : Window(imp) {
}

void Table::InsertColumn(int col, const std::string& text) {
  if (imp()) {
    imp()->DevInsertColumn(col, text);
  }
}

void Table::InsertRow() {
  if (imp()) {
    imp()->DevInsertRow();
  }
}

int Table::InsertText(int nItem, const std::string& text) {
  if (imp()) {
    return imp()->DevInsertText(nItem, text);
  }
  return 0;
}

void Table::SetText(int nItem, int nSubItem, const std::string& text) {
  if (imp()) {
    imp()->DevSetText(nItem, nSubItem, text);
  }
}

void Table::set_background_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_background_color(color);
  }
}

ARGB Table::background_color() const {
  return imp() ? imp()->dev_background_color() : 0xFF00000;
}

void Table::set_text_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_text_color(color);
  }
}

ARGB Table::text_color() const {
  return imp() ? imp()->dev_text_color() : 0xFF00000;
}

void Table::AutoSize(int cols) {
  if (imp()) {
    imp()->DevAutoSize(cols);
  }
}

void TableItem::set_imp(TableItemImp* imp) { 
  imp_.reset(imp);
  /*if (imp) {
    imp->set_table_item(this);
  }*/
}

void TableItem::set_table(boost::weak_ptr<Table> table) {
  table_ = table;  
  if (imp()) {
    imp()->dev_set_table(table);
  }
/*  std::list<boost::shared_ptr<TreeNode> >::iterator i = children_.begin();
  for (; i != children_.end(); ++i) {
    boost::shared_ptr<TreeNode> node = *i;
    node->set_tree(tree);    
  }*/
}

void TableItem::WorkClick() {
  OnClick();
  if (!table_.expired()) {
    // table_.lock()->OnClick(this);
  }
}

void TableItem::set_text(const std::string& text) {
  if (imp()) {
    imp()->dev_set_text(text);
  }
}

std::string TableItem::text() const {
  return (imp()) ? imp()->dev_text() : "no imp";  
}

ComboBox::ComboBox() : Window(ui::ImpFactory::instance().CreateComboBoxImp()) {  
  set_auto_size(false, false);
}

ComboBox::ComboBox(ComboBoxImp* imp) : Window(imp) {
}

void ComboBox::set_items(const std::vector<std::string>& itemlist) {
  if (imp()) {
    imp()->dev_set_items(itemlist);
  }
}

std::vector<std::string>  ComboBox::items() const {
  if (imp()) {
    return imp()->dev_items();
  } else {
    return std::vector<std::string>();
  }
}

void ComboBox::set_item_index(int index) {
  if (imp()) {
    imp()->dev_set_item_index(index);
  }
}

int ComboBox::item_index() const {
  if (imp()) {
    return imp() ? imp()->dev_item_index() : -1;
  }
}

Edit::Edit() : Window(ui::ImpFactory::instance().CreateEditImp()) { 
  set_auto_size(false, false);
}

Edit::Edit(EditImp* imp) : Window(imp) {
}

void Edit::set_text(const std::string& text) {
  if (imp()) {
    imp()->dev_set_text(text);
  }
}

std::string Edit::text() const { 
  return imp() ? imp()->dev_text() : "";
}

Button::Button() : Window(ui::ImpFactory::instance().CreateButtonImp()) {
  set_auto_size(false, false);
}

Button::Button(ButtonImp* imp) : Window(imp) {
}

void Button::set_text(const std::string& text) {
  if (imp()) {
    imp()->dev_set_text(text);
  }
}

std::string Button::text() const {
  return imp() ? imp()->dev_text() : "";
}

std::string Scintilla::dummy_str_ = "";

Scintilla::Scintilla() : Window(ui::ImpFactory::instance().CreateScintillaImp()) { 
  set_auto_size(false, false);
}

Scintilla::Scintilla(ScintillaImp* imp) : Window(imp) {
}

int Scintilla::f(int sci_cmd, void* lparam, void* wparam) {
  if (imp()) {
    return imp()->dev_f(sci_cmd, lparam, wparam);
  }
  return 0;
}
void Scintilla::AddText(const std::string& text) {
  if (imp()) {
    imp()->DevAddText(text);
  }
}

void Scintilla::FindText(const std::string& text, int cpmin, int cpmax, int& pos, int& cpselstart, int& cpselend) const {
  if (imp()) {
    imp()->DevFindText(text, cpmin, cpmax, pos, cpselstart, cpselend);
  }
}

void Scintilla::GotoLine(int pos) {
  if (imp()) {
    imp()->DevGotoLine(pos);
  }
}

int Scintilla::length() const  { 
  return imp() ? imp()->dev_length() : 0;
}

int Scintilla::selectionstart()  { 
  return imp() ? imp()->dev_selectionstart() : 0;
}

int Scintilla::selectionend()  {
  return imp() ? imp()->dev_selectionend() : 0;
}

void Scintilla::SetSel(int cpmin, int cpmax)  {
  if (imp()) {
    imp()->DevSetSel(cpmin, cpmax);
  }
}

bool Scintilla::has_selection() const {
  return imp() ? imp()->dev_has_selection() : false;
}

void Scintilla::set_find_match_case(bool on) {
  if (imp()) {
    imp()->dev_set_find_match_case(on);
  }
}

void Scintilla::set_find_whole_word(bool on) {
  if (imp()) {
    imp()->dev_set_find_whole_word(on);
  }
}

void Scintilla::set_find_regexp(bool on) {
  if (imp()) {
    imp()->dev_set_find_regexp(on);
  }
}

void Scintilla::LoadFile(const std::string& filename) {
  if (imp()) {
    imp()->DevLoadFile(filename);
  }
}

void Scintilla::SaveFile(const std::string& filename) {
  if (imp()) {
    imp()->DevSaveFile(filename);
  }
}

bool Scintilla::has_file() const {
  return imp() ? imp()->dev_has_file() : false;
}

void Scintilla::set_lexer(const Lexer& lexer) {
  if (imp()) {
    imp()->dev_set_lexer(lexer);
  }
}

void Scintilla::set_foreground_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_foreground_color(color);
  }
}

ARGB Scintilla::foreground_color() const {
  return imp() ? imp()->dev_foreground_color() : 0;
}

void Scintilla::set_background_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_background_color(color);
  }
}

ARGB Scintilla::background_color() const {
  return imp() ? imp()->dev_background_color() : 0;
}

void Scintilla::set_linenumber_foreground_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_linenumber_foreground_color(color);
  }
}

ARGB Scintilla::linenumber_foreground_color() const {
  return imp() ? imp()->dev_linenumber_foreground_color() : 0;
}

void Scintilla::set_linenumber_background_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_linenumber_background_color(color);
  }
}

ARGB Scintilla::linenumber_background_color() const {
  return imp() ? imp()->dev_linenumber_background_color() : 0;
}

void Scintilla::set_margin_background_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_margin_background_color(color);
  }
}

ARGB Scintilla::margin_background_color() const {
  return imp() ? imp()->dev_margin_background_color() : 0;
}

void Scintilla::set_sel_foreground_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_sel_foreground_color(color);
  }
}

  //ARGB sel_foreground_color() const { return ToARGB(ctrl().sel_foreground_color()); }  
void Scintilla::set_sel_background_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_sel_background_color(color);
  }
}

  //ARGB sel_background_color() const { return ToARGB(ctrl().sel_background_color()); }
void Scintilla::set_sel_alpha(int alpha) {
  if (imp()) {
    imp()->dev_set_sel_alpha(alpha);
  }
}

void Scintilla::set_ident_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_ident_color(color);
  }
}

void Scintilla::set_caret_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_caret_color(color);
  }
}
ARGB Scintilla::caret_color() const {
  return imp() ? imp()->dev_caret_color() : 0;
}

void Scintilla::StyleClearAll() {
  if (imp()) {
    imp()->DevStyleClearAll();
  }
}

const std::string& Scintilla::filename() const {
  return imp() ? imp()->dev_filename() : dummy_str_;
}

bool Scintilla::is_modified() const {
  return imp() ? imp()->dev_is_modified() : false;
}

GameController::GameController() : id_(-1) {
  xpos_ = ypos_ = zpos_ = 0;
}

void GameController::AfterUpdate(const GameController& old_state) {
  for (int b = 0; b < 32; ++b) {
    bool old_btn_set = old_state.buttons().test(b);
    bool current_btn_set = buttons().test(b);
    if (current_btn_set != old_btn_set) {
      if (current_btn_set) {
        OnButtonDown(b);
      } else {
        OnButtonUp(b);
      }
    }
    if (xpos() != old_state.xpos()) {
      OnXAxis(xpos(), old_state.xpos());
    }
    if (ypos() != old_state.ypos()) {
      OnYAxis(ypos(), old_state.ypos());
    }
    if (zpos() != old_state.zpos()) {
      OnZAxis(zpos(), old_state.zpos());
    }
  }  
}

// Ui Factory
Systems& Systems::instance() {
  static Systems instance_;
  if (!instance_.concrete_factory_.get()) {
    instance_.concrete_factory_.reset(new ui::mfc::Systems());
  }
  return instance_;
}

SystemMetrics& Systems::metrics() {
  static ui::mfc::SystemMetrics metrics_;  
  return metrics_;
}


void Systems::set_concret_factory(Systems& concrete_factory) {
  concrete_factory_.reset(&concrete_factory);
}

ui::Region* Systems::CreateRegion() { 
  assert(concrete_factory_.get());
  return concrete_factory_->CreateRegion(); 
}

ui::Graphics* Systems::CreateGraphics() { 
  assert(concrete_factory_.get());
  return concrete_factory_->CreateGraphics(); 
}

ui::Graphics* Systems::CreateGraphics(void* dc) { 
  assert(concrete_factory_.get());
  return concrete_factory_->CreateGraphics(dc); 
}

ui::Image* Systems::CreateImage() { 
  assert(concrete_factory_.get());
  return concrete_factory_->CreateImage(); 
}

ui::Font* Systems::CreateFont() { 
  assert(concrete_factory_.get());
  return concrete_factory_->CreateFont(); 
}

ui::Window* Systems::CreateWin() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateWin(); 
}

ui::Frame* Systems::CreateFrame() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateFrame(); 
}

ui::ComboBox* Systems::CreateComboBox() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateComboBox(); 
}

ui::Edit* Systems::CreateEdit() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateEdit(); 
}

ui::Button* Systems::CreateButton() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateButton(); 
}

ui::ScrollBar* Systems::CreateScrollBar() {  
  return new ui::ScrollBar();
}

ui::TreeView* Systems::CreateTreeView() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateTreeView(); 
}

ui::MenuContainer* Systems::CreateMenuBar() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateMenuBar(); 
}

ui::PopupMenu* Systems::CreatePopupMenu() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreatePopupMenu(); 
}

// WindowImp

void WindowImp::OnDevDraw(Graphics* g, Region& draw_region) {
  if (window_) {    
    window_->DrawBackground(g, draw_region);    
    window_->Draw(g, draw_region);    
  }
}

void WindowImp::OnDevSize(double width, double height) {
  if (window_) {
    window_->needsupdate();
    window_->OnSize(width, height);
  }
}

void WindowImp::OnDevMouseDown(MouseEvent& ev) {
  if (window_ && window_->root()) {
    window_->root()->WorkMouseDown(ev);
  }
}
void WindowImp::OnDevMouseUp(MouseEvent& ev) {
  if (window_ && window_->root()) {
    window_->root()->WorkMouseUp(ev);    
  }
}

void WindowImp::OnDevDblclick(MouseEvent& ev) {
  if (window_ && window_->root()) {
    window_->root()->WorkDblClick(ev);
  }
}

void WindowImp::OnDevMouseMove(MouseEvent& ev) {
  if (window_ && window_->root()) {    
    window_->root()->WorkMouseMove(ev);    
  }
}  

// Key Events
void WindowImp::OnDevKeyDown(KeyEvent& ev) { 
  if (window_ && window_->root()) {
    window_->root()->WorkKeyDown(ev);
  }
}

void WindowImp::OnDevKeyUp(KeyEvent& ev) {
  if (window_ && window_->root()) {
    window_->root()->WorkKeyUp(ev);
  }
}

void FrameImp::OnDevClose() {
  if (window()) {
    ((Frame*)window())->OnClose();
  }
}

void ButtonImp::OnDevClick() {
  if (window()) {
    ((Button*)window())->OnClick();
  }
}

// ImpFactory
ImpFactory& ImpFactory::instance() {
  static ImpFactory instance_;
  if (!instance_.concrete_factory_.get()) {
    instance_.concrete_factory_.reset(new ui::mfc::ImpFactory());
  }
  return instance_;
} 

void ImpFactory::set_concret_factory(ImpFactory& concrete_factory) {
  concrete_factory_.reset(&concrete_factory);
}

ui::WindowImp* ImpFactory::CreateWindowImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateWindowImp(); 
}

bool ImpFactory::DestroyWindowImp(ui::WindowImp* imp) {
  assert(concrete_factory_.get());
  return concrete_factory_->DestroyWindowImp(imp); 
}

ui::WindowImp* ImpFactory::CreateWindowCompositedImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateWindowCompositedImp(); 
}

ui::FrameImp* ImpFactory::CreateFrameImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateFrameImp(); 
}

ui::ScrollBarImp* ImpFactory::CreateScrollBarImp(ui::Orientation orientation) {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateScrollBarImp(orientation); 
}

ui::ComboBoxImp* ImpFactory::CreateComboBoxImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateComboBoxImp(); 
}

ui::EditImp* ImpFactory::CreateEditImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateEditImp(); 
}

ui::TreeViewImp* ImpFactory::CreateTreeViewImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateTreeViewImp(); 
}

ui::MenuContainerImp* ImpFactory::CreateMenuContainerImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateMenuContainerImp();
}

ui::MenuContainerImp* ImpFactory::CreatePopupMenuImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreatePopupMenuImp();
}

ui::MenuImp* ImpFactory::CreateMenuImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateMenuImp(); 
}

ui::TableImp* ImpFactory::CreateTableImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateTableImp(); 
}

ui::ButtonImp* ImpFactory::CreateButtonImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateButtonImp(); 
}

ui::RegionImp* ImpFactory::CreateRegionImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateRegionImp(); 
}

ui::GraphicsImp* ImpFactory::CreateGraphicsImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateGraphicsImp(); 
}

ui::GraphicsImp* ImpFactory::CreateGraphicsImp(CDC* cr) {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateGraphicsImp(cr); 
}

ui::ImageImp* ImpFactory::CreateImageImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateImageImp(); 
}


ui::ScintillaImp* ImpFactory::CreateScintillaImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateScintillaImp(); 
}

ui::GameControllersImp* ImpFactory::CreateGameControllersImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateGameControllersImp(); 
}

} // namespace ui
} // namespace host
} // namespace psycle