// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation universalis::processor::exception
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "exception.hpp"

// weird, must be included last or mingw 3.4.1 segfaults
//#include "exceptions/code_description.hpp"

#include "exceptions/fpu.hpp"
#include <universalis/operating_system/loggers.hpp>
#include <universalis/operating_system/threads/id.hpp>
#include <universalis/compiler/typenameof.hpp>
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
#endif
#if defined DIVERSALIS__COMPILER__MICROSOFT
	#include <eh.h>
#endif

#include <universalis/operating_system/detail/check_version.hpp> //
#include <universalis/operating_system/paths.hpp> //

// weird, must be included last or mingw 3.4.1 segfaults
#include "exceptions/code_description.hpp"

namespace universalis
{
	namespace processor
	{
		namespace
		{
			#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
				void structured_exception_translator(unsigned int code, ::EXCEPTION_POINTERS * exception_pointers) throw(exception)
				{
					#if !defined NDEBUG
						::EXCEPTION_RECORD const & exception_record(*exception_pointers->ExceptionRecord);
					#endif
					assert(code == exception_record.ExceptionCode);
					switch(code)
					{
						////////////////////
						// things to ignore

						case STATUS_BREAKPOINT: // [bohan] not sure what to do with break points...
						case STATUS_SINGLE_STEP:
							return;

						/////////////////////////////
						// floating point exceptions

						case STATUS_FLOAT_INEXACT_RESULT:
						case STATUS_FLOAT_DENORMAL_OPERAND:
						case STATUS_FLOAT_UNDERFLOW:
							return; // unimportant exception, continue the execution.
						case STATUS_FLOAT_OVERFLOW:
						case STATUS_FLOAT_STACK_CHECK:
						case STATUS_FLOAT_DIVIDE_BY_ZERO:
						case STATUS_FLOAT_INVALID_OPERATION:
							exceptions::fpu::status::clear();
							// and throw ...

						///////////
						// default

						default:
							throw exception(code, UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
					}


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
				
				::LPTOP_LEVEL_EXCEPTION_FILTER static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE thread_unhandled_exception_previous_filter(0);

				::LONG WINAPI unhandled_exception_filter(EXCEPTION_POINTERS * exception_pointers) throw(exception)
				{
					// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/unhandledexceptionfilter.asp
					::EXCEPTION_RECORD const & exception_record(*exception_pointers->ExceptionRecord);
					unsigned int const code(exception_record.ExceptionCode);
					structured_exception_translator(code, exception_pointers);
					// note: The following code is never reached because structured_exception_translator throws a c++ exception.
					//       It is just kept here for documentation and because the function signature requires a return instruction.
					return thread_unhandled_exception_previous_filter ? thread_unhandled_exception_previous_filter(exception_pointers) : EXCEPTION_CONTINUE_SEARCH;
				}
			#endif

			/// thread name
			std::string const static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE * thread_name(0);
		}

		void exception::install_handler_in_thread(std::string const & name)
		{
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
						operating_system::loggers::information()(s.str());
					}
				}
			}

			if(operating_system::loggers::information()())
			{
				std::ostringstream s;
				s << "installing cpu/os exception handler in thread: ";
				if(name.size()) s << "name: " << name << ", ";
				s << "id: " << operating_system::threads::id::current();
				operating_system::loggers::information()(s.str());
			}

			thread_name = &name;

			// sets the hardware exception handler for the thread
			{
				#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/seterrormode.asp
					//::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
							
					#if defined DIVERSALIS__COMPILER__MICROSOFT
						// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt__set_se_translator.asp
						// In a multithreaded environment,
						// translator functions are maintained separately for each thread.
						// Each new thread needs to install its own translator function.
						// Thus, each thread is in charge of its own translation handling.
						// There is no default translator function.
						// [bohan] This requires compilation with the asynchronous exception handling model (/EHa)
						// [bohan] warning C4535: calling ::_set_se_translator() requires /EHa; the command line options /EHc and /GX are insufficient
						#if 1 // DIVERSALIS__COMPILER__VERSION__MAJOR >= 8 || defined NDEBUG // causes problems with the debugger in msvc7.1
							::_set_se_translator(structured_exception_translator);
						#endif
					#else
						static bool once(false);
						if(!once)
						{
							///\todo review and test this

							///\todo why only once?
							//once = true;
							// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/setunhandledexceptionfilter.asp
							thread_unhandled_exception_previous_filter = ::SetUnhandledExceptionFilter(unhandled_exception_filter);

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
									operating_system::loggers::information()("unhandled exception filter has not been loaded");
								}
							#endif
						}
					#endif
				#endif
			}
		}

		char const * exception::what() const throw()
		{
			if(!what_) what_ = new std::string(exceptions::code_description(code()));
			return what_->c_str();
		}

		#if !defined NDEBUG
			exception::exception(unsigned int const & code, compiler::location const & location) throw()
			:
				operating_system::exception(code, location)
			{
				// This type of exception is usually likely followed by an abrupt termination of the whole process.
				// So, we automatically log them as soon as they are created, that is, even before they are thrown.
				if(operating_system::loggers::crash()())
				{
					std::ostringstream s;
					s
						<< "cpu/os exception: "
						<< "thread: ";
					if(thread_name && thread_name->size()) s << "name: " << *thread_name << ", ";
					s
						<< "id: " << operating_system::threads::id::current() << "\n"
						<< compiler::typenameof(*this) << ": " << what();
					operating_system::loggers::crash()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			}
		#endif
	}
}
