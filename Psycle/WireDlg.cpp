// WireDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "Machine.h"
#include "WireDlg.h"
#include "Helpers.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWireDlg dialog


CWireDlg::CWireDlg(CChildView* pParent /*=NULL*/)
	: CDialog(CWireDlg::IDD, pParent)
{
	m_pParent = pParent;
	//{{AFX_DATA_INIT(CWireDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CWireDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWireDlg)
	DDX_Control(pDX, IDC_STATIC1, m_volabel);
	DDX_Control(pDX, IDC_SLIDER1, m_volslider);
	DDX_Control(pDX, IDC_SLIDER, m_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_slider2);
	DDX_Control(pDX, IDC_BUTTON, m_mode);

	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWireDlg, CDialog)
	//{{AFX_MSG_MAP(CWireDlg)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_WM_TIMER()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER, OnCustomdrawSlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_BN_CLICKED(IDC_BUTTON, OnMode)
	ON_BN_CLICKED(IDC_BUTTON2, OnHold)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWireDlg message handlers

BOOL CWireDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	scope_mode = 0;
	scope_osc_freq = 5;
	scope_osc_rate = 20;
	scope_spec_bands = 16;
	scope_spec_rate = 25;

	Inval = false;
	m_volslider.SetRange(0,256);
	m_volslider.SetTicFreq(16);
	_dstWireIndex = _pDstMachine->FindInputWire(isrcMac);

	float val;
	_pDstMachine->GetWireVolume(_dstWireIndex,val);
	invol = val;
	int t = (int)sqrtf(val*32768);
	m_volslider.SetPos(256-t);

	char buf[64];
	sprintf(buf,"[%d] %s -> %s Connection Volume", wireIndex, _pSrcMachine->_editName, _pDstMachine->_editName);
	SetWindowText(buf);

	hold = FALSE;

	memset(pSamplesL,0,sizeof(pSamplesL));
	memset(pSamplesR,0,sizeof(pSamplesR));

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
	pos = 1;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CWireDlg::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

void CWireDlg::OnCancel()
{
	_pSrcMachine->_pScopeBufferL = NULL;
	_pSrcMachine->_pScopeBufferR = NULL;
	_pSrcMachine->_scopeBufferIndex = 0;
	font.DeleteObject();
	m_pParent->WireDialog[this_index] = NULL;
	KillTimer(2304+this_index);
	DestroyWindow();
	bmpDC->DeleteObject();
	delete this;
}

void CWireDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	char buffer[32];
//	invol = (128-m_volslider.GetPos())*0.0078125f;
	invol = ((256-m_volslider.GetPos())*(256-m_volslider.GetPos()))/32768.0f;

	if (invol > 1.0f)
	{	
		sprintf(buffer,"+%.1f dB",20.0f * log10(invol)); 
	}
	else if (invol == 1.0f)
	{	
		sprintf(buffer,"0.0 dB"); 
	}
	else if (invol > 0.0f)
	{	
		sprintf(buffer,"%.1f dB",20.0f * log10(invol)); 
	}
	else 
	{				
		sprintf(buffer,"-Inf. dB"); 
	}

	m_volabel.SetWindowText(buffer);

	_pDstMachine->SetWireVolume(_dstWireIndex, invol );

	m_pParent->SetFocus();	
	*pResult = 0;
}

void CWireDlg::OnButton1() 
{
	Inval = true;
	_pSrcMachine->_connection[wireIndex] = false;
	_pSrcMachine->_numOutputs--;
	
	_pDstMachine->_inputCon[_dstWireIndex] = false;
	_pDstMachine->_numInputs--;

	OnCancel();
}



inline int CWireDlg::GetY(float f)
{
	f*=(64.0f/32768.0f);
	f=64-f;
	if (f < 1) 
	{
		clip = TRUE;
		return 1;
	}
	else if (f > 126) 
	{
		clip = TRUE;
		return 126;
	}
	return int(f);
}

