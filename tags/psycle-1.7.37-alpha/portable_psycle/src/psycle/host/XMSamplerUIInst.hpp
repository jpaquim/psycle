#pragma once

// XMSamplerUIInst
NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)
class XMSampler;
	/// XMInstrument ***** [bohan] iso-(10)646 encoding only please! *****
class XMSamplerUIInst : public CPropertyPage
	{

#if 0

		/** EnvelopeEditor Class 
		 * Envelope Graph ***** [bohan] iso-(10)646 encoding only please! *****Edit***** [bohan] iso-(10)646 encoding only please! ***** */
		class EnvelopeEditor : public CScrollView
		{
		public:
			// constant
			static const int MARGIN_RIGHT = 100 /* pixel */;
			static const int POINT_SIZE = 6 /* pixel */;///< Envelope Point Ç***** [bohan] iso-(10)646 encoding only please! *****
			static const int HITTEST_NOT_FOUND = -1;///< HitTest
			// Exception
			
	
			/// Window Class Definition
			DECLARE_WND_SUPERCLASS(_T("EnvelopeEditor"),GetWndClassName())
			
			/// Constructor
			EnvelopeEditor();
		
			/// Initialize ***** [bohan] iso-(10)646 encoding only please! *****
			void Initialize(XMSampler * const pSampler,XMInstrument::Envelope * const pEnvelope);

			/// Envelope***** [bohan] iso-(10)646 encoding only please! *****
			void EditEnvelope(XMInstrument::Envelope * const pEnvelope);
			
			/// ***** [bohan] iso-(10)646 encoding only please! *****
			void Color(const BYTE a,const BYTE r ,const BYTE g,const BYTE b)
			{
				m_Color.SetValue(::Gdiplus::Color::MakeARGB(a,r,g,b));
			};

			// size operations
			void SetScrollSize(int cx, int cy, BOOL bRedraw = TRUE)
			{
				// reset current range to prevent scroll bar problems
				SCROLLINFO si = { sizeof(si), SIF_RANGE, 0, 0 };
				SetScrollInfo(SB_HORZ, &si, FALSE);
				SetScrollInfo(SB_VERT, &si, FALSE);
	
				m_sizeAll.cx = cx;
				m_sizeAll.cy = cy;

				m_ptOffset.x = 0;
				m_ptOffset.y = 0;

				si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL;
				si.nMin = 0;


				si.nMax = m_sizeAll.cy - 1;
				si.nPage = m_sizeClient.cy;
				si.nPos = m_ptOffset.y;
				SetScrollInfo(SB_VERT, &si, bRedraw);

				si.nMax = m_sizeAll.cx - 1;
				si.nPage = m_sizeClient.cx;
				si.nPos = m_ptOffset.x;
				SetScrollInfo(SB_HORZ, &si, bRedraw);

				SetScrollLine(0, 0);
				SetScrollPage(0, 0);

				Invalidate();
			};

			void DoPaint(CDCHandle dc);///< WM_PAINT Handler
			void OnLButtonDown(const UINT vKey, WTL::CPoint position);
			void OnLButtonUp(const UINT vKey, WTL::CPoint position);
			void OnMouseMove(const UINT vKey, WTL::CPoint position);
			void OnRButtonDown(const UINT vKey, WTL::CPoint position);
		    void OnAddPoint(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
		    void OnDelPoint(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
		    void OnSetSustain(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
		    void OnRemoveSustain(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
		    void OnSetLoopStart(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
		    void OnSetLoopEnd(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
		    void OnRemoveLoop(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
			
			static LRESULT CALLBACK DummyWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		private:
			typedef boost::optional<int> EnvelopePointIndex;

			/** ***** [bohan] iso-(10)646 encoding only please! ***** */
			const EnvelopePointIndex GetEnvelopePointIndexAtPoint(const int x,const int y)
			{
				const int _points = m_pEnvelope->NumOfPoints();
				for(int i = 0;i < _points ;i++)
				{
					CPoint _pt_env;
					_pt_env.y = (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(i)));
					_pt_env.x = (int)(m_Zoom * (float)m_pEnvelope->Point(i));
					if(((_pt_env.x - POINT_SIZE / 2) <= x) & ((_pt_env.x + POINT_SIZE / 2) >= x) &
						((_pt_env.y - POINT_SIZE / 2) <= y) & ((_pt_env.y + POINT_SIZE / 2) >= y))
					{
						return EnvelopePointIndex(i);
					}
				}

				return EnvelopePointIndex();// ***** [bohan] iso-(10)646 encoding only please! *****
			};

			/** Scroll Size ***** [bohan] iso-(10)646 encoding only please! ***** */
			void AdjustScrollRect(const int maxPoint);
			//const WTL::CPoint TickToPosition(const int index);///< convert Tick To Screen Position

			XMInstrument::Envelope* m_pEnvelope;///< Envelope Class Pointer
			XMInstrument::Envelope m_EnvelopeEditing;///< Envelope Data backup
			XMSampler* m_pXMSampler;///< XMSampler Pointer
			bool m_bInitialized;///< Initialize Flag
			float m_Zoom;///< Zoom
			int m_CurrentScrollWidth;///< ***** [bohan] iso-(10)646 encoding only please! *****
			int m_CurrentScrollHeight;///< ***** [bohan] iso-(10)646 encoding only please! *****
			
			bool m_bPointEditing;///< ***** [bohan] iso-(10)646 encoding only please! ***** EnvelopePoint ***** [bohan] iso-(10)646 encoding only please! *****
			int m_EditPoint;///< ***** [bohan] iso-(10)646 encoding only please! ***** Envelope Point Index
			int m_EditPointOrig;///< ***** [bohan] iso-(10)646 encoding only please! ***** Envelope Point Index
			int m_EditPointX;///< ***** [bohan] iso-(10)646 encoding only please! ***** Envelope PointÇÃXç¿ïW
			int m_EditPointY;///< ***** [bohan] iso-(10)646 encoding only please! ***** Envelope PointÇÃYç¿ïW
			Gdiplus::Color m_Color;///< Line Color  
			
			//CComPtr<ID3DXLine> m_pLine;
			//CComPtr<IDirect3DTexture9>	m_pTexture;
			//D3DXVECTOR2 m_Vect[16];
		public:
			// Message Map
			BEGIN_MSG_MAP_EX(EnvelopeEditor)
				//MSG_WM_PAINT(OnPaint)
				MSG_WM_LBUTTONDOWN(OnLButtonDown)
				MSG_WM_LBUTTONUP(OnLButtonUp)
		        MSG_WM_RBUTTONDOWN(OnRButtonDown)
				MSG_WM_MOUSEMOVE(OnMouseMove)
		        COMMAND_ID_HANDLER_EX(ID_ADD_POINT, OnAddPoint)
				COMMAND_ID_HANDLER_EX(ID_DEL_POINT, OnDelPoint)
				COMMAND_ID_HANDLER_EX(ID_SET_SUSTAIN, OnSetSustain)
				COMMAND_ID_HANDLER_EX(ID_REMOVE_SUSTAIN, OnRemoveSustain)
				COMMAND_ID_HANDLER_EX(ID_SET_LOOPSTART, OnSetLoopStart)
				COMMAND_ID_HANDLER_EX(ID_SET_LOOPEND, OnSetLoopEnd)
				COMMAND_ID_HANDLER_EX(ID_REMOVE_LOOP, OnRemoveLoop)
				CHAIN_MSG_MAP(CScrollWindowImpl<XMSamplerUIInst::EnvelopeEditor>)
				CHAIN_MSG_MAP_ALT(CScrollWindowImpl<XMSamplerUIInst::EnvelopeEditor>, 1)
			END_MSG_MAP();


		};
		
		/** SamplerAssignEditor ÉNÉâÉX  */
		class SampleAssignEditor : public CScrollWindowImpl<XMSamplerUIInst::SampleAssignEditor>
		{
		public:
			static const UINT SHARPKEY_XPOS[5];
			
			static const DWORD IDC_EDIT_SAMPLE_NO = 1001;
			static const DWORD IDC_UPDOWN_SAMPLE_NO = 1002;

			/// Window Class Definition
			DECLARE_WND_SUPERCLASS(_T("SampleAssignEditor"),GetWndClassName())
			/// ***** [bohan] iso-(10)646 encoding only please! *****
			SampleAssignEditor();
			/// ***** [bohan] iso-(10)646 encoding only please! *****
			virtual ~SampleAssignEditor();
			void DoPaint(CDCHandle dc);///< WM_PAINT Handler
			void Initialize(XMSampler* const pXMSampler,const int targetInstrumentNo);
			// size operations
			void SetScrollSize(int cx, int cy, BOOL bRedraw = TRUE)
			{
				// reset current range to prevent scroll bar problems
				SCROLLINFO si = { sizeof(si), SIF_RANGE, 0, 0 };
				SetScrollInfo(SB_HORZ, &si, FALSE);
				SetScrollInfo(SB_VERT, &si, FALSE);
	
				m_sizeAll.cx = cx;
				m_sizeAll.cy = cy;

				m_ptOffset.x = 0;
				m_ptOffset.y = 0;

				si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL;
				si.nMin = 0;


				si.nMax = m_sizeAll.cy - 1;
				si.nPage = m_sizeClient.cy;
				si.nPos = m_ptOffset.y;
				SetScrollInfo(SB_VERT, &si, bRedraw);

				si.nMax = m_sizeAll.cx - 1;
				si.nPage = m_sizeClient.cx;
				si.nPos = m_ptOffset.x;
				SetScrollInfo(SB_HORZ, &si, bRedraw);

				SetScrollLine(0, 0);
				SetScrollPage(0, 0);

				Invalidate();
			};

			void OnSize(const UINT wParam, const CSize size)
			{

				m_sizeClient.cx = size.cx;
				m_sizeClient.cy = size.cy;

				SIZE szDelta = { 0, 0 };

				if(m_ptOffset.x + m_sizeClient.cx > m_sizeAll.cx)
				{
					if(m_sizeClient.cx >= m_sizeAll.cx)
					{
						szDelta.cx = m_ptOffset.x;
						m_ptOffset.x = 0;
					}
					else
					{
						szDelta.cx = m_ptOffset.x;
						m_ptOffset.x = m_sizeAll.cx - m_sizeClient.cx;
						szDelta.cx -= m_ptOffset.x;
					}
				}

				if(m_ptOffset.y + m_sizeClient.cy > m_sizeAll.cy)
				{
					if(m_sizeClient.cy >= m_sizeAll.cy)
					{
						szDelta.cy = m_ptOffset.y;
						m_ptOffset.y = 0;
					}
					else
					{
						szDelta.cy = m_ptOffset.y;
						m_ptOffset.y = m_sizeAll.cy - m_sizeClient.cy;
						szDelta.cy -= m_ptOffset.y;
					}
				}

				SCROLLINFO si;
				si.cbSize = sizeof(si);
				si.fMask = SIF_PAGE|SIF_POS;

				si.nPage = m_sizeClient.cx;
				si.nPos = m_ptOffset.x;
				SetScrollInfo(SB_HORZ, &si, TRUE);

				si.nPage = m_sizeClient.cy;
				si.nPos = m_ptOffset.y;
				SetScrollInfo(SB_VERT, &si, TRUE);

				ScrollWindowEx(szDelta.cx, szDelta.cy, m_uScrollFlags);

				SetMsgHandled(TRUE);
			}

			//BOOL SubclassWindow(const HWND hwnd)
			//{
			//	// ***** [bohan] iso-(10)646 encoding only please! *****
			//	::WNDCLASSEX _wc = GetWndClassInfo().m_wc;
			//	_wc.lpfnWndProc = m_pWndProcBackup;
			//	return CScrollWindowImpl<SampleAssignEditor>::SubclassWindow(hwnd);
			//}
			
			LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
			{
				CScrollWindowImpl<XMSamplerUIInst::SampleAssignEditor>::OnCreate(uMsg,wParam,lParam,bHandled);
				InitializeScrollWindow();
				return 1;
			};

			/** ç∂É{É^ÉìÉ_ÉEÉìÉCÉxÉìÉgÉnÉìÉhÉâ */
			void OnLButtonDown(const UINT vKey, WTL::CPoint position);
			/** ç∂É{É^ÉìÉAÉbÉvÉCÉxÉìÉgÉnÉìÉhÉâ */
			void OnLButtonUp(const UINT vKey, WTL::CPoint position);
			/** MouseÉJÅ[É\Éãà⁄ìÆÉCÉxÉìÉgÉnÉìÉhÉâ */
			void OnMouseMove(const UINT vKey, WTL::CPoint position);
			/** âEÉ{É^ÉìÉ_ÉEÉìÉCÉxÉìÉgÉnÉìÉhÉâ */
			void OnRButtonDown(const UINT vKey, WTL::CPoint position);

		private:
			/// ***** [bohan] iso-(10)646 encoding only please! *****
			void InitializeScrollWindow();

			/// ***** [bohan] iso-(10)646 encoding only please! *****Index***** [bohan] iso-(10)646 encoding only please! *****
			const boost::optional<int> GetKeyIndexAtPoint(const int x,const int y,WTL::CRect& keyRect);
	
			XMSampler * m_pXMSampler;///< XMSampler***** [bohan] iso-(10)646 encoding only please! *****
			int m_InstrumentNo;///< ***** [bohan] iso-(10)646 encoding only please! ***** Instrument***** [bohan] iso-(10)646 encoding only please! *****
			bool m_bInitialized;///< ***** [bohan] iso-(10)646 encoding only please! *****
			//static boost::shared_ptr<Gdiplus::Image> m_pNoteKey;///< ***** [bohan] iso-(10)646 encoding only please! *****
			//static boost::shared_ptr<Gdiplus::Image> m_pSeminoteKey;///< ***** [bohan] iso-(10)646 encoding only please! *****
			static Gdiplus::Bitmap * m_pNaturalKey;///< ***** [bohan] iso-(10)646 encoding only please! *****
			static Gdiplus::Bitmap * m_pSharpKey;///< ***** [bohan] iso-(10)646 encoding only please! *****
			static int m_CreateCount;///< ***** [bohan] iso-(10)646 encoding only please! *****
			static const int m_NoteAssign[KEY_PER_OCTAVE];/// ***** [bohan] iso-(10)646 encoding only please! *****
			WNDPROC m_pWndProcBackup;///< ***** [bohan] iso-(10)646 encoding only please! *****
			boost::optional<int> m_FocusKeyIndex;///< ***** [bohan] iso-(10)646 encoding only please! *****
			CRect m_FocusKeyRect;///< ***** [bohan] iso-(10)646 encoding only please! *****
			WTL::CEdit m_EditSampleNo;///< ***** [bohan] iso-(10)646 encoding only please! *****
			WTL::CUpDownCtrl m_UpDownSampleNo;///< ***** [bohan] iso-(10)646 encoding only please! *****
		public:
			// Message Map
			BEGIN_MSG_MAP_EX(XMSamplerUIInst::SampleAssignEditor)
				MSG_WM_SIZE(OnSize)
				//MSG_WM_PAINT(OnPaint)
				MSG_WM_LBUTTONDOWN(OnLButtonDown)
				MSG_WM_LBUTTONUP(OnLButtonUp)
		        MSG_WM_RBUTTONDOWN(OnRButtonDown)
				MSG_WM_MOUSEMOVE(OnMouseMove)

		  //      COMMAND_ID_HANDLER_EX(ID_ADD_POINT, OnAddPoint)
				//COMMAND_ID_HANDLER_EX(ID_DEL_POINT, OnDelPoint)
				//COMMAND_ID_HANDLER_EX(ID_SET_SUSTAIN, OnSetSustain)
				//COMMAND_ID_HANDLER_EX(ID_REMOVE_SUSTAIN, OnRemoveSustain)
				//COMMAND_ID_HANDLER_EX(ID_SET_LOOPSTART, OnSetLoopStart)
				//COMMAND_ID_HANDLER_EX(ID_SET_LOOPEND, OnSetLoopEnd)
				//COMMAND_ID_HANDLER_EX(ID_REMOVE_LOOP, OnRemoveLoop)
				MESSAGE_HANDLER(WM_CREATE, OnCreate)
				CHAIN_MSG_MAP(CScrollWindowImpl<SampleAssignEditor>)
				CHAIN_MSG_MAP_ALT(CScrollWindowImpl<SampleAssignEditor>, 1)
			END_MSG_MAP()

		};// SampleAssignEditor

	public:
		/// Dialog ID
		enum { IDD = IDD_XMSAMPLERUIINST };

		/// Constrcutor
		XMSamplerUIInst();
		/// Destructor
		~XMSamplerUIInst()
		{
		};
		
		void pMachine(XMSampler * const p){m_pMachine = p;};
		XMSampler * const pMachine(){return m_pMachine;};

	private:
		void SetInstrumentData(const int instno);
		
		void SetNNACombo(const int nna);
//		void SetFilterModeCombo(const int mode);

		
		WTL::CEdit m_InstNo;
		WTL::CEdit m_InstName;
//		WTL::CComboBox m_FilterType;
		WTL::CComboBox m_NNA;
		WTL::CTrackBarCtrl m_Pan;
//		SF::UI::Volume m_Pan;
		SF::UI::Volume<float> m_Test;
		WTL::CTrackBarCtrl m_Cutoff;
		WTL::CTrackBarCtrl m_Q;
		WTL::CTrackBarCtrl m_Envelope;
		WTL::CTrackBarCtrl m_FilterType;
		
		WTL::CButton m_RandomPanning;
		WTL::CButton m_RandomVCFCuttoff;
		WTL::CButton m_RandomVCFResonance;
		
		EnvelopeEditor m_EnvelopeEditor;
		SampleAssignEditor m_SampleAssignEditor;


		WTL::CStatic m_CutoffLabel;
		WTL::CStatic m_QLabel;
		WTL::CStatic m_EnvelopeLabel;
		WTL::CStatic m_PanLabel;
		WTL::CStatic m_ModeLabel;

		WTL::CUpDownCtrl m_InstNoUpDown;

		XMSampler *m_pMachine;
//		XMSampler m_XMSamplerBack;
		
		int m_CurrentInstNo;
		bool m_bInitialized;
	
	public:
			// Message Map
		BEGIN_MSG_MAP_EX(XMSamplerUIInst)
			MSG_WM_INITDIALOG(OnInitDialog)
			COMMAND_HANDLER_EX(IDC_INST_NO,EN_CHANGE,OnChangeInstNo)
			COMMAND_HANDLER_EX(IDC_CUSTOM1,EN_CHANGE,OnChangeCustom)
			COMMAND_HANDLER_EX(IDC_INST_NAME, EN_CHANGE, OnChangeInstName)
		
			NOTIFY_HANDLER_EX(IDC_CUTOFF, NM_CUSTOMDRAW, OnCustomdrawSliderCutoff)
			NOTIFY_HANDLER_EX(IDC_Q, NM_CUSTOMDRAW, OnCustomdrawSliderQ)
			NOTIFY_HANDLER_EX(IDC_ENVELOPE, NM_CUSTOMDRAW, OnCustomdrawEnvelope)
	//		NOTIFY_HANDLER_EX(IDC_PANSLIDER, NM_CUSTOMDRAW, OnCustomdrawPan)
			NOTIFY_HANDLER_EX(IDC_SLIDER_MODE, NM_CUSTOMDRAW, OnCustomdrawMode)
			REFLECT_NOTIFICATIONS()
//			CHAIN_MSG_MAP(CPropertyPageImpl<XMSamplerUIInst>)
		END_MSG_MAP()
	private:

	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);(& )

		// Event Handlers
		
		LRESULT OnInitDialog(HWND hwnd, LPARAM lParam);
		LRESULT OnCustomdrawSliderCutoff(NMHDR* pNMHDR);
		LRESULT OnCustomdrawSliderQ(NMHDR* pNMHDR);
		LRESULT OnCustomdrawEnvelope(NMHDR* pNMHDR);
		LRESULT OnCustomdrawPan(NMHDR* pNMHDR);
		LRESULT OnCustomdrawMode(NMHDR* pNMHDR);

		void OnChangeInstNo(UINT id,int command,HWND hwnd);
		void OnChangeInstName(UINT id,int command,HWND hwnd);
		void OnChangeCustom(UINT id,int command,HWND hwnd){
			ATLTRACE2(_T("OnChangeCustom\n"));
		};
#endif 

};
NAMESPACE__END
NAMESPACE__END

