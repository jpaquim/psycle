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
	bufBM = new CBitmap;
	bufBM->CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);
	clearBM = new CBitmap;
	clearBM->CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);

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
	KillTimer(2304+this_index);
	_pSrcMachine->_pScopeBufferL = NULL;
	_pSrcMachine->_pScopeBufferR = NULL;
	_pSrcMachine->_scopeBufferIndex = 0;
	m_pParent->WireDialog[this_index] = NULL;
	DestroyWindow();
	font.DeleteObject();
	bufBM->DeleteObject();
	clearBM->DeleteObject();
	linepenL.DeleteObject();
	linepenR.DeleteObject();
	linepenbL.DeleteObject();
	linepenbR.DeleteObject();
	delete this;
}

void CWireDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	char buffer[32];
//	invol = (128-m_volslider.GetPos())*0.0078125f;
	invol = ((256-m_volslider.GetPos())*(256-m_volslider.GetPos()))/16384.0f;

	if (invol > 1.0f)
	{	
		sprintf(buffer,"+%.1f dB\n%.2f%%",20.0f * log10(invol),invol*100); 
	}
	else if (invol == 1.0f)
	{	
		sprintf(buffer,"0.0 dB\n100.00%%"); 
	}
	else if (invol > 0.0f)
	{	
		sprintf(buffer,"%.1f dB\n%.2f%%",20.0f * log10(invol),invol*100); 
	}
	else 
	{				
		sprintf(buffer,"-Inf. dB\n0.00%%"); 
	}

	m_volabel.SetWindowText(buffer);

	_pDstMachine->SetWireVolume(_dstWireIndex, invol );

