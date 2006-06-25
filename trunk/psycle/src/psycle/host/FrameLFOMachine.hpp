//interface definitions for CFrameLFOMachine.
#pragma once
#include "FrameMachine.hpp"
#include <psycle/engine/constants.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

	class Machine;	
	class LFO;

	class CFrameLFOMachine : public CFrameMachine
	{
		enum ShowText
		{
			off=0,
			top,
			right,
			bottom,
			left
		};
		class LFOControl
		{
		public:
			LFOControl(int x, int y, int minVal, int maxVal)
				: d_x(x)
				, d_y(y)
				, d_minValue(minVal)
				, d_maxValue(maxVal)
				, d_defValue(0)
				, d_bDblClkReset(true) 
				{};
			virtual ~LFOControl() {}
			
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

		class Knob : public LFOControl
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

		class ComboBox : public LFOControl
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

		int const static WIN_CX = 400;
		int const static WIN_CY = 245;

		DECLARE_DYNCREATE(CFrameLFOMachine)
	protected:
		CFrameLFOMachine(); // protected constructor used by dynamic creation
		// Attributes
	public:
		CFrameLFOMachine(int dum);
	private:
		LFO* d_pLFO;

		// graphics

		CBitmap m_combobox;
		CBitmap *bmpDC;

		std::vector<LFOControl*> d_pParams;
		ComboBox *d_pView;	//instanciated separately-- does not correspond to a parameter in the actual lfo

		//determines which outputs are visible
		int d_view;

		//used to detect shift or control presses during a tweak
		int d_shiftAndCtrlState;

		// Operations
	public:
		virtual void SelectMachine(Machine* pMachine);
		virtual void SwitchView();
		virtual void Generate(){}
		virtual int ConvertXYtoParam(int x, int y);

		// Implementation
	protected:
		void UpdateParamRanges();
		virtual ~CFrameLFOMachine();
		// Generated message map functions
		//{{AFX_MSG(CFrameLFOMachine)
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
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};

	//{{AFX_INSERT_LOCATION}}
	// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
