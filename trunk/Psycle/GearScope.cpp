// GearScope.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "GearScope.h"
#include "ChildView.h"
#include "Machine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearScope dialog


CGearScope::CGearScope(CChildView* pParent /*=NULL*/)
	: CDialog(CGearScope::IDD, pParent)
{
	m_pParent = pParent;
	//{{AFX_DATA_INIT(CGearScope)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

}


void CGearScope::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearScope)
	DDX_Control(pDX, IDC_SLIDER, m_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_slider2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearScope, CDialog)
	//{{AFX_MSG_MAP(CGearScope)
	ON_WM_TIMER()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER, OnCustomdrawSlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_BN_CLICKED(IDC_BUTTON, OnMode)
	ON_BN_CLICKED(IDC_BUTTON2, OnHold)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearScope message handlers

BOOL CGearScope::OnInitDialog() 
{
	CDialog::OnInitDialog();

	hold = FALSE;

	pos = 0;

	for (int i = 0; i < MAX_SCOPE_BANDS; i++)
	{
		bar_heightsl[i]=256;
		bar_heightsr[i]=256;
	}

	_pMachine->bCanDraw = TRUE;
	CClientDC dc(this);
	rc.top = 2;
	rc.left = 2;
	rc.bottom = 128+rc.top;
	rc.right = 256+rc.left;
	bmpDC = new CBitmap;
	bmpDC->CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);

	CDC bufDC;
	bufDC.CreateCompatibleDC(&dc);
	CBitmap* oldbmp;
	oldbmp = bufDC.SelectObject(bmpDC);

	bufDC.FillSolidRect(0,0,rc.right-rc.left,rc.bottom-rc.top,0);

	bufDC.SelectObject(oldbmp);
	bufDC.DeleteDC();

	font.CreatePointFont(70,"Tahoma");

	SetMode();
	_pMachine->bCanDraw = TRUE;

	return TRUE;
}


BOOL CGearScope::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

void CGearScope::OnCancel()
{
	_pMachine->bCanDraw = FALSE;
	font.DeleteObject();
	m_pParent->ScopeMachineDialog[this_index] = NULL;
	KillTimer(2304+this_index);
	DestroyWindow();
	bmpDC->DeleteObject();
	delete this;
}

inline int CGearScope::GetY(float f)
{
	f*=(64.0f/32768.0f);
	f=64-f;
	if (f < 1) return 1;
	else if (f > 126) return 126;
	return int(f);
}

