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
  return ui::Rect(ui::Point(rc.left, rc.top), ui::Point(rc.right, rc.bottom));
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
  return ui::Rect(dev_pos_, ui::Dimension(rc.Width(), rc.Height()));
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
  area.Add(RectShape(ui::Rect(ui::Point(), ui::Point(pos.Width(), pos.Height()))));
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
template class WindowTemplateImp<CListCtrl, ui::TableImp>;
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
  ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnClick)
END_MESSAGE_MAP()

BOOL TreeImp::OnClick(NMHDR * pNotifyStruct, LRESULT * result) {
  HTREEITEM hTtreeSelectedItem = GetSelectedItem();
  std::map<HTREEITEM, TreeNodeImp*>::iterator it;
  it = tree_nodes.find(hTtreeSelectedItem);
  if (it != tree_nodes.end()) {
    it->second->OnDevClick();  
  }
  return FALSE;
}

void TreeNodeImp::dev_set_tree(const boost::weak_ptr<ui::TreeNode>& parent,
                               boost::weak_ptr<ui::Tree> tree) {
  HTREEITEM p = 0;
  if (!parent.expired()) {
    p = ((mfc::TreeNodeImp*) parent.lock()->imp())->hItem;
  }
  TVINSERTSTRUCT tvInsert;
  tvInsert.hParent = p;
  tvInsert.hInsertAfter = p;
  tvInsert.item.mask = TVIF_TEXT;  
  tvInsert.item.pszText = const_cast<char *>(text_.c_str());  
  mfc::TreeImp* treectrl = (mfc::TreeImp*) tree.lock()->imp();
  hItem = treectrl->InsertItem(&tvInsert);
  ((TreeImp*)tree.lock()->imp())->tree_nodes[hItem] = this;
}

void TreeImp::DevAddNode(ui::TreeNode& node) {  
  TVINSERTSTRUCT tvInsert;
  tvInsert.hParent = NULL;
  tvInsert.hInsertAfter = NULL;
  tvInsert.item.mask = TVIF_TEXT;
  std::string text(node.text());
  tvInsert.item.pszText = const_cast<char *>(text.c_str());
  //ui::mfc::TreeNodeImp* mfc_node = (ui::mfc::TreeNodeImp*)(node.imp_.get());
  InsertItem(&tvInsert);
}

BEGIN_MESSAGE_MAP(TableImp, CListCtrl)  
  ON_WM_ERASEBKGND()
	ON_WM_PAINT()  
END_MESSAGE_MAP()

void TableImp::test() {
  // Insert columns
 /* InsertColumn(0, "One", LVCFMT_LEFT, -1, 0);
  InsertColumn(1, "Two", LVCFMT_LEFT, -1, 1);
// Insert first row
   int Index = InsertItem(LVIF_TEXT, 0, "One one", 0, 0, 0, NULL);
   SetItem(Index, 1, LVIF_TEXT, "One two", 0, 0, 0, NULL);
   //Insert second row
   Index = InsertItem(LVIF_TEXT, 1, "Two one", 0, 0, 0, NULL);
   SetItem(Index, 1, LVIF_TEXT, "Two two", 0, 0, 0, NULL);*/
// Set column widths (an optional nice touch)
//   SetColumnWidth(0, LVSCW_AUTOSIZE);
   //SetColumnWidth(1, LVSCW_AUTOSIZE);   
}

void TableImp::DevInsertColumn(int col, const std::string& text) {
  InsertColumn(col, text.c_str(), LVCFMT_LEFT, -1, 0);
}

void TableImp::DevInsertRow() {
}

int TableImp::DevInsertText(int nItem, const std::string& text) {
  int index = InsertItem(LVIF_TEXT, nItem, text.c_str(), 0, 0, 0, NULL);
  return index;
}

void TableImp::DevSetText(int nItem, int nSubItem, const std::string& text) {
  SetItem(nItem, nSubItem, LVIF_TEXT, text.c_str(), 0, 0, 0, NULL);  
}

void TableImp::DevAutoSize(int cols) {
  for (int i = 0; i < cols; i++) {
    SetColumnWidth(i, LVSCW_AUTOSIZE);    
  }
}

void TableItemImp::dev_set_table(boost::weak_ptr<ui::Table> table) {
  /*HTREEITEM p = 0;
  if (!parent.expired()) {
    p = ((mfc::TreeNodeImp*) parent.lock()->imp())->hItem;
  }
  TVINSERTSTRUCT tvInsert;
  tvInsert.hParent = p;
  tvInsert.hInsertAfter = p;
  tvInsert.item.mask = TVIF_TEXT;  
  tvInsert.item.pszText = const_cast<char *>(text_.c_str());  
  mfc::TreeImp* treectrl = (mfc::TreeImp*) tree.lock()->imp();
  hItem = treectrl->InsertItem(&tvInsert);
  ((TreeImp*)tree.lock()->imp())->tree_nodes[hItem] = this;*/
}

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