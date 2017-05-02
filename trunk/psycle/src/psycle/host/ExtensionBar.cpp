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
#include "Canvas.hpp"
#include "LuaPlugin.hpp"
#include "MfcUi.hpp"

namespace psycle {
namespace host {

	ExtensionWindow::ExtensionWindow()
	{
		using namespace ui;
		set_aligner(Aligner::Ptr(new DefaultAligner()));
	}
					
	void ExtensionWindow::Pop()
	{
		if (HasExtensions()) {			
			RemoveAll();
			extensions_.pop();
		}
		if (extensions_.empty()) {
			Hide();
		}
	}
		
	void ExtensionWindow::DisplayTop()
	{
		if (HasExtensions()) {
			using namespace ui;	
			Viewport::Ptr top = extensions_.top().lock()->viewport().lock();
			top->set_align(AlignStyle::CLIENT);
			Add(top, false);
			top->PreventFls();			
			UpdateAlign();
			top->EnableFls();
		}
	}	

	void ExtensionWindow::Push(LuaPlugin& plugin)
	{						 					 		 
		if (!plugin.viewport().expired()) {
			if (HasExtensions()) {
				RemoveAll();
			} else {
				Show();
			}
			if (!HasExtensions() || extensions_.top().lock().get() != &plugin) {
				extensions_.push(plugin.shared_from_this());
			}
		}			
	}
		
	void ExtensionWindow::RemoveExtensions()
	{						
		if (HasExtensions()) {			
			RemoveAll();
			extensions_ = Extensions();
			Hide();
		}
	}
		
	void ExtensionWindow::UpdateMenu(MenuHandle& menu_handle)
	{
		menu_handle.clear();
		if (HasExtensions()) {
			menu_handle.set_menu(extensions_.top().lock()->proxy().menu_root_node());
		}
		::AfxGetMainWnd()->DrawMenuBar();
	}						


	extern CPsycleApp theApp;

	IMPLEMENT_DYNAMIC(ExtensionBar, CDialogBar)

	ExtensionBar::ExtensionBar()
	{
		m_sizeFloating.SetSize(500, 200);
	}

	ExtensionBar::~ExtensionBar()
	{
	}

	int ExtensionBar::OnCreate(LPCREATESTRUCT lpCreateStruct) {
		if (CDialogBar::OnCreate(lpCreateStruct) == -1) {
			return -1;
		}
		m_luaWndView.reset(new ExtensionWindow());
		ui::mfc::WindowImp* mfc_imp = (ui::mfc::WindowImp*) m_luaWndView->imp();
		mfc_imp->SetParent(this);		
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
		using namespace ui;
	    if (docked(dwMode)) {
			CRect rc;
			m_pDockBar->GetParent()->GetClientRect(&rc);
			m_luaWndView->set_position(Rect(Point(10, 0),
				Dimension(rc.Width(), m_luaWndView->min_dimension().height())));
			return CSize(rc.Width(), m_luaWndView->min_dimension().height());
		}
		if (dwMode & LM_MRUWIDTH) {
			m_luaWndView->set_position(Rect(Point(), Dimension(m_sizeFloating.cx, m_sizeFloating.cy)));			
			return m_sizeFloating;
		} 
		if (dwMode & LM_LENGTHY) {
			m_sizeFloating.cy = length;
			m_luaWndView->set_position(Rect(Point(), Dimension(m_sizeFloating.cx, m_sizeFloating.cy)));			
			return CSize(m_sizeFloating.cx, m_sizeFloating.cy);
		}
		else {
			m_sizeFloating.cx = length;
			m_luaWndView->set_position(Rect(Point(), Dimension(m_sizeFloating.cx, m_sizeFloating.cy)));			
			return CSize(m_sizeFloating.cx, m_sizeFloating.cy);
		}		
	}
	
	void ExtensionBar::Add(LuaPlugin& plugin)
	{
		m_luaWndView->set_min_dimension(plugin.viewport().lock()->min_dimension());									
		m_luaWndView->Push(plugin);				
		m_luaWndView->DisplayTop();
		((CMainFrame*)::AfxGetMainWnd())->RecalcLayout();		
	}	

}}
