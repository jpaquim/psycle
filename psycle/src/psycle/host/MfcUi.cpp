#include <psycle/host/detail/project.hpp>
#include "MfcUi.hpp"
#include "Psycle.hpp"
#include "Mmsystem.h"

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

template<class T, class I>
BOOL WindowTemplateImp<T, I>::PreTranslateMessage(MSG* pMsg) {
  if (pMsg->message==WM_KEYDOWN ) {
    CWnd* hwndTest = GetFocus();
    if (hwndTest) {            
      UINT nFlags = 0;
      UINT flags = Win32KeyFlags(nFlags);      
      KeyEvent ev(pMsg->wParam, flags);
      OnDevKeyDown(ev);
      return ev.is_default_prevented();      
    }          
  } else
  if (pMsg->message == WM_KEYUP) {          
    CWnd* hwndTest = GetFocus();
    if (hwndTest) {
      UINT nFlags = 0;
      UINT flags = Win32KeyFlags(nFlags);
      KeyEvent ev(pMsg->wParam, flags);
      OnDevKeyUp(ev);
      return ev.is_default_prevented();
    }
  }
  return CWnd::PreTranslateMessage(pMsg);
}

/*
template<class T, class I>
BOOL WindowTemplateImp<T, I>::PreCreateWindow(CREATESTRUCT& cs) {
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;
	// CS_HREDRAW | CS_VREDRAW |
	cs.dwExStyle = 0; // |= WS_EX_CLIENTEDGE;
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
}*/

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
  dev_pos_.set(pos.left(), pos.top());
  SetWindowPos(0, 
               pos.left(),
               pos.top(),
               pos.width(),
               pos.height(),            
               SWP_NOREDRAW |
               SWP_NOZORDER
               // SWP_ASYNCWINDOWPOS
               );


    //SWP_ASYNCWINDOWPOS
  /*MoveWindow(pos.left(),
             pos.top(),
             pos.width(),
             pos.height());*/
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::dev_pos() const {
  CRect rc;
  GetClientRect(&rc);
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
    ShowWindow(SW_SHOW);    
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
  OnDevSize(cw, ch);
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
  ON_WM_KEYDOWN()
  ON_WM_KEYUP()  
END_MESSAGE_MAP()


BEGIN_MESSAGE_MAP(FrameImp, CFrameWnd)
  ON_WM_SIZE()
	ON_WM_PAINT()
  ON_WM_CLOSE()
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

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

BEGIN_MESSAGE_MAP(ButtonImp, CButton)  
	ON_WM_PAINT()  
  ON_CONTROL_REFLECT(BN_CLICKED, OnClick)
END_MESSAGE_MAP()

void ButtonImp::OnClick() {
  OnDevClick();
}

BEGIN_MESSAGE_MAP(ComboBoxImp, CComboBox)  
	ON_WM_PAINT()
  ON_CONTROL_REFLECT_EX(CBN_SELENDOK, OnSelect)
END_MESSAGE_MAP()

BOOL ComboBoxImp::OnSelect() {  
  ui::ComboBox* combo_box = dynamic_cast<ui::ComboBox*>(window());
  assert(combo_box);
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
  ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnClick)
  ON_NOTIFY_REFLECT_EX(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
  ON_NOTIFY_REFLECT_EX(TVN_ENDLABELEDIT, OnEndLabelEdit)
  ON_NOTIFY_REFLECT_EX(NM_RCLICK, OnRightClick)  
END_MESSAGE_MAP()

void TreeViewImp::DevUpdate(boost::shared_ptr<Node> node, boost::shared_ptr<Node> prev_node) {  
  struct {
      TreeViewImp* that;       
      void operator()(boost::shared_ptr<ui::Node> node, boost::shared_ptr<ui::Node> prev_node) {
        NodeImp* prev_node_imp = 0;
        if (prev_node) {
          boost::ptr_list<NodeImp>::iterator it = prev_node->imps.begin();
          while (it != prev_node->imps.end()) {
            NodeImp* i = &(*it);
            if (i->owner() == that) {
              prev_node_imp = i;
              break;
            } else {
              ++it;
            }        
          }
        }

        boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
        while (it != node->imps.end()) {
          NodeImp* i = &(*it);
          if (i->owner() == that) {
            it = node->imps.erase(it);            
          } else {
            ++it;
          }
        }
        TreeNodeImp* new_imp = new TreeNodeImp();
        node->AddImp(new_imp);
        new_imp->set_owner(that);               
        node->changed.connect(boost::bind(&TreeViewImp::OnNodeChanged, that, _1));                
        if (!node->parent().expired()) {
          boost::shared_ptr<ui::Node> parent_node = node->parent().lock();   
          boost::ptr_list<NodeImp>::iterator it = parent_node->imps.begin();
          for ( ; it != parent_node->imps.end(); ++it) {            
            TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(&(*it));
            if (imp) {              
              TreeNodeImp* prev_imp = dynamic_cast<TreeNodeImp*>(prev_node_imp);
              new_imp->hItem = imp->DevInsert(that, *node.get(), prev_imp);
              that->htreeitem_node_map_[new_imp->hItem] = node;
            }
          }
        }
      }
    } f;
  f.that = this;  
  node->traverse(f, prev_node);
}

void TreeViewImp::DevErase(boost::shared_ptr<Node> node) {
  boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
  for (; it != node->imps.end(); ++it) {     
    if (it->owner() == this) {
      TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(&(*it));
      if (imp) {
        DeleteItem(imp->hItem);
      }
      node->imps.erase(it);
      break;
    } 
  }
}

void TreeViewImp::DevEditNode(boost::shared_ptr<ui::Node> node) {    
  boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
  for ( ; it != node->imps.end(); ++it) {    
    TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(&(*it));
    if (imp) {      
      EditLabel(imp->hItem);
      break;
    }
  }
}

void TreeViewImp::dev_select_node(const boost::shared_ptr<ui::Node>& node) {
  boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
  for ( ; it != node->imps.end(); ++it) {    
    TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(&(*it));
    if (imp) {      
      SelectItem(imp->hItem);      
    }
  }
}

boost::weak_ptr<Node> TreeViewImp::dev_selected() {
  ui::Node* node = find_selected_node();
  return node ? node->shared_from_this() : boost::weak_ptr<ui::Node>();
}

void TreeViewImp::OnNodeChanged(Node& node) {
  boost::ptr_list<NodeImp>::iterator it = node.imps.begin();
  for ( ; it != node.imps.end(); ++it) {    
    TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(&(*it));
    if (imp) {      
      SetItemText(imp->hItem, node.text().c_str());       
    }
  }
}

BOOL TreeViewImp::OnClick(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    tree_view()->OnClick(node->shared_from_this());
  }
  return FALSE;
}

BOOL TreeViewImp::OnRightClick(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    tree_view()->OnRightClick(node->shared_from_this());
  }
  return FALSE;
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
  ON_NOTIFY_REFLECT_EX(LVN_ITEMCHANGED, OnClick)
  ON_NOTIFY_REFLECT_EX(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
  ON_NOTIFY_REFLECT_EX(TVN_ENDLABELEDIT, OnEndLabelEdit)
  ON_NOTIFY_REFLECT_EX(NM_RCLICK, OnRightClick)  
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,OnCustomDrawList)
END_MESSAGE_MAP()

void ListNodeImp::DevInsertFirst(ui::mfc::ListViewImp* list, const ui::Node& node, ListNodeImp* node_imp, ListNodeImp* prev_imp) {
  node_imp->pos_ = prev_imp ? prev_imp->pos() + 1 : 0;
  LVITEM lvi;
  lvi.mask =  LVIF_TEXT | TVIF_IMAGE;
  lvi.cColumns = 0;
  node_imp->text_ = node.text();
  lvi.pszText = const_cast<char *>(node_imp->text_.c_str());
  lvi.iImage = node.image_index();
  lvi.iItem = node_imp->pos_;
  lvi.iSubItem = 0;
  list->InsertItem(&lvi);  
}

void ListNodeImp::DevSetSub(ui::mfc::ListViewImp* list, const ui::Node& node, ListNodeImp* node_imp, ListNodeImp* prev_imp, int level) {
  LVITEM lvi;
  lvi.mask =  LVIF_TEXT | TVIF_IMAGE;
  lvi.cColumns = 0;
  node_imp->text_ = node.text();
  lvi.pszText = const_cast<char *>(node_imp->text_.c_str());
  lvi.iImage = node.image_index(); 
  lvi.iItem = pos_;
  lvi.iSubItem = level - 1;
  list->SetItem(&lvi);
  node_imp->set_pos(pos_);
}

void ListViewImp::DevUpdate(boost::shared_ptr<Node> node, boost::shared_ptr<Node> prev_node) {  
  struct {
      ListViewImp* that;       
      void operator()(boost::shared_ptr<ui::Node> node, boost::shared_ptr<ui::Node> prev_node) {
        NodeImp* prev_node_imp = 0;
        if (prev_node) {
          boost::ptr_list<NodeImp>::iterator it = prev_node->imps.begin();
          while (it != prev_node->imps.end()) {
            NodeImp* i = &(*it);
            if (i->owner() == that) {
              prev_node_imp = i;
              break;
            } else {
              ++it;
            }        
          }
        }

        boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
        while (it != node->imps.end()) {
          NodeImp* i = &(*it);
          if (i->owner() == that) {
            it = node->imps.erase(it);            
          } else {
            ++it;
          }
        }
        ListNodeImp* new_imp = new ListNodeImp();
        node->AddImp(new_imp);
        new_imp->set_owner(that);               
        node->changed.connect(boost::bind(&ListViewImp::OnNodeChanged, that, _1));                
        if (!node->parent().expired()) {
          boost::shared_ptr<ui::Node> parent_node = node->parent().lock();   
          boost::ptr_list<NodeImp>::iterator it = parent_node->imps.begin();
          for ( ; it != parent_node->imps.end(); ++it) {            
            ListNodeImp* imp = dynamic_cast<ListNodeImp*>(&(*it));
            if (imp) {              
              ListNodeImp* prev_imp = dynamic_cast<ListNodeImp*>(prev_node_imp);
              int level = node->level();
              if (level == 1) {
                imp->DevInsertFirst(that, *node.get(), new_imp, prev_imp);
              } else {
                imp->DevSetSub(that, *node.get(), new_imp, prev_imp, level);
              }
            }
          }
        }
      }
    } f;
  f.that = this;  
  node->traverse(f, prev_node);
}

void ListViewImp::DevErase(boost::shared_ptr<Node> node) {
  boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
  for (; it != node->imps.end(); ++it) {     
    if (it->owner() == this) {
      ListNodeImp* imp = dynamic_cast<ListNodeImp*>(&(*it));
      if (imp) {
      //  DeleteItem(imp->hItem);
      }
      node->imps.erase(it);
      break;
    } 
  }
}

void ListViewImp::DevEditNode(boost::shared_ptr<ui::Node> node) {    
  boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
  for ( ; it != node->imps.end(); ++it) {    
    ListNodeImp* imp = dynamic_cast<ListNodeImp*>(&(*it));
    if (imp) {      
      //EditLabel(imp->hItem);
      break;
    }
  }
}

void ListViewImp::dev_select_node(const boost::shared_ptr<ui::Node>& node) {
  boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
  for ( ; it != node->imps.end(); ++it) {    
    ListNodeImp* imp = dynamic_cast<ListNodeImp*>(&(*it));
    if (imp) {      
      //SelectItem(imp->hItem);      
    }
  }
}

boost::weak_ptr<Node> ListViewImp::dev_selected() {
  ui::Node* node = find_selected_node();
  return node ? node->shared_from_this() : boost::weak_ptr<ui::Node>();
}

void ListViewImp::OnNodeChanged(Node& node) {
  boost::ptr_list<NodeImp>::iterator it = node.imps.begin();
  for ( ; it != node.imps.end(); ++it) {    
    ListNodeImp* imp = dynamic_cast<ListNodeImp*>(&(*it));
    if (imp) {      
      //SetItemText(imp->hItem, node.text().c_str());       
    }
  }
}

BOOL ListViewImp::OnClick(NMHDR * pNotifyStruct, LRESULT * result) {   
  Node* node = find_selected_node();
  if (node) {
    list_view()->OnClick(node->shared_from_this());
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
  struct {
   ListViewImp* that;
    int iItem;
    Node::Ptr found;    
    void operator()(boost::shared_ptr<ui::Node> node, boost::shared_ptr<ui::Node> prev_node) {
      boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
      NodeImp* imp = 0;
      while (it != node->imps.end()) {
        NodeImp* i = &(*it);
        if (i->owner() == that) {
          imp = i;
          break;
        } else {
          ++it;        
        }
      }
      if (imp) {
        ListNodeImp* imp = dynamic_cast<ListNodeImp*>(&(*it));
        if ((node->level() > 0) && (imp->pos() == iItem)) {
          found = node;
        }
      }
    }
  } f;
  f.that = this;  
  f.iItem = selected;
  list_view()->root_node().lock()->traverse(f);
  return f.found.get();
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

IMPLEMENT_DYNAMIC(ScintillaImp, CWnd)

BEGIN_MESSAGE_MAP(ScintillaImp, CWnd)
ON_NOTIFY_REFLECT_EX(SCN_CHARADDED, OnModified)
ON_NOTIFY_REFLECT_EX(SCN_MARGINCLICK, OnFolder)         
END_MESSAGE_MAP()

void ScintillaImp::dev_set_lexer(const Lexer& lexer) {
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

void MenuContainerImp::DevUpdate(boost::shared_ptr<Node> node, boost::shared_ptr<Node> prev_node) {
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