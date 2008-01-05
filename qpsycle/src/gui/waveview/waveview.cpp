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
#include <qpsyclePch.hpp>

#include "waveview.hpp"

#include "../global.hpp"
#include "../configuration.hpp"
#include "../../model/instrumentsmodel.hpp"
#include "wavedisplay.hpp"
#include "waveamp.hpp"

#include <QAction>
#include <QMessageBox>
#include <QFileDialog>

#include <iostream>
#include <iomanip>

namespace qpsycle {

WaveView::WaveView( InstrumentsModel *instrumentsModel, QWidget *parent) 
	: QWidget(parent),
		instrumentsModel_( instrumentsModel )
{
	layout_ = new QVBoxLayout();
	setLayout(layout_);
	
	waveDisplay_ = new WaveDisplay(this, instrumentsModel);
		
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
	QLabel *toolLabel = new QLabel("Tools : ", this);
	tselAct_ = new QAction( QIcon(":images/selector.png"), "Selector", this);
	tselAct_->setCheckable(true);
	connect( tselAct_, SIGNAL(triggered()), this, SLOT(onSelectorButtonClicked()));
	tselAct_->setChecked(true);
	
	tzoomAct_ = new QAction (QIcon(":images/zoom-in.png"), "Zoom", this);
	tzoomAct_->setCheckable(true);
	connect( tzoomAct_, SIGNAL(triggered()), this, SLOT(onZoomButtonClicked()));
	
	//processing
	processBar_ = new QToolBar();
	processBar_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	QLabel *efxLabel = new QLabel("Process: ");
	ampEfx_ = new QAction("Amplify", this);
	connect( ampEfx_, SIGNAL( triggered() ), this, SLOT( onAmpButtonClicked() ));
	convEfx_ = new QAction("Convert to Mono", this);
	remdcEfx_ = new QAction("Remove DC offset", this);
	invEfx_ = new QAction("Invert Channels", this);
			
	//zoom and various
	zoomBar_ = new QToolBar();
	zoomBar_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	QLabel *zoomLabel = new QLabel("Zoom: ");
	zoomMore_ = new QAction(QIcon(":/images/plus.png"), tr("Zoom More"), this);
	connect( zoomMore_, SIGNAL( triggered() ), this, SLOT( onPlusButtonClicked() ));
		
	zoomLess_ = new QAction(QIcon(":/images/minus.png"), tr("Zoom Less"), this);
	connect( zoomLess_, SIGNAL( triggered() ), this, SLOT( onMinusButtonClicked() ));
	sampName_ = new QLabel("Sample Name", this);
	sampName_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	
	zoomSlide_ = new QSlider(Qt::Horizontal, this);
	zoomSlide_->setMinimum(0);
	zoomSlide_->setMaximum(100);
	zoomSlide_->setPageStep(10);
		
	zoomBar_->addWidget( zoomLabel );
	zoomBar_->addAction( zoomLess_ );
	zoomBar_->addWidget( zoomSlide_ );
	zoomBar_->addAction( zoomMore_ );
	zoomBar_->addWidget( sampName_);
				
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
	layout_->addWidget( waveDisplay_ );
	layout_->addWidget( zoomBar_ );
	
	
}

void WaveView::onLoadButtonClicked()
{
	QString samplePath = QString::fromStdString( Global::configuration().samplePath() );
	QString pathToWavfile = QFileDialog::getOpenFileName( this, tr("Open File"),
									samplePath,
									tr("Wave files (*.wav)") );
	int curInstrIndex = instrumentsModel_->selectedInstrumentIndex();

	if ( !instrumentsModel_->slotIsEmpty( curInstrIndex ) )
	{
		int ret = QMessageBox::warning(this, tr("Overwrite sample?"),
							tr("A sample is already loaded here.\n"
							"Do you want to overwrite the current sample?"),
							QMessageBox::Ok | QMessageBox::Cancel );
		if ( ret == QMessageBox::Cancel ) return;
	}
	instrumentsModel_->loadInstrument( curInstrIndex, pathToWavfile );
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
		waveDisplay_->scale( 1.5, 1 );
	}
}

void WaveView::onMinusButtonClicked()
{
	if (zoomSlide_->value() > 0)
	{
		zoomSlide_->setValue(zoomSlide_->value() - 1);
		waveDisplay_->scale( 2.0/3.0, 1 );
	}
}

void WaveView::onAmpButtonClicked()
{
	WaveAmp *amp = new WaveAmp();
	amp->show();
}

void WaveView::onSelectorButtonClicked()
{
	tselAct_->setChecked(true);
	tzoomAct_->setChecked(false);
}

void WaveView::onZoomButtonClicked()
{
	tzoomAct_->setChecked(true);
	tselAct_->setChecked(false);
}

} // namespace qpsycle
