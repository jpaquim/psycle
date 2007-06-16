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

#include <psycle/core/player.h>

#include "audioconfigdlg.h"
#include "global.h"
#include "configuration.h"

#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <iostream>

AudioConfigDlg::AudioConfigDlg( QWidget *parent )
	: QDialog( parent )
{
	setWindowTitle("Select Audio Driver");
	config_ = Global::pConfig();
	selectedDriver_ = 0;

	QHBoxLayout *mainLay = new QHBoxLayout();

	QWidget *settingsPanel = new QWidget( this );
	QWidget *notes = new QWidget( this );

	driverCbx_ = new QComboBox( this );
	restartBtn_ = new QPushButton( "Restart Driver", this );

	connect( driverCbx_, SIGNAL( currentIndexChanged( const QString & ) ),
		 this, SLOT( onDriverSelected( const QString & ) ) );
	connect( restartBtn_, SIGNAL( clicked() ),
		 this, SLOT( onRestartDriver() ) );

	mainLay->addWidget( driverCbx_ );
	mainLay->addWidget( restartBtn_ );
	setLayout( mainLay );

	initDriverList();
}

void AudioConfigDlg::initDriverList( )
{
	std::map<std::string, psy::core::AudioDriver*> & driverMap =  config_->driverMap();
	std::map<std::string, psy::core::AudioDriver*>::iterator it = driverMap.begin();
	for ( ; it != driverMap.end(); it++ ) {
		QString driverName = QString::fromStdString( it->first );
		driverCbx_->addItem( driverName );
	}
}

void AudioConfigDlg::onDriverSelected( const QString & text )
{
	std::map<std::string, psy::core::AudioDriver*> & driverMap =  config_->driverMap();
	std::map<std::string, psy::core::AudioDriver*>::iterator it = driverMap.find( text.toStdString() );
	if ( it != driverMap.end() ) {
		psy::core::AudioDriver* driver = it->second;
		selectedDriver_ = driver;
		if ( text == "alsa" ) { // FIXME: bugs if device not set, temp fix.
			psy::core::AudioDriverSettings settings = it->second->settings();
			settings.setDeviceName( "plughw:0" );
			it->second->setSettings( settings );
		}
	}
}

void AudioConfigDlg::onRestartDriver()
{		
	if ( selectedDriver_ ) {
		// disable old driver
		psy::core::Player::Instance()->driver().Enable( false );
		// set new Driver to Player
		psy::core::Player::Instance()->setDriver( *selectedDriver_ );
	}
}
