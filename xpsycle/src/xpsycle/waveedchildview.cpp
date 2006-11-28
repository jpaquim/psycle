/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
  *   natti@linux   *
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  *   This program is distributed in the hope that it will be useful,       *
  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
  *   GNU General Public License for more details.                          *
  *                                                                         *
  *   You should have received a copy of the GNU General Public License     *
  *   along with this program; if not, write to the                         *
  *   Free Software Foundation, Inc.,                                       *
  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
  ***************************************************************************/
//#include "xpsycle.cpp"
#include "song.h"
#include "mainwindow.h"
#include "waveedamplifydlg.h"
#include "waveedinsertsilencedlg.h"
#include "waveedmixdlg.h"
#include "waveedchildview.h"
#include "waveedxfadedlg.h"
#ifdef __unix__
#include <X11/cursorfont.h>
#endif
#include <ngrs/nwindow.h>
#include <ngrs/ngraphics.h>
#include <ngrs/napp.h>
#include <ngrs/npopupmenu.h>
#include <ngrs/nmenu.h>
#include <ngrs/nitem.h>
#include <ngrs/nmenuseperator.h>
#include <ngrs/nscrollbar.h>
#include <ngrs/nbutton.h>
#include <ngrs/nstatusbar.h>
#include <ngrs/ntextstatusitem.h>
//#include <mmreg.h>
#include <cmath>

namespace psycle { namespace host {

	
	float const WaveEdChildView::zoomBase = 1.06f;

	WaveEdChildView::WaveEdChildView(Song* pSong_)
	{
		pSong = pSong_;

		clrHi.setRGB(0x77, 0xDD, 0x00);
		clrMe.setRGB(0xCC, 0xCC, 0xCC);
		clrLo.setRGB(0x00, 0x00, 0xFF);
		clrWhite.setRGB(0xEE, 0xEE, 0xEE);
		clrBlack.setRGB(0, 0, 0);
		
		std::string fontname="Lucida Typewriter";
		fntLoop=new NFont(fontname);
		fntLoop->setTextColor(clrLo);

		bSnapToZero=true;
		bDragLoopStart = bDragLoopEnd = false;
		SelStart=0;
		cursorPos=0;
		pSong->waved.SetVolume(0.4f);
		wdWave=false;
		wsInstrument=-1;
//		prevHeadLoopS = prevBodyLoopS = prevHeadLoopE = prevBodyLoopE = 0;
//		prevBodyX = prevHeadX = 0;
		bLButtonDown = false;

		setLayout(NAlignLayout());
		InitPopupMenu();
		InitStatusBar();
		InitZoomBar();
		
		waveArea = new WavePanel(this);
		add(waveArea,nAlClient);
		setAlign(nAlClient);

		timer.setIntervalTime(750);
		timer.enableTimer();
		timer.timerEvent.connect(this, &WaveEdChildView::onTimer);
		
		AmpDlg = new WaveEdAmplifyDialog;
		add(AmpDlg);
		SilenceDlg = new WaveEdInsertSilenceDialog;
		add(SilenceDlg);
		MixDlg = new WaveEdMixDialog;
		add(MixDlg);
		XFadeDlg = new WaveEdCrossfadeDialog;
		add(XFadeDlg);
//		SetTimer(31415, 750, 0);
//		ResetScrollBars();
	}

	WaveEdChildView::~WaveEdChildView()
	{
	}

	void WaveEdChildView::onTimer()
	{
		cursorBlink = !cursorBlink;
		repaint();
	}

	void WaveEdChildView::resize()
	{
		NPanel::resize();
		RefreshDisplayData(true);
	}
	
	void WaveEdChildView::InitPopupMenu()
	{
		popup = new NPopupMenu;

		NMenuItem* zoomInItem = new NMenuItem("Zoom In");
		zoomInItem->click.connect(this, &WaveEdChildView::onPopupZoomIn);
		popup->add(zoomInItem);

		NMenuItem* zoomOutItem =new NMenuItem("Zoom Out");
		zoomOutItem->click.connect(this, &WaveEdChildView::onPopupZoomOut);
		popup->add(zoomOutItem);
		
		NMenuItem* zoomSelItem = new NMenuItem("Zoom to Selection");
		zoomSelItem->click.connect(this, &WaveEdChildView::onSelectionZoomSel);
		popup->add(zoomSelItem);

		popup->add(new NMenuSeperator);

		NMenuItem* loopStartItem =new NMenuItem("Set Loop Start");
		loopStartItem->click.connect(this, &WaveEdChildView::onPopupSetLoopStart);
		popup->add(loopStartItem);
		
		NMenuItem* loopEndItem =new NMenuItem("Set Loop End");
		loopEndItem->click.connect(this, &WaveEdChildView::onPopupSetLoopEnd);
		popup->add(loopEndItem);
		
		NMenuItem* loopSelItem =new NMenuItem("Set Selection to Loop");
		loopSelItem->click.connect(this, &WaveEdChildView::onPopupSelectionToLoop);
		popup->add(loopSelItem);
	}

	void WaveEdChildView::InitZoomBar()
	{
		zoomBar = new NPanel();
		zoomBar->setLayout(NAlignLayout());
		volSlider = new VolumeSlider(pSong);
		volSlider->setPreferredSize(75,15);
		zoomSlider = new NSlider();
		zoomSlider->setPreferredSize(100,15);
		zoomOutButton = new NButton("-");//, 15, 15);
		zoomOutButton->setPreferredSize(15,15);
		zoomInButton = new NButton("+");//, 15, 15);
		zoomInButton->setPreferredSize(15,15);
		scrollBar = new NScrollBar();
		
		scrollBar->setOrientation(nHorizontal);
		scrollBar->setSmallChange(1);
 
		scrollBar->change.connect(this,&WaveEdChildView::onHScroll);
		
		volSlider->slider()->setTransparent(true);
		volSlider->setRange(0, 100);
		volSlider->setPos(pSong->waved.GetVolume()*100.0f);
    volSlider->change.connect(this, &WaveEdChildView::onVolSliderScroll);
		
		zoomSlider->setTransparent(false);
		zoomSlider->setOrientation(nHorizontal);
		zoomSlider->change.connect(this, &WaveEdChildView::onZoomSliderScroll);
		
		zoomOutButton->clicked.connect(this, &WaveEdChildView::onSelectionZoomOut);
		zoomOutButton->setFlat(false);
		zoomInButton ->clicked.connect(this, &WaveEdChildView::onSelectionZoomIn);
		zoomInButton->setFlat(false);
		
		
		add(zoomBar, nAlBottom);
		
		zoomBar->add(volSlider,nAlLeft);
		zoomBar->add(zoomInButton,nAlRight);
		zoomBar->add(zoomSlider,nAlRight);
		zoomBar->add(zoomOutButton,nAlRight);
		zoomBar->add(scrollBar,nAlClient);
	}

	void WaveEdChildView::InitStatusBar()
	{
		statusBar=new NStatusBar();
		lengthText = new NTextStatusItem();
		selText = new NTextStatusItem();
		statusText = new NTextStatusItem();
		modeText = new NTextStatusItem();
		statusBar->add(statusText, nAlLeft);
		statusBar->add(modeText, nAlRight);
		statusBar->add(lengthText, nAlRight);
		statusBar->add(selText, nAlRight);
		add(statusBar, nAlBottom);
	}
	
	void WaveEdChildView::UpdateStatusBar()
	{
		{
			std::ostringstream s;
	
			if(blLength==0 || !pSong->_pInstrument[wsInstrument])
				s << "No Data in Selection.";
			else
			{
				//todo!!
				float slInSecs = blLength / 44100.0f;
				s << "Selection: "<<blLength<<" ("<<slInSecs<<" secs.)";
			}
			selText->setText(s.str());
		}
		
		{
			std::ostringstream s;
			//todo!!
			float wlInSecs = wdLength / 44100.0f;
			s<<"Size: "<<wdLength<<" ("<<wlInSecs<<" secs.)";
			lengthText->setText(s.str());
		}
		
		if (wdWave)
		{
			if (wdStereo)	modeText->setText("Mode: Stereo");
			else			modeText->setText("Mode: Mono");
		}
		else modeText->setText("Mode: Empty");
		
		//for now..
		statusText->setText("Ready");
	}
	void WaveEdChildView::onHScroll( NScrollBar *sender )
	{  
		diStart = sender->pos();
		if(diStart>wdLength-diLength) diStart = wdLength-diLength;
		RefreshDisplayData();
		repaint();			
	}
	void WaveEdChildView::onVolSliderScroll( NSlider *slider)
	{
		pSong->waved.SetVolume( slider->pos() / 100.0f);
		volSlider->repaint();
	}
	void WaveEdChildView::onZoomSliderScroll( NSlider *slider )
	{
		int newzoom = (int) slider->pos();
		SetSpecificZoom(newzoom);
		waveArea->repaint();
	}


