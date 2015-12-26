// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#include <psycle/host/detail/project.private.hpp>
#include "Canvas.hpp"
#include <algorithm> // for std::transform
#include <cctype> // for std::tolower

namespace psycle {
namespace host  {
namespace ui {
namespace canvas {


class Rect : public Item {
 public:
  Rect() : Item() {
    bx_ = by_ = 0;
    width_ = height_ = 10;
    fillcolor_ = strokecolor_ = 0;
  }    

  static std::string type() { return "canvasrect"; }
  virtual void Draw(Graphics* g, Region& draw_region);

  void SetPos(double x1, double y1, double width, double height) {
    STR();
    x_ = x1;
    y_ = y1;
    width_ = width;
    height_ = height;
    FLS();
  }

  virtual void OnSize(double width, double height) {
    STR();
    width_ = width;
    height_ = height;
    FLS();
  }

  void pos(double &x, double &y, double &w, double &h) const {
    x = x_;
    y = y_;
    w = width();
    h = height();
  }
  double width() const { return width_; }
  double height() const { return height_; }
  // fill colors
  void SetFillColor(ARGB color) { fillcolor_ = color; FLS(); }
  ARGB fillcolor() const { return fillcolor_; }
  // stroke colors
  void SetStrokeColor(ARGB color) { strokecolor_ = color; FLS(); }
  ARGB strokecolor() const { return strokecolor_; }
  void SetBorder(double bx, double by) { STR(); bx_ = bx; by_ = by; FLS(); }
  void border(double &bx, double &by) const { bx = bx_; by = by_; }  
  virtual bool onupdateregion();
 private:
  void Init();
  // bool paintRect(CDC &hdc, RECT dim, COLORREF penCol, COLORREF brushCol, unsigned int opacity);
  double width_, height_, bx_, by_;
  ARGB fillcolor_, strokecolor_;
};

class Pic : public Item {
 public:
  Pic() : Item() {
    image_ = 0;
    width_ = height_ = xsrc_ = ysrc_ = 0;
    transparent_ = pmdone = false;
  }
  
  static std::string type() { return "canvaspic"; }

  virtual void Draw(Graphics* g, Region& draw_region);
  virtual bool onupdateregion();
  void OnSize(double width, double height) {
    STR();
    width_ = width;
    height_ = height;
    FLS();
  }
  void SetSource(double xsrc, double ysrc) { 
    xsrc_ = xsrc;
    ysrc_ = ysrc;
    FLS(); 
  }
  void SetImage(Image* image);
  void Size(double& width, double& height) {
    width = width_;
    height = height_;
  }  

 private:  
  double width_, height_, xsrc_, ysrc_;  
  bool transparent_, pmdone;  
  Image* image_;
};

class Line : public Item {
 public:
  Line() : Item(), color_(0) { }    
  
  static std::string type() { return "canvasline"; }

  virtual void Draw(Graphics* g, Region& draw_region);
  virtual Item::Ptr Intersect(double x, double y, Event* ev, bool &worked);  
  void SetPoints(const Points& pts) {  STR(); pts_ = pts; FLS(); }
  void SetPoint(int idx, const Point& pt) { STR(); pts_[idx] = pt; FLS(); }
  const Points& points() const { return pts_; }
  const Point& PointAt(int index) const { return pts_.at(index); }
  void SetColor(ARGB color) { color_ = color; FLS(); }
  ARGB color() const { return color_; }
  virtual bool onupdateregion();
 private:
  Points pts_;
  ARGB color_;
};

class Text : public Item {
 public:
  Text() : Item() { Init(1.0); }  
  Text(const std::string& text) : text_(text) { Init(1.0); }

  static std::string type() { return "canvastext"; }

  virtual void Draw(Graphics* cr, Region& draw_region);
  virtual bool onupdateregion();
  void SetText(const std::string& text) { STR(); text_ = text; FLS(); }
  const std::string& text() const { return text_; }
  void SetColor(ARGB color) { color_ = color; FLS(); }
  ARGB color() const { return color_; }
  void SetFont(const CFont& font) {
    LOGFONT lf;
    const_cast<CFont&>(font).GetLogFont(&lf);
    font_.DeleteObject();
    font_.CreateFontIndirect(&lf);
  }
  
