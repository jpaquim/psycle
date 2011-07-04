#pragma once
/** @file
 *  @brief Volume UI 
 *  $Date: 2004/06/20 21:05:52 $
 *  $Revision: 1.2 $
 */
namespace SF {
	namespace UI {
		template < class ValueT >
		class Volume : public ATL::CWindowImpl<Volume< ValueT > >
		{
		public:
			DECLARE_WND_SUPERCLASS(_T("Volume"),GetWndClassName())
			/// Constructor
			Volume();
			/// Destructor
			~Volume();
			
			/// Create
			HWND Create(HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName,
				DWORD dwStyle, DWORD dwExStyle, _U_MENUorID MenuOrID, LPVOID lpCreateParam);
			
			// メッセージマップ
			BEGIN_MSG_MAP_EX(Volume<ValueT> )
		        MSG_WM_HSCROLL(OnHScroll)
				MSG_WM_CREATE(OnCreate)
				MSG_WM_TIMER(OnTimer)
				MSG_WM_PAINT(OnPaint)
				MSG_WM_SETFOCUS(OnSetFocus)
				MSG_WM_KILLFOCUS(OnKillFocus)
				MSG_WM_MOUSEMOVE(OnMouseMove)
				MSG_WM_MOUSEWHEEL(OnMouseWheel)
				MSG_WM_LBUTTONDOWN(OnLButtonDown)
				MSG_WM_NCMOUSEMOVE(OnNCMouseMove)
			END_MSG_MAP()

			
			LRESULT OnCreate(const LPCREATESTRUCT pCreateStruct)
			{
				Initialize();
				return 0;
			};

			void OnTimer(const UINT wParam, TIMERPROC lParam)
			{
				ATLTRACE2(_T("OnHScroll\n"));
				KillTimer(wParam);
			}

			void OnMouseMove(const UINT flag,const WTL::CPoint& point)
			{
				//ATLTRACE2(_T("OnMouseMove:x:%d:y:%d\n"),point.x,point.y);
				
				if(!m_bCapture){
					SetCapture();
					m_bCapture = true;
				}

				if((m_RectSize.top < point.y) | (m_RectSize.bottom > point.y) |
					(m_RectSize.left < point.x) | (m_RectSize.right > point.x))
				{
					ReleaseCapture();
					m_bCapture = false;
				}

			}

			void OnLButtonDown(const UINT flags, const WTL::CPoint & point)
			{
				ATLTRACE2(_T("OnLButtonDown:x:%d:y:%d\n"),point.x,point.y);
				SetFocus();
			}

			void OnSetFocus(const HWND hwnd){
				ATLTRACE2(_T("OnSetFocus\n"));
				OnPaint(NULL);
				SetMsgHandled(TRUE);
			}
			
			void OnKillFocus(const HWND hwnd){
				ATLTRACE2(_T("OnKillFocus\n"));
				OnPaint(NULL);
				SetMsgHandled(TRUE);
			}

		    void OnHScroll(const int nCode, const short nPos, const HWND hWnd)
			{
				ATLTRACE2(_T("OnHScroll\n"));
				OnPaint(NULL);
				SetMsgHandled(TRUE);
			}

			LRESULT OnMouseWheel(const UINT flags,const short value,const WTL::CPoint& point)
			{
				ATLTRACE2(_T("OnMouseWheel:x:%d:y:%d:flag:%x:value:%d\n"),point.x,point.y,flags,value);
				if(value > 0){
					m_Value += m_Tick;
					if(m_Value > m_Max){
						m_Value = m_Max;
					} else {
						LRESULT res = ::SendMessage(GetParent(),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(),EN_CHANGE),(LPARAM)m_hWnd);
						ATLTRACE2("%x\n",res);
					}
				} else if(value < 0)
				{
					m_Value -= m_Tick;
					if(m_Value < m_Min)
					{
						m_Value = m_Min;
					} else {
						LRESULT res = ::SendMessage(GetParent(),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(),EN_CHANGE),(LPARAM)m_hWnd);
						ATLTRACE2("%x\n",res);
					}
				}
				return 0;
			};
			
			void OnNCMouseMove(const UINT flags,const CPoint & point)
			{
				ATLTRACE2(_T("OnNCMouseMove\n"));
			
			};

			void OnPaint(HDC hdc);


			/// Initialize Volume Control
			void Initialize();

			/// this method avoid memory reak check
			static void Release()
			{
				m_pVolumeSprite.Release();
				m_pTexture.Release();
			};

			/// Get Value
			const ValueT& Value(){ return m_Value;};
			/// Set Value
			void Value(const ValueT& value){m_Value = value;};
			
			/// Get Tick
			const ValueT& Tick(){return m_Tick;};
			/// Set Tick
			void Tick(const ValueT & value){m_Tick = value;};

			/// Get Min Value
			const ValueT& Min(){return m_Min;};
			/// Set Min Value
			void Min(const ValueT& value){m_Min = value;};
				
			/// Get Min Value
			const ValueT& Max(){return m_Max;};
			/// Set Min Value
			void Max(const ValueT& value){m_Max = value;};

		private:
			CRect m_RectSize;
			ValueT m_Value;
			ValueT m_Tick;
			ValueT m_Min;
			ValueT m_Max;
			bool m_bCapture;
			static CComPtr<ID3DXSprite> m_pVolumeSprite;
			static CComPtr<IDirect3DTexture9>	m_pTexture;
		};

	}
}
