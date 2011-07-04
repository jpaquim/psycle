///\file
///\brief interface file for psycle::host::CNativeGui.
#pragma once
#include "Psycle.hpp"
#include "BaseParamView.hpp"
#include "PsycleConfig.hpp"
namespace psycle {
namespace host {

		class Machine;

		/// Native Knob-based UI for psycle plugins and non-GUI VSTs
		class CNativeView : public CBaseParamView
		{
		public:
			CNativeView(CFrameMachine* frame,Machine* effect);
			virtual ~CNativeView(){};
		// Operations
			virtual bool GetViewSize(CRect& rect);

			BOOL PreCreateWindow(CREATESTRUCT& cs);
		protected:
			virtual void SelectMachine(Machine* pMachine);
			inline Machine& machine(){ return *_pMachine; }
			virtual int ConvertXYtoParam(int x, int y);
			
			DECLARE_MESSAGE_MAP()
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnSetFocus(CWnd* pOldWnd);
			afx_msg void OnPaint();
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnLButtonDblClk(UINT nFlags, CPoint pt);
			afx_msg void OnMouseMove(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		
		public:
			static PsycleConfig::MachineParam* uiSetting;
		protected:
			Machine* _pMachine;

			int ncol;
			int numParameters;
			int parspercol;
			int colwidth;

			// Enviroment
			bool istweak;
			bool finetweak;
			float visualtweakvalue;
			bool ultrafinetweak;
			int tweakpar;
			int tweakbase;
			int minval;
			int maxval;
			int sourcepoint;
			int prevval;

		};

	}   // namespace host
}   // namespace psycle