    void WaveEdChildView::WavePanel::paint(NGraphics* g)
    {
		int wrHeight = 0;
		int wrHeadHeight=0;
		int c;

		if(wView_->wdWave)
		{
			//const NRegion invalidRgn = g->repaintArea();
			int const barHeight = 0;	//height of scroll bar and status bar combined
			
			int const nHeadHeight=clientHeight()/10;
			int const nWidth=clientWidth();
			int const nHeight=clientHeight()-nHeadHeight-barHeight;
			
			int const my =		(int)(nHeight*0.5f);
			int const myHead =	(int)(nHeadHeight*0.5f);

			if(wView_->wdStereo)
			{
				wrHeight =		(int)(my*0.5f);
				wrHeadHeight =	(int)(myHead*0.5f);
			}
			else 
			{
				wrHeight=my;
				wrHeadHeight=myHead;
			}

			//ratios used to convert from sample indices to pixels
			float dispRatio = nWidth/(float)wView_->diLength;
			float headDispRatio = nWidth/(float)wView_->wdLength;

			g->setForeground(wView_->clrBlack);
			g->fillRect(0, 0, nWidth, clientHeight()-barHeight);

			if(wView_->blLength)
			{
				unsigned long selx, selx2;
				selx =wView_->blStart;
				selx2=wView_->blStart+wView_->blLength;
	
				int HeadSelX = int(selx * headDispRatio);
				int HeadSelX2= int(selx2* headDispRatio);
				g->setForeground(wView_->clrWhite);
				g->fillRect(HeadSelX, 0, HeadSelX2-HeadSelX, nHeadHeight);
	
				if(selx<wView_->diStart) selx=wView_->diStart;
				if(selx2>wView_->diStart+wView_->diLength) selx2=wView_->diStart+wView_->diLength;
				//if the selected block is entirely off the screen, the above statements will flip the order
				if(selx2>selx)					//if not, it will just clip the drawing
				{
					selx = int((selx -wView_->diStart)*dispRatio) ;
					selx2= int((selx2-wView_->diStart)*dispRatio) ;
					g->fillRect(selx, nHeadHeight,  selx2-selx, nHeight);
	
				}
			}
			// Draw preliminary stuff
			g->setForeground(wView_->clrMe);
			
			// Left channel 0 amplitude line
			g->drawLine(0, wrHeight+nHeadHeight,  nWidth, wrHeight+nHeadHeight);
			// Left Header 0 amplitude line
			g->drawLine(0, wrHeadHeight,  nWidth, wrHeadHeight);
			
			int const wrHeight_R = my + wrHeight;
			int const wrHeadHeight_R = myHead + wrHeadHeight;
			
			g->setForeground(wView_->clrWhite);
			// Header/Body divider
			g->drawLine(0, nHeadHeight,  nWidth, nHeadHeight);
			if(wView_->wdStereo)
			{
				// Stereo channels separator line
				g->setForeground(wView_->clrLo);
				g->drawLine(0, my+nHeadHeight,  nWidth, my+nHeadHeight);
				// Stereo channels Header Separator
				g->drawLine(0, myHead,  nWidth, myHead);
				
				g->setForeground(wView_->clrMe);
				// Right channel 0 amplitude line
				g->drawLine(0, wrHeight_R+nHeadHeight,  nWidth, wrHeight_R+nHeadHeight);
				// Right Header 0 amplitude line
				g->drawLine(0, wrHeadHeight_R,  nWidth, wrHeadHeight_R);
			}
			// Draw samples in channels (Fideloop's)
			g->setForeground(wView_->clrHi);
			for(c = 0; c < nWidth; c++)
			{
				g->drawLine(c, wrHeight - wView_->lDisplay.at(c).first  + nHeadHeight,
							c, wrHeight - wView_->lDisplay.at(c).second + nHeadHeight);
				g->drawLine(c, wrHeadHeight - wView_->lHeadDisplay.at(c).first,
							c, wrHeadHeight - wView_->lHeadDisplay.at(c).second );
			}
			if(wView_->wdStereo)
			{
				//draw right channel wave data
				for(c = 0; c < nWidth; c++)
				{
					g->drawLine(c,wrHeight_R - wView_->rDisplay.at(c).first + nHeadHeight,
								c,wrHeight_R - wView_->rDisplay.at(c).second + nHeadHeight);
					g->drawLine(c, wrHeadHeight_R-wView_->rHeadDisplay.at(c).first,
								c, wrHeadHeight_R-wView_->rHeadDisplay.at(c).second );
				}
			}
			//draw loop points
			if ( wView_->wdLoop )
			{
				g->setForeground(wView_->clrLo);
				g->setFont(*wView_->fntLoop);
				if ( wView_->wdLoopS >= wView_->diStart && wView_->wdLoopS < wView_->diStart+wView_->diLength)
				{
					int ls = int((wView_->wdLoopS-wView_->diStart)*dispRatio);
					g->drawLine(ls, nHeadHeight,  ls, nHeight+nHeadHeight);
					int textlen = g->textWidth("Start");
					g->drawText(ls-textlen/2, nHeadHeight+g->textHeight(), "Start" );
				}
				g->setForeground(wView_->clrLo);
				if ( wView_->wdLoopE >= wView_->diStart && wView_->wdLoopE < wView_->diStart+wView_->diLength)
				{
					int le = int((wView_->wdLoopE-wView_->diStart)*dispRatio);
					g->drawLine(le, nHeadHeight,  le, nHeight+nHeadHeight);
					int textLen = g->textWidth("End");
					g->drawText( le - textLen/2, nHeight+nHeadHeight, "End");
				}
				
				//draw loop points in header
				int ls = (int)(wView_->wdLoopS * headDispRatio);
				int le = (int)(wView_->wdLoopE * headDispRatio);

				g->setForeground(wView_->clrLo);
				g->drawLine(ls, 0,  ls, nHeadHeight);
				g->drawLine(le, 0, le, nHeadHeight);
			}

			//draw screen size on header
			g->setForeground(wView_->clrWhite);
			int screenx  = int( wView_->diStart           * headDispRatio);
			int screenx2 = int((wView_->diStart+wView_->diLength) * headDispRatio);
			g->drawLine(screenx,  0,  				screenx,  nHeadHeight-1);
			g->drawLine(screenx,  nHeadHeight-1,	screenx2, nHeadHeight-1);
			g->drawLine(screenx2, nHeadHeight-1,	screenx2, 0);
			g->drawLine(screenx2, 0,				screenx,  0);
			
			if(wView_->cursorBlink	&&	wView_->cursorPos >= wView_->diStart	&&	wView_->cursorPos <= wView_->diStart+wView_->diLength)
			{
				int cursorX = int((wView_->cursorPos-wView_->diStart)*dispRatio);
				g->drawLine(cursorX, nHeadHeight,  cursorX, nHeadHeight+nHeight);
			}
		}
		else
		{
			g->setForeground(wView_->clrWhite);
			g->drawText(4, 4+g->textHeight(), "No Wave Data");
		}

}





	void  WaveEdChildView::SetViewData(int ins)
	{
		wsInstrument = ins;
		int wl=pSong->_pInstrument[ins]->waveLength;
		
		if(wl)
		{
			wdWave=true;
				
			wdLength=wl;
			wdLeft=pSong->_pInstrument[ins]->waveDataL;
			wdRight=pSong->_pInstrument[ins]->waveDataR;
			wdStereo=pSong->_pInstrument[ins]->waveStereo;
			wdLoop=pSong->_pInstrument[ins]->waveLoopType;
			wdLoopS=pSong->_pInstrument[ins]->waveLoopStart;
			wdLoopE=pSong->_pInstrument[ins]->waveLoopEnd;

			diStart=0;
			diLength=wl;
			blStart=0;
			blLength=0;

		}
		else
		{
			wdWave=false;
//			SetWindowText("Wave Editor [No Data]");
		}

		blSelection=false;
		RefreshDisplayData(true);
		ResetScrollBars(true);
		UpdateStatusBar();
		repaint();
	}

