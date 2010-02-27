// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "configuration.hpp"
#include <psycle/audiodrivers/audiodriver.h>
#include <psycle/audiodrivers/wavefileout.h>
#include <universalis/os/fs.hpp>
#include <universalis/os/loggers.hpp>

#if defined PSYCLE__SYDNEY_AVAILABLE
	#include <psycle/audiodrivers/sydneyout.hpp>
#endif
#if defined PSYCLE__GSTREAMER_AVAILABLE
	#include <psycle/audiodrivers/gstreamerout.h>
#endif
#if defined PSYCLE__JACK_AVAILABLE
	#include <psycle/audiodrivers/jackout.h>
#endif
#if defined PSYCLE__ALSA_AVAILABLE
	#include <psycle/audiodrivers/alsaout.h>
#endif
#if defined PSYCLE__ESOUND_AVAILABLE
	#include <psycle/audiodrivers/esoundout.h>
#endif
#if defined PSYCLE__NET_AUDIO_AVAILABLE
	#include <psycle/audiodrivers/netaudioout.h>
#endif
#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
	#include <psycle/audiodrivers/microsoftdirectsoundout.h>
#endif
#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
	#include <psycle/audiodrivers/microsoftmmewaveout.h>
#endif
#if defined PSYCLE__STEINBERG_ASIO_AVAILABLE
	#include <psycle/audiodrivers/asiointerface.h>
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

