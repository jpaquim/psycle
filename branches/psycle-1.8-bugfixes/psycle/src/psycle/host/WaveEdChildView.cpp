///\file
///\brief implementation file for psycle::host::CWaveEdChildView.
#include <project.private.hpp>
#include "psycle.hpp"
#include "Helpers.hpp"
#include "WaveEdChildView.hpp"
#include "MainFrm.hpp"
#include <mmreg.h>
#include <math.h>
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)

	
		float const CWaveEdChildView::zoomBase = 1.06f;

		CWaveEdChildView::CWaveEdChildView()
		{
		}

		CWaveEdChildView::~CWaveEdChildView()
		{
		}


		BEGIN_MESSAGE_MAP(CWaveEdChildView, CWnd)
			//{{AFX_MSG_MAP(CWaveEdChildView)
			ON_WM_PAINT()
			ON_WM_RBUTTONDOWN()
			ON_WM_LBUTTONDOWN()
			ON_WM_LBUTTONDBLCLK()
			ON_WM_MOUSEMOVE()
			ON_WM_LBUTTONUP()
			ON_COMMAND(ID_SELECCION_ZOOMIN, OnSelectionZoomIn)
			ON_COMMAND(ID_SELECCION_ZOOMOUT, OnSelectionZoomOut)
			ON_COMMAND(ID_SELECTION_ZOOMSEL, OnSelectionZoomSel)
			ON_COMMAND(ID_POPUP_ZOOMSEL, OnSelectionZoomSel)
			ON_COMMAND(ID_SELECTION_FADEIN, OnSelectionFadeIn)
			ON_COMMAND(ID_SELECTION_FADEOUT, OnSelectionFadeOut)
			ON_COMMAND(ID_SELECTION_NORMALIZE, OnSelectionNormalize)
			ON_COMMAND(ID_SELECTION_REMOVEDC, OnSelectionRemoveDC)
			ON_COMMAND(ID_SELECTION_AMPLIFY, OnSelectionAmplify)
			ON_COMMAND(ID_SELECTION_REVERSE, OnSelectionReverse)
			ON_COMMAND(ID_SELECTION_SHOWALL, OnSelectionShowall)
			ON_COMMAND(ID_SELECTION_INSERTSILENCE, OnSelectionInsertSilence)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_AMPLIFY, OnUpdateSelectionAmplify)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_REVERSE, OnUpdateSelectionReverse)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_FADEIN, OnUpdateSelectionFadein)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_FADEOUT, OnUpdateSelectionFadeout)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_NORMALIZE, OnUpdateSelectionNormalize)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_REMOVEDC, OnUpdateSelectionRemovedc)
			ON_UPDATE_COMMAND_UI(ID_SELECCION_ZOOMIN, OnUpdateSeleccionZoomIn)
			ON_UPDATE_COMMAND_UI(ID_POPUP_ZOOMIN, OnUpdateSeleccionZoomIn)
			ON_UPDATE_COMMAND_UI(ID_SELECCION_ZOOMOUT, OnUpdateSeleccionZoomOut)
			ON_UPDATE_COMMAND_UI(ID_POPUP_ZOOMOUT, OnUpdateSeleccionZoomOut)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_SHOWALL, OnUpdateSelectionShowall)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_ZOOMSEL, OnUpdateSelectionZoomSel)
			ON_UPDATE_COMMAND_UI(ID_POPUP_ZOOMSEL, OnUpdateSelectionZoomSel)
			ON_UPDATE_COMMAND_UI(ID_SELECTION_INSERTSILENCE, OnUpdateSelectionInsertSilence)
			ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
			ON_COMMAND(ID_POPUP_COPY, OnEditCopy)
			ON_COMMAND(ID_EDIT_CUT, OnEditCut)
			ON_COMMAND(ID_POPUP_CUT, OnEditCut)
			ON_COMMAND(ID_EDIT_CROP, OnEditCrop)
			ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
			ON_COMMAND(ID_POPUP_PASTE, OnEditPaste)
			ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
			ON_COMMAND(ID_POPUP_DELETE, OnEditDelete)
			ON_COMMAND(ID_CONVERT_MONO, OnConvertMono)
			ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
			ON_COMMAND(ID_EDIT_SNAPTOZERO, OnEditSnapToZero)
			ON_COMMAND(ID_PASTE_OVERWRITE, OnPasteOverwrite)
			ON_COMMAND(ID_PASTE_MIX, OnPasteMix)
			ON_COMMAND(ID_PASTE_CROSSFADE, OnPasteCrossfade)
			ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
			ON_UPDATE_COMMAND_UI(ID_POPUP_COPY, OnUpdateEditCopy)
			ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
			ON_UPDATE_COMMAND_UI(ID_POPUP_CUT, OnUpdateEditCut)
			ON_UPDATE_COMMAND_UI(ID_EDIT_CROP, OnUpdateEditCrop)
			ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
			ON_UPDATE_COMMAND_UI(ID_POPUP_PASTE, OnUpdateEditPaste)
			ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
			ON_UPDATE_COMMAND_UI(ID_POPUP_DELETE, OnUpdateEditDelete)
			ON_UPDATE_COMMAND_UI(ID_CONVERT_MONO, OnUpdateConvertMono)
			ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
			ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
			ON_UPDATE_COMMAND_UI(ID_EDIT_SNAPTOZERO, OnUpdateEditSnapToZero)
			ON_UPDATE_COMMAND_UI(ID_PASTE_OVERWRITE, OnUpdatePasteOverwrite)
			ON_UPDATE_COMMAND_UI(ID_PASTE_CROSSFADE, OnUpdatePasteCrossfade)
			ON_UPDATE_COMMAND_UI(ID_PASTE_MIX, OnUpdatePasteMix)
			ON_UPDATE_COMMAND_UI(ID_POPUP_SETLOOPSTART, OnUpdateSetLoopStart)
			ON_UPDATE_COMMAND_UI(ID_POPUP_SETLOOPEND, OnUpdateSetLoopEnd)
			ON_WM_DESTROYCLIPBOARD()
			//}}AFX_MSG_MAP
			ON_COMMAND(ID_POPUP_SETLOOPSTART, OnPopupSetLoopStart)
			ON_COMMAND(ID_POPUP_SETLOOPEND, OnPopupSetLoopEnd)
			ON_COMMAND(ID_POPUP_SELECTIONTOLOOP, OnPopupSelectionToLoop)
			ON_COMMAND(ID_POPUP_ZOOMIN, OnPopupZoomIn)
			ON_COMMAND(ID_POPUP_ZOOMOUT, OnPopupZoomOut)

			ON_WM_CREATE()
			ON_WM_DESTROY()
			ON_WM_HSCROLL()
			ON_WM_SIZE()
			ON_BN_CLICKED(IDC_ZOOMIN, OnSelectionZoomIn)
			ON_BN_CLICKED(IDC_ZOOMOUT, OnSelectionZoomOut)
			ON_WM_TIMER()
			ON_WM_CONTEXTMENU()
			ON_NOTIFY(NM_CUSTOMDRAW, IDC_VOLSLIDE, OnCustomdrawVolSlider)

		END_MESSAGE_MAP()

		/////////////////////////////////////////////////////////////////////////////
		// CWaveEdChildView drawing

		void CWaveEdChildView::OnPaint(void)
		{
			CPaintDC dc(this);
			CPaintDC *pDC = &dc;

			int wrHeight = 0;
			int wrHeadHeight=0;
			__int32 c;

			int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
				
			if(wdWave)
			{
				CRect invalidRect;
				pDC->GetClipBox(&invalidRect);
				CRect rect;
				GetClientRect(&rect);

				CBitmap* bmpBuffer = new CBitmap;
				CBitmap* oldbmp;
				bmpBuffer->CreateCompatibleBitmap(pDC, invalidRect.Width(), invalidRect.Height());
				CDC memDC;
				
				memDC.CreateCompatibleDC(pDC);
				oldbmp = memDC.SelectObject(bmpBuffer);
				memDC.SetWindowOrg(invalidRect.left, invalidRect.top);
				
				int const nHeadHeight=f2i(rect.Height()*0.1f);
				int const nWidth=rect.Width();
				int const nHeight=rect.Height()-cyHScroll-nHeadHeight;

				int const my =		f2i(nHeight*0.5f);
				int const myHead =	f2i(nHeadHeight*0.5f);

				if(wdStereo)
				{
					wrHeight =		f2i(my*0.5f);
					wrHeadHeight =	f2i(myHead*0.5f);
				}
				else 
				{
					wrHeight=my;
					wrHeadHeight=myHead;
				}

				//ratios used to convert from sample indices to pixels
				float dispRatio = nWidth/(float)diLength;
				float headDispRatio = nWidth/(float)wdLength;

				// Draw preliminary stuff
				CPen *oldpen= memDC.SelectObject(&cpen_me);
				
				// Left channel 0 amplitude line
				memDC.MoveTo(0,wrHeight+nHeadHeight);
				memDC.LineTo(nWidth,wrHeight+nHeadHeight);
				// Left Header 0 amplitude line
				memDC.MoveTo(0,wrHeadHeight);
				memDC.LineTo(nWidth,wrHeadHeight);
				
				int const wrHeight_R = my + wrHeight;
				int const wrHeadHeight_R = myHead + wrHeadHeight;
				
				memDC.SelectObject(&cpen_white);
				// Header/Body divider
				memDC.MoveTo(0, nHeadHeight);
				memDC.LineTo(nWidth, nHeadHeight);
				if(wdStereo)
				{
					// Stereo channels separator line
					memDC.SelectObject(&cpen_lo);
					memDC.MoveTo(0,my+nHeadHeight);
					memDC.LineTo(nWidth,my+nHeadHeight);
					// Stereo channels Header Separator
					memDC.MoveTo(0, myHead);
					memDC.LineTo(nWidth,myHead);
					
					// Right channel 0 amplitude line
					memDC.SelectObject(&cpen_me);
					memDC.MoveTo(0,wrHeight_R+nHeadHeight);
					memDC.LineTo(nWidth,wrHeight_R+nHeadHeight);
					// Right Header 0 amplitude line
					memDC.MoveTo(0,wrHeadHeight_R);
					memDC.LineTo(nWidth, wrHeadHeight_R);

				}
				
				// Draw samples in channels (Fideloop's)

				memDC.SelectObject(&cpen_hi);

				for(c = 0; c < nWidth; c++)
				{
					memDC.MoveTo(c,wrHeight - lDisplay.at(c).first + nHeadHeight);
					memDC.LineTo(c,wrHeight - lDisplay.at(c).second + nHeadHeight);
					memDC.MoveTo(c, wrHeadHeight-lHeadDisplay.at(c).first);
					memDC.LineTo(c, wrHeadHeight-lHeadDisplay.at(c).second );
				}

				if(wdStereo)
				{
					//draw right channel wave data
					for(c = 0; c < nWidth; c++)
					{
						memDC.MoveTo(c,wrHeight_R - rDisplay.at(c).first + nHeadHeight);
						memDC.LineTo(c,wrHeight_R - rDisplay.at(c).second + nHeadHeight);
						memDC.MoveTo(c, wrHeadHeight_R-rHeadDisplay.at(c).first);
						memDC.LineTo(c, wrHeadHeight_R-rHeadDisplay.at(c).second );
					}
				}

				//draw loop points
				if ( wdLoop )
				{
					memDC.SelectObject(&cpen_lo);
					if ( wdLoopS >= diStart && wdLoopS < diStart+diLength)
					{
						int ls = f2i((wdLoopS-diStart)*dispRatio);
						memDC.MoveTo(ls,nHeadHeight);
						memDC.LineTo(ls,nHeight+nHeadHeight);
						CSize textsize = memDC.GetTextExtent("Start");
						memDC.TextOut( ls - textsize.cx/2, nHeadHeight, "Start" );
					}
					if ( wdLoopE >= diStart && wdLoopE < diStart+diLength)
					{
						int le = f2i((wdLoopE-diStart)*dispRatio);
						memDC.MoveTo(le,nHeadHeight);
						memDC.LineTo(le,nHeight+nHeadHeight);
						CSize textsize = memDC.GetTextExtent("End");
						memDC.TextOut( le - textsize.cx/2, nHeight+nHeadHeight-textsize.cy, "End" );
					}

					//draw loop points in header
					int ls = f2i(wdLoopS * headDispRatio);
					int le = f2i(wdLoopE * headDispRatio);

					memDC.MoveTo(ls, 0);
					memDC.LineTo(ls, nHeadHeight);

					memDC.MoveTo(le, 0);
					memDC.LineTo(le, nHeadHeight);
				}

				//draw screen size on header
				memDC.SelectObject(&cpen_white);
				int screenx  = f2i( diStart           * headDispRatio);
				int screenx2 = f2i((diStart+diLength) * headDispRatio);
				memDC.MoveTo(screenx, 0);
				memDC.LineTo(screenx, nHeadHeight-1);
				memDC.LineTo(screenx2,nHeadHeight-1);
				memDC.LineTo(screenx2,0);
				memDC.LineTo(screenx, 0);

				memDC.SelectObject(oldpen);

				memDC.SetROP2(R2_NOT);

				if(cursorBlink	&&	cursorPos >= diStart	&&	cursorPos <= diStart+diLength)
				{
					int cursorX = f2i((cursorPos-diStart)*dispRatio);
					memDC.MoveTo(cursorX, nHeadHeight);
					memDC.LineTo(cursorX, nHeadHeight+nHeight);
				}
				unsigned long selx, selx2;
				selx =blStart;
				selx2=blStart+blLength;

				int HeadSelX = f2i(selx * headDispRatio);
				int HeadSelX2= f2i(selx2* headDispRatio);
				memDC.Rectangle(HeadSelX,0,		HeadSelX2,nHeadHeight);

				if(selx<diStart) selx=diStart;
				if(selx2>diStart+diLength) selx2=diStart+diLength;
				//if the selected block is entirely off the screen, the above statements will flip the order
				if(selx<selx2)					//if not, it will just clip the drawing
				{
					selx = f2i((selx -diStart)*dispRatio) ;
					selx2= f2i((selx2-diStart)*dispRatio) ;
					memDC.Rectangle(selx,nHeadHeight,selx2,nHeight+nHeadHeight);
				}

				pDC->BitBlt(invalidRect.left, invalidRect.top, invalidRect.Width(), invalidRect.Height(),
							&memDC, invalidRect.left, invalidRect.top, SRCCOPY);
				memDC.SelectObject(oldbmp);
				memDC.DeleteDC();
				bmpBuffer->DeleteObject();
				delete bmpBuffer;

			}
			else
			{
				pDC->TextOut(4,4,"No Wave Data");
			}
			
			// Do not call CWnd::OnPaint() for painting messages
		}




		/////////////////////////////////////////////////////////////////////////////
		// CWaveEdChildView message handlers


		BOOL CWaveEdChildView::PreCreateWindow(CREATESTRUCT& cs) 
		{
			if (!CWnd::PreCreateWindow(cs)) return false;

			cs.dwExStyle |= WS_EX_CLIENTEDGE;
			cs.style &= ~WS_BORDER;
			cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
			::LoadCursor(NULL, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH), NULL);

			return true;
			
		//	return CWnd::PreCreateWindow(cs);
		}

		int CWaveEdChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
		{
			if (CWnd::OnCreate(lpCreateStruct) == -1)
				return -1;

			cpen_lo.CreatePen(PS_SOLID,0,0xFF0000);
			cpen_me.CreatePen(PS_SOLID,0,0xCCCCCC);
			cpen_hi.CreatePen(PS_SOLID,0,0x00DD77);
			cpen_white.CreatePen(PS_SOLID,0,0xEEEEEE);

			zoombar.Create(this, IDD_WAVED_ZOOMBAR, CBRS_BOTTOM | WS_CHILD, AFX_IDW_DIALOGBAR);

			hResizeLR = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
			hIBeam = AfxGetApp()->LoadStandardCursor(IDC_IBEAM);

			bSnapToZero=false;
			bDragLoopStart = bDragLoopEnd = false;
			SelStart=0;
			cursorPos=0;
			_pSong->waved.SetVolume(0.4f);
			wdWave=false;
			wsInstrument=-1;
			prevHeadLoopS = prevBodyLoopS = prevHeadLoopE = prevBodyLoopE = 0;
			prevBodyX = prevHeadX = 0;

			SetTimer(31415, 750, 0);

			return 0;
		}

		void CWaveEdChildView::OnDestroy()
		{
			CWnd::OnDestroy();

			cpen_lo.DeleteObject();
			cpen_me.DeleteObject();
			cpen_hi.DeleteObject();
			cpen_white.DeleteObject();
			zoombar.DestroyWindow();
		}


		//todo: use some sort of multiplier to prevent scrolling from being limited for waves with more samples than can be expressed
		//		in a signed 32-bit int.
		void CWaveEdChildView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
		{
			CRect client;
			GetClientRect(&client);
			int delta;
			int nWidth;

			CScrollBar* hScroll = (CScrollBar*) zoombar.GetDlgItem(IDC_HSCROLL);
			CSliderCtrl* zoomSlider = (CSliderCtrl*) zoombar.GetDlgItem(IDC_ZOOMSLIDE);
			CSliderCtrl* volSlider = (CSliderCtrl*) zoombar.GetDlgItem(IDC_VOLSLIDE);
			
			if(pScrollBar == hScroll)
			{
				switch (nSBCode)
				{
				case SB_LEFT:
					diStart=0;
					break;
				case SB_RIGHT:
					diStart=wdLength-1;
					break;
				case SB_ENDSCROLL:
					break;
				case SB_THUMBTRACK:
				case SB_THUMBPOSITION:
					SCROLLINFO si;
					hScroll->GetScrollInfo(&si, SIF_TRACKPOS|SIF_POS);	//this is necessary because the nPos arg to OnHScroll is
					diStart = si.nTrackPos;								//transparently restricted to 16 bits
					break;
				case SB_LINELEFT:
					nWidth=client.Width();					// n samps / 20 pix  =  diLength samps / nWidth pix
					delta = 20 * diLength/nWidth;			//	n = 20 * diLength/nWidth
					if(delta<1) delta=1;			//in case we're zoomed in reeeally far
					diStart -= delta;
					break;
				case SB_LINERIGHT:
					nWidth=client.Width();
					delta = 20 * diLength/nWidth;
					if(delta<1) delta=1;
					diStart += delta;
					break;
				case SB_PAGELEFT:
					diStart -= diLength;
					break;
				case SB_PAGERIGHT:
					diStart+= diLength;
					break;
				}

				hScroll->SetScrollPos(diStart);
				diStart = hScroll->GetScrollPos();		//petzold's let-windows-do-the-boundary-checking method

				RefreshDisplayData();
				client.bottom -= GetSystemMetrics(SM_CYHSCROLL);
				InvalidateRect(&client, false);
			}
			else if((CSliderCtrl*)pScrollBar == zoomSlider)
			{
				int newzoom = zoomSlider->GetPos();
				SetSpecificZoom(newzoom);
				this->SetFocus();
			}
			else if((CSliderCtrl*)pScrollBar == volSlider)
			{
				_pSong->waved.SetVolume( volSlider->GetPos()/100.0f );
				volSlider->Invalidate(false);
				this->SetFocus();
			}
			else
				throw;
			
			CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
		}

		void CWaveEdChildView::OnSize(UINT nType, int cx, int cy)
		{
			CWnd::OnSize(nType, cx, cy);

			int cyZoombar = GetSystemMetrics(SM_CYHSCROLL);

			int clientcy = cy - cyZoombar;

			RefreshDisplayData(true);

			zoombar.MoveWindow(0, clientcy, cx, cyZoombar);

			CButton* cb;
			cb=(CButton *)zoombar.GetDlgItem(IDC_ZOOMIN);
			cb->SetWindowPos(NULL, cx-20, 0, 15, cyZoombar, SWP_NOZORDER);
			cb=(CButton *)zoombar.GetDlgItem(IDC_ZOOMOUT);
			cb->SetWindowPos(NULL, cx-115, 0, 15, cyZoombar, SWP_NOZORDER);

			CSliderCtrl* cs;
			cs = (CSliderCtrl*)zoombar.GetDlgItem(IDC_ZOOMSLIDE);
			cs->SetWindowPos(NULL, cx-100, 0, 80, cyZoombar, SWP_NOZORDER);

			cs = (CSliderCtrl*)zoombar.GetDlgItem(IDC_VOLSLIDE);
			cs->SetWindowPos(NULL, 0, 0, 75, cyZoombar, SWP_NOZORDER);

			CScrollBar *csb;
			csb = (CScrollBar*)zoombar.GetDlgItem(IDC_HSCROLL);
			
			csb->SetWindowPos(NULL, 75, 0, cx-190, cyZoombar, SWP_NOZORDER);
			
		}


		void CWaveEdChildView::OnTimer(UINT nIDEvent)
		{
			if(nIDEvent==31415)
			{
				cursorBlink = !cursorBlink;
				CRect rect;
				GetClientRect(&rect);
				rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
				InvalidateRect(&rect, false);
			}
			CWnd::OnTimer(nIDEvent);
		}

		void CWaveEdChildView::OnContextMenu(CWnd* pWnd, CPoint point)
		{
			CMenu menu;
			menu.LoadMenu(IDR_WAVED_POPUP);
			CMenu *popup;
			popup=menu.GetSubMenu(0);
			assert(popup);

			//i would very much like to know why this nonsense is necessary.. i've been told that OnUpdateCommandUI messages
			//don't work for non-CFrameWnd-derived windows, but ChildView.cpp seems to do alright..  if another solution is
			//not found, it might be best to just move the context menu to CWaveEdFrame.
			popup->EnableMenuItem(ID_POPUP_COPY, (wdWave&&blSelection? MF_ENABLED: MF_GRAYED));
			popup->EnableMenuItem(ID_POPUP_CUT, (wdWave&&blSelection? MF_ENABLED: MF_GRAYED));
			popup->EnableMenuItem(ID_POPUP_PASTE, (IsClipboardFormatAvailable(CF_WAVE)? MF_ENABLED: MF_GRAYED));
			popup->EnableMenuItem(ID_POPUP_ZOOMOUT, (wdWave && diLength<wdLength? MF_ENABLED: MF_GRAYED));
			popup->EnableMenuItem(ID_POPUP_ZOOMIN, (wdWave && diLength>8? MF_ENABLED: MF_GRAYED));
			popup->EnableMenuItem(ID_POPUP_ZOOMSEL, (wdWave && blSelection? MF_ENABLED: MF_GRAYED));
			popup->EnableMenuItem(ID_POPUP_DELETE, (wdWave && blSelection? MF_ENABLED: MF_GRAYED));
			popup->EnableMenuItem(ID_POPUP_SETLOOPSTART, (wdWave? MF_ENABLED: MF_GRAYED));
			popup->EnableMenuItem(ID_POPUP_SETLOOPEND, (wdWave? MF_ENABLED: MF_GRAYED));
			popup->EnableMenuItem(ID_POPUP_SELECTIONTOLOOP, (wdWave && blSelection? MF_ENABLED: MF_GRAYED));
			popup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, point.x, point.y, this);

			popup->DestroyMenu();	//i'm not sure this is necessary here..
			CRect rect;
			GetWindowRect(&rect);
			rbX = point.x-rect.left;
			rbY = point.y-rect.top;
		}

		
		void CWaveEdChildView::OnCustomdrawVolSlider(NMHDR* pNMHDR, LRESULT* pResult)
		{
			NMCUSTOMDRAW nmcd = *(LPNMCUSTOMDRAW)pNMHDR;

			if ( nmcd.dwDrawStage == CDDS_PREPAINT )
			{
				float vol = _pSong->waved.GetVolume();
				CDC* pDC = CDC::FromHandle( nmcd.hdc );
				CDC memDC;
				memDC.CreateCompatibleDC(pDC);
				CRect rc;
				zoombar.GetWindowRect(&rc);
				rc.bottom -=rc.top;
				rc.top=0;
				rc.left=0;
				rc.right=74;

				CBitmap *bmpBuf = new CBitmap;
				CBitmap *oldBmp;
				bmpBuf->CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());
				oldBmp = memDC.SelectObject(bmpBuf);

				CBrush blueBrush;
				CBrush *oldBrush;
				blueBrush.CreateSolidBrush( RGB(0,128,200) );

				oldBrush = (CBrush*)memDC.SelectStockObject(BLACK_BRUSH);
				memDC.Rectangle(&rc);

				memDC.SelectStockObject(DKGRAY_BRUSH);
				memDC.Rectangle(rc.left+7, rc.top+2, rc.right-7, rc.bottom-2);

				memDC.SelectObject( &blueBrush );
				memDC.Rectangle(	rc.left+7,
								rc.top+2,
								rc.left+7+f2i( vol*(rc.right-rc.left-14) ),
								rc.bottom-2);
				
				memDC.SelectStockObject(BLACK_BRUSH);
				CPoint points[3];
				points[0].x = rc.left+5;			points[0].y = rc.bottom-6;
				points[1].x = rc.right-2;		points[1].y = rc.top+1;
				points[2].x = rc.left+5;			points[2].y = rc.top+1;
				memDC.Polygon(points, 3);

				pDC->BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), &memDC, rc.left, rc.top, SRCCOPY);

				pDC->Detach();
				memDC.SelectObject(oldBrush);
				memDC.SelectObject(oldBmp);
				memDC.DeleteDC();
				bmpBuf->DeleteObject();
				delete bmpBuf;
				*pResult = CDRF_SKIPDEFAULT;
			}
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

				ResetScrollBars(true);
				RefreshDisplayData(true);
				Invalidate();
			}
			else
			{
				wdWave=false;

				ResetScrollBars(true);
				RefreshDisplayData(true);
				SetWindowText("Wave Editor [No Data]");
				Invalidate(true);
			}

			blSelection=false;
		}



		//////////////////////////////////////////////////////////////////////////
		//////		Zoom Functions



		void CWaveEdChildView::OnSelectionZoomIn()
		{
			if(wdWave && wdLength>8)
			{
				if(diLength>=12)
					diLength = diLength*2/3;
				else
					diLength=8;

				if(cursorPos<diLength/2)
					diStart=0;
				else if(cursorPos+diLength/2>wdLength)
					diStart=wdLength-diLength;
				else
					diStart=cursorPos-diLength/2;
			}

			ResetScrollBars();
			RefreshDisplayData();
			Invalidate(false);

		}

		void CWaveEdChildView::OnPopupZoomIn()
		{
			CRect rect;
			GetClientRect(&rect);
			int nWidth = rect.Width();
			unsigned long newCenter = diStart + rbX*diLength/nWidth;
			if(wdWave && wdLength>8)
			{
				if(diLength>=12)
					diLength = diLength*2/3;
				else diLength=8;

				if(newCenter<diLength/2)
					diStart=0;
				else if(newCenter+diLength/2>wdLength)
					diStart=wdLength-diLength;
				else
					diStart=newCenter-diLength/2;
			}

			ResetScrollBars();
			RefreshDisplayData();
			Invalidate(false);
		}

		void CWaveEdChildView::OnSelectionZoomOut()
		{
			if(wdWave && diLength<wdLength)
			{
				diLength=diLength*3/2;

				if(diLength>wdLength) diLength=wdLength;

				if(cursorPos<diLength/2)	//cursorPos is unsigned, so we can't just set it and check if it's < 0
					diStart=0;
				else if(cursorPos+diLength/2>wdLength)
					diStart=wdLength-diLength;
				else
					diStart = cursorPos-diLength/2;

				ResetScrollBars();
				RefreshDisplayData();
				Invalidate(false);
			}
		}

		void CWaveEdChildView::OnPopupZoomOut()
		{
			CRect rect;
			GetClientRect(&rect);
			int nWidth = rect.Width();
			unsigned long newCenter = diStart + rbX*diLength/nWidth;
			if(wdWave && diLength<wdLength)
			{
				diLength=diLength*3/2;

				if(diLength>wdLength) diLength=wdLength;

				if(newCenter<diLength/2)
					diStart=0;
				else if(newCenter+diLength/2>wdLength)
					diStart=wdLength-diLength;
				else
					diStart=newCenter-diLength/2;
			}

			ResetScrollBars();
			RefreshDisplayData();
			Invalidate(false);
		}



		void CWaveEdChildView::OnSelectionZoomSel()
		{
			if(blSelection && wdWave)
			{
				diStart = blStart;
				diLength = blLength;
				if(diLength<8)
				{
					unsigned long diCenter = diStart+diLength/2;
					diLength=8;
					if(diCenter<diLength/2)
						diStart=0;
					else
						diStart = diCenter-diLength/2;
				}
				if(diLength+diStart>=wdLength) //???
					diLength=wdLength-diStart;

				ResetScrollBars();
				RefreshDisplayData();
				Invalidate(false);
			}
		}

		void CWaveEdChildView::SetSpecificZoom(int factor)
		{
			float ratio = 1 / (float)pow(zoomBase, factor);
			int newLength=wdLength*ratio;
			if(newLength>=8)
			{
				diLength=wdLength*ratio;

				if(diLength>wdLength) diLength=wdLength;

				if(cursorPos<diLength/2)
					diStart=0;
				else if(cursorPos+diLength/2 > wdLength)
					diStart=wdLength-diLength;
				else
					diStart=cursorPos-diLength/2;

				ResetScrollBars();
				RefreshDisplayData();
				Invalidate(false);
			}
		}

		void CWaveEdChildView::OnSelectionShowall() 
		{
			diStart = 0;
			diLength = wdLength;

			ResetScrollBars(0);
			RefreshDisplayData();
			Invalidate(true);
		}


		//////////////////////////////////////////////////////////////////////////
		//////		Mouse event handlers


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
					wdLoopS = _pSong->_pInstrument[wsInstrument]->waveLoopStart;
					if (!wdLoop) 
					{
						wdLoop=true;
						_pSong->_pInstrument[wsInstrument]->waveLoopType=true;
					}
					_pSong->IsInvalided(false);
					pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
					rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
					InvalidateRect(&rect, false);

				}
