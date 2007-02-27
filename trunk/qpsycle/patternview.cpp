/***************************************************************************
*   Copyright (C) 2006 by  Stefan   *
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

#include <QtGui>

 #include "patternview.h"

 PatternView::PatternView(QWidget *parent) 
    : QWidget(parent)
 {
     setPalette(QPalette(QColor(50, 50, 50)));
     setAutoFillBackground(true);
     layout = new QGridLayout();
     setLayout(layout);

     createToolBar();
 }

 void PatternView::createToolBar()
 {
      toolBar_ = new QToolBar();
      meterCbx_ = new QComboBox();
      meterCbx_->addItem("4/4");
      meterCbx_->addItem("3/4");

      delBarAct_ = new QAction(tr("Delete Bar"), this);
      delBarAct_->setStatusTip(tr("Delete a bar"));

/*      patternCbx_ = new QComboBox();
      for (int i = 1; i <=16; i++) {
          patternCbx_->addItem( new QString(i) );
      }

      octaveCombo_ = new ngrs::ComboBox();
      for (int i=0; i<9; i++) octaveCombo_->add(new ngrs::Item(stringify(i)));
      octaveCombo_->itemSelected.connect(this,&PatternView::onOctaveChange);
      octaveCombo_->setPreferredSize( 40, 20 );
      octaveCombo_->setIndex(4);
      octaveCombo_->enableFocus(false);
      setEditOctave(4);
      toolBar->add(octaveCombo_);

      trackCombo_ = new ngrs::ComboBox();
      trackCombo_->setPreferredSize( 40, 20 );
      trackCombo_->itemSelected.connect(this,&PatternView::onTrackChange);
      for( int i=4; i<=MAX_TRACKS; i++ ) {
        trackCombo_->add(new ngrs::Item(stringify(i)));
      }
      trackCombo_->setIndex( _pSong->tracks() - 4 );  // starts at 4 .. so 16 - 4 = 12 ^= 16

      sharpBtn_ = new ngrs::Button("#");
      sharpBtn_->clicked.connect(this,&PatternView::onToggleSharpMode);


      sideBox = new ngrs::NCheckBox("Tweak left/right");
      sideBox->clicked.connect(this,&PatternView::onSideChange);*/

      toolBar_->addWidget(meterCbx_);
      toolBar_->addAction(delBarAct_);
//      toolBar_->addWidget(patternCbx_);

      layout->addWidget(toolBar_);
//      ngrs::Button* btn = toolBar->add(new ngrs::Button("add Bar"));
 //     toolBar->add(new ngrs::Label("Pattern Step"));
  //    toolBar->add(patternCombo_);
   //   toolBar->add(new ngrs::Label("Octave"));
    //  toolBar->add(new ngrs::Label("Tracks"));
     // toolBar->add(trackCombo_);
//      toolBar->add( sharpBtn_ );
 //     toolBar->add(sideBox);
  }

