//#include "stdafx.h"

#include "MfcUi.hpp"
#include "Mmsystem.h"
#include "resources/resources.hpp"
// #include "Resource.h"

namespace psycle {
namespace host {
namespace ui {
namespace mfc {


void GraphicsImp::FillRegion(const ui::Region& rgn) {    
  check_pen_update();
  check_brush_update();        
  mfc::RegionImp* imp = dynamic_cast<mfc::RegionImp*>(rgn.imp());
  assert(imp);
  cr_->FillRgn(&imp->crgn(), &brush);
}

void GraphicsImp::SetClip(ui::Region* rgn) {
  if (rgn) {
    mfc::RegionImp* imp = dynamic_cast<mfc::RegionImp*>(rgn->imp());
    assert(imp);
    cr_->SelectClipRgn(&imp->crgn());
  } else {
    cr_->SelectClipRgn(0);
  }  
}

RegionImp::RegionImp() { rgn_.CreateRectRgn(0, 0, 0, 0); }
  
RegionImp::RegionImp(const CRgn& rgn) {
  assert(rgn.m_hObject);
  rgn_.CreateRectRgn(0, 0, 0, 0);
  rgn_.CopyRgn(&rgn);
}

RegionImp::~RegionImp() {
  rgn_.DeleteObject();
}

RegionImp* RegionImp::DevClone() const {
  return new RegionImp(rgn_);  
}
  
void RegionImp::DevOffset(double dx, double dy) {
  CPoint pt(dx, dy);
  rgn_.OffsetRgn(pt);
}

int RegionImp::DevCombine(const ui::Region& other, int combinemode) {
  mfc::RegionImp* imp = dynamic_cast<mfc::RegionImp*>(other.imp());
  assert(imp);
  return rgn_.CombineRgn(&rgn_, &imp->crgn(), combinemode);
} 

ui::Rect RegionImp::DevBounds() const { 
  CRect rc;
  rgn_.GetRgnBox(&rc);
  return ui::Rect(ui::Point(rc.left, rc.top), ui::Point(rc.right, rc.bottom));
}

bool RegionImp::DevIntersect(double x, double y) const {
  return rgn_.PtInRegion(x, y);  
}

bool RegionImp::DevIntersectRect(const ui::Rect& rect) const {
  CRect rc(rect.left(), rect.top(), rect.right(), rect.bottom());
  return rgn_.RectInRegion(rc);  
}
  
void RegionImp::DevSetRect(const ui::Rect& rect) {
  rgn_.SetRectRgn(rect.left(), rect.top(), rect.right(), rect.bottom()); 
}

void RegionImp::DevClear() {
  DevSetRect(ui::Rect());  
}

  
std::map<HWND, ui::WindowImp*> WindowHook::windows_;
HHOOK WindowHook::_hook = 0;
int WindowID::id_counter = ID_DYNAMIC_CONTROLS_BEGIN;
CWnd DummyWindow::dummy_wnd_;

#define BEGIN_TEMPLATE_MESSAGE_MAP2(theClass, type_name, type_name2, baseClass)			\
	PTM_WARNING_DISABLE														\
	template < typename type_name, typename type_name2 >											\
	const AFX_MSGMAP* theClass< type_name, type_name2 >::GetMessageMap() const			\
		{ return GetThisMessageMap(); }										\
	template < typename type_name, typename type_name2 >											\
	const AFX_MSGMAP* PASCAL theClass< type_name, type_name2 >::GetThisMessageMap()		\
	{																		\
		typedef theClass< type_name, type_name2 > ThisClass;							\
		typedef baseClass TheBaseClass;										\
		static const AFX_MSGMAP_ENTRY _messageEntries[] =					\
		{

// CanvasView
BEGIN_TEMPLATE_MESSAGE_MAP2(WindowTemplateImp, T, I, T)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  //ON_WM_SETFOCUS()
  ON_WM_KILLFOCUS()
	ON_WM_PAINT()
  ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
  ON_WM_MOUSELEAVE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
  ON_WM_KEYDOWN()
	ON_WM_KEYUP()
  ON_WM_SETCURSOR()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_SIZE()  
  ON_WM_MOUSEACTIVATE()
END_MESSAGE_MAP()

template<class T, class I>
BOOL WindowTemplateImp<T, I>::PreTranslateMessage(MSG* pMsg) {  
  if (pMsg->message==WM_KEYDOWN ) {
    UINT nFlags = 0;
    UINT flags = Win32KeyFlags(nFlags);      
    KeyEvent ev(pMsg->wParam, flags);    
    return WorkEvent(ev, &Window::OnKeyDown, window(), pMsg);
  } else
  if (pMsg->message == WM_KEYUP) {
    UINT nFlags = 0;
    UINT flags = Win32KeyFlags(nFlags);      
    KeyEvent ev(pMsg->wParam, flags);      
    return WorkEvent(ev, &Window::OnKeyUp, window(), pMsg);    
  } else
  if (pMsg->message == WM_MOUSELEAVE) {
    mouse_enter_ = true;
    CPoint pt(pMsg->pt);
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + this->dev_abs_pos().left(), pt.y - rc.top + this->dev_abs_pos().top(), 1, pMsg->wParam);
    return WorkEvent(ev, &Window::OnMouseOut, window(), pMsg);    
  }  else
  if (pMsg->message == WM_LBUTTONDOWN) {
    CPoint pt(pMsg->pt);        
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + this->dev_abs_pos().left(), pt.y - rc.top + this->dev_abs_pos().top(), 1, pMsg->wParam);    
    return WorkEvent(ev, &Window::OnMouseDown, window(), pMsg);
  } else
  if (pMsg->message == WM_LBUTTONDBLCLK) {
    CPoint pt(pMsg->pt);        
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + this->dev_abs_pos().left(), pt.y - rc.top + this->dev_abs_pos().top(), 1, pMsg->wParam);    
    return WorkEvent(ev, &Window::OnDblclick, window(), pMsg);
  } else
  if (pMsg->message == WM_LBUTTONUP) {
    CPoint pt(pMsg->pt);        
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + this->dev_abs_pos().left(), pt.y - rc.top + this->dev_abs_pos().top(), 1, pMsg->wParam);    
    return WorkEvent(ev, &Window::OnMouseUp, window(), pMsg);
  } else
  if (pMsg->message == WM_RBUTTONDOWN) {
    CPoint pt(pMsg->pt);        
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + this->dev_abs_pos().left(), pt.y - rc.top + this->dev_abs_pos().top(), 2, pMsg->wParam);    
    return WorkEvent(ev, &Window::OnMouseDown, window(), pMsg);
  } else    
  if (pMsg->message == WM_RBUTTONDBLCLK) {
    CPoint pt(pMsg->pt);        
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + this->dev_abs_pos().left(), pt.y - rc.top + this->dev_abs_pos().top(), 2, pMsg->wParam);    
    return WorkEvent(ev, &Window::OnDblclick, window(), pMsg);
  } else
  if (pMsg->message == WM_RBUTTONUP) {
    CPoint pt(pMsg->pt);        
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + this->dev_abs_pos().left(), pt.y - rc.top + this->dev_abs_pos().top(), 2, pMsg->wParam);    
    return WorkEvent(ev, &Window::OnMouseUp, window(), pMsg);
  } else  
  if (pMsg->message == WM_MOUSEMOVE) {
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = TME_LEAVE;
    tme.dwHoverTime = 1;    
    m_bTracking = _TrackMouseEvent(&tme);     
    CPoint pt(pMsg->pt);
    CRect rc;
    GetWindowRect(&rc);    
    MouseEvent ev(pt.x - rc.left + this->dev_abs_pos().left(), pt.y - rc.top + this->dev_abs_pos().top(), 1, pMsg->wParam);
    if (mouse_enter_) {
      mouse_enter_ = false;
      if (window()) {
        try {
          window()->OnMouseEnter(ev);
        } catch (std::exception& e) {
          ::AfxMessageBox(e.what());      
        }      
      }
    }
    return WorkEvent(ev, &Window::OnMouseMove, window(), pMsg);    
  }
  return CWnd::PreTranslateMessage(pMsg);
}

