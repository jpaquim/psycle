#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "ExListBox.h"
#include "Ui.hpp"

namespace psycle { namespace host {
		
	class LuaPlugin;

	class ExtensionWindow : public ui::Group {
		public:
			ExtensionWindow();
			~ExtensionWindow() {}

			void Push(LuaPlugin& plugin);			   
			
			void Push();
			void Pop();
			void DisplayTop();
			bool HasExtensions() const { return !extensions_.empty(); }
			void RemoveExtensions();
			void UpdateMenu(class MenuHandle& menu_handle);

		protected:
			virtual void OnSize(const ui::Dimension& dimension) { UpdateAlign(); }

		private:
			typedef std::stack<boost::weak_ptr<LuaPlugin> > Extensions;
			Extensions extensions_;
	};
	
	class ExtensionBar : public CDialogBar
	{		
		DECLARE_DYNAMIC(ExtensionBar)
	public:
		ExtensionBar();   // standard constructor
		virtual ~ExtensionBar();
				
		void Add(LuaPlugin& plugin);
		boost::shared_ptr<ExtensionWindow> m_luaWndView;

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			
		DECLARE_MESSAGE_MAP()
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg LRESULT OnInitDialog ( WPARAM , LPARAM );													
		CSize CalcDynamicLayout(int length, DWORD dwMode);			

	private:
		CSize m_sizeFloating;
		bool docked(DWORD mode) const { return (mode & LM_VERTDOCK) || (mode & LM_HORZDOCK); }
	};
}}
