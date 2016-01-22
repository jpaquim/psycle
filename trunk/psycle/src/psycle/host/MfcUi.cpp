#include <psycle/host/detail/project.hpp>
#include "MfcUi.hpp"
#include "Psycle.hpp"

namespace psycle {
namespace host {
namespace ui {
namespace mfc {

Region::Region() { Init(0, 0, 0, 0); }
  
Region::Region(int x, int y, int width, int height) {
  Init(x, y, width, height);
}  

Region::Region(const CRgn& rgn)  {
  assert(rgn.m_hObject);
  Init(0, 0, 0, 0);
  rgn_.CopyRgn(&rgn);    
}
  
Region::~Region() {
  rgn_.DeleteObject();
}
  
Region* Region::Clone() const {
  return new Region(rgn_);  
}  

void Region::Offset(double dx, double dy) {
  CPoint pt(dx, dy);
  rgn_.OffsetRgn(pt);
}

int Region::Combine(const ui::Region& other, int combinemode) {    
  return rgn_.CombineRgn(&rgn_, (const CRgn*)other.source(), combinemode);
} 

ui::Rect Region::bounds() const { 
  CRect rc;
  rgn_.GetRgnBox(&rc);
  return ui::Rect(rc.left, rc.top, rc.right, rc.bottom);    
}

bool Region::Intersect(double x, double y) const {
  return rgn_.PtInRegion(x, y);  
}

bool Region::IntersectRect(double x, double y, double width, double height) const {
  CRect rc(x, y, x + width, y + height);
  return rgn_.RectInRegion(rc);  
}
  
void Region::SetRect(double x, double y, double width, double height) {    
  rgn_.SetRectRgn(x, y, x + width, y + height); 
}

void Region::Clear() {
  SetRect(0, 0, 0, 0);  
}

void* Region::source() {
  return &rgn_;
}
  
const void* Region::source() const {
  return &rgn_;
};
  
void Region::Init(int x, int y, int width, int height) {   
  rgn_.CreateRectRgn(x, y, x+width, y+height);  
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
  ON_WM_KEYDOWN()
  ON_WM_KEYUP()  
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
      return ev.is_prevent_default();      
    }          
  } else
  if (pMsg->message == WM_KEYUP) {      
    CWnd* hwndTest = GetFocus();
    if (hwndTest) {
      UINT nFlags = 0;
      UINT flags = Win32KeyFlags(nFlags);
      KeyEvent ev(pMsg->wParam, flags);
      OnDevKeyUp(ev);
      return ev.is_prevent_default();
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
  MoveWindow(pos.left(),
             pos.top(),
             pos.width(),
             pos.height());
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::dev_pos() const {
  CRect rc;
  GetClientRect(&rc);
  return ui::Rect(dev_pos_.x(),
                  dev_pos_.y(),
                  dev_pos_.x() + rc.Width(),
                  dev_pos_.y() + rc.Height());
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
  ui::mfc::Graphics g(&dc);
  ui::mfc::Region draw_rgn(rgn);
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
  area.Add(RectShape(ui::Rect(0, 0, pos.Width(), pos.Height())));
  return true;
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
template class WindowTemplateImp<CTreeCtrl, ui::TreeImp>;
template class WindowTemplateImp<CFrameWnd, ui::FrameImp>;

BEGIN_MESSAGE_MAP(FrameImp, CFrameWnd)
  ON_WM_SIZE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(ButtonImp, CButton)  
	ON_WM_PAINT()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(EditImp, CEdit)  
	ON_WM_PAINT()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(TreeImp, CTreeCtrl)  
  ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(ScrollBarImp, CScrollBar)  
	ON_WM_PAINT()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(ScintillaImp, CWnd)

BEGIN_MESSAGE_MAP(ScintillaImp, CWnd)
ON_NOTIFY_REFLECT_EX(SCN_CHARADDED, OnModified)
END_MESSAGE_MAP()

void ScintillaImp::dev_set_lexer(const Lexer& lexer) {
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
  WindowList::iterator it = windows_.begin();
  for (; it != windows_.end(); ++it) {
    ui::WindowImp* window = (*it).second;
    if (::IsChild(((CWnd*)window)->GetSafeHwnd(), hwnd)) {
      window->OnDevFocusChange(((CWnd*)window)->GetFocus()->GetDlgCtrlID());
    }
  } 
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

} // namespace mfc
} // namespace ui
} // namespace host
} // namespace psycle