/*template<class T, class I>
void WindowTemplateImp<T, I>::OnHScroll(UINT a, UINT b, CScrollBar* pScrollBar) {
  ScrollBarImp* sb = (ScrollBarImp*) pScrollBar;
  sb->OnDevScroll(a);
}*/

template<class T, class I>
BOOL WindowTemplateImp<T, I>::prevent_propagate_event(ui::Event& ev, MSG* pMsg) {
  if (!::IsWindow(m_hWnd)) {
    return true;
  }  
  if (!ev.is_default_prevented()) {
     pMsg->hwnd = GetSafeHwnd();
    ::TranslateMessage(pMsg);          
	  ::DispatchMessage(pMsg);        
  }
  return ev.is_propagation_stopped();  
}

template<class T, class I>
void WindowTemplateImp<T, I>::OnKillFocus(CWnd* pNewWnd) {  
}

template<class T, class I>
int WindowTemplateImp<T, I>::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}
  WindowHook::windows_[GetSafeHwnd()] = this;
  // Set the hook
  WindowHook::SetFocusHook();  
	return 0;
}

template<class T, class I>
void WindowTemplateImp<T, I>::OnDestroy() {  
  std::map<HWND, ui::WindowImp*>::iterator it = WindowHook::windows_.find(GetSafeHwnd());
  if (it != WindowHook::windows_.end()) {
    WindowHook::windows_.erase(it);
  }
  bmpDC.DeleteObject();
}

