// XMSamplerUIInst.cpp : XMSamplerUIInst の実装
/** @file
 *  @brief implementation file
 *  $Date$
 *  $Revision$
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

// Constructor
XMSamplerUIInst::XMSamplerUIInst()
{
	m_bInitialized = false;
	m_CurrentInstNo = 0;


}


LRESULT XMSamplerUIInst::OnInitDialog(HWND hwnd,LPARAM lparam)
{
	ExecuteDlgInit(IDD);

	m_InstNo.Attach(GetDlgItem(IDC_INST_NO));
	
	m_InstNo.SetWindowText(_T("0"));

	m_InstName.Attach(GetDlgItem(IDC_INST_NAME));
	m_InstName.SetLimitText(31);

	m_FilterType.Attach(GetDlgItem(IDC_SLIDER_MODE));
	m_FilterType.SetRange(0,4);

	
//	m_FilterType.AddString(_T("ﾛｰﾊﾟｽ"));
//	m_FilterType.AddString(_T("ﾊｲﾊﾟｽ"));
//	m_FilterType.AddString(_T("ﾊﾞﾝﾄﾞﾊﾟｽ"));
//	m_FilterType.AddString(_T("ﾉｯﾁﾊﾞﾝﾄﾞ"));
//	m_FilterType.AddString(_T("無し"));

	
	m_NNA.Attach(GetDlgItem(IDC_XM_NNA_COMBO));
	m_NNA.AddString(_T("Note ｶｯﾄ"));
	m_NNA.AddString(_T("Note ﾘﾘｰｽ"));
	m_NNA.AddString(_T("無し"));

//	m_Pan.Initialize(GetDlgItem(IDC_PANSLIDER));
	m_Pan.Attach(GetDlgItem(IDC_PANSLIDER));
	m_Pan.SetRange(0,100);

	m_Test.SubclassWindow(GetDlgItem(IDC_CUSTOM1));
	m_Test.Initialize();
	m_Test.Min(0.0f);
	m_Test.Max(1.0f);
	m_Test.Tick(0.1f);

	
	m_Cutoff.Attach(GetDlgItem(IDC_CUTOFF));
	m_Cutoff.SetRange(0,127);

	m_Q.Attach(GetDlgItem(IDC_Q));
	m_Q.SetRange(0,127);
	
	m_Envelope.Attach(GetDlgItem(IDC_ENVELOPE));
	m_Envelope.SetRange(0,256);
	
	m_RandomPanning.Attach(GetDlgItem(IDC_RPAN));
	m_RandomVCFCuttoff.Attach(GetDlgItem(IDC_RCUT));
	m_RandomVCFResonance.Attach(GetDlgItem(IDC_RRES));
	
	m_SampleAssignEditor.SubclassWindow(GetDlgItem(IDC_SAMPLE_ASSIGN_EDITOR));
	//m_SampleAssignEditor.Create(m_hWnd, rcDefault, NULL,
    //        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
    //        WS_EX_CLIENTEDGE);
	m_SampleAssignEditor.Initialize(m_pMachine,0);

	//m_SampleAssignEditor.Initialize(m_pMachine);

	m_EnvelopeEditor.SubclassWindow(GetDlgItem(IDC_XM_EDIT_ENVELOPE));
	m_EnvelopeEditor.Initialize(m_pMachine,m_pMachine->Instrument(0).AmpEnvelope());


// 
	m_InstNoUpDown.Attach(GetDlgItem(IDC_SPIN_INST));
	m_InstNoUpDown.SetBuddy(m_InstNo);
	m_InstNoUpDown.SetRange32(0,255);
	m_InstNoUpDown.SetPos(0);
	m_InstNoUpDown.SetBase(10);
	
	m_CutoffLabel.Attach(GetDlgItem(IDC_CUTOFF_LBL));
	m_QLabel.Attach(GetDlgItem(IDC_Q_LBL));
	m_EnvelopeLabel.Attach(GetDlgItem(IDC_ENV_LBL));
	m_PanLabel.Attach(GetDlgItem(IDC_PAN_LBL));
	m_ModeLabel.Attach(GetDlgItem(IDC_MODE_LBL));

	SetInstrumentData(0);

	m_bInitialized = true;
	UpdateWindow();
	return 0;
}

void XMSamplerUIInst::SetInstrumentData(const int instno)
{
	m_bInitialized = false;
	XMInstrument& _inst = m_pMachine->Instrument(instno);
	
	m_InstName.SetWindowText(_inst.Name().c_str());
	//m_InstName.SetWindowText(_T("Test"));
	
	SetNNACombo(_inst.NNA());
	
	m_FilterType.SetPos(4 - _inst.FilterType());
	
	//SetFilterModeCombo(_inst.FilterType());

	m_Pan.SetPos((int)(_inst.Pan() * 100.0f));
	
	m_Cutoff.SetPos(_inst.FilterCutoff());
	m_Q.SetPos(_inst.FilterResonance());
	
	//m_Envelope.SetPos(_inst.EnSetRange(0,256);
	
	m_RandomPanning.SetCheck(_inst.IsRandomPanning());
	m_RandomVCFCuttoff.SetCheck(_inst.IsRandomCutoff());
	m_RandomVCFResonance.SetCheck(_inst.IsRandomResonance());
	m_CurrentInstNo = instno;

	m_bInitialized = true;
}

void XMSamplerUIInst::SetNNACombo(const int nna)
{
	switch(nna)
	{
	case 0:m_NNA.SelectString(0,_T("Note ｶｯﾄ"));break;
	case 1:m_NNA.SelectString(0,_T("Note ﾘﾘｰｽ"));break;
	case 2:m_NNA.SelectString(0,_T("無し"));break;
	}
}

//void XMSamplerUIInst::SetFilterModeCombo(const int mode)
//{
//	switch(mode)
//	{
//	case 0:
//		m_FilterType.SelectString(0,_T("LPF"));
//		break;
//	case 1:
//		m_FilterType.SelectString(0,_T("HPF"));
//		break;
//	case 2:
//		m_FilterType.SelectString(0,_T("BPF"));
//		break;
//	case 3:
//		m_FilterType.SelectString(0,_T("NBF"));
//		break;
//	case 4:
//		m_FilterType.SelectString(0,_T("OFF"));
//		break;
//	}
//
//}

void XMSamplerUIInst::OnChangeInstNo(UINT id,int command,HWND hwnd)
{
	
	TCHAR _buffer[5];

	if(m_bInitialized){
		m_InstNo.GetWindowText(_buffer,4);
	
		int _instNo = boost::lexical_cast<int>(_buffer);
	
		if(_instNo > (XMSampler::MAX_INSTRUMENT - 1))
		{
			_instNo = 255;
			m_InstNo.SetWindowText((boost::lexical_cast<SF::string>(_instNo)).c_str());
		}

		if(_instNo < 0){
			_instNo = 0;
			m_InstNo.SetWindowText((boost::lexical_cast<SF::string>(_instNo)).data());
		}

		SetInstrumentData(_instNo);
		m_EnvelopeEditor.Initialize(m_pMachine,m_pMachine->Instrument(_instNo).AmpEnvelope());
		m_SampleAssignEditor.Initialize(m_pMachine,_instNo);
		
		m_CurrentInstNo = _instNo;
	}

}

void XMSamplerUIInst::OnChangeInstName(UINT id,int command,HWND hwnd)
{
	if(m_bInitialized){
		XMInstrument& _inst = m_pMachine->Instrument(m_CurrentInstNo);
		TCHAR _buf[256];
		m_InstName.GetWindowText(_buf,sizeof(_buf));
		_inst.Name(SF::string(_buf));
	}
}

LRESULT XMSamplerUIInst::OnCustomdrawSliderCutoff(NMHDR* pNMHDR) 
{
	XMInstrument& _inst = m_pMachine->Instrument(m_CurrentInstNo);

	_inst.FilterCutoff(m_Cutoff.GetPos());

	m_CutoffLabel.SetWindowText((boost::lexical_cast<SF::string>(_inst.FilterCutoff())).c_str());

	return TRUE;

}// XMSamplerUIInst::OnCustomdrawSliderCutoff

LRESULT XMSamplerUIInst::OnCustomdrawSliderQ(NMHDR* pNMHDR) 
{
	XMInstrument& _inst = m_pMachine->Instrument(m_CurrentInstNo);

	_inst.FilterResonance(m_Q.GetPos());

	//_stprintf(buffer,_T("%.0f%%"),_pSong->pInstrument(si)->ENV_F_RQ * 0.78740);
	m_QLabel.SetWindowText
		((SF::format(_T("%.0f%%")) % ((float)m_Q.GetPos() * 0.78740f )).str().c_str());

	return TRUE;
}


LRESULT XMSamplerUIInst::OnCustomdrawEnvelope(NMHDR* pNMHDR) 
{
	XMInstrument& _inst = m_pMachine->Instrument(m_CurrentInstNo);
	_inst.FilterEnvAmount(m_Envelope.GetPos() - 128);
	m_EnvelopeLabel.SetWindowText
		((SF::format(_T("%.0f")) % ((float)_inst.FilterEnvAmount() * 0.78125f )).str().c_str());
	return TRUE;
}// XMSamplerUIInst::OnCustomdrawEnvelope

LRESULT XMSamplerUIInst::OnCustomdrawPan(NMHDR* pNMHDR)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	ATLTRACE2(_T("OnCustomdrawPan"));
	switch(pNMCD->dwDrawStage)
	{
	case CDDS_PREPAINT:
		ATLTRACE2(_T("CDDS_REPAINT\n"));
		return 	CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		ATLTRACE2(_T("CDDS_ITEMPREPAINT\n"));
		if (pNMCD->dwItemSpec == TBCD_THUMB)
		{
			ATLTRACE2(_T("TBCD_THUMB\n"));
			return CDRF_NOTIFYPOSTPAINT;
		}

		return CDRF_DODEFAULT;
		break;
	case CDDS_ITEMPREERASE:
		ATLTRACE2(_T("CDDS_ITEMPREERASE\n"));
		break;

	}
	ATLTRACE2((SF::tformat(_T("%d %d %d %d %d \n")) % pNMCD->dwItemSpec % pNMCD->rc.top % pNMCD->rc.left % pNMCD->rc.right % pNMCD->rc.bottom).str().c_str());
	XMInstrument& _inst = m_pMachine->Instrument(m_CurrentInstNo);
	
	_inst.Pan((float)m_Pan.GetPos() * 0.01f);
	m_PanLabel.SetWindowText
		((SF::format(_T("%d%%")) % ((float)_inst.Pan() * 100.0f)).str().c_str());
	SetMsgHandled(true);
	return CDRF_DODEFAULT;
//	return CDRF_SKIPDEFAULT;

}//XMSamplerUIInst::OnCustomdrawPan

LRESULT XMSamplerUIInst::OnCustomdrawMode(NMHDR* pNMHDR)
{
	XMInstrument& _inst = m_pMachine->Instrument(m_CurrentInstNo);
	
	_inst.FilterType(4 - m_FilterType.GetPos());

	SF::string _mode_str;
	switch(_inst.FilterType())
	{
	case 0:
		_mode_str = _T("LPF");
		break;
	case 1:
		_mode_str = _T("HPF");
		break;
	case 2:
		_mode_str = _T("BPF");
		break;
	case 3:
		_mode_str = _T("NBF");
		break;
	case 4:
		_mode_str = _T("OFF");
		break;
	}

	m_ModeLabel.SetWindowText(_mode_str.c_str());

	return TRUE;
}//XMSamplerUIInst::OnCustomdrawMode

//////////////////////////////////////////////////////////////////////////////
// EnvelopeEditor ------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
XMSamplerUIInst::EnvelopeEditor::EnvelopeEditor() : m_Color(255, 0, 0, 255)
{
	m_bInitialized = false;
	m_bPointEditing = false;
	
	m_pEnvelope = NULL;
	m_pXMSampler = NULL;

	// ダミーのウィンドウクラスの登録
	// SubclassWindowが呼ばれるまではこのウィンドウプロシージャで処理する
	::WNDCLASSEX _wc = GetWndClassInfo().m_wc;
	_wc.lpfnWndProc = &DummyWindowProc;
	::RegisterClassEx(&_wc);
}// XMSamplerUIInst::EnvelopeEditor::EnvelopeEditor

void XMSamplerUIInst::EnvelopeEditor::Initialize(XMSampler * const pSampler,XMInstrument::Envelope * const pEnvelope)
{
	m_bInitialized = false;
	// 
	WTL::CRect _rect_client;

	//スクロールバー表示の調整
	GetClientRect(&_rect_client);
	// H ScrollBarをいったん表示させる
	SetScrollSize(_rect_client.Width() * 2,_rect_client.Height(),false);
	// ここではまだclientのY size は変わらない
	GetClientRect(&_rect_client);
	// H ScrollBarを使用不可にする
	SetScrollSize(_rect_client.Width(),_rect_client.Height(),false);
	// ここでclientのY sizeが変わる
	GetClientRect(&_rect_client);
	// その値を再度セット
	SetScrollSize(_rect_client.Width(),_rect_client.Height(),false);
	// 終わり．．．なんでこうしなくちゃうまくいかないのかは不明

	//LPVOID lpMsgBuf;
	//if (_ret == 0 && FormatMessage( 
 //   FORMAT_MESSAGE_ALLOCATE_BUFFER | 
 //   FORMAT_MESSAGE_FROM_SYSTEM | 
 //   FORMAT_MESSAGE_IGNORE_INSERTS,
 //   NULL,
 //   GetLastError(),
 //   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
 //   (LPTSTR) &lpMsgBuf,
 //   0,
 //   NULL ))
	//{
	//	MessageBox( (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	//	::LocalFree(lpMsgBuf);
	//}

	m_pXMSampler = pSampler;
	m_CurrentScrollWidth = 0;
	m_CurrentScrollHeight = 0;
	EditEnvelope(pEnvelope);

	m_Zoom = 0.005f;

	//ShowScrollBar(SB_HORZ);
	//EnableScrollBar(SB_HORZ);

	//if(m_pEnvelope->IsEnabled()){
	
	int _num_pts = m_pEnvelope->NumOfPoints();
	int _max = 0;

	//if(_num_pts > 0){
	//	_scroll_size.cx = (int)(m_Zoom *  (float)(pEnvelope->Point(_num_pts - 1)));
	//	_scroll_size.cx += 100 /* pixel */; 
	//} else {

	//	_scroll_size.cx = _rect_client.right;
	//}

	//_scroll_size.cy = _rect_client.bottom; 
	//
	//m_CurrentScrollWidth = _scroll_size.cx;
	//m_CurrentScrollHeight = _scroll_size.cy;
	
	//SetScrollSize(_scroll_size);
