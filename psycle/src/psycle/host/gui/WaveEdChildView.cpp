///\file
///\brief implementation file for psycle::host::CWaveEdChildView.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/host/gui/psycle.hpp>
#include <psycle/host/gui/WaveEdChildView.hpp>
#include <psycle/host/gui/MainFrm.hpp>
#include <mmreg.h>
#include <math.h>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		CWaveEdChildView::CWaveEdChildView()
		{
			cpen_lo.CreatePen(PS_SOLID,0,0xFF0000);
			cpen_me.CreatePen(PS_SOLID,0,0xCCCCCC);
			cpen_hi.CreatePen(PS_SOLID,0,0x00FF00);

			hResizeLR = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
			hIBeam = AfxGetApp()->LoadStandardCursor(IDC_IBEAM);

			drawwave=true;
			SelStart=0;
			
			wdWave=false;
			wsInstrument=-1;

			selx=0;selx2=0;
		}

		CWaveEdChildView::~CWaveEdChildView()
		{
			cpen_lo.DeleteObject();
			cpen_me.DeleteObject();
			cpen_hi.DeleteObject();
		}

		BEGIN_MESSAGE_MAP(CWaveEdChildView, CWnd)
			//{{AFX_MSG_MAP(CWaveEdChildView)
			ON_WM_PAINT()
			ON_WM_RBUTTONDOWN()
			ON_WM_LBUTTONDOWN()
			ON_WM_LBUTTONDBLCLK()
			ON_WM_MOUSEMOVE()
			ON_WM_LBUTTONUP()
			ON_COMMAND(ID_SELECCION_ZOOM, OnSelectionZoom)
			ON_COMMAND(ID_SELECCION_ZOOMOUT, OnSelectionZoomOut)
			ON_COMMAND(ID_SELECTION_FADEIN, OnSelectionFadeIn)
			ON_COMMAND(ID_SELECTION_FADEOUT, OnSelectionFadeOut)
			ON_COMMAND(ID_SELECTION_NORMALIZE, OnSelectionNormalize)
			ON_COMMAND(ID_SELECTION_REMOVEDC, OnSelectionRemoveDC)
			ON_COMMAND(ID_SELECTION_AMPLIFY, OnSelectionAmplify)
			ON_COMMAND(ID_SELECTION_REVERSE, OnSelectionReverse)
			ON_COMMAND(ID_SELECTION_SHOWALL, OnSelectionShowall)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_AMPLIFY, OnUpdateSelectionAmplify)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_REVERSE, OnUpdateSelectionReverse)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_FADEIN, OnUpdateSelectionFadein)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_FADEOUT, OnUpdateSelectionFadeout)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_NORMALIZE, OnUpdateSelectionNormalize)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_REMOVEDC, OnUpdateSelectionRemovedc)
			ON_UPDATE_COMMAND_UI(ID_SELECCION_ZOOM, OnUpdateSeleccionZoom)
			ON_UPDATE_COMMAND_UI(ID_SELECCION_ZOOMOUT, OnUpdateSeleccionZoomout)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_SHOWALL, OnUpdateSelectionShowall)
			ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
			ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
			ON_COMMAND(ID_EDIT_CUT, OnEditCut)
			ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
			ON_COMMAND(ID_EDIT_CROP, OnEditCrop)
			ON_UPDATE_COMMAND_UI(ID_EDIT_CROP, OnUpdateEditCrop)
			ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
			ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
			ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
			ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
			ON_COMMAND(ID_CONVERT_MONO, OnConvertMono)
			ON_UPDATE_COMMAND_UI(ID_CONVERT_MONO, OnUpdateConvertMono)
			ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
			ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
			ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
			ON_WM_DESTROYCLIPBOARD()
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		/////////////////////////////////////////////////////////////////////////////
		// CWaveEdChildView drawing

		void CWaveEdChildView::OnPaint(void)
		{
			CPaintDC dc(this);
			CPaintDC *pDC = &dc;

		//	int wrHeight = 0, wrHeight_R = 0, yLow = 0, yHi = 0, absBuf = 0, abs_yBuf = 0;
			int wrHeight = 0, yLow = 0, yHi = 0;
			double OffsetStep = 0;
			__int32 c, d;
		//	LOGPEN _pen;
				
			if(wdWave)
			{
				CRect rect;
				GetClientRect(&rect);
				
				int const nWidth=rect.Width();
				int const nHeight=rect.Height();
				int const my=nHeight/2;
				
				if(wdStereo) wrHeight=my/2;
				else wrHeight=my;
				
				if(drawwave)
				{
					// Draw preliminary stuff
					
					CPen *oldpen= pDC->SelectObject(&cpen_me);
					
					// Left channel 0 amplitude line
					pDC->MoveTo(0,wrHeight);
					pDC->LineTo(nWidth,wrHeight);
					
					int const wrHeight_R = my + wrHeight;
					
					if(wdStereo)
					{
						// Stereo channels separator line
						pDC->SelectObject(&cpen_lo);
						pDC->MoveTo(0,my);
						pDC->LineTo(nWidth,my);
						
						// Right channel 0 amplitude line
						pDC->SelectObject(&cpen_me);
						pDC->MoveTo(0,wrHeight_R);
						pDC->LineTo(nWidth,wrHeight_R);
					}
					
					// Draw samples in channels (Fideloop's)

					pDC->SelectObject(&cpen_hi);
		//			cpen_hi.GetLogPen(&_pen);

					OffsetStep = (double) diLength / nWidth;

					for(c = 0; c < nWidth; c++)
					{
						long const offset = diStart + (long)(c * OffsetStep);

						yLow = 0, yHi = 0;

						for (d = offset; d < offset + ((OffsetStep <1) ? 1 : OffsetStep); d++)
						{
							if (yLow > *(wdLeft + d)) yLow = *(wdLeft + d);
							if (yHi < *(wdLeft + d)) yHi = *(wdLeft + d);
						}

						int const ryLow = (wrHeight * yLow)/32768; // 32767...
						int const ryHi = (wrHeight * yHi)/32768;
						
						pDC->MoveTo(c,wrHeight - ryLow);
						pDC->LineTo(c,wrHeight - ryHi);
					}

					if(wdStereo)
					{
						for(c = 0; c < nWidth; c++)
						{
							long const offset = diStart + (long)(c * OffsetStep);

							yLow = 0, yHi = 0;

							for (d = offset; d < offset + ((OffsetStep <1) ? 1 : OffsetStep); d++)
							{
								if (yLow > *(wdRight + d)) yLow = *(wdRight + d);
								if (yHi < *(wdRight + d)) yHi = *(wdRight + d);
							}

							int const ryLow = (wrHeight * yLow)/32768; // 32767...
							int const ryHi = (wrHeight * yHi)/32768;
							
							pDC->MoveTo(c,wrHeight_R - ryLow);
							pDC->LineTo(c,wrHeight_R - ryHi);
						}
					}

					if ( wdLoop )
					{
						pDC->SelectObject(&cpen_lo);
						if ( wdLoopS >= diStart && wdLoopS < diStart+diLength)
						{
							int ls = ((wdLoopS-diStart)*nWidth)/diLength;
							pDC->MoveTo(ls,0);
							pDC->LineTo(ls,nHeight);
							pDC->TextOut(ls,0,"Start");
						}
						if ( wdLoopE >= diStart && wdLoopE < diStart+diLength)
						{
							int le = ((wdLoopE-diStart)*nWidth)/diLength;
							pDC->MoveTo(le,0);
							pDC->LineTo(le,nHeight);
							pDC->TextOut(le-8,nHeight-16,"End");
						}

					}
					pDC->SelectObject(oldpen);
					
				}// Draw wave
				
				pDC->SetROP2(R2_NOT);
				
				if(!drawwave)
				{
					// Unmark selection
					pDC->Rectangle(selx,0,selx2,nHeight);
				}
				
				selx=((blStart-diStart)*nWidth)/diLength;
				selx2=(((blStart+blLength)-diStart)*nWidth)/diLength;
				pDC->Rectangle(selx,0,selx2,nHeight);
			}
			else
			{
				pDC->TextOut(4,4,"No Wave Data");
			}
			
			drawwave=true;
			
			// Do not call CWnd::OnPaint() for painting messages
		}

		/////////////////////////////////////////////////////////////////////////////
		// CWaveEdChildView message handlers

		BOOL CWaveEdChildView::PreCreateWindow(CREATESTRUCT& cs) 
		{
			LOGBRUSH LogB;
			HBRUSH	hBrush;

			LogB.lbColor = 0x00000000;
			LogB.lbStyle = BS_SOLID;

			hBrush = CreateBrushIndirect(&LogB);
			
			if (!CWnd::PreCreateWindow(cs)) return FALSE;

			cs.dwExStyle |= WS_EX_CLIENTEDGE;
			cs.style &= ~WS_BORDER;
			cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
			::LoadCursor(NULL, IDC_ARROW), hBrush, NULL);

			DeleteObject(hBrush);

			return TRUE;
			
		//	return CWnd::PreCreateWindow(cs);
		}

		void CWaveEdChildView::GenerateAndShow()
		{
			blSelection=false;
			UpdateWindow();
		}

		void  CWaveEdChildView::SetViewData(int ins)
		{
			int wl=_pSong->_pInstrument[ins]->waveLength;

			wsInstrument=ins;	// Do not put inside of "if(wl)". Pasting needs this.

			if(wl)
			{
				wdWave=true;
					
				wdLength=wl;
				wdLeft=_pSong->_pInstrument[ins]->waveDataL;
				wdRight=_pSong->_pInstrument[ins]->waveDataR;
				wdStereo=_pSong->_pInstrument[ins]->waveStereo;
				wdLoop=_pSong->_pInstrument[ins]->waveLoopType;
				wdLoopS=_pSong->_pInstrument[ins]->waveLoopStart;
				wdLoopE=_pSong->_pInstrument[ins]->waveLoopEnd;

				diStart=0;
				diLength=wl;
				blStart=0;
				blLength=0;
				Invalidate();
			}
			else
			{
				wdWave=false;
				
				SetWindowText("Wave Editor [No Data]");
				Invalidate(true);
			}

			blSelection=false;
		}

		/*void CWaveEdChildView::FitWave()
		{
			if (blStart < 0) blStart = 0;
			if ( (blStart + blLength) > wdLength) blLength = wdLength - blStart;
		}*/

		void CWaveEdChildView::OnSelectionZoom()
		{
			if(blSelection && wdWave)
			{
				CRect rect;
				GetClientRect(&rect);
				unsigned long const nWidth=rect.Width();

				if ( blLength*8 < nWidth ) // Selection is too small, zoom to smallest possible
				{
					diLength=(unsigned long)(nWidth*0.125f);
					if ( diLength > wdLength ) { diStart=0; diLength=wdLength; }
					else if ( blStart+diLength > wdLength ) diStart = wdLength-diLength;
					else diStart = blStart;
				}
				else
				{
					diStart= blStart;
					diLength = blLength;
				}
				Invalidate();
			}
		}

		void CWaveEdChildView::OnSelectionZoomOut()
		{
			if(diLength<wdLength)
			{
				if ( diLength >= diStart )
					diStart=0;
				else
					diStart-=diLength;
				
				diLength=diLength*3;
				if(diLength+diStart>wdLength) diLength=wdLength-diStart;
				
				Invalidate();
			}
		}

		void CWaveEdChildView::OnRButtonDown(UINT nFlags, CPoint point) 
		{
			if(wdWave)
			{
				int const x=point.x;

				if ( nFlags & MK_CONTROL )
				{
					pParent->m_wndView.AddMacViewUndo();
					_pSong->IsInvalided(true);
					Sleep(LOCK_LATENCY);

					CRect rect;
					GetClientRect(&rect);
					wdLoopE = diStart+((x*diLength)/rect.Width());
					_pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopE;
					if (_pSong->_pInstrument[wsInstrument]->waveLoopStart> wdLoopE )
					{
						_pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopE;
					}
					if (!wdLoop) 
					{
						wdLoop=true;
						_pSong->_pInstrument[wsInstrument]->waveLoopType=true;
					}
					_pSong->IsInvalided(false);
					drawwave=true;
					pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
					Invalidate();

				}
				else
				{
					if (blSelection) OnSelectionZoom();
					else OnSelectionZoomOut();
				}
			}
			CWnd::OnRButtonDown(nFlags, point);
		}


		void CWaveEdChildView::OnLButtonDown(UINT nFlags, CPoint point) 
		{
			SetCapture();
			if(wdWave)
			{
				int const x=point.x;

				if ( nFlags & MK_CONTROL )
				{
					pParent->m_wndView.AddMacViewUndo();
					_pSong->IsInvalided(true);
					Sleep(LOCK_LATENCY);

					CRect rect;
					GetClientRect(&rect);
					wdLoopS = diStart+((x*diLength)/rect.Width());
					_pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopS;
					if (_pSong->_pInstrument[wsInstrument]->waveLoopEnd < wdLoopS )
					{
						_pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopS;
					}
					if (!wdLoop) 
					{
						wdLoop=true;
						_pSong->_pInstrument[wsInstrument]->waveLoopType=true;
					}
					_pSong->IsInvalided(false);
					pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
					drawwave=true;
					Invalidate();
				}
				else if ( nFlags == MK_LBUTTON )
				{
					CRect rect;
					GetClientRect(&rect);
					int const nWidth=rect.Width();

					//the casts are to resolve ambiguity with abs()
					if		( blSelection	&&	diLength!=0		/*jic..*/	&&
							abs(int(x - (blStart-diStart)			* nWidth / diLength)) < 10 )	//mouse down on block start
					{
						SelStart = (blStart+blLength-diStart)*nWidth/diLength;			//set SelStart to the end we're -not- moving
					}
					else if ( blSelection	&&	diLength!=0		/*jic..*/	&&
							abs(int(x - (blStart+blLength-diStart)	* nWidth / diLength)) < 10 )	//mouse down on block end
					{
						SelStart = (blStart-diStart)*nWidth/diLength;					//set SelStart to the end we're -not- moving
					}
					else
					{
						blSelection=false;
						
						blStart=diStart+((x*diLength)/nWidth);
						blLength=0;
						SelStart = x; //!!! new variable!

					}

					::SetCursor(hResizeLR);
					
					drawwave=false;
				
					Invalidate(false);
				}
			}
			
			CWnd::OnLButtonDown(nFlags, point);
		}

		void CWaveEdChildView::OnLButtonDblClk( UINT nFlags, CPoint point )
		{
			//todo: any ideas what this is for? blSelection gets turned off on the first LButtonDown unless control or
			//		shift is down..  but if control is down on a dblclk, the display is changed into monochromatic modern art.
			//		plus, it appears to set the selection to only the visible portion of the wave.  is it meant to be a 
			//		shortcut to OnEditSelectAll()?
			if(blSelection)
			{
				blStart=diStart;
				blLength=diLength;
				drawwave=false;
				Invalidate(false);
			}
		}

		void CWaveEdChildView::OnMouseMove(UINT nFlags, CPoint point) //Fideloop's
		{
			int x=point.x;
			CRect rect;
			GetClientRect(&rect);
			int const nWidth=rect.Width();

			if	(		blSelection		&&		diLength!=0		/*jic..*/	&&
					(	abs ( int( x - ( blStart-diStart )			* nWidth / diLength))  < 10	||
						abs ( int( x - ( blStart+blLength-diStart)	* nWidth / diLength))  < 10	)
				)
				::SetCursor(hResizeLR);
			else
				::SetCursor(hIBeam);

			if(nFlags == MK_LBUTTON && wdWave)
			{
				float diRatio = (float) diLength/nWidth;
				
				if (x >= (long) SelStart)
				{
					if (x > nWidth)	{ x = nWidth; }
					blStart = (long) (SelStart*diRatio + diStart);
					blLength = (long)(x*diRatio + diStart - blStart);

				}
								
				else
				{
					if (x < 0) { x = 0; }
					blStart = (long) ( x*diRatio + diStart);
					blLength = (long) (SelStart*diRatio + diStart - blStart);
				}
				blSelection=true;
				drawwave=false;
				Invalidate(false);
			}
			
			
			CWnd::OnMouseMove(nFlags, point);
		}

		void CWaveEdChildView::OnLButtonUp(UINT nFlags, CPoint point) 
		{
			if(blLength==0)
				blSelection=false;
			ReleaseCapture();
			CWnd::OnLButtonUp(nFlags, point);
		}

		void CWaveEdChildView::OnSelectionFadeIn()
		{
			if(blSelection && wdWave)
			{
				pParent->m_wndView.AddMacViewUndo();

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);

				double fpass=1.0/blLength;
				double val=0.0f;
				
				for(unsigned long c=blStart;c<blStart+blLength;c++)
				{
					float const vl=*(wdLeft+c);
					
					*(wdLeft+c)=signed short(vl*val);
					
					if(wdStereo)
					{
						float const vr=*(wdRight+c);
						*(wdRight+c)=signed short(vr*val);
					}
					
					val+=fpass;
				}
				
				Invalidate(true);
				_pSong->IsInvalided(false);
			}
		}


		void CWaveEdChildView::OnSelectionFadeOut()
		{
			if(blSelection && wdWave)
			{
				pParent->m_wndView.AddMacViewUndo();

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);

				double fpass=1.0/blLength;
				double val=1.0f;
				
				for(unsigned long c=blStart;c<blStart+blLength;c++)
				{
					
					float const vl=*(wdLeft+c);
					
					*(wdLeft+c)=signed short(vl*val);
					
					if(wdStereo)
					{
						float const vr=*(wdRight+c);
						*(wdRight+c)=signed short(vr*val);
					}
					
					val-=fpass;
				}
				
				Invalidate(true);
				_pSong->IsInvalided(false);
			}
		}

		void CWaveEdChildView::OnSelectionNormalize() // (Fideloop's)
		{
			signed short maxL = 0, maxR = 0, absBuf;
			double ratio = 0, buf;
			unsigned long c = 0;

			if (wdWave)
			{
				pParent->m_wndView.AddMacViewUndo();

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);

				for (c = 0 ; c < wdLength ; c++)
				{

					if (*(wdLeft+c) < 0)
					{
						if (!( *(wdLeft+c) == -32768) )	absBuf = - *(wdLeft + c);
						else absBuf = 32767;
					}
					else absBuf = *(wdLeft + c);
					if (maxL < absBuf) maxL = absBuf;
				}

				if (wdStereo)

				{
					for (c = 0; c< wdLength; c++)
					{
						if (*(wdRight+c) < 0)
						{
							if (!( *(wdRight+c) == -32768) )	absBuf = - *(wdRight + c);
							else absBuf = 32767;
						}
						else absBuf = *(wdRight + c);
						if (maxR < absBuf) maxR = absBuf;
					}
				}

				if ( (maxL < maxR) && (wdStereo) ) maxL = maxR;
				
				if (maxL) ratio = (double) 32767 / maxL;
				
				if (ratio != 1)
				{
					for (c=0; c<wdLength; c++)
					{
						buf = *(wdLeft + c);
						*(wdLeft + c) = (short)(buf*ratio);

						if (wdStereo)
						{
							buf = *(wdRight + c);
							*(wdRight + c) = (short)(buf*ratio);
						}
					}
				}

				Invalidate(true);
				_pSong->IsInvalided(false);
			}
		}

		void CWaveEdChildView::OnSelectionRemoveDC() // (Fideloop's)
		{
			double meanL = 0, meanR = 0;
			unsigned long c = 0;
			signed short buf;

			if (wdWave)
			{
				pParent->m_wndView.AddMacViewUndo();

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);

				for (c=0; c<wdLength; c++)
				{
					meanL = meanL + ( (double) *(wdLeft+c) / wdLength);

					if (wdStereo) meanR = (double) meanR + ((double) *(wdRight+c) / wdLength);
				}

				for (c=0; c<wdLength; c++)
				{
					buf = *(wdLeft+c);
					if (meanL > 0)
					{
						if ((double)(buf - meanL) < (-32768))	*(wdLeft+c) = -32768;
						else	*(wdLeft+c) = (short)(buf - meanL);
					}
					else
					{
						if (meanL < 0)
						{
							if ((double)(buf - meanL) > 32767) *(wdLeft+c) = 32767;
						}
						else *(wdLeft + c) = (short)(buf - meanL);
					}

				}
			
				if (wdStereo)
				{
					for (c=0; c<wdLength; c++)
					{
						buf = *(wdRight+c);
						if (meanR > 0)
						{
							if ((double)(buf - meanR) < (-32768))	*(wdRight + c) = -32768;
							else	*(wdRight+c) = (short)(buf - meanR);
						}
						else
						{
							if (meanR < 0)
							{
								if ((double)(buf - meanR) > 32767) *(wdRight+c) = 32767;
							}
							else *(wdRight + c) = (short)(buf - meanR);
						}
					}
				}
				_pSong->IsInvalided(false);
				Invalidate(true);
			}
		}

		void CWaveEdChildView::OnSelectionAmplify()
		{
			short buf = 0;
			double ratio =1;
			unsigned int c =0;
			int pos = 0;

			if (wdWave && blSelection)
			{
				pParent->m_wndView.AddMacViewUndo();

				pos = AmpDialog.DoModal();
				if (pos != AMP_DIALOG_CANCEL)
				{
					_pSong->IsInvalided(true);
					Sleep(LOCK_LATENCY);
					ratio = pow(10.0, (double) pos / (double) 2000.0);
				
					for (c=blStart; c<blStart+blLength; c++)
					{
						buf = *(wdLeft + c);
						if (buf < 0)
						{
							if ( (double) (buf*ratio) < -32768 ) *(wdLeft+c) = -32768;
							else *(wdLeft + c) = (short)(buf*ratio);
						}
						else
						{
							if ( (double) (buf*ratio) > 32767 ) *(wdLeft+c) = 32767;
							else *(wdLeft + c) = (short)(buf*ratio);
						}


						if (wdStereo)
						{
							buf = *(wdRight + c);
							if (buf < 0)
							{	
								if ( (double) (buf*ratio) < -32768 ) *(wdRight+c) = -32768;
								else *(wdRight + c) = (short)(buf*ratio);
							}
							else
							{
								if ( (double) (buf*ratio) > 32767 ) *(wdRight+c) = 32767;
								else *(wdRight + c) = (short)(buf*ratio);
							}
						}
					}
					_pSong->IsInvalided(false);
					Invalidate(true);
				}
			}
		}

		void CWaveEdChildView::OnSelectionReverse() 
		{
			short buf = 0;
			int c, halved = 0;

			if (wdWave && blSelection)
			{
				pParent->m_wndView.AddMacViewUndo();

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);

				halved = (int) floor(blLength/2.0);

				for (c = 0; c < halved; c++)
				{
					buf = *(wdLeft+blStart+blLength - c);
					*(wdLeft+blStart+blLength - c) = *(wdLeft+blStart + c);
					*(wdLeft+blStart + c) = buf;

					if (wdStereo)
					{
						buf = *(wdRight+blStart+blLength - c);
						*(wdRight+blStart+blLength - c) = *(wdRight+blStart + c);
						*(wdRight+blStart + c) = buf;
					}

				}
				Invalidate(true);
				_pSong->IsInvalided(false);
			}
		}

		void CWaveEdChildView::OnSelectionShowall() 
		{
			diStart = 0;
			diLength = wdLength;
			Invalidate(true);
		}

		void CWaveEdChildView::OnConvertMono() 
		{
			if (wdWave && wdStereo)
			{
				pParent->m_wndView.AddMacViewUndo();

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);

		//		SetUndo(4, wdLeft, wdRight, wdLength); 
				for (unsigned int c = 0; c < wdLength; c++)
				{
					*(wdLeft + c) = ( *(wdLeft + c) + *(wdRight + c) ) / 2;
				}

				_pSong->_pInstrument[wsInstrument]->waveStereo = false;
				wdStereo = false;
				zapArray(_pSong->_pInstrument[wsInstrument]->waveDataR);
				Invalidate(true);
				_pSong->IsInvalided(false);
			}
		}

		//Clipboard
		void CWaveEdChildView::OnEditDelete()
		{
			short* pTmp = 0, *pTmpR = 0;
			long datalen = 0;

			if (wdWave && blSelection)
			{
				pParent->m_wndView.AddMacViewUndo();

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);

				datalen = (wdLength - blLength);
				if (datalen)
				{
					pTmp = new signed short[datalen];
					
					if (wdStereo)
					{
						pTmpR= new signed short[datalen];
						CopyMemory(pTmpR, wdRight, blStart*sizeof(short));
						CopyMemory( (pTmpR+blStart), (wdRight + blStart + blLength), (wdLength - blStart - blLength)*sizeof(short) );
						zapArray(_pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
						wdRight = pTmpR;
					}

					CopyMemory( pTmp, wdLeft, blStart*sizeof(short) );
					CopyMemory( (pTmp+blStart), (wdLeft + blStart + blLength), (wdLength - blStart - blLength)*sizeof(short) );
					zapArray(_pSong->_pInstrument[wsInstrument]->waveDataL,pTmp);
					wdLeft = pTmp;
					_pSong->_pInstrument[wsInstrument]->waveLength = datalen;
					wdLength = datalen;
				}
				else
				{
					_pSong->DeleteLayer(wsInstrument);
					wdLength = 0;
					wdWave   = false;
				}
			
				//Validate display
				if ( (diStart + diLength) > wdLength )
				{
					long newlen = wdLength - diLength;

					if ( newlen < 0 )
						this->OnSelectionShowall();
					else
						diStart = (unsigned)newlen;
				}
				
				blSelection = false;
				blLength  = 0;
				blStart   = 0;
				pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.

				Invalidate(true);
				_pSong->IsInvalided(false);
			}
		}

		//Menu update handlers.

		void CWaveEdChildView::OnUpdateSelectionAmplify(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection && blLength > 1);
		}

		void CWaveEdChildView::OnUpdateSelectionReverse(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection && blLength > 1);
			
		}

		void CWaveEdChildView::OnUpdateSelectionFadein(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection && blLength > 1);
			
		}

		void CWaveEdChildView::OnUpdateSelectionFadeout(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection && blLength > 1);	
		}

		void CWaveEdChildView::OnUpdateSelectionNormalize(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave);	
		}

		void CWaveEdChildView::OnUpdateSelectionRemovedc(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave);	
		}

		void CWaveEdChildView::OnUpdateSeleccionZoom(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection);
		}

		void CWaveEdChildView::OnUpdateSeleccionZoomout(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && (diLength < wdLength) );
		}

		void CWaveEdChildView::OnUpdateSelectionShowall(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && (diLength < wdLength) );		
		}

		void CWaveEdChildView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection && blLength > 1);
		}

		void CWaveEdChildView::OnUpdateEditCut(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection && blLength > 1);
		}

		void CWaveEdChildView::OnUpdateEditCrop(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection && blLength > 1);
		}

		void CWaveEdChildView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable( ((wdWave && blSelection && blLength == 1) || !wdWave) && IsClipboardFormatAvailable(CF_WAVE));
		}

		void CWaveEdChildView::OnUpdateEditDelete(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection && blLength > 1);	
		}

		void CWaveEdChildView::OnUpdateConvertMono(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && wdStereo);
		}

		/* Clipboard functions */

		void CWaveEdChildView::OnEditCopy() 
		{
			unsigned long c = 0;
				
			struct fullheader
			{
				DWORD	head;
				DWORD	size;
				DWORD	head2;
				DWORD	fmthead;
				DWORD	fmtsize;
				WAVEFORMATEX	fmtcontent;
				DWORD datahead;
				DWORD datasize;
			} wavheader;

			OpenClipboard();
			EmptyClipboard();
			hClipboardData = GlobalAlloc(GMEM_MOVEABLE, ( wdStereo ? blLength*4 + sizeof(fullheader) : blLength*2 + sizeof(fullheader)));
			
			wavheader.head = 'FFIR';
			wavheader.size = wdStereo ? (blLength*4 + sizeof(fullheader) - 8) : (blLength*2 + sizeof(fullheader) - 8);
			wavheader.head2= 'EVAW';
			wavheader.fmthead = ' tmf';
			wavheader.fmtsize = sizeof(WAVEFORMATEX) + 2; // !!!!!!!!!!!!!!!!????????? - works...
			wavheader.fmtcontent.wFormatTag = WAVE_FORMAT_PCM;
			wavheader.fmtcontent.nChannels = wdStereo ? 2 : 1;
			wavheader.fmtcontent.nSamplesPerSec = 44100;
			wavheader.fmtcontent.wBitsPerSample = 16;
			wavheader.fmtcontent.nAvgBytesPerSec = wavheader.fmtcontent.wBitsPerSample/8*wavheader.fmtcontent.nChannels*wavheader.fmtcontent.nSamplesPerSec;
			wavheader.fmtcontent.nBlockAlign = wdStereo ? 4 : 2 ;
			wavheader.fmtcontent.cbSize = 0;
			wavheader.datahead = 'atad';
			wavheader.datasize = wdStereo ? blLength*4 : blLength*2;

			pClipboardData = (char*) GlobalLock(hClipboardData);
			
			CopyMemory(pClipboardData, &wavheader, sizeof(fullheader) );
			if (wdStereo)
			{
				pClipboardData += sizeof(fullheader);
				for (c = 0; c < blLength*2; c += 2)
				{
					*((signed short*)pClipboardData + c) = *(wdLeft + blStart + (long)(c*0.5));
					*((signed short*)pClipboardData + c + 1) = *(wdRight + blStart + (long)(c*0.5));
				}
			}
			else
			{
				CopyMemory(pClipboardData + sizeof(fullheader), (wdLeft + blStart), blLength*2);
			}

			GlobalUnlock(hClipboardData);
			SetClipboardData(CF_WAVE, hClipboardData);
			CloseClipboard();
			Invalidate(true);
		}

		void CWaveEdChildView::OnEditCut() 
		{
			OnEditCopy();
			OnEditDelete();
		}

		void CWaveEdChildView::OnEditCrop()
		{
			unsigned long blStartTemp = blStart;
			
			blStart += blLength;
			blLength = (wdLength - blStart);
			OnEditDelete();
			
			blSelection = true;
			blStart = 0;
			blLength = blStartTemp;
			OnEditDelete();
		}

		void CWaveEdChildView::OnEditPaste() 
		{
			unsigned long c = 0;

			pParent->m_wndView.AddMacViewUndo();

			char *pData;
			DWORD lFmt, lData;
			
			WAVEFORMATEX* pFmt;
			short* pPasteData;
			short* pTmp = 0, *pTmpR = 0;

			OpenClipboard();
			hPasteData = GetClipboardData(CF_WAVE);
			pPasteData = (short*)GlobalLock(hPasteData);

			if ((*(DWORD*)pPasteData != 'FFIR') && (*((DWORD*)pPasteData + 2)!='EVAW')) return;
			lFmt= *(DWORD*)((char*)pPasteData + 16);
			pFmt = (WAVEFORMATEX*)((char*)pPasteData + 20); //'RIFF' + len. +'WAVE' + 'fmt ' + len. = 20 bytes.

			lData = *(DWORD*)((char*)pPasteData + 20 + lFmt + 4);
			pData = (char*)pPasteData + 20 + lFmt + 8;

			_pSong->IsInvalided(true);
			Sleep(LOCK_LATENCY);

			if (!wdWave)
			{
				if (pFmt->wBitsPerSample == 16)
				{
					_pSong->WavAlloc(wsInstrument, (pFmt->nChannels==2) ? true : false, (pFmt->nChannels==2) ? (DWORD)(lData*0.25) : (DWORD)(lData*0.5), "Clipboard");
					if (pFmt->nChannels == 1)
					{
						memcpy(_pSong->_pInstrument[wsInstrument]->waveDataL, pData, lData);
						wdLength = (DWORD)(lData*0.5);
						wdLeft  = _pSong->_pInstrument[wsInstrument]->waveDataL;
						wdStereo = false;
					}
					else if (pFmt->nChannels == 2)
					{
						for (c = 0; c < lData*0.5; c += 2)
						{
							*(_pSong->_pInstrument[wsInstrument]->waveDataL + (long)(c*0.5)) = *((signed short*)pData + c);
							*(_pSong->_pInstrument[wsInstrument]->waveDataR + (long)(c*0.5)) = *((signed short*)pData + c + 1);
						}
						wdLength = (DWORD)(lData *0.25);
						wdLeft = _pSong->_pInstrument[wsInstrument]->waveDataL;
						wdRight = _pSong->_pInstrument[wsInstrument]->waveDataR;
						wdStereo = true;
					}
					wdWave = true;
				}
			}
			else
			{
				if (pFmt->wBitsPerSample == 16)
				{
					if ( (pFmt->nChannels == 1) && (wdStereo == false) )
					{ 
						pTmp = new signed short[(DWORD)(lData*0.5) + wdLength];
						memcpy(pTmp, wdLeft, blStart*2);
						memcpy(pTmp + blStart, pData, lData);
						memcpy((BYTE*)pTmp + blStart*2 + lData, wdLeft + blStart, 2*(wdLength - blStart));

						zapArray(_pSong->_pInstrument[wsInstrument]->waveDataL,pTmp);
						_pSong->_pInstrument[wsInstrument]->waveLength = wdLength + (DWORD)(lData*0.5);
						wdLength = wdLength + (DWORD)(lData*0.5);
					}
					else if ( (pFmt->nChannels == 2) && (wdStereo == true) )
					{
						unsigned long c;
						pTmp = new signed short[(DWORD)(lData*0.25) + wdLength];
						pTmpR= new signed short[(DWORD)(lData*0.25) + wdLength];
						memcpy(pTmp, wdLeft, blStart*2);
						memcpy(pTmpR,wdRight,blStart*2);
						for (c = 0; c < (unsigned long)(lData*0.25); c++)
						{
							*(pTmp + blStart + c) = *(short*)(pData + c*4);
							*(pTmpR+ blStart + c) = *(short*)(pData + c*4 + 2);
						}
						memcpy((BYTE*)pTmp + blStart*2 + (unsigned long)(lData*0.5), wdLeft + blStart, 2*(wdLength - blStart));
						memcpy((BYTE*)pTmpR+ blStart*2 + (unsigned long)(lData*0.5), wdRight + blStart, 2*(wdLength - blStart));

						wdLeft = zapArray(_pSong->_pInstrument[wsInstrument]->waveDataL, pTmp);
						wdRight = zapArray(_pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
						_pSong->_pInstrument[wsInstrument]->waveLength = wdLength + (DWORD)(lData*0.25);
						wdLength = wdLength + (DWORD)(lData*0.25);
					}
				}
			}

			GlobalUnlock(hPasteData);
			CloseClipboard();
			OnSelectionShowall();

			pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.
			Invalidate(true);
			_pSong->IsInvalided(false);
		}

		void CWaveEdChildView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(false);
			
		}

		void CWaveEdChildView::OnEditSelectAll() 
		{
			diStart = 0;
			blStart = 0;
			diLength = wdLength;
			blLength = wdLength;
			blSelection = true;
			Invalidate(true);
		}

		void CWaveEdChildView::OnUpdateEditSelectAll(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave);
		}

		void CWaveEdChildView::OnDestroyClipboard() 
		{
			CWnd::OnDestroyClipboard();
			GlobalFree(hClipboardData);
		}

		void CWaveEdChildView::SetSong(Song* _sng)
		{
			_pSong = _sng;
		}
		void CWaveEdChildView::SetParent(CMainFrame* parent)
		{
			pParent = parent;
		}	
		unsigned long CWaveEdChildView::GetSelectionLength()
		{
			if(wdWave)
				return blLength;
			else
				return 0;
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
