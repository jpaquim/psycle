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

#include "../global.h"
#include "../configuration.h"
#include "waveview.h"
#include "wavedisplay.h"

#include <QtGui>
#include <iostream>
#include <QtGui/QGraphicsView> //temp... just for testing...
#include <QGraphicsScene>
#include <QGraphicsRectItem>

#include <iostream>
#include <iomanip>


WaveView::WaveView( QStandardItemModel *instrumentsModel, psy::core::Song *song, QWidget *parent) 
	: QWidget(parent),
	  instrumentsModel_( instrumentsModel )
{
	song_ = song;
	layout_ = new QVBoxLayout();
	setLayout(layout_);
	 
	//TODO: this should be in the waveview.h and waveformview should be an object of WaveFormView
	WaveDisplay *wavedisp = new WaveDisplay(false, this);
		
	toolBar_ = new QToolBar();
	toolBar_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	 
	//sample loading
	QLabel *playLabel = new QLabel("Play: ");
	 
	loadSmp_ = new QAction( QIcon(":images/wave-open.png"), "Load Sample", this );
	connect( loadSmp_, SIGNAL( triggered() ), this, SLOT( onLoadButtonClicked() ) );
	 
	saveSmp_ = new QAction( QIcon(":images/wave-save.png"), "Save Sample", this);
	connect( saveSmp_, SIGNAL( triggered() ), this, SLOT( onSaveButtonClicked() ) );
	 
	playsSmp_ = new QAction(QIcon(":/images/wave_playstart.png"), tr("Play From Start"), this);
	playSmp_ = new QAction(QIcon(":/images/wave_play.png"), tr("Play"), this);
	stopSmp_ = new QAction(QIcon(":/images/wave_stop.png"), tr("Stop"), this);
	 
	//editing
	QLabel *editLabel = new QLabel("Edit: ", this);
	cutAct_ = new QAction( QIcon(":images/wave-cut.png"), "Cut", this);
	copyAct_ = new QAction( QIcon(":images/wave-copy.png"), "Copy", this);
	pasteAct_ = new QAction( QIcon(":images/wave-paste.png"), "Paste", this);
	selAct_ = new QAction("Select All", this);
	
	//tools
	QLabel *toolLabel = new QLabel("Tools :", this);
	tselAct_ = new QAction( QIcon(":images/selector.png"), "Selector", this);
	tzoomAct_ = new QAction (QIcon(":images/zoom-in.png"), "Zoom", this);
	
	//processing
	processBar_ = new QToolBar();
	processBar_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	QLabel *efxLabel = new QLabel("Process: ");
	ampEfx_ = new QAction("Amplify", this);
	convEfx_ = new QAction("Convert to Mono", this);
	remdcEfx_ = new QAction("Remove DC offset", this);
	invEfx_ = new QAction("Invert Channels", this);
	
	//scrollbar for the WaveFormView
	waveScroll_ = new QScrollBar(Qt::Horizontal, this);
	 
	//zoom and various
	zoomBar_ = new QToolBar();
	zoomBar_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	QLabel *zoomLabel = new QLabel("Zoom: ");
	zoomMore_ = new QAction(QIcon(":/images/plus.png"), tr("Zoom More"), this);
	connect( zoomMore_, SIGNAL( triggered() ), this, SLOT( onPlusButtonClicked() ));
	 
	zoomLess_ = new QAction(QIcon(":/images/minus.png"), tr("Zoom Less"), this);
	connect( zoomLess_, SIGNAL( triggered() ), this, SLOT( onMinusButtonClicked() ));

	zoomSlide_ = new QSlider(Qt::Horizontal, this);
	zoomSlide_->setMinimum(0);
	zoomSlide_->setMaximum(100);
	zoomSlide_->setPageStep(10);
	 
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
	toolBar_->addWidget( editLabel );
	toolBar_->addAction( cutAct_ );
	toolBar_->addAction( copyAct_ );
	toolBar_->addAction( pasteAct_ );
	toolBar_->addAction( selAct_);
	toolBar_->addSeparator();
	toolBar_->addWidget( toolLabel );
	toolBar_->addAction( tselAct_ );
	toolBar_->addAction( tzoomAct_ );

	processBar_->addWidget( efxLabel );
	processBar_->addAction( ampEfx_ );
	processBar_->addAction( convEfx_ );
	processBar_->addAction( remdcEfx_ );
	processBar_->addAction( invEfx_ );

	layout_->addWidget( toolBar_ );
	layout_->addWidget( processBar_ );
	layout_->addWidget( wavedisp );
	layout_->addWidget( waveScroll_);
	layout_->addWidget( zoomBar_ );
}

void WaveView::onLoadButtonClicked()
{
	QString samplePath = QString::fromStdString( Global::configuration().samplePath() );
	QString pathToWavfile = QFileDialog::getOpenFileName( this, tr("Open File"),
							      samplePath,
							      tr("Wave files (*.wav)") );
	int curInstrIndex = song()->instSelected;
	std::cout << "inst sel: " << curInstrIndex << std::endl;

	if ( !song()->_pInstrument[curInstrIndex]->Empty() )
	{
		int ret = QMessageBox::warning(this, tr("Overwrite sample?"),
					       tr("A sample is already loaded here.\n"
						  "Do you want to overwrite the current sample?"),
					       QMessageBox::Ok | QMessageBox::Cancel );
		if ( ret == QMessageBox::Cancel ) return;
	}
	if ( song()->WavAlloc( curInstrIndex, pathToWavfile.toStdString().c_str() ) )
	{
		QStandardItem *item = instrumentsModel_->item( curInstrIndex );
		std::ostringstream buffer;
		buffer.setf(std::ios::uppercase);			       
		buffer.str("");
		buffer << std::setfill('0') << std::hex << std::setw(2);
		buffer << curInstrIndex << ": " << song()->_pInstrument[curInstrIndex]->_sName;
		QString name = QString::fromStdString( buffer.str() );
		item->setText( name );
	}
}

void WaveView::onSaveButtonClicked()
{
	QString samplePath = QString::fromStdString( Global::configuration().samplePath() );
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
							samplePath,
							tr("Wave files (*.wav)"));
}

void WaveView::onPlusButtonClicked()
{
	if(zoomSlide_->value() < 100)
	{
		zoomSlide_->setValue(zoomSlide_->value() + 1);
	}
}

void WaveView::onMinusButtonClicked()
{
	if (zoomSlide_->value() > 0)
	{
		zoomSlide_->setValue(zoomSlide_->value() - 1);
	}
}

psy::core::Song* WaveView::song()
{
	return song_;
}
