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
#include "configuration.h"
#include "waveedframe.h"
#include "waveedchildview.h"
#include "defaultbitmaps.h"
#include <ngrs/nvisualcomponent.h>
#include <ngrs/nitem.h>
#include <ngrs/nmenubar.h>
#include <ngrs/nmenuitem.h>
#include <ngrs/nmenu.h>
#include <ngrs/ncheckmenuitem.h>
#include <ngrs/nmenuseperator.h>
#include <ngrs/ntoolbar.h>
#include <ngrs/nimage.h>

namespace psycle { namespace host {


	WaveEdFrame::WaveEdFrame()
	{
		wavView = new WaveEdChildView(Global::pSong());

//		setPosition(0,0,800,600);
//		setPositionToScreenCenter();
		this->InitMenus();
		this->InitToolBar();
		//this->setTitle("Wave Editor");
		pane()->add(wavView);
	}

	WaveEdFrame::~WaveEdFrame() throw()
	{
	}
	
/*	void WaveEdFrame::setVisible(bool on)
	{
		NWindow::setVisible(on);
		wavView->resize();
	}

	int WaveEdFrame::onClose()
	{
		setVisible(false);
		return nHideWindow;
	}*/
	
	void WaveEdFrame::InitMenus()
	{
		menuBar=new NMenuBar();
		pane()->add(menuBar);
		processMenu = new NMenu("Process");
		processMenu->add(new NMenuItem("Amplify Selection..."))->click.connect(wavView,&WaveEdChildView::onSelectionAmplify);
		processMenu->add(new NMenuItem("Fade In"))->click.connect(wavView,&WaveEdChildView::onSelectionFadeIn);
		processMenu->add(new NMenuItem("Fade Out"))->click.connect(wavView,&WaveEdChildView::onSelectionFadeOut);
		processMenu->add(new NMenuItem("InsertSilence..."))->click.connect(wavView,&WaveEdChildView::onSelectionInsertSilence);
		processMenu->add(new NMenuItem("Normalize"))->click.connect(wavView,&WaveEdChildView::onSelectionNormalize);
		processMenu->add(new NMenuItem("Remove DC"))->click.connect(wavView,&WaveEdChildView::onSelectionRemoveDC);
		processMenu->add(new NMenuItem("Reverse"))->click.connect(wavView,&WaveEdChildView::onSelectionReverse);
		editMenu = new NMenu("Edit");
		editMenu->add(new NMenuItem("Select All"))->click.connect(wavView,&WaveEdChildView::onEditSelectAll);
		editMenu->add(new NMenuSeperator());
		editMenu->add(new NMenuItem("Cut"))->click.connect(wavView,&WaveEdChildView::onEditCut);
		editMenu->add(new NMenuItem("Crop"))->click.connect(wavView,&WaveEdChildView::onEditCrop);
		editMenu->add(new NMenuItem("Copy"))->click.connect(wavView,&WaveEdChildView::onEditCopy);
		editMenu->add(new NMenuItem("Paste->Insert"))->click.connect(wavView,&WaveEdChildView::onEditPaste);
		editMenu->add(new NMenuItem("Paste->Overwrite"))->click.connect(wavView,&WaveEdChildView::onPasteOverwrite);
		editMenu->add(new NMenuItem("Paste->Mix..."))->click.connect(wavView,&WaveEdChildView::onPasteMix);
		editMenu->add(new NMenuItem("Paste->Crossfade..."))->click.connect(wavView,&WaveEdChildView::onPasteCrossfade);
		editMenu->add(new NMenuItem("Delete"))->click.connect(wavView,&WaveEdChildView::onEditDelete);
		editMenu->add(new NMenuSeperator());
		NCheckMenuItem *snapToZero = new NCheckMenuItem("Snap to Zero");
	//	snapToZero->setCheck(false); 
		editMenu->add(snapToZero)->click.connect(wavView,&WaveEdChildView::onEditSnapToZero);
		viewMenu = new NMenu("View");
		viewMenu->add(new NMenuItem("Zoom In"))->click.connect(wavView, &WaveEdChildView::onSelectionZoomIn);
		viewMenu->add(new NMenuItem("Zoom Out"))->click.connect(wavView, &WaveEdChildView::onSelectionZoomOut);
		viewMenu->add(new NMenuItem("Zoom Selection"))->click.connect(wavView, &WaveEdChildView::onSelectionZoomSel);
		viewMenu->add(new NMenuItem("Show All"))->click.connect(wavView, &WaveEdChildView::onSelectionShowall);
		convertMenu = new NMenu("Convert");
		convertMenu->add(new NMenuItem("Convert to Mono"))->click.connect(wavView, &WaveEdChildView::onConvertMono);
		menuBar->add(processMenu);
		menuBar->add(editMenu);
		menuBar->add(viewMenu);
		menuBar->add(convertMenu);
	}

