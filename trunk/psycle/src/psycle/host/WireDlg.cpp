///\file
///\brief implementation file for psycle::host::CWireDlg.

#include "WireDlg.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/machine.h>
#include <psycle/core/song.h>
using namespace psy::core;
#else
#include "Machine.hpp"
#include "Song.hpp"
#endif

#include "Configuration.hpp"
#include "ChildView.hpp"
#include "InputHandler.hpp"
#include "VolumeDlg.hpp"
#include "Zap.hpp"
#include "WireGui.hpp"
#include <psycle/helpers/helpers.hpp>
#include <psycle/helpers/math.hpp>
#include <psycle/helpers/fft.hpp>
#include <psycle/helpers/dsp.hpp>

PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		CWireDlg::CWireDlg(CChildView* pParent)
			: CDialog(CWireDlg::IDD, pParent),
			  m_pParent(pParent),
			  wire_gui_(0)
		{
		}

		CWireDlg::CWireDlg(CChildView* pParent, WireGui* wire_gui)
			: CDialog(CWireDlg::IDD, pParent),
			  m_pParent(pParent),
			  wire_gui_(wire_gui)
		{
		}

		void CWireDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_VOLUME_DB, m_volabel_db);
			DDX_Control(pDX, IDC_VOLUME_PER, m_volabel_per);
			DDX_Control(pDX, IDC_SLIDER1, m_volslider);
			DDX_Control(pDX, IDC_SLIDER, m_slider);
			DDX_Control(pDX, IDC_SLIDER2, m_slider2);
			DDX_Control(pDX, IDC_BUTTON, m_mode);
		}

		BEGIN_MESSAGE_MAP(CWireDlg, CDialog)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
			ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
			ON_WM_TIMER()
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER, OnCustomdrawSlider)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
			ON_BN_CLICKED(IDC_BUTTON, OnMode)
			ON_BN_CLICKED(IDC_BUTTON2, OnHold)
			ON_BN_CLICKED(IDC_VOLUME_DB, OnVolumeDb)
			ON_BN_CLICKED(IDC_VOLUME_PER, OnVolumePer)
		END_MESSAGE_MAP()

		BOOL CWireDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();
		#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
			pSamplesL = static_cast<float*>(_aligned_malloc(SCOPE_BUF_SIZE*sizeof(float),16));
			pSamplesR = static_cast<float*>(_aligned_malloc(SCOPE_BUF_SIZE*sizeof(float),16));
			inl = static_cast<float*>(_aligned_malloc(SCOPE_SPEC_SAMPLES*sizeof(float),16));
			inr = static_cast<float*>(_aligned_malloc(SCOPE_SPEC_SAMPLES*sizeof(float),16));
		#elif defined DIVERSALIS__PROCESSOR__X86 &&  defined DIVERSALIS__COMPILER__GNU
			posix_memalign(reinterpret_cast<void**>(pSamplesL),16,SCOPE_BUF_SIZE*sizeof(float));
			posix_memalign(reinterpret_cast<void**>(pSamplesR),16,SCOPE_BUF_SIZE*sizeof(float));
			posix_memalign(reinterpret_cast<void**>(inl),16,SCOPE_SPEC_SAMPLES*sizeof(float));
			posix_memalign(reinterpret_cast<void**>(inr),16,SCOPE_SPEC_SAMPLES*sizeof(float));
		#else
			pSamplesL = new float[SCOPE_BUF_SIZE];
			pSamplesR = new float[SCOPE_BUF_SIZE];
			inl = new float[SCOPE_SPEC_SAMPLES];
			inr = new float[SCOPE_SPEC_SAMPLES];
		#endif
			psycle::helpers::dsp::Clear(pSamplesL,SCOPE_BUF_SIZE);
			psycle::helpers::dsp::Clear(pSamplesR,SCOPE_BUF_SIZE);

			scope_mode = 0;
			scope_peak_rate = 20;
			scope_osc_freq = 5;
			scope_osc_rate = 20;
			scope_spec_bands = 128;
			scope_spec_rate = 20;
			scope_spec_mode = 1;
			scope_phase_rate = 20;
			InitSpectrum();

			Inval = false;
			m_volslider.SetRange(0,256*4);
			m_volslider.SetTicFreq(16*4);
			_dstWireIndex = _pDstMachine->FindInputWire(isrcMac);

			float val;
			_pDstMachine->GetWireVolume(_dstWireIndex,val);
			invol = val;
			int t = (int)sqrtf(val*16384*4*4);
			m_volslider.SetPos(256*4-t);

			char buf[128];
			sprintf(buf,"[%d] %s -> %s Connection Volume", wireIndex, _pSrcMachine->GetEditName().c_str(), _pDstMachine->GetEditName().c_str());
			SetWindowText(buf);

			hold = FALSE;

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

