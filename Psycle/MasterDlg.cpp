// MasterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "ChildView.h"
#include "MasterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg dialog


CMasterDlg::CMasterDlg(CChildView* pParent /*=NULL*/)
	: CDialog(CMasterDlg::IDD, pParent)
{
	m_pParent = pParent;
	//{{AFX_DATA_INIT(CMasterDlg)
	//}}AFX_DATA_INIT
	memset(macname,0,15*MAX_CONNECTIONS);
}


void CMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMasterDlg)
	DDX_Control(pDX, IDC_MASTERPEAK, m_masterpeak);
	DDX_Control(pDX, IDC_SLIDERMASTER, m_slidermaster);
	DDX_Control(pDX, IDC_SLIDERM9, m_sliderm9);
	DDX_Control(pDX, IDC_SLIDERM8, m_sliderm8);
	DDX_Control(pDX, IDC_SLIDERM7, m_sliderm7);
	DDX_Control(pDX, IDC_SLIDERM6, m_sliderm6);
	DDX_Control(pDX, IDC_SLIDERM5, m_sliderm5);
	DDX_Control(pDX, IDC_SLIDERM4, m_sliderm4);
	DDX_Control(pDX, IDC_SLIDERM3, m_sliderm3);
	DDX_Control(pDX, IDC_SLIDERM2, m_sliderm2);
	DDX_Control(pDX, IDC_SLIDERM12, m_sliderm12);
	DDX_Control(pDX, IDC_SLIDERM11, m_sliderm11);
	DDX_Control(pDX, IDC_SLIDERM10, m_sliderm10);
	DDX_Control(pDX, IDC_SLIDERM1, m_sliderm1);
	DDX_Control(pDX, IDC_MIXERVIEW, m_mixerview);
	DDX_Control(pDX, IDC_AUTODEC, m_autodec);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMasterDlg, CDialog)
	//{{AFX_MSG_MAP(CMasterDlg)
	ON_BN_CLICKED(IDC_AUTODEC, OnAutodec)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERMASTER, OnCustomdrawSlidermaster)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM1, OnCustomdrawSliderm1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM10, OnCustomdrawSliderm10)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM11, OnCustomdrawSliderm11)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM12, OnCustomdrawSliderm12)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM2, OnCustomdrawSliderm2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM3, OnCustomdrawSliderm3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM4, OnCustomdrawSliderm4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM5, OnCustomdrawSliderm5)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM6, OnCustomdrawSliderm6)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM7, OnCustomdrawSliderm7)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM8, OnCustomdrawSliderm8)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERM9, OnCustomdrawSliderm9)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg message handlers

BOOL CMasterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	namesFont.CreatePointFont(80,"Tahoma");
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
	
	m_slidermaster.SetPos(256-_pMachine->_outDry);

	float val;
	if (_pMachine->_inputCon[0])
	{
		_pMachine->GetWireVolume(0,val);
		m_sliderm1.SetPos(256-f2i(val*256));
	}
	if (_pMachine->_inputCon[1])
	{
		_pMachine->GetWireVolume(1,val);
		m_sliderm2.SetPos(256-f2i(val*256));
	}
	if (_pMachine->_inputCon[2])
	{
		_pMachine->GetWireVolume(2,val);
		m_sliderm3.SetPos(256-f2i(val*256));
	}
	if (_pMachine->_inputCon[3])
	{
		_pMachine->GetWireVolume(3,val);
		m_sliderm4.SetPos(256-f2i(val*256));
	}
	if (_pMachine->_inputCon[4])
	{
		_pMachine->GetWireVolume(4,val);
		m_sliderm5.SetPos(256-f2i(val*256));
	}
	if (_pMachine->_inputCon[5])
	{
		_pMachine->GetWireVolume(5,val);
		m_sliderm6.SetPos(256-f2i(val*256));
	}
	if (_pMachine->_inputCon[6])
	{
			_pMachine->GetWireVolume(6,val);
			m_sliderm7.SetPos(256-f2i(val*256));
	}
	if (_pMachine->_inputCon[7])
	{
		_pMachine->GetWireVolume(7,val);
		m_sliderm8.SetPos(256-f2i(val*256));
	}
	if (_pMachine->_inputCon[8])
	{
		_pMachine->GetWireVolume(8,val);
		m_sliderm9.SetPos(256-f2i(val*256));
	}
	if (_pMachine->_inputCon[9])
	{
		_pMachine->GetWireVolume(9,val);
		m_sliderm10.SetPos(256-f2i(val*256));
	}
	if (_pMachine->_inputCon[10])
	{
		_pMachine->GetWireVolume(10,val);
		m_sliderm11.SetPos(256-f2i(val*256));
	}
	if (_pMachine->_inputCon[11])
	{
		_pMachine->GetWireVolume(11,val);
		m_sliderm12.SetPos(256-f2i(val*256));
	}
	
	if (((Master*)_pMachine)->decreaseOnClip) m_autodec.SetCheck(1);
	else m_autodec.SetCheck(0);
	return TRUE;
}