/*				else
				{
					if (blSelection) OnSelectionZoomSel();
					else OnSelectionZoomOut();
				}
*/
			}
			CWnd::OnRButtonDown(nFlags, point);
		}


		void CWaveEdChildView::OnLButtonDown(UINT nFlags, CPoint point) 
		{
			SetCapture();
			if(wdWave)
			{
				int const x=point.x;
				int const y=point.y;

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
					wdLoopE = _pSong->_pInstrument[wsInstrument]->waveLoopEnd;

					if (!wdLoop) 
					{
						wdLoop=true;
						_pSong->_pInstrument[wsInstrument]->waveLoopType=true;
					}
					_pSong->IsInvalided(false);
					pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
					rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
					InvalidateRect(&rect, false);
				}
				else if ( nFlags == MK_LBUTTON )
				{
					CRect rect;
					GetClientRect(&rect);
					int const nWidth=rect.Width();
					int const nHeadHeight = rect.Height()/10;


					if(y>nHeadHeight && diLength!=0)		//we're clicking on the main body
					{
						float dispRatio = nWidth/float(diLength);
						if		( blSelection	&&
								abs(x - f2i((blStart-diStart)			* dispRatio )) < 10 )	//mouse down on block start
						{
							SelStart = blStart+blLength;				//set SelStart to the end we're -not- moving
							cursorPos=blStart;
						}
						else if ( blSelection	&&
								abs(x - f2i((blStart+blLength-diStart)	* dispRatio )) < 10 )	//mouse down on block end
						{
							SelStart=blStart;							//set SelStart to the end we're -not- moving
							cursorPos=blStart+blLength;
						}
						else if ( wdLoop		&&
								abs(x - f2i((wdLoopS-diStart)			* dispRatio )) < 10 )	//mouse down on loop start
						{
							bDragLoopStart=true;
						}
						else if ( wdLoop		&&
								abs(x - f2i((wdLoopE-diStart)			* dispRatio )) < 10 )	//mouse down on loop end
						{
							bDragLoopEnd=true;
						}
						else
						{
							blSelection=false;
							
							blStart=diStart+int(x*diLength/nWidth);
							blLength=0;
							SelStart = blStart;
							cursorPos = blStart;

						}
					}
					else					//we're clicking on the header
					{
						float headDispRatio = nWidth/float(wdLength);
						if		( blSelection		&&
								abs( x - f2i( blStart				* headDispRatio ) ) < 10 )	//mouse down on block start
						{
							SelStart = blStart+blLength;
						}
						else if ( blSelection		&&
								abs( x - f2i((blStart+blLength)	* headDispRatio ) ) < 10 )	//mouse down on block end
						{
							SelStart = blStart;
						}
						else
						{
							blSelection=false;
							
							blStart = f2i((x*wdLength)/nWidth);
							blLength=0;
							SelStart = blStart;

						}
					}
					::SetCursor(hResizeLR);
					
					rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
					InvalidateRect(&rect, false);
				}
			}
			
			CWnd::OnLButtonDown(nFlags, point);
		}

		void CWaveEdChildView::OnLButtonDblClk( UINT nFlags, CPoint point )
		{
			OnEditSelectAll();
		}

		void CWaveEdChildView::OnMouseMove(UINT nFlags, CPoint point) //Fideloop's
		{
			int x=point.x;
			int y=point.y;
			CRect rect;
			GetClientRect(&rect);
			int const nWidth=rect.Width();
			int const nHeadHeight = rect.Height()/10;
			rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);

			if(nFlags == MK_LBUTTON && wdWave)
			{
				CRect invHead;
				CRect invBody;
				if(y>nHeadHeight)		//mouse is over body
				{
					float diRatio = (float) diLength/nWidth;
					unsigned long newpos =  (x*diRatio+diStart > 0? x*diRatio+diStart: 0);
					int headX = f2i((diStart+x*diRatio)*nWidth/float(wdLength));
					if(bDragLoopStart)
					{
						if(newpos > wdLoopE)		wdLoopS = wdLoopE;
						else						wdLoopS = newpos;
						_pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopS;
						_pSong->IsInvalided(false);
						pParent->m_wndInst.WaveUpdate();

						//set invalid rects
						float sampWidth = nWidth/(float)diLength+20;
						if(x<prevBodyLoopS)	invBody.SetRect(x-sampWidth, nHeadHeight,				prevBodyLoopS+sampWidth,	rect.Height());
						else				invBody.SetRect(prevBodyLoopS-sampWidth, nHeadHeight,	x+sampWidth,	rect.Height());
						if(headX<prevHeadLoopS)	invHead.SetRect(headX-20, 0,				prevHeadLoopS+20,	nHeadHeight);
						else					invHead.SetRect(prevHeadLoopS-20,0,			headX+20, nHeadHeight);
						prevBodyLoopS=x;	prevHeadLoopS=headX;
					}
					else if(bDragLoopEnd)
					{
						if(newpos >= wdLength)		wdLoopE = wdLength-1;
						else if(newpos >= wdLoopS)	wdLoopE = newpos;
						else						wdLoopE = wdLoopS;
						_pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopE;
						_pSong->IsInvalided(false);
						pParent->m_wndInst.WaveUpdate();

						//set invalid rects
						float sampWidth = nWidth/(float)diLength + 20;
						if(x<prevBodyLoopE)	invBody.SetRect(x-sampWidth, nHeadHeight,				prevBodyLoopE+sampWidth,	rect.Height());
						else				invBody.SetRect(prevBodyLoopE-sampWidth, nHeadHeight,	x+sampWidth,	rect.Height());
						if(headX<prevHeadLoopE)	invHead.SetRect(headX-20, 0,				prevHeadLoopE+20,	nHeadHeight);
						else					invHead.SetRect(prevHeadLoopE-20,0,			headX+20, nHeadHeight);
						prevBodyLoopE=x;	prevHeadLoopE=headX;
					}
					else
					{
						if (newpos >= SelStart)
						{
							if (newpos >= wdLength)	{ newpos = wdLength-1; }
							blStart = (SelStart);
							blLength = newpos - blStart;
							cursorPos=blStart+blLength;
						}
						else
						{
							if (newpos < 0) { newpos = 0; }
							blStart = newpos;
							blLength = SelStart - blStart;
							cursorPos=blStart;
						}
						//set invalid rects
						int sampWidth = nWidth/(float)diLength+1;
						if(x<prevBodyX)			invBody.SetRect(x-sampWidth, nHeadHeight,			prevBodyX+sampWidth,	rect.Height());	
						else					invBody.SetRect(prevBodyX-sampWidth, nHeadHeight,	x+sampWidth,			rect.Height());	
						if(headX<prevHeadX)		invHead.SetRect(headX-1, 0,					prevHeadX+1,	nHeadHeight);
						else					invHead.SetRect(prevHeadX-1, 0,				headX+1,		nHeadHeight);
						prevHeadX=headX;
						prevBodyX=x;
					}
				}
				else					//mouse is over header
				{
					float diRatio = (float) wdLength/nWidth;
					unsigned long newpos = (x * diRatio > 0? x*diRatio: 0);
					if (newpos >= SelStart)
					{
						if (newpos >= wdLength)	{ newpos = wdLength-1;	}
						blStart = SelStart;
						blLength = newpos - blStart;
					}
					else
					{
						blStart = newpos;
						blLength = SelStart-blStart;
					}
					//set invalid rects
					int bodyX = f2i( (x*wdLength - diStart*nWidth)/diLength );
					if(bodyX<0 || bodyX>nWidth)
						invBody.SetRectEmpty();
					else
						if(bodyX<prevBodyX)	invBody.SetRect(bodyX-1,		nHeadHeight,	prevBodyX+1,	rect.Height());
						else				invBody.SetRect(prevBodyX-1,	nHeadHeight,	bodyX+1,		rect.Height());

					if(x<prevHeadX)			invHead.SetRect(x-1, 0,					prevHeadX+1,	nHeadHeight);
					else					invHead.SetRect(prevHeadX-1, 0,			x+1,			nHeadHeight);
					prevBodyX=bodyX;
					prevHeadX=x;
				}
				blSelection=true;
				CRect invalid;
				invalid.UnionRect(&invBody, &invHead);
				InvalidateRect(&invalid, false);
			}
			else 
			{

				if(y>nHeadHeight && diLength!=0)		//mouse is over body
				{
					float dispRatio = nWidth/(float)diLength;
					if	(		blSelection		&&
							(	abs ( x - f2i((  blStart-diStart )			* dispRatio ))  < 10		||
								abs ( x - f2i((  blStart+blLength-diStart)	* dispRatio ))  < 10	)	||
							(	wdLoop &&
							(	abs ( x - f2i((  wdLoopS-diStart )			* dispRatio ))  < 10		||
								abs ( x - f2i((  wdLoopE-diStart )			* dispRatio ))  < 10) )
						)
						::SetCursor(hResizeLR);
					else
						::SetCursor(hIBeam);
				}
				else if (wdLength!=0)					//mouse is over header
				{
					
					float dispRatio = nWidth/(float)wdLength;
					if (		blSelection		&&
							(	abs ( x - f2i(   blStart			* dispRatio ))	< 10 ||
								abs ( x - f2i((  blStart+blLength)	* dispRatio ))	< 10 )
						)
						::SetCursor(hResizeLR);
					else
						::SetCursor(hIBeam);
				}
			}
			
			
			CWnd::OnMouseMove(nFlags, point);
		}

		void CWaveEdChildView::OnLButtonUp(UINT nFlags, CPoint point) 
		{
			if(blLength==0)
				blSelection=false;
			if(bSnapToZero)
			{
				if(blSelection)
				{
					long delta = blStart - FindNearestZero(blStart);
					blStart-=delta;
					blLength+=delta;
					blLength = FindNearestZero(blStart+blLength) - blStart;
				}
				cursorPos = FindNearestZero(cursorPos);
			}
			ReleaseCapture();
			bDragLoopEnd = bDragLoopStart = false;
			CRect rect;
			GetClientRect(&rect);
			rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
			InvalidateRect(&rect, false);
			CWnd::OnLButtonUp(nFlags, point);
		}



		//////////////////////////////////////////////////////////////////////////
		//////		Audio processing functions


		void CWaveEdChildView::OnSelectionFadeIn()
		{
			unsigned long startPoint = (blSelection? blStart: 0);
			unsigned long length = (blSelection? blLength+1: wdLength);
			if(wdWave)
			{
				pParent->m_wndView.AddMacViewUndo();

				Fade(wdLeft+startPoint, length, 0, 1.0f);
				if(wdStereo)
					Fade(wdRight+startPoint, length, 0, 1.0f);

				RefreshDisplayData(true);
				Invalidate(true);
				_pSong->IsInvalided(false);
			}
		}


		void CWaveEdChildView::OnSelectionFadeOut()
		{
			unsigned long startPoint = (blSelection? blStart: 0);
			unsigned long length = (blSelection? blLength+1: wdLength);
			if(wdWave)
			{
				pParent->m_wndView.AddMacViewUndo();

				Fade(wdLeft+startPoint, length, 1.0f, 0);
				if(wdStereo)
					Fade(wdRight+startPoint, length, 1.0f, 0);

				RefreshDisplayData(true);
				Invalidate(true);
				_pSong->IsInvalided(false);
			}
		}

		void CWaveEdChildView::OnSelectionNormalize() // (Fideloop's)
		{
			signed short maxL = 0, maxR = 0, absBuf;
			double ratio = 0;
			unsigned long c = 0;
			unsigned long startPoint = (blSelection? blStart: 0);
			unsigned long length = (blSelection? blLength+1: wdLength);

			if (wdWave)
			{
				pParent->m_wndView.AddMacViewUndo();

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);

				for (c = startPoint ; c < startPoint+length ; c++)
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
					for (c = startPoint; c< length; c++)
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
					Amplify(wdLeft+startPoint, length, ratio);
					if (wdStereo)
					{
						Amplify(wdRight+startPoint, length, ratio);
					}
				}

				RefreshDisplayData(true);
				Invalidate(true);
				_pSong->IsInvalided(false);
			}
		}

		void CWaveEdChildView::OnSelectionRemoveDC() // (Fideloop's)
		{
			double meanL = 0, meanR = 0;
			unsigned long c = 0;
			unsigned long startPoint = (blSelection? blStart: 0);
			unsigned long length = (blSelection? blLength+1: wdLength);
			signed short buf;

			if (wdWave)
			{
				pParent->m_wndView.AddMacViewUndo();

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);

				for (c=startPoint; c<startPoint+length; c++)
				{
					meanL = meanL + ( (double) *(wdLeft+c) / wdLength);

					if (wdStereo) meanR = (double) meanR + ((double) *(wdRight+c) / wdLength);
				}

				for (c=startPoint; c<startPoint+length; c++)
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
					for (c=startPoint; c<startPoint+length; c++)
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
				RefreshDisplayData(true);
				Invalidate(true);
			}
		}

		void CWaveEdChildView::OnSelectionAmplify()
		{
			double ratio =1;
			unsigned long startPoint = (blSelection? blStart: 0);
			unsigned long length = (blSelection? blLength+1: wdLength);
			int pos = 0;

			if (wdWave)
			{
				pParent->m_wndView.AddMacViewUndo();

				pos = AmpDialog.DoModal();
				if (pos != AMP_DIALOG_CANCEL)
				{
					_pSong->IsInvalided(true);
					Sleep(LOCK_LATENCY);
					ratio = pow(10.0, (double) pos / (double) 2000.0);

					Amplify(wdLeft+startPoint, length, ratio);
					if (wdStereo)
						Amplify(wdRight+startPoint, length, ratio);

					_pSong->IsInvalided(false);
					RefreshDisplayData(true);
					Invalidate(true);
				}
			}
		}

		void CWaveEdChildView::OnSelectionReverse() 
		{
			short buf = 0;
			int c, halved = 0;
			unsigned long startPoint = (blSelection? blStart: 0);
			unsigned long length = (blSelection? blLength: wdLength-1);

			if (wdWave)
			{
				pParent->m_wndView.AddMacViewUndo();

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);

				//halved = (int) floor(length/2.0);	
				//<dw> if length is odd (even number of samples), middle two samples aren't flipped:
				halved = (int) ceil(length/2.0f - .1);

				for (c = 0; c < halved; c++)
				{
					buf = *(wdLeft+startPoint+length - c);
					*(wdLeft+startPoint+length - c) = *(wdLeft+startPoint + c);
					*(wdLeft+startPoint + c) = buf;

					if (wdStereo)
					{
						buf = *(wdRight+startPoint+length - c);
						*(wdRight+startPoint+length - c) = *(wdRight+startPoint + c);
						*(wdRight+startPoint + c) = buf;
					}

				}
				RefreshDisplayData(true);
				Invalidate(true);
				_pSong->IsInvalided(false);
			}
		}

		void CWaveEdChildView::OnSelectionInsertSilence()
		{
			if(SilenceDlg.DoModal()!=IDCANCEL)
			{
				unsigned long timeInSamps = Global::configuration().GetSamplesPerSec() * SilenceDlg.timeInSecs;
				if(!wdWave)
				{
					_pSong->WavAlloc(wsInstrument, false, timeInSamps, "New Waveform");
					short *pTmp= new signed short[timeInSamps];
					memset(pTmp, 0, timeInSamps*2 );
					wdLeft = zapArray(_pSong->_pInstrument[wsInstrument]->waveDataL, pTmp);
					wdLength=timeInSamps;
					wdStereo=false;
					wdWave=true;
					OnSelectionShowall();
				}
				else
				{
					unsigned long insertPos;
					switch(SilenceDlg.insertPos)
					{
					case CWaveEdInsertSilenceDialog::at_start:
						insertPos = 0;
						break;
					case CWaveEdInsertSilenceDialog::at_end:
						insertPos = wdLength-1;
						break;
					case CWaveEdInsertSilenceDialog::at_cursor:
						insertPos = cursorPos;
						break;
					default:
						throw;
					}

					short *pTmp = new signed short[timeInSamps + wdLength];		//create new buffer
					memcpy(pTmp, wdLeft, insertPos*2);							//copy pre-insert data
					memset(pTmp + insertPos, 0, timeInSamps*2);					//insert silence
					memcpy((unsigned char*)pTmp + 2*(insertPos+timeInSamps), wdLeft + insertPos, 2*(wdLength - insertPos));	//copy post-insert data
					wdLeft = zapArray(_pSong->_pInstrument[wsInstrument]->waveDataL,pTmp);

					if(wdStereo)
					{
						short *pTmpR= new signed short[timeInSamps + wdLength];
						memcpy(pTmpR,wdRight,insertPos*2);
						memset(pTmpR+insertPos, 0, timeInSamps*2);
						memcpy((unsigned char*)pTmpR+ 2*(insertPos+timeInSamps), wdRight + insertPos, 2*(wdLength - insertPos));
						wdRight = zapArray(_pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
					}

					_pSong->_pInstrument[wsInstrument]->waveLength = wdLength = wdLength + timeInSamps;

					if(wdLoop)		//update loop points if necessary
					{
						if(insertPos<wdLoopS)
						{
							wdLoopS += timeInSamps;
							_pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopS;
						}
						if(insertPos<wdLoopE)
						{
							wdLoopE += timeInSamps;
							_pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopE;
						}
						_pSong->IsInvalided(false);
						pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
					}
				}


				pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.
				RefreshDisplayData(true);
				Invalidate(true);
				_pSong->IsInvalided(false);

			}
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
				RefreshDisplayData(true);
				Invalidate(true);
				_pSong->IsInvalided(false);
			}
		}



		//////////////////////////////////////////////////////////////////////////
		//////		Menu Update Handlers

		void CWaveEdChildView::OnUpdateSelectionAmplify(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave);
		}

		void CWaveEdChildView::OnUpdateSelectionReverse(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave);
			
		}

		void CWaveEdChildView::OnUpdateSelectionFadein(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave);
			
		}

		void CWaveEdChildView::OnUpdateSelectionFadeout(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave);	
		}

		void CWaveEdChildView::OnUpdateSelectionNormalize(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave);	
		}

		void CWaveEdChildView::OnUpdateSelectionRemovedc(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave);	
		}

		void CWaveEdChildView::OnUpdateSeleccionZoomIn(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && (diLength > 16));
		}

		void CWaveEdChildView::OnUpdateSeleccionZoomOut(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && (diLength < wdLength) );
		}

		void CWaveEdChildView::OnUpdateSelectionZoomSel(CCmdUI* pCmdUI)
		{
			pCmdUI->Enable(wdWave && blSelection);
		}

		void CWaveEdChildView::OnUpdateSelectionShowall(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && (diLength < wdLength) );		
		}

		void CWaveEdChildView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection);
		}

		void CWaveEdChildView::OnUpdateEditCut(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection);
		}

		void CWaveEdChildView::OnUpdateEditCrop(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection);
		}

		void CWaveEdChildView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable( IsClipboardFormatAvailable(CF_WAVE) );	
		}

		void CWaveEdChildView::OnUpdateEditDelete(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && blSelection);	
		}

		void CWaveEdChildView::OnUpdateConvertMono(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave && wdStereo);
		}

		void CWaveEdChildView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(false);
		}

		void CWaveEdChildView::OnUpdateEditSelectAll(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(wdWave);
		}

		void CWaveEdChildView::OnUpdateEditSnapToZero(CCmdUI* pCmdUI)
		{
			pCmdUI->SetCheck((bSnapToZero? 1: 0));
			pCmdUI->Enable();
		}

		void CWaveEdChildView::OnUpdatePasteOverwrite(CCmdUI* pCmdUI)
		{
			pCmdUI->Enable(wdWave && IsClipboardFormatAvailable(CF_WAVE) );
		}

		void CWaveEdChildView::OnUpdatePasteMix(CCmdUI* pCmdUI)
		{
			pCmdUI->Enable(wdWave && IsClipboardFormatAvailable(CF_WAVE) );
		}

		void CWaveEdChildView::OnUpdatePasteCrossfade(CCmdUI* pCmdUI)
		{
			pCmdUI->Enable(wdWave && IsClipboardFormatAvailable(CF_WAVE) );
		}

		void CWaveEdChildView::OnUpdateSelectionInsertSilence(CCmdUI* pCmdUI)
		{
			pCmdUI->Enable();
		}

		afx_msg void CWaveEdChildView::OnUpdateSetLoopStart(CCmdUI* pCmdUI)
		{
			pCmdUI->Enable( wdWave );
		}
		afx_msg void CWaveEdChildView::OnUpdateSetLoopEnd(CCmdUI* pCmdUI)
		{
			pCmdUI->Enable( wdWave );
		}


		//////////////////////////////////////////////////////////////////////////
		//////		Clipboard Functions

		void CWaveEdChildView::OnEditDelete()
		{
			short* pTmp = 0, *pTmpR = 0;
			long datalen = 0;

			if (wdWave && blSelection)
			{
				pParent->m_wndView.AddMacViewUndo();

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);
				unsigned long length = blLength+1;

				datalen = (wdLength - length);
				if (datalen)
				{
					pTmp = new signed short[datalen];
					
					if (wdStereo)
					{
						pTmpR= new signed short[datalen];
						CopyMemory(pTmpR, wdRight, blStart*sizeof(short));
						CopyMemory( (pTmpR+blStart), (wdRight + blStart + length), (wdLength - blStart - length)*sizeof(short) );
						zapArray(_pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
						wdRight = pTmpR;
					}

					CopyMemory( pTmp, wdLeft, blStart*sizeof(short) );
					CopyMemory( (pTmp+blStart), (wdLeft + blStart + length), (wdLength - blStart - length)*sizeof(short) );
					zapArray(_pSong->_pInstrument[wsInstrument]->waveDataL,pTmp);
					wdLeft = pTmp;
					_pSong->_pInstrument[wsInstrument]->waveLength = datalen;
					wdLength = datalen;
					//	adjust loop points if necessary
					if(wdLoop)
					{
						if(blStart+length<wdLoopS)
						{
							wdLoopS -= length;
							_pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopS;
						}
						if(blStart+length<wdLoopE)
						{
							wdLoopE -= length;
							_pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopE;
						}
					}

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

				ResetScrollBars();
				RefreshDisplayData();

				pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.

				RefreshDisplayData(true);
				Invalidate(true);
				_pSong->IsInvalided(false);
			}
		}

		void CWaveEdChildView::OnEditCopy() 
		{
			unsigned long c = 0;
			unsigned long length = blLength+1;
				
			struct fullheader
			{
				std::uint32_t	head;
				std::uint32_t	size;
				std::uint32_t	head2;
				std::uint32_t	fmthead;
				std::uint32_t	fmtsize;
				WAVEFORMATEX	fmtcontent;
				std::uint32_t datahead;
				std::uint32_t datasize;
			} wavheader;

			OpenClipboard();
			EmptyClipboard();
			hClipboardData = GlobalAlloc(GMEM_MOVEABLE, ( wdStereo ? length*4 + sizeof(fullheader) : length*2 + sizeof(fullheader)));
			
			wavheader.head = 'FFIR';
			wavheader.size = wdStereo ? (length*4 + sizeof(fullheader) - 8) : (length*2 + sizeof(fullheader) - 8);
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
			wavheader.datasize = wdStereo ? length*4 : length*2;

			pClipboardData = (char*) GlobalLock(hClipboardData);
			
			CopyMemory(pClipboardData, &wavheader, sizeof(fullheader) );
			if (wdStereo)
			{
				pClipboardData += sizeof(fullheader);
				for (c = 0; c < length*2; c += 2)
				{
					*((signed short*)pClipboardData + c) = *(wdLeft + blStart + (long)(c*0.5));
					*((signed short*)pClipboardData + c + 1) = *(wdRight + blStart + (long)(c*0.5));
				}
			}
			else
			{
				CopyMemory(pClipboardData + sizeof(fullheader), (wdLeft + blStart), length*2);
			}

			GlobalUnlock(hClipboardData);
			SetClipboardData(CF_WAVE, hClipboardData);
			CloseClipboard();
			RefreshDisplayData(true);
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
			///\todo : fix the blLengths. There need to be some +1 and -1 throughout the source.
			if (blLength > 2 ) OnEditDelete();
			
			blSelection = true;
			blStart = 0;
			blLength = blStartTemp;
			if (blLength > 2 ) OnEditDelete();
		}

		void CWaveEdChildView::OnEditPaste() 
		{
			unsigned long c = 0;

			pParent->m_wndView.AddMacViewUndo();

			char *pData;
			std::uint32_t lFmt, lData;
			
			WAVEFORMATEX* pFmt;
			short* pPasteData;
			short* pTmp = 0, *pTmpR = 0;

			OpenClipboard();
			hPasteData = GetClipboardData(CF_WAVE);
			pPasteData = (short*)GlobalLock(hPasteData);

			if ((*(std::uint32_t*)pPasteData != 'FFIR') && (*((std::uint32_t*)pPasteData + 2)!='EVAW')) return;
			lFmt= *(std::uint32_t*)((char*)pPasteData + 16);
			pFmt = (WAVEFORMATEX*)((char*)pPasteData + 20); //'RIFF' + len. +'WAVE' + 'fmt ' + len. = 20 bytes.

			lData = *(std::uint32_t*)((char*)pPasteData + 20 + lFmt + 4);
			pData = (char*)pPasteData + 20 + lFmt + 8;

			unsigned long lDataSamps = (unsigned long)(lData/pFmt->nBlockAlign);	//data length in bytes divided by number of bytes per sample
			int bytesPerSamp = (int)(pFmt->nBlockAlign/pFmt->nChannels);
			_pSong->IsInvalided(true);
			Sleep(LOCK_LATENCY);

			if (!wdWave)
			{
				if (pFmt->wBitsPerSample == 16)
				{
					_pSong->WavAlloc(wsInstrument, (pFmt->nChannels==2) ? true : false, lDataSamps, "Clipboard");
					wdLength = lDataSamps;
					wdLeft  = _pSong->_pInstrument[wsInstrument]->waveDataL;
					if (pFmt->nChannels == 1)
					{
						memcpy(_pSong->_pInstrument[wsInstrument]->waveDataL, pData, lData);
						wdStereo = false;
					}
					else if (pFmt->nChannels == 2)
					{
						for (c = 0; c < lDataSamps; ++c)
						{
							*(_pSong->_pInstrument[wsInstrument]->waveDataL + c) = *(signed short*)(pData + c*pFmt->nBlockAlign);
							*(_pSong->_pInstrument[wsInstrument]->waveDataR + c) = *(signed short*)(pData + c*pFmt->nBlockAlign + (int)(pFmt->nBlockAlign/2));
						}
						wdRight = _pSong->_pInstrument[wsInstrument]->waveDataR;
						wdStereo = true;
					}
					wdWave = true;
					OnSelectionShowall();
				}
			}
			else
			{
				if (pFmt->wBitsPerSample == 16 && pFmt->nChannels==1 || pFmt->nChannels==2)
				{
					if ( ((pFmt->nChannels == 1) && (wdStereo == true)) ||		//todo: deal with this better.. i.e. dialog box offering to convert clipboard data
						 ((pFmt->nChannels == 2) && (wdStereo == false)) )
						 return;

					//paste left channel
					pTmp = new signed short[lDataSamps + wdLength];
					memcpy(pTmp, wdLeft, cursorPos*bytesPerSamp);

					for (c = 0; c < lDataSamps; c++)
						*(pTmp + cursorPos + c) = *(short*)(pData + c*pFmt->nBlockAlign);

					memcpy((unsigned char*)pTmp + bytesPerSamp*(cursorPos+lDataSamps), wdLeft+cursorPos, bytesPerSamp*(wdLength-cursorPos));
					wdLeft  = zapArray(_pSong->_pInstrument[wsInstrument]->waveDataL, pTmp);

					if(pFmt->nChannels==2)	//if stereo, paste right channel
					{
						pTmpR= new signed short[lDataSamps + wdLength];
						memcpy(pTmpR, wdRight, cursorPos*bytesPerSamp);

						for (c = 0; c < lDataSamps; c++)
							*(pTmpR+ cursorPos + c) = *(short*)(pData + c*pFmt->nBlockAlign + (int)(pFmt->nBlockAlign/2));

						memcpy((unsigned char*)pTmpR+ bytesPerSamp*(cursorPos+lDataSamps), wdRight + cursorPos, bytesPerSamp*(wdLength - cursorPos));
						wdRight = zapArray(_pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
					}

					//update length
					_pSong->_pInstrument[wsInstrument]->waveLength = wdLength + lDataSamps;
					wdLength = wdLength + lDataSamps;

					//	adjust loop points if necessary
					if(wdLoop)
					{
						if(cursorPos<wdLoopS)
						{
							wdLoopS += lDataSamps;
							_pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopS;
						}
						if(cursorPos<wdLoopE)
						{
							wdLoopE += lDataSamps;
							_pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopE;
						}
					}
				}
			}

			GlobalUnlock(hPasteData);
			CloseClipboard();
			//OnSelectionShowall();

			ResetScrollBars();
			RefreshDisplayData(true);

			pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.
			Invalidate(true);
			_pSong->IsInvalided(false);
		}

		void CWaveEdChildView::OnPasteOverwrite()
		{
			unsigned long startPoint;

			pParent->m_wndView.AddMacViewUndo();

			char *pData;
			std::uint32_t lFmt, lData;
			
			WAVEFORMATEX* pFmt;
			short* pPasteData;
			short* pTmp = 0, *pTmpR = 0;

			OpenClipboard();
			hPasteData = GetClipboardData(CF_WAVE);
			pPasteData = (short*)GlobalLock(hPasteData);

			if ((*(std::uint32_t*)pPasteData != 'FFIR') && (*((std::uint32_t*)pPasteData + 2)!='EVAW')) return;
			lFmt= *(std::uint32_t*)((char*)pPasteData + 16);
			pFmt = (WAVEFORMATEX*)((char*)pPasteData + 20); //'RIFF' + len. +'WAVE' + 'fmt ' + len. = 20 bytes.

			lData = *(std::uint32_t*)((char*)pPasteData + 20 + lFmt + 4);
			pData = (char*)pPasteData + 20 + lFmt + 8;

			unsigned long lDataSamps = (int)(lData/pFmt->nBlockAlign);	//data length in bytes divided by number of bytes per sample

			_pSong->IsInvalided(true);
			Sleep(LOCK_LATENCY);

			if (pFmt->wBitsPerSample == 16 && pFmt->nChannels==1 || pFmt->nChannels==2)
			{
				if ( ((pFmt->nChannels == 1) && (wdStereo == true)) ||		//todo: deal with this better.. i.e. dialog box offering to convert clipboard data
						((pFmt->nChannels == 2) && (wdStereo == false)) )
						return;
				unsigned long c;

				if(blSelection)	//overwrite selected block
				{
					//if clipboard data is longer than selection, truncate it
					if(lDataSamps>blLength+1)
					{
						lData=(blLength+1)*pFmt->nBlockAlign; 
						lDataSamps=blLength+1;
					}
					startPoint=blStart;
				}
				else		//overwrite at cursor
				{
					//truncate to current wave size	(should we be extending in this case??)
					if(lDataSamps>(wdLength-cursorPos))
					{
						lData=(wdLength-cursorPos)*pFmt->nBlockAlign;
						lDataSamps=wdLength-cursorPos;
					}
					startPoint=cursorPos;
				}

				//do left channel
				pTmp = new signed short[wdLength];
				memcpy(pTmp, wdLeft, startPoint*2);
				for (c = 0; c < lDataSamps; c++)
					*(pTmp + startPoint + c) = *(short*)(pData + c*pFmt->nBlockAlign);
				memcpy((unsigned char*)pTmp + 2*(startPoint+lDataSamps), wdLeft  + startPoint+lDataSamps, 2*(wdLength-startPoint-lDataSamps));
				wdLeft = zapArray(_pSong->_pInstrument[wsInstrument]->waveDataL, pTmp);
				
				if(pFmt->nChannels==2)	//do right channel if stereo
				{
					pTmpR= new signed short[wdLength];
					memcpy(pTmpR,wdRight,startPoint*2);
					for (c = 0; c < lDataSamps; c++)
						*(pTmpR+ startPoint + c) = *(short*)(pData + c*pFmt->nBlockAlign + int(pFmt->nBlockAlign*0.5));
					memcpy((unsigned char*)pTmpR+ 2*(startPoint+lDataSamps), wdRight + startPoint+lDataSamps, 2*(wdLength-startPoint-lDataSamps));
					wdRight = zapArray(_pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
				}
			}
			GlobalUnlock(hPasteData);
			CloseClipboard();
			//OnSelectionShowall();

			ResetScrollBars();
			RefreshDisplayData(true);

			pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.
			Invalidate(true);
			_pSong->IsInvalided(false);

		}

		void CWaveEdChildView::OnPasteMix()
		{
			unsigned long c = 0;
			unsigned long startPoint;

			if(MixDlg.DoModal() != IDCANCEL)
			{

				pParent->m_wndView.AddMacViewUndo();

				char *pData;
				std::uint32_t lFmt, lData;
				
				WAVEFORMATEX* pFmt;
				short* pPasteData;
				short* pTmp = 0, *pTmpR = 0;

				OpenClipboard();
				hPasteData = GetClipboardData(CF_WAVE);
				pPasteData = (short*)GlobalLock(hPasteData);

				if ((*(std::uint32_t*)pPasteData != 'FFIR') && (*((std::uint32_t*)pPasteData + 2)!='EVAW')) return;
				lFmt= *(std::uint32_t*)((char*)pPasteData + 16);
				pFmt = (WAVEFORMATEX*)((char*)pPasteData + 20); //'RIFF' + len. +'WAVE' + 'fmt ' + len. = 20 bytes.

				lData = *(std::uint32_t*)((char*)pPasteData + 20 + lFmt + 4);
				pData = (char*)pPasteData + 20 + lFmt + 8;

				unsigned long lDataSamps = (unsigned long)(lData/pFmt->nBlockAlign);	//data length in bytes divided by number of bytes per sample

				unsigned long fadeInSamps(0), fadeOutSamps(0);
				unsigned long destFadeIn(0);	

				if(MixDlg.bFadeIn)
					fadeInSamps = Global::configuration().GetSamplesPerSec() * MixDlg.fadeInTime;
				if(MixDlg.bFadeOut)
					fadeOutSamps= Global::configuration().GetSamplesPerSec() * MixDlg.fadeOutTime;

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);

				if (pFmt->wBitsPerSample == 16 && ( pFmt->nChannels==1 || pFmt->nChannels==2 ) )
				{
					if ( ((pFmt->nChannels == 1) && (wdStereo == true)) ||		//todo: deal with this better.. i.e. dialog box offering to convert clipboard data
							((pFmt->nChannels == 2) && (wdStereo == false)) )
							return;

					if(blSelection)	//overwrite selected block
					{
						//if clipboard data is longer than selection, truncate it
						if(lDataSamps>blLength+1) 
						{
							lData=blLength*pFmt->nBlockAlign;
							lDataSamps = blLength+1;
						}
						startPoint=blStart;
					}
					else		//overwrite at cursor
						startPoint=cursorPos;

					unsigned long newLength;
					if(startPoint+lDataSamps < wdLength)
						newLength = wdLength;
					else
						newLength = startPoint+lDataSamps;

					if(fadeInSamps>lDataSamps) fadeInSamps=lDataSamps;
					if(fadeOutSamps>lDataSamps) fadeOutSamps=lDataSamps;
					if(startPoint+fadeInSamps<wdLength)	destFadeIn = fadeInSamps;		//we need to do some thinking about what reason the user
					else								destFadeIn = wdLength-startPoint;	//could possibly have for using it this way, and if this
																							//is how the program should behave if it happens
					pTmp = new signed short[newLength];

					for( c=0; c<newLength; c++ ) pTmp[c] = 0;	//zero out pTmp
					for (c = 0; c < lDataSamps; c++)
						*(pTmp+startPoint+c) = *(short*)(pData + c*pFmt->nBlockAlign);	//copy clipboard data to pTmp
					Fade(pTmp+startPoint, fadeInSamps, 0, MixDlg.srcVol);				//do fade in on clipboard data
					Fade(wdLeft +startPoint, destFadeIn, 1.0f, MixDlg.destVol);			//do fade in on wave data
					Amplify(pTmp +startPoint+fadeInSamps, lDataSamps-fadeInSamps-fadeOutSamps, MixDlg.srcVol);	//amplify non-faded part of clipboard data

					if(startPoint+lDataSamps < wdLength)
					{
						Amplify(wdLeft +startPoint+destFadeIn, lDataSamps-destFadeIn-fadeOutSamps, MixDlg.destVol); //amplify wave data
						Fade(wdLeft +startPoint+lDataSamps-fadeOutSamps, fadeOutSamps, MixDlg.destVol, 1.0f);	//fade out wave data
						Fade(pTmp   +startPoint+lDataSamps-fadeOutSamps, fadeOutSamps, MixDlg.srcVol, 0);		//fade out clipboard data
					}
					else	//ignore fade out in this case, it doesn't make sense here
						Amplify(wdLeft +startPoint+destFadeIn, wdLength-startPoint-destFadeIn, MixDlg.destVol);	//amplify wave data

					Mix(pTmp, wdLeft, newLength, wdLength);		//mix into pTmp
					wdLeft =zapArray(_pSong->_pInstrument[wsInstrument]->waveDataL,pTmp);

					if( pFmt->nChannels == 2 )
					{
						pTmpR= new signed short[newLength];
						for ( c=0; c<newLength; c++ )	pTmpR[c]=0;
						for (c = 0; c < lDataSamps; c++)
							*(pTmpR+startPoint+c)= *(short*)(pData + c*pFmt->nBlockAlign + int(pFmt->nBlockAlign*0.5));

						Fade(pTmpR+startPoint, fadeInSamps, 0, MixDlg.srcVol);
						Fade(wdRight+startPoint, destFadeIn, 1.0f, MixDlg.destVol);
						Amplify(pTmpR+startPoint+fadeInSamps, lDataSamps-fadeInSamps-fadeOutSamps, MixDlg.srcVol);

						if(startPoint+lDataSamps < wdLength)
						{
							Amplify(wdRight+startPoint+destFadeIn, lDataSamps-destFadeIn-fadeOutSamps, MixDlg.destVol);
							Fade(wdRight+startPoint+lDataSamps-fadeOutSamps, fadeOutSamps, MixDlg.destVol, 1.0f);
							Fade(pTmpR+startPoint+lDataSamps-fadeOutSamps, fadeOutSamps, MixDlg.srcVol, 0);
						}
						else
							Amplify(wdRight+startPoint+destFadeIn, wdLength-startPoint-destFadeIn, MixDlg.destVol);

						Mix(pTmpR, wdRight, newLength, wdLength);		//mix into pTmpR
						wdRight=zapArray(_pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
					}

					if(newLength>wdLength)
						_pSong->_pInstrument[wsInstrument]->waveLength = wdLength = startPoint + lDataSamps;

				}

				GlobalUnlock(hPasteData);
				CloseClipboard();
				//OnSelectionShowall();

				ResetScrollBars();
				RefreshDisplayData(true);

				pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.
				Invalidate(true);
				_pSong->IsInvalided(false);
			}

		}



		void CWaveEdChildView::OnPasteCrossfade()
		{
			if(XFadeDlg.DoModal() != IDCANCEL)
			{
				unsigned long c = 0;
				unsigned long startPoint, endPoint;

				pParent->m_wndView.AddMacViewUndo();

				char *pData;
				std::uint32_t lFmt, lData;
				
				WAVEFORMATEX* pFmt;
				short* pPasteData;
				short* pTmp = 0, *pTmpR = 0;

				OpenClipboard();
				hPasteData = GetClipboardData(CF_WAVE);
				pPasteData = (short*)GlobalLock(hPasteData);

				if ((*(std::uint32_t*)pPasteData != 'FFIR') && (*((std::uint32_t*)pPasteData + 2)!='EVAW')) return;
				lFmt= *(std::uint32_t*)((char*)pPasteData + 16);
				pFmt = (WAVEFORMATEX*)((char*)pPasteData + 20); //'RIFF' + len. +'WAVE' + 'fmt ' + len. = 20 bytes.

				lData = *(std::uint32_t*)((char*)pPasteData + 20 + lFmt + 4);
				pData = (char*)pPasteData + 20 + lFmt + 8;

				unsigned long lDataSamps = (unsigned long)(lData/pFmt->nBlockAlign);	//data length in bytes divided by number of bytes per sample

				_pSong->IsInvalided(true);
				Sleep(LOCK_LATENCY);

				if (pFmt->wBitsPerSample == 16 && (pFmt->nChannels == 1 || pFmt->nChannels == 2))
				{
					if ( ((pFmt->nChannels == 1) && (wdStereo == true)) ||		//todo: deal with this better.. i.e. dialog box offering to convert clipboard data
							((pFmt->nChannels == 2) && (wdStereo == false)) )
							return;

					if(blSelection)	//overwrite selected block
					{
						startPoint=blStart;
						if(lDataSamps>blLength+1)
							endPoint = startPoint+blLength+1;		//selection determines length of the crossfade
						else 
							endPoint = startPoint+lDataSamps;	//if selection is longer, fade length is length of clipboard data
					}
					else		//overwrite at cursor
					{
						startPoint=cursorPos;
						if(startPoint+lDataSamps < wdLength)
							endPoint = startPoint+lDataSamps;	//if clipboard data fits in existing wave, its length is fade length
						else
							endPoint = wdLength;				//if not, the end of the existing wave marks the end of the fade
					}

					unsigned long newLength;
					if(startPoint+lDataSamps < wdLength)
						newLength = wdLength;				//end wave same size as start wave
					else
						newLength = startPoint + lDataSamps;	//end wave larger than at start

					//process left channel:

					pTmp = new signed short[newLength];	
					for( c=0; c<newLength; c++ ) pTmp[c]=0;									//zero out pTmp
					for (c = 0; c < lDataSamps; c++)										//copy clipboard into pTmp for processing
						*(pTmp + startPoint + c) = *(short*)(pData + c*pFmt->nBlockAlign);
					Fade(pTmp +startPoint, endPoint-startPoint, XFadeDlg.srcStartVol, XFadeDlg.srcEndVol);			//fade clipboard data
					Fade(wdLeft +startPoint, endPoint-startPoint, XFadeDlg.destStartVol, XFadeDlg.destEndVol);		//fade wave data
					Mix(pTmp, wdLeft, newLength, wdLength);															//mix clipboard with wave
					wdLeft = zapArray(_pSong->_pInstrument[wsInstrument]->waveDataL, pTmp);

					if(pFmt->nChannels==2)	//process right channel
					{
						pTmpR= new signed short[newLength];
						for( c=0; c<newLength; c++ ) pTmpR[c]=0;
						for (c = 0; c < lDataSamps; c++)
							*(pTmpR + startPoint + c) = *(short*)(pData + c*pFmt->nBlockAlign + (int)(pFmt->nBlockAlign*0.5));
						Fade(pTmpR+startPoint, endPoint-startPoint, XFadeDlg.srcStartVol, XFadeDlg.srcEndVol);
						Fade(wdRight+startPoint, endPoint-startPoint, XFadeDlg.destStartVol, XFadeDlg.destEndVol);
						Mix(pTmpR, wdRight, newLength, wdLength);
						wdRight = zapArray(_pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
					}
					if(newLength > wdLength)
						_pSong->_pInstrument[wsInstrument]->waveLength = wdLength = newLength;
				}


				GlobalUnlock(hPasteData);
				CloseClipboard();

				ResetScrollBars();
				RefreshDisplayData(true);

				pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.
				Invalidate(true);
				_pSong->IsInvalided(false);
			}

		}



		void CWaveEdChildView::OnEditSelectAll() 
		{
			diStart = 0;
			blStart = 0;
			diLength = wdLength;
			blLength = (wdLength>1? wdLength-2: 0);	//blStart+blLength+1 needs to point to a valid sample- wdLength is one too many
			blSelection = true;

			ResetScrollBars();
			RefreshDisplayData();

			Invalidate(true);
		}

		void CWaveEdChildView::OnDestroyClipboard() 
		{
			CWnd::OnDestroyClipboard();
			GlobalFree(hClipboardData);
		}

		void CWaveEdChildView::OnEditSnapToZero()
		{
			bSnapToZero=!bSnapToZero;
		}


		void CWaveEdChildView::OnPopupSetLoopStart()
		{
			pParent->m_wndView.AddMacViewUndo();
			_pSong->IsInvalided(true);
			Sleep(LOCK_LATENCY);
			CRect rect;
			GetClientRect(&rect);
			int nWidth = rect.Width();
			wdLoopS = diStart + rbX * diLength/nWidth;
			_pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopS;
			if (_pSong->_pInstrument[wsInstrument]->waveLoopEnd< wdLoopS )
			{
				_pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopS;
			}
			wdLoopE = _pSong->_pInstrument[wsInstrument]->waveLoopEnd;
			if (!wdLoop) 
			{
				wdLoop=true;
				_pSong->_pInstrument[wsInstrument]->waveLoopType=true;
			}
			_pSong->IsInvalided(false);

			pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
			rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
			InvalidateRect(&rect, false);
		}
		void CWaveEdChildView::OnPopupSetLoopEnd()
		{
			pParent->m_wndView.AddMacViewUndo();
			_pSong->IsInvalided(true);
			Sleep(LOCK_LATENCY);

			CRect rect;
			GetClientRect(&rect);
			int nWidth = rect.Width();
			wdLoopE = diStart + rbX * diLength/nWidth;
			_pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopE;
			if (_pSong->_pInstrument[wsInstrument]->waveLoopStart> wdLoopE )
			{
				_pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopE;
			}
			wdLoopS = _pSong->_pInstrument[wsInstrument]->waveLoopStart;
			if (!wdLoop) 
			{
				wdLoop=true;
				_pSong->_pInstrument[wsInstrument]->waveLoopType=true;
			}
			_pSong->IsInvalided(false);
			pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
			rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
			InvalidateRect(&rect, false);
		}

		void CWaveEdChildView::OnPopupSelectionToLoop()
		{
			if(!blSelection) return;

			wdLoopS = blStart;
			wdLoopE = blStart+blLength;
			_pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopS;
			_pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopE;
			if (!wdLoop) 
			{
				wdLoop=true;
				_pSong->_pInstrument[wsInstrument]->waveLoopType=true;
			}

			_pSong->IsInvalided(false);
			pParent->m_wndInst.WaveUpdate();
			CRect rect;
			GetClientRect(&rect);
			rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
			InvalidateRect(&rect, false);
		}


		void CWaveEdChildView::SetSong(Song* _sng)
		{
			_pSong = _sng;
		}
		void CWaveEdChildView::SetParent(CMainFrame* parent)
		{
			pParent = parent;
		}
		unsigned long CWaveEdChildView::GetWaveLength()
		{
			if(wdWave)
				return wdLength;
			else
				return 0;
		}
		bool CWaveEdChildView::IsStereo()
		{
			return wdStereo;
		}
		unsigned long CWaveEdChildView::GetSelectionLength()
		{
			if(wdWave && blSelection)
				return blLength;
			else
				return 0;
		}
		unsigned long CWaveEdChildView::GetCursorPos()
		{
			if(wdWave)
				return cursorPos;
			else
				return 0;
		}
		void CWaveEdChildView::SetCursorPos(unsigned long newpos)
		{
			if(newpos<0 || newpos>=wdLength)
				return;

			cursorPos = newpos;
			if(cursorPos < diLength) diStart=0;
			else if(cursorPos>wdLength-diLength) diStart=wdLength-diLength;
			else diStart = cursorPos-(diLength/2);

			ResetScrollBars();
			RefreshDisplayData();
			Invalidate(false);
		}

		void CWaveEdChildView::ResetScrollBars(bool bNewLength)
		{
			CScrollBar* hScroll = (CScrollBar*) zoombar.GetDlgItem(IDC_HSCROLL);
			CSliderCtrl* zoomSlider = (CSliderCtrl*) zoombar.GetDlgItem(IDC_ZOOMSLIDE);
			CSliderCtrl* volSlider = (CSliderCtrl*) zoombar.GetDlgItem(IDC_VOLSLIDE);

			if(wdWave)
			{
				//set horizontal scroll bar
				SCROLLINFO si;
				si.cbSize=sizeof si;
				si.nPage=diLength;
				si.nPos=diStart;
				if(bNewLength)
				{
					si.fMask=SIF_PAGE | SIF_POS | SIF_RANGE;
					si.nMin=0; si.nMax=wdLength-1;
				}
				else
					si.fMask=SIF_PAGE | SIF_POS;

				hScroll->SetScrollInfo(&si);

				//set zoom slider
				if(bNewLength)
				{
					//here we're looking for the highest n where wdLength/(b^n)>8 , where b is zoomBase and n is the max value for the zoom slider.
					//the value of the slider determines a wdLength/diLength ratio of b^n.. so this way, diLength is limited to a minimum of 8 samples.
					//another alternative to consider would be to use a fixed range, and change the zoomBase based on the wavelength to match..

					float maxzoom = log10(float(wdLength/8.0f))/log10(zoomBase);	// wdLength/(b^n)>=8    <==>   n <= log<b>(wdLength/8)
					// log<10>(x)/log<10>(b) == log<b>(x)
					int slidermax = f2i(floor(maxzoom));
					if(slidermax<0) slidermax=0;			//possible for waves with less than 8 samples (!)
					zoomSlider->SetRange(0, slidermax);
				}
				if(diLength!=0)
				{

					//this is the same concept, except this is to give us some idea of where to draw the slider based on the existing zoom
					//so, instead of wdLength/8 (the max zoom), we're doing wdLength/diLength (the current zoom)
					float zoomfactor = log10(wdLength/(float)diLength)/log10(zoomBase);
					int newpos = f2i(zoomfactor+0.5f);
					if(newpos<0)	newpos=0;		//i'm not sure how this would happen, but just in case
					zoomSlider->SetPos(newpos);
				}
			}
			else
			{
				//disabled scrollbar
				SCROLLINFO si;
				si.cbSize=sizeof si;
				si.nPage=0;
				si.nPos=0;
				si.fMask=SIF_PAGE | SIF_POS | SIF_RANGE;
				si.nMin=0; si.nMax=0;
				hScroll->SetScrollInfo(&si);

				//disabled zoombar
				zoomSlider->SetRange(0, 0);
				zoomSlider->SetPos(0);
			}

			//set volume slider
			volSlider->SetRange(0, 100);
			volSlider->SetPos( _pSong->waved.GetVolume() );
			volSlider->Invalidate(false);
		}


		////////////////////////////////////////////
		////		FindNearestZero()
		////	searches for the zero crossing nearest to a given sample index.
		////  returns the sample index of the nearest zero, or, in the event that the nearest zero crossing never actually hits zero,
		////	it will return the index of the sample that comes the closest to zero.  if the index is out of range, the last sample
		////	index is returned.  the first and last sample of the wave are considered zero.
		unsigned long CWaveEdChildView::FindNearestZero(unsigned long startpos)
		{
			if(startpos>=wdLength) return wdLength-1;
			
			float sign;
			bool bLCLZ=false, bLCRZ=false, bRCLZ=false, bRCRZ = false;		//right/left chan, right/left zero
			unsigned long LCLZPos(0), LCRZPos(0), RCLZPos(0), RCRZPos(0);
			unsigned long ltRange, rtRange;	//these refer to the left and right side of the startpos, not the left/right channel
			ltRange=startpos;
			rtRange=wdLength-startpos;

			// do left channel
			if(wdLeft[startpos]<0) sign=-1;
			else if(wdLeft[startpos]>0) sign=1;
			else return startpos;		//easy enough..

			//left chan, left side
			for(unsigned long i=1; i<=ltRange; ++i)		//start with i=1-- since we're looking for a sign change from startpos, i=0 will never give us what we want
			{
				if( wdLeft[startpos-i] * sign <= 0 )		//if this product is negative, sign has switched.
				{
					LCLZPos=startpos-i;
					if(abs(wdLeft[LCLZPos+1]) < abs(wdLeft[LCLZPos]))				//check if the last one was closer to zero..
						LCLZPos++;													//and if so, set to that sample.
					bLCLZ=true;
					if(rtRange>i)rtRange=i;		//limit further searches
					ltRange=i;
					break;
				}
			}
			
			//left chan, right side
			for(unsigned long i=1; i<rtRange; ++i)
			{
				if( wdLeft[startpos+i] * sign <= 0 )
				{
					LCRZPos = startpos+i;
					if(abs(wdLeft[LCRZPos-1]) < abs(wdLeft[LCRZPos]))
						LCRZPos--;
					bLCRZ=true;
					break;
				}
			}

			if(wdStereo)
			{
				// do right channel
				if(wdRight[startpos]<0) sign=-1;
				else if(wdRight[startpos]>0) sign=1;
				else return startpos;		//easy enough..

				//right chan, left side
				for(unsigned long i=1; i<=ltRange; ++i)
				{
					if( wdRight[startpos-i] * sign <= 0 )
					{
						RCLZPos=startpos-i;
						if(abs(wdRight[RCLZPos+1]) < abs(wdRight[RCLZPos]))
							RCLZPos++;
						bRCLZ=true;
						break;
					}
				}
				
				//right chan, right side
				for(unsigned long i=1; i<rtRange; ++i)
				{
					if( wdRight[startpos+i] * sign <= 0 )
					{
						RCRZPos = startpos+i;
						if(abs(wdRight[RCRZPos-1]) < abs(wdRight[RCRZPos]))
							RCRZPos--;
						bRCRZ=true;
						break;
					}
				}

			}

			//find the closest
			unsigned long ltNearest=0, rtNearest=wdLength-1;

			if(wdStereo)
			{
				if(bLCLZ || bRCLZ)		//there's a zero on the left side
				{
					if(!bRCLZ)				//only in the left channel?
						ltNearest=LCLZPos;	//then that one's closest..
					else if(!bLCLZ)			//..and vice versa
						ltNearest=RCLZPos;
					else					//zeros in both chans?
						ltNearest = ( LCLZPos>RCLZPos? LCLZPos: RCLZPos );	//both should be lower than startpos, so the highest is the closest
				}

				if(bLCRZ || bLCRZ)
				{
					if(!bRCRZ)
						rtNearest=LCRZPos;
					else if(!bLCRZ)
						rtNearest=RCRZPos;
					else
						rtNearest = (LCRZPos<RCRZPos? LCRZPos: RCRZPos );
				}
			}
			else		//mono sample
			{
				if(bLCLZ)
					ltNearest = LCLZPos;
				if(bLCRZ)
					rtNearest = LCRZPos;
			}

			if(startpos-ltNearest < rtNearest-startpos)
				return ltNearest;
			else
				return rtNearest;
		}

		void CWaveEdChildView::RefreshDisplayData(bool bRefreshHeader /*=false */)
		{
			if(wdWave)
			{
				CRect rect;
				GetClientRect(&rect);
				int const cyHScroll=GetSystemMetrics(SM_CYHSCROLL);
				int const nWidth = rect.Width();
				if(nWidth==0)return;
				int const nHeadHeight = rect.Height()/10;
				int const nHeight= rect.Height()-cyHScroll-nHeadHeight;
				int const my=nHeight/2;
				int const myHead=nHeadHeight/2;
				int wrHeight;
				int wrHeadHeight;

				if(wdStereo)
				{	wrHeight=my/2;	wrHeadHeight=myHead/2;	}
				else 
				{	wrHeight=my;	wrHeadHeight=myHead;	}

				float OffsetStep = (float) diLength / nWidth;
				float HeaderStep = (float) wdLength / nWidth;
				int yLow, yHi;

				lDisplay.resize(nWidth);
				
				for(int c(0); c < nWidth; c++)
				{
					long const offset = diStart + (long)(c * OffsetStep);
					yLow = 0, yHi = 0;

					for (long d(offset); d < offset + ((OffsetStep <1) ? 1 : OffsetStep); d++)
					{
						int value = *(wdLeft+d);
						if (yLow > value) yLow = value;
						if (yHi <  value) yHi  = value;
					}

					lDisplay[c].first  = (wrHeight * yLow)/32768;
					lDisplay[c].second = (wrHeight * yHi )/32768;
				}
				if(wdStereo)
				{
					rDisplay.resize(nWidth);
					for(int c(0); c < nWidth; c++)
					{
						long const offset = diStart + (long)(c * OffsetStep);
						yLow = 0, yHi = 0;

						for (long d(offset); d < offset + ((OffsetStep <1) ? 1 : OffsetStep); d++)
						{
							int value = *(wdRight+d);
							if (yLow > value) yLow = value;
							if (yHi <  value) yHi  = value;
						}

						rDisplay[c].first  = (wrHeight * yLow)/32768;
						rDisplay[c].second = (wrHeight * yHi )/32768;
					}
				}
				if(bRefreshHeader)
				{
					// left channel of header
					// todo: very low-volume samples tend to disappear.. we should round up instead of down
					lHeadDisplay.resize(nWidth);
					for(int c(0); c < nWidth; c++)
					{
						long const offset = long(c * HeaderStep);
						yLow = 0; yHi = 0;

						for (long d(offset); d < offset + (HeaderStep<1? 1: HeaderStep); d++)
						{
							int value = *(wdLeft+d);
							if (yLow > value) yLow = value;
							if (yHi <  value) yHi  = value;
						}
						lHeadDisplay[c].first = (wrHeadHeight * yLow)/32768;
						lHeadDisplay[c].second= (wrHeadHeight * yHi )/32768;
					}
					if(wdStereo)
					{
						// right channel of header
						// todo: very low-volume samples tend to disappear.. we should round up instead of down
						rHeadDisplay.resize(nWidth);
						for(int c(0); c < nWidth; c++)
						{
							long const offset = long(c * HeaderStep);
							yLow = 0; yHi = 0;

							for ( long d(offset); d < offset + (HeaderStep<1? 1: HeaderStep); d++)
							{
								int value = *(wdRight+d);
								if (yLow > value) yLow = value;
								if (yHi <  value) yHi  = value;
							}
							rHeadDisplay[c].first = (wrHeadHeight * yLow)/32768;
							rHeadDisplay[c].second= (wrHeadHeight * yHi )/32768;
						}
					}

				}

			}
			else
			{
				lDisplay.clear();
				rDisplay.clear();
				lHeadDisplay.clear();
				rHeadDisplay.clear();
			}
		}

		//Mix - mixes two audio buffers, possibly of different lengths.
		//mixed buffer will be put in the first buffer, so if the lengths are different, be sure that
		//the bigger one is the first argument.  passing a negative value for lhsVol and/or rhsVol will effectively
		//invert both/either buffer in addition to mixing.
		void CWaveEdChildView::Mix(short* lhs, short *rhs, int lhsSize, int rhsSize, float lhsVol, float rhsVol)
		{
			if(rhsSize>lhsSize)
				return;
			if(lhsSize<=0 || rhsSize<0) return;

			for( int i(0); i<rhsSize; i++ )
			{
				*(lhs+i) = *(lhs+i) * lhsVol + *(rhs+i) * rhsVol;
			}
			for( int i(rhsSize); i<lhsSize; ++i )
			{
				*(lhs+i)*=lhsVol;
			}
		}

		//Fade - fades an audio buffer from one volume level to another.
		void CWaveEdChildView::Fade(short* data, int length, float startVol, float endVol)
		{
			if(length<=0) return;
			float slope = (endVol-startVol)/(float)length;

			for(int i(0);i<length;++i)
				*(data+i) *= startVol+i*slope;
		}

		//Amplify - multiplies an audio buffer by a given factor.  buffer can be inverted by passing
		//	a negative value for vol.
		void CWaveEdChildView::Amplify(short *data, int length, float vol)
		{
			if(length<=0) return;

			int current;
			for(int i(0);i<length;++i)
			{
				current = *(data+i) * vol;
				if( current>32767 )		current=32767;
				else if( current<-32768 )	current=-32768;
				*(data+i) = static_cast<short>(current);
			}
		}
	
	NAMESPACE__END
NAMESPACE__END
