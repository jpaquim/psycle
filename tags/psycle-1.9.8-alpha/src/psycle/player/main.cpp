// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright 2006 Johan Boule <bohan@jabber.org>
// Copyright 2006 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::player
#include <packageneric/pre-compiled.private.hpp>
#include <universalis/operating_system/paths.hpp>
#include <universalis/operating_system/terminal.hpp>
#include <boost/static_assert.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>
#include <string>
namespace psycle
{
	namespace player
	{
	}
}

int main(unsigned int const argument_count, char * const arguments[])
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

	if(argument_count == 1) return 0;

	std::string file_string(arguments[0]);
	boost::filesystem::path file_path(file_string, boost::filesystem::native);
	std::cout << file_path.string() << std::endl;
}
