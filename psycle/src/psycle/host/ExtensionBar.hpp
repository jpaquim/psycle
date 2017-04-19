#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "ExListBox.h"
#include "Ui.hpp"

namespace psycle { namespace host {
	class CMainFrame;
	class CChildView;	

	class ExtensionBar : public CDialogBar
	{
		friend class CMainFrame;
		DECLARE_DYNAMIC(ExtensionBar)
	public:
		ExtensionBar();   // standard constructor
		virtual ~ExtensionBar();
		
		void InitializeValues(CMainFrame* frame, CChildView* view);
		void set_minimum_dimension(const ui::Dimension& dimension);
		boost::shared_ptr<CWnd> m_luaWndView;
		void Resize();

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			
		DECLARE_MESSAGE_MAP()
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg LRESULT OnInitDialog ( WPARAM , LPARAM );									
		void ResizeFloating();			
		CMainFrame* m_pParentMain;
		CChildView*  m_pWndView;		
		CSize CalcDynamicLayout(int length, DWORD dwMode);	

	private:
		ui::Dimension minimum_dimension_;
		CSize m_sizeFloating;
		bool docked(DWORD mode) const { return (mode & LM_VERTDOCK) || (mode & LM_HORZDOCK); }
	};
}}
