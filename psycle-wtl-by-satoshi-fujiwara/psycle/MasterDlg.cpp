/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.5 $
 */
#include "stdafx.h"
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
#include "PsycleWTLView.h"
#include "MasterDlg.h"
#include ".\masterdlg.h"




/////////////////////////////////////////////////////////////////////////////
// CMasterDlg dialog


CMasterDlg::CMasterDlg(CPsycleWTLView* pParent /*=NULL*/)
{
	m_pParent = pParent;
	//{{AFX_DATA_INIT(CMasterDlg)
	//}}AFX_DATA_INIT
	memset(macname,0,32 * MAX_CONNECTIONS * sizeof(TCHAR));
}

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg message handlers


void CMasterDlg::PaintNumbers(int val, int x, int y)
{
	WTL::CDC dc(m_mixerview.GetDC());
	WTL::CDC memDC;// Managed DC Class
	memDC.CreateCompatibleDC(dc);
	CBitmapHandle oldbmp(memDC.SelectBitmap((HBITMAP)m_numbers));
	
	PaintNumbersDC(&dc,&memDC,val,x,y);
	
	memDC.SelectBitmap((HBITMAP)oldbmp);
	dc.Detach();
}

void CMasterDlg::PaintNumbersDC(CDC *dc, CDC *memDC, int val, int x, int y)
{
//  val*=0.390625f // Percentage ( 0% ..100% )
	if (val > 0 ) // dB (-99.9dB .. 0dB)
	{
		val = f2i(200.0f * log10f(CValueMapper::Map_255_1(val))); // better don't aproximate with log2
	}
	else val = -999;
	
	TCHAR valtxt[6];
	CFontHandle oldfont(dc->SelectFont(namesFont));
	dc->SetTextColor(0x00FFFFFF); // White
	dc->SetBkColor(0x00000000); // Black
	
	if ( abs(val) < 100)
	{
		if ( val < 0 ) _stprintf(valtxt,_T("%.01f"),val/10.0f);
		else _stprintf(valtxt,_T(" %.01f"),val/10.0f);
		dc->TextOut(x,y - 2,valtxt);
	}
	else
	{
		if (val == -999 ) _tcscpy(valtxt,_T("-99"));
		else if (val < 0 ) _stprintf(valtxt,_T("%.0f "),val/10.0f);
		else  _stprintf(valtxt,_T(" %.0f "),val/10.0f);
		dc->TextOut(x,y - 2,valtxt);
	}
	dc->SelectFont(oldfont);
}

void CMasterDlg::PaintNames(const TCHAR *name, int x, int y)
{
	WTL::CDC dc(m_mixerview.GetDC());
	CFontHandle oldfont(dc.SelectFont((HFONT)namesFont));
	dc.SetTextColor(0x00FFFFFF); // White
	dc.SetBkColor(0x00000000); // Black
	dc.TextOut(x,y,name);
	dc.SelectFont((HFONT)oldfont);
	dc.Detach();
}

