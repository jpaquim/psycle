#include <stdafx.h>
#define OPERATING_SYSTEM__EXCEPTION
#include <operating_system/exception.h>
#include <operating_system/logger.h>
#include <iostream>
#if defined OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
#endif
///\file
///\brief implementation file for operating_system::exception
namespace operating_system
{
	exception::exception(const std::string & what) throw() : std::runtime_error(what)
	{
		std::cerr << "exception: " << this->what() << std::endl; 
	}

	exception::operator const std::string() const throw()
	{
		return std::exception::what(); 
	}

	namespace exceptions
	{
		namespace
		{
			const std::string code_description(const unsigned int & code) throw()
			{
				std::ostringstream s;
				s << "microsoft nt structured exception (external cpu/os exception): 0x" << std::hex << code << ": ";
				switch(code)
				{
					#if defined C
						#error macro clash
					#endif
					#define C(exception, description) case exception: s << #exception ": " description; break;

					// see http://www.iseran.com/Win32/FAQ/except.html
					/*
					// warnings
					C(EXCEPTION_GUARD_PAGE,"")
					C(EXCEPTION_DATATYPE_MISALIGNMENT, "The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.")
					C(EXCEPTION_BREAKPOINT, "A breakpoint was encountered.")
					C(EXCEPTION_SINGLE_STEP, "A trace trap or other single-instruction mechanism signaled that one instruction has been executed.")
					*/
					// errors
					C(EXCEPTION_ACCESS_VIOLATION, "The thread tried to read from or write to a virtual address for which it does not have the appropriate access.")
					/*
					C(EXCEPTION_IN_PAGE_ERROR, "The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.")
					C(STATUS_INVALID_HANDLE,"An invalid handle was used in an api function call.")
					C(STATUS_NO_MEMORY,"")
					C(EXCEPTION_ILLEGAL_INSTRUCTION, "The thread tried to execute an invalid instruction.")
					C(EXCEPTION_NONCONTINUABLE_EXCEPTION, "The thread tried to continue execution after a noncontinuable exception occurred.")
					C(EXCEPTION_INVALID_DISPOSITION, "An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.")
					C(EXCEPTION_ARRAY_BOUNDS_EXCEEDED, "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.")
					C(EXCEPTION_FLT_DENORMAL_OPERAND, "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.")
					C(EXCEPTION_FLT_DIVIDE_BY_ZERO, "The thread tried to divide a floating-point value by a floating-point divisor of zero.")
					C(EXCEPTION_FLT_INEXACT_RESULT, "The result of a floating-point operation cannot be represented exactly as a decimal fraction.")
					C(EXCEPTION_FLT_INVALID_OPERATION, "This exception represents any floating-point exception not included in this list.")
					C(EXCEPTION_FLT_OVERFLOW, "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.")
					C(EXCEPTION_FLT_STACK_CHECK, "The stack overflowed or underflowed as the result of a floating-point operation.")
					C(EXCEPTION_FLT_UNDERFLOW, "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.")
					C(EXCEPTION_INT_DIVIDE_BY_ZERO, "The thread tried to divide an integer value by an integer divisor of zero.")
					C(EXCEPTION_INT_OVERFLOW, "The result of an integer operation caused a carry out of the most significant bit of the result.")
					C(EXCEPTION_PRIV_INSTRUCTION, "The thread tried to execute an instruction whose operation is not allowed in the current machine mode.")
					C(EXCEPTION_STACK_OVERFLOW, "The thread used up its stack.")
					C(STATUS_CONTROL_C_EXIT,"")
					C(0xE06D7363,"Visual C++ exception: The SEH exception used by the c++ try/catch/throw mechanism in Visual C++")
					*/
					#undef C
				default:
					DWORD flags
					(
						FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_FROM_SYSTEM |
						FORMAT_MESSAGE_IGNORE_INSERTS
					);
					char * error_message_pointer(0);
					HMODULE hModule(::LoadLibraryEx("ntdll", 0, LOAD_LIBRARY_AS_DATAFILE));
					if(hModule != 0) flags |= FORMAT_MESSAGE_FROM_HMODULE;
					if
					(
						!::FormatMessage // ouch! plain old c api style, this is ugly...
						(
							flags,
							hModule,
							code,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
							reinterpret_cast<char*>(&error_message_pointer), // we *must* hard-cast! this seems to be a hack to extend an originally badly designed api... there is no other way to do it
							0,
							0
						)
					)
					{
						s << "unkown exception code: 0x" << std::hex << code << " !!!";
					}
					else
					{
						s << error_message_pointer;
					}
					if(error_message_pointer != 0) ::LocalFree(error_message_pointer);
					if(hModule != 0) ::FreeLibrary(hModule);
				}
				return s.str();
			}

			void structured_exception_translator(unsigned int code, EXCEPTION_POINTERS *) throw(translated)
			{
				///\todo adds more information using EXCEPTION_POINTERS *
				if(code == EXCEPTION_BREAKPOINT) return; // <bohan> not sure what to do with break points...
				throw translated(code);
			}
		}

		translated::translated(const unsigned int & code) throw() : exception(code_description(code))
		{
			// This type of exception is usually likely followed by a bad crash of the whole program,
			// because it is caused by really bad things like wrong memory access, etc...
			// So, we automatically log them as soon as they are created, that is, even before they are thrown.
			std::ostringstream s;
			s << "exception: ";
			#if defined OPERATING_SYSTEM__MICROSOFT
				s << "thread id: " << ::GetCurrentThreadId() << ", ";
			#endif
			s << typeid(*this).name() << ": " << what();
			psycle::host::loggers::crash(s.str());
		}

		void translated::new_thread(const std::string & name)
		{
			{
				std::ostringstream s;
				s << "new thread: ";
				if(name.size()) s << "name: " << name.c_str() << ", ";
				#if defined OPERATING_SYSTEM__MICROSOFT
					s << "id: " << ::GetCurrentThreadId();
				#endif
				psycle::host::loggers::trace(s.str());
			}
			#if defined OPERATING_SYSTEM__MICROSOFT
				// In a multithreaded environment,
				// translator functions are maintained separately for each thread.
				// Each new thread needs to install its own translator function.
				// Thus, each thread is in charge of its own translation handling.
				// To specify a custom translation function,
				// call ::_set_se_translator with the name of your translation function as its argument.
				// The translator function that you write is called once for each function invocation on the stack that has try blocks.
				// There is no default translator function.
				// <bohan> This requires compilation with the asynchronous exception handling model (/EHa)
				// <bohan> warning C4535: calling _set_se_translator() requires /EHa; the command line options /EHc and /GX are insufficient
				::_set_se_translator(structured_exception_translator);
				//::SetErrorMode(SEM_FAILCRITICALERRORS);
			#endif
		}
	}
}