void CWireDlg::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 2304+this_index )
	{
		CClientDC dc(this);

		CDC bufDC;
		bufDC.CreateCompatibleDC(&dc);
		CBitmap* oldbmp;
		oldbmp = bufDC.SelectObject(bmpDC);
		bufDC.FillSolidRect(0,0,rc.right-rc.left,rc.bottom-rc.top,0);

		switch (scope_mode)
		{
		case 0: // off
			{
				char buf[64];
				sprintf(buf,"Scope Off");

				CFont* oldFont= bufDC.SelectObject(&font);
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x606060);
				bufDC.TextOut(4, 128-14, buf);
				bufDC.SelectObject(oldFont);
				KillTimer(2304+this_index);
			}
			break;
		case 1: // oscilloscope
			{
				int freq = scope_osc_freq*scope_osc_freq;

				char buf[64];
				sprintf(buf,"Frequency %dhz Refresh %.2fhz",freq,1000.0f/scope_osc_rate);

				CFont* oldFont= bufDC.SelectObject(&font);
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x606060);
				bufDC.TextOut(4, 128-14, buf);
				bufDC.SelectObject(oldFont);

				CPen linepen(PS_SOLID, 8, 0x00303030);

				CPen *oldpen = bufDC.SelectObject(&linepen);

				// now draw our scope

				// red line if last frame was clipping
				bufDC.MoveTo(0,64);
				if (clip)
				{
					linepen.DeleteObject();
					linepen.CreatePen(PS_SOLID, 8, 0x00101040);
					bufDC.SelectObject(&linepen);
					bufDC.LineTo(255,64);
					linepen.DeleteObject();
					linepen.CreatePen(PS_SOLID, 4, 0x00101080);
				}
				// or grey line if fine
				else
				{
					bufDC.LineTo(255,64);
					linepen.DeleteObject();
					linepen.CreatePen(PS_SOLID, 4, 0x00404040);
				}
				bufDC.SelectObject(&linepen);
				bufDC.LineTo(0,64);

				linepen.DeleteObject();
				linepen.CreatePen(PS_SOLID, 2, 0xc08080);
				bufDC.SelectObject(&linepen);
				clip = FALSE;

				// ok this is a little tricky - it chases the wrapping buffer, starting at the last sample 
				// buffered and working backwards - it does it this way to minimize chances of drawing 
				// erroneous data across the buffering point

				float add = (float(Global::pConfig->_pOutputDriver->_samplesPerSec)/(float(freq)))/64.0f;

				float n = float(_pSrcMachine->_scopeBufferIndex-pos);
				bufDC.MoveTo(256,GetY(pSamplesL[int(n)&(SCOPE_BUF_SIZE-1)]*invol));
				for (int x = 256-4; x >= 0; x-=4)
				{
					n -= add;
					bufDC.LineTo(x,GetY(pSamplesL[int(n)&(SCOPE_BUF_SIZE-1)]*invol));
				}

				linepen.DeleteObject();
				linepen.CreatePen(PS_SOLID, 2, 0x80c080);
				bufDC.SelectObject(&linepen);

				n = float(_pSrcMachine->_scopeBufferIndex-pos);
				bufDC.MoveTo(256,GetY(pSamplesL[int(n)&(SCOPE_BUF_SIZE-1)]*invol));
				for (x = 256-4; x >= 0; x-=4)
				{
					n -= add;
					bufDC.LineTo(x,GetY(pSamplesL[int(n)&(SCOPE_BUF_SIZE-1)]*invol));
				}

				bufDC.SelectObject(oldpen);
				linepen.DeleteObject();

			}
			break;

		case 2: // spectrum analyzer
			{
				char buf[64];
				sprintf(buf,"%d Bands Refresh %.2fhz",scope_spec_bands,1000.0f/scope_spec_rate);

				CFont* oldFont= bufDC.SelectObject(&font);
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x606060);
				bufDC.TextOut(4, 128-14, buf);
				bufDC.SelectObject(oldFont);

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

			   // calculate our bands using same buffer chasing technique

			   int index = _pSrcMachine->_scopeBufferIndex;
			   for (int i=0;i<SCOPE_SPEC_SAMPLES;i++) 
			   { 
					index--;
					index&=(SCOPE_BUF_SIZE-1);
					float wl=(pSamplesL[index]*invol);///32768; 
					float wr=(pSamplesR[index]*invol);///32768; 
					int im = i-(SCOPE_SPEC_SAMPLES/2); 
					for(int h=0;h<scope_spec_bands;h++) 
					{ 
						float th=((F_PI/(SCOPE_SPEC_SAMPLES/2))*((float(h*h)/scope_spec_bands)+1.0f))*im; 
						float cth = cosf(th);
						float sth = sinf(th);
						aal[h]+=wl*cth; 
						bbl[h]+=wl*sth; 
						aar[h]+=wr*cth; 
						bbr[h]+=wr*sth; 
					} 
				} 
				for (int h=0;h<scope_spec_bands;h++) 
				{
					ampl[h]= sqrtf(aal[h]*aal[h]+bbl[h]*bbl[h])/(SCOPE_SPEC_SAMPLES/2); 
					ampr[h]= sqrtf(aar[h]*aar[h]+bbr[h]*bbr[h])/(SCOPE_SPEC_SAMPLES/2); 
				}
				int width = 128/scope_spec_bands;
				COLORREF cl = 0xb07070;
				COLORREF cr = 0x70b070;
				CPen linepen(PS_SOLID, width, cl);

				CPen *oldpen = bufDC.SelectObject(&linepen);

				linepen.DeleteObject();
				int x = (width/2);

				// draw our bands

				for (i = 0; i < scope_spec_bands; i++)
				{
//					int aml = 128+(width)-f2i((sqrtf(ampr[i]*16384)));
					int aml = 128+(width)-f2i(sqrtf(ampr[i]));
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

//					int amr = 128+(width)-f2i((sqrtf(ampr[i]*16384)));
					int amr = 128+(width)-f2i(sqrtf(ampr[i]));
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

					int add=0x000003*MAX_SCOPE_BANDS/scope_spec_bands;

					cl += add;
					cl -= add<<16|add<<8;
					cr += add;
					cr -= add<<16|add<<8;

					bar_heightsl[i]+=scope_spec_rate/10;
					if (bar_heightsl[i] > 128+width)
					{
						bar_heightsl[i] = 128+width;
					}
					bar_heightsr[i]+=scope_spec_rate/10;
					if (bar_heightsr[i] > 128+width)
					{
						bar_heightsr[i] = 128+width;
					}
				}
				bufDC.SelectObject(oldpen);
				linepen.DeleteObject();
			}
			break;
		case 3: // phase scope
			{
				char buf[64];
				sprintf(buf,"Phase Scope");

				CPen linepen(PS_SOLID, 8, 0x00303030);

				CPen *oldpen = bufDC.SelectObject(&linepen);

				if (clip)
				{
					linepen.DeleteObject();
					linepen.CreatePen(PS_SOLID, 8, 0x00101040);
					bufDC.SelectObject(&linepen);
				}

				// now draw our scope

				bufDC.MoveTo(32,32);
				bufDC.LineTo(128,128);
				bufDC.LineTo(128,0);
				bufDC.MoveTo(128,128);
				bufDC.LineTo(256-32,32);
				bufDC.Arc(0,0,256,256,256,128,0,128);
//				bufDC.Arc(32,32,256-32,256-32,256-32,128,32,128);
//				bufDC.Arc(64,64,256-64,256-64,256-64,128,64,128);
				bufDC.Arc(96,96,256-96,256-96,256-96,128,96,128);

				bufDC.Arc(48,48,256-48,256-48,256-48,128,48,128);

				linepen.DeleteObject();
				if (clip)
				{
					linepen.CreatePen(PS_SOLID, 4, 0x00101080);
				}
				else
				{
					linepen.CreatePen(PS_SOLID, 4, 0x00404040);
				}
				bufDC.SelectObject(&linepen);
//				bufDC.LineTo(128,128);
//				bufDC.LineTo(128,0);
//				bufDC.MoveTo(128,128);
//				bufDC.LineTo(0,0);
				bufDC.MoveTo(128,0);
				bufDC.LineTo(128,128);

//				bufDC.Arc(0,0,256,256,256,128,0,128);
//				bufDC.Arc(32,32,256-32,256-32,256-32,128,32,128);
//				bufDC.Arc(64,64,256-64,256-64,256-64,128,64,128);
//				bufDC.Arc(96,96,256-96,256-96,256-96,128,96,128);

//				bufDC.Arc(48,48,256-48,256-48,256-48,128,48,128);

				linepen.DeleteObject();
				linepen.CreatePen(PS_SOLID, 2, 0xc08080);
				bufDC.SelectObject(&linepen);
				clip = FALSE;

				// ok we need some points:

				// max vol center
				// max vol phase center
				// max vol dif phase center
				// max vol left
				// max vol dif left
				// max vol phase left
				// max vol dif phase left
				// max vol right
				// max vol dif right
				// max vol phase right
				// max vol dif phase right

				float mvc, mvpc, mvdpc, mvl, mvdl, mvpl, mvdpl, mvr, mvdr, mvpr, mvdpr;
				mvc = mvpc = mvdpc = mvl = mvdl = mvpl = mvdpl = mvr = mvdr = mvpr = mvdpr = 0.0f;

			   int index = _pSrcMachine->_scopeBufferIndex;
			   for (int i=0;i<SCOPE_SPEC_SAMPLES;i++) 
			   { 
					index--;
					index&=(SCOPE_BUF_SIZE-1);
					float wl=(pSamplesL[index]*invol);///32768; 
					float wr=(pSamplesR[index]*invol);///32768; 
					float awl=fabsf(wl);
					float awr=fabsf(wr);
					if ((wl < 0 && wr > 0) || (wl > 0 && wr < 0))
					{
						// phase difference
						if (awl > awr)
						{
							// left
							if (awl+awr > mvpl)
							{
								mvpl = awl+awr;
							}
							if (awl-awr > mvdpl)
							{
								mvdpl = awl-awr; 
							}
						}
						else if (awl < awr)
						{
							// right
							if (awr+awl > mvpr)
							{
								mvpr = awr+awl;
							}
							if (awr-awl > mvdpr)
							{
								mvdpr = awr-awl; 
							}
						}
						else
						{
							// center
							if (awr+awl > mvpc)
							{
								mvpc = awr+awl;
							}
							if (awr-awl > mvdpc)
							{
								mvdpc = awr-awl; 
							}
						}
					}
					else if (awl > awr)
					{
						// left
						if (awl > mvl)
						{
							mvl = awl;
						}
						if (awl-awr > mvdl)
						{
							mvdl = awl-awr;
						}
					}
					else if (awl < awr)
					{
						// right
						if (awr > mvr)
						{
							mvr = awr;
						}
						if (awr-awl > mvdr)
						{
							mvdr = awr-awl;
						}
					}
					else 
					{
						// center
						if (awl > mvc)
						{
							mvc = awl;
						}
					}
					if (awl > 32768.0f || awr > 32768.0f)
					{
						clip = TRUE;
					}
				} 

			   // ok we have some data, lets make some points and draw them
			   // let's move left to right phase data first

				int x,y;

				linepen.DeleteObject();
				linepen.CreatePen(PS_SOLID, 3, 0xc08080);
				bufDC.SelectObject(&linepen);

				x=f2i(sinf(-(F_PI/4.0f)-(mvdpl*F_PI/(32768.0f*4.0f)))
							*mvpl*(128.0f/32768.0f))+128;
				y=f2i(-cosf(-(F_PI/4.0f)-(mvdpl*F_PI/(32768.0f*4.0f)))
							*mvpl*(128.0f/32768.0f))+128;
				bufDC.MoveTo(x,y);
				bufDC.LineTo(128,128);
				bufDC.LineTo(128,128-f2i(mvpc*(128.0f/32768.0f)));
				bufDC.MoveTo(128,128);
				x=f2i(sinf((F_PI/4.0f)+(mvdpr*F_PI/(32768.0f*4.0f)))
							*mvpr*(128.0f/32768.0f))+128;
				y=f2i(-cosf((F_PI/4.0f)+(mvdpr*F_PI/(32768.0f*4.0f)))
							*mvpr*(128.0f/32768.0f))+128;
				bufDC.LineTo(x,y);
								
				// panning data
				linepen.DeleteObject();
				linepen.CreatePen(PS_SOLID, 3, 0x80c080);
				bufDC.SelectObject(&linepen);

				x=f2i(sinf(-(mvdl*F_PI/(32768.0f*4.0f)))
							*mvl*(128.0f/32768.0f))+128;
				y=f2i(-cosf(-(mvdl*F_PI/(32768.0f*4.0f)))
							*mvl*(128.0f/32768.0f))+128;
				bufDC.MoveTo(x,y);
				bufDC.LineTo(128,128);
				bufDC.LineTo(128,128-f2i(mvc*(128.0f/32768.0f)));
				bufDC.MoveTo(128,128);
				x=f2i(sinf((mvdr*F_PI/(32768.0f*4.0f)))
							*mvr*(128.0f/32768.0f))+128;
				y=f2i(-cosf((mvdr*F_PI/(32768.0f*4.0f)))
							*mvr*(128.0f/32768.0f))+128;
				bufDC.LineTo(x,y);

				CFont* oldFont= bufDC.SelectObject(&font);
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x606060);
				bufDC.TextOut(4, 128-14, buf);
				bufDC.SelectObject(oldFont);
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

void CWireDlg::OnCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	switch (scope_mode)
	{
	case 1:
		scope_osc_freq = m_slider.GetPos();
		if (hold)
		{
			m_slider2.SetRange(1,1+int(Global::pConfig->_pOutputDriver->_samplesPerSec*2.0f/(scope_osc_freq*scope_osc_freq)));
		}
		break;
	case 2:
		scope_spec_bands = m_slider.GetPos();
		break;
	}
	m_pParent->SetFocus();	
	*pResult = 0;
}

void CWireDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	switch (scope_mode)
	{
	case 1:
		if (hold)
		{
			pos = m_slider2.GetPos()&(SCOPE_BUF_SIZE-1);
		}
		else
		{
			pos = 1;
			if (scope_osc_rate != m_slider2.GetPos())
			{
				scope_osc_rate = m_slider2.GetPos();
				KillTimer(2304+this_index);
				SetTimer(2304+this_index,scope_osc_rate,0);
			}
		}
		break;
	case 2:
		if (scope_spec_rate != m_slider2.GetPos())
		{
			scope_spec_rate = m_slider2.GetPos();
			KillTimer(2304+this_index);
			SetTimer(2304+this_index,scope_spec_rate,0);
		}
		break;
	}
	m_pParent->SetFocus();	
	*pResult = 0;
}

void CWireDlg::OnMode()
{
	scope_mode++;
	if (scope_mode > 3)
	{
		scope_mode = 0;
	}
	SetMode();
	m_pParent->SetFocus();	
}

void CWireDlg::OnHold()
{
	hold = !hold;
	pos = 1;
	switch (scope_mode)
	{
	case 1:
		if (hold)
		{
			m_slider2.SetRange(1,1+int(Global::pConfig->_pOutputDriver->_samplesPerSec*2.0f/(scope_osc_freq*scope_osc_freq)));
			m_slider2.SetPos(1);
		}
		else
		{
			m_slider2.SetRange(10,100);
			m_slider2.SetPos(scope_osc_rate);
		}
	}
	if (hold)
	{
		_pSrcMachine->_pScopeBufferL = NULL;
		_pSrcMachine->_pScopeBufferR = NULL;
	}
	else
	{
		_pSrcMachine->_pScopeBufferL = pSamplesL;
		_pSrcMachine->_pScopeBufferR = pSamplesR;
	}
	m_pParent->SetFocus();	
}

void CWireDlg::SetMode()
{
	char buffer[64];
	switch (scope_mode)
	{
	case 0:
		KillTimer(2304+this_index);
		_pSrcMachine->_pScopeBufferL = NULL;
		_pSrcMachine->_pScopeBufferR = NULL;
		sprintf(buffer,"Scope Mode");
		SetTimer(2304+this_index,scope_osc_rate,0);
		break;
	case 1:
		KillTimer(2304+this_index);
		m_slider.SetRange(1, 148);
		m_slider.SetPos(scope_osc_freq);
		m_slider2.SetRange(10,100);
		m_slider2.SetPos(scope_osc_rate);
		sprintf(buffer,"Oscilloscope");
		_pSrcMachine->_pScopeBufferL = pSamplesL;
		_pSrcMachine->_pScopeBufferR = pSamplesR;
		SetTimer(2304+this_index,scope_osc_rate,0);
		break;
	case 2:
		KillTimer(2304+this_index);
		{
			for (int i = 0; i < MAX_SCOPE_BANDS; i++)
			{
				bar_heightsl[i]=256;
				bar_heightsr[i]=256;
			}
		}
		m_slider.SetRange(4, MAX_SCOPE_BANDS);
		m_slider.SetPos(scope_spec_bands);
		m_slider2.SetRange(10,100);
		m_slider2.SetPos(scope_spec_rate);
		sprintf(buffer,"Spectrum Analyzer");
		_pSrcMachine->_pScopeBufferL = pSamplesL;
		_pSrcMachine->_pScopeBufferR = pSamplesR;
		SetTimer(2304+this_index,scope_osc_rate,0);
		break;
	case 3:
		KillTimer(2304+this_index);
		_pSrcMachine->_pScopeBufferL = pSamplesL;
		_pSrcMachine->_pScopeBufferR = pSamplesR;
		sprintf(buffer,"Stereo Phase");
		SetTimer(2304+this_index,scope_osc_rate,0);
		break;
	}
	m_mode.SetWindowText(buffer);
	hold = false;
	pos = 1;
}

