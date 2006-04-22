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
#include <nalignlayout.h>
#include <nflowlayout.h>
#include <nlabel.h>
#include <nbevelborder.h>
#include <nframeborder.h>
#include <nbutton.h>
#include <nedit.h>
#include <ngroupbox.h>
#include <ncombobox.h>
#include <nitem.h>
#include <nslider.h>
#include <ncheckbox.h>
#include <nitemevent.h>

InstrumentEditor::InstrumentEditor()
 : NWindow()
{
  init();
}


InstrumentEditor::~InstrumentEditor()
{
}

void InstrumentEditor::init( )
{
  setTitle("Instrument Editor");

  NPanel* header = new NPanel();
    header->setHeight(20);
    header->setLayout(new NFlowLayout(nAlLeft,5,5),true);
    header->add(new NLabel("Instrument"), nAlLeft);
    instNumberLbl = new NLabel("   ");
       instNumberLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
       instNumberLbl->border()->setSpacing(NSize(2,2,2,2));
    header->add(instNumberLbl,nAlLeft);
    decInstBtn = new NButton("<");
      decInstBtn->clicked.connect(this,&InstrumentEditor::onBtnPress);
      decInstBtn->setFlat(false);
    header->add(decInstBtn,nAlLeft);
    incInstBtn = new NButton(">");
      incInstBtn->setFlat(false);
      incInstBtn->clicked.connect(this,&InstrumentEditor::onBtnPress);
    header->add(incInstBtn,nAlLeft);
    instNameEd = new NEdit();
      instNameEd->setWidth(100);
      instNameEd->setHeight(instNameEd->preferredHeight());
    header->add(instNameEd,nAlLeft);
    killBtn = new NButton("Kill...");
      killBtn->clicked.connect(this,&InstrumentEditor::onBtnPress);
      killBtn->setFlat(false);
    header->add(killBtn, nAlLeft);
  pane()->add(header, nAlTop);

  NGroupBox* properties = new NGroupBox();
     properties->setLayout(new NAlignLayout(5,5),true);
     properties->setHeaderText("Instrument Properties");
     NPanel* noteActionPnl = new NPanel();
        noteActionPnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
        noteActionPnl->add(new NLabel("New Note Action"));
          newNoteActionCb = new NComboBox();
          newNoteActionCb->setWidth(100);
          newNoteActionCb->setHeight(newNoteActionCb->preferredHeight());
          newNoteActionCb->add(new NItem("Note Cut"));
          newNoteActionCb->add(new NItem("Note Release"));
          newNoteActionCb->add(new NItem("None"));
          newNoteActionCb->itemSelected.connect(this,&InstrumentEditor::onComboSelected);
        noteActionPnl->add(newNoteActionCb,nAlTop);
     properties->add(noteActionPnl,nAlTop);
     NPanel* panningPnl = new NPanel();
        panningPnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
        panningSlider = new NSlider();
          panningSlider->setOrientation(nHorizontal);
          panningSlider->setWidth(150);
          panningSlider->setHeight(20);
          panningSlider->setRange(0,256);
          panningSlider->posChanged.connect(this,&InstrumentEditor::onSliderMove);
        panningPnl->add(panningSlider,nAlLeft);
        panningLbl = new NLabel("   ");
           panningLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
           panningLbl->border()->setSpacing(NSize(2,2,2,2));
        panningPnl->add(panningLbl,nAlLeft);
     properties->add(panningPnl,nAlTop);
     rndPanningCbx = new NCheckBox("Random panning");
        rndPanningCbx->clicked.connect(this,&InstrumentEditor::onBtnPress);
     properties->add(rndPanningCbx,nAlTop);
     rndVCFCutCbx  = new NCheckBox("Random VCF Cutoff");
        rndVCFCutCbx->clicked.connect(this,&InstrumentEditor::onBtnPress);
     properties->add(rndVCFCutCbx,nAlTop);
     rndVCFResoCbx = new NCheckBox("Random VCF Reso/Bw.");
        rndVCFResoCbx->clicked.connect(this,&InstrumentEditor::onBtnPress);
     properties->add(rndVCFResoCbx,nAlTop);

     NGroupBox* tempoGrpBox = new NGroupBox();
        tempoGrpBox->setLayout(new NFlowLayout(nAlLeft,5,5),true);
        tempoGrpBox->setHeaderText("Tempo Looping Tool");
        playSampleFitCbx = new NCheckBox("Play sample to fit");
        tempoGrpBox->add(playSampleFitCbx);
        patRowLbl = new NLabel("   ");
           patRowLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
           patRowLbl->border()->setSpacing(NSize(2,2,2,2));
        tempoGrpBox->add(patRowLbl,nAlLeft);
        tempoGrpBox->add(new NLabel("Pattern rows"),nAlLeft);
     properties->add(tempoGrpBox,nAlTop);

     NGroupBox* waveLayerGrpBox = new NGroupBox();
        waveLayerGrpBox->setLayout(new NAlignLayout(5,5),true);
        waveLayerGrpBox->setHeaderText("Instrument Wave Layer");
        NPanel* volumePnl = new NPanel();
        volumePnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
        volumePnl->add(new NLabel("Volume"),nAlLeft);
        volumeSlider = new NSlider();
          volumeSlider->setOrientation(nHorizontal);
          volumeSlider->setWidth(150);
          volumeSlider->setHeight(20);
          volumeSlider->setRange(0,512);
          volumeSlider->posChanged.connect(this,&InstrumentEditor::onSliderMove);
        volumePnl->add(volumeSlider,nAlLeft);
        volumeLbl = new NLabel("   ");
           volumeLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
           volumeLbl->border()->setSpacing(NSize(2,2,2,2));
        volumePnl->add(volumeLbl,nAlLeft);
        waveLayerGrpBox->add(volumePnl,nAlTop);
        NPanel* fineTunePnl = new NPanel();
        fineTunePnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
        fineTunePnl->add(new NLabel("Finetune"),nAlLeft);
        fineTuneSlider = new NSlider();
          fineTuneSlider->setOrientation(nHorizontal);
          fineTuneSlider->setWidth(150);
          fineTuneSlider->setHeight(20);
          fineTuneSlider->setRange(0,256);
          fineTuneSlider->posChanged.connect(this,&InstrumentEditor::onSliderMove);
        fineTunePnl->add(fineTuneSlider,nAlLeft);
        fineTuneLbl = new NLabel("   ");
           fineTuneLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
           fineTuneLbl->border()->setSpacing(NSize(2,2,2,2));
        fineTunePnl->add(fineTuneLbl,nAlLeft);
        waveLayerGrpBox->add(fineTunePnl,nAlTop);
        NPanel* tunePnl = new NPanel();
           tunePnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
           tunePnl->add(new NLabel("Tune"));
           octDecBtn = new NButton("Oct-");
             octDecBtn->setFlat(false);
             octDecBtn->clicked.connect(this,&InstrumentEditor::onBtnPress);
           tunePnl->add(octDecBtn);
           noteDecBtn = new NButton("Note-");
             noteDecBtn->setFlat(false);
             noteDecBtn->clicked.connect(this,&InstrumentEditor::onBtnPress);
           tunePnl->add(noteDecBtn);
           noteIncBtn = new NButton("Note+");
             noteIncBtn->setFlat(false);
             noteIncBtn->clicked.connect(this,&InstrumentEditor::onBtnPress);
           tunePnl->add(noteIncBtn);
           octIncBtn  = new NButton("Oct+");
             octIncBtn->setFlat(false);
             octIncBtn->clicked.connect(this,&InstrumentEditor::onBtnPress);
           tunePnl->add(octIncBtn);
           octLbl   = new NLabel("  ");
             octLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
             octLbl->border()->setSpacing(NSize(2,2,2,2));
           tunePnl->add(octLbl);
        waveLayerGrpBox->add(tunePnl,nAlTop);
        NPanel* loopPnl = new NPanel();
           loopPnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
           loopPnl->add(new NLabel("Loop"));
           NButton* offBtn = new NButton("Off");
             offBtn->setFlat(false);
           loopPnl->add(offBtn);
           NButton* forwardBtn = new NButton("Forward");
             forwardBtn->setFlat(false);
           loopPnl->add(forwardBtn);
           loopLbl   = new NLabel("  ");
             loopLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
             loopLbl->border()->setSpacing(NSize(2,2,2,2));
           loopPnl->add(loopLbl);
        waveLayerGrpBox->add(loopPnl,nAlTop);
        NPanel* loopAtPnl = new NPanel();
           loopAtPnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
           loopAtPnl->add(new NLabel("Loop At"),nAlLeft);
           loopAtFromLbl = new NLabel("  ");
             loopAtFromLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
             loopAtFromLbl->border()->setSpacing(NSize(2,2,2,2));
           loopAtPnl->add(loopAtFromLbl,nAlLeft);
           loopAtPnl->add(new NLabel("to"),nAlLeft);
           loopAtToLbl   = new NLabel("  ");
             loopAtToLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
             loopAtToLbl->border()->setSpacing(NSize(2,2,2,2));
           loopAtPnl->add(loopAtToLbl,nAlLeft);
           loopAtPnl->add(new NLabel("Length"),nAlLeft);
           lenLbl   = new NLabel("  ");
             lenLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
             lenLbl->border()->setSpacing(NSize(2,2,2,2));
           loopAtPnl->add(lenLbl,nAlLeft);
        waveLayerGrpBox->add(loopAtPnl,nAlTop);
        NButton* amplitudeBtn = new NButton("Amplitudes/Filter Envelopes");
          amplitudeBtn->setFlat(false);
        waveLayerGrpBox->add(amplitudeBtn,nAlTop);
     properties->add(waveLayerGrpBox,nAlClient);

  pane()->add(properties,nAlClient);

  setPosition(0,0,400,600);
}

