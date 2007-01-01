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
#include "riff.h"
#include "configuration.h"
#include "waveedframe.h"
#include "waveedchildview.h"
#include "defaultbitmaps.h"
#include "instrumenteditor.h"
#include "global.h"
#include <iomanip>
#include <iostream>
#include <ngrs/nvisualcomponent.h>
#include <ngrs/nitem.h>
#include <ngrs/nitemevent.h>
#include <ngrs/nmenubar.h>
#include <ngrs/nmenuitem.h>
#include <ngrs/nmenu.h>
#include <ngrs/ncheckmenuitem.h>
#include <ngrs/nmenuseperator.h>
#include <ngrs/ntoolbar.h>
#include <ngrs/nimage.h>
#include <ngrs/ncombobox.h>
#include <ngrs/nfiledialog.h>
#include <ngrs/nlabel.h>
#include <ngrs/ntoolbarseparator.h>
#include <ngrs/napp.h>
#include <iostream>

template<class T> inline T str_hex(const std::string &  value) {
   T result;

   std::stringstream str;
   str << value;
   str >> std::hex >> result;

   return result;
}

namespace psycle { namespace host {


	WaveEdFrame::WaveEdFrame( Song* song )
	{
		_pSong = song;
		wavView = new WaveEdChildView( song );

//		setPosition(0,0,800,600);
//		setPositionToScreenCenter();
		this->InitMenus();
		this->InitToolBar();
		//this->setTitle("Wave Editor");
		pane()->add(wavView);
		wavSaveFileDlg = new NFileDialog();
			wavSaveFileDlg->setMode(nSave);
		add(wavSaveFileDlg);
		// creates the instrument editor for editing samples
  	add( instrumentEditor = new InstrumentEditor(song) );
	}

	WaveEdFrame::~WaveEdFrame() throw()
	{
	}

	Song * WaveEdFrame::pSong( )
	{
		return _pSong;
	}
		
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
		NMenuItem* pasteItem = new NMenuItem("Paste");
		editMenu->add(pasteItem);
		NMenu* subEditMenu = new NMenu();
		pasteItem->add(subEditMenu);
		subEditMenu->add(new NMenuItem("Insert"))->click.connect(wavView,&WaveEdChildView::onEditPaste);
		subEditMenu->add(new NMenuItem("Overwrite"))->click.connect(wavView,&WaveEdChildView::onPasteOverwrite);
		subEditMenu->add(new NMenuItem("Mix..."))->click.connect(wavView,&WaveEdChildView::onPasteMix);
		subEditMenu->add(new NMenuItem("Crossfade..."))->click.connect(wavView,&WaveEdChildView::onPasteCrossfade);
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
	img->setSharedBitmap(&icons.playstart() );// play_flat());
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
	toolBar->add(new NToolBarSeparator());
		/*auxSelectCombo_ = new NComboBox();
	/*	auxSelectCombo_->setWidth(70);
		auxSelectCombo_->setHeight(20);
		auxSelectCombo_->add(new NItem("Wave"));
		auxSelectCombo_->setIndex(0);
	toolBar->add(auxSelectCombo_);*/
	

	img = new NImage();
		img->setSharedBitmap(&icons.littleleft());
		img->setPreferredSize(25,25);
	toolBar->add(new NButton(img))->clicked.connect(this,&WaveEdFrame::onDecInsBtn);

	img = new NImage();
		img->setSharedBitmap(&icons.littleright());
		img->setPreferredSize(25,25);
	toolBar->add(new NButton(img))->clicked.connect(this,&WaveEdFrame::onIncInsBtn);

	toolBar->add(new NButton("Load"))->clicked.connect(this,&WaveEdFrame::onLoadWave);
	toolBar->add(new NButton("Save"))->clicked.connect(this,&WaveEdFrame::onSaveWave);
	toolBar->add(new NButton("Edit"))->clicked.connect(this,&WaveEdFrame::onEditInstrument);
	toolBar->add(new NButton("Wave Ed"))->clicked.connect(this,&WaveEdFrame::onEditWave);

