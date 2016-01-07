#include <psycle/host/detail/project.hpp>
#include "MfcUi.hpp"
#include "Psycle.hpp"

namespace psycle {
namespace host {
namespace ui {
namespace mfc {

std::map<HWND, WindowImp*> WindowImp::windows_;

HHOOK WindowImp::_hook = 0;


// CanvasView
BEGIN_MESSAGE_MAP(WindowImp, CWnd)  
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_WM_SETFOCUS()
	ON_WM_PAINT()
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

BOOL WindowImp::PreTranslateMessage(MSG* pMsg) {
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


BOOL WindowImp::PreCreateWindow(CREATESTRUCT& cs) {
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;
	// CS_HREDRAW | CS_VREDRAW |
	cs.dwExStyle |= WS_EX_CLIENTEDGE;
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
}

int WindowImp::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}
  windows_[GetSafeHwnd()] = this;
  // Set the hook
  SetFocusHook();  
	return 0;
}

void WindowImp::OnDestroy() {  
  std::map<HWND, WindowImp*>::iterator it = windows_.find(GetSafeHwnd());
  if (it != windows_.end()) {
    windows_.erase(it);
  }
  bmpDC.DeleteObject();
}

void WindowImp::OnPaint() {
  CRgn rgn;
  rgn.CreateRectRgn(0, 0, 0, 0);
	int result = GetUpdateRgn(&rgn, FALSE);

  if (!result) return; // If no area to update, exit.
	
	CPaintDC dc(this);

  if (!bmpDC.m_hObject) { // buffer creation	
		CRect rc;
		GetClientRect(&rc);		
		bmpDC.CreateCompatibleBitmap(&dc, rc.right - rc.left, rc.bottom - rc.top);
		char buf[128];
		sprintf(buf,"CanvasView::OnPaint(). Initialized bmpDC to 0x%p\n",(void*)bmpDC);
		TRACE(buf);
	}
  CDC bufDC;
	bufDC.CreateCompatibleDC(&dc);
	CBitmap* oldbmp = bufDC.SelectObject(&bmpDC);	
  ui::mfc::Graphics g(&bufDC);
  ui::mfc::Region draw_rgn(rgn);
  OnDevDraw(&g, draw_rgn);
  g.Dispose();
  CRect rc;
  GetClientRect(&rc);
	dc.BitBlt(0, 0, rc.right-rc.left, rc.bottom-rc.top, &bufDC, 0, 0, SRCCOPY);
	bufDC.SelectObject(oldbmp);
	bufDC.DeleteDC();
  rgn.DeleteObject();    
}

void WindowImp::OnSize(UINT nType, int cw, int ch) {  
  if (bmpDC.m_hObject != NULL) { // remove old buffer to force recreating it with new size
	  TRACE("CanvasView::OnResize(). Deleted bmpDC\n");
	  bmpDC.DeleteObject();	  
  }  
  OnDevSize(cw, ch);
  CWnd::OnSize(nType, cw, ch);
}

LRESULT __stdcall WindowImp::HookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    CWPSTRUCT* info = (CWPSTRUCT*) lParam;        
    if (info->message == WM_SETFOCUS) {
      FilterHook(info->hwnd);                     
    }
  }
  return CallNextHookEx(_hook , nCode, wParam, lParam);       
}

void WindowImp::FilterHook(HWND hwnd) {  
  typedef std::map<HWND, WindowImp*> WindowList;
  WindowList::iterator it = windows_.begin();
  for (; it != windows_.end(); ++it) {
    WindowImp* window = (*it).second;
    if (::IsChild(window->GetSafeHwnd(), hwnd)) {
      window->OnDevFocusChange(GetFocus()->GetDlgCtrlID());
    }
  } 
}

void WindowImp::SetFocusHook() {
  if (_hook) {
    return;
  }
  if (!(_hook = SetWindowsHookEx(WH_CALLWNDPROC, 
                                 WindowImp::HookCallback,
                                 AfxGetInstanceHandle(),
                                 GetCurrentThreadId()))) {
    TRACE(_T("ui::canvas::MFCView : Failed to install hook!\n"));
  }
}

void WindowImp::ReleaseHook() { 
  UnhookWindowsHookEx(_hook);
}


} // namespace mfc
} // namespace ui
} // namespace host
} // namespace psycle