 private:
  void Init(double zoom);
  std::string text_;
  ARGB color_;
  CFont font_;
  mutable int text_w, text_h;
};

// mfc wraper
template <class T>
class CWndItem : public ui::canvas::Item {
 public:  
  typedef boost::weak_ptr<CWndItem<T> > WeakPtr;
  CWndItem() :
      ui::canvas::Item(),
      id_(newid()),      
      p_wnd_(dummy_wnd()) {
    id_map_[id_] = this;
    needsupdate();  
  }  
  virtual ~CWndItem() { control_.DestroyWindow(); }

  virtual void Draw(ui::Graphics* g, ui::Region& draw_region) {                    
    double cx = zoomabsx();
    double cy = zoomabsy();
    CRect rc;
    control_.GetClientRect(&rc);
    std::stringstream str;          
    str << "cwnd drawpos" << cx << "," << cy << "," << rc.Width() << "," << "," << rc.Height() << std::endl;
		TRACE(str.str().c_str());              
  }     

  virtual void set_parent(const Item::WeakPtr& parent) { 
    canvas::Item::set_parent(parent);
    canvas::Canvas* c = root();
    if (parent.expired()) {
      p_wnd_ = dummy_wnd();
      control_.SetParent(p_wnd_);
    } else if (c && c->wnd() && c->wnd() != p_wnd_) {       
       p_wnd_ = c->wnd();
       control_.SetParent(p_wnd_);
       c->wnd()->RegisterMfcCtrl(shared_from_this(), ctrl().GetDlgCtrlID());
    }
  }
  virtual bool onupdateregion() { 
    CRect rc;  
    control_.GetClientRect(&rc);
    rgn_->SetRect(0, 0, rc.Width(), rc.Height());
    return true;
  }
  virtual void OnMessage(ui::canvas::CanvasMsg msg) {
    if (msg == ui::canvas::HIDE) {
      control_.ShowWindow(SW_HIDE);
    } 
    if (msg == ui::canvas::SHOW && visible()) {
      int cx = zoomabsx();
      int cy = zoomabsy(); 
      CRect rc;
      control_.GetClientRect(&rc);
      control_.MoveWindow(cx, cy, rc.Width(), rc.Height());
      control_.ShowWindow(SW_SHOW);     
    }
    if (msg == ui::canvas::ONWND) {
      CWnd* wnd = root()->wnd();      
      if (!wnd) {
        p_wnd_ = dummy_wnd();
        control_.SetParent(p_wnd_);        
      } else 
      if (p_wnd_!= wnd) {
        p_wnd_ = wnd;
        control_.SetParent(p_wnd_);
        // todo correct clipping
        /*control_.ModifyStyle(0, WS_CLIPSIBLINGS); 
         CRgn rgn;
         rgn.CreateRectRgn(0, 0, 10, 10);  
         control_.SetWindowRgn(rgn, true);*/
        if (visible() && IsInGroupVisible()) {
          int cx = zoomabsx();
          int cy = zoomabsy();
          CRect rc;
          control_.GetClientRect(&rc);
          control_.MoveWindow(cx, cy, rc.Width(), rc.Height());
          control_.ShowWindow(SW_SHOW);
        }
      }
    }
  }  

  virtual void SetXY(double x, double y) {  
    CRect rc;
    control_.GetClientRect(&rc);
    SetPos(x, y, rc.Width(), rc.Height());    
  }  

  virtual void OnSize(double w, double h) {
    SetPos(x_, y_, w, h); 
  }