template<class T, class I>
void WindowTemplateImp<T, I>::dev_set_pos(const ui::Rect& pos) {
  dev_pos_.set_xy(pos.left(), pos.top());
  SetWindowPos(0, 
               pos.left(),
               pos.top(),
               pos.width(),
               pos.height(),            
               SWP_NOREDRAW |
               SWP_NOZORDER |
               SWP_NOACTIVATE
               // SWP_ASYNCWINDOWPOS
               );
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::dev_pos() const {
  CRect rc;
  GetWindowRect(&rc);
  return ui::Rect(dev_pos_, ui::Dimension(rc.Width(), rc.Height()));
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::dev_abs_pos() const {
  CRect rc;
  GetClientRect(&rc);
  CPoint abs_pos(rc.left, rc.top);
  MapPointToRoot(abs_pos);
  return ui::Rect(ui::Point(abs_pos.x, abs_pos.y),
                  ui::Dimension(rc.Width(), rc.Height()));
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::dev_desktop_pos() const {
  CRect rc;
  GetClientRect(&rc);  
  CPoint pos(rc.left, rc.top);
  MapPointToDesktop(pos);
  return ui::Rect(ui::Point(pos.x, pos.y),
                  ui::Dimension(rc.Width(), rc.Height()));
}

template<class T, class I>
void WindowTemplateImp<T, I>::dev_set_parent(Window* parent) {  
  if (parent && parent->imp()) {    
    SetParent(dynamic_cast<CWnd*>(parent->imp()));    
    ShowWindow(SW_SHOW | SW_SHOWNOACTIVATE);    
  } else {
    SetParent(DummyWindow::dummy());
  }
}

template<class T, class I>
void WindowTemplateImp<T, I>::OnPaint() {
  CRgn rgn;
  rgn.CreateRectRgn(0, 0, 0, 0);
	int result = GetUpdateRgn(&rgn, FALSE);

  if (!result) return; // If no area to update, exit.
	
	CPaintDC dc(this);

  /*if (!bmpDC.m_hObject) { // buffer creation	
		CRect rc;
		GetClientRect(&rc);		
		bmpDC.CreateCompatibleBitmap(&dc, rc.right - rc.left, rc.bottom - rc.top);
		char buf[128];
		sprintf(buf,"CanvasView::OnPaint(). Initialized bmpDC to 0x%p\n",(void*)bmpDC);
		TRACE(buf);
	}
  CDC bufDC;*/
	//bufDC.CreateCompatibleDC(&dc);
	//CBitmap* oldbmp = bufDC.SelectObject(&bmpDC);	
  ui::Graphics g(&dc);
  ui::Region draw_rgn(new ui::mfc::RegionImp(rgn));
  ui::Rect bounds = draw_rgn.bounds();
  OnDevDraw(&g, draw_rgn);
  g.Dispose();
  //CRect rc;
  //GetClientRect(&rc);
	//dc.BitBlt(0, 0, rc.right-rc.left, rc.bottom-rc.top, &bufDC, 0, 0, SRCCOPY);
	//bufDC.SelectObject(oldbmp);
	//bufDC.DeleteDC();
  rgn.DeleteObject();    
}

template<class T, class I>
void WindowTemplateImp<T, I>::OnSize(UINT nType, int cw, int ch) {  
  if (bmpDC.m_hObject != NULL) { // remove old buffer to force recreating it with new size
	  TRACE("CanvasView::OnResize(). Deleted bmpDC\n");
	  bmpDC.DeleteObject();	  
  }  
  OnDevSize(ui::Dimension(cw, ch));
  CWnd::OnSize(nType, cw, ch);
}

template<class T, class I>
bool WindowTemplateImp<T, I>::OnDevUpdateArea(ui::Area& area) {
  CRect pos;
  GetClientRect(&pos);
  area.Clear();
  area.Add(RectShape(ui::Rect(ui::Point(), ui::Point(pos.Width(), pos.Height()))));
  return true;
}

template<class T, class I>
ui::Window* WindowTemplateImp<T, I>::dev_focus_window() {
  Window* result = 0;
  HWND hwnd = ::GetFocus();
  if (hwnd) {
    std::map<HWND, ui::WindowImp*>::iterator it;
    do {
      it = WindowHook::windows_.find(hwnd);
      if (it != WindowHook::windows_.end()) {
        result = it->second->window();
        break;
      }
      hwnd = ::GetParent(hwnd);
    } while (hwnd);
  }
  return result;
}

void WindowImp::DevSetCursor(CursorStyle style) {
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

template class WindowTemplateImp<CWnd, ui::WindowImp>;
template class WindowTemplateImp<CComboBox, ui::ComboBoxImp>;
template class WindowTemplateImp<CScrollBar, ui::ScrollBarImp>;
template class WindowTemplateImp<CButton, ui::ButtonImp>;
template class WindowTemplateImp<CButton, ui::CheckBoxImp>;
template class WindowTemplateImp<CEdit, ui::EditImp>;
template class WindowTemplateImp<CWnd, ui::ScintillaImp>;
template class WindowTemplateImp<CTreeCtrl, ui::TreeViewImp>;
template class WindowTemplateImp<CListCtrl, ui::ListViewImp>;
template class WindowTemplateImp<CFrameWnd, ui::FrameImp>;

BEGIN_MESSAGE_MAP(WindowImp, CWnd)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  //ON_WM_SETFOCUS()
	ON_WM_PAINT()
  ON_WM_ERASEBKGND()
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
END_MESSAGE_MAP()


BEGIN_MESSAGE_MAP(FrameImp, CFrameWnd)
  ON_WM_SIZE()
	ON_WM_PAINT()
  ON_WM_CLOSE()
  ON_WM_HSCROLL()
  ON_WM_ERASEBKGND()
  ON_WM_KILLFOCUS()
  ON_WM_SETFOCUS()
  ON_WM_NCRBUTTONDOWN()
END_MESSAGE_MAP()

BOOL FrameImp::PreTranslateMessage(MSG* pMsg) {
  if (pMsg->message==WM_NCRBUTTONDOWN) {    
    ui::Event ev;
    ui::Point point(pMsg->pt.x, pMsg->pt.y);
    ((Frame*)window())->WorkOnContextPopup(ev, point);    
    return ev.is_propagation_stopped();    
  }
  return WindowTemplateImp<CFrameWnd, ui::FrameImp>::PreTranslateMessage(pMsg);
}

void FrameImp::DevShowDecoration() {
  ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
  ModifyStyle(0, WS_CAPTION, SWP_FRAMECHANGED); 
  ModifyStyle(0, WS_BORDER, SWP_FRAMECHANGED);
  ModifyStyle(0, WS_THICKFRAME, SWP_FRAMECHANGED);
}

void FrameImp::DevHideDecoration() {
  ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);
  ModifyStyle(WS_CAPTION, 0, SWP_FRAMECHANGED); 
  ModifyStyle(WS_BORDER, 0, SWP_FRAMECHANGED);
  ModifyStyle(WS_THICKFRAME, 0, SWP_FRAMECHANGED);
}

void FrameImp::DevPreventResize() {
  ModifyStyle(WS_SIZEBOX, 0, SWP_FRAMECHANGED);
}

void FrameImp::DevAllowResize() {  
  ModifyStyle(0, WS_SIZEBOX, SWP_FRAMECHANGED);
}

ui::FrameImp* PopupFrameImp::popup_frame_ = 0;

LRESULT CALLBACK MouseHook(int Code, WPARAM wParam, LPARAM lParam) {
    if (PopupFrameImp::popup_frame_ && 
       (wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONDBLCLK)) {         
        PopupFrameImp::popup_frame_->DevHide();      
    }    
    return CallNextHookEx(NULL, Code, wParam, lParam);
}

void PopupFrameImp::DevShow() {
  popup_frame_ = 0;  
  mouse_hook_ = SetWindowsHookEx(WH_MOUSE, MouseHook, 0, GetCurrentThreadId());
  ShowWindow(SW_SHOWNOACTIVATE);    
  popup_frame_ = this;
}

void PopupFrameImp::DevHide() {
  PopupFrameImp::popup_frame_ = 0;
  FrameImp::DevHide();  
  if (mouse_hook_) {
    UnhookWindowsHookEx(mouse_hook_);
  }
  mouse_hook_ = 0;  
}

BEGIN_MESSAGE_MAP(ButtonImp, CButton)  
	ON_WM_PAINT()  
  ON_CONTROL_REFLECT(BN_CLICKED, OnClick)
END_MESSAGE_MAP()

void ButtonImp::OnClick() {
  OnDevClick();
}

BEGIN_MESSAGE_MAP(CheckBoxImp, CButton)  
	ON_WM_PAINT()  
  ON_CONTROL_REFLECT(BN_CLICKED, OnClick)
END_MESSAGE_MAP()

void CheckBoxImp::OnClick() {
  OnDevClick();
}

BEGIN_MESSAGE_MAP(ComboBoxImp, CComboBox)  
	ON_WM_PAINT()
  ON_CONTROL_REFLECT_EX(CBN_SELENDOK, OnSelect)
END_MESSAGE_MAP()

BOOL ComboBoxImp::prevent_propagate_event(ui::Event& ev, MSG* pMsg) {
  if (!::IsWindow(m_hWnd)) {
    return true;
  }
  if (pMsg->message == WM_LBUTTONDOWN) {
    ev.StopPropagation(); 
  }
  if (!ev.is_default_prevented()) {    
    ::TranslateMessage(pMsg);          
	  ::DispatchMessage(pMsg);        
  }
  return ev.is_propagation_stopped();  
}

BOOL ComboBoxImp::OnSelect() {  
  ui::ComboBox* combo_box = dynamic_cast<ui::ComboBox*>(window());
  assert(combo_box);
  combo_box->select(*combo_box);
  combo_box->OnSelect();
  return FALSE;
}

BEGIN_MESSAGE_MAP(EditImp, CEdit)  
	ON_WM_PAINT()
END_MESSAGE_MAP()

HTREEITEM TreeNodeImp::DevInsert(TreeViewImp* tree, const ui::Node& node, TreeNodeImp* prev_imp) {  
  TVINSERTSTRUCT tvInsert;
  tvInsert.hParent = hItem;
  tvInsert.hInsertAfter = prev_imp ? prev_imp->hItem : TVI_LAST;
  tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;  
  tvInsert.item.iImage = node.image_index();
  tvInsert.item.iSelectedImage =  node.selected_image_index();   
  text_ = node.text();
  tvInsert.item.pszText = const_cast<char *>(text_.c_str());  
  return tree->InsertItem(&tvInsert);
}

BEGIN_MESSAGE_MAP(TreeViewImp, CTreeCtrl)  
  ON_WM_ERASEBKGND()
	ON_WM_PAINT()  
  ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnChange)
  ON_NOTIFY_REFLECT_EX(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
  ON_NOTIFY_REFLECT_EX(TVN_ENDLABELEDIT, OnEndLabelEdit)
  ON_NOTIFY_REFLECT_EX(NM_RCLICK, OnRightClick)
  ON_NOTIFY_REFLECT_EX(NM_DBLCLK, OnDblClick)  
END_MESSAGE_MAP()

BOOL TreeViewImp::prevent_propagate_event(ui::Event& ev, MSG* pMsg) {  
  if (!::IsWindow(m_hWnd)) {
    return true;
  }    
  if (!ev.is_default_prevented()) {
    if (pMsg->message == WM_KEYUP || pMsg->message == WM_KEYDOWN) {
      pMsg->hwnd = ::GetFocus();
    }
    ::TranslateMessage(pMsg);          
	  ::DispatchMessage(pMsg);            
  }
  pMsg->hwnd = GetSafeHwnd();
  return ev.is_propagation_stopped();  
}

void TreeViewImp::UpdateNode(boost::shared_ptr<Node> node, boost::shared_ptr<Node> prev_node) {  
  NodeImp* prev_node_imp = prev_node ? prev_node->imp(*this) : 0;
  boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
  node->erase_imp(this);  
  TreeNodeImp* new_imp = new TreeNodeImp();
  node->AddImp(new_imp);
  new_imp->set_owner(this);
  node->changed.connect(boost::bind(&TreeViewImp::OnNodeChanged, this, _1));
  if (!node->parent().expired()) {
    boost::shared_ptr<ui::Node> parent_node = node->parent().lock();
    TreeNodeImp* parent_imp = dynamic_cast<TreeNodeImp*>(parent_node->imp(*this));
    if (parent_imp) {
      TreeNodeImp* prev_imp = dynamic_cast<TreeNodeImp*>(prev_node_imp);
       new_imp->hItem = parent_imp->DevInsert(this, *node.get(), prev_imp);
       htreeitem_node_map_[new_imp->hItem] = node;      
    }
  }
}

void TreeViewImp::DevUpdate(const Node::Ptr& node, boost::shared_ptr<Node> prev_node) {  
  recursive_node_iterator end = node->recursive_end();
  recursive_node_iterator it = node->recursive_begin();
  boost::shared_ptr<Node> prev = prev_node;
  UpdateNode(node, prev);
  for ( ; it != end; ++it) {
    UpdateNode((*it), prev);
    prev = *it;
  }
}

void TreeViewImp::DevErase(boost::shared_ptr<Node> node) {  
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node->imp(*this));
  if (imp) {
    DeleteItem(imp->hItem);
  } 
}

void TreeViewImp::DevEditNode(boost::shared_ptr<ui::Node> node) { 
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node->imp(*this));
  if (imp) {
    EditLabel(imp->hItem);    
  }
}

void TreeViewImp::dev_select_node(const boost::shared_ptr<ui::Node>& node) {
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node->imp(*this));
  if (imp) {  
    SelectItem(imp->hItem);    
  }
}