  toolBar->add(new NButton("Copy"))->clicked.connect(this,&WaveEdFrame::onSlotCopy);
  toolBar->add(new NButton("Paste"))->clicked.connect(this,&WaveEdFrame::onSlotPaste);
	
}


	void WaveEdFrame::Notify(void)
	{
		wsInstrument = pSong()->instSelected;
		wavView->SetViewData(wsInstrument);
    //updateComboIns( true );
	}
	

	void WaveEdFrame::onPlay(NButtonEvent *ev) {PlayFrom(wavView->GetCursorPos());}
	void WaveEdFrame::onPlayFromStart(NButtonEvent *ev) {PlayFrom(0);}
	void WaveEdFrame::onRelease(NButtonEvent *ev)
	{
		pSong()->waved.Release();
	}
	void WaveEdFrame::onStop(NButtonEvent *ev)
	{
		Stop();
	}

	void WaveEdFrame::PlayFrom(unsigned long startPos)
	{
		if( startPos<0 || startPos >= pSong()->_pInstrument[wsInstrument]->waveLength )
			return;		//this also protects against playing non-existent instruments

		Stop();

		pSong()->waved.SetInstrument( pSong()->_pInstrument[wsInstrument] );
		pSong()->waved.Play(startPos);
	}

	void WaveEdFrame::Stop()
	{
		pSong()->waved.Stop();
	}
	void WaveEdFrame::onFastForward(NButtonEvent *ev)
	{
		wavView->SetCursorPos( wavView->GetWaveLength()-1 );
	}
	void WaveEdFrame::onRewind(NButtonEvent *ev)
	{
		wavView->SetCursorPos( 0 );
	}

	void WaveEdFrame::onLoadWave( NButtonEvent * ev )
	{
		NFileDialog* dialog = new NFileDialog();
		add(dialog);

		dialog->addFilter("Wav Files(*.wav)","!S*.wav");

		if (dialog->execute()) {
			int si = pSong()->instSelected;
			//added by sampler
			if ( pSong()->_pInstrument[si]->waveLength != 0)
			{
        //if (MessageBox("Overwrite current sample on the slot?","A sample is already loaded here",MB_YESNO) == IDNO)  return;
			}

		if (pSong()->WavAlloc(si,dialog->fileName().c_str()))
		{
			Notify();
			updateInstrumentCbx( pSong()->instSelected, true );
		}
	}
	NApp::addRemovePipe(dialog);
}

	void WaveEdFrame::onSaveWave( NButtonEvent * ev )
	{
		WaveFile output;
		Song* _pSong = pSong();

		if (_pSong->_pInstrument[_pSong->instSelected]->waveLength)
		{
			if ( wavSaveFileDlg->execute() )
			{
				output.OpenForWrite(wavSaveFileDlg->fileName().c_str(), 44100, 16, (_pSong->_pInstrument[_pSong->instSelected]->waveStereo) ? (2) : (1) );
				if (_pSong->_pInstrument[_pSong->instSelected]->waveStereo)
				{
					for ( unsigned int c=0; c < _pSong->_pInstrument[_pSong->instSelected]->waveLength; c++)
					{
						output.WriteStereoSample( *(_pSong->_pInstrument[_pSong->instSelected]->waveDataL + c), *(_pSong->_pInstrument[_pSong->instSelected]->waveDataR + c) );
					}
				}
			else {
				output.WriteData(_pSong->_pInstrument[_pSong->instSelected]->waveDataL, _pSong->_pInstrument[_pSong->instSelected]->waveLength);
			}
			output.Close();
			}
		}
   //else MessageBox("Nothing to save...\nSelect nonempty wave first.", "Error", MB_ICONERROR);
   //m_wndView.SetFocus();
}


void WaveEdFrame::onEditInstrument( NButtonEvent * ev )
{
		instrumentEditor->setInstrument( pSong()->instSelected);
		instrumentEditor->setVisible(true);
}

