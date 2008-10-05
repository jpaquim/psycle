#include "configuration.hpp"
#include <psycle/core/machinefactory.h>
#include <psycle/core/song.h>
#include <psycle/core/player.h>
#include <psycle/audiodrivers/audiodriver.h>
#include <iostream>
#include <string>
#include <sstream>

#if defined _WIN32
	#include <windows.h> // for Sleep(ms)
#else
	#include <unistd.h> // for sleep(s)
#endif

#if defined _WIN32
	#define PSYCLE__PLAYER__EOF "z" // ctrl+z == EOF
#else
	#define PSYCLE__PLAYER__EOF "d" // ctrl+d == EOF
#endif

using namespace psy::core;
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
					", asio"
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
		std::cerr << "psycle: player: native plugin path not configured. You can set the PSYCLE_PATH environment variable.\n";
	} else {
		std::cout << "psycle: player: native plugins are looked for in: " << configuration.pluginPath() << '\n';
	}
	if(!configuration.ladspaPath().length()) {
		std::cerr << "psycle: player: ladspa plugin path not configured. You can set the LADSPA_PATH environment variable.\n";
	} else {
		std::cout << "psycle: player: ladspa plugins are looked for in: " << configuration.ladspaPath() << '\n';
	}

	Player & player(Player::singleton());
	// If you use a derived pluginfinder class, instantiate it before this call, and pass its address to the machinefactory Initialize function.
	MachineFactory & factory(MachineFactory::getInstance());
	factory.Initialize(&player);
	factory.setPsyclePath(configuration.pluginPath());
	factory.setLadspaPath(configuration.ladspaPath());

	if(output_driver_name.length()) {
		std::cout << "psycle: player: setting output driver name to: " << output_driver_name << '\n';
		configuration.set_driver_by_name(output_driver_name);
	}
	psy::core::AudioDriver & output_driver(configuration.output_driver());

	player.setDriver(output_driver);

	///\todo it seems player.setDriver resets the settings to their default, so we need to set the settings after.
	if(output_device_name.length()) {
		std::cout << "psycle: player: setting output driver device name to: " << output_device_name << '\n';
		psy::core::AudioDriverSettings settings(player.driver().settings()); ///\todo why do we do a copy?
		settings.setDeviceName(output_device_name);
		player.driver().setSettings(settings); ///\todo why do we copy?
	}

	if(output_file_name.length()) {
		std::cout << "psycle: player: setting output file name to: " << output_file_name << '\n';
		player.setFileName(output_file_name);
	}
	player.driver().Enable(false); ///\todo setDriver enables it and we disable it just after, why?

	CoreSong song;
	if(input_file_name.length()) {
		std::cout << "psycle: player: loading song file: " << input_file_name << '\n';
		if(!song.load(input_file_name)) {
			std::cerr << "psycle: player: could not load song file: " << input_file_name << '\n';
			return 2;
		}
		player.song(&song);

		// [JosepMa] workaround some "bugs" where machines are not well setup until a call to work is issued
		// [JosepMa] (i.e. some machines read the samplerate in the work call, or similar things)
		{ int samples = 256; player.Work(&player, samples); }

		if(output_file_name.length()) player.startRecording();
		player.driver().Enable(true);
		player.start(0);
		std::cout << "psycle: player: playing...\n";

		if(wait) {
			std::cout << "psycle: player: press enter or ctrl+" PSYCLE__PLAYER__EOF " (EOF) to stop.\n";
			std::string s; std::getline(std::cin, s);
		} else {
			///\todo we need to find a way to wait until the song is finished with the sequence.
			std::cout << "psycle: player: currently, we have no way find out when the song is finished with the sequence... for now, please press enter or ctrl+" PSYCLE__PLAYER__EOF " (EOF) to stop.\n";
			std::string s; std::getline(std::cin, s);
		}

		std::cout << "\npsycle: player: stopping at position " << player.playPos() << ".\n";
		player.stop();
		if(output_file_name.length()) player.stopRecording();
	}
	player.driver().Enable(false);
	#if defined _WIN32
		Sleep(1000);
	#else
		sleep(1);
	#endif

	return 0;
}