///*
	
	if(_num_pts > 0){
		AdjustScrollRect((int)(m_Zoom *  (float)(m_pEnvelope->Point(_num_pts - 1))));
	} else {

		m_CurrentScrollWidth = _rect_client.Width();
		m_CurrentScrollHeight = _rect_client.Height();
		
		SetScrollSize(_rect_client.Width(),_rect_client.Height(),false);
	}
  
		//SetScrollLine(1,1);
		//SetScrollPage(1,1);
		//ShowScrollBar(SB_HORZ,TRUE);


//*/
		
		//::SCROLLINFO _info;
	/*
		ZeroMemory(&_info,sizeof(::SCROLLINFO));
		_info.cbSize = sizeof(SCROLLINFO);
		_info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS ;
		_info.nMin = 0;
		_info.nMax = 2048;
		_info.nPage = 2;
		_info.nPos = 0;*/


		//this->SetScrollInfo(SB_HORZ,&_info);
		
		//m_pEnvelope = pEnvelope;

		//if(!m_bInitialized)
		//{
		//	 テクスチャを読み込む
		//	LPDIRECT3DDEVICE9 const _pDevice = D3D::GetInstance().pD3DDevice();
		//	HRESULT hr = S_OK;

		//	hr = ::D3DXCreateTextureFromFile(_pDevice, "c:\\test.png", &m_pTexture);
		//	hr = ::D3DXCreateLine(_pDevice,&m_pLine);
		//	 lineのstyleを設定する
		//	m_pLine->SetAntialias(TRUE);
		//	m_pLine->SetGLLines(TRUE);
		//	m_pLine->SetPattern(1);
		//	m_pLine->SetWidth(2.0f);
		//	m_pLine->SetPatternScale(0.25f);
		//}

		//m_Vect[0].x = 0.0f;
		//m_Vect[0].y = 0.0f;
		//m_Vect[1].x = 1024.0f;
		//m_Vect[1].y = 768.0f;
		m_bInitialized = true;
	//}

	Invalidate();
	//OnPaint(NULL);
}// XMSamplerUIInst::EnvelopeEditor::Initialize

	
	void XMSamplerUIInst::EnvelopeEditor::EditEnvelope(XMInstrument::Envelope * const pEnvelope)
	{
		m_pEnvelope = pEnvelope;
	}

	/** エンベロープエディタを描画します。 
	 * @param CDCHandle Device Context */
	void XMSamplerUIInst::EnvelopeEditor::DoPaint(CDCHandle dc)
	{

		if(m_bInitialized){
			
			// 描画用 HDCの取得
			HDC hdc = dc.m_hDC;
			CPoint _pt_viewport_org;
			dc.GetViewportOrg(&_pt_viewport_org);// 現在の
			WTL::CRect _rect;
			GetClientRect(&_rect);
	
			Graphics graphics(hdc);

			Pen      _line_pen(m_Color,1.0);
			Pen      _gridpen(Gdiplus::Color(32, 0, 0, 255),1.0);
			Pen      _gridpen1(Gdiplus::Color(32, 255, 0, 0),1.0);
			Pen      _gridpen2(Gdiplus::Color(64, 255, 0, 0),1.0);
			
			Gdiplus::SolidBrush  brush(Gdiplus::Color(255, 255, 255, 255));

			//graphics.DrawLine(&_line_pen, 0, 0, 255, 255);
			const int _points =  m_pEnvelope->NumOfPoints();


			// 画面をいったんクリアする
			
			graphics.Clear(Gdiplus::Color(255,255,255,255));
			graphics.SetRenderingOrigin(_pt_viewport_org.x,_pt_viewport_org.y);
			
			// Draw Grid
			
			Gdiplus::SolidBrush  _fontbrush(Gdiplus::Color(255, 0, 0, 255));
			Gdiplus::FontFamily  fontFamily(L"Times New Roman");
			Gdiplus::Font        font(&fontFamily, 8, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
			Gdiplus::PointF      pointF(10.0f, 20.0f);
			
			// 文字列をAnti Aliasに
			graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
			
			const REAL _stepx = (REAL)(4410.0 * m_Zoom );
			const REAL _width = (m_CurrentScrollWidth > _rect.right)?m_CurrentScrollWidth:(REAL)_rect.right;
			int _mod = 0;
			float _sec = 0.0f;
			
			std::wstring _str;

			// X方向のグリッド
			if(_stepx >= 4.0){
				for(REAL i = 0.0; i < _width; i += _stepx)
				{
					if(_mod == 5)
					{
						graphics.DrawLine(&_gridpen1,i,0.0,i,(REAL)m_CurrentScrollHeight);
						_mod++;

						// ラベル描画
						_sec += 0.5f;
						_str = boost::lexical_cast<std::wstring,float>(_sec);		
						graphics.DrawString(_str.c_str(),_str.length(),&font,PointF(i,0),&_fontbrush);

					} else if(_mod == 10) {
						_mod = 1;
						graphics.DrawLine(&_gridpen2,i,0.0,i,(REAL)m_CurrentScrollHeight);
						// ラベル描画
						_sec += 0.5f;
						_str = boost::lexical_cast<std::wstring,float>(_sec);		
						graphics.DrawString(_str.c_str(),_str.length(),&font,PointF(i,0),&_fontbrush);
					} else {
						graphics.DrawLine(&_gridpen,i,0.0,i,(REAL)m_CurrentScrollHeight);
						_mod++;
					}
				}
			}

			// Y 方向のグリッド
			REAL _stepy = ((REAL)(m_CurrentScrollHeight)) / 100.0f * 10.0f;
			
			for(REAL i = (REAL)_rect.top; i <= (REAL)m_CurrentScrollHeight; i += _stepy)
			{
				graphics.DrawLine(&_gridpen,0.0,i,_width,i);
			}

			
			Gdiplus::Point _pt_start(0,m_CurrentScrollHeight);
			Gdiplus::SolidBrush  _point_brush(Gdiplus::Color(255, 255, 128, 128));
			Gdiplus::SolidBrush  _point_edit_brush(Gdiplus::Color(255, 128, 64, 64));
			Pen _point_pen(Gdiplus::Color(128, 255, 128, 255),1.0);
			Pen _point_edit_pen(Gdiplus::Color(128, 255, 128, 255),1.0);

			graphics.SetSmoothingMode(SmoothingModeAntiAlias);
			
			// エンベロープ間のライン描画
			
			for(int i = 0;i < _points ;i++)
			{
				Point _pt_end;
				_pt_end.Y = (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(i)));
				_pt_end.X = (int)(m_Zoom * (float)m_pEnvelope->Point(i)); 
				graphics.DrawLine(&_line_pen,_pt_start,_pt_end);
				_pt_start = _pt_end;
		
			}
			

			// 編集中のライン描画
			if(m_bPointEditing){// ポイントをドラッグ中の場合
				_pt_start.X = 0;
				_pt_start.Y = m_CurrentScrollHeight;
				// エンベロープ間のライン描画
				for(int i = 0;i < _points ;i++)
				{
					Point _pt_end;
					_pt_end.Y = (int)((float)m_CurrentScrollHeight * (1.0f - m_EnvelopeEditing.Value(i)));
					_pt_end.X = (int)(m_Zoom * (float)m_EnvelopeEditing.Point(i)); 
					graphics.DrawLine(&_point_edit_pen,_pt_start,_pt_end);
					_pt_start = _pt_end;
				}
			}
			
			// エンベロープポイントの描画
			for(int i = 0;i < _points ;i++)
			{
				Point _pt(
					/* X */ (int)(m_Zoom * (float)m_pEnvelope->Point(i)), 
					/* Y */ (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(i)))
				);
				graphics.FillRectangle(&_point_brush,_pt.X - POINT_SIZE / 2,_pt.Y - POINT_SIZE / 2,POINT_SIZE,POINT_SIZE);
			}
			
			// 編集中のポイントを描画する
			
			if(m_bPointEditing)
			{
				Point _pt_org((int)(m_Zoom * (float)m_pEnvelope->Point(m_EditPointOrig)),(int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(m_EditPointOrig))));
				Point _pt_edit(m_EditPointX,m_EditPointY);
				Pen _edit_line_pen(Gdiplus::Color(64, 64, 64, 64),1.0);
				REAL dashValues[2] = {2,2};
				_edit_line_pen.SetDashPattern(dashValues, 2);
				graphics.DrawLine(&_edit_line_pen,_pt_org,_pt_edit);
				
				graphics.FillRectangle(&_point_brush,_pt_edit.X - POINT_SIZE / 2,_pt_edit.Y - POINT_SIZE / 2,POINT_SIZE,POINT_SIZE);
				graphics.FillRectangle(&_point_edit_brush,_pt_org.X - POINT_SIZE / 2,_pt_org.Y - POINT_SIZE / 2,POINT_SIZE,POINT_SIZE);

			}
			
			// Sustain Pointを描画する

			if(m_pEnvelope->SustainBegin() != XMInstrument::Envelope::INVALID)
			{
				Point _pt_st(
					/* X */(int)(m_Zoom * (float)m_pEnvelope->Point(m_pEnvelope->SustainBegin())), 
					/* Y */(int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(m_pEnvelope->SustainBegin())))
				);

				Point _pt_st_lbl;

				if(_pt_st.X > m_CurrentScrollWidth / 2)
				{
					_pt_st_lbl.X = _pt_st.X  - 5;
				} else {
					_pt_st_lbl.X = _pt_st.X  + 5;
				}
				
				if(_pt_st.Y < m_CurrentScrollHeight / 8)
				{
					_pt_st_lbl.Y = _pt_st.Y + m_CurrentScrollHeight / 8;
				} else {
					/* 3 / 4*/
					_pt_st_lbl.Y = _pt_st.Y - m_CurrentScrollHeight / 8 ;
				}


				// Envelope Point から Sustain Labelまで破線を引く
				Pen _edit_line_pen(Gdiplus::Color(96, 128, 128, 64),1.0);
				REAL dashValues[2] = {2,2};
				_edit_line_pen.SetDashPattern(dashValues, 2);

				graphics.DrawLine(&_edit_line_pen,_pt_st,_pt_st_lbl);
				
				// ラベルを描画する 
				Gdiplus::FontFamily  _font_family_sustain(L"Times New Roman");
				Gdiplus::Font        _font_sustain(&_font_family_sustain, 10, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
				Gdiplus::SolidBrush  _font_sustain_brush(Gdiplus::Color(255, 255, 128, 64));
			
				graphics.DrawString(L"Sustain",7,&_font_sustain,PointF((float)_pt_st_lbl.X,(float)_pt_st_lbl.Y - 3),&_font_sustain_brush);
	
			}
			
			// Loop StartとLoop End を描画する

			if(m_pEnvelope->LoopStart() != XMInstrument::Envelope::INVALID && 
				m_pEnvelope->LoopEnd() != XMInstrument::Envelope::INVALID)
			{
				const int _pt_loop_start_x = m_Zoom * (float)m_pEnvelope->Point(m_pEnvelope->LoopStart());
				const int _pt_loop_start_y = (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(m_pEnvelope->LoopStart())));

				const int _pt_loop_end_x = m_Zoom * (float)m_pEnvelope->Point(m_pEnvelope->LoopEnd());
				const int _pt_loop_end_y = (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(m_pEnvelope->LoopEnd())));

				// Envelope Point から Sustain Labelまで破線を引く
				Pen _loop_pen(Gdiplus::Color(255, 0, 192, 0),1.0);
				REAL dashValues[2] = {2,2};
				_loop_pen.SetDashPattern(dashValues, 2);

				Gdiplus::SolidBrush  _loop_brush(Gdiplus::Color(64, 0, 128, 0));
				graphics.FillRectangle(&_loop_brush,_pt_loop_start_x,0,_pt_loop_end_x - _pt_loop_start_x,m_CurrentScrollHeight);

				graphics.DrawLine(&_loop_pen,_pt_loop_start_x,0,_pt_loop_start_x,m_CurrentScrollHeight);
				graphics.DrawLine(&_loop_pen,_pt_loop_end_x,0,_pt_loop_end_x,m_CurrentScrollHeight);
			
				Gdiplus::FontFamily  _font_family_loop(L"Times New Roman");
				Gdiplus::Font        _font_loop(&_font_family_loop, 10, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
				Gdiplus::SolidBrush  _font_loop_brush(Gdiplus::Color(255, 0, 64, 0));

				graphics.DrawString(L"Loop",4,&_font_loop,
					PointF(	(float)((_pt_loop_end_x - _pt_loop_start_x) / 2 + _pt_loop_start_x - 20),
							(float)(m_CurrentScrollHeight / 2)),&_font_loop_brush);

			}