namespace psycle { namespace player {

using namespace audiodrivers;
namespace loggers = universalis::os::loggers;

Configuration::Configuration()
:
	enable_sound_()
{
	add_driver(*(dummy_driver_ = new DummyDriver));
	set_driver_by_name("dummy");

	add_driver(*new WaveFileOut);

	#if defined PSYCLE__ALSA_AVAILABLE
		add_driver(*new AlsaOut);
		// use alsa by default
		set_driver_by_name("alsa");
		enable_sound_ = true;
	#endif
	#if defined PSYCLE__JACK_AVAILABLE
		add_driver(*new JackOut);
	#endif
	#if defined PSYCLE__ESOUND_AVAILABLE
		add_driver(*new ESoundOut);
	#endif
	#if defined PSYCLE__GSTREAMER_AVAILABLE
		add_driver(*new GStreamerOut);
	#endif
	#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
		add_driver(*new MsDirectSound);
		// use dsound by default
		set_driver_by_name("dsound");
		enable_sound_ = true;
	#endif
	#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
		add_driver(*new MsWaveOut);
	#endif
	#if defined PSYCLE__STEINBERG_ASIO_AVAILABLE
		add_driver(*new ASIOInterface);
	#endif
	#if defined PSYCLE__NET_AUDIO_AVAILABLE
		add_driver(*new NetAudioOut);
	#endif


	{ char const * const env(std::getenv("PSYCLE_PATH"));
		if(env) pluginPath_ = env;
	}

	{ char const * const env(std::getenv("LADSPA_PATH"));
		if(env) ladspaPath_ = env;
	}

	loadConfig();
}

Configuration::~Configuration() {
	for(std::map<std::string, AudioDriver*>::iterator i(driver_map_.begin()), e(driver_map_.end()); i != e; ++i)
		delete i->second;
}

void Configuration::add_driver(AudioDriver & driver) {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << "psycle: player: config: audio driver registered: " <<  driver.info().name();
		loggers::trace()(s.str());
	}
	driver_map_[driver.info().name()] = &driver;
}

void Configuration::set_driver_by_name(std::string const & driver_name) {
	std::map< std::string, AudioDriver*>::iterator it = driver_map_.begin();
	if((it = driver_map_.find(driver_name)) != driver_map_.end()) {
		// driver found
		output_driver_ = it->second;
		if(loggers::trace()()) {
			std::ostringstream s;
			s << "psycle: player: config: audio driver set to: " << driver_name;
			loggers::trace()(s.str());
		}
	} else {
		if(loggers::exception()()) {
			std::ostringstream s;
			s << "psycle: player: config: audio driver not found: " << driver_name << ", setting fallback: " << dummy_driver_->info().name();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		// driver not found, set silent default driver
		output_driver_ = dummy_driver_;
	}
}

void Configuration::loadConfig() {
	boost::filesystem::path const path(universalis::os::fs::home_app_local("psycle") / "config.xml");
	if(boost::filesystem::exists(path)) {
		try {
			loadConfig(path.file_string());
		} catch( std::exception const & e ) {
			if(loggers::exception()()) {
				std::ostringstream s;
				s << "psycle: player: config: error: " << e.what();
				loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	} else {
		#if 0
			#if defined PSYCLE__INSTALL_PATHS__CONFIGURATION
				path = PSYCLE__INSTALL_PATHS__CONFIGURATION "/psycle/config.xml";
			#endif
			if(boost::filesystem::exists(path)) {
				try {
					loadConfig(path.file_string());
				} catch(std::exception const & e) {
					if(loggers::exception()()) {
						std::ostringstream s;
						s << "psycle: player: config: error: " << e.what();
						loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
			}
		#endif
	}
}

void Configuration::loadConfig(std::string const & path) {
	try {
		#if defined PSYCLE__LIBXMLPP_AVAILABLE
			xmlpp::DomParser parser;
			//parser.set_validate();
			parser.set_substitute_entities(); // We just want the text to be resolved/unescaped automatically.
			parser.parse_file(path);
			if(parser) {
				std::ostringstream msg;
				xmlpp::Element const & root_element(*parser.get_document()->get_root_node()); // deleted by xmlpp::DomParser
				{ // paths
					xmlpp::Node::NodeList const paths(root_element.get_children("path"));
					for(xmlpp::Node::NodeList::const_iterator i(paths.begin()); i != paths.end(); ++i) {
						xmlpp::Element const & path(dynamic_cast<xmlpp::Element const &>(**i));
						xmlpp::Attribute const * const id_attribute(path.get_attribute("id"));
						if(!id_attribute) msg << "expected id attribute in path element\n";
						else {
							std::string id(id_attribute->get_value());
							xmlpp::Attribute const * const src_attribute(path.get_attribute("src"));
							if(!src_attribute) msg << "expected src attribute in path element\n";
							else {
								std::string src(src_attribute->get_value());
								if(id == "plugindir") pluginPath_ = src;
								else if(id == "ladspadir") ladspaPath_ = src;
							}
						}
					}
				}
				{ // audio
					{ // enable
						xmlpp::Node::NodeList const audio_nodes(root_element.get_children("audio"));
						if(audio_nodes.begin() != audio_nodes.end()) {
							xmlpp::Element const & audio(dynamic_cast<xmlpp::Element const &>(**audio_nodes.begin()));
							xmlpp::Attribute const * const enable_attribute(audio.get_attribute("enable"));
							if(!enable_attribute) msg << "expected enable attribute in audio element\n";
							else {
								std::string enable(enable_attribute->get_value());
								if(enable != "" && enable != "0") {
									enable_sound_ = true;
									do_enable_sound_ = true;
								} else {
									enable_sound_ = false;
									set_driver_by_name("silent");
									do_enable_sound_ = false;
								}
							}
						}
					}
					{ // driver
						xmlpp::Node::NodeList const driver_nodes(root_element.get_children("driver"));
						if(driver_nodes.begin() != driver_nodes.end()) {
							xmlpp::Element const & driver(dynamic_cast<xmlpp::Element const &>(**driver_nodes.begin()));
							xmlpp::Attribute const * const name_attribute(driver.get_attribute("name"));
							if(!name_attribute) msg << "expected name attribute in driver element\n";
							else {
								std::string name(name_attribute->get_value());
								if(do_enable_sound_) set_driver_by_name(name);
							}
						}
					}
					// alsa driver
					if(output_driver_->info().name() == "alsa") {
						xmlpp::Node::NodeList const alsa_nodes(root_element.get_children("alsa"));
						if(alsa_nodes.begin() != alsa_nodes.end()) {
							xmlpp::Element const & alsa(dynamic_cast<xmlpp::Element const &>(**alsa_nodes.begin()));
							xmlpp::Attribute const * const device_attribute(alsa.get_attribute("device"));
							if(!device_attribute) msg << "expected device attribute in alsa element\n";
							else {
								std::string device(device_attribute->get_value());
								std::map<std::string, AudioDriver*>::iterator i(driver_map_.find("alsa"));
								if(i != driver_map_.end()) {
									AudioDriver & audiodriver(*i->second);
									AudioDriverSettings settings(audiodriver.playbackSettings()); ///\todo why do we do a copy?
									settings.setDeviceName(device);
									audiodriver.setPlaybackSettings(settings); ///\todo why do we copy?
								}
							}
						}
					}
				}
				if(loggers::warning()()) {
					std::string const s = msg.str();
					if(s.length()) {
						std::ostringstream oss;
						oss << "psycle: player: config: nonconforming config file:\n" << s;
						loggers::warning()(oss.str());
					}
				}
			}
		#else
			//no need to error. #error none of the supported xml parser libs appear to be available
		#endif
		do_enable_sound_ = true;
	} catch(std::exception const & e) {
		if(loggers::exception()()) {
			std::ostringstream s;
			s << "psycle: player: config: exception while parsing: " << e.what();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}
}

}}
