//interface definitions for CFrameLFOMachine.
#pragma once
#include "FrameMachine.hpp"
#include <psycle/host/engine/constants.hpp>
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
			LFOControl(int x, int y, int minVal, int maxVal) : d_x(x), d_y(y), d_minValue(minVal), d_maxValue(maxVal) {};
			virtual ~LFOControl() {};
			
			virtual void Paint(CDC* dc,int value, char* valString)=0;
			virtual bool LButtonDown(UINT nFlags, int x, int y, int &value)=0;
			virtual bool MouseMove(UINT nFlags, int x, int y, int &value)=0;
			virtual bool LButtonUp(UINT nFlags,int x, int y, int &value)=0;
			virtual bool PointInParam(int x, int y)=0;

			int d_x;
			int d_y;
			int d_minValue;
			int d_maxValue;
			int d_twkSrc_x;
			int d_twkSrc_y;
			int d_tweakBase;

			char d_lblString[128];
			ShowText d_showValue;
			ShowText d_showLabel;

		};

		class Knob : public LFOControl
		{
		public:
			Knob(int x, int y, int minVal, int maxVal, char* label);
			virtual ~Knob() {};
			
			virtual void Paint(CDC* dc,int value, char* valString);
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
			ComboBox(int x, int y, int minVal, int maxVal, int length,  char* label);
			virtual ~ComboBox() {};
			
			virtual void Paint(CDC* dc,int value, char* valString);
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

		enum
		{
			prm_waveform=0,
			prm_pwidth,
			prm_speed,
			prm_macout0,	prm_macout1,	prm_macout2,	prm_macout3,
			prm_prmout0,	prm_prmout1,	prm_prmout2,	prm_prmout3,
			prm_level0,		prm_level1,		prm_level2,		prm_level3,	
			prm_phase0,		prm_phase1,		prm_phase2,		prm_phase3,	
			prm_lfopos,
			num_params
		};

		int const static WIN_CX = 360;
		int const static WIN_CY = 445;

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
//		CBitmap m_sliderback;
//		CBitmap m_sliderknob;
		CBitmap *bmpDC;

		LFOControl *d_pParams[num_params];

		std::vector<std::string> d_machNames;
		std::map<int, std::vector<std::string> > d_paramNames;

		// Operations
	public:
		virtual void SelectMachine(Machine* pMachine);
		virtual void Generate(){};
		virtual int ConvertXYtoParam(int x, int y);

		// Implementation
	protected:
		void UpdateNames();
		virtual ~CFrameLFOMachine();
		// Generated message map functions
		//{{AFX_MSG(CFrameLFOMachine)
		afx_msg void OnPaint();
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
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
