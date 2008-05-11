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
#include "dirspage.hpp"
#include "../global.hpp"
#include "../configuration.hpp"

#include <iostream>

#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>
#include <QLineEdit>
#include <QFileDialog>

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
		songPathEdit_ = new QLineEdit();
		songPathEdit_->setReadOnly( true );
		QString songPathString = settings.value( "paths/songPath", "." ).toString();
		songPathEdit_->setText( songPathString );
		dirsLay->addWidget( songPathEdit_ );
		
		QPushButton *songBrowse = new QPushButton( "Browse..." );
		dirsLay->addWidget( songBrowse );
		connect( songBrowse, SIGNAL( clicked() ), this, SLOT( onSongBrowse() ) );





		dirsLay->addWidget( new QLabel("Psycle Plugins Directory") );
		pluginsPathEdit_ = new QLineEdit();
		pluginsPathEdit_->setReadOnly( true );
		QString pluginsPathString = settings.value( "paths/pluginsPath", "." ).toString();
		pluginsPathEdit_->setText( pluginsPathString );
		dirsLay->addWidget( pluginsPathEdit_ );

		QPushButton *pluginsBrowse = new QPushButton( "Browse..." );
		dirsLay->addWidget( pluginsBrowse );

		connect( pluginsBrowse, SIGNAL( clicked() ), this, SLOT( onPluginsBrowse() ) );


		dirsLay->addWidget( new QLabel("Psycle Presets Directory") );
		presetsPathEdit_ = new QLineEdit();
		presetsPathEdit_->setReadOnly( true );
		QString presetsPathString = settings.value( "paths/presetsPath", "." ).toString();
		presetsPathEdit_->setText( presetsPathString );
		dirsLay->addWidget( presetsPathEdit_ );

		QPushButton *presetsBrowse = new QPushButton( "Browse..." );
		dirsLay->addWidget( presetsBrowse );

		connect( presetsBrowse, SIGNAL( clicked() ), this, SLOT( onPresetsBrowse() ) );




		dirsLay->addWidget( new QLabel("Ladspa Plugins Directory") );
		ladspaPathEdit_ = new QLineEdit();
		ladspaPathEdit_->setReadOnly( true );
		QString ladspaPathString = settings.value( "paths/ladspaPath", "/usr/lib/ladspa" ).toString();
		ladspaPathEdit_->setText( ladspaPathString );
		dirsLay->addWidget( ladspaPathEdit_ );

		QPushButton *ladspaBrowse = new QPushButton( "Browse..." );
		dirsLay->addWidget( ladspaBrowse );

		connect( ladspaBrowse, SIGNAL( clicked() ), this, SLOT( onLadspaBrowse() ) );

		dirsLay->addWidget( new QLabel("Samples Directory") );
		samplesPathEdit_ = new QLineEdit();
		samplesPathEdit_->setReadOnly( true );
		QString samplesPathString = settings.value( "paths/samplesPath", "/home/samples" ).toString();
		samplesPathEdit_->setText( samplesPathString );
		dirsLay->addWidget( samplesPathEdit_ );

		QPushButton *samplesBrowse = new QPushButton( "Browse..." );
		dirsLay->addWidget( samplesBrowse );

		connect( samplesBrowse, SIGNAL( clicked() ), this, SLOT( onSamplesBrowse() ) );





		mainLay->addWidget( dirsGroup );

		setLayout( mainLay );
	}

	void DirsPage::onSongBrowse()
	{
		QString newSongPath = QFileDialog::getExistingDirectory(
			this, "Choose song directory",
			songPathEdit_->text() );

		settings.setValue( "paths/songPath", newSongPath );
		songPathEdit_->setText( newSongPath );
	}

	void DirsPage::onPluginsBrowse()
	{
		QString newPluginsPath = QFileDialog::getExistingDirectory(
			this, "Choose plugins directory",
			pluginsPathEdit_->text() );

		settings.setValue( "paths/pluginsPath", newPluginsPath );
		pluginsPathEdit_->setText( newPluginsPath );
	}


	void DirsPage::onPresetsBrowse()
	{
		QString newPresetsPath = QFileDialog::getExistingDirectory(
			this, "Choose presets directory",
			presetsPathEdit_->text() );

		settings.setValue( "paths/presetsPath", newPresetsPath );
		presetsPathEdit_->setText( newPresetsPath );
	}

	void DirsPage::onLadspaBrowse()
	{
		QString newLadspaPath = QFileDialog::getExistingDirectory(
			this, "Choose ladspa directory",
			ladspaPathEdit_->text() );

		settings.setValue( "paths/ladspaPath", newLadspaPath );
		ladspaPathEdit_->setText( newLadspaPath );
	}

	void DirsPage::onSamplesBrowse()
	{
		QString newSamplesPath = QFileDialog::getExistingDirectory(
			this, "Choose Samples directory",
			samplesPathEdit_->text() );

		settings.setValue( "paths/samplesPath", newSamplesPath );
		samplesPathEdit_->setText( newSamplesPath );
	}



} // namespace qpsycle
