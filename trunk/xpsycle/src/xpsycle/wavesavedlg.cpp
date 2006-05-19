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
#include "wavesavedlg.h"
#include "napp.h"
#include <ngrs/nlabel.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/ntablelayout.h>
#include <ngrs/nflowlayout.h>
#include <ngrs/nalignconstraint.h>
#include <ngrs/nbutton.h>
#include <ngrs/nedit.h>
#include <ngrs/ncheckbox.h>
#include <ngrs/nradiobutton.h>
#include <ngrs/ntogglepanel.h>
#include <ngrs/ncombobox.h>
#include <ngrs/nitem.h>
#include <ngrs/nprogressbar.h>
#include <ngrs/nfiledialog.h>

WaveSaveDlg::WaveSaveDlg()
 : NDialog()
{
  setTitle("Render as Wav File");
  setSize(800,400);

  fileDlg = new NFileDialog();
  add(fileDlg);

  pane()->setLayout(NAlignLayout(5,5));

  NPanel* pathPanel = new NPanel();
    pathPanel->setLayout(NAlignLayout(5,5));
    pathPanel->add(new NLabel("Output Path"), nAlLeft);
    NButton* browseBtn = new NButton("Browse");
      browseBtn->setFlat(false);
      browseBtn->clicked.connect(this,&WaveSaveDlg::onBrowseBtn);
    pathPanel->add(browseBtn,nAlRight);
    pathEdt = new NEdit();
  pathPanel->add(pathEdt,nAlClient);

  pane()->add( pathPanel, nAlTop );

  wireChkBox = new NCheckBox();
    wireChkBox->setText("Save each unmuted input to master as a separated wav (wire number will be appended to filename)");
  pane()->add(wireChkBox, nAlTop);

  trackChkBox = new NCheckBox();
    trackChkBox->setText("Save each unmuted track as a separated wav (track number will be appended to filename)** may suffer from 'delay bleed' - insert silence at the end of your file if this is a problem");
  pane()->add(trackChkBox, nAlTop);

  generatorChkBox = new NCheckBox();
    generatorChkBox->setText("Save each unmuted generator as a separated wav (generator number will be appended to filename)** may suffer from 'delay bleed' - insert silence at the end of your file if this is a problem");
  pane()->add(generatorChkBox, nAlTop);

  NTogglePanel* gBox = new NTogglePanel();
    NTableLayout tableLayout(4,3);
      //tableLayout.setVGap(5);
      //tableLayout.setHGap(5);
    gBox->setLayout(tableLayout);
    entireRBtn = new NRadioButton();
     entireRBtn->setText("Record the entire song");
    gBox->add(entireRBtn,NAlignConstraint(nAlLeft,0,0));
    numberRBtn = new NRadioButton();
     numberRBtn->setText("Record pattern number");
    gBox->add(numberRBtn,NAlignConstraint(nAlLeft,0,1));
    numberEdt = new NEdit();
    gBox->add(numberEdt,NAlignConstraint(nAlLeft,1,1));
    gBox->add(new NLabel("in HEX value"),NAlignConstraint(nAlLeft,2,1));
    seqRBtn = new NRadioButton();
     seqRBtn->setText("Sequence positions from");
    gBox->add(seqRBtn,NAlignConstraint(nAlLeft,0,2));
    fromEdt = new NEdit();
    gBox->add(fromEdt,NAlignConstraint(nAlLeft,1,2));
    gBox->add(new NLabel("to"),NAlignConstraint(nAlLeft,2,2));
    toEdt = new NEdit();
    gBox->add(toEdt,NAlignConstraint(nAlLeft,3,2));
    gBox->add(new NLabel("in HEX value"),NAlignConstraint(nAlLeft,4,2));
  pane()->add(gBox, nAlTop);
  NPanel * audioPanel = new NPanel();
    audioPanel->setLayout(NAlignLayout());
  audioPanel->add(new NLabel("Note many filters\nscrew up when rendereing\nat slow sample rates"),nAlLeft);
    NPanel* cboxPanel = new NPanel();
       cboxPanel->setLayout(NAlignLayout());
       cboxPanel->add(new NLabel("sampling rate"),nAlTop);
       sampleRateCbx = new NComboBox();
          sampleRateCbx->add(new NItem("8192 hz"));
          sampleRateCbx->add(new NItem("11025 hz"));
          sampleRateCbx->add(new NItem("22050 hz"));
          sampleRateCbx->add(new NItem("44100 hz"));
          sampleRateCbx->add(new NItem("48000 hz"));
          sampleRateCbx->add(new NItem("96000 hz"));
          sampleRateCbx->setIndex(3);
       cboxPanel->add(sampleRateCbx,nAlTop);
       cboxPanel->add(new NLabel("bit rate"),nAlTop);
       bitDepthCbx = new NComboBox();
          bitDepthCbx->add(new NItem("8"));
          bitDepthCbx->add(new NItem("16"));
          bitDepthCbx->add(new NItem("24"));
          bitDepthCbx->add(new NItem("32"));
          bitDepthCbx->setIndex(1);
       cboxPanel->add(bitDepthCbx,nAlTop);
       cboxPanel->add(new NLabel("channels"),nAlTop); 
       channelsCbx = new NComboBox();
          channelsCbx->add(new NItem("Mono [mix]"));
          channelsCbx->add(new NItem("Mono [left]"));
          channelsCbx->add(new NItem("Mono [right]"));
          channelsCbx->add(new NItem("Stereo"));
          channelsCbx->setIndex(3);
       cboxPanel->add(channelsCbx,nAlTop);
    audioPanel->add(cboxPanel, nAlClient);
  pane()->add(audioPanel,nAlTop);
  progressBar = new NProgressBar();
     progressBar->setValue(0);
  pane()->add(progressBar,nAlTop);
  NPanel* btnPanel = new NPanel();
    btnPanel->setLayout(NFlowLayout(nAlRight,5,5));
    NButton* closeBtn = new NButton("Close");
      closeBtn->setFlat(false);
      closeBtn->clicked.connect(this,&WaveSaveDlg::onCloseBtn);
    btnPanel->add(closeBtn);
    NButton* saveBtn  = new NButton("Save as Wav");
      saveBtn->setFlat(false);
      saveBtn->clicked.connect(this,&WaveSaveDlg::onSaveBtn);
    btnPanel->add(saveBtn);
  pane()->add(btnPanel,nAlTop);
}


WaveSaveDlg::~WaveSaveDlg()
{
}

void WaveSaveDlg::onBrowseBtn( NButtonEvent * ev )
{
  if (fileDlg->execute()) {

  }
}

void WaveSaveDlg::onCloseBtn( NButtonEvent * ev )
{
}

void WaveSaveDlg::onSaveBtn( NButtonEvent * ev )
{
}


