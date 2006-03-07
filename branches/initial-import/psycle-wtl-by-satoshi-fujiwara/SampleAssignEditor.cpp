// XMSamplerUIInst.cpp : XMSamplerUIInst の実装
/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 */
#include "stdafx.h"
#include "volume.h"
#include "d3d.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "XMInstrument.h"
#include "XMSampler.h"
#include "XMSamplerUIInst.h"

using namespace Gdiplus;

namespace SF {

	//////////////////////////////////////////////////////////////////////////////
	// SampleAssignEditor ------------------------------------------------------------
	//////////////////////////////////////////////////////////////////////////////

	//boost::shared_ptr<Gdiplus::Image> XMSamplerUIInst::SampleAssignEditor::m_pNaturalKey;///< 白鍵
	//boost::shared_ptr<Gdiplus::Image> XMSamplerUIInst::SampleAssignEditor::m_pSharpKey;///< 黒鍵
	Gdiplus::Bitmap* XMSamplerUIInst::SampleAssignEditor::m_pNaturalKey;///< 白鍵
	Gdiplus::Bitmap* XMSamplerUIInst::SampleAssignEditor::m_pSharpKey;///< 黒鍵
	const UINT XMSamplerUIInst::SampleAssignEditor::SHARPKEY_XPOS[SHARP_KEY_PER_OCTAVE] = {27,96,190,246,304};
	const int XMSamplerUIInst::SampleAssignEditor::m_NoteAssign[KEY_PER_OCTAVE] 
	= {NATURAL_KEY /*白*/,SHARP_KEY /*黒*/,NATURAL_KEY,SHARP_KEY,NATURAL_KEY,NATURAL_KEY,SHARP_KEY,NATURAL_KEY,SHARP_KEY,NATURAL_KEY,SHARP_KEY,NATURAL_KEY};
	int XMSamplerUIInst::SampleAssignEditor::m_CreateCount = 0;///< リソースがロードされたかどうか

	/** コンストラクタ */
	XMSamplerUIInst::SampleAssignEditor::SampleAssignEditor() : m_FocusKeyIndex()
	{
		// ダミーのウィンドウクラスの登録
		//SubclassWindowが呼ばれるまではこのウィンドウプロシージャで処理する
		::WNDCLASSEX _wc = GetWndClassInfo().m_wc;
		m_pWndProcBackup = _wc.lpfnWndProc;
		_wc.lpfnWndProc = &::DefWindowProc;
		::RegisterClassEx(&_wc);
		m_pXMSampler = NULL;
		

		m_bInitialized = false;

		if(m_CreateCount == 0)
		{
			std::wstring _note_fname = CT2CW(::Global::CurrentDirectory().c_str());
			_note_fname += L"\\Plugins\\SF\\XMSampler\\Resources\\notekey.png";

			std::wstring _sharpkey_fname = CT2CW(::Global::CurrentDirectory().c_str());
			_sharpkey_fname += L"\\Plugins\\SF\\XMSampler\\Resources\\sharpkey.png";
			//m_pNaturalKey.reset(Image::FromFile(_note_fname.c_str()));
			m_pNaturalKey = Bitmap::FromFile(_note_fname.c_str());
			//m_pSharpKey.reset(Image::FromFile(_sharpkey_fname.c_str()));
			m_pSharpKey = Bitmap::FromFile(_sharpkey_fname.c_str());
		}
		m_CreateCount++;
	}

	/** ~SampleAssignEditor() */
	XMSamplerUIInst::SampleAssignEditor::~SampleAssignEditor()
	{
		m_CreateCount--;
		if(m_CreateCount == 0)
		{
			delete m_pNaturalKey;
			delete m_pSharpKey;
		}
	}
	