void CGearScope::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 2304+this_index )
	{
		CClientDC dc(this);

		CDC bufDC;
		bufDC.CreateCompatibleDC(&dc);
		CBitmap* oldbmp;
		oldbmp = bufDC.SelectObject(bmpDC);
		bufDC.FillSolidRect(0,0,rc.right-rc.left,rc.bottom-rc.top,0);

		switch (_pMachine->scope_mode)
		{
		case 0: // oscilloscope
			{

				int freq = _pMachine->scope_osc_freq*_pMachine->scope_osc_freq;

				char buf[64];
				sprintf(buf,"Frequency %dhz Refresh %.2fhz",freq,1000.0f/_pMachine->scope_osc_rate);

				CFont* oldFont= bufDC.SelectObject(&font);
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x606060);
				bufDC.TextOut(4, 128-14, buf);
				bufDC.SelectObject(oldFont);

				CPen linepen1(PS_SOLID, 2, 0xc08080);
				CPen linepen2(PS_SOLID, 2, 0x80c080);
				CPen linepen3(PS_SOLID, 4, 0x00404040);
				CPen linepen4(PS_SOLID, 8, 0x00202020);

				CPen *oldpen = bufDC.SelectObject(&linepen4);

				// now draw our scope

				bufDC.MoveTo(0,64);
				bufDC.LineTo(255,64);

				bufDC.SelectObject(&linepen3);
				bufDC.LineTo(0,64);

				bufDC.SelectObject(&linepen1);

				float add = (float(Global::pConfig->_pOutputDriver->_samplesPerSec)/(float(freq)))/64.0f;

				bufDC.MoveTo(0,GetY(_pMachine->bufL[pos]));
				float n = float(pos)+add;
				for (int x = 1; x < 256; x+=4)
				{
					bufDC.LineTo(x,GetY(_pMachine->bufL[int(n)&(SCOPE_BUF_SIZE-1)]));
					n += add;
				}

				bufDC.SelectObject(&linepen2);
				bufDC.MoveTo(0,GetY(_pMachine->bufR[pos]));
				n = float(pos)+add;
				for (x = 1; x < 256; x+=4)
				{
					bufDC.LineTo(x,GetY(_pMachine->bufR[int(n)&(SCOPE_BUF_SIZE-1)]));
					n += add;
				}

				bufDC.SelectObject(oldpen);
				linepen1.DeleteObject();
				linepen2.DeleteObject();
				linepen3.DeleteObject();
				linepen4.DeleteObject();

				if (!hold)
				{
					pos = int(n);
					pos&=(SCOPE_BUF_SIZE-1);
				}
			}
			break;

		case 1: // spectrum analyzer
			{
			   float aal[MAX_SCOPE_BANDS]; 
			   float aar[MAX_SCOPE_BANDS]; 
			   float bbl[MAX_SCOPE_BANDS]; 
			   float bbr[MAX_SCOPE_BANDS]; 
			   float ampl[MAX_SCOPE_BANDS];
			   float ampr[MAX_SCOPE_BANDS];
			   memset (aal,0,sizeof(aal));
			   memset (bbl,0,sizeof(bbl));
			   memset (aar,0,sizeof(aar));
			   memset (bbr,0,sizeof(bbr));

			   for (int i=0;i<SCOPE_SPEC_SAMPLES;i++) 
			   { 
				 float wl=(_pMachine->bufL[i])/32768; 
				 float wr=(_pMachine->bufR[i])/32768; 
				 int im = i-(SCOPE_SPEC_SAMPLES/2); 
				 for(int h=0;h<_pMachine->scope_spec_bands;h++) 
				 { 
					float th=((F_PI/(SCOPE_SPEC_SAMPLES/2))*(h+1))*im; 
					float cth = cosf(th);
					float sth = sinf(th);
					aal[h]+=wl*cth; 
					bbl[h]+=wl*sth; 
					aar[h]+=wr*cth; 
					bbr[h]+=wr*sth; 
				 } 
			   } 
			   for (int h=0;h<_pMachine->scope_spec_bands;h++) 
			   {
				   ampl[h]= sqrtf(aal[h]*aal[h]+bbl[h]*bbl[h])/(SCOPE_SPEC_SAMPLES/2); 
				   ampr[h]= sqrtf(aar[h]*aar[h]+bbr[h]*bbr[h])/(SCOPE_SPEC_SAMPLES/2); 
			   }

			   int width = 128/_pMachine->scope_spec_bands;
				COLORREF cl = 0xb07070;
				COLORREF cr = 0x70b070;
				CPen linepen(PS_SOLID, width, cl);

				CPen *oldpen = bufDC.SelectObject(&linepen);

				linepen.DeleteObject();
				int x = (width/2);

				for (i = 0; i < _pMachine->scope_spec_bands; i++)
				{


					int aml = 128+(width)-f2i((sqrtf(ampr[i]*16384)));
					if (aml < 0)
					{
						aml = 0;
					}
					if (aml < bar_heightsl[i])
					{
						bar_heightsl[i]=aml;
					}
					linepen.CreatePen(PS_SOLID, width, cl);
					bufDC.SelectObject(&linepen);
					bufDC.MoveTo(x,bar_heightsl[i]);
					bufDC.LineTo(x,128+width);
					linepen.DeleteObject();
					linepen.CreatePen(PS_SOLID, width, cl+0x202020);
					bufDC.SelectObject(&linepen);
					bufDC.MoveTo(x,128+width);
					linepen.DeleteObject();
					bufDC.LineTo(x,aml);
					linepen.DeleteObject();

					x+=width;

					int amr = 128+(width)-f2i((sqrtf(ampr[i]*16384)));
					if (amr < 0)
					{
						amr = 0;
					}
					if (amr < bar_heightsr[i])
					{
						bar_heightsr[i]=amr;
					}
					linepen.CreatePen(PS_SOLID, width, cr);
					bufDC.SelectObject(&linepen);
					bufDC.MoveTo(x,bar_heightsr[i]);
					bufDC.LineTo(x,128+width);
					linepen.DeleteObject();
					linepen.CreatePen(PS_SOLID, width, cr+0x202020);
					bufDC.SelectObject(&linepen);
					bufDC.MoveTo(x,128+width);
					bufDC.LineTo(x,amr);
					linepen.DeleteObject();

					x+=width;

					cl += 0x000002;
					cl -= 0x020200;
					cr += 0x000002;
					cr -= 0x020200;

					bar_heightsl[i]+=_pMachine->scope_spec_rate/10;
					if (bar_heightsl[i] > 128+width)
					{
						bar_heightsl[i] = 128+width;
					}
					bar_heightsr[i]+=_pMachine->scope_spec_rate/10;
					if (bar_heightsr[i] > 128+width)
					{
						bar_heightsr[i] = 128+width;
					}
				}
				bufDC.SelectObject(oldpen);
				linepen.DeleteObject();
	
			}
			break;
		}
		// and debuffer
		dc.BitBlt(rc.top,rc.left,rc.right-rc.left,rc.bottom-rc.top,&bufDC,0,0,SRCCOPY);

		bufDC.SelectObject(oldbmp);
		bufDC.DeleteDC();
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CGearScope::OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	switch (_pMachine->scope_mode)
	{
	case 0:
		_pMachine->scope_osc_freq = m_slider.GetPos();
		if (hold)
		{
			m_slider2.SetRange(0,int(Global::pConfig->_pOutputDriver->_samplesPerSec*2.0f/(_pMachine->scope_osc_freq*_pMachine->scope_osc_freq)));
		}
		break;
	case 1:
		_pMachine->scope_spec_bands = m_slider.GetPos();
		break;
	}
	m_pParent->SetFocus();	
	*pResult = 0;
}

