// -*- mode:c++; indent-tabs-mode:t -*-
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

#if defined PSYCLE__LIBXMLPP_AVAILABLE
	#include <libxml++/parsers/domparser.h>
#else
	//no need to error. #error none of the supported xml parser libs appear to be available
#endif

Configuration::Configuration()
{
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

	{
		char const * const env(std::getenv("PSYCLE_PATH"));
		if(env) pluginPath_ = env;
	}

	{
		char const * const env(std::getenv("LADSPA_PATH"));
		if(env) ladspaPath_ = env;
	}

	loadConfig();
}
Configuration::~Configuration()
{
	std::map< std::string, AudioDriver*>::iterator it = driverMap_.begin();
	while ( it != driverMap_.end())
	{
		delete(it->second);
		++it;
	}
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
		std::cerr << "psycle: configuration: audio driver not found: " << driverName << ", setting fallback: " << _pSilentDriver->info().name() << "\n";
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
			if (path.length()!=0) {
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
	try {
		#if defined PSYCLE__LIBXMLPP_AVAILABLE
			xmlpp::DomParser parser;
			//parser.set_validate();
			parser.set_substitute_entities(); // We just want the text to be resolved/unescaped automatically.
			parser.parse_file(path);
			if(parser) {
				xmlpp::Element const & root_element(*parser.get_document()->get_root_node()); // deleted by xmlpp::DomParser
				// paths
				{
					xmlpp::Node::NodeList const paths(root_element.get_children("path"));
					for(xmlpp::Node::NodeList::const_iterator i(paths.begin()); i != paths.end(); ++i) {
						xmlpp::Element const & path(dynamic_cast<xmlpp::Element const &>(**i));
						xmlpp::Attribute const * const id_attribute(path.get_attribute("id"));
						if(!id_attribute) std::cerr << "psycle: configuration: expected id attribute in path element\n";
						else {
							std::string id(id_attribute->get_value());
							xmlpp::Attribute const * const src_attribute(path.get_attribute("src"));
							if(!src_attribute) std::cerr << "psycle: configuration: expected src attribute in path element\n";
							else {
								std::string src(src_attribute->get_value());
								if(id == "plugindir") pluginPath_ = src;
								else if (id == "ladspadir") ladspaPath_ = src;
							}
						}
					}
				}
				// audio
				{
					// enable
					{
						xmlpp::Node::NodeList const audio_nodes(root_element.get_children("audio"));
						if(audio_nodes.begin() != audio_nodes.end()) {
							xmlpp::Element const & audio(dynamic_cast<xmlpp::Element const &>(**audio_nodes.begin()));
							xmlpp::Attribute const * const enable_attribute(audio.get_attribute("enable"));
							if(!enable_attribute) std::cerr << "psycle: configuration: expected enable attribute in audio element\n";
							else {
								std::string enable(enable_attribute->get_value());
								if(enable != "" && enable != "0") {
									enableSound_ = true;
									doEnableSound = true;
								} else {
									enableSound_ = false;
									setDriverByName("silent");
									doEnableSound = false;
								}
							}
						}
					}
					// driver
					{
						xmlpp::Node::NodeList const driver_nodes(root_element.get_children("driver"));
						if(driver_nodes.begin() != driver_nodes.end()) {
							xmlpp::Element const & driver(dynamic_cast<xmlpp::Element const &>(**driver_nodes.begin()));
							xmlpp::Attribute const * const name_attribute(driver.get_attribute("name"));
							if(!name_attribute) std::cerr << "psycle: configuration: expected name attribute in driver element\n";
							else {
								std::string name(name_attribute->get_value());
								if(doEnableSound) setDriverByName(name);
							}
						}
					}
					// alsa driver
					if(_pOutputDriver->info().name() == "alsa") {
						xmlpp::Node::NodeList const alsa_nodes(root_element.get_children("alsa"));
						if(alsa_nodes.begin() != alsa_nodes.end()) {
							xmlpp::Element const & alsa(dynamic_cast<xmlpp::Element const &>(**alsa_nodes.begin()));
							xmlpp::Attribute const * const device_attribute(alsa.get_attribute("device"));
							if(!device_attribute) std::cerr << "psycle: configuration: expected device attribute in alsa element\n";
							else {
								std::string device(device_attribute->get_value());
								std::map<std::string, AudioDriver*>::iterator i(driverMap_.find("alsa"));
								if(i != driverMap_.end()) {
									psy::core::AudioDriver & audiodriver(*i->second);
									psy::core::AudioDriverSettings settings(audiodriver.settings()); ///\todo why do we do a copy?
									settings.setDeviceName(device);
									audiodriver.setSettings(settings); ///\todo why do we copy?
								}
							}
						}
					}
				}
			}
		#else
			//no need to error. #error none of the supported xml parser libs appear to be available
		#endif
		doEnableSound = true;
	} catch(std::exception const & e) {
		std::cerr << "psycle: configuration: exception while parsing: " << e.what() << "\n";
	}
}
