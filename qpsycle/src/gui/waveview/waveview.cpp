// -*- mode:c++; indent-tabs-mode:t -*-
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
#include <psycle/core/instrument.h>
#include <psycle/core/sampler.h>

#include "waveview.hpp"

#include "../global.hpp"
#include "../configuration.hpp"
#include "../../model/instrumentsmodel.hpp"
#include "wavedisplay.hpp"
#include "waveamp.hpp"

#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QSlider>
#include <QCheckBox>

#include <iostream>
#include <iomanip>

namespace qpsycle {

WaveView::WaveView( InstrumentsModel *instrumentsModel, QWidget *parent) 
	: QWidget(parent),
		instrumentsModel_( instrumentsModel )
{
	QVBoxLayout *mainlayout = new QVBoxLayout();
	setLayout(mainlayout);
	
	// top row: instrument index, name, and output; load/kill buttons
	{
		QHBoxLayout *hlayout = new QHBoxLayout();
		QGroupBox *frame = new QGroupBox( tr("Instrument"), this);
		frame->setLayout(hlayout);
	
		instIndexSpin_ = new HexSpinBox(this);
		instIndexSpin_->setRange(0, 255);
		instName_ = new QLineEdit(this);
		outputCbx_ = new QComboBox(this);
		QPushButton *loadBtn = new QPushButton( tr("Load"), this);
		QPushButton *killBtn = new QPushButton( tr("Kill"), this);

		connect( instIndexSpin_, SIGNAL(valueChanged(int)), this, SLOT(indexSpinChanged(int)) );
		connect( instName_, SIGNAL(editingFinished()), this, SLOT(nameChanged()) );
		connect( loadBtn, SIGNAL(clicked()), this, SLOT(onLoadButtonClicked()) );
		connect( killBtn, SIGNAL(clicked()), this, SLOT(onKillButtonClicked()) );

		hlayout->addWidget( instIndexSpin_ );
		hlayout->addWidget( instName_ );
		hlayout->addSpacing(20);
		hlayout->addWidget( new QLabel( tr("Output Machine: "), this) );
		hlayout->addWidget( outputCbx_ );
		hlayout->addSpacing(20);
		hlayout->addWidget( loadBtn );
		hlayout->addWidget( killBtn);

		mainlayout->addWidget( frame );
	}


	// second row: wave display, play button, zoom controls
	{
		QHBoxLayout *hlayout = new QHBoxLayout();

		waveDisplay_ = new WaveDisplay(this, instrumentsModel);

		QVBoxLayout *vlayout = new QVBoxLayout();
		QPushButton *listenBtn = new QPushButton( tr("Listen"), this);

		QHBoxLayout *zoomlayout = new QHBoxLayout();
		zoomInBtn_ = new QPushButton( QIcon(":/images/zoom-in.png"), "", this);
		zoomOutBtn_ = new QPushButton( QIcon(":/images/zoom-out.png"), "", this);
	
		connect( listenBtn, SIGNAL(pressed()), this, SLOT(onListenPressed()) );
		connect( listenBtn, SIGNAL(released()), this, SLOT(onListenReleased()) );
		connect( zoomInBtn_, SIGNAL(clicked()), this, SLOT(onZoomInButtonClicked()) );
		connect( zoomOutBtn_, SIGNAL(clicked()), this, SLOT(onZoomOutButtonClicked()) );

		zoomlayout->addWidget( zoomOutBtn_ );
		zoomlayout->addWidget( zoomInBtn_ );

		vlayout->addStretch();
		vlayout->addWidget( listenBtn );
		vlayout->addLayout( zoomlayout );
		vlayout->addSpacing(20);

		hlayout->addWidget(waveDisplay_);
		hlayout->addLayout(vlayout);

		mainlayout->addLayout( hlayout );
	}


	// third row: volume, pan, tuning, nnas, randomizing
	{
		QHBoxLayout *hlayout = new QHBoxLayout();

		// volume and panning
		{
			QVBoxLayout *vlayout = new QVBoxLayout();

			QHBoxLayout *volLayout = new QHBoxLayout();
			QHBoxLayout *panLayout = new QHBoxLayout();

			QGroupBox *volGrp = new QGroupBox( tr("Volume"), this );
			QGroupBox *panGrp = new QGroupBox( tr("Pan"), this );

			volSlider_ = new QSlider( Qt::Horizontal, this );
			panSlider_ = new QSlider( Qt::Horizontal, this );
			volSlider_->setRange(0, 512);
			panSlider_->setRange(0, 256);

			volLabel_ = new QLabel( "100", this );
			panLabel_ = new QLabel( "128", this );
			volLabel_->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
			panLabel_->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
			volLabel_->setAlignment( Qt::AlignCenter );
			panLabel_->setAlignment( Qt::AlignCenter );
			volLabel_->setMinimumWidth( 40 );
			panLabel_->setMinimumWidth( 40 );

			connect( volSlider_, SIGNAL(valueChanged(int)), this, SLOT(onVolSliderMoved(int)) );
			connect( panSlider_, SIGNAL(valueChanged(int)), this, SLOT(onPanSliderMoved(int)) );

			volLayout->addWidget( volSlider_ );
			panLayout->addWidget( panSlider_ );
			volLayout->addWidget( volLabel_ );
			panLayout->addWidget( panLabel_ );

			volGrp->setLayout( volLayout );
			panGrp->setLayout( panLayout );

			vlayout->addWidget( volGrp );
			vlayout->addWidget( panGrp );

			hlayout->addLayout( vlayout );
		}

		// tuning
		{
			QGridLayout *tuneLayout = new QGridLayout();

			QGroupBox *tuneGrp = new QGroupBox("Tuning", this);

			octaveSpin_ = new QSpinBox( this );
			semiSpin_ = new QSpinBox( this );
			centSpin_ = new QSpinBox( this );
			octaveSpin_->setRange(-4, 4);
			semiSpin_->setRange(-11, 11);
			centSpin_->setRange(-99, 99);

			QLabel *octaveLabel = new QLabel("Octave", this);
			QLabel *semiLabel = new QLabel("Semitone", this);
			QLabel *centLabel = new QLabel("Fine", this);
			octaveLabel->setAlignment( Qt::AlignRight );
			semiLabel->setAlignment( Qt::AlignRight );
			centLabel->setAlignment( Qt::AlignRight );

			connect( octaveSpin_, SIGNAL(valueChanged(int)), this, SLOT(onCoarseTuneChanged()) );
			connect( semiSpin_, SIGNAL(valueChanged(int)), this, SLOT(onCoarseTuneChanged()) );
			connect( centSpin_, SIGNAL(valueChanged(int)), this, SLOT(onFineTuneChanged()) );

			tuneLayout->addWidget( octaveLabel, 0, 0);
			tuneLayout->addWidget( semiLabel, 1, 0);
			tuneLayout->addWidget( centLabel, 2, 0);
			tuneLayout->addWidget( octaveSpin_, 0, 1 );
			tuneLayout->addWidget( semiSpin_, 1, 1 );
			tuneLayout->addWidget( centSpin_, 2, 1 );

			tuneGrp->setLayout( tuneLayout );

			hlayout->addWidget( tuneGrp );
		}

		// randomization
		{
			QVBoxLayout *randLayout = new QVBoxLayout();
			QGroupBox *randGroup = new QGroupBox("Randomization", this);

			randPanChk_ = new QCheckBox("Random Panning", this);
			randResChk_ = new QCheckBox("Random Filter Resonance", this);
			randCutChk_ = new QCheckBox("Random Filter Cutoff", this);

			connect( randPanChk_, SIGNAL(stateChanged(int)), this, SLOT(onRandPanChanged(int)) );
			connect( randResChk_, SIGNAL(stateChanged(int)), this, SLOT(onRandResChanged(int)) );
			connect( randCutChk_, SIGNAL(stateChanged(int)), this, SLOT(onRandCutoffChanged(int)) );

			randLayout->addWidget(randPanChk_);
			randLayout->addWidget(randCutChk_);
			randLayout->addWidget(randResChk_);

			randGroup->setLayout( randLayout );
			hlayout->addWidget( randGroup );
		}

		// nnas, looping
		{
			QVBoxLayout *vlayout = new QVBoxLayout();

			QHBoxLayout *nnaLayout = new QHBoxLayout();
			QHBoxLayout *loopLayout = new QHBoxLayout();
			QGroupBox *nnaGroup = new QGroupBox("NNA", this);
			QGroupBox *loopGroup = new QGroupBox("Loop", this );

			nnaCbx_ = new QComboBox(this);
			loopCbx_ = new QComboBox(this);

			nnaCbx_->addItem("Cut");
			nnaCbx_->addItem("Release");
			nnaCbx_->addItem("Continue");
			loopCbx_->addItem("None");
			loopCbx_->addItem("Forward");

			connect( nnaCbx_, SIGNAL(currentIndexChanged(int)), this, SLOT(onNNAChanged(int)) );
			connect( loopCbx_, SIGNAL(currentIndexChanged(int)), this, SLOT(onLoopChanged(int)) );

			nnaLayout->addWidget(nnaCbx_);
			loopLayout->addWidget(loopCbx_);
			nnaGroup->setLayout(nnaLayout);
			loopGroup->setLayout(loopLayout);

			vlayout->addWidget(nnaGroup);
			vlayout->addWidget(loopGroup);

			hlayout->addLayout(vlayout);

		}

		mainlayout->addLayout( hlayout );
	}

	connect(
		instrumentsModel_, SIGNAL( selectedInstrumentChanged(int) ),
		this, SLOT( reset() )
	);

	instIndexSpin_->setValue( instrumentsModel_->selectedInstrumentIndex() );
	reset();
}

void WaveView::indexSpinChanged(int newidx)
{
	int curInstrIndex = instrumentsModel_->selectedInstrumentIndex();
	if ( newidx != curInstrIndex )
		instrumentsModel_->setSelectedInstrumentIndex( newidx );
}

void WaveView::reset()
{
	int index = instrumentsModel_->selectedInstrumentIndex();
	psy::core::Instrument *inst = instrumentsModel_->getInstrument( index );

	resettingwidgets_ = true;

	instIndexSpin_->setValue( index );
	instName_->setText( inst->_sName );

	bool slotEmpty = instrumentsModel_->slotIsEmpty(index);
	instName_->setReadOnly( slotEmpty );
	zoomInBtn_->setEnabled( !slotEmpty );
	zoomOutBtn_->setEnabled( !slotEmpty );

	volSlider_->setValue( inst->waveVolume );
	volLabel_->setText( QString::number(inst->waveVolume) );
	panSlider_->setValue( inst->_pan );
	panLabel_->setText( QString::number(inst->_pan) );
	octaveSpin_->setValue( inst->waveTune/12 );
	semiSpin_->setValue( inst->waveTune%12 );
	centSpin_->setValue( (int)round( 100.0*inst->waveFinetune/255.0 ) );
	randPanChk_->setCheckState( inst->_RPAN? Qt::Checked: Qt::Unchecked );
	randCutChk_->setCheckState( inst->_RCUT? Qt::Checked: Qt::Unchecked );
	randResChk_->setCheckState( inst->_RRES? Qt::Checked: Qt::Unchecked );
	nnaCbx_->setCurrentIndex( inst->_NNA );
	loopCbx_->setCurrentIndex( inst->waveLoopType? 1: 0 );

	resettingwidgets_ = false;

	psy::core::Sampler::waved.Stop();
	psy::core::Sampler::waved.SetInstrument(inst);
}

void WaveView::nameChanged()
{
	int curInst = instrumentsModel_->selectedInstrumentIndex();
	instrumentsModel_->setName( curInst, instName_->text() );
}

void WaveView::onLoadButtonClicked()
{
	QString samplePath = QString::fromStdString( Global::configuration().samplePath() );
	QString pathToWavfile = QFileDialog::getOpenFileName( this, tr("Open File"),
									samplePath,
									tr("Wave files (*.wav)") );
	int curInstrIndex = instrumentsModel_->selectedInstrumentIndex();

	///\todo this warning is displayed even if the file open was canceled
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

void WaveView::onZoomInButtonClicked()
{
	waveDisplay_->ZoomIn();
}

void WaveView::onZoomOutButtonClicked()
{
	///\todo limit zooming
	waveDisplay_->ZoomOut();
}

void WaveView::onKillButtonClicked()
{
}

void WaveView::onListenPressed()
{
	psy::core::Sampler::waved.Play();
}
void WaveView::onListenReleased()
{
	psy::core::Sampler::waved.Release();
}

void WaveView::onVolSliderMoved(int newval)
{
	int index = instrumentsModel_->selectedInstrumentIndex();
	if ( resettingwidgets_ || instrumentsModel_->slotIsEmpty(index) )
		return;

	instrumentsModel_->getInstrument(index)->waveVolume = newval;
	volLabel_->setText( QString::number(newval) );
}

void WaveView::onPanSliderMoved(int newval)
{
	int index = instrumentsModel_->selectedInstrumentIndex();
	if ( resettingwidgets_ || instrumentsModel_->slotIsEmpty(index) )
		return;

	instrumentsModel_->getInstrument(index)->_pan = newval;
	panLabel_->setText( QString::number(newval) );
}

void WaveView::onCoarseTuneChanged()
{
	int index = instrumentsModel_->selectedInstrumentIndex();
	if ( resettingwidgets_ || instrumentsModel_->slotIsEmpty(index) )
		return;

	instrumentsModel_->getInstrument(index)->waveTune =
		octaveSpin_->value() * 12 +
		semiSpin_->value();
}

void WaveView::onFineTuneChanged()
{
	int index = instrumentsModel_->selectedInstrumentIndex();
	if ( resettingwidgets_ || instrumentsModel_->slotIsEmpty(index) )
		return;

	instrumentsModel_->getInstrument(index)->waveFinetune = (int)round( 255.0*centSpin_->value()/100.0 );

}

void WaveView::onRandPanChanged(int newstate)
{
	int index = instrumentsModel_->selectedInstrumentIndex();
	if ( resettingwidgets_ || instrumentsModel_->slotIsEmpty(index) )
		return;

	instrumentsModel_->getInstrument(index)->_RPAN = (newstate==Qt::Checked? true: false);
}

void WaveView::onRandResChanged(int newstate)
{
	int index = instrumentsModel_->selectedInstrumentIndex();
	if ( resettingwidgets_ || instrumentsModel_->slotIsEmpty(index) )
		return;

	instrumentsModel_->getInstrument(index)->_RRES = (newstate==Qt::Checked? true: false);
}

void WaveView::onRandCutoffChanged(int newstate)
{
	int index = instrumentsModel_->selectedInstrumentIndex();
	if ( resettingwidgets_ || instrumentsModel_->slotIsEmpty(index) )
		return;

	instrumentsModel_->getInstrument(index)->_RCUT = (newstate==Qt::Checked? true: false);
}

void WaveView::onNNAChanged(int newstate)
{
	int index = instrumentsModel_->selectedInstrumentIndex();
	if ( resettingwidgets_ || instrumentsModel_->slotIsEmpty(index) )
		return;

	instrumentsModel_->getInstrument(index)->_NNA = (unsigned char)newstate;
}

void WaveView::onLoopChanged(int newstate)
{
	int index = instrumentsModel_->selectedInstrumentIndex();
	if ( instrumentsModel_->slotIsEmpty(index) )
		return;

	psy::core::Instrument *inst = instrumentsModel_->getInstrument(index);

	inst->waveLoopType = (newstate? true: false);

	if (inst->waveLoopEnd==0 && newstate > 0 ) {
		inst->waveLoopEnd = inst->waveLength;
	}
	//waveDisplay_->scene()->update( waveDisplay_->sceneRect() );
}



QString HexSpinBox::textFromValue ( int value ) const
{
	QString str;
	str.setNum( value, 16 );
	str = str.toUpper();
	if ( str.length() == 1 )
		str.prepend("0");
	return str;
}
int HexSpinBox::valueFromText ( const QString & text ) const
{
	bool ok;
	return text.toInt(&ok, 16);
}
QValidator::State HexSpinBox::validate ( QString &input, int &pos ) const
{
	if ( input.isEmpty() )
		return QValidator::Intermediate;

	bool ok;
	input.toInt(&ok, 16);
	bool oksize = input.length() == 2;
	if (ok) {
		if (oksize)
			return QValidator::Acceptable;
		else
			return QValidator::Intermediate;
	}

	return QValidator::Invalid;
}


} // namespace qpsycle
