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
#include <QComboBox>
#include <QPushButton>
#include <iostream>

SettingsDlg::SettingsDlg( QWidget *parent )
	: QDialog( parent )
{
	setWindowTitle("General Settings");
	config_ = Global::pConfig();

	QComboBox *knobBehaviourCombo = new QComboBox( this );
	knobBehaviourCombo->addItem("Fixed Linear");
	knobBehaviourCombo->addItem("Psycle Linear");
	knobBehaviourCombo->addItem("QDial Rotary");
	knobBehaviourCombo->addItem("QSynth Angular");

	QHBoxLayout *mainLay = new QHBoxLayout();
		
	QWidget *settingsPanel = new QWidget( this );

	mainLay->addWidget( knobBehaviourCombo );

	setLayout( mainLay );
}

