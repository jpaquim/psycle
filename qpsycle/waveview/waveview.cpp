/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
*   psycle.sourceforge.net   *
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
class QVBoxLayout;
#include "waveview.h"

#include <QtGui>
#include <iostream>

 WaveView::WaveView( psy::core::Song *song, QWidget *parent) 
    : QWidget(parent)
 {
     song_ = song;
     layout_ = new QVBoxLayout();
     setLayout(layout_);
	 
	 toolBar_ = new QToolBar();
	 loadSmp_ = new QAction( "Load Sample", this );
	 connect( loadSmp_, SIGNAL( triggered() ),
             this, SLOT( onLoadButtonClicked() ) );
	 toolBar_->addAction( loadSmp_ );

     layout_->addWidget( toolBar_ );
 }

void WaveView::onLoadButtonClicked()
{
     QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                "/home/neil/mymusic/samples/",
                                                tr("Wave files (*.wav)"));
   int si = song()->instSelected;
std::cout << "inst sel: " << si << std::endl;
				//added by sampler
				if ( song()->_pInstrument[si]->waveLength != 0)
				{
					//if (MessageBox("Overwrite current sample on the slot?","A sample is already loaded here",MB_YESNO) == IDNO)  return;
				}

				if ( song()->WavAlloc( si, fileName.toStdString().c_str() ) )
				{
					emit sampleAdded();
//					updateInstrumentCbx( song()->instSelected, true );
				}
     std::cout << fileName.toStdString() << std::endl;
}

psy::core::Song* WaveView::song()
{
    return song_;
}