LRESULT CMasterDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ExecuteDlgInit(IDD);
	m_masterpeak.Attach(GetDlgItem(IDC_MASTERPEAK));
	m_slidermaster.Attach(GetDlgItem(IDC_SLIDERMASTER));
	m_sliderm9.Attach(GetDlgItem(IDC_SLIDERM9));
	m_sliderm8.Attach(GetDlgItem(IDC_SLIDERM8));
	m_sliderm7.Attach(GetDlgItem(IDC_SLIDERM7));
	m_sliderm6.Attach(GetDlgItem(IDC_SLIDERM6));
	m_sliderm5.Attach(GetDlgItem(IDC_SLIDERM5));
	m_sliderm4.Attach(GetDlgItem(IDC_SLIDERM4));
	m_sliderm3.Attach(GetDlgItem(IDC_SLIDERM3));
	m_sliderm2.Attach(GetDlgItem(IDC_SLIDERM2));
	m_sliderm12.Attach(GetDlgItem(IDC_SLIDERM12));
	m_sliderm11.Attach(GetDlgItem(IDC_SLIDERM11));
	m_sliderm10.Attach(GetDlgItem(IDC_SLIDERM10));
	m_sliderm1.Attach(GetDlgItem(IDC_SLIDERM1));
	m_mixerview.Attach(GetDlgItem(IDC_MIXERVIEW));
	m_autodec.Attach(GetDlgItem(IDC_AUTODEC));

	namesFont.CreatePointFont(80,_T("MS UI Gothic"));
	m_numbers.LoadBitmap(IDB_MASTERNUMBERS);
	
	m_slidermaster.SetRange(0, 256);
	m_sliderm1.SetRange(0, 256);
	m_sliderm2.SetRange(0, 256);
	m_sliderm3.SetRange(0, 256);
	m_sliderm4.SetRange(0, 256);
	m_sliderm5.SetRange(0, 256);
	m_sliderm6.SetRange(0, 256);
	m_sliderm7.SetRange(0, 256);
	m_sliderm8.SetRange(0, 256);
	m_sliderm9.SetRange(0, 256);
	m_sliderm10.SetRange(0, 256);
	m_sliderm11.SetRange(0, 256);
	m_sliderm12.SetRange(0, 256);
	
	float val;
	val = sqrtf((float)_pMachine->_outDry * 64.0f);
	m_slidermaster.SetPos(256 - f2i(val));

	if (_pMachine->_inputCon[0])
	{
		_pMachine->GetWireVolume(0,val);
		val = sqrtf(val * 16384.0f);
		m_sliderm1.SetPos(256 - f2i(val));
	}
	else
	{
		m_sliderm1.SetPos(256);
	}

	if (_pMachine->_inputCon[1])
	{
		_pMachine->GetWireVolume(1,val);
		val = sqrtf(val*16384.0f);
		m_sliderm2.SetPos(256-f2i(val));
	}
	else
	{
		m_sliderm2.SetPos(256);
	}

	if (_pMachine->_inputCon[2])
	{
		_pMachine->GetWireVolume(2,val);
		val = sqrtf(val*16384.0f);
		m_sliderm3.SetPos(256-f2i(val));
	}
	else
	{
		m_sliderm3.SetPos(256);
	}

	if (_pMachine->_inputCon[3])
	{
		_pMachine->GetWireVolume(3,val);
		val = sqrtf(val * 16384.0f);
		m_sliderm4.SetPos(256-f2i(val));
	}
	else
	{
		m_sliderm4.SetPos(256);
	}

	if (_pMachine->_inputCon[4])
	{
		_pMachine->GetWireVolume(4,val);
		val = sqrtf(val * 16384.0f);
		m_sliderm5.SetPos(256 - f2i(val));
	}
	else
	{
		m_sliderm5.SetPos(256);
	}

	if (_pMachine->_inputCon[5])
	{
		_pMachine->GetWireVolume(5,val);
		val = sqrtf(val*16384.0f);
		m_sliderm6.SetPos(256-f2i(val));
	}
	else
	{
		m_sliderm6.SetPos(256);
	}

	if (_pMachine->_inputCon[6])
	{
		_pMachine->GetWireVolume(6,val);
		val = sqrtf(val*16384.0f);
		m_sliderm7.SetPos(256-f2i(val));
	}
	else
	{
		m_sliderm7.SetPos(256);
	}

	if (_pMachine->_inputCon[7])
	{
		_pMachine->GetWireVolume(7,val);
		val = sqrtf(val*16384.0f);
		m_sliderm8.SetPos(256-f2i(val));
	}
	else
	{
		m_sliderm8.SetPos(256);
	}

	if (_pMachine->_inputCon[8])
	{
		_pMachine->GetWireVolume(8,val);
		val = sqrtf(val*16384.0f);
		m_sliderm9.SetPos(256-f2i(val));
	}
	else
	{
		m_sliderm9.SetPos(256);
	}

	if (_pMachine->_inputCon[9])
	{
		_pMachine->GetWireVolume(9,val);
		val = sqrtf(val*16384.0f);
		m_sliderm10.SetPos(256-f2i(val));
	}
	else
	{
		m_sliderm10.SetPos(256);
	}

	if (_pMachine->_inputCon[10])
	{
		_pMachine->GetWireVolume(10,val);
		val = sqrtf(val*16384.0f);
		m_sliderm11.SetPos(256-f2i(val));
	}
	else
	{
		m_sliderm11.SetPos(256);
	}

	if (_pMachine->_inputCon[11])
	{
		_pMachine->GetWireVolume(11,val);
		val = sqrtf(val*16384.0f);
		m_sliderm12.SetPos(256-f2i(val));
	}
	else
	{
		m_sliderm12.SetPos(256);
	}
	
	if (((Master*)_pMachine)->decreaseOnClip){
		m_autodec.SetCheck(1);
	} else { 
		m_autodec.SetCheck(0);
	}
	return TRUE;
}