  virtual void SetPos(double x, double y, double w, double h) {
    CRect rc;
    control_.GetClientRect(&rc);
    int cx = zoomabsx() + (x-x_);
    int cy = zoomabsy() + (y-y_);
    if (cx != rc.left || cy != rc.top || w != rc.Width() || h != rc.Height()) { 
      STR();
      x_ = x;
      y_ = y; 
      ctrl().MoveWindow(cx, cy, w, h); //, SWP_NOREDRAW);    
      FLS();
    }
  }
  virtual void Show() { 
    Item::Show();
    if (!ctrl().IsWindowVisible()) {
      ctrl().ShowWindow(SW_SHOW);
    }
  }
  virtual void Hide() {
    Item::Hide();
    if (ctrl().IsWindowVisible()) {
      ctrl().ShowWindow(SW_HIDE);
    }
  }
  CWnd* p_wnd() { return p_wnd_; }
  const CWnd* p_wnd() const { return p_wnd_; }    
  int id() const { return id_; }    
  static CWndItem<T>* FindById(int id) {
    std::map<std::uint16_t, CWndItem<T>*>::const_iterator it 
      = id_map_.find(id);
    return it!=id_map_.end() ? it->second : 0;
  }
 
  T& ctrl() { return control_; }
  const T& ctrl() const { return control_; }  

  
 protected:
  static int newid() { return id_counter++; }  
  virtual void OnFocus() { ctrl().SetFocus(); }
  virtual void OnKillFocus() { ::SetFocus(NULL); }
 private:
   static CWnd* dummy_wnd() {
     if (!dummy_wnd_.m_hWnd) {   
       dummy_wnd_.Create(0, "psycleuidummywnd", 0, CRect(0, 0, 0, 0), ::AfxGetMainWnd(), 0);       
     }
     return &dummy_wnd_;
   }
  int id_;

  T control_;  
  static int id_counter;
  static std::map<std::uint16_t, CWndItem<T>*> id_map_;
  static CWnd dummy_wnd_;
  CWnd* p_wnd_;
};

template <typename T>
CWnd CWndItem<T>::dummy_wnd_;

template <typename T>
int CWndItem<T>::id_counter = ID_DYNAMIC_CONTROLS_BEGIN;

template <typename T>
std::map<std::uint16_t, CWndItem<T>*> CWndItem<T>::id_map_;


class Button : public CWndItem<CButton> {
 public:
  static std::string type() { return "canvasbuttonitem"; }
  Button() : CWndItem<CButton>() { 
    ctrl().Create("btn", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | DT_CENTER,
      CRect(0, 0, 55, 19), p_wnd(), id());   
  }  
  virtual void OnClick() {}  
};

class ComboBox : public CWndItem<CComboBox> {
 public:
  static std::string type() { return "canvascomboboxitem"; }
  ComboBox() : CWndItem<CComboBox>() { 
    ctrl().Create(WS_CHILD|WS_VSCROLL|CBS_DROPDOWNLIST,
   CRect(10,10,200,100), p_wnd(), id());
    ctrl().AddString("test1");
    ctrl().AddString("test2");
  }    
};

class CTree : public CTreeCtrl {
public:
   boost::signal<void (HTREEITEM)> onclick;
protected:
  DECLARE_DYNAMIC(CTree)     
  BOOL OnClick(NMHDR * pNotifyStruct, LRESULT * result) {
    HTREEITEM hTtreeSelectedItem = GetSelectedItem();
    onclick(hTtreeSelectedItem);   
    return FALSE;
  }
  DECLARE_MESSAGE_MAP();   
};


class TreeItem;

class Tree : public CWndItem<CTree> {
 public:  
  static std::string type() { return "canvastreeitem"; }  
  Tree() : CWndItem<CTree>() { 
    ctrl().Create(WS_CHILD, CRect(10,10,200,100), p_wnd(), id());    
    #pragma warning(disable:4996)
    ctrl().onclick.connect(bind(&Tree::onclick, this,  _1));
  }   
  ~Tree() {
  }

  void onclick(HTREEITEM hItem);

  template<class T>
  void InsertItem(T& item) {
    item->set_tree(boost::dynamic_pointer_cast<Tree>(shared_from_this()));
    item->set_parent(TreeItem::WeakPtr());  
    item->tvInsert.hParent = NULL;
    item->tvInsert.hInsertAfter = NULL;
    item->hItem = ctrl().InsertItem(&item->tvInsert);
    item->UpdateMfc();
    children_.push_back(item);
  }

