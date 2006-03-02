#pragma once
#include <string>
#include <universalis/compiler.hpp>
///\file
///\brief
namespace operating_system
{
	namespace exceptions
	{
		const std::string UNIVERSALIS__COMPILER__DEPRECATED("universalis") code_description
			(
				#if defined OPERATING_SYSTEM__POSIX
				#elif defined OPERATING_SYSTEM__MICROSOFT
					const unsigned long int & code = ::GetLastError()
				#else
					#error unsupported operating system
				#endif
			) throw();
	}
}
