#pragma once
#include "afxcmn.h"


// XMSamplerUIInst
NAMESPACE__BEGIN(psycle)
NAMESPACE__BEGIN(host)

class XMSampler;
	
	class XMSamplerUIInst : public CPropertyPage
	{
#if 0
		/** EnvelopeEditor Class 
		 * Envelope Graph *****  *****Edit*****  */
		class EnvelopeEditor : public CScrollView
		{
		public:
			// constant
			static const int MARGIN_RIGHT = 100 /* pixel */;
			static const int POINT_SIZE = 6 /* pixel */;///< Envelope Point 
			static const int HITTEST_NOT_FOUND = -1;///< HitTest
			// Exception
			
	
			/// Window Class Definition
			DECLARE_WND_SUPERCLASS(_T("EnvelopeEditor"),GetWndClassName());
			
			/// Constructor
			EnvelopeEditor();
		
			/// Initialize 
			void Initialize(XMSampler * const pSampler,XMInstrument::Envelope * const pEnvelope);

			/// Envelope
			void EditEnvelope(XMInstrument::Envelope * const pEnvelope);
			
			/// 
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

			void DoPaint(CDC* dc);///< WM_PAINT Handler
			void OnLButtonDown(const UINT vKey, CPoint position);
			void OnLButtonUp(const UINT vKey, CPoint position);
			void OnMouseMove(const UINT vKey, CPoint position);
			void OnRButtonDown(const UINT vKey, CPoint position);
		    void OnAddPoint(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
		    void OnDelPoint(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
		    void OnSetSustain(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
		    void OnRemoveSustain(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
		    void OnSetLoopStart(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
		    void OnSetLoopEnd(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
		    void OnRemoveLoop(const UINT uNotifyCode, const int nID, const HWND hWndCtl);
			
			static LRESULT CALLBACK DummyWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		private:
			typedef int EnvelopePointIndex;

			/**  */
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

				return EnvelopePointIndex();//
			};

			/** Scroll Size  */
			void AdjustScrollRect(const int maxPoint);
			//const WTL::CPoint TickToPosition(const int index);///< convert Tick To Screen Position

			XMInstrument::Envelope* m_pEnvelope;///< Envelope Class Pointer
			XMInstrument::Envelope m_EnvelopeEditing;///< Envelope Data backup
			XMSampler* m_pXMSampler;///< XMSampler Pointer
			bool m_bInitialized;///< Initialize Flag
			float m_Zoom;///< Zoom
			int m_CurrentScrollWidth;///< 
			int m_CurrentScrollHeight;///<
			
			bool m_bPointEditing;///< EnvelopePoint 
			int m_EditPoint;///< ***** Envelope Point Index
			int m_EditPointOrig;///< Envelope Point Index
			int m_EditPointX;///< Envelope Point
			int m_EditPointY;///< Envelope Point
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
		

		/** SamplerAssignEditor ƒNƒ‰ƒX  */
		class SampleAssignEditor : public CScrollWindowImpl<XMSamplerUIInst::SampleAssignEditor>
		{
		public:
			static const UINT SHARPKEY_XPOS[5];
			
			static const DWORD IDC_EDIT_SAMPLE_NO = 1001;
			static const DWORD IDC_UPDOWN_SAMPLE_NO = 1002;

			/// Window Class Definition
			DECLARE_WND_SUPERCLASS(_T("SampleAssignEditor"),GetWndClassName())
			/// 
			SampleAssignEditor();
			/// 
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
			//	
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

			/** */
			void OnLButtonDown(const UINT vKey, WTL::CPoint position);
			/** */
			void OnLButtonUp(const UINT vKey, WTL::CPoint position);
			/** */
			void OnMouseMove(const UINT vKey, WTL::CPoint position);
			/**  */
			void OnRButtonDown(const UINT vKey, WTL::CPoint position);

		private:
			/// 
			void InitializeScrollWindow();

			/// Index
			const boost::optional<int> GetKeyIndexAtPoint(const int x,const int y,WTL::CRect& keyRect);
	
			XMSampler * m_pXMSampler;///< XMSampler
			int m_InstrumentNo;///< Instrument
			bool m_bInitialized;///< 
			//static boost::shared_ptr<Gdiplus::Image> m_pNoteKey;///< 
			//static boost::shared_ptr<Gdiplus::Image> m_pSeminoteKey;///< 
			static Gdiplus::Bitmap * m_pNaturalKey;///< 
			static Gdiplus::Bitmap * m_pSharpKey;///< 
			static int m_CreateCount;///< *
			static const int m_NoteAssign[KEY_PER_OCTAVE];/// *
			WNDPROC m_pWndProcBackup;///< 
			boost::optional<int> m_FocusKeyIndex;///< 
			CRect m_FocusKeyRect;///<
			WTL::CEdit m_EditSampleNo;///< 
			WTL::CUpDownCtrl m_UpDownSampleNo;///< 
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


	DECLARE_DYNAMIC(XMSamplerUIInst)

	public:
		/// Constrcutor
		XMSamplerUIInst();
		/// Destructor
		virtual ~XMSamplerUIInst();

		/// Dialog ID
		enum { IDD = IDD_XMSAMPLERUIINST };
		
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX o DDV

	public:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnNMCustomdrawGlobvol(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnNMCustomdrawFadeout(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnNMCustomdrawDefPan(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnNMCustomdrawPitchPanMod(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnNMCustomdrawCutoff(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnNMCustomdrawRessonance(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnNMCustomdrawFilterMod(NMHDR *pNMHDR, LRESULT *pResult);

		void pMachine(XMSampler * const p){m_pMachine = p;};
		XMSampler * const pMachine(){return m_pMachine;};


	private:
		void SetInstrumentData(const int instno);
		
		void SetNNACombo(const int nna);
//		void SetFilterModeCombo(const int mode);

		
		CSliderCtrl m_SlGlobVol;
		CSliderCtrl m_SlFadeout;
		CSliderCtrl m_SlDefPan;
		CSliderCtrl m_SlPitchPanMod;
		CSliderCtrl m_SlCutoff;
		CSliderCtrl m_SlRessonance;
		CSliderCtrl m_SlFilterMod;

		WTL::CEdit m_InstNo;
		WTL::CEdit m_InstName;
//		WTL::CComboBox m_FilterType;
		WTL::CComboBox m_NNA;
		
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
		void OnChangeCustom(UINT id,int command,HWND hwnd)
		{
			ATLTRACE2(_T("OnChangeCustom\n"));
		};
#endif
};

NAMESPACE__END
NAMESPACE__END

