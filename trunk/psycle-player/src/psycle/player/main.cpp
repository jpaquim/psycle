// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "configuration.hpp"
#include <psycle/core/machinefactory.h>
#include <psycle/core/song.h>
#include <psycle/core/player.h>
#include <psycle/audiodrivers/audiodriver.h>
#include <diversalis/os.hpp>
#include <universalis/os/fs.hpp>
#include <universalis/os/loggers.hpp>
#include <universalis/stdlib/thread.hpp>
#include <universalis/os/thread_name.hpp>
#include <universalis/os/dyn_link.hpp>
#include <iostream>
#include <string>
#include <sstream>

#if defined DIVERSALIS__OS__MICROSOFT
	#define PSYCLE__PLAYER__EOF "z" // ctrl+z == EOF
#else
	#define PSYCLE__PLAYER__EOF "d" // ctrl+d == EOF
#endif

namespace psycle { namespace player {

using namespace core;
using namespace audiodrivers;
using namespace universalis::stdlib;
namespace loggers = universalis::os::loggers;

void usage() {
		std::cerr <<
			"Usage: psycle-player [options] [--input-file] <song file name>\n"
			"Plays a Psycle song file.\n\n"
			" -odrv, --output-driver <name>   name of the output driver to use.\n"
			"                                 available: dummy"
				///\todo simply use configuration.driver_map() to build the list
				#if defined PSYCLE__SYDNEY_AVAILABLE
					", sydney"
				#endif
				#if defined PSYCLE__GSTREAMER_AVAILABLE
					", gstreamer"
				#endif
				#if defined PSYCLE__JACK_AVAILABLE
					", jack"
				#endif
				#if defined PSYCLE__ALSA_AVAILABLE
					", alsa"
				#endif
				#if defined PSYCLE__ESOUND_AVAILABLE
					", esd"
				#endif
				#if defined PSYCLE__NET_AUDIO_AVAILABLE
					", netaudio"
				#endif
				#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
					", dsound"
				#endif
				#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
					", mmewaveout"
				#endif
				#if defined PSYCLE__STEINBERG_ASIO_AVAILABLE
					", asio2"
				#endif
			"\n\n"
			" -odev, --output-device <name>   name of the output device the driver should use.\n"
			"                                 The default device will be used if this option is not specified.\n"
				#if defined PSYCLE__SYDNEY_AVAILABLE
			"                                 examples for sydney: hostname:port\n"
				#endif
				#if defined PSYCLE__GSTREAMER_AVAILABLE
			"                                 examples for gstreamer: autoaudiosink, gconfaudiosink.\n"
				#endif
				#if defined PSYCLE__ALSA_AVAILABLE
			"                                 examples for alsa: default, hw:0, plughw:0, pulse.\n"
				#endif
				#if defined PSYCLE__ESOUND_AVAILABLE
			"                                 examples for esound: hostname:port\n"
				#endif
			"\n"
			" -of,   --output-file <riff wave file name>\n"
			"                                 name of the output file to render to in riff-wave format.\n\n"
			" -if,   --input-file <song file name>\n"
			"                                 name of the song file to play.\n\n"
			" -w,    --wait                   play until enter or ctrl+" PSYCLE__PLAYER__EOF " (EOF) is pressed.\n\n"
			"        --help                   display this help and exit.\n"
			"        --version                output version information and exit.\n\n"
			"Report bugs to the bug tracker at http://sourceforge.net/projects/psycle\n";
}

int main(int argument_count, char * arguments[]) {
	#if 1
	{
		using namespace universalis::os::fs;
		std::clog << home() << '\n';
		std::clog << home_app_local("psycle") << '\n';
		std::clog << home_app_roaming("psycle") << '\n';
	}
	#endif

	#if 0
	{
		using universalis::os::dyn_link::path_list_type;
		using universalis::os::dyn_link::lib_path;
		path_list_type p(lib_path());
		for(path_list_type::const_iterator i(p.begin()), e(p.end()); i != e; ++i) std::clog << *i << '\n';
		p.push_back("psycle");
		lib_path(p);
		p = lib_path();
		for(path_list_type::const_iterator i(p.begin()), e(p.end()); i != e; ++i) std::clog << *i << '\n';
	}
	#endif

	#if 0
	{
		std::string s; std::getline(std::cin, s);
		std::exit(1);
	}
	#endif

	{
		using namespace universalis::os::loggers;
		multiplex_logger::singleton().add(stream_logger::default_logger());
	}
	universalis::os::thread_name thread_name("main");

	if(argument_count < 2) {
		usage();
		return 1;
	}

	std::string input_file_name;
	std::string output_driver_name;
	std::string output_device_name;
	std::string output_file_name;
	bool wait(false);

	struct tokens { enum type {
		none,
		input_file_name,
		output_driver_name,
		output_device_name,
		output_file_name
	};};

	tokens::type token(tokens::none);

	for(int i(1); i < argument_count; ++i) {
		std::ostringstream ss; ss << arguments[i];
		std::string s = ss.str();
		switch(token) {
			case tokens::input_file_name:
				input_file_name = s;
				token = tokens::none;
				break;
			case tokens::output_driver_name:
				output_driver_name = s;
				token = tokens::none;
				break;
			case tokens::output_device_name:
				output_device_name = s;
				token = tokens::none;
				break;
			case tokens::output_file_name:
				output_file_name = s;
				token = tokens::none;
				break;
			default:
				if(s == "-odrv" || s == "--output-driver") token = tokens::output_driver_name;
				else if(s == "-odev" || s == "--output-device") token = tokens::output_device_name;
				else if(s == "-of" || s == "--output-file") token = tokens::output_file_name;
				else if(s == "-if" || s == "--input-file") token = tokens::input_file_name;
				else if(s == "-w" || s == "--wait") { wait = true; token = tokens::none; }
				else if(s == "--help") {
					usage();
					return 0;
				} else if(s == "--version") {
					std::cout << "psycle-player devel (built on " __DATE__ " " __TIME__ ")\n"; ///\todo need a real version
					return 0;
				} else if(s.length() && s[0] == '-') { // unrecognised option
					std::cerr << "error: unknown option: " << s << '\n';
					usage();
					return 1;
				} else {
					input_file_name = s;
					token = tokens::none;
				}
		}
	}
	Configuration configuration;
	if(!configuration.pluginPath().length()) {
		if(loggers::warning()())
			loggers::warning()("psycle: player: native plugin path not configured. You can set the PSYCLE_PATH environment variable.");
	} else {
		if(loggers::information()()) {
			std::ostringstream s;
			s << "psycle: player: native plugins are looked for in: " << configuration.pluginPath();
			loggers::information()(s.str());
		}
	}
	if(!configuration.ladspaPath().length()) {
		if(loggers::warning()())
			loggers::warning()("psycle: player: ladspa plugin path not configured. You can set the LADSPA_PATH environment variable.");
	} else {
		if(loggers::information()()) {
			std::ostringstream s;
			s << "psycle: player: ladspa plugins are looked for in: " << configuration.ladspaPath();
			loggers::information()(s.str());
		}
	}

	Player & player(Player::singleton());
	// If you use a derived pluginfinder class, instantiate it before this call, and pass its address to the machinefactory Initialize function.
	MachineFactory & factory(MachineFactory::getInstance());
	factory.Initialize(&player);
	factory.setPsyclePath(configuration.pluginPath());
	factory.setLadspaPath(configuration.ladspaPath());

	if(output_driver_name.length()) {
		if(loggers::information()()) {
			std::ostringstream s;
			s << "psycle: player: setting output driver name to: " << output_driver_name;
			loggers::information()(s.str());
		}
		configuration.set_driver_by_name(output_driver_name);
	}
	AudioDriver & output_driver(configuration.output_driver());

	player.setDriver(output_driver);

	///\todo it seems player.setDriver resets the settings to their default, so we need to set the settings after.
	///\todo not sure that's the case anymore
	if(output_device_name.length()) {
		if(loggers::information()()) {
			std::ostringstream s;
			s << "psycle: player: setting output driver device name to: " << output_device_name;
			loggers::information()(s.str());
		}
		AudioDriverSettings settings(player.driver().playbackSettings());
		settings.setDeviceName(output_device_name);
		player.driver().setPlaybackSettings(settings);
	}

	if(output_file_name.length()) {
		if(loggers::information()()) {
			std::ostringstream s;
			s << "psycle: player: setting output file name to: " << output_file_name;
			loggers::information()(s.str());
		}
		player.setFileName(output_file_name);
	}

	CoreSong song;
	if(input_file_name.length()) {
		//Song is assigned to player previous to load, since the plugins can ask information about
		//the song they are being loaded from.
		player.song(song);
		if(loggers::information()()) {
			std::ostringstream s;
			s << "psycle: player: loading song file: " << input_file_name;
			loggers::information()(s.str());
		}
		if(!song.load(input_file_name)) {
			if(loggers::exception()()) {
				std::ostringstream s;
				s << "psycle: player: could not load song file: " << input_file_name;
				loggers::exception()(s.str());
			}
			return 2;
		}

		///\todo since we don't have a way to wait until the song is finished with the sequence, we may as well just loop it
		player.setLoopSong();

		if(output_file_name.length()) {
			if(loggers::information()()) {
				std::ostringstream s;
				s << "psycle: player: starting to record to: " << output_file_name;
				loggers::information()(s.str());
			}
			player.startRecording();
		}
		
		//if(loggers::trace()()) loggers::trace()("psycle: player: enabling output driver", UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
		//player.driver().Enable(true);

		if(loggers::information()()) loggers::information()("psycle: player: playing...");
		output_driver.set_started(true);
		player.start(0);

		if(wait) {
			std::cout << "psycle: player: press enter or ctrl+" PSYCLE__PLAYER__EOF " (EOF) to stop.\n";
			std::string s; std::getline(std::cin, s);
		} else {
			///\todo we need to find a way to wait until the song is finished with the sequence.
			std::cout << "psycle: player: currently, we have no way find out when the song is finished with the sequence... for now, please press enter or ctrl+" PSYCLE__PLAYER__EOF " (EOF) to stop.\n";
			std::string s; std::getline(std::cin, s);
		}

		if(loggers::information()()) {
			std::ostringstream s;
			s << "psycle: player: stopping at position " << player.playPos();
			loggers::information()(s.str());
		}
		player.stop();
		
		if(output_file_name.length()) {
			if(loggers::information()()) {
				std::ostringstream s;
				s << "psycle: player: stopping recording to: " << output_file_name;
				loggers::information()(s.str());
			}
			player.stopRecording();
		}
	}
	player.driver().set_started(false);
	factory.Finalize();

	return 0;
}

}}

int main(int argument_count, char * arguments[]) {
	return psycle::player::main(argument_count, arguments);
}
