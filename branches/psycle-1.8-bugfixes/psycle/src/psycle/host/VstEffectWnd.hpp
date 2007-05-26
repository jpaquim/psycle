///\file
///\brief interface file for psycle::host::CVstEditorDlg.
#pragma once
#include <seib-vsthost/EffectWnd.hpp>
#include "NativeGui.hpp"

NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
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
		public:
			DECLARE_DYNCREATE(CVstEffectWnd)
			CVstEffectWnd(vst::plugin* effect);
			virtual ~CVstEffectWnd(){};
		protected:
			CVstEffectWnd(){}; // protected constructor used by dynamic creation

		// Attributes
		public:
			inline vst::plugin& machine(){ return *_machine; }
		protected:
			vst::plugin* _machine;

			//CParamEditWnd * pParamGui;
			//CChildView * wndView;
		// Overrides
		public:
			void PostOpenWnd();
			virtual void CloseEditorWnd() { OnClose(); }
			virtual void GetWindowSize(CRect &rcFrame, CRect &rcClient, ERect *pRect = NULL);
			virtual void ResizeWindow(int width, int height);
			virtual void ResizeWindow(ERect* pRect);
			virtual void RefreshUI(){};
			virtual bool BeginAutomating(long index){ return false; }
			virtual bool SetParameterAutomated(long index, float value) { return false; }
			virtual bool EndAutomating(long index) { return false; }
			virtual bool OpenFileSelector (VstFileSelect *ptr);
			virtual bool CloseFileSelector (VstFileSelect *ptr);
			virtual void* OpenSecondaryWnd(VstWindow& window);
			virtual bool CloseSecondaryWnd(VstWindow& window);
		protected:
			virtual void UpdateTitle(){ SetWindowText(sTitle.c_str()); };
			virtual CBaseGui* CreateView();
			CBaseGui* pView;

		// Implementation
		protected:
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnClose();
//			afx_msg void OnDestroy();
			afx_msg void OnTimer(UINT nIDEvent);
			afx_msg void OnSetFocus(CWnd* pOldWnd);
			afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
			afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
//			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			/*
			afx_msg void OnLoadPreset();
			afx_msg void OnSavePreset();
			afx_msg void OnSavePresetAs();
			afx_msg void OnParametersListDlg();
			afx_msg void OnParametersRandomparameters();
			afx_msg void OnParametersShowpreset();
			*/
			DECLARE_MESSAGE_MAP()
		};
	NAMESPACE__END
NAMESPACE__END

