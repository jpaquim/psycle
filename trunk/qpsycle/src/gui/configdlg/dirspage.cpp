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
		setupUi( this );

		songPathEdit_->setReadOnly( true );
		QString songPathString = settings.value( "paths/songPath", "." ).toString();
		songPathEdit_->setText( songPathString );
		connect( songBrowse, SIGNAL( clicked() ), this, SLOT( onSongBrowse() ) );

		pluginsPathEdit_->setReadOnly( true );
		QString pluginsPathString = settings.value( "paths/pluginsPath", "." ).toString();
		pluginsPathEdit_->setText( pluginsPathString );
		connect( pluginsBrowse, SIGNAL( clicked() ), this, SLOT( onPluginsBrowse() ) );

		presetsPathEdit_->setReadOnly( true );
		QString presetsPathString = settings.value( "paths/presetsPath", "." ).toString();
		presetsPathEdit_->setText( presetsPathString );
		connect( presetsBrowse, SIGNAL( clicked() ), this, SLOT( onPresetsBrowse() ) );

		ladspaPathEdit_->setReadOnly( true );
		QString ladspaPathString = settings.value( "paths/ladspaPath", "/usr/lib/ladspa" ).toString();
		ladspaPathEdit_->setText( ladspaPathString );
		connect( ladspaBrowse, SIGNAL( clicked() ), this, SLOT( onLadspaBrowse() ) );

		samplesPathEdit_->setReadOnly( true );
		QString samplesPathString = settings.value( "paths/samplesPath", "/home/samples" ).toString();
		samplesPathEdit_->setText( samplesPathString );
		connect( samplesBrowse, SIGNAL( clicked() ), this, SLOT( onSamplesBrowse() ) );
	}

	void DirsPage::onSongBrowse()
	{
		QString newSongPath = QFileDialog::getExistingDirectory(
			this, "Choose song directory",
			songPathEdit_->text() );

		if ( !newSongPath.isEmpty() ) {
			settings.setValue( "paths/songPath", newSongPath );
			songPathEdit_->setText( newSongPath );
		}
	}

	void DirsPage::onPluginsBrowse()
	{
		QString newPluginsPath = QFileDialog::getExistingDirectory(
			this, "Choose plugins directory",
			pluginsPathEdit_->text() );

		if ( !newPluginsPath.isEmpty() ) {
			settings.setValue( "paths/pluginsPath", newPluginsPath );
			pluginsPathEdit_->setText( newPluginsPath );
		}
	}


	void DirsPage::onPresetsBrowse()
	{
		QString newPresetsPath = QFileDialog::getExistingDirectory(
			this, "Choose presets directory",
			presetsPathEdit_->text() );

		if ( !newPresetsPath.isEmpty() ) {
			settings.setValue( "paths/presetsPath", newPresetsPath );
			presetsPathEdit_->setText( newPresetsPath );
		}
	}

	void DirsPage::onLadspaBrowse()
	{
		QString newLadspaPath = QFileDialog::getExistingDirectory(
			this, "Choose ladspa directory",
			ladspaPathEdit_->text() );

		if ( !newLadspaPath.isEmpty() ) {
			settings.setValue( "paths/ladspaPath", newLadspaPath );
			ladspaPathEdit_->setText( newLadspaPath );
		}
	}

	void DirsPage::onSamplesBrowse()
	{
		QString newSamplesPath = QFileDialog::getExistingDirectory(
			this, "Choose Samples directory",
			samplesPathEdit_->text() );

		if ( !newSamplesPath.isEmpty() ) {
			settings.setValue( "paths/samplesPath", newSamplesPath );
			samplesPathEdit_->setText( newSamplesPath );
		}
	}



} // namespace qpsycle
