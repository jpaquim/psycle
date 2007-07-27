///\file
///\brief interface file for psycle::host::CFrameMachine.
#pragma once
#include "FrameMachine.hpp"
#include <psycle/engine/constants.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

	class Machine;	
	class Mixer;

	/// mixer window.
	class CFrameMixerMachine : public CFrameMachine
	{
		enum
		{
			rowlabels=0,
			send1,
			send2,
			send3,
			send4,
			send5,
			send6,
			send7,
			send8,
			send9,
			send10,
			send11,
			send12,
			dry,
			//some other buttons.
			slider,
		};
		class Knob
		{
		public:
			Knob(){};
			virtual ~Knob(){};
			
			static void Draw(CDC* dc,int x_knob,int y_knob,float value);
			static bool LButtonDown(UINT nFlags, int x, int y);
			static void MouseMove(UINT nFlags, int x, int y);
			static bool LButtonUp(UINT nFlags,int x, int y);
			
			static int width;
			static int height;
			static int numFrames;
			static CDC knobDC;
		};

		class InfoLabel
		{
		public:
			InfoLabel(){};
			virtual ~InfoLabel(){};

			static void Draw(CDC *dc, int x, int y,const char *parName,const char *parValue);
			static void DrawValue(CDC *dc, int x, int y,const char *parValue);
			static void DrawHLight(CDC *dc,CFont *font_bold, int x, int y,const char *parName,const char *parValue);
			static void DrawHLightValue(CDC *dc, int x, int y,const char *parValue);
//			static void DrawHeader(CDC *dc, int x, int y,const char *parName, const char *parValue);

			static int xoffset;
			static int width;
			static int height;
		};

/*		class HLightInfoLabel : public InfoLabel
		{
		public:
			HLightInfoLabel(){};
			virtual ~HLightInfoLabel(){};

			virtual void Draw(CDC *dc, int x, int y,const char *parName, const char *parValue);
		};
		class HeaderInfoLabel : public InfoLabel
		{
		public:
			HeaderInfoLabel(){};
			virtual ~HeaderInfoLabel(){};

			virtual void Draw(CDC *dc, int x, int y,const char *parName, const char *parValue);
		};

*/		class GraphSlider
		{
		public:
			GraphSlider(){};
			virtual ~GraphSlider(){};

			static void Draw(CDC *dc,int x, int y,float);
			static void DrawKnob(CDC *dc,int x, int y, float value);
			static bool LButtonDown(UINT nFlags, int x, int y);
			static void MouseMove(UINT nFlags, int x, int y);
			static bool LButtonUp(UINT nFlags,int x, int y);

			static int width;
			static int height;
			static int knobheight;
			static int knobwidth;
			static int xoffset;
			static CDC backDC;
			static CDC knobDC;
		};
		class VuMeter
		{
		public:
			VuMeter(){};
			virtual ~VuMeter(){};

			static void Draw(CDC *dc,int x, int y, float value);

			static int width;
			static int height;
			static CDC VuOff;
			static CDC VuOn;
		};

		DECLARE_DYNCREATE(CFrameMixerMachine)
	protected:
		CFrameMixerMachine(); // protected constructor used by dynamic creation
		// Attributes
	public:
		CFrameMixerMachine(int dum);
	private:
		Mixer* _pMixer;
		int numChans;
		int numSends;


		// graphics
		bool updateBuffer;

		CBitmap m_sliderback;
		CBitmap m_vumeteroff;
		CBitmap m_sliderknob;
		CBitmap m_vumeteron;
		CBitmap *bmpDC;

		std::string sendNames[MAX_CONNECTIONS];

		// Operations
	public:
		virtual void SelectMachine(Machine* pMachine);
		virtual void Generate(){};
		virtual int ConvertXYtoParam(int x, int y);
		// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CFrameMixerMachine)
		//}}AFX_VIRTUAL
		// Implementation
	protected:
		void Generate(CDC& dc);
		bool UpdateSendsandChans();
		int GetColumn(int x,int &xoffset);
		int GetRow(int y,int &yoffset);
		int GetParamFromPos(int col,int row);
		virtual ~CFrameMixerMachine();
		// Generated message map functions
		//{{AFX_MSG(CFrameMixerMachine)
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
