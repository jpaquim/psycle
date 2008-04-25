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
#include "lookspage.hpp"

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
#include <QStyleFactory>

namespace qpsycle {

	LooksPage::LooksPage( QWidget *parent )
		: QWidget( parent )
	{
		QVBoxLayout *mainLay = new QVBoxLayout();
		mainLay->setAlignment( Qt::AlignTop );
		QHBoxLayout *themeLay = new QHBoxLayout();
		
		
		QGroupBox *miscGroup = new QGroupBox( this );

		QLabel *themeLabel = new QLabel( "Theme" );
		themeCombo = new QComboBox( this );
		connect( themeCombo, SIGNAL( activated( QString ) ), this, SLOT( onthemeComboChanged( QString ) ) );
		QStringList themes = QStyleFactory::keys();
		themeCombo->addItems( themes );
		
		QSettings settings;
		QString currentTheme = settings.value( "looks/theme" ).toString();
		int currentIndex = themeCombo->findText( currentTheme );
		themeCombo->setCurrentIndex( currentIndex );

		QLabel *colorSchemeLabel = new QLabel( "Color Scheme:" );
		QComboBox *colorSchemeCombo = new QComboBox( this );
		connect( colorSchemeCombo, SIGNAL( activated( QString ) ), this, SLOT( oncolorSchemeComboChanged( QString ) ) );
		QDir themeDir(":/colorschemes");
		QStringList colorSchemes = themeDir.entryList();
		colorSchemeCombo->addItems( colorSchemes );

		QString currentColorScheme = settings.value( "looks/sheet" ).toString();
		currentIndex = colorSchemeCombo->findText( currentColorScheme );
		colorSchemeCombo->setCurrentIndex( currentIndex );
		

		miscGroup->setLayout( themeLay );

		themeLay->addWidget( themeLabel );
		themeLay->addWidget( themeCombo );

		themeLay->addWidget( colorSchemeLabel );
		themeLay->addWidget( colorSchemeCombo );
	
		mainLay->addWidget( miscGroup );

		setLayout( mainLay );
	}

	void LooksPage::oncolorSchemeComboChanged( const QString &sheetName )
	{
		QSettings settings;
		settings.setValue( "looks/sheet", sheetName );

		QFile file( ":/themes/" + sheetName.toLower() );
		file.open( QFile::ReadOnly );
		QString styleSheet = QLatin1String( file.readAll() );
		
		qApp->setStyleSheet( styleSheet );
	}

	void LooksPage::onthemeComboChanged( const QString &themeName )
	{
		QSettings settings;
		settings.setValue( "looks/theme", themeName );

		QApplication::setStyle(themeName);
	}

} // namespace qpsycle