LRESULT CMasterDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO : ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
	CPaintDC dc(m_hWnd); // device context for painting
	
	if ( dc.m_ps.rcPaint.bottom >= 145 && dc.m_ps.rcPaint.top <= 155)
	{
		CDC dcm(m_mixerview.GetDC());
		CDC memDC;
		memDC.CreateCompatibleDC(dcm);
		CBitmapHandle oldbmp(memDC.SelectBitmap((HBITMAP)m_numbers));

		PaintNumbersDC(&dcm,&memDC,256-m_slidermaster.GetPos(),40,156);
		PaintNumbersDC(&dcm,&memDC,256-m_sliderm1.GetPos(),110,156);
		PaintNumbersDC(&dcm,&memDC,256-m_sliderm2.GetPos(),133,156);
		PaintNumbersDC(&dcm,&memDC,256-m_sliderm3.GetPos(),157,156);
		PaintNumbersDC(&dcm,&memDC,256-m_sliderm4.GetPos(),180,156);
		PaintNumbersDC(&dcm,&memDC,256-m_sliderm5.GetPos(),204,156);
		PaintNumbersDC(&dcm,&memDC,256-m_sliderm6.GetPos(),226,156);
		PaintNumbersDC(&dcm,&memDC,256-m_sliderm7.GetPos(),250,156);
		PaintNumbersDC(&dcm,&memDC,256-m_sliderm8.GetPos(),274,156);
		PaintNumbersDC(&dcm,&memDC,256-m_sliderm9.GetPos(),297,156);
		PaintNumbersDC(&dcm,&memDC,256-m_sliderm10.GetPos(),321,156);
		PaintNumbersDC(&dcm,&memDC,256-m_sliderm11.GetPos(),343,156);
		PaintNumbersDC(&dcm,&memDC,256-m_sliderm12.GetPos(),366,156);

		memDC.SelectBitmap((HBITMAP)oldbmp);
		memDC.DeleteDC();
		dcm.Detach();
	}
	if ( dc.m_ps.rcPaint.bottom >= 25 && dc.m_ps.rcPaint.top<=155 && dc.m_ps.rcPaint.right >=350)
	{
		CDC dcm(m_mixerview.GetDC());
		CFontHandle oldfont(dcm.SelectFont((HFONT)namesFont));
		
		dcm.SetTextColor(0x00FFFFFF); // White
		dcm.SetBkColor(0x00000000); // Black
		dcm.TextOut(420,28,macname[0]);
		dcm.TextOut(508,28,macname[1]);
		dcm.TextOut(420,52,macname[2]);
		dcm.TextOut(508,52,macname[3]);
		dcm.TextOut(420,76,macname[4]);
		dcm.TextOut(508,76,macname[5]);
		dcm.TextOut(420,99,macname[6]);
		dcm.TextOut(508,99,macname[7]);
		dcm.TextOut(420,123,macname[8]);
		dcm.TextOut(508,123,macname[9]);
		dcm.TextOut(420,148,macname[10]);
		dcm.TextOut(508,148,macname[11]);
		dcm.SelectFont((HFONT)oldfont);
		dcm.Detach();
	}
	// Do not call CDialog::OnPaint() for painting messages
	return 0;
}

