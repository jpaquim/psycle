#include "configuration.hpp"
#include <psycle/core/machinefactory.h>
#include <psycle/core/song.h>
#include <psycle/core/player.h>
#include <psycle/audiodrivers/audiodriver.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace psy::core;
void usage() {
		std::cerr <<
			"Usage: psycle-player [options] [--input-file] <song file name>\n"
			"Plays a Psycle song file.\n"
			"\n"
			" -odrv, --output-driver <name>   name of the output driver to use.\n"
			"                                 examples: silent, alsa, jack, esd, dsound, mmewaveout\n"
			"\n"
			" -odev, --output-device <name>   name of the output device the driver should use.\n"
			"                                 The default device will be used if this option is not specified.\n"
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
			"        --version                output version information and exit.\n"
			"\n"
			"Report bugs to the bug tracker at http://sourceforge.net/projects/psycle\n"
			;
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
		std::cout << "psycle: player: native plugins are looked for in: " << configuration.pluginPath() << "\n";
	}
	if(!configuration.ladspaPath().length()) {
		std::cerr << "psycle: player: ladspa plugin path not configured. You can set the LADSPA_PATH environment variable.\n";
	} else {
		std::cout << "psycle: player: ladspa plugins are looked for in: " << configuration.ladspaPath() << "\n";
	}

	Player &player = *Player::Instance();
	// If you use a derived pluginfinder class, instantiate it before this call, and pass its address to the machinefactory Initialize function.
	MachineFactory& mfactory = MachineFactory::getInstance();
	mfactory.Initialize(&player);
	mfactory.setPsyclePath(configuration.pluginPath());
	mfactory.setLadspaPath(configuration.ladspaPath());
	
	if(output_driver_name.length()) {
		std::cout << "psycle: player: setting output driver name to: " << output_driver_name << "\n";
		configuration.setDriverByName(output_driver_name);
	}
	psy::core::AudioDriver & output_driver(*configuration._pOutputDriver); ///\todo needs a getter

	if(output_device_name.length()) {
		std::cout << "psycle: player: setting output driver device name to: " << output_device_name << "\n";
		psy::core::AudioDriverSettings settings(output_driver.settings()); ///\todo why do we do a copy?
		settings.setDeviceName(output_device_name);
		output_driver.setSettings(settings); ///\todo why do we copy?
	}
	player.setDriver(output_driver);

	if(output_file_name.length()) {
		std::cout << "psycle: player: setting output file name to: " << output_file_name << "\n";
		player.setFileName(output_file_name);
	}
	// since driver is cloned, we cannot use output_driver!!!!
	player.driver().Enable(false);
	
	CoreSong song;
	if(input_file_name.length()) {
		std::cout << "psycle: player: loading song file: " << input_file_name << "\n";
		if(!song.load(input_file_name)) {
			std::cerr << "psycle: player: could not load song file: " << input_file_name << "\n";
			return 2;
		}
		player.song(&song);

		int itmp=256;
		player.Work(&player,itmp);

		if(output_file_name.length()) player.startRecording();
		// since driver is cloned, we cannot use output_driver!!!!
		player.driver().Enable(true);
		player.start(0);
		std::cout << "psycle: player: playing..." << std::endl;
		std::cout << "psycle: player: press any letter and enter to stop" << std::endl;

		std::string s; std::cin >> s;
		
		std::cout << std::endl << "psycle: player: stopping at position " << player.playPos() << "." << std::endl;
		player.stop();
		if(output_file_name.length()) player.stopRecording();
	}
	// since driver is cloned, we cannot use output_driver!!!!
	player.driver().Enable(false);
	sleep(1);
	//configuration.setDriverByName("silent");
	//player.setDriver(*configuration._pOutputDriver);
	
	return 0;
}
