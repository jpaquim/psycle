/** @file 
 *  @brief SequenceBar dialog
 *  $Date: 2010-08-15 18:18:35 +0200 (dg., 15 ag 2010) $
 *  $Revision: 9831 $
 */
#include <psycle/host/detail/project.private.hpp>
#include "ExtensionBar.hpp"

#include "PsycleConfig.hpp"
#include "MainFrm.hpp"
#include "ChildView.hpp"

namespace psycle{ namespace host{

	extern CPsycleApp theApp;

	IMPLEMENT_DYNAMIC(ExtensionBar, CDialogBar)

	ExtensionBar::ExtensionBar()
	   : m_luaWndView(new CWnd()),
		 minimum_dimension_(ui::Dimension(500, 200))
	{
	  m_sizeFloating.SetSize(500, 200);
	}

	ExtensionBar::~ExtensionBar()
	{
	}

	int ExtensionBar::OnCreate(LPCREATESTRUCT lpCreateStruct) {
		if (CDialogBar::OnCreate(lpCreateStruct) == -1)
			return -1;
		m_luaWndView->Create(NULL, NULL, WS_CHILD | WS_VISIBLE,
			CRect(10, 0, 1024, 60), this, AFX_IDW_PANE_FIRST + 1, NULL);		
		return 0;
	}

	void ExtensionBar::DoDataExchange(CDataExchange* pDX)
	{
		CDialogBar::DoDataExchange(pDX);
	}

	//Message Maps are defined in CMainFrame, since this isn't a window, but a DialogBar.
	BEGIN_MESSAGE_MAP(ExtensionBar, CDialogBar)
		ON_WM_CREATE()
		ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
	END_MESSAGE_MAP()

	void ExtensionBar::InitializeValues(CMainFrame* frame, CChildView* view)
	{
		m_pParentMain = frame;
		m_pWndView = view;		
	}

	// SequenceBar message handlers
	LRESULT ExtensionBar::OnInitDialog ( WPARAM wParam, LPARAM lParam)
	{
		BOOL bRet = HandleInitDialog(wParam, lParam);

		if (!UpdateData(FALSE))
		{
		   TRACE0("Warning: UpdateData failed during dialog init.\n");
		}

		return bRet;
	}

	CSize ExtensionBar::CalcDynamicLayout(int length, DWORD dwMode) {
	    if (docked(dwMode)) {
			CRect rc;
			m_pDockBar->GetParent()->GetClientRect(&rc);
			int min_height = minimum_dimension_.height();
			m_luaWndView->MoveWindow(CRect(10, 0, rc.Width(), min_height));
			Resize();
			return CSize(rc.Width(), min_height);
		}
		if (dwMode & LM_MRUWIDTH) {
			m_luaWndView->MoveWindow(CRect(0, 0, m_sizeFloating.cx, m_sizeFloating.cy));
			CWnd* child = m_luaWndView->GetWindow(GW_CHILD);
			if (child) {
				child->MoveWindow(0, 0,  m_sizeFloating.cx, m_sizeFloating.cy);       
			}
			return m_sizeFloating;
		} 
		if (dwMode & LM_LENGTHY) {
			m_sizeFloating.cy = length;
			m_luaWndView->MoveWindow(CRect(0, 0, m_sizeFloating.cx, m_sizeFloating.cy));
			CWnd* child = m_luaWndView->GetWindow(GW_CHILD);
			if (child) {
				child->MoveWindow(0, 0,  m_sizeFloating.cx, m_sizeFloating.cy);       
			}
			return CSize(m_sizeFloating.cx, m_sizeFloating.cy);
		}
		else {
			m_sizeFloating.cx = length;
			m_luaWndView->MoveWindow(CRect(0, 0, m_sizeFloating.cx, m_sizeFloating.cy));
			CWnd* child = m_luaWndView->GetWindow(GW_CHILD);
			if (child) {
				child->MoveWindow(0, 0,  m_sizeFloating.cx, m_sizeFloating.cy);       
			}
			return CSize(m_sizeFloating.cx, m_sizeFloating.cy);
		}		
	}

	void ExtensionBar::Resize() {	 
		CRect rc;
		m_pDockBar->GetParent()->GetClientRect(&rc);
		int min_height = minimum_dimension_.height();
		CWnd* child = m_luaWndView->GetWindow(GW_CHILD);
		if (child) {
			child->MoveWindow(0, 0,  rc.Width(), min_height);        
		}
	}

	void ExtensionBar::set_minimum_dimension(const ui::Dimension& dimension) {
		minimum_dimension_ = dimension;
		CFrameWnd* frm = (CFrameWnd*) (::AfxGetMainWnd());
		frm->RecalcLayout();
	}

	void ExtensionBar::ResizeFloating() {
		m_luaWndView->MoveWindow(CRect(0, 0, m_sizeFloating.cx, m_sizeFloating.cy));
		CWnd* child = m_luaWndView->GetWindow(GW_CHILD);
		if (child) {
			child->MoveWindow(0, 0,  m_sizeFloating.cx, m_sizeFloating.cy);       
		}
	}

}}