LRESULT CMasterDlg::OnBnClickedAutodec(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	
	// TODO : ここにコントロール通知ハンドラ コードを追加します。

	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSlidermaster(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
//	_pMachine->_outDry = 256-m_slidermaster.GetPos();
	_pMachine->_outDry = ((256 - m_slidermaster.GetPos()) * (256 - m_slidermaster.GetPos()))/64;

	PaintNumbers(_pMachine->_outDry,40,156);
	
/*	float const mv = CValueMapper::Map_255_1(_pMachine->_outDry);
	char buffer[16];
	
	  if (mv > 0.0f)
	  {
	  _stprintf(buffer,_T("%.1f dB"),20.0f * log10(mv));
	  }
	  else
	  {
	  _stprintf(buffer,_T("-Inf. dB"));
	  }
	m_dblevel.SetWindowText(buffer);
*/
	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSliderm1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	int val = f2i(((256-m_sliderm1.GetPos())*(256-m_sliderm1.GetPos()))/64.0f);
	_pMachine->SetWireVolume(0,CValueMapper::Map_255_1(val));
	PaintNumbers(val,110,156);
	
	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSliderm2(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
//	_pMachine->SetWireVolume(1,(256-m_sliderm2.GetPos())*0.00390625f);
//	PaintNumbers(256-m_sliderm2.GetPos(),112,142);
	int val = f2i(((256-m_sliderm2.GetPos())*(256-m_sliderm2.GetPos()))/64.0f);
	_pMachine->SetWireVolume(1,CValueMapper::Map_255_1(val));
	PaintNumbers(val,133,156);
	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSliderm3(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
//	_pMachine->SetWireVolume(2,(256-m_sliderm3.GetPos())*0.00390625f);
//	PaintNumbers(256-m_sliderm3.GetPos(),132,142);
	int val = f2i(((256-m_sliderm3.GetPos())*(256-m_sliderm3.GetPos()))/64.0f);
	_pMachine->SetWireVolume(2,CValueMapper::Map_255_1(val));
	PaintNumbers(val,157,156);
	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSliderm4(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
//	_pMachine->SetWireVolume(3,(256-m_sliderm4.GetPos())*0.00390625f);
//	PaintNumbers(256-m_sliderm4.GetPos(),152,142);
	int val = f2i(((256-m_sliderm4.GetPos())*(256-m_sliderm4.GetPos()))/64.0f);
	_pMachine->SetWireVolume(3,CValueMapper::Map_255_1(val));
	PaintNumbers(val,180,156);
	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSliderm5(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
//	_pMachine->SetWireVolume(4,(256-m_sliderm5.GetPos())*0.00390625f);
//	PaintNumbers(256-m_sliderm5.GetPos(),172,142);
	int val = f2i(((256-m_sliderm5.GetPos())*(256-m_sliderm5.GetPos()))/64.0f);
	_pMachine->SetWireVolume(4,CValueMapper::Map_255_1(val));
	PaintNumbers(val,204,156);
	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSliderm6(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
//	_pMachine->SetWireVolume(5,(256-m_sliderm6.GetPos())*0.00390625f);
//	PaintNumbers(256-m_sliderm6.GetPos(),192,142);
	int val = f2i(((256-m_sliderm6.GetPos())*(256-m_sliderm6.GetPos()))/64.0f);
	_pMachine->SetWireVolume(5,CValueMapper::Map_255_1(val));
	PaintNumbers(val,226,156);
	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSliderm7(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
	_pMachine->SetWireVolume(6,(256-m_sliderm7.GetPos())*0.00390625f);
//	PaintNumbers(256-m_sliderm7.GetPos(),212,142);
	int val = f2i(((256-m_sliderm7.GetPos())*(256-m_sliderm7.GetPos()))/64.0f);
	_pMachine->SetWireVolume(6,CValueMapper::Map_255_1(val));
	PaintNumbers(val,250,156);
	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSliderm8(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
//	_pMachine->SetWireVolume(7,(256-m_sliderm8.GetPos())*0.00390625f);
//	PaintNumbers(256-m_sliderm8.GetPos(),232,142);
	int val = f2i(((256-m_sliderm8.GetPos())*(256-m_sliderm8.GetPos()))/64.0f);
	_pMachine->SetWireVolume(7,CValueMapper::Map_255_1(val));
	PaintNumbers(val,274,156);
	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSliderm9(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
//	_pMachine->SetWireVolume(8,(256-m_sliderm9.GetPos())*0.00390625f);
//	PaintNumbers(256-m_sliderm9.GetPos(),252,142);
	int val = f2i(((256-m_sliderm9.GetPos())*(256-m_sliderm9.GetPos()))/64.0f);
	_pMachine->SetWireVolume(8,CValueMapper::Map_255_1(val));
	PaintNumbers(val,297,156);
	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSliderm10(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
	int val = f2i(((256-m_sliderm10.GetPos())*(256-m_sliderm10.GetPos()))/64.0f);
	_pMachine->SetWireVolume(9,CValueMapper::Map_255_1(val));
	PaintNumbers(val,321,156);
	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSliderm11(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	//	_pMachine->SetWireVolume(10,(256-m_sliderm11.GetPos())*0.00390625f);
	//	PaintNumbers(256-m_sliderm11.GetPos(),292,142);
	int val = f2i(((256-m_sliderm11.GetPos())*(256-m_sliderm11.GetPos()))/64.0f);
	_pMachine->SetWireVolume(10,CValueMapper::Map_255_1(val));
	PaintNumbers(val,343,156);

	return 0;
}

LRESULT CMasterDlg::OnNMCustomdrawSliderm12(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO : ここにコントロール通知ハンドラ コードを追加します。
	//	_pMachine->SetWireVolume(11,(256-m_sliderm12.GetPos())*0.00390625f);
	//	PaintNumbers(256-m_sliderm12.GetPos(),312,142);
	int val = f2i(((256-m_sliderm12.GetPos())*(256-m_sliderm12.GetPos()))/64.0f);
	_pMachine->SetWireVolume(11,CValueMapper::Map_255_1(val));
	PaintNumbers(val,366,156);

// I know the following is Ugly, but it is the only solution I've found, because first,
// OnPaint is called, after the bitmap is drawn, and finally the sliders are redrawn.
	CDC dcm(m_mixerview.GetDC());
	CFontHandle oldfont(dcm.SelectFont((HFONT)namesFont));
	dcm.SetTextColor(0x00FFFFFF); // White
	dcm.SetBkColor(0x00000000); // Black
	dcm.TextOut(420,28,macname[0]);
	dcm.TextOut(508,28,macname[1]);
	dcm.TextOut(420,52,macname[2]);
	dcm.TextOut(508,52,macname[3]);
	dcm.TextOut(420,76,macname[4]);
	dcm.TextOut(508,76,macname[5]);
	dcm.TextOut(420,99,macname[6]);
	dcm.TextOut(508,99,macname[7]);
	dcm.TextOut(420,123,macname[8]);
	dcm.TextOut(508,123,macname[9]);
	dcm.TextOut(420,148,macname[10]);
	dcm.TextOut(508,148,macname[11]);
	dcm.SelectFont((HFONT)oldfont);	
	return 0;
}

//LRESULT CMasterDlg::OnCancelMode(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//{
//	// TODO : ここにメッセージ ハンドラ コードを追加するか、既定の処理を呼び出します。
//
//	return 0;
//}

//LRESULT CMasterDlg::OnCancelMode(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//{
//	m_pParent->MasterMachineDialog = NULL;
//	this->EndDialog(0);
//	return TRUE;
//}

LRESULT CMasterDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	m_pParent->MasterMachineDialog = NULL;
	DestroyWindow();
	bHandled = FALSE;
	return 0;
}

LRESULT CMasterDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	//m_hWnd = NULL;
	//delete this;
	bHandled = FALSE;
	return 0;
}

void CMasterDlg::OnFinalMessage(HWND hWnd)
{
	CDialogImpl<CMasterDlg>::OnFinalMessage(hWnd);
	delete this;
}
