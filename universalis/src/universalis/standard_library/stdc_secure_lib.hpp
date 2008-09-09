// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>
// copyright 2005-2006 Leonard Ritter <paniq@paniq.org>

///\file secure version of standard C functions
///\see http://www.opengroup.org/platform/single_unix_specification/uploads/40/6355/n1093.pdf
#pragma once
#include <universalis/detail/project.hpp>
#if defined __STDC_SECURE_LIB__
	#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION__MAJOR == 8
		#include <cstdio>
		#include <cwchar>
		#include <cstdarg>
		#include <cstring>
		namespace std {
			using ::freopen_s;
			using ::vsprintf_s;
			using ::vswprintf_s;
			using ::strcpy_s;
		}
	#endif
#else
	#include <cstdio>
	#include <cwchar>
	#include <cstdarg>
	#include <cstring>
	#include <stdexcept>
	namespace std {
		FILE inline * freopen_s(FILE * restrict * restrict result, char const * restrict path, char const * restrict mode, FILE * restrict stream) throw() {
			return *result = freopen(path, mode, stream);
		}

		int inline vsprintf_s(char * restrict destination, size_t max_length, char const * restrict format, va_list values) throw(runtime_error) {
			int const result(
				#if (\
					defined DIVERSALIS__OPERATING_SYSTEM__LINUX || \
					(defined DIVERSALIS__OPERATING_SYSTEM__BSD && DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR >= 4 && DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR >= 4) || \
					defined DIVERSALIS__OPEPATING_SYSTEM__MICROSOFT \
				)
					vsnprintf(destination, max_length, format, values)
				#else
					vsprintf(destination, format, values)
				#endif
			);
			if(result < 0 || static_cast<size_t>(result) > max_length) throw runtime_error("buffer overflow");
			return result;
		}

		///\todo there might be a bug to submit to the w-api people about the following
		#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && !defined DIVERSALIS__COMPILER__GNU
			int inline vswprintf_s(wchar_t * restrict destination, size_t max_length, wchar_t const * restrict format, va_list values) throw(runtime_error) {
				///\todo there might be a bug to submit to the w-api people about the following
				#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					int const result(vswprintf(destination, format, values));
				#else
					int const result(vswprintf(destination, max_length, format, values));
				#endif
				if(result < 0 || static_cast<size_t>(result) > max_length) throw runtime_error("buffer overflow");
				return result;
			}
		#endif

		char inline * strcpy_s(char * restrict destination, size_t max_length, char const * restrict source) throw(runtime_error) {
			char * const result(strncpy(destination, source, max_length));
			if(result[max_length - 1]) throw runtime_error("buffer overflow");
			return result;
		}
	}
	using std::freopen_s;
	using std::vsprintf_s;
	///\todo there might be a bug to submit to the w-api people about the following
	#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && !defined DIVERSALIS__COMPILER__GNU
		using std::vswprintf_s;
	#endif
	using std::strcpy_s;
#endif
