///\file
///\see http://www.opengroup.org/platform/single_unix_specification/uploads/40/6355/n1093.pdf
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include "restrict.hpp"
#if !defined __STDC_SECURE_LIB__ && (!defined COMPILER__MICROSOFT || COMPILER__VERSION__MAJOR < 8)
	#include <cstdio>
	#include <cwchar>
	#include <cstdarg>
	namespace std
	{
		FILE inline * freopen_s(FILE ** result, char const * restrict path, char const * restrict mode, FILE * restrict stream) throw()
		{
			return *result = freopen(path, mode, stream);
		}

		int vsprintf_s(char * string, size_t max_length, char const * format, va_list values)
		{
			#if \
			(\
				defined OPERATING_SYSTEM__LINUX || \
				(defined OPERATING_SYSTEM__BSD && OPERATING_SYSTEM__VERSION__MAJOR >= 4 && OPERATING_SYSTEM__VERSION__MINOR >= 4) || \
				defined OPEPATING_SYSTEM__MICROSOFT \
			)
				return vsnprintf(string, max_length, format, values);
			#else
				return vsprintf(string, format, values);
			#endif
		}

		int vswprintf_s(wchar_t * string, size_t max_length, wchar_t const * format, va_list values)
		{
			return vswprintf(string, max_length, format, values);
		}
	}
#endif
