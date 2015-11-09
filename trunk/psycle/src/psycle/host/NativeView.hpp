///\file
///\brief interface file for psycle::host::CNativeGui.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "BaseParamView.hpp"
#include "PsycleConfig.hpp"
#include "Canvas.hpp"

namespace psycle {
namespace host {

		class Machine;

    class CanvasParamView : public CBaseParamView, public ui::canvas::BaseView {
     public:
       CanvasParamView(CFrameMachine* frame, Machine* effect) :
          CBaseParamView(frame), ui::canvas::BaseView(this) {}
       BOOL PreCreateWindow(CREATESTRUCT& cs);
     protected:
       DECLARE_MESSAGE_MAP()
       int OnCreate(LPCREATESTRUCT lpCreateStruct);
       void OnDestroy();
       void OnPaint() { Draw(); }
       void OnLButtonDown(UINT nFlags, CPoint pt) { 
         DelegateEvent(ui::canvas::Event::BUTTON_PRESS, 1, nFlags, pt);
       }
       void OnRButtonDown(UINT nFlags, CPoint pt) {
         DelegateEvent(ui::canvas::Event::BUTTON_PRESS, 2, nFlags, pt);
       }
		   void OnLButtonDblClk(UINT nFlags, CPoint pt) {
         DelegateEvent(ui::canvas::Event::BUTTON_2PRESS, 1, nFlags, pt);
       }
		   void OnMouseMove(UINT nFlags, CPoint pt) {
         DelegateEvent(ui::canvas::Event::MOTION_NOTIFY, 0, nFlags, pt);
       }
		   void OnLButtonUp(UINT nFlags, CPoint pt) {
         DelegateEvent(ui::canvas::Event::BUTTON_RELEASE, 1, nFlags, pt);
       }
		   void OnRButtonUp(UINT nFlags, CPoint pt) {
         DelegateEvent(ui::canvas::Event::BUTTON_RELEASE, 2, nFlags, pt);
       }
       void OnReload(Machine* mac);
    };

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
      afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnLButtonDblClk(UINT nFlags, CPoint pt);
			afx_msg void OnMouseMove(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		
		public:
			static PsycleConfig::MachineParam* uiSetting;
		protected:      
      void ComputeLeds(int tweakpar, std::vector<int>& on, int &maxf, int &koffset, int &amp_v, int &x_knob, int &y_knob);
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
			int sourcex;
			bool positioning;
			bool allowmove;
			int prevval;
			int painttimer;
			std::vector<int> blink;      
		};

	}   // namespace host
}   // namespace psycle