//	m_pParent->SetFocus();	
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
		oldbmp = bufDC.SelectObject(bufBM);

		CDC clrDC;
		clrDC.CreateCompatibleDC(&dc);
		CBitmap* oldbmp2;
		oldbmp2 = clrDC.SelectObject(clearBM);

		bufDC.BitBlt(0,0,256,128,&clrDC,0,0,SRCCOPY);

		clrDC.SelectObject(oldbmp2);
		clrDC.DeleteDC();

		switch (scope_mode)
		{
		case 0: // off
			{
				// now draw our scope

				int index = _pSrcMachine->_scopeBufferIndex;
				float tawl,tawr;
				tawl = 0;
				tawr = 0;
				for (int i=0;i<SCOPE_SPEC_SAMPLES;i++) 
				{ 
					index--;
					index&=(SCOPE_BUF_SIZE-1);
					float awl=fabsf(pSamplesL[index]*invol*mult*_pSrcMachine->_lVol);///32768; 
					float awr=fabsf(pSamplesR[index]*invol*mult*_pSrcMachine->_rVol);///32768; 

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

				RECT rect;

				int y;

				y = 128-f2i(sqrtf(peak2L/6));
				if (y < 0)
				{
					y = 0;
				}

				int cd = (peakLifeL/17);
				COLORREF lbColor = 0x804000+(128-y)+(cd<<16|cd<<8|cd)+((y<32+23)?32:0);
				rect.left = 128-32-24;
				rect.right = rect.left+48;
				rect.top = y;

				y = 128-f2i(sqrtf(peakL/6));
				if (y < 0)
				{
					y = 0;
				}

				rect.bottom = y;
				bufDC.FillSolidRect(&rect,lbColor);

				rect.top = rect.bottom;
				lbColor = 0xb07030+(128-y)+((y<32+23)?32:0);

				y = 128-f2i(sqrtf(tawl/6));
				if (y < 0)
				{
					y = 0;
				}

				rect.bottom = y;
				bufDC.FillSolidRect(&rect,lbColor);

				rect.top = rect.bottom;
				rect.bottom = 128;
				lbColor = 0xd09048+(128-y)+((y<32+23)?32:0);

				bufDC.FillSolidRect(&rect,lbColor);

				y = 128-f2i(sqrtf(peak2R/6));
				if (y < 0)
				{
					y = 0;
				}

				cd = (peakLifeR/17);
				lbColor = 0x408000+(128-y)+(cd<<16|cd<<8|cd)+((y<32+23)?32:0);

				rect.left = 128+32-24;
				rect.right = rect.left+48;
				rect.top = y;

				y = 128-f2i(sqrtf(peakR/6));
				if (y < 0)
				{
					y = 0;
				}

				rect.bottom = y;
				bufDC.FillSolidRect(&rect,lbColor);

				rect.top = rect.bottom;
				lbColor = 0x70b030+(128-y)+((y<32+23)?32:0);

				y = 128-f2i(sqrtf(tawr/6));
				if (y < 0)
				{
					y = 0;
				}

				rect.bottom = y;
				bufDC.FillSolidRect(&rect,lbColor);

				rect.top = rect.bottom;
				rect.bottom = 128;
				lbColor = 0x90d048+(128-y)+((y<32+23)?32:0);
				bufDC.FillSolidRect(&rect,lbColor);

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

				char buf[64];
				sprintf(buf,"Refresh %.2fhz",1000.0f/scope_peak_rate);
				CFont* oldFont= bufDC.SelectObject(&font);
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x505050);
				bufDC.TextOut(4, 128-14, buf);
				bufDC.SelectObject(oldFont);
			}
			break;
		case 1: // oscilloscope
			{
				int freq = scope_osc_freq*scope_osc_freq;

				// now draw our scope

				// red line if last frame was clipping
				if (clip)
				{
					RECT rect;

					rect.left = 0;
					rect.right = 256;
					rect.top = 32;
					rect.bottom = rect.top+2;

					bufDC.FillSolidRect(&rect,0x00202040);

					rect.top = 95;
					rect.bottom = rect.top+2;
					bufDC.FillSolidRect(&rect,0x00202040);

					rect.top = 64-4;
					rect.bottom = rect.top+8;
					bufDC.FillSolidRect(&rect,0x00202050);

					rect.top = 64-2;
					rect.bottom = rect.top+4;
					bufDC.FillSolidRect(&rect,0x00101080);

					clip = FALSE;
				}

				CPen *oldpen = bufDC.SelectObject(&linepenL);

				// ok this is a little tricky - it chases the wrapping buffer, starting at the last sample 
				// buffered and working backwards - it does it this way to minimize chances of drawing 
				// erroneous data across the buffering point

				float add = (float(Global::pConfig->_pOutputDriver->_samplesPerSec)/(float(freq)))/64.0f;

				float n = float(_pSrcMachine->_scopeBufferIndex-pos);
				bufDC.MoveTo(256,GetY(pSamplesL[int(n)&(SCOPE_BUF_SIZE-1)]*invol*mult*_pSrcMachine->_lVol));
				for (int x = 256-2; x >= 0; x-=2)
				{
					n -= add;
					bufDC.LineTo(x,GetY(pSamplesL[int(n)&(SCOPE_BUF_SIZE-1)]*invol*mult*_pSrcMachine->_lVol));
//					bufDC.LineTo(x,GetY(32768/2));
				}
				bufDC.SelectObject(&linepenR);

				n = float(_pSrcMachine->_scopeBufferIndex-pos);
				bufDC.MoveTo(256,GetY(pSamplesR[int(n)&(SCOPE_BUF_SIZE-1)]*invol*mult*_pSrcMachine->_rVol));
				for (x = 256-2; x >= 0; x-=2)
				{
					n -= add;
					bufDC.LineTo(x,GetY(pSamplesR[int(n)&(SCOPE_BUF_SIZE-1)]*invol*mult*_pSrcMachine->_rVol));
				}

				bufDC.SelectObject(oldpen);

				char buf[64];
				sprintf(buf,"Frequency %dhz Refresh %.2fhz",freq,1000.0f/scope_osc_rate);
				CFont* oldFont= bufDC.SelectObject(&font);
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x505050);
				bufDC.TextOut(4, 128-14, buf);
				bufDC.SelectObject(oldFont);
			}
			break;

		case 2: // spectrum analyzer
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

			   // calculate our bands using same buffer chasing technique

			   int index = _pSrcMachine->_scopeBufferIndex;
			   for (int i=0;i<SCOPE_SPEC_SAMPLES;i++) 
			   { 
					index--;
					index&=(SCOPE_BUF_SIZE-1);
					float wl=(pSamplesL[index]*invol*mult*_pSrcMachine->_lVol);///32768; 
					float wr=(pSamplesR[index]*invol*mult*_pSrcMachine->_rVol);///32768; 
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

				RECT rect;
				rect.left = 0;

				// draw our bands

				for (i = 0; i < scope_spec_bands; i++)
				{
					int aml = 128-f2i(sqrtf(ampl[i]));
					if (aml < 0)
					{
						aml = 0;
					}
					if (aml < bar_heightsl[i])
					{
						bar_heightsl[i]=aml;
					}

					rect.right = rect.left+width;
					rect.top = bar_heightsl[i];
					rect.bottom = aml;
					bufDC.FillSolidRect(&rect,cl);

					rect.top = rect.bottom;
					rect.bottom = 128;
					bufDC.FillSolidRect(&rect,cl+0x303030);
					
					rect.left+=width;

					int amr = 128-f2i(sqrtf(ampr[i]));
					if (amr < 0)
					{
						amr = 0;
					}
					if (amr < bar_heightsr[i])
					{
						bar_heightsr[i]=amr;
					}

					rect.right = rect.left+width;
					rect.top = bar_heightsr[i];
					rect.bottom = amr;
					bufDC.FillSolidRect(&rect,cr);

					rect.top = rect.bottom;
					rect.bottom = 128;
					bufDC.FillSolidRect(&rect,cr+0x303030);

					rect.left+=width;

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
				char buf[64];
				sprintf(buf,"%d Bands Refresh %.2fhz",scope_spec_bands,1000.0f/scope_spec_rate);
				CFont* oldFont= bufDC.SelectObject(&font);
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x505050);
				bufDC.TextOut(4, 128-14, buf);
				bufDC.SelectObject(oldFont);
			}
			break;
		case 3: // phase scope
			{

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
					float wl=(pSamplesL[index]*invol*mult*_pSrcMachine->_lVol);///32768; 
					float wr=(pSamplesR[index]*invol*mult*_pSrcMachine->_rVol);///32768; 
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

				CPen* oldpen = bufDC.SelectObject(&linepenbL);

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
				bufDC.SelectObject(&linepenbR);

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

				bufDC.SelectObject(&linepenL);

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
				bufDC.SelectObject(&linepenR);

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
				bufDC.SelectObject(oldpen);

				char buf[64];
				sprintf(buf,"Refresh %.2fhz",1000.0f/scope_phase_rate);
				CFont* oldFont= bufDC.SelectObject(&font);
				bufDC.SetBkMode(TRANSPARENT);
				bufDC.SetTextColor(0x505050);
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
//	m_pParent->SetFocus();	
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
//	m_pParent->SetFocus();	
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
//	m_pParent->SetFocus();	
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
			pos = 1;
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
//	m_pParent->SetFocus();	
}

