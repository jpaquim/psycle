// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation universalis::operating_system::loggers
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "loggers.hpp"
namespace universalis
{
	namespace operating_system
	{
		void logger::log(int const & level, std::string const & string) throw()
		{
			#if !defined DIVERSALIS__COMPILER__MICROSOFT || DIVERSALIS__COMPILER__VERSION__MAJOR > 7
				//\todo this is crashing here at runtime with msvc7.1
				// some static var doesn't seem to get initialized properly
				boost::mutex::scoped_lock lock(mutex());
				do_log(level, string);
			#endif
		}

		namespace loggers
		{
			multiplex_logger multiplex_logger::singleton_;
			
			bool multiplex_logger::add(logger & logger)
			{
				boost::mutex::scoped_lock lock(mutex());
				iterator i(std::find(begin(), end(), &logger));
				if(i != end()) return false;
				push_back(&logger);
				return true;
			}

			bool multiplex_logger::remove(logger const & logger)
			{
				boost::mutex::scoped_lock lock(mutex());
				iterator i(std::find(begin(), end(), &logger));
				if(i == end()) return false;
				erase(i);
				return true;
			}
			
			void multiplex_logger::do_log(int const & level, std::string const & string) throw()
			{
				for(iterator i(begin()) ; i != end() ; ++i) (**i).log(level, string);
			}

			stream_logger stream_logger::default_logger_(std::cout /* std::clog is unbuffered by default, this can be changed at runtime tho */);

			stream_logger::stream_logger(std::ostream & ostream) : ostream_(ostream) {}

			// [bohan] msvc-7.1 crashes if we put this function in the implementation file instead of inlined in the header.
			void stream_logger::do_log(int const & level, std::string const & string) throw()
			{
				// ansi terminal
				int const static color [] = {0, 2, 6, 1, 5, 3, 4, 7};
				try
				{
					ostream() << "\033[1;3" << color[level % sizeof color] << "mlogger: " << level << ": \033[0m" << string << '\n';
				}
				catch(...)
				{
					// oh dear!
					// report the error to std::cerr ...
					std::cerr << UNIVERSALIS__COMPILER__LOCATION << "\033[1;31mlogger crashed!\033[0m" << std::endl;
					// ... and fallback to std::clog
					std::clog << "\033[1;3" << color[level % sizeof color] << "mlogger: " << level << ": \033[0m" << string << '\n';
				}
			}
		}
	}
}
