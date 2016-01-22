#pragma once
#include <psycle/host/detail/project.hpp>
#include "Ui.hpp"
#include "Psycle.hpp"

#include "MfcUi.hpp"

namespace psycle {
namespace host {
namespace ui {


boost::shared_ptr<ui::Region> Window::dummy_region_(ui::Systems::instance().CreateRegion());
Window::List Window::dummy_list_;
Window::Ptr Window::nullpointer;

Area* Area::Clone() const {
  Area* area = new Area();  
  area->rect_shapes_.insert(area->rect_shapes_.end(), rect_shapes_.begin(),
                            rect_shapes_.end());  
  return area;  
}

std::auto_ptr<ui::Region> Area::region() const {
  std::auto_ptr<ui::Region> rgn(ui::Systems::instance().CreateRegion());
  rect_const_iterator i = rect_shapes_.begin();
  for (; i != rect_shapes_.end(); ++i) {
    ui::Rect bounds = (*i).bounds();
    rgn->SetRect(bounds.left(), bounds.top(), bounds.width(), bounds.height());
  }
  return rgn;
}

void Area::Offset(double dx, double dy) {
  rect_iterator i = rect_shapes_.begin();
  for (; i != rect_shapes_.end(); ++i) {
    i->Offset(dx, dy);
  }
}

void Area::Clear() {
  rect_shapes_.clear();
}

bool Area::Intersect(double x, double y) const {
  return region()->Intersect(x, y);
}

void Area::Add(const RectShape& rect) {
  rect_shapes_.push_back(rect);
}

int Area::Combine(const Area& other, int combinemode) {
  if (combinemode == RGN_OR) {
    rect_shapes_.insert(rect_shapes_.end(), other.rect_shapes_.begin(),
                        other.rect_shapes_.end());
    return COMPLEXREGION;
  } 
  return ERROR;
}

ui::Rect Area::bounds() const {
  if (rect_shapes_.empty()) {
    return Rect(0, 0, 0, 0);
  }
  ui::Rect result(std::numeric_limits<double>::max(),
                  std::numeric_limits<double>::max(),
                  std::numeric_limits<double>::min(),
                  std::numeric_limits<double>::min());
  rect_const_iterator i = rect_shapes_.begin();
  for (; i != rect_shapes_.end(); ++i) {
    ui::Rect bounds = (*i).bounds();
    if (bounds.left() < result.left()) {
      result.set_left(bounds.left());
    }
    if (bounds.top() < result.top()) {
      result.set_top(bounds.top());
    }
    if (bounds.right() > result.right()) {
      result.set_right(bounds.right());
    }
    if (bounds.bottom() > result.bottom()) {
      result.set_bottom(bounds.bottom());
    }
  }
  return result;
}

Window::Window() :
    update_(true),
    area_(new Area()),
    auto_size_width_(true),
    auto_size_height_(true),
    visible_(true),
    pointer_events_(true) {
}

Window::Window(WindowImp* imp) : 
    update_(true),
    area_(new Area()),
    auto_size_width_(true),
    auto_size_height_(true),
    visible_(true),
    pointer_events_(true) {
  imp_.reset(imp);
}

Window::~Window() {
  if (imp_.get()) {
    imp_->DevDestroy(); 
  }
}

void Window::set_imp(WindowImp* imp) { 
  imp_.reset(imp);
  if (imp) {
    imp->set_window(this);
  }
}

void Window::set_parent(const Window::WeakPtr& parent) {  
  parent_ = parent;  
  if (imp_.get()) {
    imp_->dev_set_parent(parent.lock().get());
  }  
}

Window* Window::root() {  
  Window::Ptr window = shared_from_this();
  do {
    if (window->is_root()) return window.get();
    window = window->parent().lock();
  } while (window);  
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
  ornament_ = ornament;
  if (ornament) {
    std::auto_ptr<ui::Rect> opad = ornament->padding();
    if (opad.get()) {
      ui::Rect pad = style()->padding();
      pad.set(pad.left() + opad->left(),
              pad.top() + opad->top(),
              pad.right() + opad->right(),
              pad.bottom() + opad->bottom());    
      style()->set_padding(pad);
    }
  } else if (has_style()) { 
    style()->set_padding(ui::Rect());
  }
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

ui::Rect Window::abs_pos() const {      
  return ui::Rect(CalcAbsPos(pos_),
                  ui::Point(pos_.x() + dim().width(),
                            pos_.y() + dim().height()));
}

ui::Point Window::CalcAbsPos(const Point& pos) const {  
  std::vector<Window::ConstPtr> items;
  Window::ConstWeakPtr p = parent();
  while (!p.expired()) {
    items.push_back(p.lock());
    p = p.lock()->parent();
  }
  Point result = pos;  
  std::vector<Window::ConstPtr>::const_reverse_iterator rev_it = items.rbegin();
  for (; rev_it != items.rend(); ++rev_it) {
    Window::ConstPtr item = *rev_it;    
    result.set(result.x() + item->pos().left(), result.y() + item->pos().top());
  }
  return result;
}

void Window::set_pos(const ui::Point& pos) {    
  set_pos(ui::Rect(pos.x(),
                   pos.y(),
                   pos.x() + area().bounds().width(),
                   area().bounds().height()));
}

void Window::set_pos(const ui::Rect& pos) {
  bool size_changed = pos.width() != area().bounds().width() || 
                      pos.height() != area().bounds().height();
  pos_.set(pos.left(), pos.top());
  if (imp_.get()) {    
    ui::Point bottom_right(pos_.x() + (auto_size_width() ? dim().width() : pos.width()),
                           pos_.y() + (auto_size_height() ? dim().height() : pos.height()));
    imp_->dev_set_pos(ui::Rect(pos_, bottom_right));
  }
  if (size_changed) {
    OnSize(pos.width(), pos.height());
  }
  WorkChildPos();
}

ui::Rect Window::pos() const { 
 ui::Point bottom_right(pos_.x() + dim().width(),pos_.y() + dim().height());
 return ui::Rect(pos_, bottom_right);
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
  
void Window::Show() {  
  if (imp_.get()) {
    imp_->DevShow();  
  }  
  visible_ = true;
}

void Window::Hide() {   
  if (imp_.get()) {
    imp_->DevHide();  
  }  
  visible_ = false;
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

void Window::DrawBackground(Graphics* g, Region& draw_region) {
 if (draw_region.bounds().height() > 0) {    
    if (!ornament().expired()) { 
      Window::Ptr t = shared_from_this();
      ui::Rect bounds = area().bounds();
      ornament().lock()->Draw(t, g, draw_region);
    }
  }
}

template<class T>
void Window::PreOrderTreeTraverse(T& functor) {  
  if (!functor(*this)) {
    for (iterator i = begin(); i != end(); ++i) {
      Window::Ptr item = *i;          
      item->PreOrderTreeTraverse(functor);
    }  
  }
}

template<class T>
void Window::PostOrderTreeTraverse(T& functor) {  
  for (iterator child_it = begin(); child_it != end(); ++child_it) {    
    (*child_it)->PostOrderTreeTraverse(functor);
  }
  functor(*this);  
}

// Aligner
bool Aligner::full_align_ = true;

void CalcDim::operator()(Aligner& aligner) const { aligner.CalcDimensions(); };
void SetPos::operator()(Aligner& aligner) const { aligner.SetPositions(); };

template<class T>
void Aligner::PreOrderTreeTraverse(T& functor) {
  Window* g = group_.lock().get();  
  pos_ = g->pos();
  functor(*this);
  for (iterator child_it = begin(); child_it != end(); ++child_it) {
    Window::Ptr child = *child_it;
    if (child->visible() && child->aligner()) {      
      child->aligner()->PreOrderTreeTraverse(functor);    
    }
  }  
}

template<class T>
void Aligner::PostOrderTreeTraverse(T& functor) {  
  for (iterator i = begin(); i != end(); ++i) {
    Window::Ptr window = *i;
    if (window->visible() && window->aligner()) {      
      window->aligner()->PostOrderTreeTraverse(functor);
    }
  }
  functor(*this);  
}

Window::List Window::SubItems() {
    Window::List allitems;
    iterator it = begin();
    for (; it != end(); ++it) {
      Window::Ptr item = *it;
      allitems.push_back(item);
      Window::List subs = item->SubItems();
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
    area_->Add(RectShape(ui::Rect(area_->bounds().left(),
                                  area_->bounds().top(), 
                                  area_->bounds().left() + imp()->dev_pos().width(), 
                                  area_->bounds().top() + imp()->dev_pos().height())));
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
    area_->Add(RectShape(ui::Rect(bounds.left(),
                                  bounds.top(), 
                                  imp()->dev_pos().width(),
                                  bounds.height())));
  }
  if (!auto_size_height()) {        
    ui::Rect bounds = area_->bounds();
    area_->Clear();
    area_->Add(RectShape(ui::Rect(bounds.left(),
                                  bounds.top(), 
                                  bounds.width(),
                                  imp()->dev_pos().height())));
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
  if (aligner_) {
    aligner_->Align();
  }  
}

ScrollBar::ScrollBar(ScrollBarImp* imp) : Window(imp) {}

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

Frame::Frame() { set_auto_size(false, false); }

Frame::Frame(FrameImp* imp) : Window(imp) { set_auto_size(false, false); }

void Frame::set_title(const std::string& title) {
  if (imp()) {
    imp()->dev_set_title(title);
  }
}

void Frame::set_view(ui::Window::Ptr view) {
  if (imp()) {
    imp()->dev_set_view(view);
    if (view) {
      view->Show();
    }
  }
}

Tree::Tree(TreeImp* imp) : Window(imp) {}

void Tree::set_background_color(ARGB color) {
  if (imp()) {
    imp()->dev_set_background_color(color);
  }
}

ARGB Tree::background_color() const {
  return imp() ? imp()->dev_background_color() : 0xFF00000;
}

ComboBox::ComboBox(ComboBoxImp* imp) : Window(imp) {}

Edit::Edit(EditImp* imp) : Window(imp) {}

void Edit::set_text(const std::string& text) {
  if (imp()) {
    imp()->dev_set_text(text);
  }
}

std::string Edit::text() const { return imp() ? imp()->dev_text() : ""; }


Button::Button(ButtonImp* imp) : Window(imp) {}

void Button::set_text(const std::string& text) {
  if (imp()) {
    imp()->dev_set_text(text);
  }
}

std::string Button::text() const { return imp() ? imp()->dev_text() : ""; }

std::string Scintilla::dummy_str_ = "";

Scintilla::Scintilla(ScintillaImp* imp) : Window(imp) {}

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

// Ui Factory
Systems& Systems::instance() {
  static Systems instance_;
  if (!instance_.concrete_factory_.get()) {
    instance_.concrete_factory_.reset(new ui::mfc::Systems());
  }
  return instance_;
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
  assert(concrete_factory_.get());
  return concrete_factory_->CreateScrollBar(); 
}

ui::Tree* Systems::CreateTree() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateTree(); 
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
  if (window_) {
    window_->root()->WorkMouseDown(ev);
  }
}
void WindowImp::OnDevMouseUp(MouseEvent& ev) {
  if (window_) {
    assert(window_->root());
    window_->root()->WorkMouseUp(ev);
  }
}

void WindowImp::OnDevDblclick(MouseEvent& ev) {
  if (window_) {
    window_->root()->WorkDblClick(ev);
  }
}

void WindowImp::OnDevMouseMove(MouseEvent& ev) {
  if (window_) {
    window_->root()->WorkMouseMove(ev);
  }
}  

// Key Events
void WindowImp::OnDevKeyDown(KeyEvent& ev) { 
  if (window_) {
    window_->root()->WorkKeyDown(ev);
  }
}

void WindowImp::OnDevKeyUp(KeyEvent& ev) {
  if (window_) {
    window_->root()->WorkKeyDown(ev);
  }
}

void WindowImp::OnDevFocusChange(int id) {
  if (window_) {
    window_->OnFocusChange(id);
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

ui::TreeImp* ImpFactory::CreateTreeImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateTreeImp(); 
}

ui::ButtonImp* ImpFactory::CreateButtonImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateButtonImp(); 
}

ui::ScintillaImp* ImpFactory::CreateScintillaImp() {
  assert(concrete_factory_.get());
  return concrete_factory_->CreateScintillaImp(); 
}

} // namespace ui
} // namespace host
} // namespace psycle