boost::weak_ptr<Node> TreeViewImp::dev_selected() {
  ui::Node* node = find_selected_node();
  return node ? node->shared_from_this() : boost::weak_ptr<ui::Node>();
}

void TreeViewImp::OnNodeChanged(Node& node) {
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node.imp(*this));  
  if (imp) {      
    SetItemText(imp->hItem, node.text().c_str());   
  }
}

BOOL TreeViewImp::OnChange(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    tree_view()->OnChange(node->shared_from_this());
  }
  return FALSE;
}

BOOL TreeViewImp::OnRightClick(NMHDR * pNotifyStruct, LRESULT * result) {
  POINT pt;
  ::GetCursorPos(&pt);
  Node::Ptr node = dev_node_at(ui::Point(pt.x, pt.y));
  ui::Event ev;
  tree_view()->WorkOnContextPopup(ev, ui::Point(pt.x, pt.y), node);
  tree_view()->OnRightClick(node);  
  return FALSE;
}

BOOL TreeViewImp::OnDblClick(NMHDR * pNotifyStruct, LRESULT * result) {
  CPoint pt;
  ::GetCursorPos(&pt);
  ScreenToClient(&pt);  
  MouseEvent ev(pt.x, pt.y, 1, 0);    
  if (window()) {
    window()->OnDblclick(ev);    
  }
  return FALSE;
}

