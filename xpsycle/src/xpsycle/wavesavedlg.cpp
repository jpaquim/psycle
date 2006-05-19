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
#include <ngrs/nbutton.h>
#include <ngrs/nedit.h>
#include <ngrs/ncheckbox.h>
#include <ngrs/nradiobutton.h>
#include <ngrs/ntogglepanel.h>

WaveSaveDlg::WaveSaveDlg()
 : NDialog()
{
  setTitle("Render as Wav File");
  setSize(500,500);

  NPanel* pathPanel = new NPanel();
    pathPanel->setLayout(NAlignLayout(5,5));
    pathPanel->add(new NLabel("Output Path"), nAlLeft);
    NButton* browseBtn = new NButton("Browse");
      browseBtn->setFlat(false);
    pathPanel->add(browseBtn,nAlRight);
    NEdit* pathEdt = new NEdit();
  pathPanel->add(pathEdt,nAlClient);

  pane()->add( pathPanel, nAlTop );

  NCheckBox* wireChkBox = new NCheckBox();
    wireChkBox->setText("Save each unmuted input to master as a separated wav (wire number will be appended to filename)");
  pane()->add(wireChkBox, nAlTop);

  NCheckBox* trackChkBox = new NCheckBox();
    trackChkBox->setText("Save each unmuted track as a separated wav (track number will be appended to filename)** may suffer from 'delay bleed' - insert silence at the end of your file if this is a problem");
  pane()->add(trackChkBox, nAlTop);

  NCheckBox* generatorChkBox = new NCheckBox();
    generatorChkBox->setText("Save each unmuted generator as a separated wav (generator number will be appended to filename)** may suffer from 'delay bleed' - insert silence at the end of your file if this is a problem");
  pane()->add(generatorChkBox, nAlTop);

  NTogglePanel* gBox = new NTogglePanel();
    gBox->setLayout(NAlignLayout());
    NRadioButton* entireRBtn = new NRadioButton();
     entireRBtn->setText("Record the entire song");
    gBox->add(entireRBtn,nAlTop);
    NRadioButton* numberRBtn = new NRadioButton();
     numberRBtn->setText("Record the entire song");
    gBox->add(numberRBtn,nAlTop);
    NRadioButton* seqRBtn = new NRadioButton();
     seqRBtn->setText("Record the entire song");
    gBox->add(seqRBtn,nAlTop);
  pane()->add(gBox, nAlTop);
}


WaveSaveDlg::~WaveSaveDlg()
{
}


