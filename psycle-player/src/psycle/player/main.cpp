#include "configuration.h"
#include <psycore/player.h>
#include <psycore/song.h>
#include <iostream>
#include <string>
int main(int argument_count, char * arguments[])
{
	if(argument_count < 2)
	{
		std::cerr << "psycle: player: usage: psycle-player <song file name>\n";
		return 1;
	}
	
	Configuration configuration;
	std::cout << "psycle: player: plugins are looked for in: " << configuration.pluginPath() << "\n";
	
	psy::core::Player & player(*psy::core::Player::Instance());
	psy::core::Song song(&player);

	std::string song_file_name(arguments[1]);
	std::cout << "psycle: player: loading song file: " << song_file_name << "\n";
	if(!song.load(configuration.pluginPath(), song_file_name))
	{
		std::cerr << "psycle: player: could not load song file: " << song_file_name << "\n";
		return 2;
	}

	player.start(0);
	std::string s; std::cin >> s;
	
	///\todo segfaults! ... this is time to implement proper thread synchronization
	//player.stop();

	return 0;
}