	void WaveEdChildView::VolumeSlider::paint(NGraphics *g)
	{
		float vol = pSong->waved.GetVolume();
		NColor clrBlue;
		NColor clrGray;
		NColor clrBlack;
		clrBlue.setRGB(0, 128, 200);
		clrGray.setRGB(40, 40, 40);
		clrBlack.setRGB(0, 0, 0);
	
		int left=0;
		int top=0;
		int width = clientWidth();
		int height = clientHeight();
		
		g->setForeground(clrBlack);
		g->fillRect(left, top, width, height);
		
		g->setForeground(clrGray);
		g->fillRect(left+7, top+2, width-14, height-4);
		g->setForeground(clrBlue);
		g->fillRect(left+7, top+2, int(vol*(width-14)), height-4);

		g->setForeground(clrBlack);		
		
		NPoint points[3];
		points[0].setX( left+5 );		
    points[0].setY( top+height-6 );
		points[1].setX( left+width-2);	
    points[1].setY( top+1 );
		points[2].setX( left+5);		
    points[2].setY( top+1 );
		g->fillPolygon(points, 3);
		
	}

	//////////////////////////////////////////////////////////////////////////
	//////		Zoom Functions



	void WaveEdChildView::onSelectionZoomIn(NButtonEvent* ev)
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
		repaint();

	}

	void WaveEdChildView::onPopupZoomIn(NButtonEvent* ev)
	{
		int nWidth = clientWidth();
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
		repaint();
	}

	void WaveEdChildView::onSelectionZoomOut(NButtonEvent* ev)
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
			repaint();
		}
	}

	void WaveEdChildView::onPopupZoomOut(NButtonEvent* ev)
	{
		int nWidth = clientWidth();
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
		repaint();
	}



	void WaveEdChildView::onSelectionZoomSel(NButtonEvent* ev)
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
			repaint();
		}
	}

	void WaveEdChildView::SetSpecificZoom(int factor)
	{
		float ratio = 1 / (float)pow(zoomBase, factor);
		int newLength=(int)(wdLength*ratio);
		if(newLength>=8)
		{
			diLength=(int)(wdLength*ratio);

			if(diLength>wdLength) diLength=wdLength;

			if(cursorPos<diLength/2)
				diStart=0;
			else if(cursorPos+diLength/2 > wdLength)
				diStart=wdLength-diLength;
			else
				diStart=cursorPos-diLength/2;

			ResetScrollBars();
			RefreshDisplayData();
			repaint();
		}
	}

	void WaveEdChildView::onSelectionShowall(NButtonEvent* ev) 
	{
		diStart = 0;
		diLength = wdLength;

		ResetScrollBars(0);
		RefreshDisplayData();
//		Invalidate(true);
	}


	//////////////////////////////////////////////////////////////////////////
	//////		Mouse event handlers


