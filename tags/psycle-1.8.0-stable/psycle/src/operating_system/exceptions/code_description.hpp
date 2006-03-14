#pragma once
#include <string>
#if defined OPERATING_SYSTEM__EXCEPTIONS__CODE_DESCRIPTION
	#include LIBRARY__EXPORT
#else
	#include LIBRARY__IMPORT
#endif
///\file
///\brief
namespace operating_system
{
	namespace exceptions
	{
		const std::string LIBRARY code_description
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