ui::Node::Ptr TreeViewImp::dev_node_at(const ui::Point& pos) const {
  CPoint p(pos.x(), pos.y());
  ScreenToClient(&p);
  HTREEITEM item = HitTest(p);  
  if (item) {
    std::map<HTREEITEM, boost::weak_ptr<ui::Node> >::const_iterator it;
    it = htreeitem_node_map_.find(item);
    return (it != htreeitem_node_map_.end()) ? it->second.lock() : nullpointer;
  }
  return nullpointer;
}

ui::Node* TreeViewImp::find_selected_node() {  
  std::map<HTREEITEM, boost::weak_ptr<ui::Node> >::iterator it;
  it = htreeitem_node_map_.find(GetSelectedItem());
  return (it != htreeitem_node_map_.end()) ? it->second.lock().get() : 0;
}

BOOL TreeViewImp::OnBeginLabelEdit(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    CEdit* edit = GetEditControl();
    CString s;    
    edit->GetWindowTextA(s);
    is_editing_ = true;
    tree_view()->OnEditing(node->shared_from_this(), s.GetString());
  }
  return FALSE;
}


BOOL TreeViewImp::OnEndLabelEdit(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    CEdit* edit = GetEditControl();
    CString s;    
    edit->GetWindowTextA(s);
    is_editing_ = false;
    tree_view()->OnEdited(node->shared_from_this(), s.GetString());
  }
  return FALSE;
}

void TreeViewImp::DevClear() { DeleteAllItems(); }

void TreeViewImp::DevShowLines() {
  ModifyStyle(0, TVS_HASLINES | TVS_LINESATROOT);
}

void TreeViewImp::DevHideLines() {
  ModifyStyle(TVS_HASLINES | TVS_LINESATROOT, 0);
}

void TreeViewImp::DevShowButtons() {
  ModifyStyle(0, TVS_HASBUTTONS);
}

void TreeViewImp::DevHideButtons() {
  ModifyStyle(TVS_HASBUTTONS, 0);
}


BEGIN_MESSAGE_MAP(ListViewImp, CListCtrl)
  ON_WM_ERASEBKGND()
	ON_WM_PAINT()  
  ON_NOTIFY_REFLECT_EX(LVN_ITEMCHANGED, OnChange)
  ON_NOTIFY_REFLECT_EX(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
  ON_NOTIFY_REFLECT_EX(TVN_ENDLABELEDIT, OnEndLabelEdit)
  ON_NOTIFY_REFLECT_EX(NM_RCLICK, OnRightClick)  
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,OnCustomDrawList)
END_MESSAGE_MAP()

BOOL ListViewImp::prevent_propagate_event(ui::Event& ev, MSG* pMsg) {  
  if (!::IsWindow(m_hWnd)) {
    return true;
  }  
  if (!ev.is_default_prevented()) {    
    ::TranslateMessage(pMsg);          
	  ::DispatchMessage(pMsg);        
  }
  return ev.is_propagation_stopped();  
}

void ListNodeImp::DevInsertFirst(ui::mfc::ListViewImp* list, const ui::Node& node, ListNodeImp* node_imp, ListNodeImp* prev_imp, int pos) {  
  LVITEM lvi;
  lvi.mask =  LVIF_TEXT | TVIF_IMAGE;
  lvi.cColumns = 0;
  node_imp->text_ = node.text();
  lvi.pszText = const_cast<char *>(node_imp->text_.c_str());
  lvi.iImage = node.image_index();
  lvi.iItem = pos;
  lvi.iSubItem = 0;
  node_imp->lvi = lvi;
  list->InsertItem(&lvi);  
}

void ListNodeImp::set_text(ui::mfc::ListViewImp* list, const std::string& text) {
  text_ = text;
  lvi.pszText = const_cast<char *>(text_.c_str());
  list->SetItemText(lvi.iItem, lvi.iSubItem, text.c_str());
}

void ListNodeImp::DevSetSub(ui::mfc::ListViewImp* list, const ui::Node& node, ListNodeImp* node_imp, ListNodeImp* prev_imp, int level) {
  LVITEM lvi;
  lvi.mask =  LVIF_TEXT | TVIF_IMAGE;
  lvi.cColumns = 0;
  node_imp->text_ = node.text();
  lvi.pszText = const_cast<char *>(node_imp->text_.c_str());
  lvi.iImage = node.image_index(); 
  lvi.iItem = pos();
  lvi.iSubItem = level - 1;
  node_imp->lvi = lvi;
  list->SetItem(&lvi);
  node_imp->set_pos(pos());
}

