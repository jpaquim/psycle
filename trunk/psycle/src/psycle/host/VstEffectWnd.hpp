///\file
///\brief interface file for psycle::host::CVstEditorDlg.
#pragma once
#include <seib-vsthost/EffectWnd.hpp>
#include "NativeGui.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		class CVstParamList;

		using namespace seib::vst;
		namespace vst
		{
			class plugin;
		}

		class CVstGui : public CBaseGui
		{
		public:
			CVstGui(vst::plugin*effect);
			void Open();
			bool GetViewSize(CRect& rect);
			void WindowIdle();
		protected:
			vst::plugin* pEffect;
		};

		/// vst editor window.
		class CVstEffectWnd : public CFrameWnd, public CEffectWnd
		{
			DECLARE_DYNAMIC(CVstEffectWnd)
		public: 
			CVstEffectWnd(vst::plugin* effect);
			virtual ~CVstEffectWnd(){};
		protected:
			CVstEffectWnd(){}; // protected constructor used by dynamic creation

		// Attributes
		public:
			inline vst::plugin& machine(){ return *_machine; }
			bool *_pActive;
		protected:
			vst::plugin* _machine;

			//CChildView * wndView;
		// Overrides
		public:
			void PostOpenWnd();
			virtual void CloseEditorWnd() { OnClose(); }
			virtual void GetWindowSize(CRect &rcFrame, CRect &rcClient, ERect *pRect = NULL);
			virtual void ResizeWindow(int width, int height);
			virtual void ResizeWindow(ERect* pRect);
			virtual void RefreshUI();
			virtual bool BeginAutomating(long index){ return false; }
			virtual bool SetParameterAutomated(long index, float value);
			virtual bool EndAutomating(long index) { return false; }
			virtual bool OpenFileSelector (VstFileSelect *ptr);
			virtual bool CloseFileSelector (VstFileSelect *ptr);
			virtual void* OpenSecondaryWnd(VstWindow& window);
			virtual bool CloseSecondaryWnd(VstWindow& window);
		protected:
			virtual void UpdateTitle(){ SetWindowText(sTitle.c_str()); }
			virtual CBaseGui* CreateView();
			void FillProgramCombobox();
			void FillPopup(CMenu* pPopupMenu);
			CBaseGui* pView;
			CVstParamList* pParamGui;
			CToolBar toolBar;
			CComboBox comboBank;
			CComboBox comboProgram;
			std::list<HWND> secwinlist;

		// Implementation
		public:
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnClose();
//			afx_msg void OnDestroy();
			afx_msg void OnTimer(UINT nIDEvent);
			afx_msg void OnSetFocus(CWnd* pOldWnd);
			afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
			afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
			afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
//			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnOperationsEnabled();
			afx_msg void OnUpdateOperationsEnabled(CCmdUI *pCmdUI);
			afx_msg void OnProgramsOpenpreset();
			afx_msg void OnProgramsSavepreset();
			afx_msg void OnProgramsRandomizeprogram();
			afx_msg void OnViewsParameterlist();
			afx_msg void OnUpdateViewsParameterlist(CCmdUI *pCmdUI);
			afx_msg void OnViewsBankmanager();
			afx_msg void OnUpdateViewsBankmanager(CCmdUI *pCmdUI);
			afx_msg void OnViewsMidichannels();
			afx_msg void OnUpdateViewsMidichannels(CCmdUI *pCmdUI);
			afx_msg void OnAboutAboutvst();
			afx_msg void OnSelchangeProgram();
			afx_msg void OnCloseupProgram();
			afx_msg void OnSetProgram(UINT nID);
			afx_msg void OnProgramLess();
			afx_msg void OnUpdateProgramLess(CCmdUI *pCmdUI);
			afx_msg void OnProgramMore();
			afx_msg void OnUpdateProgramMore(CCmdUI *pCmdUI);
			afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
			DECLARE_MESSAGE_MAP()
			afx_msg void OnViewsShowtoolbar();
			afx_msg void OnUpdateViewsShowtoolbar(CCmdUI *pCmdUI);
		};

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

