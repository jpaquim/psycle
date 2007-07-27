//interface definitions for CFrameAutomator.
#pragma once
#include "FrameMachine.hpp"
#include "ScrollableDlgBar.hpp"
#include <psycle/engine/internal_machines.hpp>

#include <psycle/engine/constants.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

	class Machine;

	class EnvelopeWindow
	{
	public:
		EnvelopeWindow(int x, int y, int width, int height) 
			: d_x(x), d_y(y), d_width(width), d_height(height), d_pAutomator(0) {}
		virtual ~EnvelopeWindow() {}

		virtual void SetAutomator(Automator* pAutomator) {d_pAutomator = pAutomator; }
		
		void Paint(CDC* dc);
		bool LButtonDown(unsigned int nflags, int x, int y);
		bool LButtonDblClk(unsigned int nflags, int x, int y);
		void MouseMove(unsigned int nflags, int x, int y);
		bool PointInParam(int x, int y) { return (x>d_x  &&  x<d_x+d_width  &&  y>d_y  &&  y<d_y+d_height); }

		void SetHeight(int newheight) { d_height = (newheight>1? newheight: 1); }
		void SetWidth(int newwidth)   { d_width  = (newwidth>1?  newwidth:  1); }
		int GetXPos() {return d_x;}
		int GetYPos() {return d_y;}

	private:
		Automator* d_pAutomator;
		int d_x, d_y;
		int d_width, d_height;
		//for tweaking-- iterator to the currently selected node
		//(this seems dangerous.. how easily can this be invalidated? is there a safer way?)
		std::vector<Automator::Node>::iterator d_curNode;
	};


	//////////////////////////////////////////////////////////
	///CFrameAutomator

	///todo:
	// -add trigger button(modulates currently selected machine/parameter)
	// -scrollbars, zoom buttons
	// -either make original led bitmaps, or make sure the synth1 guy is okay with us stealing them from his vsti :)
	// -use different widgets for envelope length controls (h.sliders, textboxes.. anything but knobs)
	// -draw a numbered ticks or a grid to reresent pattern rows

	class CFrameAutomator : public CFrameMachine
	{
		enum ShowText
		{
			off=0,
			top,
			right,
			bottom,
			left
		};
		class AutomatorCtrl
		{
		public:
			AutomatorCtrl(int x, int y, int minVal, int maxVal)
				: d_x(x)
				, d_y(y)
				, d_minValue(minVal)
				, d_maxValue(maxVal)
				, d_defValue(0)
				, d_bDblClkReset(true) 
				{};
			virtual ~AutomatorCtrl() {}
			
			virtual void Paint(CDC* dc,int value, const std::string& valString)=0;
			virtual bool LButtonDown(UINT nFlags, int x, int y, int &value)=0;
			virtual bool MouseMove(UINT nFlags, int x, int y, int &value)=0;
			virtual bool LButtonUp(UINT nFlags,int x, int y, int &value)=0;
			virtual bool PointInParam(int x, int y)=0;
			virtual void ResetTweakSrc(int x, int y, int value) {d_twkSrc_x=x; d_twkSrc_y=y; d_tweakBase = value;}
			virtual void Visible(bool isVis) {d_bVisible=isVis;}

			int d_x;
			int d_y;
			int d_minValue;
			int d_maxValue;
			int d_defValue;
			int d_twkSrc_x;
			int d_twkSrc_y;
			int d_tweakBase;

			bool d_bDblClkReset;
			bool d_bVisible;
			bool d_bTweakable;
			std::string d_lblString;
			ShowText d_showValue;
			ShowText d_showLabel;

		};

		class Knob : public AutomatorCtrl
		{
		public:
			Knob(int x, int y, int minVal, int maxVal, const std::string& label);
			virtual ~Knob() {}
			
			virtual void Paint(CDC* dc,int value, const std::string& valString);
			virtual bool LButtonDown(UINT nFlags, int x, int y, int &value);
			virtual bool MouseMove(UINT nFlags, int x, int y, int &value);
			virtual bool LButtonUp(UINT nFlags,int x, int y, int &value);
			virtual bool PointInParam(int x, int y);
			
			static int s_width;
			static int s_height;
			static int s_numFrames;
			static CDC s_knobDC;

			COLORREF d_lblTextColor;
			COLORREF d_lblBkColor;
			COLORREF d_valTextColor;
			COLORREF d_valBkColor;

		};

		class ComboBox : public AutomatorCtrl
		{
		public:
			ComboBox(int x, int y, int minVal, int maxVal, int length,  const std::string& label);
			virtual ~ComboBox() {}
			
			virtual void Paint(CDC* dc,int value, const std::string& valString);
			virtual bool LButtonDown(UINT nFlags, int x, int y, int &value);
			virtual bool MouseMove(UINT nFlags, int x, int y, int &value);
			virtual bool LButtonUp(UINT nFlags,int x, int y, int &value);
			virtual bool PointInParam(int x, int y);
			
			static int s_width;
			static int s_height;
			static CDC s_ComboBoxDC;

			int d_length;

			COLORREF d_lblTextColor;
			COLORREF d_lblBkColor;
			COLORREF d_valTextColor;

		};

		class Switch : public AutomatorCtrl
		{
		public:
			Switch(int x, int y, int minVal, int maxVal, const std::string& label, const std::vector<std::string>& valStrings);
			virtual ~Switch() {}

			virtual void Paint(CDC* dc, int value, const std::string& valString);
			virtual bool LButtonDown(UINT nFlags, int x, int y, int &value);
			virtual bool MouseMove(UINT nFlags, int x, int y, int &value);
			virtual bool LButtonUp(UINT nFlags,int x, int y, int &value);
			virtual bool PointInParam(int x, int y);

		
			static int s_width;
			static int s_height;
			static int s_spacer;
			static CDC s_SwitchDC;

			std::vector<std::string> d_valStrings;
			int d_count;

			COLORREF d_lblTextColor;
			COLORREF d_lblBkColor;
			COLORREF d_valTextColor;
			COLORREF d_valBkColor;
		};



		int const static WIN_CX = 575;
		int const static WIN_CY = 245;

		DECLARE_DYNCREATE(CFrameAutomator)
	protected:
		CFrameAutomator(); // protected constructor used by dynamic creation
		// Attributes
	public:
		CFrameAutomator(int dum);
	private:
		Automator* d_pAutomator;

		// graphics

		CBitmap m_combobox;
		CBitmap m_switch;
		CBitmap *bmpDC;

		std::vector<AutomatorCtrl*> d_pParams;

		EnvelopeWindow *envWind;

		CScrollableDlgBar zoombar;

		//used to detect shift or control presses during a tweak
		int d_shiftAndCtrlState;

		// Operations
	public:
		virtual void SelectMachine(Machine* pMachine);
		virtual void Generate(){}
		virtual int ConvertXYtoParam(int x, int y);

		// Implementation
	protected:
		virtual ~CFrameAutomator();
		// Generated message map functions
		//{{AFX_MSG(CFrameAutomator)
		afx_msg void OnPaint();
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnDestroy();
		afx_msg void OnSetFocus(CWnd* pOldWnd);
		afx_msg void OnTimer(UINT nIDEvent);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnSize(UINT nType, int cx, int cy);

		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};

	//{{AFX_INSERT_LOCATION}}
	// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
