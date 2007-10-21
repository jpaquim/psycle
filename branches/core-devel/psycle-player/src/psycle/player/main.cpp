#include "configuration.hpp"
#include <psycle/core/player.h>
#include <psycle/core/song.h>
#include <iostream>
#include <string>
#include <sstream>

void usage() {
		std::cerr <<
			"usage: psycle-player [options] [--input-file] <song file name>\n"
			"options:\n"
			" -odrv, --output-driver <name>   name of the output driver to use.\n"
			"                                 examples: silent, alsa, jack, esd, dsound, mmewaveout\n"
			"\n"
			" -odev, --output-device <name>   name of the output device the driver should use.\n"
			"                                 The default driver will be used if this option is not specified.\n"
			"                                 examples for alsa: default, hw:0, plughw:0, pulse.\n"
			//"                                 examples for gstreamer: autoaudiosink, gconfaudiosink.\n"
			//"                                 examples for pulseaudio: hostname:port\n"
			//"                                 examples for esound: hostname:port\n"
			"\n"
			" -of,   --output-file <riff wave file name>\n"
			"                                 name of the output file to render to in riff-wave format.\n"
			"\n"
			" -if,   --input-file <song file name>\n"
			"                                 name of the song file to play.\n"
			"\n"
			"        --help                   display this help and exit.\n"
			"        --version                output version information and exit."
			;
}

int main(int argument_count, char * arguments[]) {
	if(argument_count < 2) {
		usage();
		return 1;
	}
	
	std::string input_file_name;
	std::string output_driver_name("auto");
	std::string output_device_name("auto");
	std::string output_file_name;
	
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
			case tokens::output_file_name:
				output_file_name = s;
				token = tokens::none;
				break;
			default:
				if(s == "-odrv" || s == "--output-driver") token = tokens::output_driver_name;
				else if(s == "-odev" || s == "--output-device") token = tokens::output_device_name;
				else if(s == "-of" || s == "--output-file") token = tokens::output_file_name;
				else if(s == "-if" || s == "--input-file") token = tokens::input_file_name;
				else if(s == "--help") {
					usage();
					return 1;
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

	std::cout << output_driver_name << " " << output_file_name << "\n";
	
	psy::core::Player player;

	
	if(output_file_name.length()) {
		std::cout << "psycle: player: setting output file name to: " << output_file_name << "\n";
		player.setFileName(output_file_name);
	}

	Configuration configuration;

	if(!configuration.pluginPath().length())
		std::cerr << "psycle: player: native plugin path not configured. You can set the PSYCLE_PATH environment variable.\n";
	else
		std::cout << "psycle: player: native plugins are looked for in: " << configuration.pluginPath() << "\n";

	if(!configuration.ladspaPath().length())
		std::cerr << "psycle: player: ladspa plugin path not configured. You can set the LADSPA_PATH environment variable.\n";
	else
		std::cout << "psycle: player: ladspa plugins are looked for in: " << configuration.ladspaPath() << "\n";

	if(output_driver_name.length()) {
		std::cout << "psycle: player: setting output driver name to: " << output_driver_name << "\n";
		configuration.setDriverByName(output_driver_name);
	}

	player.setDriver(*configuration._pOutputDriver); ///\todo needs a getter
	
	psy::core::Song song(&player);
	
	if(output_file_name.length()) player.startRecording();
	
	if(input_file_name.length()) {
		std::cout << "psycle: player: loading song file: " << input_file_name << "\n";
		if(!song.load(configuration.pluginPath(), input_file_name)) {
			std::cerr << "psycle: player: could not load song file: " << input_file_name << "\n";
			return 2;
		}

		///\todo this is not intuitive... we pass the player when creating the song, and now we have to pass the song to the player?
		player.song(&song);

		player.start(0);
		std::cout << "psycle: player: playing...\n";

		std::cout << "psycle: player: (enter anything to stop)\n";
		std::string s; std::cin >> s;
		
    std::cout << "psycle: player: stopping at position " << player.playPos() << "." << std::endl;
		player.stop();
		configuration.setDriverByName("silent");
		player.setDriver(*configuration._pOutputDriver);
	}

	if(output_file_name.length()) player.stopRecording();
	
	return 0;
}