	void WaveEdFrame::onEditWave( NButtonEvent * ev)
	{
		Notify();
	}

	void WaveEdFrame::onDecInsBtn( NButtonEvent * ev )
	{
		int index = pSong()->instSelected -1;
		if (index >=0 ) {
			pSong()->instSelected=   index;
			pSong()->auxcolSelected= index;
			Notify();

			updateInstrumentCbx.emit( index, false );		
		}
	}

	void WaveEdFrame::onIncInsBtn( NButtonEvent * ev )
	{
		int index = pSong()->instSelected +1;
		if (index <= 255) {
			pSong()->instSelected=   index;
			pSong()->auxcolSelected= index;
			Notify();

			updateInstrumentCbx.emit( index, false );
		}
	}


void WaveEdFrame::onSlotCopy( NButtonEvent * ev )
{
  if ( pSong()->_pInstrument[ pSong()->instSelected ] ) {
		std::string xml = "<instsel>";
		xml+= pSong()->_pInstrument[ pSong()->instSelected ]->toXml();
		xml+= "</instsel>";		
		NApp::system().clipBoard().setAsText( xml );
  }
}

void WaveEdFrame::onSlotPaste( NButtonEvent * ev )
{
	if ( NApp::system().clipBoard().asText() != "" ) {
    NXmlParser parser;
		mySel_ = false;
    parser.tagParse.connect(this, &WaveEdFrame::onTagParse);
    parser.parseString( NApp::system().clipBoard().asText() );		

		if (mySel_ && (pDataLeft!=0 || pDataRight!=0) ) {
			Instrument* inst = pSong()->_pInstrument[ pSong()->instSelected ];
			if (inst) {
				inst->setCompressedData(  pDataLeft, pDataRight);
				delete pDataLeft;
				delete pDataRight;
			}	
			updateInstrumentCbx.emit( pSong()->instSelected , true );
			Notify();
			repaint();
		}

  }
	
}

void WaveEdFrame::onTagParse( const NXmlParser & parser, const std::string & tagName ) {
	if (tagName == "instsel") {
		mySel_ = true;
		onWaveLeft = true;
		pDataLeft = 0;
		pDataRight = 0;
		if (pSong()->_pInstrument[ pSong()->instSelected ]) {
			delete pSong()->_pInstrument[ pSong()->instSelected ];
		}
		pSong()->_pInstrument[ pSong()->instSelected ] = new Instrument();				
	} else
	if ( tagName == "instrument" ) {
		std::string name = parser.getAttribValue("name");
		Instrument* inst = pSong()->_pInstrument[ pSong()->instSelected ];
		inst->setName( name );
	} else
  if (mySel_) {
		Instrument* inst = pSong()->_pInstrument[ pSong()->instSelected ];

		if ( tagName == "header" ) {
			std::string bin  = parser.getAttribValue("bin");
			inst->createHeader( bin );			
		} else
		if ( tagName == "wave" ) {
			std::string name = parser.getAttribValue("name");
			std::string bin  = parser.getAttribValue("bin");
			inst->createWavHeader( name, bin);
		} else 
		if ( tagName == "waveleft" ) {
			onWaveLeft = true;
			data_pos = 0;
			leftSize = str_hex<int>( parser.getAttribValue("size") );
			pDataLeft = new std::uint8_t[leftSize+4];
		} else
		if ( tagName == "waveright" ) {
			onWaveLeft = false;
			data_pos = 0;
			rightSize = str_hex<int>( parser.getAttribValue("size") );
			pDataRight = new std::uint8_t[rightSize+4];
		} else 
		if ( tagName == "hex" ) {
			if (onWaveLeft) {
				inst->getData( pDataLeft, parser.getAttribValue("v"));
			} else {
				inst->getData( pDataRight, parser.getAttribValue("v"));
			}
		}
	}
}

}}