int InstrumentEditor::onClose( )
{
  setVisible(false);
  return nHideWindow;
}

void InstrumentEditor::setInstrument( int index )
{
  Global::pSong()->instSelected=   index;
  Global::pSong()->auxcolSelected= index;

  char buf[64]; sprintf(buf,"%.2X",index);
  instNumberLbl->setText(buf);

  instNameEd->setText(Global::pSong()->_pInstrument[index]->_sName);

  newNoteActionCb->setIndex(Global::pSong()->_pInstrument[index]->_NNA);

  // Volume bar
  volumeSlider->setPos(Global::pSong()->_pInstrument[index]->waveVolume);
  fineTuneSlider->setPos(Global::pSong()->_pInstrument[index]->waveFinetune+256);

  panningSlider->setPos(Global::pSong()->_pInstrument[index]->_pan);
  rndPanningCbx->setCheck(Global::pSong()->_pInstrument[index]->_RPAN);
  rndVCFCutCbx->setCheck(Global::pSong()->_pInstrument[index]->_RCUT);
  rndVCFResoCbx->setCheck(Global::pSong()->_pInstrument[index]->_RRES);

  panningLbl->setText(stringify(Global::pSong()->_pInstrument[index]->_pan));

  octLbl->setText(noteToString((Global::pSong()->_pInstrument[index]->waveTune+48)));

  bool const ils = Global::pSong()->_pInstrument[index]->_loop;

  playSampleFitCbx->setCheck(ils);
  patRowLbl->setText(stringify(Global::pSong()->_pInstrument[index]->_lines));

  // Set looptype
  if(Global::pSong()->_pInstrument[index]->waveLoopType)
     loopLbl->setText("Forward");
  else
     loopLbl->setText("Off");

  // Display Loop Points & Wave Length	

  loopAtFromLbl->setText(stringify((int)Global::pSong()->_pInstrument[index]->waveLoopStart));
  loopAtToLbl->setText(stringify((int)Global::pSong()->_pInstrument[index]->waveLoopEnd));
  lenLbl->setText(stringify((int)Global::pSong()->_pInstrument[index]->waveLength));

  fineTuneLbl->setText(stringify(Global::pSong()->_pInstrument[index]->waveFinetune));
  volumeLbl->setText(stringify(Global::pSong()->_pInstrument[index]->waveVolume)+"%");

  pane()->resize();

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

void InstrumentEditor::onBtnPress( NButtonEvent * ev )
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
    Global::pSong()->DeleteInstrument(instrumentIndex());
    setInstrument(instrumentIndex());
    pane()->resize();
    pane()->repaint();
  } else
  if (ev->sender() == rndPanningCbx) {
    Global::pSong()->_pInstrument[instrumentIndex()]->_RPAN = rndPanningCbx->checked();
  } else
  if (ev->sender() == rndVCFCutCbx) {
    Global::pSong()->_pInstrument[instrumentIndex()]->_RCUT = rndVCFCutCbx->checked();
  } else
  if (ev->sender() == rndVCFResoCbx) {
    Global::pSong()->_pInstrument[instrumentIndex()]->_RRES = rndVCFResoCbx->checked();
  } else
  if (ev->sender() == octDecBtn) {
     if ( Global::pSong()->_pInstrument[instrumentIndex()]->waveTune>-37)
       Global::pSong()->_pInstrument[instrumentIndex()]->waveTune-=12;
     else Global::pSong()->_pInstrument[instrumentIndex()]->waveTune=-48;
     octLbl->setText(noteToString((Global::pSong()->_pInstrument[instrumentIndex()]->waveTune+48)));
     octLbl->repaint();
  } else
  if (ev->sender() == octIncBtn) {
     if ( Global::pSong()->_pInstrument[instrumentIndex()]->waveTune < 60)
       Global::pSong()->_pInstrument[instrumentIndex()]->waveTune+=12;
     else 
       Global::pSong()->_pInstrument[instrumentIndex()]->waveTune=71;
     octLbl->setText(noteToString((Global::pSong()->_pInstrument[instrumentIndex()]->waveTune+48)));
     octLbl->repaint();
  } else
  if (ev->sender() == noteDecBtn) {
     if ( Global::pSong()->_pInstrument[instrumentIndex()]->waveTune>-47)
        Global::pSong()->_pInstrument[instrumentIndex()]->waveTune-=1;
     else Global::pSong()->_pInstrument[instrumentIndex()]->waveTune=-48;
     octLbl->setText(noteToString((Global::pSong()->_pInstrument[instrumentIndex()]->waveTune+48)));
     octLbl->repaint();
  } else
  if (ev->sender() == noteIncBtn) {
    if ( Global::pSong()->_pInstrument[instrumentIndex()]->waveTune < 71)
       Global::pSong()->_pInstrument[instrumentIndex()]->waveTune+=1;
    else Global::pSong()->_pInstrument[instrumentIndex()]->waveTune=71;
    octLbl->setText(noteToString((Global::pSong()->_pInstrument[instrumentIndex()]->waveTune+48)));
    octLbl->repaint();
  }
}

