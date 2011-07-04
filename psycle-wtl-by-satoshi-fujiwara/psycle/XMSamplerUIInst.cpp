// XMSamplerUIInst.cpp : XMSamplerUIInst �̎���
/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.12 $
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

	
//	m_FilterType.AddString(_T("۰�߽"));
//	m_FilterType.AddString(_T("ʲ�߽"));
//	m_FilterType.AddString(_T("������߽"));
//	m_FilterType.AddString(_T("ɯ������"));
//	m_FilterType.AddString(_T("����"));

	
	m_NNA.Attach(GetDlgItem(IDC_XM_NNA_COMBO));
	m_NNA.AddString(SF::CResourceString(IDS_MSG0046));
	m_NNA.AddString(SF::CResourceString(IDS_MSG0047));
	m_NNA.AddString(SF::CResourceString(IDS_MSG0048));

//	m_Pan.Initialize(GetDlgItem(IDC_PANSLIDER));
	m_Pan.Attach(GetDlgItem(IDC_PANSLIDER));
	m_Pan.SetRange(0,100);

//	m_Test.SubclassWindow(GetDlgItem(IDC_CUSTOM1));
//	m_Test.Initialize();
//	m_Test.Min(0.0f);
//	m_Test.Max(1.0f);
//	m_Test.Tick(0.1f);

	
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
	case 0:m_NNA.SelectString(0,SF::CResourceString(IDS_MSG0046));break;
	case 1:m_NNA.SelectString(0,SF::CResourceString(IDS_MSG0047));break;
	case 2:m_NNA.SelectString(0,SF::CResourceString(IDS_MSG0048));break;
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
			m_InstNo.SetWindowText((boost::lexical_cast<string>(_instNo)).c_str());
		}

		if(_instNo < 0){
			_instNo = 0;
			m_InstNo.SetWindowText((boost::lexical_cast<string>(_instNo)).data());
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
		_inst.Name(string(_buf));
	}
}

LRESULT XMSamplerUIInst::OnCustomdrawSliderCutoff(NMHDR* pNMHDR) 
{
	XMInstrument& _inst = m_pMachine->Instrument(m_CurrentInstNo);

	_inst.FilterCutoff(m_Cutoff.GetPos());

	m_CutoffLabel.SetWindowText((boost::lexical_cast<string>(_inst.FilterCutoff())).c_str());

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

	string _mode_str;
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

	// �_�~�[�̃E�B���h�E�N���X�̓o�^
	// SubclassWindow���Ă΂��܂ł͂��̃E�B���h�E�v���V�[�W���ŏ�������
	::WNDCLASSEX _wc = GetWndClassInfo().m_wc;
	_wc.lpfnWndProc = &DummyWindowProc;
	::RegisterClassEx(&_wc);
}// XMSamplerUIInst::EnvelopeEditor::EnvelopeEditor

