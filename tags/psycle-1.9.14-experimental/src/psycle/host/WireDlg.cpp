///\file
///\brief implementation file for psycle::host::CWireDlg.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/psycle.hpp>
#include <psycle/host/WireDlg.hpp>
#include <psycle/host/VolumeDlg.hpp>
#include <psycle/engine/machine.hpp>
#include <psycle/engine/player.hpp>
#include <psycle/helpers/helpers.hpp>
#include <psycle/helpers/dsp.hpp>
//#include <psycle/helpers/fft.hpp>
#include <psycle/host/ChildView.hpp>
//#include <psycle/host/InputHandler.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		// CWaveScopeCtrl
		CMultiScopeCtrl::CMultiScopeCtrl()
		: bufBM(0)
		, clearBM(0)
		, hold(false)
		, clip(false)
		, scope_offset(0)
		, conn_sel_in(-1)
		, conn_sel_out(-1)
		, pSamplesL(0)
		, pSamplesR(0)
		, _pSrcMachine(0)
		, _pDstMachine(0)
		{
		}

		CMultiScopeCtrl::~CMultiScopeCtrl()
		{
			font.DeleteObject();
			if (bufBM) bufBM->DeleteObject();
			if (clearBM) clearBM->DeleteObject();
			linepenL.DeleteObject();
			linepenR.DeleteObject();
			linepenbL.DeleteObject();
			linepenbR.DeleteObject();
			zapObject(bufBM);
			zapObject(clearBM);
		}

		void CMultiScopeCtrl::Initialize()
		{
			font.CreatePointFont(70,"Tahoma");

			rcscope.top = 30;
			rcscope.left = 6;
			rcscope.bottom = 130+rcscope.top;
			rcscope.right = 252+rcscope.left;

			rccons.left = 4;
			rccons.right = rccons.left + 256;
			rccons.top = 32;
			rccons.bottom = rccons.top + 200;

			scope_mode = CMultiScopeCtrl::mode_vu;
			scope_peak_rate = 20;
			scope_osc_freq = 10;
			scope_osc_rate = 20;
			scope_spec_bands = 16;
			scope_spec_rate = 25;
			scope_phase_rate = 20;

			rangecorrection=1.0f/_pSrcMachine->GetAudioRange();  // correction for a -1.0f 1.0f range.
			vucorrection = 5329.0f*rangecorrection;
			scopecorrection = 64.0f*rangecorrection;
			spectrumcorrection = 9.0f*rangecorrection;
			InitSpectrum();

			// get values for connections - NEED TO BE MODIFIED TO WORK WITH "GetNumINputs()" etc
			numIns = 8; 
			numOuts = 8; 

			//set up inputs - later, this will have to be modified to make it match up stereo pairs (if available) by default.
			for (int i = 0; i < numIns; i++)
			{
				ins_node_used.push_back(0);
			}
			for (int i = 0; i < numOuts; i++)
			{
				outs_node_used.push_back(0);
			}

			conn_ins_spacing = 200 / numIns;
			conn_outs_spacing = 200 / numOuts;

		}
		void CMultiScopeCtrl::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
		{
			if (lpDrawItemStruct->itemAction == ODA_DRAWENTIRE)
			{
				CDC dc;
				dc.Attach(lpDrawItemStruct->hDC);

				CRect rc;
				rc = (scope_mode == CMultiScopeCtrl::mode_viewcons)?rccons:rcscope;

				if ( !bufBM )
				{
					bufBM = new CBitmap;
					bufBM->CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);
					clearBM = new CBitmap;
					clearBM->CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);
					PrepareView();
				}

				CDC bufDC;
				bufDC.CreateCompatibleDC(&dc);
				CBitmap* oldbmp;
				oldbmp = bufDC.SelectObject(bufBM);

				CDC clrDC;
				clrDC.CreateCompatibleDC(&dc);
				CBitmap* oldbmp2;
				oldbmp2 = clrDC.SelectObject(clearBM);

				bufDC.BitBlt(0,0,rc.Width(),rc.Height(),&clrDC,0,0,SRCCOPY);

				clrDC.SelectObject(oldbmp2);
				clrDC.DeleteDC();

				switch (scope_mode)
				{
				case CMultiScopeCtrl::mode_vu: 
					PaintVu(&bufDC);
					break;
				case CMultiScopeCtrl::mode_scope:
					PaintOsci(&bufDC);
					break;
				case CMultiScopeCtrl::mode_spectrum:
					PaintSpectrum(&bufDC);
					break;
				case CMultiScopeCtrl::mode_phase:
					PaintPhase(&bufDC);
					break;

				case CMultiScopeCtrl::mode_viewcons:
					PaintConnections(&bufDC);
					break;
				}
				// and debuffer
				dc.BitBlt(0,0,rc.Width(),rc.Height(),&bufDC,0,0,SRCCOPY);

				bufDC.SelectObject(oldbmp);
				bufDC.DeleteDC();
				dc.Detach();
			}
		}

		void CMultiScopeCtrl::PrepareView()
		{
			CClientDC dc(this);

			CDC bufDC;
			bufDC.CreateCompatibleDC(&dc);
			CBitmap* oldbmp;
			oldbmp = bufDC.SelectObject(clearBM);

			bufDC.FillSolidRect(0,0,rcscope.right-rcscope.left,rcscope.bottom-rcscope.top,0);

			linepenL.DeleteObject();
			linepenR.DeleteObject();
			linepenbL.DeleteObject();
			linepenbR.DeleteObject();

			std::string buf;
			switch (scope_mode)
			{
			case CMultiScopeCtrl::mode_vu:
				{
					CFont* oldFont= bufDC.SelectObject(&font);
					bufDC.SetBkMode(TRANSPARENT);
					bufDC.SetTextColor(0x606060);

					RECT rect;
					rect.left = 32+24;
					rect.right = 256-rect.left;

					rect.top = 32-8;
					rect.bottom = rect.top+1;
					bufDC.FillSolidRect(&rect,0x00606060);
					buf = "+6 db";
					bufDC.TextOut(32-1, 32-8-6, buf.c_str(),buf.size());
					bufDC.TextOut(256-32-22, 32-8-6, buf.c_str(),buf.size());

					rect.top = 32+44;
					rect.bottom = rect.top+1;
					bufDC.FillSolidRect(&rect,0x00606060);
					buf ="-6 db";
					bufDC.TextOut(32-1+4, 32+44-6, buf.c_str(),buf.size());
					bufDC.TextOut(256-32-22, 32+44-6, buf.c_str(),buf.size());

					rect.top = 32+44+16;
					rect.bottom = rect.top+1;
					bufDC.FillSolidRect(&rect,0x00606060);
					buf ="-12 db";
					bufDC.TextOut(32-1-6+4, 32+44+16-6, buf.c_str(),buf.size());
					bufDC.TextOut(256-32-22, 32+44+16-6, buf.c_str(),buf.size());

					rect.top = 32+44+16+18;
					rect.bottom = rect.top+1;
					bufDC.FillSolidRect(&rect,0x00606060);
					buf ="-24 db";
					bufDC.TextOut(32-1-6+4, 32+44+16+18-6, buf.c_str(),buf.size());
					bufDC.TextOut(256-32-22, 32+44+16+18-6, buf.c_str(),buf.size());

					rect.top = 32+23;
					rect.bottom = rect.top+1;
					bufDC.SetTextColor(0x00707070);
					bufDC.FillSolidRect(&rect,0x00707070);
					buf ="0 db";
					bufDC.TextOut(32-1+6, 32+23-6, buf.c_str(),buf.size());
					bufDC.TextOut(256-32-22, 32+23-6, buf.c_str(),buf.size());

					bufDC.SelectObject(oldFont);

					linepenL.CreatePen(PS_SOLID, 2, 0xc08080);
					linepenR.CreatePen(PS_SOLID, 2, 0x80c080);
				}
				break;
			case CMultiScopeCtrl::mode_scope:
				{
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
				break;
			case CMultiScopeCtrl::mode_spectrum:
				{
					for (int i = 0; i < MAX_SCOPE_BANDS; i++)
					{
						bar_heightsl[i]=128;
						bar_heightsr[i]=128;
					}
				}
				break;
			case CMultiScopeCtrl::mode_phase:
				{
					CPen linepen(PS_SOLID, 8, 0x00303030);
					CPen *oldpen = bufDC.SelectObject(&linepen);

					bufDC.MoveTo(32,32);
					bufDC.LineTo(128,128);
					bufDC.LineTo(128,0);
					bufDC.MoveTo(128,128);
					bufDC.LineTo(256-32,32);
					bufDC.Arc(0,0,256,256,256,128,0,128);
					bufDC.Arc(96,96,256-96,256-96,256-96,128,96,128);

					bufDC.Arc(48,48,256-48,256-48,256-48,128,48,128);

					bufDC.SelectObject(oldpen);
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

					linepenbL.CreatePen(PS_SOLID, 5, 0x806060);
					linepenbR.CreatePen(PS_SOLID, 5, 0x608060);
					linepenL.CreatePen(PS_SOLID, 3, 0xc08080);
					linepenR.CreatePen(PS_SOLID, 3, 0x80c080);
				}
				break;
			case CMultiScopeCtrl::mode_viewcons:
				{
					bufDC.FillSolidRect (&rccons, 0x00FFFFFF);
					CPen linepen(PS_SOLID, 8, 0x00303030);
					CPen *oldpen = bufDC.SelectObject(&linepen);
					CFont* oldFont= bufDC.SelectObject(&font);

					std::string buffer;
					RECT tempnode;
					COLORREF colour = 0x303030;

					buffer = "Outs";
					bufDC.MoveTo(32,32);
					bufDC.LineTo(224,32);
					bufDC.TextOut(32,16,buffer.c_str(),buffer.size());
					buf = "Ins";
					bufDC.TextOut(192,16,buf.c_str(),buf.size());

					for (int j = 0; j < numOuts; j++) 
					{
						tempnode = GetNodeRect (j, 1);
						dc.FillSolidRect (&tempnode, 0x00000000);
						//						buffer = pWire->GetSrcMachine().GetOutPort(j).Name() << ".";
						buffer = "unfinished";
						bufDC.SetBkMode(TRANSPARENT); dc.SetTextColor (colour);
						dc.TextOut (tempnode.left - 50, tempnode.top - 5, buffer.c_str(),buffer.size());
					}
					for (int j = 0; j < numIns; j++) 
					{
						tempnode = GetNodeRect (j, 0);
						bufDC.FillSolidRect (&tempnode,0x00000000);
						//						buffer << pWire->GetDstMachine().GetInPort(j).Name() << ".";
						buffer = "unfinished";
						bufDC.SetBkMode(TRANSPARENT); dc.SetTextColor (colour);
						dc.TextOut (tempnode.left + 5, tempnode.top - 5, buffer.c_str(),buffer.size());
					}
					bufDC.SelectObject(oldFont);
					bufDC.SelectObject(oldpen);
				}
				break;
			default:
				break;
			}

			bufDC.SelectObject(oldbmp);
			bufDC.DeleteDC();
		}
		void CMultiScopeCtrl::InitSpectrum()
		{
			int constant2 = MAX_SCOPE_BANDS/scope_spec_bands;
			for (int i=0;i<SCOPE_SPEC_SAMPLES;i++)
			{ 
				float constant = (F_PI/(SCOPE_SPEC_SAMPLES/2))*(i-(SCOPE_SPEC_SAMPLES/2)); 
				int j=0;
				for(int h=0;h<scope_spec_bands;h++)
				{ 
					float th=((float(j*j)/MAX_SCOPE_BANDS)+1.0f)*constant; 
					cth[i][h] = cosf(th);
					sth[i][h] = sinf(th);
					j+=constant2;
				}
			}
			int j=0;
			for(int h=0;h<scope_spec_bands;h++)
			{ 
				heightcompensation[h]=96.0f + 128.0f*(log(1.0+((float)j*2.0/MAX_SCOPE_BANDS))/log(2.0));
				j+=constant2;
			}
		}

		inline int CMultiScopeCtrl::GetY(float f,float _invol)
		{
			f=64-(f*scopecorrection*_invol);

			if (f < 1) 
			{
				clip = true;
				return 1;
			}
			else if (f > 126) 
			{
				clip = true;
				return 126;
			}
			return int(f);
		}
		void CMultiScopeCtrl::PaintVu(CDC* bufDC)
		{
			int index = _pSrcMachine->_scopeBufferIndex;
			float curpeakl,curpeakr;
			curpeakl = curpeakr = 0;

			for (int i=0;i<SCOPE_SPEC_SAMPLES;i++) 
			{ 
				index--;
				index&=(SCOPE_BUF_SIZE-1);
				const float awl=fabsf(pSamplesL[index]);
				const float awr=fabsf(pSamplesR[index]);

				if (awl>curpeakl)	{	curpeakl = awl;	}
				if (awr>curpeakr)	{	curpeakr = awr;	}
			}
			curpeakl=128-f2i(sqrtf(curpeakl*vucorrection*involL)); //conversion to a cardinal value.
			curpeakr=128-f2i(sqrtf(curpeakr*vucorrection*involR));

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
				CPen *oldpen = bufDC->SelectObject(&linepenL);
				bufDC->MoveTo(rect.left-1,peak2L);
				bufDC->LineTo(rect.right-1,peak2L);
				bufDC->SelectObject(oldpen);
			}

			rect.top = peakL;
			rect.bottom = curpeakl;
			COLORREF lbColor = 0x00503000 + ((peak2L<32+23)?0x60:0x30);
			bufDC->FillSolidRect(&rect,lbColor);

			rect.top = rect.bottom;
			rect.bottom = 128;
			lbColor = 0xC08000+((curpeakl<32+23)?0xF0:0x80);
			bufDC->FillSolidRect(&rect,lbColor);

			// RIGHT CHANNEL 
			rect.left = 128+32-24;
			rect.right = rect.left+48;

			if (peak2R<128)
			{
				CPen *oldpen = bufDC->SelectObject(&linepenR);
				bufDC->MoveTo(rect.left-1,peak2R);
				bufDC->LineTo(rect.right-1,peak2R);
				bufDC->SelectObject(oldpen);
			}

			rect.top = peakR;
			rect.bottom = curpeakr;

			lbColor = 0x00406000 + ((peakR<32+23)?0x60:0x10);
			bufDC->FillSolidRect(&rect,lbColor);


			rect.top = rect.bottom;
			rect.bottom = 128;
			lbColor = 0x90D000+((curpeakr<32+23)?0xD0:0x20);
			bufDC->FillSolidRect(&rect,lbColor);

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

			char buf[64];
			sprintf(buf,"Refresh %.2fhz",1000.0f/scope_peak_rate);
			CFont* oldFont= bufDC->SelectObject(&font);
			bufDC->SetBkMode(TRANSPARENT);
			bufDC->SetTextColor(0x505050);
			bufDC->TextOut(4, 128-14, buf);
			bufDC->SelectObject(oldFont);

		}
		void CMultiScopeCtrl::PaintOsci(CDC* bufDC)
		{
			const int freq = scope_osc_freq*scope_osc_freq;

			const COLORREF colorL = 0xc08080;
			const COLORREF colorR = 0x80c080;

			// red line if last frame was clipping
			if (clip)
			{
				RECT rect;

				rect.left = 0;
				rect.right = 256;
				rect.top = 32;
				rect.bottom = rect.top+2;

				bufDC->FillSolidRect(&rect,0x00202040);

				rect.top = 95;
				rect.bottom = rect.top+2;
				bufDC->FillSolidRect(&rect,0x00202040);

				rect.top = 64-4;
				rect.bottom = rect.top+8;
				bufDC->FillSolidRect(&rect,0x00202050);

				rect.top = 64-2;
				rect.bottom = rect.top+4;
				bufDC->FillSolidRect(&rect,0x00101080);

				clip = false;
			}

			CPen *oldpen = bufDC->SelectObject(&linepenL);

			// ok this is a little tricky - it chases the wrapping buffer, starting at the last sample 
			// buffered and working backwards - it does it this way to minimize chances of drawing 
			// erroneous data across the buffering point
			float add = (float(Global::player().SampleRate())/(float(freq)))/128.0f;

			float n = float(_pSrcMachine->_scopeBufferIndex-scope_offset);
			for (int x = 256; x > 0; x--)
			{
				bufDC->SetPixelV(x,GetY(pSamplesL[((int)n)&(SCOPE_BUF_SIZE-1)],involL),colorL);
				n -= add;
			}
			bufDC->SelectObject(&linepenR);

			n = float(_pSrcMachine->_scopeBufferIndex-scope_offset);
			for (int x = 256; x > 0; x--)
			{
				bufDC->SetPixelV(x,GetY(pSamplesL[((int)n)&(SCOPE_BUF_SIZE-1)],involR),colorR);
				n -= add;
			}

			bufDC->SelectObject(oldpen);

			char buf[64];
			sprintf(buf,"Frequency %dhz Refresh %.2fhz",freq,1000.0f/scope_osc_rate);
			CFont* oldFont= bufDC->SelectObject(&font);
			bufDC->SetBkMode(TRANSPARENT);
			bufDC->SetTextColor(0x505050);
			bufDC->TextOut(4, 128-14, buf);
			bufDC->SelectObject(oldFont);

		}
		void CMultiScopeCtrl::PaintSpectrum(CDC* bufDC)
		{
			int width = 128/scope_spec_bands;

			float aal[MAX_SCOPE_BANDS]; 
			float aar[MAX_SCOPE_BANDS]; 
			float bbl[MAX_SCOPE_BANDS]; 
			float bbr[MAX_SCOPE_BANDS]; 
			float ampl[MAX_SCOPE_BANDS];
			float ampr[MAX_SCOPE_BANDS];
			std::memset(aal,0,sizeof(aal));
			std::memset(bbl,0,sizeof(bbl));
			std::memset(aar,0,sizeof(aar));
			std::memset(bbr,0,sizeof(bbr));

			// calculate our bands using same buffer chasing technique

			int index = _pSrcMachine->_scopeBufferIndex;
			for (int i=0;i<SCOPE_SPEC_SAMPLES;i++) 
			{ 
				index--;
				index&=(SCOPE_BUF_SIZE-1);
				const float wl=(pSamplesL[index]*spectrumcorrection);
				const float wr=(pSamplesR[index]*spectrumcorrection);
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
				ampl[h]= sqrtf(aal[h]*aal[h]+bbl[h]*bbl[h])/(SCOPE_SPEC_SAMPLES/2); 
				ampr[h]= sqrtf(aar[h]*aar[h]+bbr[h]*bbr[h])/(SCOPE_SPEC_SAMPLES/2); 
			}
			/*
			// Alternate FFT, using the fft.h library.

			int minbars=32;
			while (minbars<scope_spec_bands)
			{
			minbars<<=1;
			}
			float bufl[MAX_SCOPE_BANDS*2];
			//						float bufr[MAX_SCOPE_BANDS*2];
			//						int index = _pSrcMachine->_scopeBufferIndex - (minbars*2);
			index = _pSrcMachine->_scopeBufferIndex - (minbars*2);
			for (int i=0;i<(minbars*2);i++) 
			{ 
			index--;
			index&=(SCOPE_BUF_SIZE-1);
			bufl[i]=(pSamplesL[index]*involL*mult*_pSrcMachine->_lVol)*0.0000305f;
			//							bufr[i]=(pSamplesR[index]*involR*mult*_pSrcMachine->_rVol)*0.0000305f; 
			} 
			*/
			/*
			int index = _pSrcMachine->_scopeBufferIndex - SCOPE_SPEC_SAMPLES;
			if ( index  >= 0) 
			{
			memcpy(bufl,pSamplesL+index,SCOPE_SPEC_SAMPLES*sizeof(float));
			memcpy(bufr,pSamplesR+index,SCOPE_SPEC_SAMPLES*sizeof(float));
			}
			else
			{
			index&=(SCOPE_BUF_SIZE-1);
			int index2=SCOPE_BUF_SIZE-index-1;
			memcpy(bufl,pSamplesL+index,index2*sizeof(float));
			memcpy(bufr,pSamplesR+index,index2*sizeof(float));
			memcpy(bufl,pSamplesL,(SCOPE_SPEC_SAMPLES-index2)*sizeof(float));
			memcpy(bufr,pSamplesR,(SCOPE_SPEC_SAMPLES-index2)*sizeof(float));

			}
			*/
			//						dsp::PowerSpectrum(minbars*2,bufl,ampl);
			//						dsp::PowerSpectrum(minbars*2,bufr,ampr);


			COLORREF cl = 0x402020;
			COLORREF cr = 0x204020;

			RECT rect;
			rect.left = 0;

			// draw our bands

			for (int i = 0; i < scope_spec_bands; i++)
			{
				//							int aml = 128-f2i(powf(1+((float)i/scope_spec_bands),2.0f)*sqrtf(ampl[i])*10);
				int aml = 128 - (log(1+ampl[i])*heightcompensation[i]*involL);
				//				int aml = 128-f2i(sqrtf(ampl[i]));
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
				bufDC->FillSolidRect(&rect,cl);

				rect.top = rect.bottom;
				rect.bottom = 128;
				bufDC->FillSolidRect(&rect,cl+0x804040);

				rect.left+=width;

				int amr = 128 - (log(1+ampr[i])*heightcompensation[i]*involR);
				//				int amr = 128-f2i(sqrtf(ampr[i]));
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
				bufDC->FillSolidRect(&rect,cr);

				rect.top = rect.bottom;
				rect.bottom = 128;
				bufDC->FillSolidRect(&rect,cr+0x804040);

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
			CFont* oldFont= bufDC->SelectObject(&font);
			bufDC->SetBkMode(TRANSPARENT);
			bufDC->SetTextColor(0x505050);
			bufDC->TextOut(4, 128-14, buf);
			bufDC->SelectObject(oldFont);

		}
		void CMultiScopeCtrl::PaintPhase(CDC* bufDC)
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
				float wl=(pSamplesL[index]*rangecorrection*involL);///32768; 
				float wr=(pSamplesR[index]*rangecorrection*involR);///32768; 
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
					clip = true;
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

			CPen* oldpen = bufDC->SelectObject(&linepenbL);

			x=f2i(sinf(-(F_PI/4.0f)-(o_mvdpl*F_PI/(4.0f)))
				*o_mvpl*(128.0f))+128;
			y=f2i(-cosf(-(F_PI/4.0f)-(o_mvdpl*F_PI/(4.0f)))
				*o_mvpl*(128.0f))+128;
			bufDC->MoveTo(x,y);
			bufDC->LineTo(128,128);
			bufDC->LineTo(128,128-f2i(o_mvpc*(128.0f)));
			bufDC->MoveTo(128,128);
			x=f2i(sinf((F_PI/4.0f)+(o_mvdpr*F_PI/(4.0f)))
				*o_mvpr*(128.0f))+128;
			y=f2i(-cosf((F_PI/4.0f)+(o_mvdpr*F_PI/(4.0f)))
				*o_mvpr*(128.0f))+128;
			bufDC->LineTo(x,y);

			// panning data
			bufDC->SelectObject(&linepenbR);

			x=f2i(sinf(-(o_mvdl*F_PI/(4.0f)))
				*o_mvl*(128.0f))+128;
			y=f2i(-cosf(-(o_mvdl*F_PI/(4.0f)))
				*o_mvl*(128.0f))+128;
			bufDC->MoveTo(x,y);
			bufDC->LineTo(128,128);
			bufDC->LineTo(128,128-f2i(o_mvc*(128.0f)));
			bufDC->MoveTo(128,128);
			x=f2i(sinf((o_mvdr*F_PI/(4.0f)))
				*o_mvr*(128.0f))+128;
			y=f2i(-cosf((o_mvdr*F_PI/(4.0f)))
				*o_mvr*(128.0f))+128;
			bufDC->LineTo(x,y);

			bufDC->SelectObject(&linepenL);

			x=f2i(sinf(-(F_PI/4.0f)-(mvdpl*F_PI/(4.0f)))
				*mvpl*(128.0f))+128;
			y=f2i(-cosf(-(F_PI/4.0f)-(mvdpl*F_PI/(4.0f)))
				*mvpl*(128.0f))+128;
			bufDC->MoveTo(x,y);
			bufDC->LineTo(128,128);
			bufDC->LineTo(128,128-f2i(mvpc*(128.0f)));
			bufDC->MoveTo(128,128);
			x=f2i(sinf((F_PI/4.0f)+(mvdpr*F_PI/(4.0f)))
				*mvpr*(128.0f))+128;
			y=f2i(-cosf((F_PI/4.0f)+(mvdpr*F_PI/(4.0f)))
				*mvpr*(128.0f))+128;
			bufDC->LineTo(x,y);

			// panning data
			bufDC->SelectObject(&linepenR);

			x=f2i(sinf(-(mvdl*F_PI/(4.0f)))
				*mvl*(128.0f))+128;
			y=f2i(-cosf(-(mvdl*F_PI/(4.0f)))
				*mvl*(128.0f))+128;
			bufDC->MoveTo(x,y);
			bufDC->LineTo(128,128);
			bufDC->LineTo(128,128-f2i(mvc*(128.0f)));
			bufDC->MoveTo(128,128);
			x=f2i(sinf((mvdr*F_PI/(4.0f)))
				*mvr*(128.0f))+128;
			y=f2i(-cosf((mvdr*F_PI/(4.0f)))
				*mvr*(128.0f))+128;
			bufDC->LineTo(x,y);

			if (!hold)
			{
				o_mvpl -= scope_phase_rate*32.0f;
				o_mvpc -= scope_phase_rate*32.0f;
				o_mvpr -= scope_phase_rate*32.0f;
				o_mvl -= scope_phase_rate*32.0f;
				o_mvc -= scope_phase_rate*32.0f;
				o_mvr -= scope_phase_rate*32.0f;
			}
			bufDC->SelectObject(oldpen);

			char buf[64];
			sprintf(buf,"Refresh %.2fhz",1000.0f/scope_phase_rate);
			CFont* oldFont= bufDC->SelectObject(&font);
			bufDC->SetBkMode(TRANSPARENT);
			bufDC->SetTextColor(0x505050);
			bufDC->TextOut(4, 128-14, buf);
			bufDC->SelectObject(oldFont);

		}
		void CMultiScopeCtrl::PaintConnections(CDC* bufDC)
		{
			RECT tempnode;
			for (int j = 0; j < numIns; j++) 
			{
				if ( ins_node_used[j])
				{
					tempnode = GetNodeRect (j, 0);
					bufDC->FillSolidRect (&tempnode, 0x00BBBBBB);
				}
			}

			for (int j = 0; j < numOuts; j++) 
			{
				if ( outs_node_used[j])
				{
					tempnode = GetNodeRect (j, 1);
					bufDC->FillSolidRect (&tempnode, 0x00BBBBBB);
				}
			}

			//draw lines;
			for (int i = 0; i < numIns; i++)
			{
				for (int j = 0; j < numOuts; j++)
				{
					//  check if value is "1" for a line.
					if (connections[i][j] == 1)
					{
						//draw line
						RECT temprect = GetNodeRect(i, 0);
						bufDC->MoveTo (temprect.left + 3, temprect.top + 3);
						RECT temprect2 = GetNodeRect(j, 1);
						bufDC->LineTo (temprect2.left + 3, temprect2.top + 3);
					}
				}
			}

			//unselect points
			if (!bOneSelected) {conn_sel_in = -1; conn_sel_out = -1;}

			//draw selected items
			//selected input

			if (conn_sel_in > -1)
			{
				RECT inrect;
				inrect = GetNodeRect (conn_sel_in, 0);
				bufDC->FillSolidRect (&inrect, (bSelectionType) ? 0x00FF0000 : 0x000000FF );
			}
			if (conn_sel_out > -1)
			{
				RECT outrect;
				outrect = GetNodeRect (conn_sel_out, 1);
				bufDC->FillSolidRect (&outrect, (bSelectionType) ? 0x00FF0000 : 0x000000FF);
			}
		}

		RECT CMultiScopeCtrl::GetNodeRect (int node, bool bDirection)
		{
			RECT rect;
			if (bDirection)
			{
				//an output
				rect.left = 213;
				rect.right = rect.left + 7;

				rect.top = rccons.top + (node + 0.5) * conn_outs_spacing - 4;
				rect.bottom = rect.top + 7;
			}
			else
			{
				//an input
				rect.left = 37;
				rect.right = rect.left + 7;

				rect.top = rccons.top + (node + 0.5) * conn_ins_spacing - 4;
				rect.bottom = rect.top + 7;
			}
			return rect;
		}