void WaveEdChildView::WavePanel::onMousePress(int x, int y, int button)
{
	if(button==1)	//left
	{
//		SetCapture();
		if(wView_->wdWave)
		{
			if (   NApp::system().shiftState() & nsCtrl )
			{
//				pParent->m_wndView.AddMacViewUndo();
				wView_->pSong->IsInvalided(true);
//				Sleep(LOCK_LATENCY);

				wView_->wdLoopS = wView_->diStart+((x*wView_->diLength)/clientWidth());
				wView_->pSong->_pInstrument[wView_->wsInstrument]->waveLoopStart=wView_->wdLoopS;
				if (wView_->pSong->_pInstrument[wView_->wsInstrument]->waveLoopEnd < wView_->wdLoopS )
				{
					wView_->pSong->_pInstrument[wView_->wsInstrument]->waveLoopEnd=wView_->wdLoopS;
				}
				wView_->wdLoopE = wView_->pSong->_pInstrument[wView_->wsInstrument]->waveLoopEnd;

				if (!wView_->wdLoop) 
				{
					wView_->wdLoop=true;
					wView_->pSong->_pInstrument[wView_->wsInstrument]->waveLoopType=true;
				}
				wView_->pSong->IsInvalided(false);
//				pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
				repaint();
			}
			else
			{
				int const nWidth=clientWidth();
				int const nHeadHeight = clientHeight()/10;


				if(y>nHeadHeight && wView_->diLength!=0)		//we're clicking on the main body
				{
					float dispRatio = nWidth/float(wView_->diLength);
					if		( wView_->blSelection	&&
							abs(x - f2i((wView_->blStart-wView_->diStart)			* dispRatio )) < 10 )	//mouse down on block start
					{
						wView_->SelStart = wView_->blStart+wView_->blLength;				//set SelStart to the end we're -not- moving
						wView_->cursorPos=wView_->blStart;
					}
					else if ( wView_->blSelection	&&
							abs(x - f2i((wView_->blStart+wView_->blLength-wView_->diStart)	* dispRatio )) < 10 )	//mouse down on block end
					{
						wView_->SelStart=wView_->blStart;							//set SelStart to the end we're -not- moving
						wView_->cursorPos=wView_->blStart+wView_->blLength;
					}
					else if ( wView_->wdLoop		&&
							abs(x - f2i((wView_->wdLoopS-wView_->diStart)			* dispRatio )) < 10 )	//mouse down on loop start
					{
						wView_->bDragLoopStart=true;
					}
					else if ( wView_->wdLoop		&&
							abs(x - f2i((wView_->wdLoopE-wView_->diStart)			* dispRatio )) < 10 )	//mouse down on loop end
					{
						wView_->bDragLoopEnd=true;
					}
					else
					{
						wView_->blSelection=false;
						
						wView_->blStart=wView_->diStart+int(x*wView_->diLength/nWidth);
						wView_->blLength=0;
						wView_->SelStart = wView_->blStart;
						wView_->cursorPos = wView_->blStart;

					}
				}
				else					//we're clicking on the header
				{
					float headDispRatio = nWidth/float(wView_->wdLength);
					if		( wView_->blSelection		&&
							abs( x - f2i( wView_->blStart				* headDispRatio ) ) < 10 )	//mouse down on block start
					{
						wView_->SelStart = wView_->blStart+wView_->blLength;
					}
					else if ( wView_->blSelection		&&
							abs( x - f2i((wView_->blStart+wView_->blLength)	* headDispRatio ) ) < 10 )	//mouse down on block end
					{
						wView_->SelStart = wView_->blStart;
					}
					else
					{
						wView_->blSelection=false;
						
						wView_->blStart = f2i((x*wView_->wdLength)/nWidth);
						wView_->blLength=0;
						wView_->SelStart = wView_->blStart;

					}
				}
				#ifdef __unix__
				XDefineCursor(NApp::system().dpy(),window()->win(),XCreateFontCursor(NApp::system().dpy(),XC_right_side));
                #endif				
				repaint();
				wView_->UpdateStatusBar();
			}
		}
		wView_->bLButtonDown = true;
	}
	else if(button==3)
	{
		if(wView_->wdWave)
		{

			if ( NApp::system().shiftState() & nsCtrl )
			{
//				pParent->m_wndView.AddMacViewUndo();
				wView_->pSong->IsInvalided(true);

				wView_->wdLoopE = wView_->diStart+((x*wView_->diLength)/clientWidth());
				wView_->pSong->_pInstrument[wView_->wsInstrument]->waveLoopEnd=wView_->wdLoopE;
				if (wView_->pSong->_pInstrument[wView_->wsInstrument]->waveLoopStart> wView_->wdLoopE )
				{
					wView_->pSong->_pInstrument[wView_->wsInstrument]->waveLoopStart=wView_->wdLoopE;
				}
				wView_->wdLoopS = wView_->pSong->_pInstrument[wView_->wsInstrument]->waveLoopStart;
				if (!wView_->wdLoop) 
				{
					wView_->wdLoop=true;
					wView_->pSong->_pInstrument[wView_->wsInstrument]->waveLoopType=true;
				}
				wView_->pSong->IsInvalided(false);
//				pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
				repaint();

			}
			else	//do context menu
			{
				wView_->rbX=x;
				wView_->rbY=y;
				wView_->popup->setPosition(x+absoluteLeft()+window()->left(), y+absoluteTop()+window()->top(),100,100);
				wView_->popup->setVisible(true);
			}
		}
		
	}
}
	
	void WaveEdChildView::WavePanel::onMouseDoublePress(int x, int y, int button)
	{
		//...
	}

	void WaveEdChildView::WavePanel::onMouseOver(int x, int y)
	{
		int const nWidth=clientWidth();
		int const nHeadHeight = clientHeight()/10;

		if(wView_->bLButtonDown && wView_->wdWave)
		{
			if(y>nHeadHeight)		//mouse is over body
			{
				float diRatio = (float) wView_->diLength/nWidth;
				unsigned long newpos =  (x*diRatio+wView_->diStart > 0? (unsigned long)(x*diRatio+wView_->diStart): 0);
				int headX = int( (wView_->diStart+x*diRatio)*nWidth/float(wView_->wdLength) );
				if(wView_->bDragLoopStart)
				{
					if(newpos > wView_->wdLoopE)		wView_->wdLoopS = wView_->wdLoopE;
					else						wView_->wdLoopS = newpos;
					wView_->pSong->_pInstrument[wView_->wsInstrument]->waveLoopStart=wView_->wdLoopS;
					wView_->pSong->IsInvalided(false);
//					pParent->m_wndInst.WaveUpdate();

					//set invalid rects
/*					float sampWidth = nWidth/(float)diLength+20;
					if(x<prevBodyLoopS)	invBody.SetRect(x-sampWidth, nHeadHeight,				prevBodyLoopS+sampWidth,	rect.Height());
					else				invBody.SetRect(prevBodyLoopS-sampWidth, nHeadHeight,	x+sampWidth,	rect.Height());
					if(headX<prevHeadLoopS)	invHead.SetRect(headX-20, 0,				prevHeadLoopS+20,	nHeadHeight);
					else					invHead.SetRect(prevHeadLoopS-20,0,			headX+20, nHeadHeight);
					prevBodyLoopS=x;	prevHeadLoopS=headX;
*/				}
				else if(wView_->bDragLoopEnd)
				{
					if(newpos >= wView_->wdLength)		wView_->wdLoopE = wView_->wdLength-1;
					else if(newpos >= wView_->wdLoopS)	wView_->wdLoopE = newpos;
					else						wView_->wdLoopE = wView_->wdLoopS;
					wView_->pSong->_pInstrument[wView_->wsInstrument]->waveLoopEnd=wView_->wdLoopE;
					wView_->pSong->IsInvalided(false);
//					pParent->m_wndInst.WaveUpdate();

					//set invalid rects
/*					float sampWidth = nWidth/(float)diLength + 20;
					if(x<prevBodyLoopE)	invBody.SetRect(x-sampWidth, nHeadHeight,				prevBodyLoopE+sampWidth,	rect.Height());
					else				invBody.SetRect(prevBodyLoopE-sampWidth, nHeadHeight,	x+sampWidth,	rect.Height());
					if(headX<prevHeadLoopE)	invHead.SetRect(headX-20, 0,				prevHeadLoopE+20,	nHeadHeight);
					else					invHead.SetRect(prevHeadLoopE-20,0,			headX+20, nHeadHeight);
					prevBodyLoopE=x;	prevHeadLoopE=headX;
*/				}
				else
				{
					if (newpos >= wView_->SelStart)
					{
						if (newpos >= wView_->wdLength)	{ newpos = wView_->wdLength-1; }
						wView_->blStart = wView_->SelStart;
						wView_->blLength = newpos - wView_->blStart;
						wView_->cursorPos=wView_->blStart+wView_->blLength;
					}
					else
					{
						if (newpos < 0) { newpos = 0; }
						wView_->blStart = newpos;
						wView_->blLength = wView_->SelStart - wView_->blStart;
						wView_->cursorPos=wView_->blStart;
					}
					//set invalid rects
/*					int sampWidth = nWidth/(float)diLength+1;
					if(x<prevBodyX)			invBody.SetRect(x-sampWidth, nHeadHeight,			prevBodyX+sampWidth,	rect.Height());	
					else					invBody.SetRect(prevBodyX-sampWidth, nHeadHeight,	x+sampWidth,			rect.Height());	
					if(headX<prevHeadX)		invHead.SetRect(headX-1, 0,					prevHeadX+1,	nHeadHeight);
					else					invHead.SetRect(prevHeadX-1, 0,				headX+1,		nHeadHeight);
					prevHeadX=headX;
					prevBodyX=x;
*/
					wView_->UpdateStatusBar();
				}
			}
			else					//mouse is over header
			{
				float diRatio = (float) wView_->wdLength/nWidth;
				unsigned long newpos = (x * diRatio > 0? (unsigned long)(x*diRatio): 0);
				if (newpos >= wView_->SelStart)
				{
					if (newpos >= wView_->wdLength)	{ newpos = wView_->wdLength-1;	}
					wView_->blStart = wView_->SelStart;
					wView_->blLength = newpos - wView_->blStart;
				}
				else
				{
					wView_->blStart = newpos;
					wView_->blLength = wView_->SelStart-wView_->blStart;
				}
				//set invalid rects
/*				int bodyX = int( (x*wdLength - diStart*nWidth)/diLength );
				if(bodyX<0 || bodyX>nWidth)
					invBody.SetRectEmpty();
				else
					if(bodyX<prevBodyX)	invBody.SetRect(bodyX-1,		nHeadHeight,	prevBodyX+1,	rect.Height());
					else				invBody.SetRect(prevBodyX-1,	nHeadHeight,	bodyX+1,		rect.Height());

				if(x<prevHeadX)			invHead.SetRect(x-1, 0,					prevHeadX+1,	nHeadHeight);
				else					invHead.SetRect(prevHeadX-1, 0,			x+1,			nHeadHeight);
				prevBodyX=bodyX;
				prevHeadX=x;
*/
				wView_->UpdateStatusBar();			
			}
			wView_->blSelection=true;
/*			CRect invalid;
			invalid.UnionRect(&invBody, &invHead);
			InvalidateRect(&invalid, false);
*/		wView_->repaint();
		}
		else 
		{

			//todo
			
			if(y>nHeadHeight && wView_->diLength!=0)		//mouse is over body
			{
				float dispRatio = nWidth/(float)wView_->diLength;
				if	(		wView_->blSelection		&&
						(	abs ( x - int((  wView_->blStart-wView_->diStart )			* dispRatio ))  < 10		||
							abs ( x - int((  wView_->blStart+wView_->blLength-wView_->diStart)	* dispRatio ))  < 10	)	||
						(	wView_->wdLoop &&
						(	abs ( x - int((  wView_->wdLoopS-wView_->diStart )			* dispRatio ))  < 10		||
							abs ( x - int((  wView_->wdLoopE-wView_->diStart )			* dispRatio ))  < 10) )
					)
					#ifdef __unix__
					XDefineCursor(NApp::system().dpy(),window()->win(),XCreateFontCursor(NApp::system().dpy(),XC_right_side));
                    #else	
                    ;
                    #endif				
				else
				    #ifdef __unix__
					XDefineCursor(NApp::system().dpy(),window()->win(),XCreateFontCursor(NApp::system().dpy(),XC_xterm));
					#else
					;
                    #endif					
			}
			else if (wView_->wdLength!=0)					//mouse is over header
			{
				
				float dispRatio = nWidth/(float)wView_->wdLength;
				if (		wView_->blSelection		&&
						(	abs ( x - int(  wView_-> blStart			* dispRatio ))	< 10 ||
							abs ( x - int((  wView_->blStart+wView_->blLength)	* dispRatio ))	< 10 )
					)
					#ifdef __unix__
					XDefineCursor(NApp::system().dpy(),window()->win(),XCreateFontCursor(NApp::system().dpy(),XC_right_side));
					#else
					;
					#endif
				else
					#ifdef __unix__
					XDefineCursor(NApp::system().dpy(),window()->win(),XCreateFontCursor(NApp::system().dpy(),XC_xterm));
                    #else
                    ;
                    #endif					
			}
		}
		
	}

	void WaveEdChildView::WavePanel::onMousePressed(int x, int y, int button)
	{
		if(button==1)	//left
		{
			if(wView_->blLength==0)
				wView_->blSelection=false;
			if(wView_->bSnapToZero)
			{
				if(wView_->blSelection)
				{
					long delta = wView_->blStart - wView_->FindNearestZero(wView_->blStart);
					wView_->blStart-=delta;
					wView_->blLength+=delta;
					wView_->blLength = wView_->FindNearestZero(wView_->blStart+wView_->blLength) - wView_->blStart;
				}
				wView_->cursorPos = wView_->FindNearestZero(wView_->cursorPos);
			}
//			ReleaseCapture();
			wView_->bDragLoopEnd = wView_->bDragLoopStart = false;
			wView_->bLButtonDown = false;
			repaint();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////		Audio processing functions


	void WaveEdChildView::onSelectionFadeIn(NButtonEvent* ev)
	{
		unsigned long startPoint = (blSelection? blStart: 0);
		unsigned long length = (blSelection? blLength+1: wdLength);
		if(wdWave)
		{
			//pParent->m_wndView.AddMacViewUndo();

			Fade(wdLeft+startPoint, length, 0, 1.0f);
			if(wdStereo)
				Fade(wdRight+startPoint, length, 0, 1.0f);

			RefreshDisplayData(true);
			this->repaint();
			pSong->IsInvalided(false);
		}
	}


	void WaveEdChildView::onSelectionFadeOut(NButtonEvent* ev)
	{
		unsigned long startPoint = (blSelection? blStart: 0);
		unsigned long length = (blSelection? blLength+1: wdLength);
		if(wdWave)
		{
			//pParent->m_wndView.AddMacViewUndo();

			Fade(wdLeft+startPoint, length, 1.0f, 0);
			if(wdStereo)
				Fade(wdRight+startPoint, length, 1.0f, 0);

			RefreshDisplayData(true);
			this->repaint();
			pSong->IsInvalided(false);
		}
	}

	void WaveEdChildView::onSelectionNormalize(NButtonEvent* ev) // (Fideloop's)
	{
		signed short maxL = 0, maxR = 0, absBuf;
		double ratio = 0;
		unsigned long c = 0;
		unsigned long startPoint = (blSelection? blStart: 0);
		unsigned long length = (blSelection? blLength+1: wdLength);

		if (wdWave)
		{
//			pParent->m_wndView.AddMacViewUndo();

			pSong->IsInvalided(true);
//			Sleep(LOCK_LATENCY);

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
			this->repaint();
			pSong->IsInvalided(false);
		}
	}

	void WaveEdChildView::onSelectionRemoveDC(NButtonEvent* ev) // (Fideloop's)
	{
		double meanL = 0, meanR = 0;
		unsigned long c = 0;
		unsigned long startPoint = (blSelection? blStart: 0);
		unsigned long length = (blSelection? blLength+1: wdLength);
		signed short buf;

		if (wdWave)
		{
//			pParent->m_wndView.AddMacViewUndo();

			pSong->IsInvalided(true);
//			Sleep(LOCK_LATENCY);

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
			pSong->IsInvalided(false);
			RefreshDisplayData(true);
			this->repaint();
		}
	}

	void WaveEdChildView::onSelectionAmplify(NButtonEvent* ev)
	{
		double ratio =1;
		unsigned long startPoint = (blSelection? blStart: 0);
		unsigned long length = (blSelection? blLength+1: wdLength);
		int pos = 0;

		if (wdWave)
		{
//			pParent->m_wndView.AddMacViewUndo();
			if (AmpDlg->execute())
			{
				pos = AmpDlg->db_i;
				pSong->IsInvalided(true);
//				Sleep(LOCK_LATENCY);
				ratio = pow(10.0, (double) pos / (double) 2000.0);

				Amplify(wdLeft+startPoint, length, ratio);
				if (wdStereo)
					Amplify(wdRight+startPoint, length, ratio);

				pSong->IsInvalided(false);
				RefreshDisplayData(true);
				this->repaint();
			}
		}
	}

	void WaveEdChildView::onSelectionReverse(NButtonEvent* ev) 
	{
		short buf = 0;
		int c, halved = 0;
		unsigned long startPoint = (blSelection? blStart: 0);
		unsigned long length = (blSelection? blLength: wdLength-1);

		if (wdWave)
		{
//			pParent->m_wndView.AddMacViewUndo();

			pSong->IsInvalided(true);
//			Sleep(LOCK_LATENCY);

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
			this->repaint();
			pSong->IsInvalided(false);
		}
	}

	void WaveEdChildView::onSelectionInsertSilence(NButtonEvent* ev)
	{
		if(SilenceDlg->execute())
		{
			//todo!
			unsigned long timeInSamps = 44100 * SilenceDlg->timeInSecs;
			if(!wdWave)
			{
				pSong->WavAlloc(wsInstrument, false, timeInSamps, "New Waveform");
				short *pTmp= new signed short[timeInSamps];
				memset(pTmp, 0, timeInSamps*2 );
				wdLeft = zapArray(pSong->_pInstrument[wsInstrument]->waveDataL, pTmp);
				wdLength=timeInSamps;
				wdStereo=false;
				wdWave=true;
				onSelectionShowall((NButtonEvent*) 0);
			}
			else
			{
				unsigned long insertPos;
				switch(SilenceDlg->insertPos)
				{
				case WaveEdInsertSilenceDialog::at_start:
					insertPos = 0;
					break;
				case WaveEdInsertSilenceDialog::at_end:
					insertPos = wdLength-1;
					break;
				case WaveEdInsertSilenceDialog::at_cursor:
					insertPos = cursorPos;
					break;
				default:
					throw;
				}

				short *pTmp = new signed short[timeInSamps + wdLength];		//create new buffer
				memcpy(pTmp, wdLeft, insertPos*2);							//copy pre-insert data
				memset(pTmp + insertPos, 0, timeInSamps*2);					//insert silence
				memcpy((unsigned char*)pTmp + 2*(insertPos+timeInSamps), wdLeft + insertPos, 2*(wdLength - insertPos));	//copy post-insert data
				wdLeft = zapArray(pSong->_pInstrument[wsInstrument]->waveDataL,pTmp);

				if(wdStereo)
				{
					short *pTmpR= new signed short[timeInSamps + wdLength];
					memcpy(pTmpR,wdRight,insertPos*2);
					memset(pTmpR+insertPos, 0, timeInSamps*2);
					memcpy((unsigned char*)pTmpR+ 2*(insertPos+timeInSamps), wdRight + insertPos, 2*(wdLength - insertPos));
					wdRight = zapArray(pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
				}

				pSong->_pInstrument[wsInstrument]->waveLength = wdLength = wdLength + timeInSamps;

				if(wdLoop)		//update loop points if necessary
				{
					if(insertPos<wdLoopS)
					{
						wdLoopS += timeInSamps;
						pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopS;
					}
					if(insertPos<wdLoopE)
					{
						wdLoopE += timeInSamps;
						pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopE;
					}
					pSong->IsInvalided(false);
//					pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
				}
			}


//			pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.
			RefreshDisplayData(true);
			this->repaint();
			pSong->IsInvalided(false);

		}
	}

	void WaveEdChildView::onConvertMono(NButtonEvent* ev) 
	{
		if (wdWave && wdStereo)
		{
//			pParent->m_wndView.AddMacViewUndo();

			pSong->IsInvalided(true);
//			Sleep(LOCK_LATENCY);

	//		SetUndo(4, wdLeft, wdRight, wdLength); 
			for (unsigned int c = 0; c < wdLength; c++)
			{
				*(wdLeft + c) = ( *(wdLeft + c) + *(wdRight + c) ) / 2;
			}

			pSong->_pInstrument[wsInstrument]->waveStereo = false;
			wdStereo = false;
			zapArray(pSong->_pInstrument[wsInstrument]->waveDataR);
			RefreshDisplayData(true);
			this->repaint();
			pSong->IsInvalided(false);
		}
	}



	//////////////////////////////////////////////////////////////////////////
	//////		Clipboard Functions

	void WaveEdChildView::onEditDelete(NButtonEvent* ev)
	{
		short* pTmp = 0, *pTmpR = 0;
		long datalen = 0;

		if (wdWave && blSelection)
		{
//			pParent->m_wndView.AddMacViewUndo();

			pSong->IsInvalided(true);
			unsigned long length = blLength+1;

			datalen = (wdLength - length);
			if (datalen)
			{
				pTmp = new signed short[datalen];
				
				if (wdStereo)
				{
					pTmpR= new signed short[datalen];
					memcpy(pTmpR, wdRight, blStart*sizeof(short));
					memcpy(pTmpR+blStart, wdRight+blStart+length, (wdLength-blStart-length)*sizeof(short) );
					zapArray(pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
					wdRight = pTmpR;
				}

				
				memcpy(pTmp, wdLeft, blStart*sizeof(short) );
				memcpy(pTmp+blStart, wdLeft+blStart+length, (wdLength-blStart-length)*sizeof(short) );
				zapArray(pSong->_pInstrument[wsInstrument]->waveDataL,pTmp);
				wdLeft = pTmp;
				pSong->_pInstrument[wsInstrument]->waveLength = datalen;
				wdLength = datalen;
				//	adjust loop points if necessary
				if(wdLoop)
				{
					if(blStart+length<wdLoopS)
					{
						wdLoopS -= length;
						pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopS;
					}
					if(blStart+length<wdLoopE)
					{
						wdLoopE -= length;
						pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopE;
					}
				}

			}
			else
			{
				pSong->DeleteLayer(wsInstrument);
				wdLength = 0;
				wdWave   = false;
			}
		
			//Validate display
			if ( (diStart + diLength) > wdLength )
			{
				long newlen = wdLength - diLength;

				if ( newlen < 0 )
					this->onSelectionShowall((NButtonEvent*) 0);
				else
					diStart = (unsigned)newlen;
			}
			
			blSelection = false;
			blLength  = 0;
			blStart   = 0;


//			pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.

			ResetScrollBars();
			RefreshDisplayData(true);
			UpdateStatusBar();
			this->repaint();
			pSong->IsInvalided(false);
		}
	}

	void WaveEdChildView::onEditCopy(NButtonEvent* ev) 
	{
		if(!wdWave || !blLength) return;
		cbLeft.resize(blLength+1);
		memcpy(&cbLeft[0], wdLeft+blStart, cbLeft.size()*sizeof cbLeft[0]);
		if(wdStereo)
		{
			cbRight.resize(blLength+1);
			memcpy(&cbRight[0], wdRight+blStart, cbRight.size()*sizeof cbRight[0]);
			cbStereo=true;
		}
		else
			cbStereo=false;

		}

	void WaveEdChildView::onEditCut(NButtonEvent* ev) 
	{
		onEditCopy((NButtonEvent*) 0);
		onEditDelete((NButtonEvent*) 0);
	}

	void WaveEdChildView::onEditCrop(NButtonEvent* ev)
	{
		unsigned long blStartTemp = blStart;
		
		blStart += blLength;
		blLength = (wdLength - blStart);
		onEditDelete((NButtonEvent*) 0);
		
		blSelection = true;
		blStart = 0;
		blLength = blStartTemp;
		onEditDelete((NButtonEvent*) 0);
	}

	void WaveEdChildView::onEditPaste(NButtonEvent* ev) 
	{
		unsigned long c = 0;
		signed short *pTmp, *pTmpR;
//		pParent->m_wndView.AddMacViewUndo();
		
		if(cbLeft.empty()) return;
		pSong->IsInvalided(true);

		if(!wdWave)
		{
			pSong->WavAlloc(wsInstrument, cbStereo? true: false, cbLeft.size(), "Clipboard");
			wdLength=cbLeft.size();
			wdLeft = pSong->_pInstrument[wsInstrument]->waveDataL;
			memcpy(wdLeft, &cbLeft[0], cbLeft.size() * sizeof cbLeft[0]);
			if(cbStereo)
			{
				wdRight = pSong->_pInstrument[wsInstrument]->waveDataR;
				memcpy(wdRight, &cbRight[0], cbRight.size() * sizeof cbRight[0]);
				wdStereo = true;
			}
			else
				wdStereo = false;
			wdWave = true;
			onSelectionShowall((NButtonEvent*) 0);
		}
		else	//existing data, insert it
		{
			if(cbStereo != wdStereo)  //todo: deal with this better... i.e. dialog box offering to convert clipboard data
				return;
			pTmp = new signed short[cbLeft.size() + wdLength];
			memcpy(pTmp, wdLeft, cursorPos* sizeof *wdLeft);
			memcpy(pTmp+cursorPos, &cbLeft[0], cbLeft.size()*sizeof cbLeft[0]);
			memcpy(pTmp+cursorPos+cbLeft.size(), wdLeft+cursorPos, (wdLength-cursorPos)*sizeof wdLeft[0]);
			wdLeft = zapArray(pSong->_pInstrument[wsInstrument]->waveDataL,pTmp);
			if(wdStereo)
			{
				pTmpR = new signed short[cbRight.size() + wdLength];
				memcpy(pTmpR, wdRight, cursorPos* sizeof *wdRight);
				memcpy(pTmpR+cursorPos, &cbRight[0], cbRight.size()*sizeof cbRight[0]);
				memcpy(pTmpR+cursorPos+cbRight.size(), wdRight+cursorPos, (wdLength-cursorPos)*sizeof wdRight[0]);
				wdRight = zapArray(pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
			}
			wdLength = pSong->_pInstrument[wsInstrument]->waveLength = wdLength + cbLeft.size();
			
			//	adjust loop points if necessary
			if(wdLoop)
			{
				if(cursorPos<wdLoopS)
				{
					wdLoopS += cbLeft.size();
					pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopS;
				}
				if(cursorPos<wdLoopE)
				{
					wdLoopE += cbLeft.size();
					pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopE;
				}
			}
		}


		ResetScrollBars();
		RefreshDisplayData(true);
		UpdateStatusBar();

//		pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.
		repaint();
		pSong->IsInvalided(false);
	}

	void WaveEdChildView::onPasteOverwrite(NButtonEvent* ev)
	{
//		pParent->m_wndView.AddMacViewUndo();
		long lDataSamps=cbLeft.size();
		long startPoint;
		
		if(cbLeft.empty()) return;
		if(cbStereo && cbRight.empty()) return;
		pSong->IsInvalided(true);


		if(cbStereo != wdStereo)  //todo: deal with this better... i.e. dialog box offering to convert clipboard data
			return;
			
		if(blSelection)	//overwrite selected block
		{
			//if clipboard data is longer than selection, truncate it
			if(lDataSamps>blLength+1)
				lDataSamps=blLength+1;
			startPoint=blStart;
		}
		else		//overwrite at cursor
		{
			//truncate to current wave size	(should we be extending in this case??)
			if(lDataSamps>(wdLength-cursorPos))
				lDataSamps=wdLength-cursorPos;
			startPoint=cursorPos;
		}
		memcpy(wdLeft+startPoint, &cbLeft[0], lDataSamps*sizeof cbLeft[0]);
		if(wdStereo)
		{
			memcpy(wdRight+startPoint, &cbRight[0], lDataSamps*sizeof cbRight[0]);
		}
		

		ResetScrollBars();
		RefreshDisplayData(true);
		UpdateStatusBar();

//		pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.
		repaint();
		pSong->IsInvalided(false);
	}

	void WaveEdChildView::onPasteMix(NButtonEvent* ev)
	{
		unsigned long c = 0;
		unsigned long startPoint;

		if(MixDlg->execute())
		{

//			pParent->m_wndView.AddMacViewUndo();

			char *pData;
			short* pTmp = 0, *pTmpR = 0;


			unsigned long lDataSamps = cbLeft.size();

			unsigned long fadeInSamps(0), fadeOutSamps(0);
			unsigned long destFadeIn(0);	

			if(MixDlg->bFadeIn) //todo!@
				fadeInSamps = 44100 * MixDlg->fadeInTime;
			if(MixDlg->bFadeOut) //todo!#
				fadeOutSamps= 44100 * MixDlg->fadeOutTime;

			pSong->IsInvalided(true);

			if (!cbLeft.empty() )
			{
				if ( cbStereo != wdStereo )		//todo: deal with this better.. i.e. dialog box offering to convert clipboard data
					return;

				if(blSelection)	//overwrite selected block
				{
					//if clipboard data is longer than selection, truncate it
					if(lDataSamps>blLength+1) 
					{
						cbLeft.resize(blLength);
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
				memcpy(pTmp+startPoint, &cbLeft[0], cbLeft.size() * sizeof cbLeft[0]);					//copy clipboard data into pTmp
				Fade(pTmp+startPoint, fadeInSamps, 0, MixDlg->srcVol);				//do fade in on clipboard data
				Fade(wdLeft+startPoint, destFadeIn, 1.0f, MixDlg->destVol);			//do fade in on wave data
				Amplify(pTmp +startPoint+fadeInSamps, lDataSamps-fadeInSamps-fadeOutSamps, MixDlg->srcVol);	//amplify non-faded part of clipboard data

				if(startPoint+lDataSamps < wdLength)
				{
					Amplify(wdLeft +startPoint+destFadeIn, lDataSamps-destFadeIn-fadeOutSamps, MixDlg->destVol); //amplify wave data
					Fade(wdLeft +startPoint+lDataSamps-fadeOutSamps, fadeOutSamps, MixDlg->destVol, 1.0f);	//fade out wave data
					Fade(pTmp   +startPoint+lDataSamps-fadeOutSamps, fadeOutSamps, MixDlg->srcVol, 0);		//fade out clipboard data
				}
				else	//ignore fade out in this case, it doesn't make sense here
					Amplify(wdLeft +startPoint+destFadeIn, wdLength-startPoint-destFadeIn, MixDlg->destVol);	//amplify wave data

				Mix(pTmp, wdLeft, newLength, wdLength);		//mix into pTmp
				wdLeft =zapArray(pSong->_pInstrument[wsInstrument]->waveDataL,pTmp);

				if( cbStereo )
				{
					pTmpR= new signed short[newLength];
					for ( c=0; c<newLength; c++ )	pTmpR[c]=0;
					memcpy(pTmpR+startPoint, &cbRight[0], cbRight.size() * sizeof cbRight[0]);

					Fade(pTmpR+startPoint, fadeInSamps, 0, MixDlg->srcVol);
					Fade(wdRight+startPoint, destFadeIn, 1.0f, MixDlg->destVol);
					Amplify(pTmpR+startPoint+fadeInSamps, lDataSamps-fadeInSamps-fadeOutSamps, MixDlg->srcVol);

					if(startPoint+lDataSamps < wdLength)
					{
						Amplify(wdRight+startPoint+destFadeIn, lDataSamps-destFadeIn-fadeOutSamps, MixDlg->destVol);
						Fade(wdRight+startPoint+lDataSamps-fadeOutSamps, fadeOutSamps, MixDlg->destVol, 1.0f);
						Fade(pTmpR+startPoint+lDataSamps-fadeOutSamps, fadeOutSamps, MixDlg->srcVol, 0);
					}
					else
						Amplify(wdRight+startPoint+destFadeIn, wdLength-startPoint-destFadeIn, MixDlg->destVol);

					Mix(pTmpR, wdRight, newLength, wdLength);		//mix into pTmpR
					wdRight=zapArray(pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
				}

				if(newLength>wdLength)
					pSong->_pInstrument[wsInstrument]->waveLength = wdLength = startPoint + lDataSamps;

			}

			ResetScrollBars();
			RefreshDisplayData(true);

//			pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.
			this->repaint();
			pSong->IsInvalided(false);
		}

	}



	void WaveEdChildView::onPasteCrossfade(NButtonEvent* ev)
	{
		if(XFadeDlg->execute())
		{
			unsigned long c = 0;
			unsigned long startPoint, endPoint;

//			pParent->m_wndView.AddMacViewUndo();

			short* pTmp = 0, *pTmpR = 0;

			unsigned long lDataSamps = cbLeft.size();

			pSong->IsInvalided(true);
//			Sleep(LOCK_LATENCY);

			if(!cbLeft.empty())
			{
				if ( cbStereo != wdStereo )
					return;						//todo: deal with this better.. i.e. dialog box offering to convert clipboard data

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
				memcpy(pTmp+startPoint, &cbLeft[0], lDataSamps * sizeof cbLeft[0]);						//copy clipboard into pTmp for processing
				Fade(pTmp +startPoint, endPoint-startPoint, XFadeDlg->srcStartVol, XFadeDlg->srcEndVol);			//fade clipboard data
				Fade(wdLeft +startPoint, endPoint-startPoint, XFadeDlg->destStartVol, XFadeDlg->destEndVol);		//fade wave data
				Mix(pTmp, wdLeft, newLength, wdLength);															//mix clipboard with wave
				wdLeft = zapArray(pSong->_pInstrument[wsInstrument]->waveDataL, pTmp);

				if(cbStereo)	//process right channel
				{
					pTmpR= new signed short[newLength];
					for( c=0; c<newLength; c++ ) pTmpR[c]=0;
					memcpy(pTmpR+startPoint, &cbRight[0], lDataSamps * sizeof cbLeft[0]);
					Fade(pTmpR+startPoint, endPoint-startPoint, XFadeDlg->srcStartVol, XFadeDlg->srcEndVol);
					Fade(wdRight+startPoint, endPoint-startPoint, XFadeDlg->destStartVol, XFadeDlg->destEndVol);
					Mix(pTmpR, wdRight, newLength, wdLength);
					wdRight = zapArray(pSong->_pInstrument[wsInstrument]->waveDataR,pTmpR);
				}
				if(newLength > wdLength)
					pSong->_pInstrument[wsInstrument]->waveLength = wdLength = newLength;
			}


			ResetScrollBars();
			RefreshDisplayData(true);

//			pParent->ChangeIns(wsInstrument); // This causes an update of the Instrument Editor.
			this->repaint();
			pSong->IsInvalided(false);
		}

	}



	void WaveEdChildView::onEditSelectAll(NButtonEvent* ev) 
	{
		diStart = 0;
		blStart = 0;
		diLength = wdLength;
		blLength = (wdLength>1? wdLength-2: 0);	//blStart+blLength+1 needs to point to a valid sample- wdLength is one too many
		blSelection = true;

		ResetScrollBars();
		RefreshDisplayData();

		this->repaint();
	}

	void WaveEdChildView::onEditSnapToZero(NButtonEvent* ev)
	{
		bSnapToZero=!bSnapToZero;
	}


	void WaveEdChildView::onPopupSetLoopStart(NButtonEvent* ev)
	{
//		pParent->m_wndView.AddMacViewUndo();
//		pSong->IsInvalided(true);
//		Sleep(LOCK_LATENCY);
		int nWidth = clientWidth();
		wdLoopS = diStart + rbX * diLength/nWidth;
		pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopS;
		if (pSong->_pInstrument[wsInstrument]->waveLoopEnd< wdLoopS )
		{
			pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopS;
		}
		wdLoopE = pSong->_pInstrument[wsInstrument]->waveLoopEnd;
		if (!wdLoop) 
		{
			wdLoop=true;
			pSong->_pInstrument[wsInstrument]->waveLoopType=true;
		}
		pSong->IsInvalided(false);

//		pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
//		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
//		InvalidateRect(&rect, false);
	}
	void WaveEdChildView::onPopupSetLoopEnd(NButtonEvent* ev)
	{
//		pParent->m_wndView.AddMacViewUndo();
		pSong->IsInvalided(true);
//		Sleep(LOCK_LATENCY);

		int nWidth = clientWidth();
		wdLoopE = diStart + rbX * diLength/nWidth;
		pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopE;
		if (pSong->_pInstrument[wsInstrument]->waveLoopStart> wdLoopE )
		{
			pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopE;
		}
		wdLoopS = pSong->_pInstrument[wsInstrument]->waveLoopStart;
		if (!wdLoop) 
		{
			wdLoop=true;
			pSong->_pInstrument[wsInstrument]->waveLoopType=true;
		}
		pSong->IsInvalided(false);
//		pParent->m_wndInst.WaveUpdate();// This causes an update of the Instrument Editor.
//		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
//		InvalidateRect(&rect, false);
	}

	void WaveEdChildView::onPopupSelectionToLoop(NButtonEvent* ev)
	{
		if(!blSelection) return;

		wdLoopS = blStart;
		wdLoopE = blStart+blLength;
		pSong->_pInstrument[wsInstrument]->waveLoopStart=wdLoopS;
		pSong->_pInstrument[wsInstrument]->waveLoopEnd=wdLoopE;
		if (!wdLoop) 
		{
			wdLoop=true;
			pSong->_pInstrument[wsInstrument]->waveLoopType=true;
		}

		pSong->IsInvalided(false);
//		pParent->m_wndInst.WaveUpdate();
//		CRect rect;
//		GetClientRect(&rect);
//		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
//		InvalidateRect(&rect, false);
	}


	unsigned long WaveEdChildView::GetWaveLength()
	{
		if(wdWave)
			return wdLength;
		else
			return 0;
	}
	bool WaveEdChildView::IsStereo()
	{
		return wdStereo;
	}
	unsigned long WaveEdChildView::GetSelectionLength()
	{
		if(wdWave && blSelection)
			return blLength;
		else
			return 0;
	}
	unsigned long WaveEdChildView::GetCursorPos()
	{
		if(wdWave)
			return cursorPos;
		else
			return 0;
	}
	void WaveEdChildView::SetCursorPos(unsigned long newpos)
	{
		if(newpos<0 || newpos>=wdLength)
			return;

		cursorPos = newpos;
		if(cursorPos < diLength) diStart=0;
		else if(cursorPos>wdLength-diLength) diStart=wdLength-diLength;
		else diStart = cursorPos-(diLength/2);

		ResetScrollBars();
		RefreshDisplayData();
		repaint();
	}

	void WaveEdChildView::ResetScrollBars(bool bNewLength)
	{
		if(wdWave)
		{
			//set horizontal scroll bar
//			if(bNewLength)
				scrollBar->setRange( 0, wdLength-diLength );

			//scrollBar->setPos(diStart);

			//set zoom slider
			if(bNewLength)
			{
				//here we're looking for the highest n where wdLength/(b^n)>8 , where b is zoomBase and n is the max value for the zoom slider.
				//the value of the slider determines a wdLength/diLength ratio of b^n.. so this way, diLength is limited to a minimum of 8 samples.
				//another alternative to consider would be to use a fixed range, and change the zoomBase based on the wavelength to match..

				float maxzoom = log10(float(wdLength/8.0f))/log10(zoomBase);	// wdLength/(b^n)>=8    <==>   n <= log<b>(wdLength/8)
				// log<10>(x)/log<10>(b) == log<b>(x)
				int slidermax = (int)(floor(maxzoom));
				if(slidermax<0) slidermax=0;			//possible for waves with less than 8 samples (!)
				zoomSlider->setRange(0, slidermax);
			}
			if(diLength!=0)
			{

				//this is the same concept, except this is to give us some idea of where to draw the slider based on the existing zoom
				//so, instead of wdLength/8 (the max zoom), we're doing wdLength/diLength (the current zoom)
				//float zoomfactor = log10(wdLength/(float)diLength)/log10(zoomBase);
				//int newpos = (int)(zoomfactor);
				//if(newpos<0)	newpos=0;		//i'm not sure how this would happen, but just in case
				//zoomSlider->setPos(newpos);
			}
		}
		else
		{
			//disabled scrollbar
			//scrollBar->setRange(0);
			//scrollBar->setPos(0);

			//disabled zoombar
			zoomSlider->setRange(0, 0);
			//zoomSlider->setPos(0);
		}

		//set volume slider
		volSlider->setRange(0, 100);
		volSlider->setPos( pSong->waved.GetVolume() * 100.0f);
	}

	void WaveEdChildView::RefreshDisplayData(bool bRefreshHeader)
	{
		if(wdWave)
		{
			int const nWidth = clientWidth();
			if(nWidth==0)return;
			int const nHeadHeight = clientHeight()/10;
			int const nHeight= clientHeight()-nHeadHeight;
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
//			lDisplay.clear();
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
//				lDisplay.push_back(std::pair<int, int>((wrHeight*yLow)/32768, (wrHeight*yHi)/32768));
			}
			if(wdStereo)
			{
				rDisplay.resize(nWidth);
//				rDisplay.clear();
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
//					rDisplay.push_back(std::pair<int, int>((wrHeight*yLow)/32768, (wrHeight*yHi)/32768));

				}
			}
			if(bRefreshHeader)
			{
				// left channel of header
				// todo: very low-volume samples tend to disappear.. we should round up instead of down
				lHeadDisplay.resize(nWidth);
//				lHeadDisplay.clear();
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
//					lHeadDisplay.push_back(std::pair<int, int>((wrHeadHeight*yLow)/32768, (wrHeadHeight*yHi)/32768));
				
				}
				if(wdStereo)
				{
					// right channel of header
					// todo: very low-volume samples tend to disappear.. we should round up instead of down
					rHeadDisplay.resize(nWidth);
//					rHeadDisplay.clear();
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
//						rHeadDisplay.push_back(std::pair<int, int>((wrHeadHeight*yLow)/32768, (wrHeadHeight*yHi)/32768));

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



	////////////////////////////////////////////
	////		FindNearestZero()
	////	searches for the zero crossing nearest to a given sample index.
	////  returns the sample index of the nearest zero, or, in the event that the nearest zero crossing never actually hits zero,
	////	it will return the index of the sample that comes the closest to zero.  if the index is out of range, the last sample
	////	index is returned.  the first and last sample of the wave are considered zero.
	unsigned long WaveEdChildView::FindNearestZero(unsigned long startpos)
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


	//Mix - mixes two audio buffers, possibly of different lengths.
	//mixed buffer will be put in the first buffer, so if the lengths are different, be sure that
	//the bigger one is the first argument.  passing a negative value for lhsVol and/or rhsVol will effectively
	//invert both/either buffer in addition to mixing.
	void WaveEdChildView::Mix(short* lhs, short *rhs, int lhsSize, int rhsSize, float lhsVol, float rhsVol)
	{
		if(rhsSize>lhsSize)
			return;
		if(lhsSize<=0 || rhsSize<0) return;
		for( int i(0); i<rhsSize; i++ )
		{
			int value = (short)(*(lhs+i) * lhsVol + *(rhs+i) * rhsVol);
			if(value>32767) value = 32767;
			else if(value<-32768) value = -32768;
			*(lhs+i) = static_cast<short>(value);
		}
		for( int i(rhsSize); i<lhsSize; ++i )
		{
			int value = (short)( *(lhs+i) * lhsVol );
			if(value>32767) value = 32767;
			else if(value<-32768) value = -32768;
			*(lhs+i) = static_cast<short>(value);
		}
	}

	//Fade - fades an audio buffer from one volume level to another.
	void WaveEdChildView::Fade(short* data, int length, float startVol, float endVol)
	{
		if(length<=0) return;
		float slope = (endVol-startVol)/(float)length;

		for(int i(0);i<length;++i)
		{
			int value = *(data+i) * (startVol+i*slope);
			if(value>32767) value=32767;
			else if(value<-32768) value = -32768;
			*(data+i) = static_cast<short>(value);
		}
	}

	//Amplify - multiplies an audio buffer by a given factor.  buffer can be inverted by passing
	//	a negative value for vol.
	void WaveEdChildView::Amplify(short* data, int length, float vol)
	{
		if(length<=0) return;
	
		int current;
		for(int i(0);i<length;++i)
		{
			current = (int)( *(data+i) * vol );
			if( current>32767 )		current=32767;
			else if( current<-32768 )	current=-32768;
			*(data+i) = static_cast<short>(current);
		}
	}

	WaveEdChildView::WavePanel::WavePanel(WaveEdChildView* wView )
	{
		wView_ = wView;
	}

	WaveEdChildView::WavePanel::~ WavePanel( )
	{
	}

	void WaveEdChildView::WavePanel::onMouseExit()
  {
		// reset mousearrow
		#ifdef __unix__
		XDefineCursor(NApp::system().dpy(),window()->win(),XCreateFontCursor(NApp::system().dpy(),XC_left_ptr));
        #endif		
  }

}}

