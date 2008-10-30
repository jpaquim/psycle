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
		setupUi( this );
		// see ui_behaviourpage.h, or load behaviourpage.ui into Qt Designer,
		// to see available widgets.

		knobBehaviourCombo_->addItem("Rotary (QDial)");
		knobBehaviourCombo_->addItem("Angular (QSynth)");
		knobBehaviourCombo_->addItem("Linear");
		knobBehaviourCombo_->addItem("Fixed Linear");

		QSettings settings;
		knobBehaviourCombo_->setCurrentIndex( settings.value( "behaviour/machineview/knob" ).toInt() );
		connect( knobBehaviourCombo_, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onSettingsChanged() ) );

		saveBtn_->setEnabled( false );

		connect( saveBtn_, SIGNAL( clicked() ), this, SLOT( onSaveButtonClicked() ) );

		homeEndChk_->setChecked( settings.value( "behaviour/patternview/ft2HomeEnd" ).toBool() );
		shiftChk_->setChecked( settings.value( "behaviour/patternview/shiftArrowForSelect" ).toBool() );
		wrapChk_->setChecked( settings.value( "behaviour/patternview/wrapAround" ).toBool() );
		centerCursorChk_->setChecked( settings.value( "behaviour/patternview/centerCursor" ).toBool() );

		connect( homeEndChk_, SIGNAL( stateChanged( int ) ), SLOT( onSettingsChanged() ) );
		connect( shiftChk_, SIGNAL( stateChanged( int ) ), SLOT( onSettingsChanged() ) );
		connect( wrapChk_, SIGNAL( stateChanged( int ) ), SLOT( onSettingsChanged() ) );
		connect( centerCursorChk_, SIGNAL( stateChanged( int ) ), SLOT( onSettingsChanged() ) );
	}

	void BehaviourPage::onSettingsChanged()
	{
		saveBtn_->setEnabled( true );
	}

	void BehaviourPage::onSaveButtonClicked()
	{
		QSettings settings;
		
		settings.setValue( "behaviour/machineview/knob", knobBehaviourCombo_->currentIndex() );
		settings.setValue( "behaviour/patternview/ft2HomeEnd", homeEndChk_->checkState() == Qt::Checked? true: false );
		settings.setValue( "behaviour/patternview/shiftArrowForSelect", shiftChk_->checkState() == Qt::Checked? true: false );
		settings.setValue( "behaviour/patternview/wrapAround", wrapChk_->checkState() == Qt::Checked? true: false );
		settings.setValue( "behaviour/patternview/centerCursor", centerCursorChk_->checkState() == Qt::Checked? true: false );

		saveBtn_->setEnabled( false );
	}

} // namespace qpsycle