	/** WM_PAINT ハンドラ */
	void XMSamplerUIInst::SampleAssignEditor::DoPaint(CDCHandle dc)
	{
	
		if(m_bInitialized){
			
			ATLASSERT(m_pXMSampler != NULL);

			// 描画用 HDCの取得
			HDC hdc = dc.m_hDC;
			
			CPoint _pt_viewport_org;
			dc.GetViewportOrg(&_pt_viewport_org);// 現在の
			WTL::CRect _rect;
			GetClientRect(&_rect);
	
			Graphics graphics(hdc);

			//Pen      _line_pen(m_Color,1.0);
			//Pen      _gridpen(Gdiplus::Color(32, 0, 0, 255),1.0);
			//Pen      _gridpen1(Gdiplus::Color(32, 255, 0, 0),1.0);
			//Pen      _gridpen2(Gdiplus::Color(64, 255, 0, 0),1.0);
			//
			//Gdiplus::SolidBrush  brush(Gdiplus::Color(255, 255, 255, 255));

			// 画面をいったんクリアする
			//graphics.Clear(Gdiplus::Color(255,255,255,255));
			graphics.SetRenderingOrigin(_pt_viewport_org.x,_pt_viewport_org.y);
			graphics.SetSmoothingMode(SmoothingModeAntiAlias);
			// 文字列をAnti Aliasに
			graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
			
			// Draw Grid
			
			//Gdiplus::SolidBrush  _fontbrush(Gdiplus::Color(255, 0, 0, 255));
			//Gdiplus::FontFamily  fontFamily(L"Times New Roman");
			//Gdiplus::Font        font(&fontFamily, 8, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
			//Gdiplus::PointF      pointF(10.0f, 20.0f);
			

			const UINT _width = m_pNaturalKey->GetWidth();
//			const UINT _end = XMInstrument::MAX_NOTES / 12 * NATURAL_KEY_PER_OCTAVE * _width;
			const UINT _end = XMInstrument::MAX_NOTES / 12 * NATURAL_KEY_PER_OCTAVE;

			const std::wstring _natural_key_name[NATURAL_KEY_PER_OCTAVE] = {L"C",L"D",L"E",L"F",L"G",L"A",L"B"};
			const int _natural_key_index[NATURAL_KEY_PER_OCTAVE] = {0,2,4,5,7,9,11};
			Gdiplus::FontFamily  _font_family_keyname(L"Times New Roman");
			Gdiplus::Font        _font_keyname(&_font_family_keyname, 10, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
			Gdiplus::SolidBrush  _font_keyname_brush(Gdiplus::Color(128, 0, 0, 0));// (A,R,G,B)
			Gdiplus::SolidBrush  _font_sampleno_brush(Gdiplus::Color(128, 192, 0, 0));

			CachedBitmap _cache_natural_key(m_pNaturalKey,&graphics);
			std::wstring _tmp_str;
	
			int _index = 0,_octave = 0;
			for(UINT i = 0;i < _end;i++)
			{
//				graphics.DrawString(_natural_key_name[(i / _width) % 7].c_str()) 
				_tmp_str = _natural_key_name[_index];
				int _note = _natural_key_index[_index] + _octave * KEY_PER_OCTAVE;
				_tmp_str += boost::lexical_cast<std::wstring>(i / NATURAL_KEY_PER_OCTAVE);
				//graphics.DrawCachedBitmap(&_cache_natural_key,i,0);

				if(m_FocusKeyIndex && *m_FocusKeyIndex == _note){
					SolidBrush _brush(Color(128,255,255,255));
					
					// GDI+ 1.1はまだ使えない
					//REAL srcWidth = (REAL)m_pNaturalKey->GetWidth();
					//REAL srcHeight = (REAL)m_pNaturalKey->GetHeight();
					//RectF srcRect(0.0f, 0.0f, srcWidth, srcHeight);
					//Matrix myMatrix(1.0f, 0.0f, 0.0f, 1.0f, i, 0);

					//BrightnessContrastParams briConParams;
					//briConParams.brightnessLevel = 50;
					//briConParams.contrastLevel = 25;
					//BrightnessContrast briCon;
					//briCon.SetParameters(&briConParams);

					//graphics.DrawImage(m_pNaturalKey, &srcRect, &myMatrix, &briCon, NULL, UnitPixel);
					graphics.DrawCachedBitmap(&_cache_natural_key,i * _width,0);
					graphics.FillRectangle(&_brush,i * _width,0,m_pNaturalKey->GetWidth(),m_pNaturalKey->GetHeight());

				} else {
					graphics.DrawCachedBitmap(&_cache_natural_key,i * _width,0);
				}
				
				// 音階表示
				graphics.DrawString(_tmp_str.c_str(),_tmp_str.length(),&_font_keyname,
					PointF(	(float)(i * _width + 5),98.0f),&_font_keyname_brush);
				
				// アサインしたサンプル番号を表示
				_tmp_str = boost::lexical_cast<std::wstring>(m_pXMSampler->Instrument(m_InstrumentNo).NoteToSample(_note));
				graphics.DrawString(_tmp_str.c_str(),_tmp_str.length(),&_font_keyname,
					PointF(	(float)(i * _width + 5),88.0f),&_font_sampleno_brush);
				
				_index++;
				if(_index == NATURAL_KEY_PER_OCTAVE){
					_index = 0;
					_octave++;
				}
			}

			CachedBitmap _cache_sharpkey(m_pSharpKey,&graphics);
			
			//const UINT _blkey_xpos[5] = {27,96,190,246,304};
			const std::wstring _sharpkey_name[5] = {L"C#",L"D#",L"F#",L"G#",L"A#"};
			const int _sharpkey_index[SHARP_KEY_PER_OCTAVE] = {1,3,6,8,10};

			Gdiplus::SolidBrush  _font_sharpkeyname_brush(Gdiplus::Color(128, 255, 255, 255));
			Gdiplus::SolidBrush  _font_sharpkey_brush1(Gdiplus::Color(192, 255, 128, 128));

			_index = 0;_octave = 0;
			for(UINT i = 0;i < 10/*octave*/ * 5 /*key*/;i++)
			{
				_tmp_str = _sharpkey_name[_index];
				_tmp_str += boost::lexical_cast<std::wstring>(_octave);
				
				int _sharpkey = _sharpkey_index[_index] + _octave * KEY_PER_OCTAVE;

				graphics.DrawCachedBitmap(&_cache_sharpkey,
					SHARPKEY_XPOS[_index] + _width * NATURAL_KEY_PER_OCTAVE * _octave ,0);
				if(m_FocusKeyIndex && *m_FocusKeyIndex == _sharpkey)
				{
					SolidBrush _brush(Color(128,255,255,255));
					graphics.FillRectangle(&_brush,SHARPKEY_XPOS[_index] + _width * NATURAL_KEY_PER_OCTAVE * _octave,0,m_pSharpKey->GetWidth(),m_pSharpKey->GetHeight());
				}
				// 音階表示
				graphics.DrawString(_tmp_str.c_str(),_tmp_str.length(),&_font_keyname,
					PointF(	(float)(SHARPKEY_XPOS[_index] + _width * NATURAL_KEY_PER_OCTAVE * _octave),55.0f),
					&_font_sharpkeyname_brush);

				// アサインしたサンプル番号を表示
				_tmp_str = boost::lexical_cast<std::wstring>(m_pXMSampler->Instrument(m_InstrumentNo).NoteToSample(_sharpkey));
				graphics.DrawString(_tmp_str.c_str(),_tmp_str.length(),&_font_keyname,
					PointF(	(float)(SHARPKEY_XPOS[_index] + _width * NATURAL_KEY_PER_OCTAVE * _octave),45.0f),&_font_sharpkey_brush1);

				_index++;
				if(_index == SHARP_KEY_PER_OCTAVE){
					_index = 0;
					_octave++;
				}
			}

		} else {
			InitializeScrollWindow();
		}
		SetMsgHandled(FALSE);
	}

	void XMSamplerUIInst::SampleAssignEditor::Initialize(XMSampler* const pXMSampler,const int targetInstrumentNo)
	{
		
		bool _binit = m_bInitialized;
		m_bInitialized = false;

		m_pXMSampler = pXMSampler;
		m_InstrumentNo = targetInstrumentNo;

		GetSystemSettings();

		if(!_binit){
			RECT _rect = {0,0,40,15};

			m_EditSampleNo.Create(m_hWnd,ATL::_U_RECT(_rect),NULL,WS_CHILD | WS_VISIBLE ,WS_EX_CLIENTEDGE | WS_EX_RIGHT,ATL::_U_MENUorID(IDC_EDIT_SAMPLE_NO),NULL);
			m_EditSampleNo.ShowWindow(FALSE);

			_rect.left = 20; _rect.top = 0;
			_rect.right = 30;
			_rect.bottom = 15;

			m_UpDownSampleNo.Create(m_hWnd,ATL::_U_RECT(_rect),NULL,WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT ,0,ATL::_U_MENUorID(IDC_UPDOWN_SAMPLE_NO),NULL);
			m_UpDownSampleNo.SetBuddy(m_EditSampleNo);
			m_UpDownSampleNo.ShowWindow(FALSE);
		}
		m_bInitialized = true;
	};

	/** 画面の初期化 を行う
	 * ・H ScrollBarのみの表示
	 * ・widthの設定　 */
	void XMSamplerUIInst::SampleAssignEditor::InitializeScrollWindow()
	{
		WTL::CRect _rect_client;

		const UINT _width = m_pNaturalKey->GetWidth();
		const UINT _width_total = XMInstrument::MAX_NOTES / KEY_PER_OCTAVE * NATURAL_KEY_PER_OCTAVE * _width;

		GetClientRect(&_rect_client);
		// H ScrollBarをいったん表示させる。なぜかこれでは表示されない
		SetScrollSize(_width_total,_rect_client.Height());
		// 2回目の設定。これでようやく表示される
		SetScrollSize(_width_total,_rect_client.Height());
		// スクロールバーを表示することでClient Heightが変化するのでGetする
		GetClientRect(&_rect_client);
		// 変化したしたClient Heightをスクロールのy sizeにセットして終了
		SetScrollSize(_width_total,_rect_client.Height());

		m_bInitialized = true;

		Invalidate();
	}

	void XMSamplerUIInst::SampleAssignEditor::OnLButtonUp(const UINT vKey, WTL::CPoint position)
	{
		ATLTRACE2("XMSamplerUIInst::SampleAssignEditor::OnLButtonUp\n");
	}

	void XMSamplerUIInst::SampleAssignEditor::OnMouseMove(const UINT vKey, WTL::CPoint position)
	{
		ATLTRACE2("XMSamplerUIInst::SampleAssignEditor::OnMouseMove\n");
		CPoint _offset;
		GetScrollOffset(_offset);

		position.x += _offset.x;
		position.y += _offset.y;
		
		//// どの鍵盤の上にマウスカーソルがあるかを判定する
		//if(m_FocusKeyIndex)
		//{
		//	// 以前判定したものをキャッシュして先ずそれと比較することによって処理を効率化する。
		//	// ・マウスカーソルは激しく動いていない
		//	// ・同じ鍵盤の上のあることが多い
		//	// ため、以前キャッシュしていたものに該当する確率が高い。
		//	if(position.x >= m_FocusKeyRect.left && position.x <= m_FocusKeyRect.right 
		//		&& position.y >= m_FocusKeyRect.top && position.y <= m_FocusKeyRect.bottom){
		//		return;
		//	}
		//}

		boost::optional<int> _index(GetKeyIndexAtPoint(position.x,position.y,m_FocusKeyRect));
		if(_index)
		{
			if(!m_FocusKeyIndex || *m_FocusKeyIndex != *_index){
				m_FocusKeyIndex = _index;
				Invalidate(FALSE);
			}
		} else {
			if(!m_FocusKeyIndex){
				return;
			} else {
				m_FocusKeyIndex = _index;
				Invalidate(FALSE);
			}
		}
	}

	void XMSamplerUIInst::SampleAssignEditor::OnRButtonDown(const UINT vKey, WTL::CPoint position)
	{
		ATLTRACE2("XMSamplerUIInst::SampleAssignEditor::OnRButtonDown\n");

	}

	void XMSamplerUIInst::SampleAssignEditor::OnLButtonDown(const UINT vKey, WTL::CPoint position)
	{
		ATLTRACE2("SampleAssignEditor::OnLButtonDown()\n");

		SetFocus();
	}

	/// クリックした位置にある鍵盤のIndexを返す
	const boost::optional<int> XMSamplerUIInst::SampleAssignEditor::GetKeyIndexAtPoint(const int x,const int y,WTL::CRect& keyRect)
	{
		const int _octave_width = m_pNaturalKey->GetWidth() * NATURAL_KEY_PER_OCTAVE;

		// 黒鍵盤との判定
		const int _sharpkey_num = 10 /* octave */ * SHARP_KEY_PER_OCTAVE /* sharpkeys per octave */; 
		const int _sharpkey_offset[SHARP_KEY_PER_OCTAVE] = {1,3,6,8,10};
		const int _sharpkey_width = m_pSharpKey->GetWidth();
		const int _sharpkey_height = m_pSharpKey->GetHeight();

		for(int i = 0;i < _sharpkey_num;i++)
		{
			const int _xpos = SHARPKEY_XPOS[i % SHARP_KEY_PER_OCTAVE] + (i / SHARP_KEY_PER_OCTAVE) * _octave_width;
			if(x >= _xpos && x <= (_xpos + _sharpkey_width) && y >= 0 && y <= _sharpkey_height)
			{
				keyRect.top = 0;
				keyRect.bottom = _sharpkey_height;
				keyRect.left = _xpos;
				keyRect.right = _xpos + _sharpkey_width;

				return _sharpkey_offset[i % SHARP_KEY_PER_OCTAVE] + (i / SHARP_KEY_PER_OCTAVE) * KEY_PER_OCTAVE;
			}
		}

		// 白鍵盤との判定
		const int _notekey_num = 10 /* octave */ * NATURAL_KEY_PER_OCTAVE ; 
		const int _notekey_offset[NATURAL_KEY_PER_OCTAVE] = {0,2,4,5,7,9,11};
		const int _note_width = m_pNaturalKey->GetWidth();
		const int _note_height = m_pNaturalKey->GetHeight();

		for(int i = 0;i < _notekey_num;i++)
		{
			const int _xpos = _note_width * (i % NATURAL_KEY_PER_OCTAVE) + (i / NATURAL_KEY_PER_OCTAVE) * _octave_width;
			if(x >= _xpos && x <= (_xpos + _note_width) && y >= 0 && y <= _note_height)
			{
				keyRect.top = 0;
				keyRect.bottom = _note_height;
				keyRect.left = _xpos;
				keyRect.right = _xpos + _note_width;

				return _notekey_offset[i % NATURAL_KEY_PER_OCTAVE] + (i / NATURAL_KEY_PER_OCTAVE) * KEY_PER_OCTAVE;
			}
		}
	
		//for(int i = 0;i < XMInstrument::MAX_NOTES;i++)
		//{
		//	CPoint _pt_env;
		//	_pt_env.y = (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(i)));
		//	_pt_env.x = (int)(m_Zoom * (float)m_pEnvelope->Point(i));
		//	if(((_pt_env.x - POINT_SIZE / 2) <= x) & ((_pt_env.x + POINT_SIZE / 2) >= x) &
		//		((_pt_env.y - POINT_SIZE / 2) <= y) & ((_pt_env.y + POINT_SIZE / 2) >= y))
		//	{
		//		return i;
		//	}
		//}

		return boost::optional<int>();//無効な値を返す。
	}

}

// XMSamplerUIInst

