// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule, Magnus Johnson

///\file
///\implementation universalis::processor::exception
#include <universalis/detail/project.private.hpp>
#include "exception.hpp"
#include <universalis/operating_system/loggers.hpp>
#include <universalis/operating_system/threads/id.hpp>
#include <universalis/compiler/typenameof.hpp>
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
#endif
#if defined DIVERSALIS__COMPILER__MICROSOFT
	#include <eh.h>
#endif
#include <universalis/operating_system/detail/check_version.hpp>
#include <universalis/operating_system/paths.hpp>
namespace universalis
{
	namespace processor
	{
		namespace
		{
			std::string const code_description(unsigned int const & code) throw()
			{
				#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					std::ostringstream s;
					s << "microsoft nt structured exception (external cpu/os exception): 0x" << std::hex << code << ": ";
					switch(code)
					{
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
						::DWORD flags
						(
							FORMAT_MESSAGE_ALLOCATE_BUFFER |
							FORMAT_MESSAGE_FROM_SYSTEM |
							FORMAT_MESSAGE_IGNORE_INSERTS
						);
						char * error_message_pointer(0);
						::HMODULE module(0);
						module = ::LoadLibraryEx("ntdll", 0, LOAD_LIBRARY_AS_DATAFILE);
						if(module) flags |= FORMAT_MESSAGE_FROM_HMODULE;
						if
						(
							::FormatMessage // ouch! plain old c api style, this is ugly...
							(
								flags,
								module,
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
							s << "unkown exception code: 0x" << std::hex << code << " !!!";
						}
						if(error_message_pointer) ::LocalFree(error_message_pointer);
						if(module) ::FreeLibrary(module);
					}
					return s.str();
				#else
					std::ostringstream s;
					s << "code: " << code << " 0x" << std::hex << code;
					return s.str();
				#endif
			}

			#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
				void structured_exception_translator(unsigned int code, ::EXCEPTION_POINTERS * exception_pointers) throw(exception)
				{
					#if !defined NDEBUG
						::EXCEPTION_RECORD const & exception_record(*exception_pointers->ExceptionRecord);
					#endif
					assert(code == exception_record.ExceptionCode);
					if(code == STATUS_BREAKPOINT) return; // [bohan] not sure what to do with break points ...
					throw exception(code, UNIVERSALIS__COMPILER__LOCATION);

					///\todo add more information using ::EXCEPTION_POINTERS * exception_pointers
					#if 0
						// The GetModuleBase function retrieves the base address of the module that contains the specified address. 
						::DWORD static GetModuleBase(::DWORD dwAddress)
						{
							::MEMORY_BASIC_INFORMATION Buffer;
							return ::VirtualQuery((::LPCVOID) dwAddress, &Buffer, sizeof Buffer) ? (::DWORD) Buffer.AllocationBase : 0;
						}

						// Now print information about where the fault occured
						rprintf(_T(" at location %08x"), (::DWORD) pExceptionRecord->ExceptionAddress);
						if((hModule = (::HMODULE) ::GetModuleBase((::DWORD) pExceptionRecord->ExceptionAddress)) && ::GetModuleFileName(hModule, szModule, sizeof szModule))
							rprintf(_T(" in module %s"), szModule);
						
						// If the exception was an access violation, print out some additional information, to the error log and the debugger.
						if(pExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && pExceptionRecord->NumberParameters >= 2)
							rprintf(" %s location %08x", pExceptionRecord->ExceptionInformation[0] ? "Writing to" : "Reading from", pExceptionRecord->ExceptionInformation[1]);
							
						/*drmingw::*/StackBackTrace(GetCurrentProcess(), GetCurrentThread(), pContext);		
					#endif
				}
				
				///\todo thread local storage?
				::LPTOP_LEVEL_EXCEPTION_FILTER static unhandled_exception_previous_filter(0);

				::LONG WINAPI unhandled_exception_filter(EXCEPTION_POINTERS * exception_pointers) throw(exception)
				{
					// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/unhandledexceptionfilter.asp
					::EXCEPTION_RECORD const & exception_record(*exception_pointers->ExceptionRecord);
					unsigned int const code(exception_record.ExceptionCode);
					structured_exception_translator(code, exception_pointers);
					// note: The following code is never reached because structured_exception_translator throws a c++ exception.
					//       It is just kept here for documentation and because the function signature requires a return instruction.
					return unhandled_exception_previous_filter ? unhandled_exception_previous_filter(exception_pointers) : EXCEPTION_CONTINUE_SEARCH;
				}
			#endif
		}

		///\todo thread name
		//std::string thread_local_storage thread_name;
		
		void exception::new_thread(std::string const & name)
		{
			if(operating_system::loggers::information()())
			{
				std::ostringstream s;
				s << "new thread: ";
				if(name.size()) s << "name: " << name << ", ";
				s << "id: " << operating_system::threads::id::current();
				operating_system::loggers::information()(s.str());
			}

			//thread_name = name;

			// displays the package version
			{
				static bool once(false);
				if(!once)
				{
					once = true;
					operating_system::detail::check_version();
					if(operating_system::loggers::information()())
					{
						std::ostringstream s;
						s << operating_system::paths::package::name() << " " << operating_system::paths::package::version::string();
						#if !defined NDEBUG
							s << " debug";
						#endif
						operating_system::loggers::information()(s.str());
					}
				}
			}

			// sets the hardware exception handler for the thread
			{
				#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/seterrormode.asp
					::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
							
					#if defined DIVERSALIS__COMPILER__MICROSOFT
						// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt__set_se_translator.asp
						// In a multithreaded environment,
						// translator functions are maintained separately for each thread.
						// Each new thread needs to install its own translator function.
						// Thus, each thread is in charge of its own translation handling.
						// There is no default translator function.
						// [bohan] This requires compilation with the asynchronous exception handling model (/EHa)
						// [bohan] warning C4535: calling ::_set_se_translator() requires /EHa; the command line options /EHc and /GX are insufficient
						::_set_se_translator(structured_exception_translator);
					#else
						static bool once(false);
						if(!once)
						{
							///\todo why only once?
							//once = true;
							// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/setunhandledexceptionfilter.asp
							unhandled_exception_previous_filter = ::SetUnhandledExceptionFilter(unhandled_exception_filter);

							#if defined DIVERSALIS__COMPILER__GNU
								// http://jrfonseca.dyndns.org/projects/gnu-win32/software/drmingw/index.html#exchndl
								// loads dr mingw's unhandled exception handler.
								// it does not matter if the library fails to load, we just won't have this intermediate handler.
								if(::LoadLibrary("exchndl"))
								{
									operating_system::loggers::information()("unhandled exception filter loaded");
								}
								else
								{
									operating_system::loggers::information()("unhandled exception filter was not loaded");
								}
							#endif
						}
					#endif
				#endif
			}
		}

		exception::exception(unsigned int const & code, compiler::location const & location) throw() : universalis::exception(code_description(code), location)
		{
			// This type of exception is usually likely followed by an abrupt termination of the whole process.
			// So, we automatically log them as soon as they are created, that is, even before they are thrown.
			if(operating_system::loggers::crash()())
			{
				std::ostringstream s;
				///\todo thread name
				s << "crash: thread id: " << operating_system::threads::id::current() << ", " << compiler::typenameof(*this) << ": " << what();
				operating_system::loggers::crash()(s.str());
			}
		}
	}
}