void CMasterDlg::OnAutodec() 
{
	if (m_autodec.GetState() &0x0003)
	{
		((Master*)_pMachine)->decreaseOnClip=true;
	}
	else ((Master*)_pMachine)->decreaseOnClip=false;
}

BOOL CMasterDlg::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

void CMasterDlg::OnCancel()
{
	m_pParent->MasterMachineDialog = NULL;
	DestroyWindow();
	delete this;
}


void CMasterDlg::OnCustomdrawSlidermaster(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->_outDry = 256-m_slidermaster.GetPos();
	
	PaintNumbers(_pMachine->_outDry,32,142);
	
/*	float const mv = _pMachine->_outDry*0.00390625f;
	char buffer[16];
	
	  if (mv > 0.0f)
	  {
	  sprintf(buffer,"%.1f dB",20.0f * log10(mv));
	  }
	  else
	  {
	  sprintf(buffer,"-Inf. dB");
	  }
	m_dblevel.SetWindowText(buffer);
*/
	
	*pResult = 0;
}

void CMasterDlg::PaintNumbers(int val, int x, int y)
{
	CDC *dc = m_mixerview.GetDC();
	CDC memDC;
	CBitmap* oldbmp;
	memDC.CreateCompatibleDC(dc);
	oldbmp = memDC.SelectObject(&m_numbers);
	
	PaintNumbersDC(dc,&memDC,val,x,y);
	
	memDC.SelectObject(oldbmp);
	memDC.DeleteDC();
}

void CMasterDlg::PaintNumbersDC(CDC *dc, CDC *memDC, int val, int x, int y)
{
	//  val*=0.390625f // Percentage ( 0% ..100% )
	if (val > 0 ) // dB (-99.9dB .. 0dB)
	{
		val = f2i(-200.0f * log10f(val*0.00390625f)); // better don't aproximate with log2
	}
	else val = 999;
	
	
	if ( val < 100)
	{
		dc->BitBlt(x,y,4,8,memDC,50,0,SRCCOPY);
	}
	else
	{
		dc->BitBlt(x,y,4,8,memDC,val/100*5,0,SRCCOPY);
		val = val%100;
	}
	const int vx0 = val/10;
	const int v0x = val%10;
	dc->BitBlt(x+5,y,4,8,memDC,vx0*5,0,SRCCOPY);
	dc->BitBlt(x+12,y,4,8,memDC,v0x*5,0,SRCCOPY);
	
}

void CMasterDlg::PaintNames(char *name, int x, int y)
{
	CDC *dc = m_mixerview.GetDC();
	CFont* oldfont = dc->SelectObject(&namesFont);
	dc->SetTextColor(0x00FFFFFF); // White
	dc->SetBkColor(0x00000000); // Black
	dc->TextOut(x,y,name);
	dc->SelectObject(oldfont);
}

void CMasterDlg::OnCustomdrawSliderm1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->SetWireVolume(0,(256-m_sliderm1.GetPos())*0.00390625f);
	
	PaintNumbers(256-m_sliderm1.GetPos(),92,142);
	
	*pResult = 0;
}

void CMasterDlg::OnCustomdrawSliderm10(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->SetWireVolume(9,(256-m_sliderm10.GetPos())*0.00390625f);
	
	PaintNumbers(256-m_sliderm10.GetPos(),272,142);
	
	*pResult = 0;
}

void CMasterDlg::OnCustomdrawSliderm11(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->SetWireVolume(10,(256-m_sliderm11.GetPos())*0.00390625f);
	
	PaintNumbers(256-m_sliderm11.GetPos(),292,142);
	
	*pResult = 0;
}

void CMasterDlg::OnCustomdrawSliderm12(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->SetWireVolume(11,(256-m_sliderm12.GetPos())*0.00390625f);
	
	PaintNumbers(256-m_sliderm12.GetPos(),312,142);
	
	*pResult = 0;
}

void CMasterDlg::OnCustomdrawSliderm2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->SetWireVolume(1,(256-m_sliderm2.GetPos())*0.00390625f);
	
	PaintNumbers(256-m_sliderm2.GetPos(),112,142);
	
	*pResult = 0;
}

void CMasterDlg::OnCustomdrawSliderm3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->SetWireVolume(2,(256-m_sliderm3.GetPos())*0.00390625f);
	
	PaintNumbers(256-m_sliderm3.GetPos(),132,142);
	
	*pResult = 0;
}

