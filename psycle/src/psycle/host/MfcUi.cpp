// #include "stdafx.h"

#include "MfcUi.hpp"
#include "Mmsystem.h"
#include "resources/resources.hpp"
// #include "Resource.h"

namespace psycle {
namespace host {
namespace ui {
namespace mfc {

void GraphicsImp::DevFillRegion(const ui::Region& rgn) {    
  check_pen_update();
  check_brush_update();        
  mfc::RegionImp* imp = dynamic_cast<mfc::RegionImp*>(rgn.imp());
  assert(imp);
  ::FillRgn(cr_->m_hDC, imp->crgn(), brush);  
}

void GraphicsImp::DevSetClip(ui::Region* rgn) {
  if (rgn) {
    mfc::RegionImp* imp = dynamic_cast<mfc::RegionImp*>(rgn->imp());
    assert(imp);
    cr_->SelectClipRgn(&imp->crgn());
  } else {
    cr_->SelectClipRgn(0);
  }  
}

RegionImp::RegionImp() { 
	rgn_.CreateRectRgn(0, 0, 0, 0); 
}
  
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
  CPoint pt(static_cast<int>(dx), static_cast<int>(dy));
  rgn_.OffsetRgn(pt);
}

void RegionImp::DevUnion(const ui::Region& other) {
  mfc::RegionImp* imp = dynamic_cast<mfc::RegionImp*>(other.imp());
  assert(imp);
  rgn_.CombineRgn(&rgn_, &imp->crgn(), RGN_OR);
} 

ui::Rect RegionImp::DevBounds() const { 
  CRect rc;
  rgn_.GetRgnBox(&rc);
  return ui::Rect(ui::Point(rc.left, rc.top), ui::Point(rc.right, rc.bottom));
}

bool RegionImp::DevIntersect(double x, double y) const {
  return rgn_.PtInRegion(static_cast<int>(x), static_cast<int>(y)) != 0;
}

bool RegionImp::DevIntersectRect(const ui::Rect& rect) const {  
  return rgn_.RectInRegion(TypeConverter::rect(rect)) != 0;  
}
  
void RegionImp::DevSetRect(const ui::Rect& rect) {
  rgn_.SetRectRgn(TypeConverter::rect(rect)); 
}

void RegionImp::DevClear() {
  DevSetRect(ui::Rect());  
}

void ImageImp::DevReset(const ui::Dimension& dimension) {
	Dispose();
	bmp_ = new CBitmap();
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	bmp_->CreateCompatibleBitmap(&dc, static_cast<int>(dimension.width()),
                               static_cast<int>(dimension.height()));
	::ReleaseDC(NULL, dc);
}

ui::Graphics* ImageImp::dev_graphics() {
	if (!paint_graphics_.get()) {		
		CDC* memDC = new CDC();
        memDC->CreateCompatibleDC(NULL);
		paint_graphics_.reset(new ui::Graphics(memDC));
		memDC->SelectObject(bmp_);	
	}
	return paint_graphics_.get();
}
  
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
	ON_WM_MOUSEHOVER()
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
    if (map_capslock_to_ctrl_) {
      BYTE byKeybState[256];
      ::GetKeyboardState(byKeybState);
      byKeybState[VK_CAPITAL] = 0;
      byKeybState[VK_CONTROL] = 0;
      ::SetKeyboardState(byKeybState); 
      if (pMsg->wParam == 20 || pMsg->wParam == 17) {     
        capslock_on_ = true;      
      }
      if  (capslock_on_) {        
        BYTE byKeybState[256];
        ::GetKeyboardState(byKeybState);
        byKeybState[VK_CAPITAL] = 0;      
        byKeybState[VK_CONTROL] = 0x80;
        ::SetKeyboardState(byKeybState);      
      }    
    }
    UINT flags = Win32KeyFlags(nFlags);      
    KeyEvent ev(pMsg->wParam, flags);    
    return WorkEvent(ev, &Window::OnKeyDown, window(), pMsg);
  } else
  if (pMsg->message == WM_KEYUP) {    
    if (map_capslock_to_ctrl_) {
      BYTE byKeybState[256];
      ::GetKeyboardState(byKeybState);
      byKeybState[VK_CAPITAL] = 0;
      byKeybState[VK_CONTROL] = 0;
      ::SetKeyboardState(byKeybState);
      if (capslock_on_) {    
        BYTE byKeybState[256];
        ::GetKeyboardState(byKeybState);
        byKeybState[VK_CAPITAL] = 0;
        byKeybState[VK_CONTROL] = VK_CONTROL;
        ::SetKeyboardState(byKeybState);
      }
      if (pMsg->wParam == 20 || pMsg->wParam == 17) {
        capslock_on_ = false;
      }
    }
    UINT nFlags = 0;
    UINT flags = Win32KeyFlags(nFlags);      
    KeyEvent ev(pMsg->wParam, flags);      
    return WorkEvent(ev, &Window::OnKeyUp, window(), pMsg);    
  } else
  if (pMsg->message == WM_MOUSELEAVE) {	
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		CWnd* child = ChildWindowFromPoint(pt, CWP_SKIPINVISIBLE);
		if (!child) {
      mouse_enter_ = true;      
      MouseEvent ev(MousePos(pt), 1, pMsg->wParam);
      return WorkEvent(ev, &Window::OnMouseOut, window(), pMsg);    
		}
  }  else
  if (pMsg->message == WM_LBUTTONDOWN) {    
    MouseEvent ev(MousePos(pMsg->pt), 1, pMsg->wParam);
    return WorkEvent(ev, &Window::OnMouseDown, window(), pMsg);
	} else
  if (pMsg->message == WM_LBUTTONDBLCLK) {    
    MouseEvent ev(MousePos(pMsg->pt), 1, pMsg->wParam);
    return WorkEvent(ev, &Window::OnDblclick, window(), pMsg);
  } else
  if (pMsg->message == WM_LBUTTONUP) {    
    MouseEvent ev(MousePos(pMsg->pt), 1, pMsg->wParam);
    return WorkEvent(ev, &Window::OnMouseUp, window(), pMsg);
  } else
  if (pMsg->message == WM_RBUTTONDOWN) {        
    MouseEvent ev(MousePos(pMsg->pt), 2, pMsg->wParam);
    return WorkEvent(ev, &Window::OnMouseDown, window(), pMsg);
  } else    
  if (pMsg->message == WM_RBUTTONDBLCLK) {    
    MouseEvent ev(MousePos(pMsg->pt), 2, pMsg->wParam);
    return WorkEvent(ev, &Window::OnDblclick, window(), pMsg);
  } else
  if (pMsg->message == WM_RBUTTONUP) {    
    MouseEvent ev(MousePos(pMsg->pt), 2, pMsg->wParam);
    return WorkEvent(ev, &Window::OnMouseUp, window(), pMsg);
  } else  
	if (pMsg->message == WM_MOUSEHOVER) {

	} else
  if (pMsg->message == WM_MOUSEMOVE) {    
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = TME_LEAVE;
    tme.dwHoverTime = 1;    
    m_bTracking = _TrackMouseEvent(&tme);         
    MouseEvent ev(MousePos(pMsg->pt), 1, pMsg->wParam);
    if (mouse_enter_) {
      mouse_enter_ = false;
      if (window()) {
        try {
          window()->OnMouseEnter(ev);
        } catch (std::exception& e) {
          alert(e.what());      
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
  capslock_on_ = false;
}

template<class T, class I>
int WindowTemplateImp<T, I>::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}  
	return 0;
}

template<class T, class I>
void WindowTemplateImp<T, I>::OnDestroy() {    
  bmpDC.DeleteObject();
}

template<class T, class I>
void WindowTemplateImp<T, I>::dev_set_position(const ui::Rect& pos) {
	ui::Point top_left = pos.top_left();
	top_left.offset(margin_.left(), margin_.top());
  if (window() && window()->parent()) {
	  top_left.offset(window()->parent()->border_space().left() +
			                window()->parent()->padding().left(),
									  window()->parent()->border_space().top() +
			                window()->parent()->padding().top());
  }	
  SetWindowPos(0, 
		           static_cast<int>(top_left.x()),
		           static_cast<int>(top_left.y()),
		           static_cast<int>(pos.width() + padding_.width() + border_space_.width()),
		           static_cast<int>(pos.height() + padding_.height() + border_space_.height()),
               SWP_NOREDRAW | SWP_NOZORDER | SWP_NOACTIVATE);
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::dev_position() const {
	CRect rc;
	GetWindowRect(&rc);
	if (GetParent()) {
	  ::MapWindowPoints(HWND_DESKTOP, GetParent()->m_hWnd, (LPPOINT)&rc, 2);	
	}	
	return MapPosToBoxModel(rc);
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::dev_absolute_position() const {
  CRect rc;
	GetWindowRect(&rc);  
	if (window() && window()->root()) {
    CWnd* root = dynamic_cast<CWnd*>(window()->root()->imp());
		::MapWindowPoints(NULL, root->m_hWnd, (LPPOINT)&rc, 2);    
		return MapPosToBoxModel(rc);
	}
	return ui::Rect::zero();
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::dev_absolute_system_position() const {
  CRect rc;
	GetWindowRect(&rc);  
	if (window() && window()->root()) {
    CWnd* root = dynamic_cast<CWnd*>(window()->root()->imp());
		::MapWindowPoints(NULL, root->m_hWnd, (LPPOINT)&rc, 2);    
		return ui::Rect(ui::Point(rc.left, rc.top),
                    ui::Point(rc.right, rc.bottom));
	}
	return ui::Rect::zero();
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::dev_desktop_position() const {
  CRect rc;
  GetWindowRect(&rc);
	return MapPosToBoxModel(rc);  
}

template<class T, class I>
bool WindowTemplateImp<T, I>::dev_check_position(const ui::Rect& pos) const {
  CRect rc;
	GetWindowRect(&rc);
	if (GetParent()) {
	  ::MapWindowPoints(HWND_DESKTOP, GetParent()->m_hWnd, (LPPOINT)&rc, 2);	
	}
  ui::Point top_left = pos.top_left();
	top_left.offset(margin_.left(), margin_.top());
  if (window() && window()->parent()) {
	  top_left.offset(window()->parent()->border_space().left() +
			              window()->parent()->padding().left(),
									  window()->parent()->border_space().top() +
			              window()->parent()->padding().top());
  }	
  ui::Rect pos1 = ui::Rect(top_left,
                           ui::Dimension(pos.width() + padding_.width() +
                                         border_space_.width(),
		                                     pos.height() + padding_.height() +
                                         border_space_.height()));
  return TypeConverter::rect(pos1).EqualRect(rc);
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::MapPosToBoxModel(const CRect& rc) const {
	 ui::Point top_left(rc.left - margin_.left(), rc.top - margin_.top());   
	   if (window() && window()->parent()) {       
		   top_left.offset(-window()->parent()->border_space().left() - 
                       window()->parent()->padding().left(),
			                 -window()->parent()->border_space().top() -
                       window()->parent()->padding().top());
	   }
		 return ui::Rect(top_left,
		              ui::Dimension(rc.Width() - padding_.width() -
										              border_space_.width(),
										            rc.Height() - padding_.height() -
										              border_space_.height()));
}

template<class T, class I>
void WindowTemplateImp<T, I>::dev_set_parent(Window* parent) {  
  if (parent && parent->imp()) {    
    DevHide();
    SetParent(dynamic_cast<CWnd*>(parent->imp()));
    if (window() && window()->visible()) {
      DevShow();    
    }
  } else {
    SetParent(DummyWindow::dummy());
  }
}

template<class T, class I>
void WindowTemplateImp<T, I>::OnPaint() { 
  CRgn rgn;
  rgn.CreateRectRgn(0, 0, 0, 0);
  int has_update_region = GetUpdateRgn(&rgn, FALSE);
  if (!has_update_region) {
		return; 
	}
  CPaintDC dc(this);
  if (!is_double_buffered_) {    
	  ui::Graphics g(&dc);
	  ui::Region draw_rgn(new ui::mfc::RegionImp(rgn));    
    window()->DrawBackground(&g, draw_rgn);    
	  g.Translate(Point(padding_.left() + border_space_.left(),
                      padding_.top() + border_space_.top()));
	  OnDevDraw(&g, draw_rgn);
	  g.Dispose();
  }
  else {
	  if (!bmpDC.m_hObject) { // buffer creation	
		  CRect rc;
		  GetClientRect(&rc);
		  bmpDC.CreateCompatibleBitmap(&dc, rc.right - rc.left, rc.bottom - rc.top);
		  char buf[128];
		  sprintf(buf, "CanvasView::OnPaint(). Initialized bmpDC to 0x%p\n",
              (void*)bmpDC);
		  TRACE(buf);
	  }	
	  CDC bufDC;
	  bufDC.CreateCompatibleDC(&dc);
	  CBitmap* oldbmp = bufDC.SelectObject(&bmpDC);
	  ui::Graphics g(&bufDC);
	  ui::Region draw_rgn(new ui::mfc::RegionImp(rgn));
    window()->DrawBackground(&g, draw_rgn);	  
	  OnDevDraw(&g, draw_rgn);
	  g.Dispose();
	  CRect rc;
	  GetClientRect(&rc);
	  dc.BitBlt(0, 0, rc.right - rc.left, rc.bottom - rc.top, &bufDC, 0, 0,
              SRCCOPY);
	  bufDC.SelectObject(oldbmp);
	  bufDC.DeleteDC();
  }
  rgn.DeleteObject();    
}

template<class T, class I>
void WindowTemplateImp<T, I>::OnSize(UINT nType, int cw, int ch) {
  if (bmpDC.m_hObject != NULL) { 
    // remove old buffer to force recreating it with new size
	  TRACE("CanvasView::OnResize(). Deleted bmpDC\n");
	  bmpDC.DeleteObject();	  
  }  
  OnDevSize(ui::Dimension(cw, ch));
  CWnd::OnSize(nType, cw, ch);
}

void WindowImp::DevSetCursor(CursorStyle::Type style) {
  LPTSTR c = 0;
  int ac = 0;
  using namespace CursorStyle;
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
    case CursorStyle::TEXT : c = IDC_IBEAM; break;
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
template class WindowTemplateImp<CButton, ui::RadioButtonImp>;
template class WindowTemplateImp<CButton, ui::GroupBoxImp>;
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
	ON_WM_LBUTTONDOWN()	
	ON_WM_NCHITTEST()
	ON_COMMAND_RANGE(ID_DYNAMIC_MENUS_START, ID_DYNAMIC_MENUS_END, OnDynamicMenuItems)
END_MESSAGE_MAP()

BOOL FrameImp::PreTranslateMessage(MSG* pMsg) {	
  if (pMsg->message==WM_NCRBUTTONDOWN) {    
    ui::Event ev;
    ui::Point point(pMsg->pt.x, pMsg->pt.y);
    ((Frame*)window())->WorkOnContextPopup(ev, point);    
    return ev.is_propagation_stopped();    
  }
	((Frame*)window())->PreTranslateMessage(pMsg);
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

void FrameImp::OnDynamicMenuItems(UINT nID) {
  ui::mfc::MenuContainerImp* mbimp = 
      ui::mfc::MenuContainerImp::MenuContainerImpById(nID);
  if (mbimp != 0) {
    mbimp->WorkMenuItemEvent(nID);
    return;
  }
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

void ButtonImp::dev_set_font(const Font& font) {
   font_ = font;
   mfc::FontImp* imp = dynamic_cast<mfc::FontImp*>(font_.imp());
   assert(imp);   
   ::SendMessage(this->m_hWnd, WM_SETFONT, (WPARAM)(imp->cfont()), TRUE);
}

BEGIN_MESSAGE_MAP(CheckBoxImp, CButton)  
	ON_WM_PAINT()  
  ON_CONTROL_REFLECT(BN_CLICKED, OnClick)
END_MESSAGE_MAP()

void CheckBoxImp::dev_set_font(const Font& font) {
   font_ = font;
   mfc::FontImp* imp = dynamic_cast<mfc::FontImp*>(font_.imp());
   assert(imp);   
   ::SendMessage(this->m_hWnd, WM_SETFONT, (WPARAM)(imp->cfont()), TRUE);
}

void CheckBoxImp::OnClick() {
  OnDevClick();
}

BEGIN_MESSAGE_MAP(RadioButtonImp, CButton)
	ON_WM_PAINT()
	ON_CONTROL_REFLECT(BN_CLICKED, OnClick)
END_MESSAGE_MAP()

void RadioButtonImp::dev_set_font(const Font& font) {
   font_ = font;
   mfc::FontImp* imp = dynamic_cast<mfc::FontImp*>(font_.imp());
   assert(imp);   
   ::SendMessage(this->m_hWnd, WM_SETFONT, (WPARAM)(imp->cfont()), TRUE);
}

void RadioButtonImp::OnClick() {
	OnDevClick();
}

BEGIN_MESSAGE_MAP(GroupBoxImp, CButton)
	ON_WM_PAINT()
	ON_CONTROL_REFLECT(BN_CLICKED, OnClick)
END_MESSAGE_MAP()

void GroupBoxImp::dev_set_font(const Font& font) {
   font_ = font;
   mfc::FontImp* imp = dynamic_cast<mfc::FontImp*>(font_.imp());
   assert(imp);   
   ::SendMessage(this->m_hWnd, WM_SETFONT, (WPARAM)(imp->cfont()), TRUE);
}

BEGIN_MESSAGE_MAP(ComboBoxImp, CComboBox)  
	ON_WM_PAINT()
  ON_CONTROL_REFLECT_EX(CBN_SELENDOK, OnSelect)
END_MESSAGE_MAP()

void ComboBoxImp::dev_set_font(const Font& font) {
   font_ = font;
   mfc::FontImp* imp = dynamic_cast<mfc::FontImp*>(font_.imp());
   assert(imp);   
   ::SendMessage(this->m_hWnd, WM_SETFONT, (WPARAM)(imp->cfont()), TRUE);
}

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
  ON_WM_CTLCOLOR_REFLECT()  
  ON_CONTROL_REFLECT(EN_CHANGE, OnEnChange)
END_MESSAGE_MAP()

HBRUSH EditImp::CtlColor(CDC* pDC, UINT nCtlColor) {
	pDC->SetTextColor(text_color_);	
	pDC->SetBkColor(background_color_);	
	return background_brush_;
}

void EditImp::dev_set_font(const Font& font) {
   font_ = font;
   mfc::FontImp* imp = dynamic_cast<mfc::FontImp*>(font_.imp());
   assert(imp);   
   ::SendMessage(this->m_hWnd, WM_SETFONT, (WPARAM)(imp->cfont()), TRUE);
}

HTREEITEM TreeNodeImp::DevInsert(TreeViewImp* tree, const ui::Node& node,
                                 TreeNodeImp* prev_imp) {
  TVINSERTSTRUCT tvInsert;
  tvInsert.hParent = hItem;
  tvInsert.hInsertAfter = prev_imp ? prev_imp->hItem : TVI_LAST;
  tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;  
  tvInsert.item.iImage = node.image_index();
  tvInsert.item.iSelectedImage =  node.selected_image_index();   
  text_ = Charset::utf8_to_win(node.text());
#ifdef UNICODE
	tvInsert.item.pszText = const_cast<WCHAR *>(text_.c_str());
#else
  tvInsert.item.pszText = const_cast<char *>(text_.c_str());
#endif
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

void TreeViewImp::UpdateNode(const Node::Ptr& node,
                             const Node::Ptr& prev_node) {
  NodeImp* prev_node_imp = prev_node ? prev_node->imp(*this) : 0;
  boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
  node->erase_imp(this);  
  TreeNodeImp* new_imp = new TreeNodeImp();
  node->AddImp(new_imp);
  new_imp->set_node(node);
  new_imp->set_owner(this);
  node->changed.connect(boost::bind(&TreeViewImp::OnNodeChanged, this, _1));
  if (node->parent()) {
    ui::Node* parent_node = node->parent();
    TreeNodeImp* parent_imp =
        dynamic_cast<TreeNodeImp*>(parent_node->imp(*this));
    if (parent_imp) {
      TreeNodeImp* prev_imp = dynamic_cast<TreeNodeImp*>(prev_node_imp);
       new_imp->hItem = parent_imp->DevInsert(this, *node.get(), prev_imp);
       htreeitem_node_map_[new_imp->hItem] = node;      
    }
  }
}

void TreeViewImp::DevUpdate(const Node::Ptr& node,
                            const Node::Ptr& prev_node) {
  recursive_node_iterator end = node->recursive_end();
  recursive_node_iterator it = node->recursive_begin();
  boost::shared_ptr<Node> prev = prev_node;
  UpdateNode(node, prev);
  for ( ; it != end; ++it) {
    UpdateNode((*it), prev);
    prev = *it;
  }
}

void TreeViewImp::DevErase(const Node::Ptr& node) {  
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node->imp(*this));
  if (imp) {
    DeleteItem(imp->hItem);
  } 
}

void TreeViewImp::DevEditNode(const Node::Ptr& node) { 
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node->imp(*this));
  if (imp) {
    EditLabel(imp->hItem);    
  }
}

void TreeViewImp::dev_select_node(const Node::Ptr& node) {
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node->imp(*this));
  if (imp) {  
    SelectItem(imp->hItem);    
  }
}

void TreeViewImp::dev_deselect_node(const Node::Ptr& node) {
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node->imp(*this));
  if (imp) {      
    SetItemState(imp->hItem, 0, TVIS_SELECTED);
  }
}

boost::weak_ptr<Node> TreeViewImp::dev_selected() {
  ui::Node* node = find_selected_node();
  return node ? node->shared_from_this() : boost::weak_ptr<ui::Node>();
}

void TreeViewImp::OnNodeChanged(Node& node) {
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node.imp(*this));  
  if (imp) {      
    SetItemText(imp->hItem, Charset::utf8_to_win(node.text()).c_str());
  }
}

BOOL TreeViewImp::OnChange(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
		tree_view()->change(*tree_view(), node->shared_from_this());
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
  MouseEvent ev(Point(pt.x, pt.y), 1, 0);    
  if (window()) {
    window()->OnDblclick(ev);    
  }
  return FALSE;
}

ui::Node::Ptr TreeViewImp::dev_node_at(const ui::Point& pos) const {
	CPoint pt(TypeConverter::point(pos));
  ScreenToClient(&pt);
  HTREEITEM item = HitTest(pt);  
  if (item) {
    std::map<HTREEITEM, boost::weak_ptr<ui::Node> >::const_iterator it;
    it = htreeitem_node_map_.find(item);
    return (it != htreeitem_node_map_.end()) ? it->second.lock() : ui::Node::Ptr();
  }
  return ui::Node::Ptr();
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
		if (edit) {
		  is_editing_ = true;
      CString s;    
      edit->GetWindowText(s);    
      tree_view()->OnEditing(node->shared_from_this(),
                             Charset::win_to_utf8(s.GetString()));
		}
  }
  return FALSE;
}


BOOL TreeViewImp::OnEndLabelEdit(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    CEdit* edit = GetEditControl();
		if (edit) {
			is_editing_ = false;
			CString s;    
			edit->GetWindowText(s);    
			tree_view()->OnEdited(node->shared_from_this(),
                            Charset::win_to_utf8(s.GetString()));
			tree_view()->edited(*tree_view(), node->shared_from_this(),
                           Charset::win_to_utf8(s.GetString()));
		}
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

HTREEITEM GetNextTreeItem(const CTreeCtrl& treeCtrl, HTREEITEM hItem) {
  // has this item got any children
  if (treeCtrl.ItemHasChildren(hItem)) {
    return treeCtrl.GetNextItem(hItem, TVGN_CHILD);
  } else if (treeCtrl.GetNextItem(hItem, TVGN_NEXT) != NULL) {
  // the next item at this level
    return treeCtrl.GetNextItem(hItem, TVGN_NEXT);
  } else {
    // return the next item after our parent
    hItem = treeCtrl.GetParentItem(hItem);
    if (hItem == NULL) {
      // no parent
      return NULL;
    }
    while (hItem && treeCtrl.GetNextItem(hItem, TVGN_NEXT) == NULL) {
      hItem = treeCtrl.GetParentItem(hItem);									
    }
    // next item that follows our parent
    return treeCtrl.GetNextItem(hItem, TVGN_NEXT);
  }
}

// Functions to expands all items in a tree control
void ExpandAll(CTreeCtrl& treeCtrl) {     
  HTREEITEM hRootItem = treeCtrl.GetRootItem();
  HTREEITEM hItem = hRootItem;
    while (hItem) {
    if (treeCtrl.ItemHasChildren(hItem)) {
      treeCtrl.Expand(hItem, TVE_EXPAND);
    }
    hItem = GetNextTreeItem(treeCtrl, hItem);
  }
}

void TreeViewImp::DevExpandAll() {
	ExpandAll(*this);  
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

void ListNodeImp::DevInsertFirst(ui::mfc::ListViewImp* list, const Node& node,
    ListNodeImp* node_imp, ListNodeImp* prev_imp, int pos) {
  LVITEM lvi;
  lvi.mask =  LVIF_TEXT | LVIF_IMAGE;
  lvi.cColumns = 0;
  node_imp->text_ = Charset::utf8_to_win(node.text());
//  lvi.pszText = const_cast<char *>(node_imp->text_.c_str());
#ifdef UNICODE
	lvi.pszText = const_cast<WCHAR *>(node_imp->text_.c_str());
#else
  lvi.pszText = const_cast<char *>(node_imp->text_.c_str());
#endif
  lvi.iImage = node.image_index();
  lvi.iItem = pos;    
  lvi.iSubItem = 0;
  node_imp->lvi = lvi;
  list->InsertItem(&lvi);  
}

void ListNodeImp::set_text(ui::mfc::ListViewImp* list, const std::string& text) {
  text_ = Charset::utf8_to_win(text);  
#ifdef UNICODE
	lvi.pszText = const_cast<WCHAR *>(text_.c_str());
#else
  lvi.pszText = const_cast<char *>(text_.c_str());
#endif
	
  list->SetItemText(lvi.iItem, lvi.iSubItem, Charset::utf8_to_win(text).c_str());
}

void ListNodeImp::DevSetSub(ui::mfc::ListViewImp* list, const Node& node,
     ListNodeImp* node_imp, ListNodeImp* prev_imp, int level) {
  LVITEM lvi;
  lvi.mask =  LVIF_TEXT | LVIF_IMAGE;
  lvi.cColumns = 0;
  node_imp->text_ = Charset::utf8_to_win(node.text());  
#ifdef UNICODE
	lvi.pszText = const_cast<WCHAR *>(node_imp->text_.c_str());
#else
  lvi.pszText = const_cast<char *>(node_imp->text_.c_str());
#endif
  lvi.iImage = node.image_index(); 
  lvi.iItem = position();
  lvi.iSubItem = level - 1;
  node_imp->lvi = lvi;
  list->SetItem(&lvi);
  node_imp->set_position(position());
}

void ListNodeImp::Select(ui::mfc::ListViewImp* list_view_imp) {
  list_view_imp->SetItemState(lvi.iItem, LVIS_SELECTED, LVIS_SELECTED);
  list_view_imp->SetSelectionMark(lvi.iItem);
}

void ListNodeImp::Deselect(ui::mfc::ListViewImp* list_view_imp) {
  list_view_imp->SetItemState(lvi.iItem,
      static_cast<UINT>(~LVIS_SELECTED), LVIS_SELECTED);
}

ListNodeImp* ListViewImp::UpdateNode(const Node::Ptr& node,
                                     const Node::Ptr& prev_node, int pos) {
  ListNodeImp* new_imp = new ListNodeImp();  
  NodeImp* prev_node_imp = prev_node ? prev_node->imp(*this) : 0;
  node->erase_imp(this);  
  ImpLookUpIterator it = lookup_table_.find(pos);
  if (it != lookup_table_.end()) {
    lookup_table_.erase(it);
  }  
  node->AddImp(new_imp);
  new_imp->set_node(node);
  new_imp->set_owner(this);  
  node->changed.connect(boost::bind(&ListViewImp::OnNodeChanged, this, _1));
  if (node->parent()) {
    ui::Node* parent_node = node->parent();
    boost::ptr_list<NodeImp>::iterator it = parent_node->imps.begin();
    for ( ; it != parent_node->imps.end(); ++it) {
      ListNodeImp* parent_imp =
          dynamic_cast<ListNodeImp*>(parent_node->imp(*this));
      if (parent_imp) {             
        ListNodeImp* prev_imp = dynamic_cast<ListNodeImp*>(prev_node_imp);
        int level = node->level();
        if (level == 1) {
          parent_imp->DevInsertFirst(this, *node.get(), new_imp, prev_imp,
                                     pos);
          lookup_table_[pos] = new_imp;
        } else {
          parent_imp->DevSetSub(this, *node.get(), new_imp, prev_imp, level);
          lookup_table_[new_imp->position()] = new_imp;
        }
      }
    }
  }
  return new_imp;
}

void ListViewImp::DevUpdate(const Node::Ptr& node,
                            const Node::Ptr& prev_node) {  
  recursive_node_iterator end = node->recursive_end();
  recursive_node_iterator it = node->recursive_begin();  
  int pos = 0;
  if (prev_node) {        
    ListNodeImp* prev_imp = dynamic_cast<ListNodeImp*>(prev_node->imp(*this));
    if (prev_imp) {
      pos = prev_imp->position() + 1;
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

void ListViewImp::DevErase(const Node::Ptr& node) {
	if (node) {
		node->erase_imp(this);    
	}
}

void ListViewImp::DevEditNode(const Node::Ptr& node) {    
  ListNodeImp* imp = dynamic_cast<ListNodeImp*>(node->imp(*this));
  if (imp) {
   //EditLabel(imp->hItem);    
  }
}

void ListViewImp::dev_select_node(const Node::Ptr& node) {
  ListNodeImp* imp = dynamic_cast<ListNodeImp*>(node->imp(*this));
  if (imp) {
    int selected = -1;
    POSITION pos = GetFirstSelectedItemPosition();
    if (pos != NULL) {
      while (pos) {
        selected = GetNextSelectedItem(pos);      
      }
    } 
    if (selected != -1) {     
      SetItemState(selected, static_cast<UINT>(~LVIS_SELECTED), LVIS_SELECTED);
    }
    imp->Select(this);           
  }
}

void ListViewImp::dev_deselect_node(const Node::Ptr& node) {
  ListNodeImp* imp = dynamic_cast<ListNodeImp*>(node->imp(*this));
  if (imp) {
    imp->Deselect(this);    
  }
}

boost::weak_ptr<Node> ListViewImp::dev_selected() {
  Node* node = find_selected_node();
  return node ? node->shared_from_this() : boost::weak_ptr<Node>();
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
  Node* result = 0;
  POSITION pos = GetFirstSelectedItemPosition();
  int selected = -1;
  if (pos != NULL) {
    while (pos) {
      selected = GetNextSelectedItem(pos);      
    }
  } 
  if (selected != -1) {
    ImpLookUpIterator it = lookup_table_.find(selected);
    if (it != lookup_table_.end() && !it->second->node().expired()) {            
      result = it->second->node().lock().get();      
    }
  }  
  return result;
}

std::vector<ui::Node::Ptr> ListViewImp::dev_selected_nodes() {
  std::vector<ui::Node::Ptr> nodes;
  POSITION pos = GetFirstSelectedItemPosition();
  int selected = -1;
  if (pos != NULL) {
    while (pos) {
      selected = GetNextSelectedItem(pos);
      if (selected != -1) {
        ImpLookUpIterator it = lookup_table_.find(selected);
        if (it != lookup_table_.end() && !it->second->node().expired()) {          
          nodes.push_back(it->second->node().lock());                     
        }
      }
    } 
  }
  return nodes;
}
 
BOOL ListViewImp::OnBeginLabelEdit(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    CEdit* edit = GetEditControl();
		if (edit) {
      CString s;    
      edit->GetWindowText(s);
      is_editing_ = true;
      list_view()->OnEditing(node->shared_from_this(),
                             Charset::win_to_utf8(s.GetString()));
		}
  }
  return FALSE;
}


BOOL ListViewImp::OnEndLabelEdit(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    CEdit* edit = GetEditControl();
		if (edit) {
		  is_editing_ = false;
      CString s;    
      edit->GetWindowText(s);    
      list_view()->OnEdited(node->shared_from_this(),
                            Charset::win_to_utf8(s.GetString()));
		}
  }
  return FALSE;
}

void ListViewImp::DevClear() {   
  DeleteAllItems();
}

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
      case SB_THUMBTRACK:
      dev_set_scroll_position(nPos);
      break;
    }
    
    OnDevScroll(nPos);
  }
  return  WindowTemplateImp<CScrollBar, ui::ScrollBarImp>::OnChildNotify(uMsg,
      wParam, lParam, pResult);
}

IMPLEMENT_DYNAMIC(ScintillaImp, CWnd)

BEGIN_MESSAGE_MAP(ScintillaImp, CWnd)
ON_WM_ERASEBKGND()
  ON_NOTIFY_REFLECT_EX(SCN_CHARADDED, OnModified)
  ON_NOTIFY_REFLECT_EX(SCN_MARGINCLICK, OnMarginClick)         
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
  f(SCI_STYLESETBACK, SCE_LUA_CHARACTER, ToCOLORREF(lexer.character_code_color()));    
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
  f(SCI_SETAUTOMATICFOLD,
    SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CHANGE| SC_AUTOMATICFOLD_CLICK,
    0);
}

void ScintillaImp::SetFoldingColors(const Lexer& lexer) {
 for (int i = 25; i <= 31; i++) {    
   f(SCI_MARKERSETFORE, i, 0x0); //ToCOLORREF(lexer.folding_marker_fore_color()));
   f(SCI_MARKERSETBACK, i, 0xFFFFFF); // ToCOLORREF(lexer.folding_marker_back_color()));
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

BOOL ScintillaImp::OnMarginClick(NMHDR * nhmdr,LRESULT *) { 
  SCNotification *pMsg = (SCNotification*)nhmdr;      
  long line_pos = f(SCI_LINEFROMPOSITION, pMsg->position, 0);
  // f(SCI_TOGGLEFOLD, lLine, 0);  
  Scintilla* scintilla = dynamic_cast<Scintilla*>(window());
  if (scintilla) {
    scintilla->OnMarginClick(line_pos);
  }
  return 0L;
}

// MenuContainerImp
std::map<int, MenuContainerImp*> MenuContainerImp::menu_bar_id_map_;



void MenuContainerImp::set_menu_window(CWnd* menu_window, 
                                       const Node::Ptr& root_node) {
  menu_window_ = menu_window;  
  if (!menu_window && root_node) {
    hmenu_ = 0;
    root_node->erase_imps(this);    
  } else {
    hmenu_ = menu_window->GetMenu()->m_hMenu;
    DevUpdate(root_node, ui::Node::Ptr());
  }
}

void MenuContainerImp::DevInvalidate() {
  if (menu_window_) {
    menu_window_->DrawMenuBar();
  }
}

void MenuContainerImp::DevUpdate(const Node::Ptr& node, 
                                 const Node::Ptr& prev_node) {
  if (hmenu_) {		
    UpdateNodes(node, hmenu_, ::GetMenuItemCount(hmenu_));
    DevInvalidate();
  }
}

void MenuContainerImp::RegisterMenuEvent(int id, MenuImp* menu_imp) {
  menu_item_id_map_[id] = menu_imp;
  menu_bar_id_map_[id] = this;
}

void MenuContainerImp::UpdateNodes(const Node::Ptr& parent_node, HMENU parent,
                                   int pos_start) {
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
      menu_imp->dev_set_position(pos);      
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
              UpdateNodes(node, menu_imp->hmenu());
            }
            break;
          }
        }        
      }
    }
  }
}

void MenuContainerImp::DevErase(const Node::Ptr& node) {
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
     void operator()(Node::Ptr node, Node::Ptr prev_node) {
       boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
       for ( ; it != node->imps.end(); ++it) {         
         MenuImp* imp = dynamic_cast<MenuImp*>(&(*it));
          if (imp) {
            if (imp->id() == selectedItemID) {
             bar->OnMenuItemClick(node->shared_from_this());
						 bar->menu_item_click(*bar, node->shared_from_this());
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
  std::map<int, MenuImp*>::iterator it = menu_item_id_map_.find(id);
  return (it != menu_item_id_map_.end()) ? it->second : 0;
}

MenuContainerImp* MenuContainerImp::MenuContainerImpById(int id) {
  std::map<int, MenuContainerImp*>::iterator it = menu_bar_id_map_.find(id);
  return (it != menu_bar_id_map_.end()) ? it->second : 0;
}

// PopupMenuImp
PopupMenuImp::PopupMenuImp() : popup_menu_(0) {
  popup_menu_ = ::CreatePopupMenu();
  set_hmenu(popup_menu_);
}

void PopupMenuImp::DevTrack(const ui::Point& pos) {
  ::TrackPopupMenu(popup_menu_,
    TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_HORPOSANIMATION | TPM_VERPOSANIMATION,
    static_cast<int>(pos.x()), static_cast<int>(pos.y()), 0,
    ::AfxGetMainWnd()->m_hWnd, 0);
}

//MenuImp
MenuImp::~MenuImp()  {  
  if (parent_ && ::IsMenu(parent_)) {      
     RemoveMenu(parent_, pos_, MF_BYPOSITION);    
   } else {     
     ::DestroyMenu(hmenu_);     
   }  
}

void MenuImp::CreateMenu(const std::string& text) {
  hmenu_ = ::CreateMenu();  
  AppendMenu(parent_, MF_POPUP | MF_ENABLED, (UINT_PTR)hmenu_,
             Charset::utf8_to_win(text).c_str());
  UINT count = ::GetMenuItemCount(parent_);
  pos_ = count - 1;
}

void MenuImp::CreateMenuItem(const std::string& text, ui::Image* image) {
  if (text == "-") {
    ::AppendMenu(parent_, MF_SEPARATOR, 0, NULL);  
  } else {
    id_ = ID_DYNAMIC_MENUS_START + ui::MenuContainer::id_counter++;
    ::AppendMenu(parent_, MF_STRING |  MF_ENABLED, id_,
                 Charset::utf8_to_win(text).c_str());
    if (image) {  
			assert(image->imp());			
			ImageImp* imp = dynamic_cast<ImageImp*>(image->imp());
			assert(imp);
      ::SetMenuItemBitmaps(parent_, id_, MF_BYCOMMAND,
                           (HBITMAP)imp->dev_source()->m_hObject,
                           (HBITMAP)imp->dev_source()->m_hObject);
    }
  }
}

// GameController
void GameControllersImp::DevScanPluggedControllers(
     std::vector<int>& plugged_controller_ids) {
  UINT num = joyGetNumDevs();
  UINT game_controller_id = 0;
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
  controller.set(joy_info.wXpos, joy_info.wYpos, joy_info.wZpos,
                 static_cast<int>(joy_info.wButtons));
}


/*extern "C" bool comp(FontPair& left, FontPair& right)
{
	if ((_T('@') != left.first.lfFaceName[0]) && (_T('@') == right.first.lfFaceName[0]))
		return true;
	if ((_T('@') == left.first.lfFaceName[0]) && (_T('@') != right.first.lfFaceName[0]))
		return false;
	return (_tcscmp(left.first.lfFaceName, right.first.lfFaceName) < 0);
}*/

extern "C" int CALLBACK enumerateFontsCallBack(ENUMLOGFONTEX *lpelf,
												NEWTEXTMETRICEX *lpntm,
												DWORD fontType, LPARAM lParam) {
	lpntm;

	FontVec* pFontVec = (FontVec*) lParam;
	pFontVec->push_back(FontPair(lpelf->elfLogFont, fontType));
	return TRUE;
}

} // namespace mfc
} // namespace ui
} // namespace host
} // namespace psycle