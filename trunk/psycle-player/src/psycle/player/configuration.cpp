/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community
*   psycle.sf.net
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

#include "configuration.hpp"
#include <psycle/core/file.h>

#include <psycle/audiodrivers/audiodriver.h>
#include <psycle/audiodrivers/wavefileout.h>
#if defined PSYCLE__ALSA_AVAILABLE
	#include <psycle/audiodrivers/alsaout.h>
#endif
#if defined PSYCLE__JACK_AVAILABLE
	#include <psycle/audiodrivers/jackout.h>
#endif
#if defined PSYCLE__ESOUND_AVAILABLE
	#include <psycle/audiodrivers/esoundout.h>
#endif
#if defined PSYCLE__GSTREAMER_AVAILABLE
	#include <psycle/audiodrivers/gstreamerout.h>
#endif
#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
	#include <psycle/audiodrivers/microsoftdirectsoundout.h>
#endif
#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
	#include <psycle/audiodrivers/microsoftmmewaveout.h>
#endif

#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <exception>
#include <iostream>

#include <QtCore> // For getting key binding stuff.
#include <QDomDocument> // for reading XML file

Configuration::Configuration()
{
	//setXmlDefaults();

	AudioDriver* driver = new AudioDriver;
	addAudioDriver(driver);
	_pSilentDriver = driver;
	addAudioDriver(new psy::core::WaveFileOut);

	setDriverByName("silent");
	enableSound_ = false;

	#if defined PSYCLE__ALSA_AVAILABLE
		addAudioDriver(new psy::core::AlsaOut);
	#endif
	#if defined PSYCLE__JACK_AVAILABLE
		addAudioDriver(new psy::core::JackOut);
	#endif
	#if defined PSYCLE__ESOUND_AVAILABLE
		addAudioDriver(new psy::core::ESoundOut);
	#endif		
	#if defined PSYCLE__GSTREAMER_AVAILABLE
		addAudioDriver(new psy::core::GStreamerOut);
	#endif
	#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
		addAudioDriver(new psy::core::MsDirectSound);
		// use dsound by default
		setDriverByName("dsound");
		enableSound_ = true;
	#endif
	#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
		addAudioDriver(new psy::core::MsWaveOut);
	#endif
	#if defined PSYCLE__NET_AUDIO_AVAILABLE
		addAudioDriver(new psy::core::NetAudioOut);
	#endif
	loadConfig();			
}

void Configuration::addAudioDriver(AudioDriver* driver)
{
	std::cout << "psycle: configuration: audio driver registered: " <<  driver->info().name() << std::endl;
	driverMap_[ driver->info().name() ] = driver;
}

void Configuration::setDriverByName( const std::string & driverName )
{
	std::map< std::string, AudioDriver*>::iterator it = driverMap_.begin();
	if ( ( it = driverMap_.find( driverName ) ) != driverMap_.end() ) {
		// driver found
		_pOutputDriver = it->second;
		std::cout << "psycle: configuration: audio driver set to: " << driverName << "\n";
	}
	else {
		std::cerr << "psycle: configuration: audio driver not found: " << driverName << ", setting fallback: " << _pOutputDriver->info().name() << "\n";
		// driver not found,  set silent default driver
		_pOutputDriver = _pSilentDriver;
	}
}

void Configuration::loadConfig()
{
	std::string path = psy::core::File::replaceTilde("~" + psy::core::File::slash() + ".xpsycle.xml");
	if (path.length()!=0) {
		try {
			loadConfig( psy::core::File::replaceTilde( "~" + psy::core::File::slash() + ".xpsycle.xml") );
		} catch( std::exception const & e ) {
			std::cerr << "psycle: configuration: error: " << e.what() << std::endl;
		}
	} else {
		#if 0
			#if defined PSYCLE__INSTALL_PATHS__CONFIGURATION
				path = PSYCLE__INSTALL_PATHS__CONFIGURATION "/xpsycle.xml";
			#endif
			if (path.length()!=0){
				try {
					loadConfig(path);
				} catch(std::exception const & e) {
					std::cerr << "psycle: configuration: error: " << e.what() << std::endl;
				}
			}
		#endif
	}
}

void Configuration::loadConfig( const std::string & path )
{
	QFile *file = new QFile( QString::fromStdString( path ) );
	if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
		QDomDocument *doc = new QDomDocument();
		doc->setContent( file );
		QDomElement root = doc->firstChildElement();

		// Paths.
		QDomNodeList paths = root.elementsByTagName( "path" );
		for ( int i = 0; i < paths.count(); i++ )
		{
			QDomElement path = paths.item( i ).toElement();
			std::string id = path.attribute("id").toStdString();
			std::string src = path.attribute("src").toStdString();
			if ( id == "plugindir" ) pluginPath_ = src;
			else if ( id == "ladspadir" ) ladspaPath_ = src;
		}

		// Audio.
		QDomElement audioElm = root.firstChildElement( "audio" );
		std::string enableStr = audioElm.attribute( "enable" ).toStdString();
		int enable = 0;
		if ( enableStr != "" ) enable = QString::fromStdString( enableStr ).toInt();
		enableSound_ = enable;
		if (enable == 0) {
			setDriverByName( "silent" );
			doEnableSound = false;
		} else {
			doEnableSound = true;
		}
		QDomElement driverElm = root.firstChildElement( "driver" );
		if ( doEnableSound ) {		
			setDriverByName( driverElm.attribute("name").toStdString() );
		} 

		// Alsa.  
		if ( _pOutputDriver->info().name() == "alsa" ) {
			// FIXME: what if alsa settings are missing from xml file?
			QDomElement alsaElm = root.firstChildElement( "alsa" ); 
			std::string deviceName = alsaElm.attribute("device").toStdString();
			std::map< std::string, AudioDriver*>::iterator it = driverMap_.begin();
			if ( ( it = driverMap_.find( "alsa" ) ) != driverMap_.end() ) {
				psy::core::AudioDriverSettings settings = it->second->settings();
				settings.setDeviceName( deviceName );
				it->second->setSettings( settings );
			}		
		}
	}

	doEnableSound = true;
}