  void Clear() {
    ctrl().DeleteAllItems();
    children_.clear();
  }

  std::list<boost::shared_ptr<TreeItem> > SubChildren();    
  std::list<boost::shared_ptr<TreeItem> > children_;
};

class TreeItem : public boost::enable_shared_from_this<TreeItem> {
 friend class Tree;
 public:
 
  typedef boost::shared_ptr<TreeItem> Ptr;
  typedef boost::weak_ptr<TreeItem> WeakPtr;
  typedef std::list<TreeItem::Ptr> TreeItemList; 

  TreeItem() {
    tvInsert.hParent = NULL;
    tvInsert.hInsertAfter = NULL;
    tvInsert.item.mask = TVIF_TEXT;
    tvInsert.item.pszText = _T("");
    hItem = 0;
  }

  virtual ~TreeItem() { }

  virtual void OnClick() {   }

  void InsertItem(TreeItem::Ptr item) {
    if (item) {
      item->parent_ = shared_from_this(); 
      children_.push_back(item);
      if (hItem != NULL) {
        InsertMfc(item);      
      }
    }
  }
      
  TreeItem::WeakPtr root() { 
    TreeItem::Ptr item = shared_from_this();
    do {
      if (!item->parent_.expired()) {
        item = item->parent_.lock();
      } else {
        break;
      }
    } while (true);  
    return item;
  }
  
  std::list<TreeItem::Ptr> children() { return children_; };
  void set_parent(TreeItem::WeakPtr parent) { parent_ = parent; } 
  TreeItem::WeakPtr parent() { return parent_; } 
  Tree::WeakPtr tree() { return tree_; };
  void set_tree(Tree::WeakPtr tree) { tree_ = tree; };

  TreeItemList SubChildren() {
    TreeItemList allitems;
    TreeItemList::iterator it = children_.begin();
    for (; it != children_.end(); ++it) {
      TreeItem::Ptr item = *it;
      allitems.push_back(item);
      TreeItemList subs = item->SubChildren();
      TreeItemList::iterator itsub = subs.begin();
      for (; itsub != subs.end(); ++itsub) {
        allitems.push_back(*it);
      }
    }
    return allitems;
  }

protected:
  TVINSERTSTRUCT tvInsert; 
  HTREEITEM hItem;

private:
  void InsertMfc(TreeItem::Ptr item) {
    if (!root().expired() && !root().lock()->tree_.expired()) {
      item->tvInsert.hParent = hItem;
      item->tvInsert.hInsertAfter = hItem;
      item->hItem = root().lock()->tree_.lock()->ctrl().InsertItem(&item->tvInsert);
      item->UpdateMfc();
    }
  }

  void UpdateMfc() {
    TreeItemList::iterator it = children_.begin();
    for (; it != children_.end(); ++it) {
      InsertMfc(*it);              
    }
  }

  TreeItemList children_;
  TreeItem::WeakPtr parent_; 
  Tree::WeakPtr tree_;
};

class TextTreeItem : public TreeItem {
 public:
  typedef boost::shared_ptr<TextTreeItem> Ptr;
  static std::string type() { return "canvastexttreeitem"; }
  TextTreeItem() {
    tvInsert.item.mask = TVIF_TEXT;
    tvInsert.item.pszText = const_cast<char *>(text_.c_str());
  }
  ~TextTreeItem() { }

  void set_text(const std::string& text) {
    text_ = text;
    tvInsert.item.pszText = const_cast<char *>(text_.c_str());
  }
  const std::string& text() const {  return text_; }

 private:
   std::string text_;
};

class Scintilla : public CWndItem<CScintilla> {
 public:
  typedef CScintilla CTRL;
  static std::string type() { return "canvasscintillaitem"; }
  Scintilla() : CWndItem<CScintilla>() {
    ctrl().modified.connect(boost::bind(&Scintilla::OnFirstModified, this));
    ctrl().Create(p_wnd(), id());    
    f(SCI_SETMARGINWIDTHN, 0, 32);
  }  
  