void CMasterDlg::OnCustomdrawSliderm4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->SetWireVolume(3,(256-m_sliderm4.GetPos())*0.00390625f);
	
	PaintNumbers(256-m_sliderm4.GetPos(),152,142);
	
	*pResult = 0;
}

void CMasterDlg::OnCustomdrawSliderm5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->SetWireVolume(4,(256-m_sliderm5.GetPos())*0.00390625f);
	
	PaintNumbers(256-m_sliderm5.GetPos(),172,142);
	
	*pResult = 0;
}

void CMasterDlg::OnCustomdrawSliderm6(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->SetWireVolume(5,(256-m_sliderm6.GetPos())*0.00390625f);
	
	PaintNumbers(256-m_sliderm6.GetPos(),192,142);
	
	*pResult = 0;
}

void CMasterDlg::OnCustomdrawSliderm7(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->SetWireVolume(6,(256-m_sliderm7.GetPos())*0.00390625f);
	
	PaintNumbers(256-m_sliderm7.GetPos(),212,142);
	
	*pResult = 0;
}

void CMasterDlg::OnCustomdrawSliderm8(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->SetWireVolume(7,(256-m_sliderm8.GetPos())*0.00390625f);
	
	PaintNumbers(256-m_sliderm8.GetPos(),232,142);
	
	*pResult = 0;
}

void CMasterDlg::OnCustomdrawSliderm9(NMHDR* pNMHDR, LRESULT* pResult) 
{
	_pMachine->SetWireVolume(8,(256-m_sliderm9.GetPos())*0.00390625f);
	
	PaintNumbers(256-m_sliderm9.GetPos(),252,142);
	
	*pResult = 0;
}

void CMasterDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if ( dc.m_ps.rcPaint.bottom >= 145 && dc.m_ps.rcPaint.top <= 155)
	{
		CDC *dcm = m_mixerview.GetDC();
		CDC memDC;
		CBitmap* oldbmp;
		memDC.CreateCompatibleDC(dcm);
		oldbmp = memDC.SelectObject(&m_numbers);
		
		PaintNumbersDC(dcm,&memDC,256-m_slidermaster.GetPos(),32,142);
		PaintNumbersDC(dcm,&memDC,256-m_sliderm1.GetPos(),92,142);
		PaintNumbersDC(dcm,&memDC,256-m_sliderm2.GetPos(),112,142);
		PaintNumbersDC(dcm,&memDC,256-m_sliderm3.GetPos(),132,142);
		PaintNumbersDC(dcm,&memDC,256-m_sliderm4.GetPos(),152,142);
		PaintNumbersDC(dcm,&memDC,256-m_sliderm5.GetPos(),172,142);
		PaintNumbersDC(dcm,&memDC,256-m_sliderm6.GetPos(),192,142);
		PaintNumbersDC(dcm,&memDC,256-m_sliderm7.GetPos(),212,142);
		PaintNumbersDC(dcm,&memDC,256-m_sliderm8.GetPos(),232,142);
		PaintNumbersDC(dcm,&memDC,256-m_sliderm9.GetPos(),252,142);
		PaintNumbersDC(dcm,&memDC,256-m_sliderm10.GetPos(),272,142);
		PaintNumbersDC(dcm,&memDC,256-m_sliderm11.GetPos(),292,142);
		PaintNumbersDC(dcm,&memDC,256-m_sliderm12.GetPos(),312,142);

		memDC.SelectObject(oldbmp);
		memDC.DeleteDC();
	}
	if ( dc.m_ps.rcPaint.bottom >=25 && dc.m_ps.rcPaint.top<=155 && dc.m_ps.rcPaint.right >=350)
	{
		CDC *dcm = m_mixerview.GetDC();
		CFont* oldfont = dcm->SelectObject(&namesFont);
		dcm->SetTextColor(0x00FFFFFF); // White
		dcm->SetBkColor(0x00000000); // Black
		dcm->TextOut(353,24,macname[0]);
		dcm->TextOut(428,24,macname[1]);
		dcm->TextOut(353,46,macname[2]);
		dcm->TextOut(428,46,macname[3]);
		dcm->TextOut(353,68,macname[4]);
		dcm->TextOut(428,68,macname[5]);
		dcm->TextOut(353,90,macname[6]);
		dcm->TextOut(428,90,macname[7]);
		dcm->TextOut(353,112,macname[8]);
		dcm->TextOut(428,112,macname[9]);
		dcm->TextOut(353,134,macname[10]);
		dcm->TextOut(428,134,macname[11]);
		dcm->SelectObject(oldfont);
	}
	// Do not call CDialog::OnPaint() for painting messages
}
