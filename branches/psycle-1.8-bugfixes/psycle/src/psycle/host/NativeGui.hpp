///\file
///\brief interface file for psycle::host::CNativeGui.
#pragma once
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)

#define K_XSIZE				28
#define K_YSIZE				28
#define K_NUMFRAMES			63
#define W_ROWWIDTH			150

		class Machine;

		class NativeGUISettings
		{
		public:
			NativeGUISettings();
			virtual ~NativeGUISettings();
			void LoadMachineDial();
		
			COLORREF topColor;
			COLORREF bottomColor;
			COLORREF hTopColor;
			COLORREF hBottomColor;
			COLORREF titleColor;
			COLORREF fontTopColor;
			COLORREF fontBottomColor;
			COLORREF fonthTopColor;
			COLORREF fonthBottomColor;
			COLORREF fonttitleColor;
			CFont	b_font;
			CFont	b_font_bold;
			CRect	deskrect;
			CBitmap dial;
			HBITMAP hbmMachineDial;
			int dialwidth;
			int dialheight;
			int dialframes;
		};

		class CBaseGui : public CWnd
		{
		public:
			CBaseGui(){};
			virtual void Open(){};
			virtual bool GetViewSize(CRect& rect){ return false; }
			virtual void WindowIdle() { Invalidate(false); }
		protected:
			virtual void* WindowPtr(){ return GetSafeHwnd(); }
		};

		/// Native Knob-based UI for psycle plugins.
		class CNativeGui : public CBaseGui
		{
		public:
			CNativeGui(Machine* effect);
			virtual ~CNativeGui(){};
		// Operations
			virtual bool GetViewSize(CRect& rect);
			static NativeGUISettings& uiSetting()
			{ 
				static NativeGUISettings setting;
				return setting;
			}

		protected:
			virtual void SelectMachine(Machine* pMachine);
			inline Machine& machine(){ return *_pMachine; }
			virtual int ConvertXYtoParam(int x, int y);

			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnDestroy();
			afx_msg void OnSetFocus(CWnd* pOldWnd);
			afx_msg void OnTimer(UINT nIDEvent);
			afx_msg void OnPaint();
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
			afx_msg void OnMouseMove(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
			afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
			afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
			DECLARE_MESSAGE_MAP()

		protected:
			Machine* _pMachine;

			int ncol;
			int numParameters;
			int parspercol;

			// Enviroment
			bool istweak;
			bool finetweak;
			bool ultrafinetweak;
			int tweakpar;
			int tweakbase;
			int minval;
			int maxval;
			int sourcepoint;
			int prevval;

		};
#undef BASEWINDOWCLASS
	NAMESPACE__END
NAMESPACE__END
