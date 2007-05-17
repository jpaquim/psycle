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
#include <QtGui/QGraphicsView> //temp... just for testing...
#include <QGraphicsScene>

 WaveView::WaveView( psy::core::Song *song, QWidget *parent) 
    : QWidget(parent)
 {
     song_ = song;
     layout_ = new QVBoxLayout();
     setLayout(layout_);
	 //TODO: this should be in the waveview.h and waveformview should be an object of WaveFormView
	 QGraphicsView *waveformview = new QGraphicsView(this);
	 QGraphicsScene *scene = new QGraphicsScene(this);
	 waveformview->setScene(scene);
	 waveformview->setBackgroundBrush( Qt::black );
	 waveformview->adjustSize();

	 toolBar_ = new QToolBar();
	 toolBar_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	 
	 //sample loading
	 QLabel *playLabel = new QLabel("Play: ");
	 
	 loadSmp_ = new QAction( "Load Sample", this );
	 connect( loadSmp_, SIGNAL( triggered() ), this, SLOT( onLoadButtonClicked() ) );
	 
	 saveSmp_ = new QAction( "Save Sample", this);
	 connect( saveSmp_, SIGNAL( triggered() ), this, SLOT( onSaveButtonClicked() ) );
	 
	 playsSmp_ = new QAction(QIcon(":/images/wave_playstart.png"), tr("Play From Start"), this);
	 playSmp_ = new QAction(QIcon(":/images/wave_play.png"), tr("Play"), this);
	 stopSmp_ = new QAction(QIcon(":/images/wave_stop.png"), tr("Stop"), this);
	 
	 //effects
	 QLabel *efxLabel = new QLabel("Effects: ");
	 ampEfx_ = new QAction("Amplify", this);
	 
	 //zoom and various
	 zoomBar_ = new QToolBar();
	 zoomBar_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	 QLabel *zoomLabel = new QLabel("Zoom: ");
	 zoomMore_ = new QAction( "+", this);
	 zoomLess_ = new QAction("-", this);
	 zoomSlide_ = new QSlider(Qt::Horizontal, this);
	 zoomSlide_->setMinimum(0);
	 zoomSlide_->setMaximum(20);
	 zoomSlide_->setPageStep(2);
	 
	 zoomBar_->addWidget( zoomLabel );
	 zoomBar_->addAction( zoomLess_ );
	 zoomBar_->addWidget( zoomSlide_ );
	 zoomBar_->addAction( zoomMore_ );
	 
			 
	 toolBar_->addAction( loadSmp_ );
	 toolBar_->addAction( saveSmp_ );
	 toolBar_->addSeparator();
	 toolBar_->addWidget( playLabel );
	 toolBar_->addAction( playsSmp_ );
	 toolBar_->addAction( playSmp_ );
	 toolBar_->addAction( stopSmp_ );
	 toolBar_->addSeparator();
	 toolBar_->addWidget( efxLabel );
	 toolBar_->addAction( ampEfx_ );
	 

     layout_->addWidget( toolBar_ );
	 layout_->addWidget( waveformview );
	 layout_->addWidget( zoomBar_ );
	 waveformview->show();
 }

void WaveView::onLoadButtonClicked()
{
	// FIXME: unhardcode the default sample directory.
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

void WaveView::onSaveButtonClicked()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
							"/home/neil/mymusic/samples/",
							tr("Wave files (*.wav)"));
}

psy::core::Song* WaveView::song()
{
    return song_;
}