#if PSYCLE__CONFIGURATION__USE_PSYCORE
			mult = 3268.0f / _pSrcMachine->GetAudioRange();
#else
			if ( _pSrcMachine->_type == MACH_VST || _pSrcMachine->_type == MACH_VSTFX ) // native to VST, divide.
			{
				mult = 32768.0f;
			}
			else // native to native, no need to convert.
			{
				mult = 1.0f;
			}	
#endif
			return TRUE;
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
			if (wire_gui_)
				wire_gui_->BeforeWireDlgDeletion();
			DestroyWindow();
			font.DeleteObject();
			bufBM->DeleteObject();
			clearBM->DeleteObject();
			linepenL.DeleteObject();
			linepenR.DeleteObject();
			linepenbL.DeleteObject();
			linepenbR.DeleteObject();
			zapObject(bufBM);
			zapObject(clearBM);
		#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
			_aligned_free(pSamplesL);
			_aligned_free(pSamplesR);
			_aligned_free(inl);
			_aligned_free(inr);
		#elif defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__GNU
			free(pSamplesL);
			free(pSamplesR);
			free(inl);
			free(inr);
		#else
			delete [] pSamplesL;
			delete [] pSamplesR;
			delete [] inl;
			delete [] inr;
		#endif
			delete this;
		}

		void CWireDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			char bufper[32];
			char bufdb[32];
		//	invol = (128-m_volslider.GetPos())*0.0078125f;
			invol = ((256*4-m_volslider.GetPos())*(256*4-m_volslider.GetPos()))/(16384.0f*4*4);

			if (invol > 1.0f)
			{	
				sprintf(bufper,"%.2f%%",invol*100); 
				sprintf(bufdb,"+%.1f dB",20.0f * log10(invol)); 
			}
			else if (invol == 1.0f)
			{	
				sprintf(bufper,"100.00%%"); 
				sprintf(bufdb,"0.0 dB"); 
			}
			else if (invol > 0.0f)
			{	
				sprintf(bufper,"%.2f%%",invol*100); 
				sprintf(bufdb,"%.1f dB",20.0f * log10(invol)); 
			}
			else 
			{				
				sprintf(bufper,"0.00%%"); 
				sprintf(bufdb,"-Inf. dB"); 
			}

			m_volabel_per.SetWindowText(bufper);
			m_volabel_db.SetWindowText(bufdb);

			float f;
			_pDstMachine->GetWireVolume(_dstWireIndex, f);
			if (f != invol)
			{
				m_pParent->AddMacViewUndo();
				_pDstMachine->SetWireVolume(_dstWireIndex, invol );
			}

		//	m_pParent->SetFocus();	
			*pResult = 0;
		}

		void CWireDlg::OnButton1() 
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			//Nothing to do. done in wire_gui->RemoveWire()
#else
			m_pParent->AddMacViewUndo();
			Inval = true;
			CSingleLock lock(&Global::song().door,TRUE);
			_pSrcMachine->DeleteOutputWireIndex(&Global::song(),wireIndex);
			_pDstMachine->DeleteInputWireIndex(&Global::song(),_dstWireIndex);