void XMSamplerUIInst::EnvelopeEditor::Initialize(XMSampler * const pSampler,XMInstrument::Envelope * const pEnvelope)
{
	m_bInitialized = false;
	// 
	WTL::CRect _rect_client;

	//�X�N���[���o�[�\���̒���
	GetClientRect(&_rect_client);
	// H ScrollBar����������\��������
	SetScrollSize(_rect_client.Width() * 2,_rect_client.Height(),false);
	// �����ł͂܂�client��Y size �͕ς��Ȃ�
	GetClientRect(&_rect_client);
	// H ScrollBar���g�p�s�ɂ���
	SetScrollSize(_rect_client.Width(),_rect_client.Height(),false);
	// ������client��Y size���ς��
	GetClientRect(&_rect_client);
	// ���̒l���ēx�Z�b�g
	SetScrollSize(_rect_client.Width(),_rect_client.Height(),false);
	// �I���D�D�D�Ȃ�ł������Ȃ����Ⴄ�܂������Ȃ��̂��͕s��

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
		//	 �e�N�X�`����ǂݍ���
		//	LPDIRECT3DDEVICE9 const _pDevice = D3D::GetInstance().pD3DDevice();
		//	HRESULT hr = S_OK;

		//	hr = ::D3DXCreateTextureFromFile(_pDevice, "c:\\test.png", &m_pTexture);
		//	hr = ::D3DXCreateLine(_pDevice,&m_pLine);
		//	 line��style��ݒ肷��
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

	/** �G���x���[�v�G�f�B�^��`�悵�܂��B 
	 * @param CDCHandle Device Context */
	void XMSamplerUIInst::EnvelopeEditor::DoPaint(CDCHandle dc)
	{

		if(m_bInitialized){
			
			// �`��p HDC�̎擾
			HDC hdc = dc.m_hDC;
			CPoint _pt_viewport_org;
			dc.GetViewportOrg(&_pt_viewport_org);// ���݂�
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


			// ��ʂ���������N���A����
			
			graphics.Clear(Gdiplus::Color(255,255,255,255));
			graphics.SetRenderingOrigin(_pt_viewport_org.x,_pt_viewport_org.y);
			
			// Draw Grid
			
			Gdiplus::SolidBrush  _fontbrush(Gdiplus::Color(255, 0, 0, 255));
			Gdiplus::FontFamily  fontFamily(L"Times New Roman");
			Gdiplus::Font        font(&fontFamily, 8, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
			Gdiplus::PointF      pointF(10.0f, 20.0f);
			
			// �������Anti Alias��
			graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
			
			const REAL _stepx = (REAL)(4410.0 * m_Zoom );
			const REAL _width = (m_CurrentScrollWidth > _rect.right)?m_CurrentScrollWidth:(REAL)_rect.right;
			int _mod = 0;
			float _sec = 0.0f;
			
			std::wstring _str;

			// X�����̃O���b�h
			if(_stepx >= 4.0){
				for(REAL i = 0.0; i < _width; i += _stepx)
				{
					if(_mod == 5)
					{
						graphics.DrawLine(&_gridpen1,i,0.0,i,(REAL)m_CurrentScrollHeight);
						_mod++;

						// ���x���`��
						_sec += 0.5f;
						_str = boost::lexical_cast<std::wstring,float>(_sec);		
						graphics.DrawString(_str.c_str(),_str.length(),&font,PointF(i,0),&_fontbrush);

					} else if(_mod == 10) {
						_mod = 1;
						graphics.DrawLine(&_gridpen2,i,0.0,i,(REAL)m_CurrentScrollHeight);
						// ���x���`��
						_sec += 0.5f;
						_str = boost::lexical_cast<std::wstring,float>(_sec);		
						graphics.DrawString(_str.c_str(),_str.length(),&font,PointF(i,0),&_fontbrush);
					} else {
						graphics.DrawLine(&_gridpen,i,0.0,i,(REAL)m_CurrentScrollHeight);
						_mod++;
					}
				}
			}

			// Y �����̃O���b�h
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
			
			// �G���x���[�v�Ԃ̃��C���`��
			
			for(int i = 0;i < _points ;i++)
			{
				Point _pt_end;
				_pt_end.Y = (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(i)));
				_pt_end.X = (int)(m_Zoom * (float)m_pEnvelope->Point(i)); 
				graphics.DrawLine(&_line_pen,_pt_start,_pt_end);
				_pt_start = _pt_end;
		
			}
			

			// �ҏW���̃��C���`��
			if(m_bPointEditing){// �|�C���g���h���b�O���̏ꍇ
				_pt_start.X = 0;
				_pt_start.Y = m_CurrentScrollHeight;
				// �G���x���[�v�Ԃ̃��C���`��
				for(int i = 0;i < _points ;i++)
				{
					Point _pt_end;
					_pt_end.Y = (int)((float)m_CurrentScrollHeight * (1.0f - m_EnvelopeEditing.Value(i)));
					_pt_end.X = (int)(m_Zoom * (float)m_EnvelopeEditing.Point(i)); 
					graphics.DrawLine(&_point_edit_pen,_pt_start,_pt_end);
					_pt_start = _pt_end;
				}
			}
			
			// �G���x���[�v�|�C���g�̕`��
			for(int i = 0;i < _points ;i++)
			{
				Point _pt(
					/* X */ (int)(m_Zoom * (float)m_pEnvelope->Point(i)), 
					/* Y */ (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(i)))
				);
				graphics.FillRectangle(&_point_brush,_pt.X - POINT_SIZE / 2,_pt.Y - POINT_SIZE / 2,POINT_SIZE,POINT_SIZE);
			}
			
			// �ҏW���̃|�C���g��`�悷��
			
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
			
			// Sustain Point��`�悷��

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


				// Envelope Point ���� Sustain Label�܂Ŕj��������
				Pen _edit_line_pen(Gdiplus::Color(96, 128, 128, 64),1.0);
				REAL dashValues[2] = {2,2};
				_edit_line_pen.SetDashPattern(dashValues, 2);

				graphics.DrawLine(&_edit_line_pen,_pt_st,_pt_st_lbl);
				
				// ���x����`�悷�� 
				Gdiplus::FontFamily  _font_family_sustain(L"Times New Roman");
				Gdiplus::Font        _font_sustain(&_font_family_sustain, 10, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
				Gdiplus::SolidBrush  _font_sustain_brush(Gdiplus::Color(255, 255, 128, 64));
			
				graphics.DrawString(L"Sustain",7,&_font_sustain,PointF((float)_pt_st_lbl.X,(float)_pt_st_lbl.Y - 3),&_font_sustain_brush);
	
			}
			
			// Loop Start��Loop End ��`�悷��

			if(m_pEnvelope->LoopStart() != XMInstrument::Envelope::INVALID && 
				m_pEnvelope->LoopEnd() != XMInstrument::Envelope::INVALID)
			{
				const int _pt_loop_start_x = m_Zoom * (float)m_pEnvelope->Point(m_pEnvelope->LoopStart());
				const int _pt_loop_start_y = (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(m_pEnvelope->LoopStart())));

				const int _pt_loop_end_x = m_Zoom * (float)m_pEnvelope->Point(m_pEnvelope->LoopEnd());
				const int _pt_loop_end_y = (int)((float)m_CurrentScrollHeight * (1.0f - m_pEnvelope->Value(m_pEnvelope->LoopEnd())));

				// Envelope Point ���� Sustain Label�܂Ŕj��������
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
	//		HRESULT hr = _pDevice->SetTexture(0, m_pTexture);	//�e�N�X�`���̃Z�b�g
	//
	//		//�e�N�X�`���X�e�[�W�X�e�[�g
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

	/** �}�E�X�̍��{�^���N���b�N���̏������s��
	 * @param vKey �����ɉ����ꂽ���z�L�[�R�[�h
	 * @param position �}�E�X�J�[�\���̈ʒu */
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

			// �G���x���[�v�|�C���g���N���b�N�����̂����肷��
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
	
	/** �}�E�X�̍��{�^���𗣂������̏������s��
	 * @param vKey �����ɉ����ꂽ���z�L�[�R�[�h
	 * @param position �}�E�X�J�[�\���̈ʒu */
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

	/** Scroll Size �𒲐߂���
	* �w�肵�����W(maxPoint)���A���݂�Scroll�� + 100���L���ꍇ��Scroll����maxPoint + 100��Adjust����@
	* �w�肵�����W(maxPoint)���A���݂�Scroll�� - 100��苷���ꍇ��Scroll����maxPoint + 100��Adjust����*/
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
			// �I�t�Z�b�g�̒���
			if(_scroll_size.cx > _rect.Width())
			{
				SetScrollOffset(_pt_scroll_offset,false);
			}
			Invalidate();

		}

	}

	/** �E�{�^���N���b�N�������̏��� */
	void XMSamplerUIInst::EnvelopeEditor::OnRButtonDown(const UINT vKey, WTL::CPoint position)
	{
		if(!m_bPointEditing){
			// �R���e�L�X�g�R�}���h�̏����p�ɍ��W��ۑ�����
			m_EditPointX = position.x;
			m_EditPointY = position.y;
			
			CPoint _offset;
			GetScrollOffset(_offset);

			m_EditPointX += _offset.x;
			m_EditPointY += _offset.y;
	        
			// �E�N���b�N�������W���X�N���[�����W�ɕϊ�
			ClientToScreen(&position);
	        
			// �|�b�v�A�b�v���j���[���\�[�X�̃��[�h�ƕ\��
			CMenu _menuPopup;
			_menuPopup.LoadMenu(IDR_MENU_ENV_EDIT);
			// �G���x���[�v�|�C���g���N���b�N�����̂����肷��
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
			// �G���[���b�Z�[�W 
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

    /** Envelope Point ���폜���� 
	* @param uNotifyCode �ʒm�R�[�h
	* ���b�Z�[�W���R���g���[�����瑗���Ă���ꍇ�ɂ́A�R���g���[������̒ʒm�R�[�h���w�肳��܂��B
	* �A�N�Z�����[�^���瑗��ꂽ�ꍇ�ɂ� 1 ���w�肳��܂��B���j���[���瑗��ꂽ�ꍇ�� 0 ���w�肳��܂��B
	* @param nID �R���g���[��ID
	* ���j���[�A�C�e���A�R���g���[���A�A�N�Z�����[�^�[�� ID ���w�肳��܂��B
	* @param hWndCtl �R���g���[���̃E�B���h�E�n���h��
	* 
	*/
	void XMSamplerUIInst::EnvelopeEditor::OnDelPoint(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
	{
		if(m_pEnvelope->NumOfPoints() == 0)
		{
			// �G���[���b�Z�[�W 
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


    /** �T�X�e�C���ݒ� */
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

	/** �T�X�e�B���ݒ���� */ 
	void XMSamplerUIInst::EnvelopeEditor::OnRemoveSustain(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
	{
		ATLTRACE2("OnRemoveSustain\n");
		boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->SustainBegin(XMInstrument::Envelope::INVALID);
		m_pEnvelope->SustainEnd(XMInstrument::Envelope::INVALID);
		Invalidate();

	}
	/** ���[�v�J�n�ʒu�ݒ� */   
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

    /** ���[�v�I���ʒu�ݒ� */
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

	/** Loop�ݒ���� */
	void XMSamplerUIInst::EnvelopeEditor::OnRemoveLoop(const UINT uNotifyCode, const int nID, const HWND hWndCtl)
	{
		ATLTRACE2("OnRemoveLoop\n");
		boost::recursive_mutex::scoped_lock _lock(m_pXMSampler->Mutex());
		m_pEnvelope->LoopStart(XMInstrument::Envelope::INVALID);
		m_pEnvelope->LoopEnd(XMInstrument::Envelope::INVALID);
		Invalidate();

	}

	/** �_�~�[�̃E�B���h�E�v���V�[�W�� 
	 * @brief Envelope Editor ��Dialog API �ɂ��쐬����AApplication ���T�u�N���X�������܂łɎg�p�����
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

}

// XMSamplerUIInst

