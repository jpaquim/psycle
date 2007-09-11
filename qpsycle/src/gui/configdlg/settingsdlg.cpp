/***************************************************************************
*   Copyright (C) 2007 by Psycledelics Community   *
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
#include <iostream>

SettingsDlg::SettingsDlg( QWidget *parent )
	: QDialog( parent )
{
	setWindowTitle("General Settings");
	config_ = Global::pConfig();

	QLabel *knobComboLabel = new QLabel("Knob behaviour: ");

	knobBehaviourCombo_ = new QComboBox( this );
	knobBehaviourCombo_->addItem("QDialMode");
	knobBehaviourCombo_->addItem("QSynthAngularMode");
	knobBehaviourCombo_->addItem("QSynthLinearMode");
	knobBehaviourCombo_->addItem("PsycleLinearMode");
	knobBehaviourCombo_->addItem("FixedLinearMode");

	QVBoxLayout *mainLay = new QVBoxLayout();
	QHBoxLayout *buttonsLay = new QHBoxLayout();
	QHBoxLayout *knobLay = new QHBoxLayout();
		
	QGroupBox *settingsGroup = new QGroupBox( "Machine View", this );
	QWidget *buttonsGroup = new QWidget( this );

	okBtn_ = new QPushButton( "OK" );
	applyBtn_ = new QPushButton( "Apply" );
	cancelBtn_ = new QPushButton( "Cancel" );

	knobLay->addWidget( knobComboLabel );
	knobLay->addWidget( knobBehaviourCombo_ );

	connect( okBtn_, SIGNAL( clicked() ),
		 this, SLOT( onOkButtonClicked() ) );
	connect( applyBtn_, SIGNAL( clicked() ),
		 this, SLOT( onApplyButtonClicked() ) );
	connect( cancelBtn_, SIGNAL( clicked() ),
		 this, SLOT( reject() ) );

	settingsGroup->setLayout( knobLay );

	buttonsLay->addWidget( okBtn_ );
	buttonsLay->addWidget( applyBtn_ );
	buttonsLay->addWidget( cancelBtn_ );
	buttonsGroup->setLayout( buttonsLay );


	mainLay->addWidget( settingsGroup );
	mainLay->addWidget( buttonsGroup );

	setLayout( mainLay );
}

void SettingsDlg::onOkButtonClicked()
{
	Global::pConfig()->setKnobBehaviour( (KnobMode)knobBehaviourCombo_->currentIndex() );
	accept();
}

void SettingsDlg::onApplyButtonClicked()
{
	Global::pConfig()->setKnobBehaviour( (KnobMode)knobBehaviourCombo_->currentIndex() );
}

