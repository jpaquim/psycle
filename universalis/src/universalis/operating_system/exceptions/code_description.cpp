// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation universalis::operating_system::code_description
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "code_description.hpp"
#include <universalis/standard_library/exceptions/code_description.hpp>
#include <sstream>
namespace universalis { namespace operating_system { namespace exceptions { namespace detail {

std::string code_description(
	#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		int const code
	#else
		::DWORD /* or ::HRESULT in some cases */ const code,
		bool from_processor
	#endif
) {
	#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		return standard_library::exceptions::code_description(code);
	#else
		std::ostringstream s; s
		<< (from_processor ? "processor/os exception" : "microsoft api error") << ": "
		<< code << " 0x" << std::hex << code << ": ";
		switch(code) {
			#if !defined DIVERSALIS__COMPILER__ECLIPSE
				#define $universalis(exception, description) case exception: s << #exception ": " description; break
					// see http://msdn.microsoft.com/library/en-us/debug/base/getexceptioncode.asp
					// see http://www.iseran.com/Win32/FAQ/except.html
					// errors
					$universalis(STATUS_ACCESS_VIOLATION, "The thread tried to read from or write to a virtual address for which it does not have the appropriate access.");
					//$universalis(STATUS_IN_PAGE_ERROR, "The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.");
					//$universalis(STATUS_ILLEGAL_INSTRUCTION, "The thread tried to execute an invalid instruction.");
					//$universalis(STATUS_PRIVILEGED_INSTRUCTION, "The thread tried to execute an instruction whose operation is not allowed in the current machine mode.");
					//$universalis(STATUS_STACK_OVERFLOW, "The thread used up its stack.");
					//$universalis(STATUS_NO_MEMORY, "no memory");
					//$universalis(STATUS_INVALID_DISPOSITION, "An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.");
					//$universalis(STATUS_ARRAY_BOUNDS_EXCEEDED, "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.");
					//$universalis(STATUS_FLOAT_DENORMAL_OPERAND, "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.");
					//$universalis(STATUS_FLOAT_DIVIDE_BY_ZERO, "The thread tried to divide a floating-point value by a floating-point divisor of zero.");
					//$universalis(STATUS_FLOAT_INEXACT_RESULT, "The result of a floating-point operation cannot be represented exactly as a decimal fraction.");
					//$universalis(STATUS_FLOAT_INVALID_OPERATION, "This exception represents any floating-point exception not included in this list.");
					//$universalis(STATUS_FLOAT_OVERFLOW, "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.");
					//$universalis(STATUS_FLOAT_STACK_CHECK, "The stack overflowed or underflowed as the result of a floating-point operation.");
					//$universalis(STATUS_FLOAT_UNDERFLOW, "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.");
					//$universalis(STATUS_FLOAT_MULTIPLE_FAULTS, "");
					//$universalis(STATUS_FLOAT_MULTIPLE_TRAPS, "");
					//$universalis(STATUS_INTEGER_DIVIDE_BY_ZERO, "The thread tried to divide an integer value by an integer divisor of zero.");
					//$universalis(STATUS_INTEGER_OVERFLOW, "The result of an integer operation caused a carry out of the most significant bit of the result.");
					//$universalis(STATUS_INVALID_HANDLE, "An invalid handle was used in an api function call.");
					//$universalis(STATUS_NONCONTINUABLE_EXCEPTION, "The thread tried to continue execution after a noncontinuable exception occurred.");
					// signals
					//$universalis(STATUS_CONTROL_C_EXIT, "a Control+C");
					//$universalis(DBG_CONTROL_C, "a Control+C");
					//$universalis(DBG_CONTROL_BREAK, "a Control+Break");
					//$universalis(DBG_TERMINATE_THREAD, "a Terminate Thread");
					//$universalis(DBG_TERMINATE_PROCESS, "a Terminate Process");
					// warnings
					//$universalis(STATUS_GUARD_PAGE_VIOLATION, "");
					//$universalis(STATUS_DATATYPE_MISALIGNMENT, "The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.");
					//$universalis(STATUS_BREAKPOINT, "A breakpoint was encountered.");
					//$universalis(STATUS_SINGLE_STEP, "A trace trap or other single-instruction mechanism signaled that one instruction has been executed.");
					// rpc
					//$universalis(RPC_S_UNKNOWN_IF, "an Unknown Interface");
					//$universalis(RPC_S_SERVER_UNAVAILABLE, "a Server Unavailable");
					// huh?
					//$universalis(0xE06D7363, "Visual C++ exception: The SEH exception used by the c++ try/catch/throw mechanism in Visual C++");
				#undef $universalis
			#endif
			default:
				::DWORD flags(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS
				);
				char * error_message_pointer(0);
				::HMODULE module(0);
				if(from_processor) module = ::LoadLibraryExA("ntdll", 0, LOAD_LIBRARY_AS_DATAFILE);
				try {
					if(module) flags |= FORMAT_MESSAGE_FROM_HMODULE;
					if(::FormatMessageA( // ouch! plain old c api style, this is ugly...
							flags, module, code,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
							reinterpret_cast<char*>(&error_message_pointer), // we *must* hard-cast! this seems to be a hack to extend an originally badly designed api... there is no other way to do it
							0, 0
					)) s << error_message_pointer;
					else s << "unkown exception code: " << code << " 0x" << std::hex << code;
					if(error_message_pointer) ::LocalFree(error_message_pointer);
				} catch(...) {
					if(module) ::FreeLibrary(module);
					throw;
				}
				if(module) ::FreeLibrary(module);
		}
		return s.str();
	#endif
}

}}}}
