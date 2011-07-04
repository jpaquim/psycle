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
		class CChildView;

		namespace vst
		{
			class plugin;
		}

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
		class CVstEffectWnd : public CFrameMachine, public seib::vst::CEffectWnd
		{
			DECLARE_DYNAMIC(CVstEffectWnd)
		public: 
			CVstEffectWnd(vst::plugin* effect, CChildView* wndView_, CFrameMachine** windowVar_);
			virtual ~CVstEffectWnd(){};
		protected:
			CVstEffectWnd(){}; // protected constructor used by dynamic creation

		public:
			virtual void CloseEditorWnd() { OnClose(); }
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
			std::list<HWND> secwinlist;

		// Implementation
		public:
			DECLARE_MESSAGE_MAP()
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnClose();
			afx_msg void OnProgramsOpenpreset();
			afx_msg void OnProgramsSavepreset();
		};

	}   // namespace
}   // namespace

