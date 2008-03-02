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
#include <qpsyclePch.hpp>

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

namespace qpsycle {

	LooksPage::LooksPage( QWidget *parent )
		: QWidget( parent )
	{
		QVBoxLayout *mainLay = new QVBoxLayout();
		mainLay->setAlignment( Qt::AlignTop );
		QHBoxLayout *miscLay = new QHBoxLayout();
		
		
		QGroupBox *miscGroup = new QGroupBox( this );
		QLabel *themeLabel = new QLabel( "Theme:" );
		QComboBox *themeCombo = new QComboBox( this );
		connect( themeCombo, SIGNAL( activated( QString ) ), this, SLOT( onThemeComboChanged( QString ) ) );
		QDir themeDir(":/themes");
		QStringList themes = themeDir.entryList();
		themeCombo->addItems( themes );

		miscGroup->setLayout( miscLay );

		miscLay->addWidget( themeLabel );
		miscLay->addWidget( themeCombo );
	
		mainLay->addWidget( miscGroup );

		setLayout( mainLay );
	}

	void LooksPage::onThemeComboChanged( const QString &sheetName )
	{
		QSettings settings;
		settings.setValue( "theme", sheetName );

		QFile file( ":/themes/" + sheetName.toLower() );
		file.open( QFile::ReadOnly );
		QString styleSheet = QLatin1String( file.readAll() );
		
		qApp->setStyleSheet( styleSheet );
	}
} // namespace qpsycle
