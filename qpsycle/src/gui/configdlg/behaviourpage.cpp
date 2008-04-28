// -*- mode:c++; indent-tabs-mode:t -*-
/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/
#include <psycle/core/player.h>

#include "behaviourpage.hpp"

#include <iostream>

#include <QDebug>
#include <QDir>
#include <QApplication>
#include <QFile>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QSettings>

namespace qpsycle {

	BehaviourPage::BehaviourPage( QWidget *parent )
		: QWidget( parent )
	{
		setWindowTitle("General Settings");

		QLabel *knobComboLabel = new QLabel("Knob behaviour: ");


		/*** Machine View ***/
		knobBehaviourCombo_ = new QComboBox( this );
		knobBehaviourCombo_->addItem("Rotary (QDial)");
		knobBehaviourCombo_->addItem("Angular (QSynth)");
		knobBehaviourCombo_->addItem("Linear");
		knobBehaviourCombo_->addItem("Fixed Linear");
		QSettings settings;
		knobBehaviourCombo_->setCurrentIndex( settings.value( "behaviour/machineview/knob" ).toInt() );
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
		knobLay->setAlignment( Qt::AlignRight );
		knobLay->addWidget( knobBehaviourCombo_ );

		connect( saveBtn_, SIGNAL( clicked() ), this, SLOT( onSaveButtonClicked() ) );

		settingsGroup->setLayout( knobLay );

		buttonsLay->addWidget( saveBtn_ );
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

		homeEndChk->setChecked( settings.value( "behaviour/patternview/ft2HomeEnd" ).toBool() );
		shiftChk->setChecked( settings.value( "behaviour/patternview/shiftArrowForSelect" ).toBool() );
		wrapChk->setChecked( settings.value( "behaviour/patternview/wrapAround" ).toBool() );
		centerCursorChk->setChecked( settings.value( "behaviour/patternview/centerCursor" ).toBool() );

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

	void BehaviourPage::onSettingsChanged()
	{
		saveBtn_->setEnabled( true );
	}

	void BehaviourPage::onSaveButtonClicked()
	{
		QSettings settings;
		
		settings.setValue( "behaviour/machineview/knob", knobBehaviourCombo_->currentIndex() );
		settings.setValue( "behaviour/patternview/ft2HomeEnd", homeEndChk->checkState() == Qt::Checked? true: false );
		settings.setValue( "behaviour/patternview/shiftArrowForSelect", shiftChk->checkState() == Qt::Checked? true: false );
		settings.setValue( "behaviour/patternview/wrapAround", wrapChk->checkState() == Qt::Checked? true: false );
		settings.setValue( "behaviour/patternview/centerCursor", centerCursorChk->checkState() == Qt::Checked? true: false );

		saveBtn_->setEnabled( false );
	}

} // namespace qpsycle