ListNodeImp* ListViewImp::UpdateNode(boost::shared_ptr<Node> node, boost::shared_ptr<Node> prev_node, int pos) {
  ListNodeImp* new_imp = new ListNodeImp();
  NodeImp* prev_node_imp = prev_node ? prev_node->imp(*this) : 0;
  node->erase_imp(this);  
  node->AddImp(new_imp);
  new_imp->set_owner(this);
  node->changed.connect(boost::bind(&ListViewImp::OnNodeChanged, this, _1));
  if (!node->parent().expired()) {
    boost::shared_ptr<ui::Node> parent_node = node->parent().lock();   
    boost::ptr_list<NodeImp>::iterator it = parent_node->imps.begin();
    for ( ; it != parent_node->imps.end(); ++it) {
      ListNodeImp* parent_imp = dynamic_cast<ListNodeImp*>(parent_node->imp(*this));
      if (parent_imp) {             
        ListNodeImp* prev_imp = dynamic_cast<ListNodeImp*>(prev_node_imp);
        int level = node->level();
        if (level == 1) {
          parent_imp->DevInsertFirst(this, *node.get(), new_imp, prev_imp, pos);
        } else {
          parent_imp->DevSetSub(this, *node.get(), new_imp, prev_imp, level);
        }
      }
    }
  }
  return new_imp;
}

void ListViewImp::DevUpdate(const Node::Ptr& node, boost::shared_ptr<Node> prev_node) {  
  recursive_node_iterator end = node->recursive_end();
  recursive_node_iterator it = node->recursive_begin();  
  int pos = 0;
  if (prev_node) {        
    ListNodeImp* prev_imp = dynamic_cast<ListNodeImp*>(prev_node->imp(*this));
    if (prev_imp) {
      pos = prev_imp->pos() + 1;
    }
  }
  UpdateNode(node, prev_node, pos);  
  boost::shared_ptr<Node> prev;
  pos = 0;
  for (; it != end; ++it) {    
    UpdateNode((*it), prev, pos);
    if ((*it)->level() == 1) {
      ++pos;
    }
    prev = *it;
  }
}

void ListViewImp::DevErase(ui::Node::Ptr node) {
  node->erase_imp(this);  
}

void ListViewImp::DevEditNode(ui::Node::Ptr node) {    
  ListNodeImp* imp = dynamic_cast<ListNodeImp*>(node->imp(*this));
  if (imp) {
   //EditLabel(imp->hItem);    
  }
}

void ListViewImp::dev_select_node(const ui::Node::Ptr& node) {
  ListNodeImp* imp = dynamic_cast<ListNodeImp*>(node->imp(*this));
  if (imp) {
      //SelectItem(imp->hItem);    
  }
}

boost::weak_ptr<Node> ListViewImp::dev_selected() {
  ui::Node* node = find_selected_node();
  return node ? node->shared_from_this() : boost::weak_ptr<ui::Node>();
}

void ListViewImp::OnNodeChanged(Node& node) {
  ListNodeImp* imp = dynamic_cast<ListNodeImp*>(node.imp(*this));
  if (imp) {      
    imp->set_text(this, node.text());      
  }
}

BOOL ListViewImp::OnChange(NMHDR * pNotifyStruct, LRESULT * result) {   
  Node* node = find_selected_node();
  if (node) {
    list_view()->OnChange(node->shared_from_this());
    list_view()->change(*list_view(), node->shared_from_this());
  }
  return FALSE;
}

BOOL ListViewImp::OnRightClick(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    list_view()->OnRightClick(node->shared_from_this());
  }
  return FALSE;
}

ui::Node* ListViewImp::find_selected_node() {    
  POSITION pos = GetFirstSelectedItemPosition();
  int selected = -1;
  if (pos != NULL) {
    while (pos) {
      selected = GetNextSelectedItem(pos);      
    }
  } 
  recursive_node_iterator end = list_view()->root_node().lock()->recursive_end();
  recursive_node_iterator recursive_it = list_view()->root_node().lock()->recursive_begin();  
  ui::Node::Ptr found;
  for (; recursive_it != end; ++recursive_it) {    
    boost::ptr_list<NodeImp>::iterator it = (*recursive_it)->imps.begin();
    NodeImp* imp = (*recursive_it)->imp(*this);    
    if (imp) {
      ListNodeImp* imp = dynamic_cast<ListNodeImp*>(&(*it));      
      if (imp->pos() == selected) {
        found = (*recursive_it);
        break;
      }
    }    
  }  
  return found.get();
}

std::vector<ui::Node::Ptr> ListViewImp::dev_selected_nodes() {
  std::vector<ui::Node::Ptr> nodes;
  POSITION pos = GetFirstSelectedItemPosition();
  int selected = -1;
  if (pos != NULL) {
    while (pos) {
      selected = GetNextSelectedItem(pos);
      if (selected != -1) {
        Node::Ptr root = list_view()->root_node().lock();
        ui::Node::Ptr found;
        Node::iterator it = root->begin();
        for (int pos = 0; it != root->end(); pos++, ++it) {
          if (pos == selected) {
            found = *it;
            break;
          }
        }
        assert(found.get());        
        nodes.push_back(found);                     
      }
    } 
  }
  return nodes;
}
 
BOOL ListViewImp::OnBeginLabelEdit(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    CEdit* edit = GetEditControl();
    CString s;    
    edit->GetWindowTextA(s);
    is_editing_ = true;
    list_view()->OnEditing(node->shared_from_this(), s.GetString());
  }
  return FALSE;
}


BOOL ListViewImp::OnEndLabelEdit(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    CEdit* edit = GetEditControl();
    CString s;    
    edit->GetWindowTextA(s);
    is_editing_ = false;
    list_view()->OnEdited(node->shared_from_this(), s.GetString());
  }
  return FALSE;
}

void ListViewImp::DevClear() { DeleteAllItems(); }

void ListViewImp::OnCustomDrawList(NMHDR *pNMHDR, LRESULT *pResult) {
   NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);   
   *pResult = CDRF_DODEFAULT;
   switch(pLVCD->nmcd.dwDrawStage) {
     case CDDS_PREPAINT:
      *pResult = CDRF_NOTIFYITEMDRAW;
      break;

     case CDDS_ITEMPREPAINT:
      *pResult = *pResult = CDRF_DODEFAULT; // CDRF_NOTIFYSUBITEMDRAW;
      pLVCD->clrTextBk = GetBkColor();
      break;     
   }
}