//////////////////////////////////////////////////////////////////////////
//		CWireDlg Class implementation
//////////////////////////////////////////////////////////////////////////

		CWireDlg::CWireDlg(CChildView* pParent) : CDialog(CWireDlg::IDD, pParent)
		{
			m_pParent = pParent;
			//todo:
			//pWire = _pWire;
			//pWire->SetScopeBuffer(pSamplesL,pSamplesR);
		}

		void CWireDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_SLIDER, m_slspeed);
			DDX_Control(pDX, IDC_SLIDER2, m_slsize);
			DDX_Control(pDX, IDC_SCOPE_PARAM_1, m_lblsize);
			DDX_Control(pDX, IDC_SCOPE_PARAM_2, m_lblspeed);
			DDX_Control(pDX, IDC_BUTTON, m_mode);
			DDX_Control(pDX, IDC_VOLUME_DB, m_btndb);
			DDX_Control(pDX, IDC_VOLUME_PER, m_btnper);
			DDX_Control(pDX, IDC_SLIDER1, m_slvolume);
			DDX_Control(pDX, IDC_MULTISCOPE, m_multiscope);
		}

		BEGIN_MESSAGE_MAP(CWireDlg, CDialog)
			ON_WM_TIMER()
			ON_WM_SIZING()
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER, OnCustomdrawSlspeed)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlsize)
			ON_BN_CLICKED(IDC_BUTTON, OnMode)
			ON_BN_CLICKED(IDC_SCOPE_HOLD, OnHold)
			ON_BN_CLICKED(IDC_BUTTON1, OnDelete)
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlvolume)
			ON_BN_CLICKED(IDC_VOLUME_DB, OnVolumeDb)
			ON_BN_CLICKED(IDC_VOLUME_PER, OnVolumePer)
			ON_BN_CLICKED(IDC_VIEW_SCOPE, OnBnClickedViewScope)
			ON_BN_CLICKED(IDC_VIEW_CONNECTIONS, OnBnClickedViewConnections)
			ON_WM_LBUTTONDOWN()
			ON_WM_RBUTTONDOWN()
		END_MESSAGE_MAP()

		BOOL CWireDlg::PreTranslateMessage(MSG* pMsg) 
		{
			/*
			if (pMsg->message == WM_KEYDOWN)
			{
				if (pMsg->wParam == VK_UP)
				{
					int v = m_slvolume.GetPos();
					v--;
					if (v < 0)
					{
						v = 0;
					}
					m_slvolume.SetPos(v);
					return true;
				}
				else if (pMsg->wParam == VK_DOWN)
				{
					int v = m_slvolume.GetPos();
					v++;
					if (v > 256*4)
					{
						v = 256*4;
					}
					m_slvolume.SetPos(v);
					return true;
				}
				else
				{
					m_pParent->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
				}
			}
			else if (pMsg->message == WM_KEYUP)
			{
				m_pParent->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
			}
			*/
			if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP))
			{
				m_pParent->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
				return true;
			}
			else return CDialog::PreTranslateMessage(pMsg);
		}


		BOOL CWireDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			std::ostringstream s;
			s << "[" << wireIndex << "]" << _pSrcMachine->GetEditName() << " -> " << _pDstMachine->_editName;
			SetWindowText(s.str().c_str());

			m_slvolume.SetRange(0,256*4);
			m_slvolume.SetTicFreq(16*4);

			float val;
			_dstWireIndex = _pDstMachine->FindInputWire(isrcMac);
			_pDstMachine->GetWireVolume(_dstWireIndex,val);
			m_multiscope.involL = val*_pSrcMachine->_lVol; 
			m_multiscope.involR = val*_pSrcMachine->_rVol;
			int t = (int)sqrtf(val*16384*4*4);
			m_slvolume.SetPos(256*4-t);


			((CButton*)GetDlgItem(IDC_VIEW_SCOPE))->SetCheck(1);

			std::memset(pSamplesL,0,sizeof(pSamplesL));
			std::memset(pSamplesR,0,sizeof(pSamplesR));
			_pSrcMachine->_pScopeBufferL = pSamplesL;
			_pSrcMachine->_pScopeBufferR = pSamplesR;
			_pSrcMachine->_scopeBufferIndex = 0;

			m_multiscope.pSamplesL=pSamplesL;
			m_multiscope.pSamplesR=pSamplesR;
			m_multiscope._pSrcMachine=_pSrcMachine;
			m_multiscope._pDstMachine=_pDstMachine;
			m_multiscope.Initialize();

			SetMode();

			return true;
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
			delete this;
		}

		void CWireDlg::OnBnClickedViewScope()
		{
			if (((CButton*)GetDlgItem(IDC_VIEW_SCOPE))->GetCheck ())
			{
				m_multiscope.scope_mode=CMultiScopeCtrl::mode_vu;
				SetMode();
				m_multiscope.Invalidate();
			}
		}

		void CWireDlg::OnBnClickedViewConnections()
		{
			if (((CButton*)GetDlgItem(IDC_VIEW_CONNECTIONS))->GetCheck ())
			{
				m_multiscope.scope_mode=CMultiScopeCtrl::mode_viewcons;
				SetMode();
				m_multiscope.Invalidate();
			}
		}


		void CWireDlg::OnDelete() 
		{
			m_pParent->AddMacViewUndo();
			KillTimer(2304+this_index);
			_pSrcMachine->_connection[wireIndex] = false;
			_pSrcMachine->_outputMachines[wireIndex]=-1;
			_pSrcMachine->_connectedOutputs--;
			
			_pDstMachine->_inputCon[_dstWireIndex] = false;
			_pDstMachine->_inputMachines[_dstWireIndex]=-1;
			_pDstMachine->_connectedInputs--;
			OnCancel();
		}


		void CWireDlg::OnCustomdrawSlspeed(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			switch (m_multiscope.scope_mode)
			{
			case CMultiScopeCtrl::mode_vu:
				if (m_multiscope.scope_peak_rate != m_slspeed.GetPos())
				{
					m_multiscope.scope_peak_rate = m_slspeed.GetPos();
					KillTimer(2304+this_index);
					SetTimer(2304+this_index,m_multiscope.scope_peak_rate,0);
				}
				break;
			case CMultiScopeCtrl::mode_scope:
				if (m_multiscope.hold)
				{
					m_multiscope.scope_offset = m_slspeed.GetPos()-1;
				}
				else
				{
					if (m_multiscope.scope_osc_rate != m_slspeed.GetPos())
					{
						m_multiscope.scope_osc_rate = m_slspeed.GetPos();
						KillTimer(2304+this_index);
						SetTimer(2304+this_index,m_multiscope.scope_osc_rate,0);
					}
				}
				break;
			case CMultiScopeCtrl::mode_spectrum:
				if (m_multiscope.scope_spec_rate != m_slspeed.GetPos())
				{
					m_multiscope.scope_spec_rate = m_slspeed.GetPos();
					KillTimer(2304+this_index);
					SetTimer(2304+this_index,m_multiscope.scope_spec_rate,0);
				}
				break;
			case CMultiScopeCtrl::mode_phase:
				if (m_multiscope.scope_phase_rate != m_slspeed.GetPos())
				{
					m_multiscope.scope_phase_rate = m_slspeed.GetPos();
					KillTimer(2304+this_index);
					SetTimer(2304+this_index,m_multiscope.scope_phase_rate,0);
				}
				break;
			}
		//	m_pParent->SetFocus();	
			*pResult = 0;
		}

		void CWireDlg::OnCustomdrawSlsize(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			switch (m_multiscope.scope_mode)
			{
			case CMultiScopeCtrl::mode_scope:
				m_multiscope.scope_osc_freq = m_slsize.GetPos();
				if (m_multiscope.hold)
				{
					m_slspeed.SetRange(0,SCOPE_BUF_SIZE&int(Global::player().SampleRate()*2.0f/(m_multiscope.scope_osc_freq*m_multiscope.scope_osc_freq)));
				}
				break;
			case CMultiScopeCtrl::mode_spectrum:
				m_multiscope.scope_spec_bands = m_slsize.GetPos();
				m_multiscope.InitSpectrum();
				break;
			}
		//	m_pParent->SetFocus();	
			*pResult = 0;
		}

		void CWireDlg::OnMode()
		{
			m_multiscope.scope_mode++;
			if (m_multiscope.scope_mode == CMultiScopeCtrl::num_osci_modes)
			{
				m_multiscope.scope_mode = CMultiScopeCtrl::mode_vu;
			}
			SetMode();
		//	m_pParent->SetFocus();	
		}

		void CWireDlg::OnHold()
		{
			m_multiscope.hold = !m_multiscope.hold;
			switch (m_multiscope.scope_mode)
			{
			case CMultiScopeCtrl::mode_scope:
				if (m_multiscope.hold)
				{
					m_lblspeed.SetWindowText("Offset:");
					m_slspeed.SetRange(0,SCOPE_BUF_SIZE&int(Global::player().SampleRate()*2.0f/(m_multiscope.scope_osc_freq*m_multiscope.scope_osc_freq)));
					m_slspeed.SetPos(m_multiscope.scope_offset);
				}
				else
				{
					m_lblspeed.SetWindowText("Refresh:");
					m_multiscope.scope_offset = 0;
					m_slspeed.SetRange(10,100);
					m_slspeed.SetPos(m_multiscope.scope_osc_rate);
				}
			}
		//	m_pParent->SetFocus();	
		}

		void CWireDlg::OnCustomdrawSlvolume(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			char bufper[32];
			char bufdb[32];

			const float curvol = ((256*4-m_slvolume.GetPos())*(256*4-m_slvolume.GetPos()))/(16384.0f*4*4);
			if (curvol >= 1.0f)
			{	
				sprintf(bufper,"%.2f%%",curvol*100); 
				sprintf(bufdb,"+%.1f dB",20.0f * log10(curvol)); 
			}
/*			else if (curvol == 1.0f)
			{	
				sprintf(bufper,"100.00%%"); 
				sprintf(bufdb,"0.0 dB"); 
			}*/
			else if (curvol > 0.0f)
			{	
				sprintf(bufper,"%.2f%%",curvol*100); 
				sprintf(bufdb,"%.1f dB",20.0f * log10(curvol)); 
			}
			else 
			{				
				sprintf(bufper,"0.00%%"); 
				sprintf(bufdb,"-Inf. dB"); 
			}

			m_btnper.SetWindowText(bufper);
			m_btndb.SetWindowText(bufdb);

			float f;
			_pDstMachine->GetWireVolume(_dstWireIndex, f);
			if (f != curvol)
			{
				m_pParent->AddMacViewUndo();
				_pDstMachine->SetWireVolume(_dstWireIndex, curvol );
				m_multiscope.involL = curvol*_pSrcMachine->_lVol;
				m_multiscope.involR = curvol*_pSrcMachine->_rVol;;
				invol = curvol;
			}

		//	m_pParent->SetFocus();	
			*pResult = 0;
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
				m_slvolume.SetPos(256*4-t);
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
				m_slvolume.SetPos(256*4-t);
			}
		}

		void CWireDlg::OnTimer(UINT nIDEvent) 
		{
			if ( nIDEvent == 2304+this_index )
			{
				float val;
				_pDstMachine->GetWireVolume(_dstWireIndex,val);
				m_multiscope.involL = val*_pSrcMachine->_lVol; 
				m_multiscope.involR = val*_pSrcMachine->_rVol;
				m_multiscope.Invalidate();
			}
//			CDialog::OnTimer(nIDEvent);
		}


		void CWireDlg::OnLButtonDown(UINT nFlags, CPoint point)
		{
			GraphClicked(0, point);
			CDialog::OnLButtonDown(nFlags, point);
		}

		void CWireDlg::OnRButtonDown(UINT nFlags, CPoint point)
		{
			GraphClicked(1, point);			
			CDialog::OnRButtonDown(nFlags, point);
		}

		void CWireDlg::OnSizing(UINT fwSide, LPRECT pRect)
		{
			CDialog::OnSizing(fwSide, pRect);

			//reposition controls
			//set drawing vars to account for resizing
		}
	
		void CWireDlg::SetMode()
		{
			std::ostringstream buf;
			KillTimer(2304+this_index);
			//alter sliders/text
			switch (m_multiscope.scope_mode)
			{
			case CMultiScopeCtrl::mode_vu:
				m_multiscope.peakL = m_multiscope.peakR = m_multiscope.peak2L = m_multiscope.peak2R = 128.0f;
				m_slspeed.SetPos(m_multiscope.scope_peak_rate);
				m_lblsize.SetWindowText(""); m_slsize.ShowWindow(SW_HIDE);
				buf << "Vu Mode";
				SetTimer(2304+this_index,m_multiscope.scope_peak_rate,0);
				break;
			case CMultiScopeCtrl::mode_scope:
				if (m_multiscope.hold) m_lblspeed.SetWindowText("Offset:");
				m_slspeed.SetPos(m_multiscope.scope_osc_rate);
				m_lblsize.SetWindowText("Frequency:"); m_slsize.ShowWindow(SW_SHOW);
				m_slsize.SetRange(5, 100);	m_slsize.SetPos(m_multiscope.scope_osc_freq);
				buf <<"Oscilloscope";
				SetTimer(2304+this_index,m_multiscope.scope_osc_rate,0);
				break;
			case CMultiScopeCtrl::mode_spectrum:
				m_slspeed.SetPos(m_multiscope.scope_spec_rate);
				m_lblsize.SetWindowText("Bands:"); m_slsize.ShowWindow(SW_SHOW);
				m_slsize.SetRange(4, MAX_SCOPE_BANDS);
				m_slsize.SetPos(m_multiscope.scope_spec_bands);
				buf << "Spectrum Analyzer";
				SetTimer(2304+this_index,m_multiscope.scope_spec_rate,0);
				break;
			case CMultiScopeCtrl::mode_phase:
				m_multiscope.o_mvc = m_multiscope.o_mvpc = m_multiscope.o_mvl = m_multiscope.o_mvdl = m_multiscope.o_mvpl = 0;
				m_multiscope.o_mvdpl = m_multiscope.o_mvr = m_multiscope.o_mvdr = m_multiscope.o_mvpr = m_multiscope.o_mvdpr = 0.0f;
				m_slspeed.SetPos(m_multiscope.scope_phase_rate);
				m_lblsize.SetWindowText(""); m_slsize.ShowWindow(SW_HIDE);
				buf << "Stereo Phase";
				SetTimer(2304+this_index,m_multiscope.scope_phase_rate,0);
				break;
			default:
				break;
			}
			if ( m_multiscope.scope_mode == CMultiScopeCtrl::mode_viewcons)
			{
				m_lblsize.ShowWindow (SW_HIDE);	m_lblspeed.ShowWindow (SW_HIDE);
				m_slsize.ShowWindow (SW_HIDE);	m_slspeed.ShowWindow (SW_HIDE);
				m_mode.ShowWindow (SW_HIDE);
				((CButton*)GetDlgItem(IDC_SCOPE_HOLD))->ShowWindow (SW_HIDE);
			}
			else
			{
				m_lblspeed.SetWindowText("Refresh:");
				m_slspeed.SetRange(10,100);
				m_lblspeed.ShowWindow (SW_SHOW);	m_slspeed.ShowWindow (SW_SHOW);
				m_mode.ShowWindow (SW_SHOW);	m_mode.SetWindowText(buf.str().c_str());
				((CButton*)GetDlgItem(IDC_SCOPE_HOLD))->ShowWindow (SW_SHOW);
			}

			m_multiscope.PrepareView();
		}


		void CWireDlg::GraphClicked (bool bSelType, CPoint point)
		{
			/*if (LastSelectionType != bSelectionType)
			{
				return;
			} */
			
			//NEED TO ADD MORE OF THE ABOVE STUFF ELSEWHERE IN THIS CLASS
			//check if user has clicked a point
			RECT temprect; int i(0), h(0);
			bool bPointClicked = false; bool bPointType=0;
			if ((point.x > 36) && (point.x < 44))
			{
				while (i < m_multiscope.numIns)
				{
					temprect = m_multiscope.GetNodeRect(i, 0);
					if (PtInRect(&temprect, point))	
					{
						m_multiscope.conn_sel_in = i;
						bPointClicked = true;
						i = m_multiscope.numIns + 2; //make loop end prematurely
						bPointType = 0;
					}
					i++;
				}
			}
			if (i != m_multiscope.numIns + 2)
			{
				if ((point.x > 212) && (point.x < 220))
				{
					while (h < m_multiscope.numOuts)
					{
						temprect = m_multiscope.GetNodeRect(h, 1);
						if (PtInRect(&temprect, point))	
						{
							m_multiscope.conn_sel_out = h;
							bPointClicked  = true;
							h = m_multiscope.numIns; //make loop end prematurely
							bPointType = 1;
						}
						h++;
					}
				}
			}
			if (bPointClicked)
			{
				//STILL ERRORS HERE
				if (m_multiscope.bLastSelectionType == bSelType)
				{
					if (m_multiscope.bOneSelected)
					{
						//one already selected, set the line between.
						if (m_multiscope.bSelectionType)
						{
							m_multiscope.connections[m_multiscope.conn_sel_in][m_multiscope.conn_sel_out] = 0;
							//check if both nodes are still being used 
							int i = 0; bool bConnFound = false;
							while (i < m_multiscope.numOuts)
							{
								if (m_multiscope.connections[m_multiscope.conn_sel_in][i])
								{
									bConnFound = true;
									i = m_multiscope.numOuts;
								}
								i++;
							}
							if (!bConnFound)
							{
                                m_multiscope.ins_node_used[m_multiscope.conn_sel_in] = 0;
							}
							
							i = 0; bConnFound = false;
							while (i < m_multiscope.numIns)
							{
								if (m_multiscope.connections[i][m_multiscope.conn_sel_out])
								{
									bConnFound = true;
									i = m_multiscope.numIns;
								}
								i++;
							}
							if (!bConnFound)
							{
                                m_multiscope.outs_node_used[m_multiscope.conn_sel_out] = 0;
							}
							
						}
						else
						{
							m_multiscope.connections[m_multiscope.conn_sel_in][m_multiscope.conn_sel_out] = 1;
							m_multiscope.ins_node_used[m_multiscope.conn_sel_in] = 1;
							m_multiscope.outs_node_used[m_multiscope.conn_sel_out] = 1;
						}
						m_multiscope.bOneSelected = false;
					}
					else
					{
						m_multiscope.bOneSelected = true;
					}
					
				}
				else
				{ 
					m_multiscope.bOneSelected = true;
					if (bPointType)
					{
						m_multiscope.conn_sel_in = -1;//conn_sel_out = -1;
					}
					else
					{
						m_multiscope.conn_sel_out = -1;//conn_sel_in = -1;
					}
				}
				m_multiscope.bLastSelectionType = m_multiscope.bSelectionType;
				m_multiscope.bSelectionType = bSelType;
				
			}
			//bLastSelectionType = bSelectionType;
			//char buffer[100]; sprintf (buffer, "%d", conn_sel_in);
			//SetWindowText (buffer);
			m_multiscope.Invalidate();
		}


	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