//			Gdiplus::SolidBrush  _fontbrush(Gdiplus::Color(255, 0, 0, 255));
			
			//Gdiplus::SolidBrush  brush(Gdiplus::Color(255, 0, 0, 255));
			//Gdiplus::FontFamily  fontFamily(L"Times New Roman");
			//Gdiplus::Font        font(&fontFamily, 24, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
			//Gdiplus::PointF      pointF(10.0f, 20.0f);
			//graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
	// 		graphics.DrawString(L"Hello World!", -1, &font, pointF, &brush);

	//		XMInstrument& _inst = m_pMachine->Instrument(instno);

			//LPDIRECT3DDEVICE9 const _pDevice = D3D::GetInstance().pD3DDevice();

	//		_pDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
	//								0x00000000, 1.0f, 0L );
	//		_pDevice->BeginScene();
	//		D3D::GetInstance().Render(*this);
	//		m_pLine->Begin();
	//
	//		HRESULT hr = _pDevice->SetTexture(0, m_pTexture);	//テクスチャのセット
	//
	//		//テクスチャステージステート
	//		hr = _pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	////		hr = _pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
	//		hr = _pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	//		hr = _pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTSS_COLOROP);
	//		hr = _pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	//		hr = _pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_ADD);
	//		hr = _pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	//		hr = _pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	//		hr = _pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
	//		hr = _pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE);
	//		RECT _rect,_rect1;
	//		GetClientRect(&_rect);
	//
	//		m_Vect[1].x = _rect.right;
	//		m_Vect[1].y = _rect.bottom;
	//		
	//		m_pLine->Draw(m_Vect,2,D3DCOLOR(0xffffffff));
	//		
	//		m_pLine->End();
	//		_pDevice->EndScene();
	//
	//		_rect1 = _rect;

	//		_rect1.top += 1.0f;
	//		_rect1.bottom += 1.0f;
			
	//		_rect1.left += 2.0f;
	//		_rect1.right -= 6.0f;

			//D3D::GetInstance().Window().GetClientRect(&_rect1);
	//		_pDevice->Present(&_rect1,NULL,m_hWnd,NULL);
	//		_pDevice->Present(NULL,NULL,m_hWnd,NULL);
			// EndPaint(&ps);
		}
		SetMsgHandled(false);
	}// XMSamplerUIInst::EnvelopeEditor::OnPaint

	/** マウスの左ボタンクリック時の処理を行う
	 * @param vKey 同時に押された仮想キーコード
	 * @param position マウスカーソルの位置 */
	void XMSamplerUIInst::EnvelopeEditor::OnLButtonDown(const UINT vKey, WTL::CPoint position)
	{
		
		SetFocus();

		if(!m_bPointEditing){
			const int _points =  m_pEnvelope->NumOfPoints();
			WTL::CRect _rect;
			GetClientRect(&_rect);
			POINT _offset;
			GetScrollOffset(_offset);
			position.x += _offset.x;
			position.y += _offset.y;

			// エンベロープポイントをクリックしたのか判定する
			EnvelopePointIndex _edit_point = GetEnvelopePointIndexAtPoint(position.x,position.y);
			if(_edit_point)
			{
				m_bPointEditing = true;
				SetCapture();
				m_EditPointOrig = m_EditPoint = *_edit_point;
				m_EnvelopeEditing = *m_pEnvelope;

			}
		}

	}
	
	/** マウスの左ボタンを離した時の処理を行う
	 * @param vKey 同時に押された仮想キーコード
	 * @param position マウスカーソルの位置 */
	void XMSamplerUIInst::EnvelopeEditor::OnLButtonUp(const UINT vKey, WTL::CPoint position)
	{
		if(m_bPointEditing){
			ReleaseCapture();
			m_bPointEditing =  false;
			CPoint _offset;
			GetScrollOffset(_offset);
				
			position.x += _offset.x;
			position.y += _offset.y;
			
			int _new_point = (int)((float)position.x / m_Zoom);
			float _new_value = (1.0f - (float)position.y / (float)m_CurrentScrollHeight);
			
			if(_new_value > 1.0f)
			{
				_new_value = 1.0f;
			}

			if(_new_value < 0.0f)
			{
				_new_value = 0.0f;
			}

			if( _new_point < 0)
			{
				_new_point = 0;
			}

			m_EnvelopeEditing.PointAndValue(m_EditPoint,_new_point,_new_value);

			*m_pEnvelope = m_EnvelopeEditing;
			
			if(m_pEnvelope->NumOfPoints() > 0){
				AdjustScrollRect((int)(m_Zoom *  (float)(m_pEnvelope->Point(m_pEnvelope->NumOfPoints() - 1))));
			}

			Invalidate();
		}
	}

	void XMSamplerUIInst::EnvelopeEditor::OnMouseMove(const UINT vKey, WTL::CPoint position)
	{
		if(m_bPointEditing)
		{
			CPoint _offset;
			GetScrollOffset(_offset);
			
			m_EditPointX = position.x + _offset.x;
			m_EditPointY = position.y + _offset.y;
			m_EditPoint = m_EnvelopeEditing.PointAndValue(m_EditPoint,(int)((float)m_EditPointX / m_Zoom),
				(1.0f - (float)m_EditPointY / (float)m_CurrentScrollHeight));

			//m_pEnvelope->Value(m_EditPoint,	(1.0f - (float)position.y / (float)m_CurrentScrollHeight));
			//_pt.x = (int)(m_Zoom * (float)m_pEnvelope->Point(i));
			//m_pEnvelope->Point(m_EditPoint, (int)((float)position.x / m_Zoom));

			Invalidate();

		}
	}

	/** Scroll Size を調節する
	* 指定した座標(maxPoint)が、現在のScroll幅 + 100より広い場合はScroll幅をmaxPoint + 100にAdjustする　
	* 指定した座標(maxPoint)が、現在のScroll幅 - 100より狭い場合はScroll幅をmaxPoint + 100にAdjustする*/
	void XMSamplerUIInst::EnvelopeEditor::AdjustScrollRect(const int maxPoint)
	{
		
		bool _bchanged = false;
		SIZE _scroll_size;
		CPoint _pt_scroll_offset;
		GetScrollSize(_scroll_size);
		GetScrollOffset(_pt_scroll_offset);

		WTL::CRect _rect;
		GetClientRect(&_rect);
	
		if((maxPoint + MARGIN_RIGHT) > _scroll_size.cx){
			_scroll_size.cx = maxPoint;
			_scroll_size.cx += MARGIN_RIGHT /* pixel */;
			if(_scroll_size.cx < _rect.Width()){
				_scroll_size.cx = _rect.Width();
			}
			_bchanged = true;
		} else if(maxPoint < (_scroll_size.cx - MARGIN_RIGHT))
		{
			_scroll_size.cx = maxPoint + MARGIN_RIGHT;
			if(_scroll_size.cx < _rect.Width())
			{
				_scroll_size.cx = _rect.Width();
			}
			_bchanged = true;
		}

		if(_bchanged){

			GetClientRect(_rect);

			if(_scroll_size.cy < _rect.Height()){
				_scroll_size.cy = _rect.Height();
			}
			
			m_CurrentScrollWidth = _scroll_size.cx;
			m_CurrentScrollHeight = _scroll_size.cy;

			SetScrollSize(_scroll_size.cx,_scroll_size.cy,false);
			// オフセットの調節
			if(_scroll_size.cx > _rect.Width())
			{
				SetScrollOffset(_pt_scroll_offset,false);
			}
			Invalidate();

		}

	}

	/** 右ボタンクリックした時の処理 */
	void XMSamplerUIInst::EnvelopeEditor::OnRButtonDown(const UINT vKey, WTL::CPoint position)
	{
		if(!m_bPointEditing){
			// コンテキストコマンドの処理用に座標を保存する
			m_EditPointX = position.x;
			m_EditPointY = position.y;
			
			CPoint _offset;
			GetScrollOffset(_offset);

			m_EditPointX += _offset.x;
			m_EditPointY += _offset.y;
	        
			// 右クリックした座標をスクリーン座標に変換
			ClientToScreen(&position);
	        
			// ポップアップメニューリソースのロードと表示
			CMenu _menuPopup;
			_menuPopup.LoadMenu(IDR_MENU_ENV_EDIT);
			// エンベロープポイントをクリックしたのか判定する
			EnvelopePointIndex _edit_point = GetEnvelopePointIndexAtPoint(m_EditPointX,m_EditPointY);
			if(!_edit_point)
			{
				_menuPopup.EnableMenuItem(ID_SET_SUSTAIN,MF_GRAYED);
				_menuPopup.EnableMenuItem(ID_SET_LOOPSTART,MF_GRAYED);
				_menuPopup.EnableMenuItem(ID_SET_LOOPEND,MF_GRAYED);
				_menuPopup.EnableMenuItem(ID_DEL_POINT,MF_GRAYED);
			}

			_menuPopup.GetSubMenu(0).TrackPopupMenu(
				TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, position.x, position.y, m_hWnd);
		}

	}

    void XMSamplerUIInst::EnvelopeEditor::OnAddPoint(const UINT uNotifyCode,const int nID,const  HWND hWndCtl)
	{
		if(m_pEnvelope->NumOfPoints() > XMInstrument::Envelope::MAX_POINT)
		{
			// エラーメッセージ 
			return;
		}

		CPoint _offset;
		GetScrollOffset(_offset);
			
		m_EditPointX += _offset.x;
		m_EditPointY += _offset.y;
		
		int _new_point = (int)((float)m_EditPointX / m_Zoom);
		float _new_value = (1.0f - (float)m_EditPointY / (float)m_CurrentScrollHeight);
		
		if(_new_value > 1.0f)
		{
			_new_value = 1.0f;
		}

		if(_new_value < 0.0f)
		{
			_new_value = 0.0f;
		}

		if( _new_point < 0)
		{
			_new_point = 0;
		}

		boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->Insert(_new_point,_new_value);
		Invalidate();

	}

    /** Envelope Point を削除する 
	* @param uNotifyCode 通知コード
	* メッセージがコントロールから送られている場合には、コントロールからの通知コードが指定されます。
	* アクセラレータから送られた場合には 1 が指定されます。メニューから送られた場合は 0 が指定されます。
	* @param nID コントロールID
	* メニューアイテム、コントロール、アクセラレーターの ID が指定されます。
	* @param hWndCtl コントロールのウィンドウハンドル
	* 
	*/
	void XMSamplerUIInst::EnvelopeEditor::OnDelPoint(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
	{
		if(m_pEnvelope->NumOfPoints() == 0)
		{
			// エラーメッセージ 
			return;
		}

		const int _points =  m_pEnvelope->NumOfPoints();
		WTL::CRect _rect;
		GetClientRect(&_rect);

		EnvelopePointIndex _delete_point = GetEnvelopePointIndexAtPoint(m_EditPointX,m_EditPointY);
		if(_delete_point)
		{
			boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
			m_pEnvelope->Delete(*_delete_point);
			Invalidate();
		}

	}


    /** サステイン設定 */
	void XMSamplerUIInst::EnvelopeEditor::OnSetSustain(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
	{
		ATLTRACE2("OnSetSustain\n");

		EnvelopePointIndex _sustain_index = GetEnvelopePointIndexAtPoint(m_EditPointX,m_EditPointY);
		if(_sustain_index)
		{
			boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
			m_pEnvelope->SustainBegin(*_sustain_index);
			m_pEnvelope->SustainEnd(*_sustain_index);
			Invalidate();
		}

	}

	/** サスティン設定解除 */ 
	void XMSamplerUIInst::EnvelopeEditor::OnRemoveSustain(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
	{
		ATLTRACE2("OnRemoveSustain\n");
		boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->SustainBegin(XMInstrument::Envelope::INVALID);
		m_pEnvelope->SustainEnd(XMInstrument::Envelope::INVALID);
		Invalidate();

	}
	/** ループ開始位置設定 */   
	void XMSamplerUIInst::EnvelopeEditor::OnSetLoopStart(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
	{
		ATLTRACE2("OnSetLoopStart\n");

		EnvelopePointIndex _loop_index = GetEnvelopePointIndexAtPoint(m_EditPointX,m_EditPointY);
		if(_loop_index)
		{
			boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
			m_pEnvelope->LoopStart(*_loop_index);
			Invalidate();
		}

	}

    /** ループ終了位置設定 */
	void XMSamplerUIInst::EnvelopeEditor::OnSetLoopEnd(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
	{
		ATLTRACE2("OnSetLoopEnd\n");

		EnvelopePointIndex _loop_index = GetEnvelopePointIndexAtPoint(m_EditPointX,m_EditPointY);
		if(_loop_index)
		{
			boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
			m_pEnvelope->LoopEnd(*_loop_index);
			Invalidate();
		}

	}

	/** Loop設定解除 */
	void XMSamplerUIInst::EnvelopeEditor::OnRemoveLoop(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
	{
		ATLTRACE2("OnRemoveLoop\n");
		boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->LoopStart(XMInstrument::Envelope::INVALID);
		m_pEnvelope->LoopEnd(XMInstrument::Envelope::INVALID);
		Invalidate();

	}

	/** ダミーのウィンドウプロシージャ 
	 * @brief Envelope Editor がDialog API により作成され、Application よりサブクラス化されるまでに使用される
	 */
	LRESULT CALLBACK XMSamplerUIInst::EnvelopeEditor::DummyWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	//	if(msg == WM_CREATE){
	//		::ShowScrollBar(hWnd,SB_HORZ,TRUE);
	//		RECT _rect;
	//		::GetClientRect(hWnd,&_rect);
	//		SCROLLINFO si = { sizeof(si), SIF_ALL | SIF_DISABLENOSCROLL, 0,_rect.right - _rect.left ,0,0,0};
	//	

	//		int _ret = ::SetScrollInfo(hWnd,SB_HORZ,&si,TRUE);
	//		int _ret1 = ::ShowScrollBar(hWnd,SB_HORZ,TRUE);
	////			::SetScrollRange(hWnd,SB_HORZ,0,_rect.right,TRUE);
	//		return 0;
	//	}
		return ::DefWindowProc(hWnd,msg,wParam,lParam);
	}

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
			Gdiplus::SolidBrush  _font_keyname_brush(Gdiplus::Color(128, 0, 0, 0));

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
				graphics.DrawString(_tmp_str.c_str(),_tmp_str.length(),&_font_keyname,
					PointF(	(float)(i * _width + 5),98.0f),&_font_keyname_brush);
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
				graphics.DrawString(_tmp_str.c_str(),_tmp_str.length(),&_font_keyname,
					PointF(	(float)(SHARPKEY_XPOS[_index] + _width * NATURAL_KEY_PER_OCTAVE * _octave),55.0f),
					&_font_sharpkeyname_brush);

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

	void XMSamplerUIInst::SampleAssignEditor::Initialize(XMSampler* const pXMSampler)
	{
		
		m_bInitialized = false;
		GetSystemSettings();
		m_pXMSampler = pXMSampler;
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
	};

}

// XMSamplerUIInst