#endif
			wire_gui_->RemoveWire();
			wire_gui_ = 0;
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

		void CWireDlg::InitSpectrum()
		{
			const float barsize = float(SCOPE_SPEC_SAMPLES>>1)/scope_spec_bands;
			//todo :A^X = 2^A
			const float samplesFactor = SCOPE_SPEC_SAMPLES/17.65f;
			const float samplesFactorB = 1.0f/(SCOPE_SPEC_SAMPLES>>1);
			for (int i=SCOPE_SPEC_SAMPLES-1;i>=0;i--)
			{ 
				//Linear -pi to pi.
				const float constant = 2.0f * helpers::math::pi_f * (-0.5f + ((float)i/(SCOPE_SPEC_SAMPLES-1)));
				//Hanning window 
				const float window = 0.50 - 0.50 * cosf(2.0f * M_PI * i / (SCOPE_SPEC_SAMPLES - 1));
				float j=0.0f;
				for(int h=0;h<scope_spec_bands;h++)
				{ 
					float th;
					if (scope_spec_mode == 0 ) {
						//this is linear
						th=j* constant; 
					}
					else if (scope_spec_mode == 1 ) {
						//this makes it somewhat logaritmic.
						th=(j*j*samplesFactorB)*constant; 
					}
					else {
						//This simulates a constant note scale.
						th = powf(2.0f,j/samplesFactor)*constant;
					}
					cth[i][h] = cosf(th) * window;
					sth[i][h] = sinf(th) * window;
					j+=barsize;
				}
			}
			for (int i=SCOPE_SPEC_SAMPLES-1;i>=0;i--)	{
				helpers::math::erase_all_nans_infinities_and_denormals(cth[i], scope_spec_bands);
			}
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
				CFont *oldFont=0;

				switch (scope_mode)
				{
				case 0: // off
					{
						// now draw our scope

						int index = _pSrcMachine->_scopeBufferIndex;
						float curpeakl,curpeakr;
						curpeakl = curpeakr = 0;
						float vucorrection= invol*mult*5329.0f/32768.0f;
						for (int i=0;i<SCOPE_SPEC_SAMPLES;i++) 
						{ 
							index--;
							index&=(SCOPE_BUF_SIZE-1);
							float awl=fabsf(pSamplesL[index]*_pSrcMachine->lVol());///32768; 
							float awr=fabsf(pSamplesR[index]*_pSrcMachine->rVol());///32768; 

							if (awl>curpeakl)	{	curpeakl = awl;	}
							if (awr>curpeakr)	{	curpeakr = awr;	}
						}

						curpeakl=128-helpers::math::rounded(sqrtf(curpeakl*vucorrection)); //conversion to a cardinal value.
						curpeakr=128-helpers::math::rounded(sqrtf(curpeakr*vucorrection));

						if (curpeakl<peak2L) //  it is a cardinal value, so smaller means higher peak.
							{
							if (curpeakl<0) curpeakl=0;
							peak2L = peakL = curpeakl;		peakLifeL = 100; //100 paints. 2 seconds at 50Hz
							}
						else if (curpeakl<peakL)	{	peakL = curpeakl;	}

						if (curpeakr<peak2R)//  it is a cardinal value, so smaller means higher peak.
							{
							if (curpeakr<0) curpeakr=0;
							peak2R = peakR = curpeakr;		peakLifeR = 100;
							}
						else if (curpeakr<peakR)	{	peakR = curpeakr;	}

						RECT rect;

						// LEFT CHANNEL
						rect.left = 128-32-24;
						rect.right = rect.left+48;

						if (peak2L<128)
						{
							CPen *oldpen = bufDC.SelectObject(&linepenL);
							bufDC.MoveTo(rect.left-1,peak2L);
							bufDC.LineTo(rect.right-1,peak2L);
							bufDC.SelectObject(oldpen);
						}

						rect.top = peakL;
						rect.bottom = curpeakl;
						COLORREF lbColor = 0x00503000 + ((peakL<32+23)?0x60:0x30);
						bufDC.FillSolidRect(&rect,lbColor);

						rect.top = rect.bottom;
						rect.bottom = 128;
						lbColor = 0xC08000+((curpeakl<32+23)?0xF0:0x80);
						bufDC.FillSolidRect(&rect,lbColor);

						// RIGHT CHANNEL 
						rect.left = 128+32-24;
						rect.right = rect.left+48;

						if (peak2R<128)
						{
							CPen *oldpen = bufDC.SelectObject(&linepenR);
							bufDC.MoveTo(rect.left-1,peak2R);
							bufDC.LineTo(rect.right-1,peak2R);
							bufDC.SelectObject(oldpen);
						}

						rect.top = peakR;
						rect.bottom = curpeakr;

						lbColor = 0x00406000 + ((peakR<32+23)?0x60:0x10);
						bufDC.FillSolidRect(&rect,lbColor);


						rect.top = rect.bottom;
						rect.bottom = 128;
						lbColor = 0x90D000+((curpeakr<32+23)?0xD0:0x20);
						bufDC.FillSolidRect(&rect,lbColor);

						// UPDATE PEAK.
						if (!hold)
						{
							if ( peakL < 128) peakL++;
							if ( peakR < 128) peakR++;
							if  (peakLifeL>0 ||peakLifeR>0)
							{
								peakLifeL--;
								peakLifeR--;
								if (peakLifeL <= 0)	{	peak2L = 128;	}
								if (peakLifeR <= 0)	{	peak2R = 128;	}
							}
							}
						}
						char buf[64];
						sprintf(buf,"Refresh %.2fhz",1000.0f/scope_peak_rate);
					oldFont= bufDC.SelectObject(&font);
						bufDC.SetBkMode(TRANSPARENT);
						bufDC.SetTextColor(0x505050);
						bufDC.TextOut(4, 128-14, buf);
						bufDC.SelectObject(oldFont);
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
						bufDC.MoveTo(256,GetY(pSamplesL[((int)n)&(SCOPE_BUF_SIZE-1)]*invol*mult*_pSrcMachine->lVol()));
						for (int x = 256-2; x >= 0; x-=2)
						{
							n -= add;
							bufDC.LineTo(x,GetY(pSamplesL[((int)n)&(SCOPE_BUF_SIZE-1)]*invol*mult*_pSrcMachine->lVol()));
		//					bufDC.LineTo(x,GetY(32768/2));
						}
						bufDC.SelectObject(&linepenR);

						n = float(_pSrcMachine->_scopeBufferIndex-pos);
						bufDC.MoveTo(256,GetY(pSamplesR[((int)n)&(SCOPE_BUF_SIZE-1)]*invol*mult*_pSrcMachine->rVol()));
						for (int x = 256-2; x >= 0; x-=2)
						{
							n -= add;
							bufDC.LineTo(x,GetY(pSamplesR[((int)n)&(SCOPE_BUF_SIZE-1)]*invol*mult*_pSrcMachine->rVol()));
						}

						bufDC.SelectObject(oldpen);

						char buf[64];
						sprintf(buf,"Frequency %dhz Refresh %.2fhz",freq,1000.0f/scope_osc_rate);
						oldFont= bufDC.SelectObject(&font);
						bufDC.SetBkMode(TRANSPARENT);
						bufDC.SetTextColor(0x505050);
						bufDC.TextOut(4, 128-14, buf);
						bufDC.SelectObject(oldFont);
					}
					break;

				case 2: // spectrum analyzer
					{
/*						int width = 1;
						float aal[SCOPE_SPEC_SAMPLES>>1];
						float aar[SCOPE_SPEC_SAMPLES>>1];
						memset (aal,0,sizeof(aal));
						memset (aar,0,sizeof(aar));
						int amount = SCOPE_BUF_SIZE - _pSrcMachine->_scopeBufferIndex;
						float vucorrection= invol*mult/6553500.0f;
						if (amount >= SCOPE_SPEC_SAMPLES) {
							psycle::helpers::dsp::MovMul(pSamplesL+_pSrcMachine->_scopeBufferIndex, inl, SCOPE_SPEC_SAMPLES, vucorrection*_pSrcMachine->_lVol);
							psycle::helpers::dsp::MovMul(pSamplesR+_pSrcMachine->_scopeBufferIndex, inr, SCOPE_SPEC_SAMPLES, vucorrection*_pSrcMachine->_rVol);
						} else {
							psycle::helpers::dsp::MovMul(pSamplesL+_pSrcMachine->_scopeBufferIndex, inl, amount, vucorrection*_pSrcMachine->_lVol);
							psycle::helpers::dsp::MovMul(pSamplesL, inl+amount, SCOPE_SPEC_SAMPLES-amount, vucorrection*_pSrcMachine->_rVol);
							psycle::helpers::dsp::MovMul(pSamplesR+_pSrcMachine->_scopeBufferIndex, inr, amount, vucorrection*_pSrcMachine->_lVol);
							psycle::helpers::dsp::MovMul(pSamplesR, inr+amount, SCOPE_SPEC_SAMPLES-amount, vucorrection*_pSrcMachine->_rVol);
						}

						psycle::host::dsp::WindowFunc(3,SCOPE_SPEC_SAMPLES,inl);
						psycle::host::dsp::WindowFunc(3,SCOPE_SPEC_SAMPLES,inr);
						psycle::host::dsp::PowerSpectrum(SCOPE_SPEC_SAMPLES,inl,aal);
						psycle::host::dsp::PowerSpectrum(SCOPE_SPEC_SAMPLES,inr,aar);
						psycle::helpers::dsp::Undenormalize(aal,aar,SCOPE_SPEC_SAMPLES>>1);
						
						COLORREF cl = 0x402020;
						COLORREF cr = 0x204020;

						RECT rect;
						rect.left = 0;

						// draw our bands

						for (int i = 0; i < MAX_SCOPE_BANDS; i++)
						{
							//TODO: change this code to be logarithmic
							//((pow(10.0f,i)-1.0f)/9.0f)
							//(int)((((pow(10.0f,i/(float)MAX_SCOPE_BANDS))-1.0f)/9.0f)*(SCOPE_SPEC_SAMPLES>>1))
							int aml = - psycle::helpers::dsp::dB(aal[(int)((((pow(10.0f,i/(float)MAX_SCOPE_BANDS))-1.0f)/9.0f)*(SCOPE_SPEC_SAMPLES>>1))]+0.0000001f);
							if (aml < 0)
							{
								aml = 0;
							}
							if (aml > 128) {
								aml = 128;
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
							bufDC.FillSolidRect(&rect,cl+0x804040);

							rect.left+=width;
							//TODO: change this code to be logarithmic
							int amr = - psycle::helpers::dsp::dB(aar[(int)((((pow(10.0f,i/(float)MAX_SCOPE_BANDS))-1.0f)/9.0f)*(SCOPE_SPEC_SAMPLES>>1))]+0.0000001f);
							if (amr < 0)
							{
								amr = 0;
							}
							if (amr > 128) {
								amr = 128;
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
							bufDC.FillSolidRect(&rect,cr+0x408040);

							rect.left+=width;

							if (!hold)
							{
								bar_heightsl[i]+=6;
								if (bar_heightsl[i] > 128)
								{
									bar_heightsl[i] = 128;
								}
								bar_heightsr[i]+=6;
								if (bar_heightsr[i] > 128)
								{
									bar_heightsr[i] = 128;
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
*/


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

						int width = 128/scope_spec_bands;
						const float multleft= invol*mult/32768.0f *_pSrcMachine->lVol();
						const float multright= invol*mult/32768.0f *_pSrcMachine->rVol();
						const float invSamples = 1.0f/(SCOPE_SPEC_SAMPLES>>1);
						// calculate our bands using same buffer chasing technique
						int index = _pSrcMachine->_scopeBufferIndex;
						for (int i=0;i<SCOPE_SPEC_SAMPLES;i++) 
						{ 
							index--;
							index&=(SCOPE_BUF_SIZE-1);
							const float wl=pSamplesL[index]*multleft;
							const float wr=pSamplesR[index]*multright;
							for(int h=0;h<scope_spec_bands;h++) 
							{ 
								aal[h]+=wl*cth[i][h];
								bbl[h]+=wl*sth[i][h];
								aar[h]+=wr*cth[i][h];
								bbr[h]+=wr*sth[i][h];
							} 
						} 

						for (int h=0;h<scope_spec_bands;h++) 
						{
							ampl[h]= sqrtf(aal[h]*aal[h]+bbl[h]*bbl[h])*invSamples; 
							ampr[h]= sqrtf(aar[h]*aar[h]+bbr[h]*bbr[h])*invSamples;
						}

						COLORREF cl = 0x402020;
						COLORREF cr = 0x204020;

						RECT rect;
						rect.left = 0;

						// draw our bands

						for (int i = 0; i < scope_spec_bands; i++)
						{
							//int aml = 128 - (log(1+ampl[i])*heightcompensation[i]);
							int aml = - (psycle::helpers::dsp::dB(ampl[i]+0.0000001f)+6) * 2; // 128dB of range is TOO much. reducing it to 64dB.
							//int aml = - psycle::helpers::dsp::dB(ampl[(int)((((pow(10.0f,i/(float)scope_spec_bands))-1.0f)/9.0f)*(SCOPE_SPEC_SAMPLES>>1))]+0.0000001f);

//							int aml = 128-helpers::math::rounded(sqrtf(ampl[i]));
							if (aml < 0)
							{
								aml = 0;
							}
							if (aml > 128) {
								aml = 128;
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
							bufDC.FillSolidRect(&rect,cl+0x804040);
							
							rect.left+=width;

							//int amr = 128 - (log(1+ampr[i])*heightcompensation[i]);
							int amr = - (psycle::helpers::dsp::dB(ampr[i]+0.0000001f)+6) * 2; // 128dB of range is TOO much. reducing it to 64dB.
							//int amr = - psycle::helpers::dsp::dB(ampr[(int)((((pow(10.0f,i/(float)scope_spec_bands))-1.0f)/9.0f)*(SCOPE_SPEC_SAMPLES>>1))]+0.0000001f);
//							int amr = 128-helpers::math::rounded(sqrtf(ampr[i]));
							if (amr < 0)
							{
								amr = 0;
							}
							if (amr > 128) {
								amr = 128;
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
							bufDC.FillSolidRect(&rect,cr+0x408040);

							rect.left+=width;

							if (!hold)
							{
								bar_heightsl[i]+=6;
								if (bar_heightsl[i] > 128)
								{
									bar_heightsl[i] = 128;
								}
								bar_heightsr[i]+=6;
								if (bar_heightsr[i] > 128)
								{
									bar_heightsr[i] = 128;
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
							float wl=(pSamplesL[index]*invol*mult*_pSrcMachine->lVol());///32768;
							float wr=(pSamplesR[index]*invol*mult*_pSrcMachine->rVol());///32768;
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

						x=helpers::math::rounded(sinf(-(helpers::math::pi_f/4.0f)-(o_mvdpl*helpers::math::pi_f/(32768.0f*4.0f)))
									*o_mvpl*(128.0f/32768.0f))+128;
						y=helpers::math::rounded(-cosf(-(helpers::math::pi_f/4.0f)-(o_mvdpl*helpers::math::pi_f/(32768.0f*4.0f)))
									*o_mvpl*(128.0f/32768.0f))+128;
						bufDC.MoveTo(x,y);
						bufDC.LineTo(128,128);
						bufDC.LineTo(128,128-helpers::math::rounded(o_mvpc*(128.0f/32768.0f)));
						bufDC.MoveTo(128,128);
						x=helpers::math::rounded(sinf((helpers::math::pi_f/4.0f)+(o_mvdpr*helpers::math::pi_f/(32768.0f*4.0f)))
									*o_mvpr*(128.0f/32768.0f))+128;
						y=helpers::math::rounded(-cosf((helpers::math::pi_f/4.0f)+(o_mvdpr*helpers::math::pi_f/(32768.0f*4.0f)))
									*o_mvpr*(128.0f/32768.0f))+128;
						bufDC.LineTo(x,y);
										
						// panning data
						bufDC.SelectObject(&linepenbR);

						x=helpers::math::rounded(sinf(-(o_mvdl*helpers::math::pi_f/(32768.0f*4.0f)))
									*o_mvl*(128.0f/32768.0f))+128;
						y=helpers::math::rounded(-cosf(-(o_mvdl*helpers::math::pi_f/(32768.0f*4.0f)))
									*o_mvl*(128.0f/32768.0f))+128;
						bufDC.MoveTo(x,y);
						bufDC.LineTo(128,128);
						bufDC.LineTo(128,128-helpers::math::rounded(o_mvc*(128.0f/32768.0f)));
						bufDC.MoveTo(128,128);
						x=helpers::math::rounded(sinf((o_mvdr*helpers::math::pi_f/(32768.0f*4.0f)))
									*o_mvr*(128.0f/32768.0f))+128;
						y=helpers::math::rounded(-cosf((o_mvdr*helpers::math::pi_f/(32768.0f*4.0f)))
									*o_mvr*(128.0f/32768.0f))+128;
						bufDC.LineTo(x,y);

						bufDC.SelectObject(&linepenL);

						x=helpers::math::rounded(sinf(-(helpers::math::pi_f/4.0f)-(mvdpl*helpers::math::pi_f/(32768.0f*4.0f)))
									*mvpl*(128.0f/32768.0f))+128;
						y=helpers::math::rounded(-cosf(-(helpers::math::pi_f/4.0f)-(mvdpl*helpers::math::pi_f/(32768.0f*4.0f)))
									*mvpl*(128.0f/32768.0f))+128;
						bufDC.MoveTo(x,y);
						bufDC.LineTo(128,128);
						bufDC.LineTo(128,128-helpers::math::rounded(mvpc*(128.0f/32768.0f)));
						bufDC.MoveTo(128,128);
						x=helpers::math::rounded(sinf((helpers::math::pi_f/4.0f)+(mvdpr*helpers::math::pi_f/(32768.0f*4.0f)))
									*mvpr*(128.0f/32768.0f))+128;
						y=helpers::math::rounded(-cosf((helpers::math::pi_f/4.0f)+(mvdpr*helpers::math::pi_f/(32768.0f*4.0f)))
									*mvpr*(128.0f/32768.0f))+128;
						bufDC.LineTo(x,y);
										
						// panning data
						bufDC.SelectObject(&linepenR);

						x=helpers::math::rounded(sinf(-(mvdl*helpers::math::pi_f/(32768.0f*4.0f)))
									*mvl*(128.0f/32768.0f))+128;
						y=helpers::math::rounded(-cosf(-(mvdl*helpers::math::pi_f/(32768.0f*4.0f)))
									*mvl*(128.0f/32768.0f))+128;
						bufDC.MoveTo(x,y);
						bufDC.LineTo(128,128);
						bufDC.LineTo(128,128-helpers::math::rounded(mvc*(128.0f/32768.0f)));
						bufDC.MoveTo(128,128);
						x=helpers::math::rounded(sinf((mvdr*helpers::math::pi_f/(32768.0f*4.0f)))
									*mvr*(128.0f/32768.0f))+128;
						y=helpers::math::rounded(-cosf((mvdr*helpers::math::pi_f/(32768.0f*4.0f)))
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
						oldFont= bufDC.SelectObject(&font);
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
				scope_spec_mode = m_slider.GetPos();
				SetMode();
				InitSpectrum();
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
				peakL = peakR = peak2L = peak2R = 128.0f;
				_pSrcMachine->_pScopeBufferL = pSamplesL;
				_pSrcMachine->_pScopeBufferR = pSamplesR;
				linepenL.CreatePen(PS_SOLID, 2, 0xc08080);
				linepenR.CreatePen(PS_SOLID, 2, 0x80c080);
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

				m_slider.SetRange(5, 100);
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
					CFont* oldFont= bufDC.SelectObject(&font);
					bufDC.SetBkMode(TRANSPARENT);
					bufDC.SetTextColor(0x505050);

					RECT rect;
					rect.left = 0;
					rect.right = 256;
					sprintf(buf,"db");
					bufDC.TextOut(3, 0, buf);
					bufDC.TextOut(256-13, 0, buf);
					for(int i=1;i<6;i++) {
						rect.top = 20*i;
						rect.bottom = rect.top +1;
						bufDC.FillSolidRect(&rect,0x00505050);

						sprintf(buf,"-%d0",i);
						bufDC.TextOut(0, rect.top-10, buf);
						bufDC.TextOut(256-16, rect.top-10, buf);
					}
					rect.left = 128;
					rect.right = 256;
					rect.top = 120;
					rect.bottom = rect.top +1;
					bufDC.FillSolidRect(&rect,0x00505050);

					sprintf(buf,"-60");
					bufDC.TextOut(256-16, rect.top-10, buf);

					rect.top=0;
					rect.bottom=256;
					if (scope_spec_mode == 0) rect.left=6;
					else if (scope_spec_mode == 1) rect.left=38;
					else if (scope_spec_mode == 2) rect.left=99;
					rect.right=rect.left+1;
					bufDC.FillSolidRect(&rect,0x00606060);
					sprintf(buf,"440");
					bufDC.TextOut(rect.left, 0, buf);
					bufDC.TextOut(rect.left, 128-12, buf);

					if (scope_spec_mode == 0) rect.left=82;
					else if (scope_spec_mode == 1) rect.left=146;
					else if (scope_spec_mode == 2) rect.left=256-42;
					rect.right=rect.left+1;
					bufDC.FillSolidRect(&rect,0x00606060);
					sprintf(buf,"7K");
					bufDC.TextOut(rect.left, 0, buf);
					bufDC.TextOut(rect.left, 128-12, buf);
					if (scope_spec_mode == 0) rect.left=256-70;
					else if (scope_spec_mode == 1) rect.left=256-37;
					else if (scope_spec_mode == 2) rect.left=256-7;
					rect.right=rect.left+1;
					bufDC.FillSolidRect(&rect,0x00606060);
					sprintf(buf,"16K");
					bufDC.TextOut(rect.left, 0, buf);
					bufDC.TextOut(rect.left, 128-12, buf);


					bufDC.SelectObject(oldFont);
				}
				m_slider.SetRange(0, 2);
				m_slider.SetPos(scope_spec_mode);
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


		BOOL CWireDlg::PreTranslateMessage(MSG* pMsg) 
		{
			if (pMsg->message == WM_KEYDOWN)
			{
				if (pMsg->wParam == VK_UP)
				{
					int v = m_volslider.GetPos();
					v--;
					if (v < 0)
					{
						v = 0;
					}
					m_volslider.SetPos(v);
					return true;
				}
				else if (pMsg->wParam == VK_DOWN)
				{
					int v = m_volslider.GetPos();
					v++;
					if (v > 256*4)
					{
						v = 256*4;
					}
					m_volslider.SetPos(v);
					return true;
				}
				else
				{
					m_pParent->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
					return true;
				}
			}
			else if (pMsg->message == WM_KEYUP)
			{
				if (pMsg->wParam == VK_UP ||pMsg->wParam == VK_DOWN)
				{
					return true;
				}
				m_pParent->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
				return true;
			}
			return CDialog::PreTranslateMessage(pMsg);
		}


		void CWireDlg::OnVolumeDb() 
		{
			CVolumeDlg dlg;
			dlg.volume = invol;
			dlg.edit_type = 0;
			if (dlg.DoModal() == IDOK)
			{
				m_pParent->AddMacViewUndo();

				// update from dialog
				int t = (int)sqrtf(dlg.volume*16384*4*4);
				m_volslider.SetPos(256*4-t);
			}
		}

		void CWireDlg::OnVolumePer() 
		{
			CVolumeDlg dlg;
			dlg.volume = invol;
			dlg.edit_type = 1;
			if (dlg.DoModal() == IDOK)
			{
				m_pParent->AddMacViewUndo();
				// update from dialog
				int t = (int)sqrtf(dlg.volume*16384*4*4);
				m_volslider.SetPos(256*4-t);
			}
		}
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END


