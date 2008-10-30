// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\interface universalis::processor::exception
#pragma once
#include <universalis/operating_system/exception.hpp>
#include <universalis/compiler/compiler.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__PROCESSOR__EXCEPTION
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis { namespace processor {

/// external cpu/os exception translated into a c++ one, with deferred querying of the human-readable message.
class UNIVERSALIS__COMPILER__DYNAMIC_LINK exception : public universalis::operating_system::exception {
	public:
		/// This should be called for and from any new thread created to enable cpu/os to c++ exception translation for that thread.
		void static install_handler_in_thread();

		/// This should be called for and from any new thread created to enable cpu/os to c++ exception translation for that thread.
		UNIVERSALIS__COMPILER__DEPRECATED("set the thread name using universalis::operating_system::thread_name");
		void static inline install_handler_in_thread(std::string const &) { install_handler_in_thread(); }

		#if defined NBEBUG
			inline
		#endif
			exception(unsigned int const & code, compiler::location const & location) throw()
				#if defined NDEBUG
					: universalis::operating_system::exception(code, location) {}
				#else
					;
				#endif
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>