  template<class T, class T1>
  int f(int sci_cmd, T lparam, T1 wparam) {     
    return ctrl().f(sci_cmd, (WPARAM) lparam, (LPARAM) wparam);
  }
  void AddText(const std::string& text) { ctrl().AddText(text); }
  void FindText(const std::string& text, int cpmin, int cpmax, int& pos, int& cpselstart, int& cpselend) const { 
    return ctrl().FindText(text, cpmin, cpmax, pos, cpselstart, cpselend);
  }
  void GotoLine(int pos) { ctrl().GotoLine(pos); }
  int length() const { return ctrl().length(); }
  int selectionstart() const { return ctrl().selectionstart(); }
  int selectionend() const { return ctrl().selectionend(); }
  void SetSel(int cpmin, int cpmax) { ctrl().SetSel(cpmin, cpmax); }
  bool has_selection() const { return ctrl().has_selection(); }
  void set_find_match_case(bool on) { ctrl().set_find_match_case(on); }
  void set_find_whole_word(bool on) { ctrl().set_find_whole_word(on); }
  void set_find_regexp(bool on) { ctrl().set_find_regexp(on); }
  void LoadFile(const std::string& filename) { ctrl().LoadFile(filename); }      
  void SaveFile(const std::string& filename) { ctrl().SaveFile(filename); }  
  bool has_file() const { return ctrl().has_file(); }
  const std::string& filename() const { return ctrl().filename(); }
  bool is_modified() const { return ctrl().is_modified(); } 
  virtual void OnFirstModified() {}

};

class Edit : public CWndItem<CEdit> {
 public:
  static std::string type() { return "canvasedititem"; }
  Edit() : CWndItem<CEdit>() {
    ctrl().Create(WS_CHILD | WS_TABSTOP, CRect(0, 0, 100, 20), p_wnd(), id());
    //ctrl().SetWindowRgn((HRGN)rgn_->source(), false);
  }  
  void SetText(const std::string& text) { ctrl().SetWindowText(text.c_str()); }
  std::string text() const {    
    CString s;    
    ctrl().GetWindowTextA(s);
    return s.GetString();
  }  
};

class ScrollBar : public CWndItem<CScrollBar> {
 public:
  static std::string type() { return "canvasscrollbaritem"; }
  ScrollBar() : CWndItem<CScrollBar>() {    
    ctrl().Create(SBS_VERT | WS_CHILD | WS_VISIBLE, CRect(0, 0, 100, 20), p_wnd(), id());
    SetPos(0, 0, GetSystemMetrics(SM_CXVSCROLL), 100);
  }  
  virtual void OnScroll(int pos) {}  
  void SetScrollRange(int minpos, int maxpos) { 
    ctrl().SetScrollRange(minpos, maxpos);
  }
  void scroll_range(int& minpos, int& maxpos) const { 
    ctrl().GetScrollRange(&minpos, &maxpos);
  }
  void SetOrientation(Orientation orientation) {    
    DWORD cs = (ctrl().GetStyle() & !SB_HORZ & !SB_VERT) | orientation;
    SCROLLINFO si;
    ctrl().GetScrollInfo(&si);
    CRect rc;
    ctrl().GetClientRect(&rc);
    if (orientation == HORZ) {
      rc.right = rc.Width();
      rc.bottom = GetSystemMetrics(SM_CXHSCROLL); 
    } else {
      rc.right = GetSystemMetrics(SM_CXVSCROLL);
      rc.bottom = rc.Height();
    }    
    ctrl().DestroyWindow();        
    ctrl().Create(cs, rc, p_wnd(), id());
    SetPos(rc.left, rc.top, rc.Width(), rc.Height());
  }
  void SetScrollPos(int pos) { ctrl().SetScrollPos(pos); }
  int scroll_pos() const { return ctrl().GetScrollPos(); }

  void SystemSize(int& width, int& height) const {
    width = GetSystemMetrics(SM_CXVSCROLL);
    height = GetSystemMetrics(SM_CXHSCROLL);
  }  
};


} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle