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
	scope_peak_rate = 20;
	scope_osc_freq = 5;
	scope_osc_rate = 20;
	scope_spec_bands = 16;
	scope_spec_rate = 25;
	scope_phase_rate = 20;

	Inval = false;
	m_volslider.SetRange(0,256);
	m_volslider.SetTicFreq(16);
	_dstWireIndex = _pDstMachine->FindInputWire(isrcMac);

	float val;
	_pDstMachine->GetWireVolume(_dstWireIndex,val);
	invol = val;
	int t = (int)sqrtf(val*16384);
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

	if ( _pSrcMachine->_type == MACH_VST || _pSrcMachine->_type == MACH_VSTFX ) // native to VST, divide.
	{
		mult = 32768.0f;
	}
	else												// native to native, no need to convert.
	{
		mult = 1.0f;
	}	

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
	invol = ((256-m_volslider.GetPos())*(256-m_volslider.GetPos()))/16384.0f;

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
				sprintf(buf,"Refresh %.2fhz",1000.0f/scope_peak_rate);

				CFont* oldFont= bufDC.SelectObject(&font);
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x505050);
				bufDC.TextOut(4, 128-14, buf);

				CPen linepen(PS_SOLID, 1, 0x00606060);

				CPen *oldpen = bufDC.SelectObject(&linepen);

				bufDC.MoveTo(32+24,32-8);
				bufDC.LineTo(256-32-24,32-8);
				sprintf(buf,"+6 db");
				bufDC.TextOut(32-1, 32-8-6, buf);
				bufDC.TextOut(256-32-22, 32-8-6, buf);

				bufDC.MoveTo(32+24,32+44);
				bufDC.LineTo(256-32-24,32+44);
				sprintf(buf,"-6 db");
				bufDC.TextOut(32-1+4, 32+44-6, buf);
				bufDC.TextOut(256-32-22, 32+44-6, buf);

				bufDC.MoveTo(32+24,32+44+16);
				bufDC.LineTo(256-32-24,32+44+16);
				sprintf(buf,"-12 db");
				bufDC.TextOut(32-1-6+4, 32+44+16-6, buf);
				bufDC.TextOut(256-32-22, 32+44+16-6, buf);

				bufDC.MoveTo(32+24,32+44+16+18);
				bufDC.LineTo(256-32-24,32+44+16+18);
				sprintf(buf,"-24 db");
				bufDC.TextOut(32-1-6+4, 32+44+16+18-6, buf);
				bufDC.TextOut(256-32-22, 32+44+16+18-6, buf);

				linepen.DeleteObject();
				bufDC.SetTextColor(0x707070);
				linepen.CreatePen(PS_SOLID, 1, 0x707070);
				bufDC.SelectObject(&linepen);

				bufDC.MoveTo(32+24,32+23);
				bufDC.LineTo(256-32-24,32+23);
				sprintf(buf,"0 db");
				bufDC.TextOut(32-1+6, 32+23-6, buf);
				bufDC.TextOut(256-32-22, 32+23-6, buf);

				linepen.DeleteObject();

				// now draw our scope

				int index = _pSrcMachine->_scopeBufferIndex;
				float tawl,tawr;
				tawl = 0;
				tawr = 0;
				for (int i=0;i<SCOPE_SPEC_SAMPLES;i++) 
				{ 
					index--;
					index&=(SCOPE_BUF_SIZE-1);
					float awl=fabsf(pSamplesL[index]*invol*mult);///32768; 
					float awr=fabsf(pSamplesR[index]*invol*mult);///32768; 

					if (awl>tawl)
					{
						tawl = awl;
					}
					if (awl>peak2L)
					{
						peak2L = awl;
						peakLifeL = 2048;
						peakL = awl;
					}
					else if (awl>peakL)
					{
						peakL = awl;
					}

					if (awr>tawr)
					{
						tawr = awr;
					}
					if (awr>peak2R)
					{
						peak2R = awr;
						peakLifeR = 2048;
						peakR = awr;
					}
					else if (awr>peakR)
					{
						peakR = awr;
					}
				}

				// ok draw our meters

				int y;

				LOGBRUSH lb;
				lb.lbHatch = 0;
				lb.lbStyle = 0;

				y = 128-f2i(sqrtf(peak2L/6));
				if (y < 0)
				{
					y = 0;
				}

				int cd = (peakLifeL/17);
				lb.lbColor = (0x804000+(128-y))+(cd<<16|cd<<8|cd);
				linepen.CreatePen(PS_GEOMETRIC|PS_ENDCAP_FLAT,48,&lb,0,0);
				bufDC.SelectObject(&linepen);
				bufDC.MoveTo(128-32,y);

				y = 128-f2i(sqrtf(peakL/6));
				if (y < 0)
				{
					y = 0;
				}

				bufDC.LineTo(128-32,y);
				linepen.DeleteObject();
				lb.lbColor = 0xb07030+(128-y);
				linepen.CreatePen(PS_GEOMETRIC|PS_ENDCAP_FLAT,48,&lb,0,0);
				bufDC.SelectObject(&linepen);

				y = 128-f2i(sqrtf(tawl/6));
				if (y < 0)
				{
					y = 0;
				}

				bufDC.LineTo(128-32,y);
				linepen.DeleteObject();
				lb.lbColor = 0xd09048+(128-y);
				linepen.CreatePen(PS_GEOMETRIC|PS_ENDCAP_FLAT,48,&lb,0,0);
				bufDC.SelectObject(&linepen);
				bufDC.LineTo(128-32,128);
				linepen.DeleteObject();

				y = 128-f2i(sqrtf(peak2R/6));
				if (y < 0)
				{
					y = 0;
				}

				cd = (peakLifeR/17);
				lb.lbColor = (0x408000+(128-y))+(cd<<16|cd<<8|cd);
				linepen.CreatePen(PS_GEOMETRIC|PS_ENDCAP_FLAT,48,&lb,0,0);
				bufDC.SelectObject(&linepen);
				bufDC.MoveTo(192-32,y);

				y = 128-f2i(sqrtf(peakR/6));
				if (y < 0)
				{
					y = 0;
				}

				bufDC.LineTo(192-32,y);
				linepen.DeleteObject();
				lb.lbColor = 0x70b030+(128-y);
				linepen.CreatePen(PS_GEOMETRIC|PS_ENDCAP_FLAT,48,&lb,0,0);
				bufDC.SelectObject(&linepen);

				y = 128-f2i(sqrtf(tawr/6));
				if (y < 0)
				{
					y = 0;
				}

				bufDC.LineTo(192-32,y);
				linepen.DeleteObject();
				lb.lbColor = 0x90d048+(128-y);
				linepen.CreatePen(PS_GEOMETRIC|PS_ENDCAP_FLAT,48,&lb,0,0);
				bufDC.SelectObject(&linepen);
				bufDC.LineTo(192-32,128);
				linepen.DeleteObject();

				if (!hold)
				{
					peakL -= (scope_peak_rate*scope_peak_rate);///2;
					peakR -= (scope_peak_rate*scope_peak_rate);///2;
					peakLifeL -= scope_peak_rate;
					peakLifeR -= scope_peak_rate;
					if (peakLifeL < 0)
					{
						peak2L = 0;
					}
					if (peakLifeR < 0)
					{
						peak2R = 0;
					}
				}

				bufDC.SelectObject(oldpen);
				bufDC.SelectObject(oldFont);
				linepen.DeleteObject();
			}
			break;
		case 1: // oscilloscope
			{
				int freq = scope_osc_freq*scope_osc_freq;

				char buf[64];
				sprintf(buf,"Frequency %dhz Refresh %.2fhz",freq,1000.0f/scope_osc_rate);

				CFont* oldFont= bufDC.SelectObject(&font);
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x505050);
				bufDC.TextOut(4, 128-14, buf);

				CPen linepen(PS_SOLID, 2, 0x00202020);

				CPen *oldpen = bufDC.SelectObject(&linepen);

				// now draw our scope

				// red line if last frame was clipping
				if (clip)
				{
					linepen.DeleteObject();
					linepen.CreatePen(PS_SOLID, 2, 0x00202030);
					bufDC.SelectObject(&linepen);
					bufDC.MoveTo(0,32);
					bufDC.LineTo(255,32);
					bufDC.MoveTo(0,96);
					bufDC.LineTo(255,96);
					linepen.DeleteObject();
					linepen.CreatePen(PS_SOLID, 8, 0x00202040);
					bufDC.SelectObject(&linepen);
					bufDC.MoveTo(0,64);
					bufDC.LineTo(255,64);
					linepen.DeleteObject();
					linepen.CreatePen(PS_SOLID, 4, 0x00101080);
				}
				// or grey line if fine
				else
				{
					bufDC.MoveTo(0,32);
					bufDC.LineTo(255,32);
					bufDC.MoveTo(0,96);
					bufDC.LineTo(255,96);
					linepen.DeleteObject();
					linepen.CreatePen(PS_SOLID, 4, 0x00404040);
					bufDC.MoveTo(0,64);
					bufDC.LineTo(255,64);
					linepen.DeleteObject();
					linepen.CreatePen(PS_SOLID, 4, 0x00404040);
				}
				bufDC.SelectObject(&linepen);
				bufDC.MoveTo(0,64);
				bufDC.LineTo(255,64);

				linepen.DeleteObject();
				linepen.CreatePen(PS_SOLID, 2, 0xc08080);
				bufDC.SelectObject(&linepen);
				clip = FALSE;

				// ok this is a little tricky - it chases the wrapping buffer, starting at the last sample 
				// buffered and working backwards - it does it this way to minimize chances of drawing 
				// erroneous data across the buffering point

				float add = (float(Global::pConfig->_pOutputDriver->_samplesPerSec)/(float(freq)))/64.0f;

				float n = float(_pSrcMachine->_scopeBufferIndex-pos);
				bufDC.MoveTo(256,GetY(pSamplesL[int(n)&(SCOPE_BUF_SIZE-1)]*invol*mult));
				for (int x = 256-4; x >= 0; x-=4)
				{
					n -= add;
					bufDC.LineTo(x,GetY(pSamplesL[int(n)&(SCOPE_BUF_SIZE-1)]*invol*mult));
//					bufDC.LineTo(x,GetY(32768/2));
				}

				linepen.DeleteObject();
				linepen.CreatePen(PS_SOLID, 2, 0x80c080);
				bufDC.SelectObject(&linepen);

				n = float(_pSrcMachine->_scopeBufferIndex-pos);
				bufDC.MoveTo(256,GetY(pSamplesL[int(n)&(SCOPE_BUF_SIZE-1)]*invol*mult));
				for (x = 256-4; x >= 0; x-=4)
				{
					n -= add;
					bufDC.LineTo(x,GetY(pSamplesL[int(n)&(SCOPE_BUF_SIZE-1)]*invol*mult));
				}

				bufDC.SelectObject(oldpen);
				bufDC.SelectObject(oldFont);
				linepen.DeleteObject();
			}
			break;

		case 2: // spectrum analyzer
			{
				char buf[64];
				sprintf(buf,"%d Bands Refresh %.2fhz",scope_spec_bands,1000.0f/scope_spec_rate);

				CFont* oldFont= bufDC.SelectObject(&font);

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
					float wl=(pSamplesL[index]*invol*mult);///32768; 
					float wr=(pSamplesR[index]*invol*mult);///32768; 
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
				COLORREF cl = 0xa06060;
				COLORREF cr = 0x60a060;
				CPen linepen(PS_SOLID, width, cl);

				CPen *oldpen = bufDC.SelectObject(&linepen);

				linepen.DeleteObject();
				int x = (width/2);

				// draw our bands

				LOGBRUSH lb;
				lb.lbHatch = 0;
				lb.lbStyle = 0;

				for (i = 0; i < scope_spec_bands; i++)
				{
//					int aml = 128-f2i((sqrtf(ampr[i]*16384)));
					int aml = 128-f2i(sqrtf(ampr[i]));
					if (aml < 0)
					{
						aml = 0;
					}
					if (aml < bar_heightsl[i])
					{
						bar_heightsl[i]=aml;
					}

					lb.lbColor = cl;
					linepen.CreatePen(PS_GEOMETRIC|PS_ENDCAP_FLAT,width,&lb,0,0);
					bufDC.SelectObject(&linepen);
					bufDC.MoveTo(x,bar_heightsl[i]);
					bufDC.LineTo(x,aml);
					linepen.DeleteObject();

					lb.lbColor = cl+0x303030;
					linepen.CreatePen(PS_GEOMETRIC|PS_ENDCAP_FLAT,width,&lb,0,0);
					bufDC.SelectObject(&linepen);
					bufDC.LineTo(x,128);
					linepen.DeleteObject();

					x+=width;

//					int amr = 128-f2i((sqrtf(ampr[i]*16384)));
					int amr = 128-f2i(sqrtf(ampr[i]));
					if (amr < 0)
					{
						amr = 0;
					}
					if (amr < bar_heightsr[i])
					{
						bar_heightsr[i]=amr;
					}

					lb.lbColor = cr;
					linepen.CreatePen(PS_GEOMETRIC|PS_ENDCAP_FLAT ,width,&lb,0,0);
					bufDC.SelectObject(&linepen);
					bufDC.MoveTo(x,bar_heightsr[i]);
					bufDC.LineTo(x,amr);
					linepen.DeleteObject();

					lb.lbColor = cr+0x303030;
					linepen.CreatePen(PS_GEOMETRIC|PS_ENDCAP_FLAT ,width,&lb,0,0);
					bufDC.SelectObject(&linepen);
					bufDC.LineTo(x,128);
					linepen.DeleteObject();

					x+=width;

					int add=0x000001*MAX_SCOPE_BANDS/scope_spec_bands;

					cl += add;
					cl -= add<<16|add<<8;
					cr += add;
					cr -= add<<16|add<<8;

					if (!hold)
					{
						bar_heightsl[i]+=scope_spec_rate/10;
						if (bar_heightsl[i] > 128)
						{
							bar_heightsl[i] = 128+1;
						}
						bar_heightsr[i]+=scope_spec_rate/10;
						if (bar_heightsr[i] > 128)
						{
							bar_heightsr[i] = 128+1;
						}
					}
				}
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x505050);
				bufDC.TextOut(4, 128-14, buf);

				bufDC.SelectObject(oldFont);
				bufDC.SelectObject(oldpen);
				linepen.DeleteObject();
			}
			break;
		case 3: // phase scope
			{
				CPen linepen(PS_SOLID, 8, 0x00303030);

				CPen *oldpen = bufDC.SelectObject(&linepen);

				if (clip)
				{
					linepen.DeleteObject();
					linepen.CreatePen(PS_SOLID, 8, 0x00202040);
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
				bufDC.MoveTo(32,32);
				bufDC.LineTo(128,128);
				bufDC.LineTo(128,0);
				bufDC.MoveTo(128,128);
				bufDC.LineTo(256-32,32);

//				bufDC.Arc(0,0,256,256,256,128,0,128);
//				bufDC.Arc(32,32,256-32,256-32,256-32,128,32,128);
//				bufDC.Arc(64,64,256-64,256-64,256-64,128,64,128);
//				bufDC.Arc(96,96,256-96,256-96,256-96,128,96,128);

//				bufDC.Arc(48,48,256-48,256-48,256-48,128,48,128);

				linepen.DeleteObject();
				linepen.CreatePen(PS_SOLID, 2, 0xc08080);
				bufDC.SelectObject(&linepen);
				clip = FALSE;

				char buf[64];
				sprintf(buf,"Refresh %.2fhz",1000.0f/scope_phase_rate);

				CFont* oldFont= bufDC.SelectObject(&font);
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x505050);
				bufDC.TextOut(4, 128-14, buf);

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

				float mvc, mvpc, mvl, mvdl, mvpl, mvdpl, mvr, mvdr, mvpr, mvdpr;
				mvc = mvpc = mvl = mvdl = mvpl = mvdpl = mvr = mvdr = mvpr = mvdpr = 0.0f;

			   int index = _pSrcMachine->_scopeBufferIndex;
			   for (int i=0;i<SCOPE_SPEC_SAMPLES;i++) 
			   { 
					index--;
					index&=(SCOPE_BUF_SIZE-1);
					float wl=(pSamplesL[index]*invol*mult);///32768; 
					float wr=(pSamplesR[index]*invol*mult);///32768; 
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
//							if (awr-awl > mvdpc)
//							{
//								mvdpc = awr-awl; 
//							}
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

				if (mvpl > o_mvpl)
				{
					o_mvpl = mvpl;
					o_mvdpl = mvdpl;
				}
				if (mvpc > o_mvpc)
				{
					o_mvpc = mvpc;
				}
				if (mvpr > o_mvpr)
				{
					o_mvpr = mvpr;
					o_mvdpr = mvdpr;
				}

				if (mvl > o_mvl)
				{
					o_mvl = mvl;
					o_mvdl = mvdl;
				}
				if (mvc > o_mvc)
				{
					o_mvc = mvc;
				}
				if (mvr > o_mvr)
				{
					o_mvr = mvr;
					o_mvdr = mvdr;
				}

				int x,y;

				linepen.DeleteObject();
				linepen.CreatePen(PS_SOLID, 3, 0x806060);
				bufDC.SelectObject(&linepen);

				x=f2i(sinf(-(F_PI/4.0f)-(o_mvdpl*F_PI/(32768.0f*4.0f)))
							*o_mvpl*(128.0f/32768.0f))+128;
				y=f2i(-cosf(-(F_PI/4.0f)-(o_mvdpl*F_PI/(32768.0f*4.0f)))
							*o_mvpl*(128.0f/32768.0f))+128;
				bufDC.MoveTo(x,y);
				bufDC.LineTo(128,128);
				bufDC.LineTo(128,128-f2i(o_mvpc*(128.0f/32768.0f)));
				bufDC.MoveTo(128,128);
				x=f2i(sinf((F_PI/4.0f)+(o_mvdpr*F_PI/(32768.0f*4.0f)))
							*o_mvpr*(128.0f/32768.0f))+128;
				y=f2i(-cosf((F_PI/4.0f)+(o_mvdpr*F_PI/(32768.0f*4.0f)))
							*o_mvpr*(128.0f/32768.0f))+128;
				bufDC.LineTo(x,y);
								
				// panning data
				linepen.DeleteObject();
				linepen.CreatePen(PS_SOLID, 3, 0x608060);
				bufDC.SelectObject(&linepen);

				x=f2i(sinf(-(o_mvdl*F_PI/(32768.0f*4.0f)))
							*o_mvl*(128.0f/32768.0f))+128;
				y=f2i(-cosf(-(o_mvdl*F_PI/(32768.0f*4.0f)))
							*o_mvl*(128.0f/32768.0f))+128;
				bufDC.MoveTo(x,y);
				bufDC.LineTo(128,128);
				bufDC.LineTo(128,128-f2i(o_mvc*(128.0f/32768.0f)));
				bufDC.MoveTo(128,128);
				x=f2i(sinf((o_mvdr*F_PI/(32768.0f*4.0f)))
							*o_mvr*(128.0f/32768.0f))+128;
				y=f2i(-cosf((o_mvdr*F_PI/(32768.0f*4.0f)))
							*o_mvr*(128.0f/32768.0f))+128;
				bufDC.LineTo(x,y);


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

				if (!hold)
				{
					o_mvpl -= scope_phase_rate*32.0f;
					o_mvpc -= scope_phase_rate*32.0f;
					o_mvpr -= scope_phase_rate*32.0f;
					o_mvl -= scope_phase_rate*32.0f;
					o_mvc -= scope_phase_rate*32.0f;
					o_mvr -= scope_phase_rate*32.0f;
				}
				bufDC.SelectObject(oldFont);
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
	case 0:
		if (scope_peak_rate != m_slider2.GetPos())
		{
			scope_peak_rate = m_slider2.GetPos();
			KillTimer(2304+this_index);
			SetTimer(2304+this_index,scope_peak_rate,0);
		}
		break;
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
	case 3:
		if (scope_phase_rate != m_slider2.GetPos())
		{
			scope_phase_rate = m_slider2.GetPos();
			KillTimer(2304+this_index);
			SetTimer(2304+this_index,scope_phase_rate,0);
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
		// vu
		KillTimer(2304+this_index);
		m_slider2.SetRange(10,100);
		m_slider2.SetPos(scope_peak_rate);
		sprintf(buffer,"Scope Mode");
		peakL = peakR = peak2L = peak2R = 0.0f;
		_pSrcMachine->_pScopeBufferL = pSamplesL;
		_pSrcMachine->_pScopeBufferR = pSamplesR;
		SetTimer(2304+this_index,scope_peak_rate,0);
		break;
	case 1:
		// oscilloscope
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
		// spectrum analyzer
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
		// phase
		KillTimer(2304+this_index);
		_pSrcMachine->_pScopeBufferL = pSamplesL;
		_pSrcMachine->_pScopeBufferR = pSamplesR;
		sprintf(buffer,"Stereo Phase");
		o_mvc = o_mvpc = o_mvl = o_mvdl = o_mvpl = o_mvdpl = o_mvr = o_mvdr = o_mvpr = o_mvdpr = 0.0f;
		m_slider2.SetRange(10,100);
		m_slider2.SetPos(scope_phase_rate);
		SetTimer(2304+this_index,scope_phase_rate,0);
		break;
	}
	m_mode.SetWindowText(buffer);
	hold = false;
	pos = 1;
}

