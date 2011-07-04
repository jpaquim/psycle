///\file
///\brief interface file for psycle::host::CVstEditorDlg.
#pragma once
#include "Psycle.hpp"

#include "FrameMachine.hpp"
#include "BaseParamView.hpp"
#include <Seib-Vsthost/EffectWnd.hpp>

#include <list>

namespace psycle {
	namespace host {
		namespace vst
		{
			class plugin;
		}
		using namespace seib::vst;

		class CVstParamList;

		class CVstGui : public CBaseParamView
		{
		public:
			CVstGui(CFrameMachine* frame,vst::plugin*effect);
			void Open();
			bool GetViewSize(CRect& rect);
			void WindowIdle();
		protected:
			vst::plugin* pEffect;
		};

		/// vst editor window.
		class CVstEffectWnd : public CFrameMachine, public CEffectWnd
		{
			DECLARE_DYNAMIC(CVstEffectWnd)
		public: 
			CVstEffectWnd(vst::plugin* effect);
			virtual ~CVstEffectWnd(){};
		protected:
			CVstEffectWnd(){}; // protected constructor used by dynamic creation

		public:
			virtual void CloseEditorWnd() { OnClose(); }
			virtual void GetWindowSize(CRect &rcFrame, CRect &rcClient, CRect *pRect);
			virtual void ResizeWindow(int width, int height);
			virtual void ResizeWindow(CRect* pRect);
			virtual void RefreshUI();
			virtual bool BeginAutomating(long index){ return false; }
			virtual bool SetParameterAutomated(long index, float value);
			virtual bool EndAutomating(long index) { return false; }
			virtual bool OpenFileSelector (VstFileSelect *ptr);
			virtual bool CloseFileSelector (VstFileSelect *ptr);
			virtual void* OpenSecondaryWnd(VstWindow& window);
			virtual bool CloseSecondaryWnd(VstWindow& window);
		protected:
			inline vst::plugin& vstmachine(){ return *reinterpret_cast<vst::plugin*>(_machine); }
			virtual CBaseParamView* CreateView();
			virtual void UpdateTitle(){ SetWindowText(sTitle.c_str()); }
			void FillProgramCombobox();
			void FillPopup(CMenu* pPopupMenu);
			CVstParamList* pParamGui;
			CToolBar toolBar;
			CComboBox comboBank;
			CComboBox comboProgram;
			std::list<HWND> secwinlist;

		// Implementation
		public:
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnClose();
			afx_msg void OnProgramsOpenpreset();
			afx_msg void OnProgramsSavepreset();
			afx_msg void OnViewsParameterlist();
			afx_msg void OnUpdateViewsParameterlist(CCmdUI *pCmdUI);
			afx_msg void OnViewsMidichannels();
			afx_msg void OnUpdateViewsMidichannels(CCmdUI *pCmdUI);
			afx_msg void OnSelchangeProgram();
			afx_msg void OnCloseupProgram();
			afx_msg void OnSetProgram(UINT nID);
			afx_msg void OnProgramLess();
			afx_msg void OnUpdateProgramLess(CCmdUI *pCmdUI);
			afx_msg void OnProgramMore();
			afx_msg void OnUpdateProgramMore(CCmdUI *pCmdUI);
			afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
			afx_msg void OnViewsShowtoolbar();
			afx_msg void OnUpdateViewsShowtoolbar(CCmdUI *pCmdUI);
			DECLARE_MESSAGE_MAP()
		};

	}   // namespace
}   // namespace