//end list
BEGIN_MESSAGE_MAP(ScrollBarImp, CScrollBar)  
	ON_WM_PAINT()    
END_MESSAGE_MAP()

BOOL ScrollBarImp::OnChildNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult) {
  if (uMsg == WM_HSCROLL || uMsg == WM_VSCROLL) {
    int nScrollCode = (short)LOWORD(wParam);
	  int nPos = (short)HIWORD(wParam);
    switch(nScrollCode) {
      case SB_THUMBTRACK:     //The user is dragging the scroll box. This message is sent repeatedly until the user releases the mouse button. The nPos parameter indicates the position that the scroll box has been dragged to.         
        dev_set_scroll_pos(nPos);
        break;
    }
    
    OnDevScroll(nPos);
  }
  return  WindowTemplateImp<CScrollBar, ui::ScrollBarImp>::OnChildNotify(uMsg, wParam, lParam, pResult);
}

IMPLEMENT_DYNAMIC(ScintillaImp, CWnd)

BEGIN_MESSAGE_MAP(ScintillaImp, CWnd)
ON_NOTIFY_REFLECT_EX(SCN_CHARADDED, OnModified)
ON_NOTIFY_REFLECT_EX(SCN_MARGINCLICK, OnFolder)         
END_MESSAGE_MAP()

void ScintillaImp::dev_set_lexer(const Lexer& lexer) {
  SetupLexerType();
  SetupHighlighting(lexer);
  SetupFolding(lexer);
}

void ScintillaImp::SetupHighlighting(const Lexer& lexer) {
  f(SCI_SETKEYWORDS, 0, lexer.keywords().c_str());
  f(SCI_STYLESETFORE, SCE_LUA_COMMENT, ToCOLORREF(lexer.comment_color())); 
  f(SCI_STYLESETFORE, SCE_LUA_COMMENTLINE, ToCOLORREF(lexer.comment_line_color()));
  f(SCI_STYLESETFORE, SCE_LUA_COMMENTDOC, ToCOLORREF(lexer.comment_doc_color()));
  f(SCI_STYLESETFORE, SCE_LUA_IDENTIFIER, ToCOLORREF(lexer.identifier_color()));
  f(SCI_STYLESETFORE, SCE_LUA_NUMBER, ToCOLORREF(lexer.number_color()));
  f(SCI_STYLESETFORE, SCE_LUA_STRING, ToCOLORREF(lexer.string_color()));
  f(SCI_STYLESETFORE, SCE_LUA_WORD, ToCOLORREF(lexer.word_color()));
  f(SCI_STYLESETFORE, SCE_LUA_PREPROCESSOR, ToCOLORREF(lexer.identifier_color()));
  f(SCI_STYLESETFORE, SCE_LUA_OPERATOR, ToCOLORREF(lexer.operator_color()));
  f(SCI_STYLESETFORE, SCE_LUA_CHARACTER, ToCOLORREF(lexer.character_code_color()));
}

void ScintillaImp::SetupFolding(const Lexer& lexer) {
  SetFoldingBasics();
  SetFoldingColors(lexer);
  SetFoldingMarkers();
  SetupLexerType();
}

void ScintillaImp::SetFoldingBasics() {
  f(SCI_SETMARGINWIDTHN, 2, 12);
  f(SCI_SETMARGINSENSITIVEN, 2, true);  
  f(SCI_SETMARGINTYPEN, 2, SC_MARGIN_SYMBOL);
  f(SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
  f(SCI_SETPROPERTY, _T("fold"), _T("1"));
  f(SCI_SETPROPERTY, _T("fold.compact"), _T("1"));                
  f(SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CHANGE| SC_AUTOMATICFOLD_CLICK, 0);
}

void ScintillaImp::SetFoldingColors(const Lexer& lexer) {
 for (int i = 25; i <= 31; i++) {    
   f(SCI_MARKERSETBACK, i, ToCOLORREF(lexer.folding_color()));
 }
}

void ScintillaImp::SetFoldingMarkers() {
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);  
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
}

void ScintillaImp::SetupLexerType() {
  f(SCI_SETLEXER, SCLEX_LUA, 0);
}

BOOL ScintillaImp::OnFolder(NMHDR * nhmdr,LRESULT *) { 
  SCNotification *pMsg = (SCNotification*)nhmdr;      
  long lLine = f(SCI_LINEFROMPOSITION, pMsg->position, 0);
  f(SCI_TOGGLEFOLD, lLine, 0);   
  return false;
}

LRESULT __stdcall WindowHook::HookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    CWPSTRUCT* info = (CWPSTRUCT*) lParam;        
    if (info->message == WM_SETFOCUS) {
      FilterHook(info->hwnd);                     
    }
  }
  return CallNextHookEx(_hook , nCode, wParam, lParam);       
}

void WindowHook::FilterHook(HWND hwnd) {  
  typedef std::map<HWND, ui::WindowImp*> WindowList;
  /*WindowList::iterator it = windows_.find(hwnd);
  if (it != windows_.end()) {
    it->second->window()->
  } else {
    Window::SetFocus(Window::nullpointer);
  }*/
}

void WindowHook::SetFocusHook() {
  if (_hook) {
    return;
  }
  if (!(_hook = SetWindowsHookEx(WH_CALLWNDPROC, 
                                 WindowHook::HookCallback,
                                 AfxGetInstanceHandle(),
                                 GetCurrentThreadId()))) {
    TRACE(_T("ui::canvas::MFCView : Failed to install hook!\n"));
  }
}

void WindowHook::ReleaseHook() { 
  UnhookWindowsHookEx(_hook);
}

// MenuContainerImp
std::map<std::uint16_t, MenuContainerImp*> MenuContainerImp::menu_bar_id_map_;

MenuContainerImp::MenuContainerImp() : menu_window_(0), cmenu_(0) {  
}

