///\file
///\brief interface file for psycle::host::CFrameMachine.
#pragma once
#include "FrameMachine.hpp"
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)

	class Machine;	
	class Mixer;

	/// mixer window.
	class CFrameMixerMachine : public CFrameMachine
	{
		class Knob
		{
		public:
			Knob();
			virtual ~Knob();
			
			void Draw(CDC* dc,int x_knob,int y_knob,float value);
			void OnLButtonDown(UINT nFlags, CPoint point);
			void OnMouseMove(UINT nFlags, CPoint point);
			void OnLButtonUp(UINT nFlags, CPoint point);
			
			static int width;
			static int height;
			static int numFrames;
			static CDC* pKnobDC;
		};

		class InfoLabel
		{
		public:
			InfoLabel();
			virtual ~InfoLabel();

			virtual void Draw(CDC *dc, int x, int y,char *parName, char *parValue);

			static int xoffset;
			static int width;
			static int height;
			static CFont font;
			static CFont font_bold;
		};

		class HLightInfoLabel : public InfoLabel
		{
		public:
			HLightInfoLabel();
			virtual ~HLightInfoLabel();

			virtual void Draw(CDC *dc, int x, int y,char *parName, char *parValue);
		};
		class HeaderInfoLabel : public InfoLabel
		{
		public:
			HeaderInfoLabel();
			virtual ~HeaderInfoLabel();

			virtual void Draw(CDC *dc, int x, int y,char *parName, char *parValue);
		};

		class GraphSlider
		{
		public:
			GraphSlider();
			virtual ~GraphSlider();

			virtual void Draw(CDC *dc,int x, int y, float value);

			static int width;
			static int height;
			static int knobheight;
			static int knobwidth;
			static int xoffset;
			static CDC* pBackDC;
			static CDC* pKnobDC;
		};

		DECLARE_DYNCREATE(CFrameMixerMachine)
	protected:
		CFrameMixerMachine(); // protected constructor used by dynamic creation
		// Attributes
	public:
		CFrameMixerMachine(int dum){MachineIndex = dum;};
	private:
		Mixer* _pMixer;
		CBitmap m_sliderback;
		CBitmap m_sliderknob;

		Knob m_knob;
		InfoLabel m_infolabel;
		GraphSlider m_slider;
		int numSends;
		int numChannels;

		// Operations
	public:
		void SelectMachine(Machine* pMachine);
		void Generate();
		// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CFrameMixerMachine)
		//}}AFX_VIRTUAL
		// Implementation
	protected:
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
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnSetFocus(CWnd* pOldWnd);
		afx_msg void OnTimer(UINT nIDEvent);
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};

	//{{AFX_INSERT_LOCATION}}
	// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	NAMESPACE__END
NAMESPACE__END
