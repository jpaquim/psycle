///\file
///\brief interface file for psycle::host::CNativeGui.
#pragma once
#include "Psycle.hpp"
#include "BaseParamView.hpp"
#include "PsycleConfig.hpp"
namespace psycle {
namespace host {

		class Machine;
		class CChildView;

		/// Native Knob-based UI for psycle plugins and non-GUI VSTs
		class CNativeGui : public CBaseParamView
		{
		public:
		class Knob
		{
		public:
			Knob(){};
			virtual ~Knob(){};
			
			static void Draw(CDC& dc,CDC& knobDC, int x_knob,int y_knob,float value);
			static bool LButtonDown(UINT nFlags, int x, int y);
/*			static void MouseMove(UINT nFlags, int x, int y);
			static bool LButtonUp(UINT nFlags,int x, int y);
*/
		};

		class InfoLabel
		{
		public:
			InfoLabel(){};
			virtual ~InfoLabel(){};

			static void Draw(CDC& dc, int x, int y, int width, const char *parName,const char *parValue);
			static void DrawValue(CDC& dc, int x, int y, int width, const char *parValue);
			static void DrawHLight(CDC& dc, int x, int y, int width, const char *parName,const char *parValue);
			static void DrawHLightB(CDC& dc, int x, int y, int width, const char *parName,const char *parValue);
			static void DrawHLightValue(CDC& dc, int x, int y, int width, const char *parValue);
			static void DrawHeader(CDC& dc, int x, int y,int width, const char *parName);

			static int xoffset;
		};

		class GraphSlider
		{
		public:
			GraphSlider(){};
			virtual ~GraphSlider(){};

			static void Draw(CDC& dc,CDC& backDC,CDC& knobDC, int x, int y,float);
			static void DrawKnob(CDC& dc,CDC& knobDC, int x, int y, float value);
			static bool LButtonDown(UINT nFlags, int x, int y);
/*			static void MouseMove(UINT nFlags, int x, int y);
			static bool LButtonUp(UINT nFlags,int x, int y);
*/
			static int xoffset;
		};

		class SwitchButton
		{
		public:
			SwitchButton(){};
			virtual ~SwitchButton(){};
			
			static void Draw(CDC& dc,CDC& SwitchDC, int x, int y);
		};

		class CheckedButton
		{
		public:
			CheckedButton(){};
			virtual ~CheckedButton(){};

			static void Draw(CDC& dc,int x, int y,const char*text,bool checked);
		};

		class VuMeter
		{
		public:
			VuMeter(){};
			virtual ~VuMeter(){};

			static void Draw(CDC& dc, CDC& VuOn, CDC& VuOff, int x, int y, float value);
		};

		public:
			CNativeGui(Machine* effect, CChildView* view);
			virtual ~CNativeGui(){};
		// Operations
			virtual bool GetViewSize(CRect& rect);

			BOOL PreCreateWindow(CREATESTRUCT& cs);
		protected:
			virtual void SelectMachine(Machine* pMachine);
			inline Machine& machine(){ return *_pMachine; }
			virtual int ConvertXYtoParam(int x, int y);
			
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnSetFocus(CWnd* pOldWnd);
			afx_msg void OnPaint();
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
			afx_msg void OnMouseMove(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
			DECLARE_MESSAGE_MAP()
		
		public:
			static PsycleConfig::MachineParam* uiSetting;
		protected:
			Machine* _pMachine;
			CChildView* mainView;

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
