/***************************************************************************
  *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "instrumenteditor.h"
#include "global.h"
#include "song.h"
#include "envdialog.h"
#include <ngrs/nalignlayout.h>
#include <ngrs/nflowlayout.h>
#include <ngrs/nlabel.h>
#include <ngrs/nbevelborder.h>
#include <ngrs/nframeborder.h>
#include <ngrs/nbutton.h>
#include <ngrs/nedit.h>
#include <ngrs/ngroupbox.h>
#include <ngrs/ncombobox.h>
#include <ngrs/nitem.h>
#include <ngrs/nslider.h>
#include <ngrs/ncheckbox.h>
#include <ngrs/nitemevent.h>

namespace psycle {
namespace host {

InstrumentEditor::InstrumentEditor( Song * song )
  : ngrs::NWindow(), _pSong(song)
{
  init();
}


InstrumentEditor::~InstrumentEditor()
{
}

void InstrumentEditor::init( )
{
  setTitle("Instrument Editor");

  ngrs::NBevelBorder bvlBorder( ngrs::nNone, ngrs::nLowered );
  bvlBorder.setSpacing( ngrs::NSize( 2, 2, 2, 2 ) );

  ngrs::NPanel* header = new ngrs::NPanel();
    header->setHeight(20);
    header->setLayout( ngrs::NFlowLayout( ngrs::nAlLeft, 5, 5));
    header->add(new ngrs::NLabel("Instrument"), ngrs::nAlLeft);
    instNumberLbl = new ngrs::NLabel("   ");
	    ngrs::NBevelBorder bvl( ngrs::nNone, ngrs::nLowered );
		  bvl.setSpacing( ngrs::NSize(2,2,2,2) );
        instNumberLbl->setBorder( bvl );
    header->add( instNumberLbl, ngrs::nAlLeft );
    decInstBtn = new ngrs::NButton("<");
      decInstBtn->clicked.connect( this, &InstrumentEditor::onBtnPress );
      decInstBtn->setFlat( false );
    header->add( decInstBtn, ngrs::nAlLeft );
    incInstBtn = new ngrs::NButton( ">" );
      incInstBtn->setFlat( false );
      incInstBtn->clicked.connect( this, &InstrumentEditor::onBtnPress );
    header->add( incInstBtn, ngrs::nAlLeft );
    instNameEd = new ngrs::NEdit();
      instNameEd->setWidth( 100 );
      instNameEd->setHeight( instNameEd->preferredHeight() );
    header->add( instNameEd, ngrs::nAlLeft );
    killBtn = new ngrs::NButton( "Kill..." );
      killBtn->clicked.connect( this, &InstrumentEditor::onBtnPress );
      killBtn->setFlat( false );
    header->add( killBtn, ngrs::nAlLeft );
  pane()->add( header, ngrs::nAlTop );

  ngrs::NGroupBox* properties = new ngrs::NGroupBox();
      properties->setLayout( ngrs::NAlignLayout(5,5));
      properties->setHeaderText("Instrument Properties");
      ngrs::NPanel* noteActionPnl = new ngrs::NPanel();
        noteActionPnl->setLayout( ngrs::NFlowLayout( ngrs::nAlLeft, 5, 5) );
        noteActionPnl->add( new ngrs::NLabel("New Note Action") );
          newNoteActionCb = new ngrs::NComboBox();
          newNoteActionCb->setWidth(100);
          newNoteActionCb->setHeight(newNoteActionCb->preferredHeight());
          newNoteActionCb->add(new ngrs::NItem("Note Cut"));
          newNoteActionCb->add(new ngrs::NItem("Note Release"));
          newNoteActionCb->add(new ngrs::NItem("None"));
          newNoteActionCb->itemSelected.connect(this,&InstrumentEditor::onComboSelected);
        noteActionPnl->add( newNoteActionCb, ngrs::nAlTop );
      properties->add( noteActionPnl, ngrs::nAlTop );
      ngrs::NPanel* panningPnl = new ngrs::NPanel();
        panningPnl->setLayout( ngrs::NFlowLayout(ngrs::nAlLeft,5,5) );
        panningSlider = new ngrs::NSlider();
          panningSlider->setOrientation( ngrs::nHorizontal );
          panningSlider->setWidth( 150 );
          panningSlider->setHeight( 20 );
          panningSlider->setRange( 0, 256 );
          panningSlider->change.connect( this,&InstrumentEditor::onSliderMove );
        panningPnl->add( panningSlider, ngrs::nAlLeft );
        panningLbl = new ngrs::NLabel("   ");
            panningLbl->setBorder( bvlBorder );
        panningPnl->add( panningLbl, ngrs::nAlLeft );
      properties->add( panningPnl,ngrs::nAlTop );
      rndPanningCbx = new ngrs::NCheckBox("Random panning");
        rndPanningCbx->clicked.connect(this,&InstrumentEditor::onBtnPress);
      properties->add(rndPanningCbx,ngrs::nAlTop);
      rndVCFCutCbx  = new ngrs::NCheckBox("Random VCF Cutoff");
        rndVCFCutCbx->clicked.connect(this,&InstrumentEditor::onBtnPress);
      properties->add(rndVCFCutCbx,ngrs::nAlTop);
      rndVCFResoCbx = new ngrs::NCheckBox("Random VCF Reso/Bw.");
        rndVCFResoCbx->clicked.connect(this,&InstrumentEditor::onBtnPress);
      properties->add(rndVCFResoCbx,ngrs::nAlTop);

      ngrs::NGroupBox* tempoGrpBox = new ngrs::NGroupBox();
        tempoGrpBox->setLayout( ngrs::NFlowLayout(ngrs::nAlLeft,5,5) );
        tempoGrpBox->setHeaderText("Tempo Looping Tool");
        playSampleFitCbx = new ngrs::NCheckBox("Play sample to fit");
        tempoGrpBox->add(playSampleFitCbx);
        patRowEdt = new ngrs::NEdit();
            patRowEdt->setWidth(50);
            patRowEdt->setBorder( bvlBorder );
            patRowEdt->keyPress.connect(this,&InstrumentEditor::onPatRowEdit);
        tempoGrpBox->add(patRowEdt,ngrs::nAlLeft);
        tempoGrpBox->add(new ngrs::NLabel("Pattern rows"),ngrs::nAlLeft);
      properties->add(tempoGrpBox,ngrs::nAlTop);

      ngrs::NGroupBox* waveLayerGrpBox = new ngrs::NGroupBox();
        waveLayerGrpBox->setLayout( ngrs::NAlignLayout(5,5) );
        waveLayerGrpBox->setHeaderText("Instrument Wave Layer");
        ngrs::NPanel* volumePnl = new ngrs::NPanel();
        volumePnl->setLayout( ngrs::NFlowLayout(ngrs::nAlLeft,5,5));
        volumePnl->add(new ngrs::NLabel("Volume"),ngrs::nAlLeft);
        volumeSlider = new ngrs::NSlider();
          volumeSlider->setOrientation(ngrs::nHorizontal);
          volumeSlider->setWidth(150);
          volumeSlider->setHeight(20);
          volumeSlider->setRange(0,512);
          volumeSlider->change.connect(this,&InstrumentEditor::onSliderMove);
        volumePnl->add(volumeSlider,ngrs::nAlLeft);
        volumeLbl = new ngrs::NLabel("   ");
            volumeLbl->setBorder( bvlBorder );
        volumePnl->add(volumeLbl,ngrs::nAlLeft);
        waveLayerGrpBox->add(volumePnl,ngrs::nAlTop);
        ngrs::NPanel* fineTunePnl = new ngrs::NPanel();
        fineTunePnl->setLayout( ngrs::NFlowLayout(ngrs::nAlLeft,5,5) );
        fineTunePnl->add(new ngrs::NLabel("Finetune"), ngrs::nAlLeft );
        fineTuneSlider = new ngrs::NSlider();
          fineTuneSlider->setOrientation( ngrs::nHorizontal );
          fineTuneSlider->setWidth(150);
          fineTuneSlider->setHeight(20);
          fineTuneSlider->setRange(0,256);
          fineTuneSlider->change.connect(this,&InstrumentEditor::onSliderMove);
        fineTunePnl->add(fineTuneSlider,ngrs::nAlLeft);
        fineTuneLbl = new ngrs::NLabel("   ");
            fineTuneLbl->setBorder( bvlBorder );
        fineTunePnl->add( fineTuneLbl, ngrs::nAlLeft );
        waveLayerGrpBox->add(fineTunePnl,ngrs::nAlTop);
        ngrs::NPanel* tunePnl = new ngrs::NPanel();
            tunePnl->setLayout( ngrs::NFlowLayout(ngrs::nAlLeft,5,5));
            tunePnl->add(new ngrs::NLabel("Tune"));
            octDecBtn = new ngrs::NButton("Oct-");
              octDecBtn->setFlat(false);
              octDecBtn->clicked.connect(this,&InstrumentEditor::onBtnPress);
            tunePnl->add(octDecBtn);
            noteDecBtn = new ngrs::NButton("Note-");
              noteDecBtn->setFlat(false);
              noteDecBtn->clicked.connect(this,&InstrumentEditor::onBtnPress);
            tunePnl->add(noteDecBtn);
            noteIncBtn = new ngrs::NButton("Note+");
              noteIncBtn->setFlat(false);
              noteIncBtn->clicked.connect(this,&InstrumentEditor::onBtnPress);
            tunePnl->add(noteIncBtn);
            octIncBtn  = new ngrs::NButton("Oct+");
              octIncBtn->setFlat(false);
              octIncBtn->clicked.connect(this,&InstrumentEditor::onBtnPress);
            tunePnl->add(octIncBtn);
            octLbl   = new ngrs::NLabel("  ");
              octLbl->setBorder( bvlBorder );
            tunePnl->add( octLbl );
        waveLayerGrpBox->add( tunePnl, ngrs::nAlTop );
        ngrs::NPanel* loopPnl = new ngrs::NPanel();
            loopPnl->setLayout( ngrs::NFlowLayout(ngrs::nAlLeft,5,5));
            loopPnl->add(new ngrs::NLabel("Loop"));
            ngrs::NButton* offBtn = new ngrs::NButton("Off");
              offBtn->setFlat(false);
            loopPnl->add(offBtn);
            ngrs::NButton* forwardBtn = new ngrs::NButton("Forward");
              forwardBtn->setFlat(false);
            loopPnl->add(forwardBtn);
            loopLbl   = new ngrs::NLabel("  ");
              loopLbl->setBorder( bvlBorder );
            loopPnl->add(loopLbl);
        waveLayerGrpBox->add(loopPnl,ngrs::nAlTop);
        ngrs::NPanel* loopAtPnl = new ngrs::NPanel();
            loopAtPnl->setLayout( ngrs::NFlowLayout(ngrs::nAlLeft,5,5));
            loopAtPnl->add(new ngrs::NLabel("Loop At"),ngrs::nAlLeft);
            loopAtFromLbl = new ngrs::NLabel("  ");
              loopAtFromLbl->setBorder( bvlBorder );
            loopAtPnl->add(loopAtFromLbl,ngrs::nAlLeft);
            loopAtPnl->add(new ngrs::NLabel("to"),ngrs::nAlLeft);
            loopAtToLbl   = new ngrs::NLabel("  ");
              loopAtToLbl->setBorder( bvlBorder );
            loopAtPnl->add(loopAtToLbl,ngrs::nAlLeft);
            loopAtPnl->add(new ngrs::NLabel("Length"),ngrs::nAlLeft);
            lenLbl   = new ngrs::NLabel("  ");
              lenLbl->setBorder( bvlBorder );
            loopAtPnl->add( lenLbl, ngrs::nAlLeft );
        waveLayerGrpBox->add( loopAtPnl, ngrs::nAlTop );
        ngrs::NButton* amplitudeBtn = new ngrs::NButton("Amplitudes/Filter Envelopes");
          amplitudeBtn->setFlat(false);
          amplitudeBtn->clicked.connect(this,&InstrumentEditor::onShowEnvelopeEditor);
        waveLayerGrpBox->add(amplitudeBtn,ngrs::nAlTop);
      properties->add(waveLayerGrpBox,ngrs::nAlClient);

  pane()->add(properties,ngrs::nAlClient);

  setPosition(0,0,400,600);

  envelopeEditor = new EnvDialog( _pSong );
  add(envelopeEditor);
}

int InstrumentEditor::onClose( )
{
  setVisible(false);
  return ngrs::nHideWindow;
}

void InstrumentEditor::setInstrument( int index )
{
  _pSong->instSelected=   index;
  _pSong->auxcolSelected= index;

  char buf[64]; sprintf(buf,"%.2X",index);
  instNumberLbl->setText(buf);

  instNameEd->setText( _pSong->_pInstrument[index]->_sName);

  newNoteActionCb->setIndex( _pSong->_pInstrument[index]->_NNA);

  // Volume bar
  volumeSlider->setPos( _pSong->_pInstrument[index]->waveVolume);
  fineTuneSlider->setPos( _pSong->_pInstrument[index]->waveFinetune+256);

  panningSlider->setPos( _pSong->_pInstrument[index]->_pan);
  rndPanningCbx->setCheck( _pSong->_pInstrument[index]->_RPAN);
  rndVCFCutCbx->setCheck( _pSong->_pInstrument[index]->_RCUT);
  rndVCFResoCbx->setCheck( _pSong->_pInstrument[index]->_RRES);

  panningLbl->setText(stringify( _pSong->_pInstrument[index]->_pan));

  octLbl->setText(noteToString(( _pSong->_pInstrument[index]->waveTune+48)));

  bool const ils = _pSong->_pInstrument[index]->_loop;

  playSampleFitCbx->setCheck(ils);
  patRowEdt->setText(stringify( _pSong->_pInstrument[index]->_lines));

  // Set looptype
  if( _pSong->_pInstrument[index]->waveLoopType)
      loopLbl->setText("Forward");
  else
      loopLbl->setText("Off");

  // Display Loop Points & Wave Length	

  loopAtFromLbl->setText(stringify((int) _pSong->_pInstrument[index]->waveLoopStart));
  loopAtToLbl->setText(stringify((int) _pSong->_pInstrument[index]->waveLoopEnd));
  lenLbl->setText(stringify((int) _pSong->_pInstrument[index]->waveLength));

  fineTuneLbl->setText(stringify( _pSong->_pInstrument[index]->waveFinetune));
  volumeLbl->setText(stringify( _pSong->_pInstrument[index]->waveVolume)+"%");

  pane()->resize();

  envelopeEditor->InstChanged();

}

std::string InstrumentEditor::noteToString( int value )
{
  int octave = value / 12;

  switch (value % 12) {
      case 0:   return "C-" + stringify(octave); break;
      case 1:   return "C#" + stringify(octave); break;
      case 2:   return "D-" + stringify(octave); break;
      case 3:   return "D#" + stringify(octave); break;
      case 4:   return "E-" + stringify(octave); break;
      case 5:   return "F-" + stringify(octave); break;
      case 6:   return "F#" + stringify(octave); break;
      case 7:   return "G-" + stringify(octave); break;
      case 8:   return "G#" + stringify(octave); break;
      case 9:   return "A-" + stringify(octave); break;
      case 10:  return "A#" + stringify(octave); break;
      case 11:  return "B-" + stringify(octave); break;
  }
  return "err";
}

void InstrumentEditor::onBtnPress( ngrs::NButtonEvent * ev )
{
  if (ev->sender() == decInstBtn && instrumentIndex() > 0) {
      setInstrument(instrumentIndex()-1);
      pane()->repaint();
  } else
  if (ev->sender() == incInstBtn) {
      setInstrument(instrumentIndex()+1);
      pane()->resize();
      pane()->repaint();
  } else
  if (ev->sender() == killBtn) {
    _pSong->DeleteInstrument(instrumentIndex());
    setInstrument(instrumentIndex());
    pane()->resize();
    pane()->repaint();
  } else
  if (ev->sender() == rndPanningCbx) {
    _pSong->_pInstrument[instrumentIndex()]->_RPAN = rndPanningCbx->checked();
  } else
  if (ev->sender() == rndVCFCutCbx) {
    _pSong->_pInstrument[instrumentIndex()]->_RCUT = rndVCFCutCbx->checked();
  } else
  if (ev->sender() == rndVCFResoCbx) {
    _pSong->_pInstrument[instrumentIndex()]->_RRES = rndVCFResoCbx->checked();
  } else
  if (ev->sender() == octDecBtn) {
      if ( _pSong->_pInstrument[instrumentIndex()]->waveTune>-37)
        _pSong->_pInstrument[instrumentIndex()]->waveTune-=12;
      else _pSong->_pInstrument[instrumentIndex()]->waveTune=-48;
      octLbl->setText(noteToString(( _pSong->_pInstrument[instrumentIndex()]->waveTune+48)));
      octLbl->repaint();
  } else
  if (ev->sender() == octIncBtn) {
      if ( _pSong->_pInstrument[instrumentIndex()]->waveTune < 60)
        _pSong->_pInstrument[instrumentIndex()]->waveTune+=12;
      else
        _pSong->_pInstrument[instrumentIndex()]->waveTune=71;
      octLbl->setText(noteToString(( _pSong->_pInstrument[instrumentIndex()]->waveTune+48)));
      octLbl->repaint();
  } else
  if (ev->sender() == noteDecBtn) {
      if ( _pSong->_pInstrument[instrumentIndex()]->waveTune>-47)
        _pSong->_pInstrument[instrumentIndex()]->waveTune-=1;
      else _pSong->_pInstrument[instrumentIndex()]->waveTune=-48;
      octLbl->setText(noteToString(( _pSong->_pInstrument[instrumentIndex()]->waveTune+48)));
      octLbl->repaint();
  } else
  if (ev->sender() == noteIncBtn) {
    if ( _pSong->_pInstrument[instrumentIndex()]->waveTune < 71)
        _pSong->_pInstrument[instrumentIndex()]->waveTune+=1;
    else _pSong->_pInstrument[instrumentIndex()]->waveTune=71;
    octLbl->setText(noteToString(( _pSong->_pInstrument[instrumentIndex()]->waveTune+48)));
    octLbl->repaint();
  }
}

int InstrumentEditor::instrumentIndex( )
{
    return _pSong->instSelected;
}

void InstrumentEditor::onComboSelected( ngrs::NItemEvent * ev )
{
  if (newNoteActionCb->selIndex()!=-1)
    _pSong->_pInstrument[instrumentIndex()]->_NNA = newNoteActionCb->selIndex();
}

void InstrumentEditor::onSliderMove( ngrs::NSlider * sender )
{
  double pos = sender->pos();
  if (sender == panningSlider) {
    _pSong->_pInstrument[instrumentIndex()]->_pan = (int) pos;
    panningLbl->setText(stringify(_pSong->_pInstrument[instrumentIndex()]->_pan));
    panningLbl->repaint();
  } else
  if (sender == volumeSlider) {
    _pSong->_pInstrument[instrumentIndex()]->waveVolume = (int) pos;
    volumeLbl->setText(stringify(_pSong->_pInstrument[instrumentIndex()]->waveVolume)+"%");
    volumeLbl->repaint();
  } else
  if (sender == fineTuneSlider) {
    _pSong->_pInstrument[instrumentIndex()]->waveFinetune=(int) pos - 256;
    fineTuneLbl->setText(stringify(_pSong->_pInstrument[instrumentIndex()]->waveFinetune));
    fineTuneLbl->repaint();
  }
}

void InstrumentEditor::onPatRowEdit( const ngrs::NKeyEvent & event )
{
  _pSong->_pInstrument[instrumentIndex()]->_lines = ngrs::str<int>(patRowEdt->text());
}

void InstrumentEditor::onShowEnvelopeEditor( ngrs::NButtonEvent * ev )
{
  envelopeEditor->setVisible(true);
}

}
}