	void WaveEdFrame::InitToolBar( )
	{
	DefaultBitmaps & icons = Global::pConfig()->icons();
	
	toolBar = new NToolBar();
	pane()->add(toolBar, nAlTop);
	NImage* img;

	img = new NImage();
	img->setSharedBitmap(&icons.playstart_flat());
	img->setPreferredSize(25,25);
	NButton* newBtn = new NButton(img);
	newBtn->setHint("Play from Start");
	toolBar->add(newBtn)->clicked.connect(this,&WaveEdFrame::onPlayFromStart);

	img = new NImage();
	img->setSharedBitmap(&icons.play_flat());
	img->setPreferredSize(25,25);
	newBtn = new NButton(img);
	newBtn->setHint("Play from Cursor");
	toolBar->add(newBtn)->clicked.connect(this,&WaveEdFrame::onPlay);

	img = new NImage();
	img->setSharedBitmap(&icons.release());
	img->setPreferredSize(25,25);
	newBtn = new NButton(img);
	newBtn->setHint("Release");
	toolBar->add(newBtn)->clicked.connect(this,&WaveEdFrame::onRelease);

	img = new NImage();
	img->setSharedBitmap(&icons.stop_flat());
	img->setPreferredSize(25,25);
	newBtn = new NButton(img);
	newBtn->setHint("Stop Playback");
	toolBar->add(newBtn)->clicked.connect(this,&WaveEdFrame::onStop);

	img = new NImage();
	img->setSharedBitmap(&icons.rwnd());
	img->setPreferredSize(25,25);
	newBtn = new NButton(img);
	newBtn->setHint("Set Cursor to Wave Start");
	toolBar->add(newBtn)->clicked.connect(this,&WaveEdFrame::onRewind);

	img = new NImage();
	img->setSharedBitmap(&icons.ffwd());
	img->setPreferredSize(25,25);
	newBtn = new NButton(img);
	newBtn->setHint("Set Cursor to Wave End");
	toolBar->add(newBtn)->clicked.connect(this,&WaveEdFrame::onFastForward);

	toolBar->resize();
}


	void WaveEdFrame::Notify(void)
	{
		wsInstrument = Global::pSong()->instSelected;
		wavView->SetViewData(wsInstrument);
	}
	

	void WaveEdFrame::onPlay(NButtonEvent *ev) {PlayFrom(wavView->GetCursorPos());}
	void WaveEdFrame::onPlayFromStart(NButtonEvent *ev) {PlayFrom(0);}
	void WaveEdFrame::onRelease(NButtonEvent *ev)
	{
		Global::pSong()->waved.Release();
	}
	void WaveEdFrame::onStop(NButtonEvent *ev)
	{
		Stop();
	}

	void WaveEdFrame::PlayFrom(unsigned long startPos)
	{
		if( startPos<0 || startPos >= Global::pSong()->_pInstrument[wsInstrument]->waveLength )
			return;		//this also protects against playing non-existent instruments

		Stop();

		Global::pSong()->waved.SetInstrument( Global::pSong()->_pInstrument[wsInstrument] );
		Global::pSong()->waved.Play(startPos);
	}

	void WaveEdFrame::Stop()
	{
		Global::pSong()->waved.Stop();
	}
	void WaveEdFrame::onFastForward(NButtonEvent *ev)
	{
		wavView->SetCursorPos( wavView->GetWaveLength()-1 );
	}
	void WaveEdFrame::onRewind(NButtonEvent *ev)
	{
		wavView->SetCursorPos( 0 );
	}
}}


