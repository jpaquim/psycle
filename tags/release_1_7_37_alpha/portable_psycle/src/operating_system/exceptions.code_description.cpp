///\file
///\brief
#include <project.private.hpp>
#define OPERATING_SYSTEM__EXCEPTIONS__CODE_DESCRIPTION
#include "exceptions/code_description.hpp"
#include <string>
#include <sstream>
#include <windows.h>
namespace operating_system
{
	namespace exceptions
	{
		const std::string code_description(const unsigned long int & code) throw()
		{
			char * error_message_pointer(0);
			::FormatMessage // ouch! plain old c api style, this is ugly...
			(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				0, // module to get message from. 0 means system.
				code,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
				reinterpret_cast<char*>(&error_message_pointer), // we *must* hard-cast! this seems to be a hack to extend an originally badly designed api... there is no other way to do it
				0,
				0
			);
			std::ostringstream s;
			s << "microsoft api error: " << code << " 0x" << std::hex << code << ": " << error_message_pointer;
			::LocalFree(error_message_pointer);
			return s.str();
		}
	}
}