int InstrumentEditor::instrumentIndex( )
{
   return Global::pSong()->instSelected;
}

void InstrumentEditor::onComboSelected( NItemEvent * ev )
{
  if (newNoteActionCb->selIndex()!=-1)
    Global::pSong()->_pInstrument[instrumentIndex()]->_NNA = newNoteActionCb->selIndex();
}

void InstrumentEditor::onSliderMove( NSlider * sender, double pos )
{
  if (sender == panningSlider) {
    Global::pSong()->_pInstrument[instrumentIndex()]->_pan = (int) pos;
    panningLbl->setText(stringify(Global::pSong()->_pInstrument[instrumentIndex()]->_pan));
    panningLbl->repaint();
  } else
  if (sender == volumeSlider) {
    Global::pSong()->_pInstrument[instrumentIndex()]->waveVolume = (int) pos;
    volumeLbl->setText(stringify(Global::pSong()->_pInstrument[instrumentIndex()]->waveVolume)+"%");
    volumeLbl->repaint();
  } else
  if (sender == fineTuneSlider) {
    Global::pSong()->_pInstrument[instrumentIndex()]->waveFinetune=(int) pos - 256;
    fineTuneLbl->setText(stringify(Global::pSong()->_pInstrument[instrumentIndex()]->waveFinetune));
    fineTuneLbl->repaint();
  }
}
