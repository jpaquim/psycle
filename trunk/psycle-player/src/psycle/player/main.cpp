#include "configuration.hpp"
#include <psycle/core/player.h>
#include <psycle/core/song.h>
#include <iostream>
#include <string>
int main(int argument_count, char * arguments[])
{
	if(argument_count < 2)
	{
		std::cerr << "psycle: player: usage: psycle-player <song file name>\n";
		return 1;
	}
	
	psy::core::Player player;

	Configuration configuration;
	std::cout << "psycle: player: plugins are looked for in: " << configuration.pluginPath() << "\n";

	player.setDriver(*configuration._pOutputDriver); ///\todo needs a getter
	
	psy::core::Song song(&player);
	
	std::string song_file_name(arguments[1]);
	std::cout << "psycle: player: loading song file: " << song_file_name << "\n";
	if(!song.load(configuration.pluginPath(), song_file_name))
	{
		std::cerr << "psycle: player: could not load song file: " << song_file_name << "\n";
		return 2;
	}

	///\todo this is not intuitive... we pass the player when creating the song, and now we have to pass the song to the player?
	player.song(&song);

	player.start(0);
	std::cout << "psycle: player: playing...\n";

	std::cout << "psycle: player: (enter anything to stop)\n";
	std::string s; std::cin >> s;
	
	///\todo segfaults! ... this is time to implement proper thread synchronization
	//player.stop();

	return 0;
}
