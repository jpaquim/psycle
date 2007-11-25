/**************************************************************************
*   Copyright (C) 2007 by Psycledelics Community                          *
*   psycle.sourceforge.net                                                *
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

#include <psycle/core/player.h>

#include "settingsdlg.h"
#include "../global.h"
#include "../configuration.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <iostream>

SettingsDlg::SettingsDlg( QWidget *parent )
	: QDialog( parent )
{
	setWindowTitle("General Settings");
	config_ = Global::pConfig();

	QLabel *knobComboLabel = new QLabel("Knob behaviour: ");


	/*** Machine View ***/
	knobBehaviourCombo_ = new QComboBox( this );
	knobBehaviourCombo_->addItem("QDialMode");
	knobBehaviourCombo_->addItem("QSynthAngularMode");
	knobBehaviourCombo_->addItem("QSynthLinearMode");
	knobBehaviourCombo_->addItem("PsycleLinearMode");
	knobBehaviourCombo_->addItem("FixedLinearMode");

	knobBehaviourCombo_->setCurrentIndex( (int)Global::configuration().knobBehaviour() );
	connect( knobBehaviourCombo_, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onSettingsChanged() ) );

	QVBoxLayout *mainLay = new QVBoxLayout();
	QHBoxLayout *buttonsLay = new QHBoxLayout();
	QHBoxLayout *knobLay = new QHBoxLayout();
		
	QGroupBox *settingsGroup = new QGroupBox( "Machine View", this );
	QWidget *buttonsGroup = new QWidget( this );

	saveBtn_ = new QPushButton( "Save" );
	saveBtn_->setEnabled( false );
	closeBtn_ = new QPushButton( "Close" );

	knobLay->addWidget( knobComboLabel );
	knobLay->addWidget( knobBehaviourCombo_ );

	connect( saveBtn_, SIGNAL( clicked() ), this, SLOT( onSaveButtonClicked() ) );
	connect( closeBtn_, SIGNAL( clicked() ), this, SLOT( reject() ) );

	settingsGroup->setLayout( knobLay );

	buttonsLay->addWidget( saveBtn_ );
	buttonsLay->addWidget( closeBtn_ );
	buttonsGroup->setLayout( buttonsLay );


	/*** Pattern View Settings ***/
	QGroupBox *patternView = new QGroupBox( "Pattern View", this );
	QGridLayout *patViewLay = new QGridLayout();
	patternView->setLayout( patViewLay );
	QLabel *homeEndLabel = new QLabel( "FT2 Home/End Behaviour" );
	QLabel *shiftLabel = new QLabel( "Shift Keys For Select" );
	QLabel *wrapLabel = new QLabel( "Wrap Around" );
	QLabel *centerCursorLabel = new QLabel( "Center Cursor" );
	
	homeEndChk = new QCheckBox();
	shiftChk = new QCheckBox();
	wrapChk = new QCheckBox();
	centerCursorChk = new QCheckBox();

	connect( homeEndChk, SIGNAL( stateChanged( int ) ), SLOT( onSettingsChanged() ) );

	connect( shiftChk, SIGNAL( stateChanged( int ) ), SLOT( onSettingsChanged() ) );
	connect( wrapChk, SIGNAL( stateChanged( int ) ), SLOT( onSettingsChanged() ) );
	connect( centerCursorChk, SIGNAL( stateChanged( int ) ), SLOT( onSettingsChanged() ) );

	patViewLay->addWidget( homeEndLabel );
	patViewLay->addWidget( homeEndChk );
	patViewLay->addWidget( shiftLabel );
	patViewLay->addWidget( shiftChk );
	patViewLay->addWidget( wrapLabel );
	patViewLay->addWidget( wrapChk );
	patViewLay->addWidget( centerCursorLabel );
	patViewLay->addWidget( centerCursorChk );


	mainLay->addWidget( settingsGroup );
	mainLay->addWidget( patternView );
	mainLay->addWidget( buttonsGroup );

	setLayout( mainLay );
}

void SettingsDlg::onSettingsChanged()
{
	saveBtn_->setEnabled( true );
}

void SettingsDlg::onSaveButtonClicked()
{
	Global::pConfig()->setKnobBehaviour( (KnobMode)knobBehaviourCombo_->currentIndex() );

	switch ( homeEndChk->checkState() ) {
	case Qt::Unchecked:
		Global::pConfig()->setFT2HomeEndBehaviour( false );
	case Qt::Checked:
		Global::pConfig()->setFT2HomeEndBehaviour( true );
	}

	switch ( shiftChk->checkState() ) {
	case Qt::Unchecked:
		Global::pConfig()->setShiftKeyBehaviour( false );
	case Qt::Checked:
		Global::pConfig()->setShiftKeyBehaviour( true );
	}

	switch ( wrapChk->checkState() ) {
	case Qt::Unchecked:
		Global::pConfig()->setWrapAround( false );
	case Qt::Checked:
		Global::pConfig()->setWrapAround( true );
	}

	switch ( centerCursorChk->checkState() ) {
	case Qt::Unchecked:
		Global::pConfig()->setCenterCursor( false );
	case Qt::Checked:
		Global::pConfig()->setCenterCursor( true );
	}
	
	saveBtn_->setEnabled( false );
}
