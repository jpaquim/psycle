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

#include "dirspage.hpp"
#include "../global.hpp"
#include "../configuration.hpp"

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
#include <QLineEdit>

namespace qpsycle {

	DirsPage::DirsPage( QWidget *parent )
		: QWidget( parent )
	{
		QVBoxLayout *mainLay = new QVBoxLayout();
		mainLay->setAlignment( Qt::AlignTop );

		QGroupBox *dirsGroup = new QGroupBox( this );
		QVBoxLayout *dirsLay = new QVBoxLayout();
		dirsGroup->setLayout( dirsLay );

		dirsLay->addWidget( new QLabel("Song Directory") );
		dirsLay->addWidget( new QLineEdit );

		dirsLay->addWidget( new QLabel("Psycle Plugins Dir") );
		dirsLay->addWidget( new QLineEdit );

		dirsLay->addWidget( new QLabel("Ladspa Plugins Dir") );
		dirsLay->addWidget( new QLineEdit );

		mainLay->addWidget( dirsGroup );

		setLayout( mainLay );
	}

} // namespace qpsycle
