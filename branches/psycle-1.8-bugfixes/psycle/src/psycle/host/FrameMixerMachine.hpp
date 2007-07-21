///\file
///\brief interface file for psycle::host::CFrameMachine.
#pragma once
#include "FrameMachine.hpp"
#include "Constants.hpp"
NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

	class Mixer;

	/// mixer window.
	class CFrameMixerMachine : public CFrameMachine
	{
		enum
		{
			rowlabels=0,
			send1,
			sendmax=send1+MAX_CONNECTIONS,
			mix = sendmax,
			gain,
			pan,
			slider,
			solo,
			mute,
			dryonly,
			wetonly
		};
		enum
		{
			collabels=0,
			colmaster,
			chan1,
			chanmax=chan1+MAX_CONNECTIONS,
			return1=chanmax,
			returnmax=return1+MAX_CONNECTIONS
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
			static void DrawHLightB(CDC* dc, CFont* b_font_bold,int x, int y,const char *parName,const char *parValue);
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

		class SwitchButton
		{
		public:
			SwitchButton(){};
			virtual ~SwitchButton(){};
			
			static void Draw(CDC *dc,int x, int y, bool checked);

			static int width;
			static int height;
			static CDC imgOff;
			static CDC imgOn;
		};

		class CheckedButton
		{
		public:
			CheckedButton(){};
			virtual ~CheckedButton(){};

			static void Draw(CDC *dc,CFont* b_font_bold,int x, int y,const char*text,bool checked);

			static int width;
			static int height;
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

		// graphics
		bool updateBuffer;

		CBitmap m_sliderback;
		CBitmap m_vumeteroff;
		CBitmap m_sliderknob;
		CBitmap m_vumeteron;
		CBitmap m_switchon;
		CBitmap m_switchoff;
		CBitmap *bmpDC;

		std::string sendNames[MAX_CONNECTIONS];
		// used to know if they have changed since last paint.
		int numSends;
		// used to know if they have changed since last paint.
		int numChans;
		int _swapstart;
		int _swapend;
		bool isslider;
		bool refreshheaders;

		// Operations
	public:
		virtual void SelectMachine(Machine* pMachine);
		virtual void Generate(){};
		virtual int ConvertXYtoParam(int x, int y);
		// Overrides
		// Implementation
	protected:
		virtual ~CFrameMixerMachine();
		void Generate(CDC& dc);
		bool UpdateSendsandChans();
		int GetColumn(int x,int &xoffset);
		int GetRow(int x,int y,int &yoffset);
		int GetParamFromPos(int col,int row);
		bool GetRouteState(int ret,int send);

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
		DECLARE_MESSAGE_MAP()
	};

	NAMESPACE__END
NAMESPACE__END
