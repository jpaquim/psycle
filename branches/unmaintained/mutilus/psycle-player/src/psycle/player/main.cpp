#include "configuration.hpp"
#include <psycle/core/player.h>
#include <psycle/core/song.h>
#include <iostream>
#include <string>
#include <sstream>

int main(int argument_count, char * arguments[]) {
	if(argument_count < 2) {
		std::cerr << "psycle: player: usage: psycle-player <song file name>\n";
		return 1;
	}
	
	std::string input_file_name;
	std::string output_device_name("auto");
	std::string output_file_name;
	
	struct tokens { enum type {
		none,
		input_file_name,
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
			case tokens::output_device_name:
				output_device_name = s;
				token = tokens::none;
				break;
			case tokens::output_file_name:
				output_file_name = s;
				token = tokens::none;
				break;
			default:
				if(s == "-od" || s == "--output-device") token = tokens::output_device_name;
				else if(s == "-of" || s == "--output-file") token = tokens::output_file_name;
				else if(s == "-if" || s == "--input-file") token = tokens::input_file_name;
				else {
					input_file_name = s;
					token = tokens::none;
				}
		}
	}

	std::cout << output_device_name << " " << output_file_name << "\n";
	
	psy::core::Player player;

	
	if(output_file_name.length()) {
		std::cout << "psycle: player: setting output file name to: " << output_file_name << "\n";
		player.setFileName(output_file_name);
	}

	Configuration configuration;
	std::cout << "psycle: player: plugins are looked for in: " << configuration.pluginPath() << "\n";

	if(output_device_name.length()) {
		std::cout << "psycle: player: setting output device name to: " << output_device_name << "\n";
		configuration.setDriverByName(output_device_name);
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
		
		player.stop();
		configuration.setDriverByName("silent");
		player.setDriver(*configuration._pOutputDriver);
	}

	if(output_file_name.length()) player.stopRecording();
	
	return 0;
}
