// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright 2006 Johan Boule <bohan@jabber.org>
// Copyright 2006 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::player
#include <packageneric/pre-compiled.private.hpp>
#include <universalis/operating_system/paths.hpp>
#include <universalis/operating_system/terminal.hpp>
#include <iostream>
#include <string>
namespace psycle
{
	namespace player
	{
	}
}

int main()
{
	universalis::operating_system::terminal terminal;
	std::cout << "package name...... " << universalis::operating_system::paths::package::name() << std::endl;
	std::cout << "package version... " << universalis::operating_system::paths::package::version::string() << std::endl;
	std::cout << "bin............... " << universalis::operating_system::paths::bin().string() << std::endl;
	std::cout << "lib............... " << universalis::operating_system::paths::lib().string() << std::endl;
	std::cout << "share............. " << universalis::operating_system::paths::share().string() << std::endl;
	std::cout << "var............... " << universalis::operating_system::paths::var().string() << std::endl;
	std::cout << "etc............... " << universalis::operating_system::paths::etc().string() << std::endl;
	std::cout << "home.............. " << universalis::operating_system::paths::home().string() << std::endl;
	std::cout <<  std::endl;
	std::cout << "This program does nothing. Type anything to quit." << std::endl << std::endl;
	std::string s;
	std::cin >> s;
}