void CWireDlg::SetMode()
{
	CClientDC dc(this);

	CDC bufDC;
	bufDC.CreateCompatibleDC(&dc);
	CBitmap* oldbmp;
	oldbmp = bufDC.SelectObject(clearBM);

	bufDC.FillSolidRect(0,0,rc.right-rc.left,rc.bottom-rc.top,0);

	linepenL.DeleteObject();
	linepenR.DeleteObject();
	linepenbL.DeleteObject();
	linepenbR.DeleteObject();

	char buf[64];
	switch (scope_mode)
	{
	case 0:
		// vu
		KillTimer(2304+this_index);

		{
			CFont* oldFont= bufDC.SelectObject(&font);
			bufDC.SetBkMode(TRANSPARENT);
			bufDC.SetTextColor(0x606060);

			RECT rect;

			rect.left = 32+24;
			rect.right = 256-32-24;
			rect.top = 32-8;
			rect.bottom = rect.top+1;
			bufDC.FillSolidRect(&rect,0x00606060);
			sprintf(buf,"+6 db");
			bufDC.TextOut(32-1, 32-8-6, buf);
			bufDC.TextOut(256-32-22, 32-8-6, buf);

			rect.top = 32+44;
			rect.bottom = rect.top+1;
			bufDC.FillSolidRect(&rect,0x00606060);

			sprintf(buf,"-6 db");
			bufDC.TextOut(32-1+4, 32+44-6, buf);
			bufDC.TextOut(256-32-22, 32+44-6, buf);

			rect.top = 32+44+16;
			rect.bottom = rect.top+1;
			bufDC.FillSolidRect(&rect,0x00606060);
			sprintf(buf,"-12 db");
			bufDC.TextOut(32-1-6+4, 32+44+16-6, buf);
			bufDC.TextOut(256-32-22, 32+44+16-6, buf);

			rect.top = 32+44+16+18;
			rect.bottom = rect.top+1;
			bufDC.FillSolidRect(&rect,0x00606060);
			sprintf(buf,"-24 db");
			bufDC.TextOut(32-1-6+4, 32+44+16+18-6, buf);
			bufDC.TextOut(256-32-22, 32+44+16+18-6, buf);

			rect.top = 32+23;
			rect.bottom = rect.top+1;
			bufDC.SetTextColor(0x00707070);
			bufDC.FillSolidRect(&rect,0x00707070);
			sprintf(buf,"0 db");
			bufDC.TextOut(32-1+6, 32+23-6, buf);
			bufDC.TextOut(256-32-22, 32+23-6, buf);

			bufDC.SelectObject(oldFont);
		}

		m_slider2.SetRange(10,100);
		m_slider2.SetPos(scope_peak_rate);
		sprintf(buf,"Scope Mode");
		peakL = peakR = peak2L = peak2R = 0.0f;
		_pSrcMachine->_pScopeBufferL = pSamplesL;
		_pSrcMachine->_pScopeBufferR = pSamplesR;
		SetTimer(2304+this_index,scope_peak_rate,0);
		break;
	case 1:
		// oscilloscope
		KillTimer(2304+this_index);

		{
			// now draw our scope

			RECT rect;

			rect.left = 0;
			rect.right = 256;
			rect.top = 32;
			rect.bottom = rect.top+2;

			bufDC.FillSolidRect(&rect,0x00202020);

			rect.top = 95;
			rect.bottom = rect.top+2;
			bufDC.FillSolidRect(&rect,0x00202020);

			rect.top = 64-4;
			rect.bottom = rect.top+8;
			bufDC.FillSolidRect(&rect,0x00202020);

			rect.top = 64-2;
			rect.bottom = rect.top+4;
			bufDC.FillSolidRect(&rect,0x00404040);
		}
		linepenL.CreatePen(PS_SOLID, 2, 0xc08080);
		linepenR.CreatePen(PS_SOLID, 2, 0x80c080);

		m_slider.SetRange(1, 148);
		m_slider.SetPos(scope_osc_freq);
		pos = 1;
		m_slider2.SetRange(10,100);
		m_slider2.SetPos(scope_osc_rate);
		sprintf(buf,"Oscilloscope");
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
		sprintf(buf,"Spectrum Analyzer");
		_pSrcMachine->_pScopeBufferL = pSamplesL;
		_pSrcMachine->_pScopeBufferR = pSamplesR;
		SetTimer(2304+this_index,scope_osc_rate,0);
		break;
	case 3:
		// phase
		KillTimer(2304+this_index);
		{
			CPen linepen(PS_SOLID, 8, 0x00303030);

			CPen *oldpen = bufDC.SelectObject(&linepen);

			// now draw our scope

			bufDC.MoveTo(32,32);
			bufDC.LineTo(128,128);
			bufDC.LineTo(128,0);
			bufDC.MoveTo(128,128);
			bufDC.LineTo(256-32,32);
			bufDC.Arc(0,0,256,256,256,128,0,128);
			bufDC.Arc(96,96,256-96,256-96,256-96,128,96,128);

			bufDC.Arc(48,48,256-48,256-48,256-48,128,48,128);

			linepen.DeleteObject();
			linepen.CreatePen(PS_SOLID, 4, 0x00404040);
			bufDC.SelectObject(&linepen);
			bufDC.MoveTo(32,32);
			bufDC.LineTo(128,128);
			bufDC.LineTo(128,0);
			bufDC.MoveTo(128,128);
			bufDC.LineTo(256-32,32);
			linepen.DeleteObject();

			bufDC.SelectObject(oldpen);
		}
		linepenbL.CreatePen(PS_SOLID, 5, 0x806060);
		linepenbR.CreatePen(PS_SOLID, 5, 0x608060);
		linepenL.CreatePen(PS_SOLID, 3, 0xc08080);
		linepenR.CreatePen(PS_SOLID, 3, 0x80c080);

		_pSrcMachine->_pScopeBufferL = pSamplesL;
		_pSrcMachine->_pScopeBufferR = pSamplesR;
		sprintf(buf,"Stereo Phase");
		o_mvc = o_mvpc = o_mvl = o_mvdl = o_mvpl = o_mvdpl = o_mvr = o_mvdr = o_mvpr = o_mvdpr = 0.0f;
		m_slider2.SetRange(10,100);
		m_slider2.SetPos(scope_phase_rate);
		SetTimer(2304+this_index,scope_phase_rate,0);
		break;
	default:
		sprintf(buf,"Scope Mode");
		break;
	}
	m_mode.SetWindowText(buf);
	hold = false;
	pos = 1;

	bufDC.SelectObject(oldbmp);
	bufDC.DeleteDC();

}