void MenuContainerImp::set_menu_window(CWnd* menu_window, const Node::Ptr& root_node) {
  menu_window_ = menu_window;  
  if (!menu_window && root_node) {
    cmenu_ = 0;
    root_node->erase_imps(this);    
  } else {
    cmenu_ = menu_window->GetMenu();
    DevUpdate(root_node, nullpointer);
  }
}

void MenuContainerImp::DevInvalidate() {
  if (menu_window_) {
    menu_window_->DrawMenuBar();
  }
}

void MenuContainerImp::DevUpdate(const Node::Ptr& node, boost::shared_ptr<Node> prev_node) {
  if (cmenu_) {
    UpdateNodes(node, cmenu_, cmenu_->GetMenuItemCount());
    DevInvalidate();
  }
}

void MenuContainerImp::RegisterMenuEvent(std::uint16_t id, MenuImp* menu_imp) {
  menu_item_id_map_[id] = menu_imp;
  menu_bar_id_map_[id] = this;
}

void MenuContainerImp::UpdateNodes(Node::Ptr parent_node, CMenu* parent, int pos_start) {
  if (parent_node) {
    Node::Container::iterator it = parent_node->begin();    
    for (int pos = pos_start; it != parent_node->end(); ++it, ++pos) {
      Node::Ptr node = *it;
      boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
      while (it != node->imps.end()) {
        NodeImp* i = &(*it);
        if (i->owner() == this) {
          it = node->imps.erase(it);            
        } else {
          ++it;
        }
      }
      
      MenuImp* menu_imp = new MenuImp(parent);
      menu_imp->set_owner(this);
      menu_imp->dev_set_pos(pos);      
      if (node->size() == 0) {        
        ui::Image* img = !node->image().expired() ? node->image().lock().get() : 0;
        menu_imp->CreateMenuItem(node->text(), img);
        RegisterMenuEvent(menu_imp->id(), menu_imp);
      } else {
        menu_imp->CreateMenu(node->text());        
      }
      node->imps.push_back(menu_imp);      
      if (node->size() > 0) {      
        boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
        for ( ; it != node->imps.end(); ++it) {
          if (it->owner() == this) {
            MenuImp* menu_imp =  dynamic_cast<MenuImp*>(&(*it));    
            if (menu_imp) {
              UpdateNodes(node, menu_imp->cmenu());
            }
            break;
          }
        }        
      }
    }
  }
}

void MenuContainerImp::DevErase(boost::shared_ptr<Node> node) {
  boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
  for (; it != node->imps.end(); ++it) {     
    if (it->owner() == this) {      
      node->imps.erase(it);
      break;
    } 
  }
}

void MenuContainerImp::WorkMenuItemEvent(int id) {
  MenuImp* menu_imp = FindMenuItemById(id);
  assert(menu_imp);
  if (menu_bar()) {
    struct {
     ui::MenuContainer* bar;
     int selectedItemID;
     void operator()(boost::shared_ptr<ui::Node> node, boost::shared_ptr<ui::Node> prev_node) {
       boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
       for ( ; it != node->imps.end(); ++it) {         
         MenuImp* imp = dynamic_cast<MenuImp*>(&(*it));
          if (imp) {
            if (imp->id() == selectedItemID) {
             bar->OnMenuItemClick(node->shared_from_this());
            }
          }
        }
      }
    } f;
    f.bar = menu_bar();
    f.selectedItemID = menu_imp->id();
    menu_bar()->root_node().lock()->traverse(f);  
  }
}

MenuImp* MenuContainerImp::FindMenuItemById(int id) {
  std::map<std::uint16_t, MenuImp*>::iterator it = menu_item_id_map_.find(id);
  return (it != menu_item_id_map_.end()) ? it->second : 0;
}

MenuContainerImp* MenuContainerImp::MenuContainerImpById(int id) {
  std::map<std::uint16_t, MenuContainerImp*>::iterator it = menu_bar_id_map_.find(id);
  return (it != menu_bar_id_map_.end()) ? it->second : 0;
}

// PopupMenuImp
PopupMenuImp::PopupMenuImp() : popup_menu_(new CMenu()) {
  popup_menu_->CreatePopupMenu();
  set_cmenu(popup_menu_.get());
}

void PopupMenuImp::DevTrack(const ui::Point& pos) {
  popup_menu_->TrackPopupMenu(
    TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_HORPOSANIMATION | TPM_VERPOSANIMATION,
    pos.x(), pos.y(), ::AfxGetMainWnd());
}

//MenuImp
void MenuImp::CreateMenu(const std::string& text) {
  cmenu_ = new CMenu();
  cmenu_->CreateMenu();
  parent_->AppendMenu(MF_POPUP | MF_ENABLED, (UINT_PTR)cmenu_->m_hMenu, text.c_str());    
}

void MenuImp::CreateMenuItem(const std::string& text, ui::Image* image) {
  if (text == "-") {
    parent_->AppendMenuA(MF_SEPARATOR);  
  } else {
    id_ = ID_DYNAMIC_MENUS_START + ui::MenuContainer::id_counter++;
    parent_->AppendMenu(MF_STRING, id_, text.c_str());
    if (image) {      
      parent_->SetMenuItemBitmaps(id_, MF_BYCOMMAND, (CBitmap*) image->source(), (CBitmap*) image->source()); 
    }
  }
}

void GameControllersImp::DevScanPluggedControllers(std::vector<int>& plugged_controller_ids) {
  UINT num = joyGetNumDevs();
  int game_controller_id = 0;
  for ( ; game_controller_id < num; ++game_controller_id) {
    JOYINFO joyinfo;    
    int err = joyGetPos(game_controller_id, &joyinfo);
    if (err == 0) {      
      plugged_controller_ids.push_back(game_controller_id);      
    }
  }  
}

void GameControllersImp::DevUpdateController(ui::GameController& controller) {  
  JOYINFO joy_info;
  joyGetPos(controller.id(), &joy_info);
  controller.set(joy_info.wXpos, joy_info.wYpos, joy_info.wZpos, static_cast<int>(joy_info.wButtons));
}


} // namespace mfc
} // namespace ui
} // namespace host
} // namespace psycle