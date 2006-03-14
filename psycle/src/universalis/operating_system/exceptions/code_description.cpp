// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief
#include <universalis/detail/project.private.hpp>
#include "code_description.hpp"
#include <cstring> // iso std::strerror
#include <boost/thread/mutex.hpp>
#include <sstream>
namespace universalis
{
	namespace operating_system
	{
		namespace exceptions
		{
			///\todo move to standard_library
			std::string code_description(int const & code) throw()
			{
				std::ostringstream s;
				s
					<< "error: "
					<< "standard: "
					<< code
					<< " 0x" << std::hex << code << ": ";
				static boost::mutex mutex;
				boost::mutex::scoped_lock lock(mutex);
				s << std::strerror(code);
				return s.str();
			}

			///\todo unifiy with processor::exception
			#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
				std::string microsoft_code_description(::DWORD const /* or ::HRESULT in some cases */ & code) throw()
				{
					// ::DWORD, or ::HRESULT in some cases
					assert(sizeof(::HRESULT) <= sizeof(::DWORD));
					std::ostringstream s;
					s
						<< "error: "
						<< "microsoft api: "
						<< code
						<< " 0x" << std::hex << code << ": ";
					char * error_message_pointer(0);
					if
					(
						::FormatMessage // ouch! plain old c api style, this is ugly...
						(
							FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
							0, // module to get message from. 0 means system.
							code,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
							reinterpret_cast<char*>(&error_message_pointer), // we *must* hard-cast! this seems to be a hack to extend an originally badly designed api... there is no other way to do it
							0,
							0
						)
					)
					{
						s << error_message_pointer;
					}
					else
					{
						s << "unkown code !!!";
					}
					::LocalFree(error_message_pointer);
					return s.str();
				}
			#endif
		}
	}
}
// arch-tag: 46b984ad-40f8-47f5-9cee-d38c127734ca