void CGearScope::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	switch (_pMachine->scope_mode)
	{
	case 0:
		if (hold)
		{
			pos = m_slider2.GetPos()&(SCOPE_BUF_SIZE-1);
		}
		else
		{
			if (_pMachine->scope_osc_rate != m_slider2.GetPos())
			{
				_pMachine->scope_osc_rate = m_slider2.GetPos();
				KillTimer(2304+this_index);
				SetTimer(2304+this_index,_pMachine->scope_osc_rate,0);
			}
		}
		break;
	case 1:
		if (_pMachine->scope_spec_rate != m_slider2.GetPos())
		{
			_pMachine->scope_spec_rate = m_slider2.GetPos();
			KillTimer(2304+this_index);
			SetTimer(2304+this_index,_pMachine->scope_spec_rate,0);
		}
		break;
	}
	m_pParent->SetFocus();	
	*pResult = 0;
}

void CGearScope::OnMode()
{
	_pMachine->scope_mode++;
	hold = false;
	_pMachine->bCanDraw = TRUE;
	if (_pMachine->scope_mode > 2)
	{
		_pMachine->scope_mode = 0;
	}
	SetMode();
	m_pParent->SetFocus();	
}

void CGearScope::OnHold()
{
	_pMachine->bCanDraw = hold;
	hold = !hold;
	switch (_pMachine->scope_mode)
	{
	case 0:
		if (hold)
		{
			m_slider2.SetRange(0,int(Global::pConfig->_pOutputDriver->_samplesPerSec*2.0f/(_pMachine->scope_osc_freq*_pMachine->scope_osc_freq)));
			m_slider2.SetPos(0);
		}
		else
		{
			m_slider2.SetRange(10,100);
			m_slider2.SetPos(_pMachine->scope_osc_rate);
		}
	}
	m_pParent->SetFocus();	
}

void CGearScope::SetMode()
{
	char buffer[64];
	switch (_pMachine->scope_mode)
	{
	case 0:
		m_slider.SetRange(1, 148);
		m_slider.SetPos(_pMachine->scope_osc_freq);
		m_slider2.SetRange(10,100);
		m_slider2.SetPos(_pMachine->scope_osc_rate);
		sprintf(buffer,"%s - Oscilloscope",_pMachine->_editName);
		KillTimer(2304+this_index);
		SetTimer(2304+this_index,_pMachine->scope_osc_rate,0);
		break;
	case 1:
		m_slider.SetRange(4, MAX_SCOPE_BANDS);
		m_slider.SetPos(_pMachine->scope_spec_bands);
		m_slider2.SetRange(10,100);
		m_slider2.SetPos(_pMachine->scope_spec_rate);
		sprintf(buffer,"%s - Spectrum Analyzer",_pMachine->_editName);
		SetTimer(2304+this_index,_pMachine->scope_osc_rate,0);
		break;
	default:
		sprintf(buffer,"%s - Stereo Phase",_pMachine->_editName);
		break;
	}
	SetWindowText(buffer);
}

