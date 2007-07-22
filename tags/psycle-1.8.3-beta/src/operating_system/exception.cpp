///\file
///\brief implementation file for operating_system::exception
#include <project.private.hpp>
#define OPERATING_SYSTEM__EXCEPTION
#include "exception.hpp"
#include "logger.hpp"
#include <processor/fpu.hpp>
#include <iostream>
#if defined OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
#endif
#if defined COMPILER__MICROSOFT
	#include <eh.h>
#endif
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
					#if defined $
						#error "macro clash"
					#endif
					#define $(exception, description) case exception: s << #exception ": " description; break;
						// see http://www.iseran.com/Win32/FAQ/except.html
						// warnings
						//$(STATUS_GUARD_PAGE_VIOLATION,"")
						//$(STATUS_DATATYPE_MISALIGNMENT, "The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.")
						//$(STATUS_BREAKPOINT, "A breakpoint was encountered.")
						//$(STATUS_SINGLE_STEP, "A trace trap or other single-instruction mechanism signaled that one instruction has been executed.")
						// errors
						$(STATUS_ACCESS_VIOLATION, "The thread tried to read from or write to a virtual address for which it does not have the appropriate access.")
						//$(STATUS_IN_PAGE_ERROR, "The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.")
						//$(STATUS_INVALID_HANDLE,"An invalid handle was used in an api function call.")
						//$(STATUS_NO_MEMORY,"")
						//$(STATUS_ILLEGAL_INSTRUCTION, "The thread tried to execute an invalid instruction.")
						//$(STATUS_NONCONTINUABLE_EXCEPTION, "The thread tried to continue execution after a noncontinuable exception occurred.")
						//$(STATUS_INVALID_DISPOSITION, "An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.")
						//$(STATUS_ARRAY_BOUNDS_EXCEEDED, "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.")
						//$(STATUS_FLOAT_DENORMAL_OPERAND, "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.")
						//$(STATUS_FLOAT_INEXACT_RESULT, "The result of a floating-point operation cannot be represented exactly as a decimal fraction.")
						//$(STATUS_FLOAT_DIVIDE_BY_ZERO, "The thread tried to divide a floating-point value by a floating-point divisor of zero.")
						//$(STATUS_FLOAT_OVERFLOW, "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.")
						//$(STATUS_FLOAT_UNDERFLOW, "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.")
						//$(STATUS_FLOAT_STACK_CHECK, "The stack overflowed or underflowed as the result of a floating-point operation.")
						//$(STATUS_FLOAT_INVALID_OPERATION, "This exception represents any floating-point exception not included in this list.")
						//$(STATUS_INTEGER_DIVIDE_BY_ZERO, "The thread tried to divide an integer value by an integer divisor of zero.")
						//$(STATUS_INTEGER_OVERFLOW, "The result of an integer operation caused a carry out of the most significant bit of the result.")
						//$(STATUS_PRIVILEGED_INSTRUCTION, "The thread tried to execute an instruction whose operation is not allowed in the current machine mode.")
						//$(STATUS_STACK_OVERFLOW, "The thread used up its stack.")
						//$(STATUS_CONTROL_C_EXIT,"")
						//$(0xE06D7363,"Visual C++ exception: The SEH exception used by the c++ try/catch/throw mechanism in Visual C++")
					#undef C
				default:
					DWORD flags
					(
						FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_FROM_SYSTEM |
						FORMAT_MESSAGE_IGNORE_INSERTS
					);
					char * error_message_pointer(0);
					HMODULE module(::LoadLibraryEx("ntdll", 0, LOAD_LIBRARY_AS_DATAFILE));
					if(module) flags |= FORMAT_MESSAGE_FROM_HMODULE;
					if
					(
						!::FormatMessage // ouch! plain old c api style, this is ugly...
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
						s << "unkown exception code: 0x" << std::hex << code << " !!!";
					}
					else
					{
						s << error_message_pointer;
					}
					if(error_message_pointer) ::LocalFree(error_message_pointer);
					if(module) ::FreeLibrary(module);
				}
				return s.str();
			}

			void structured_exception_translator(unsigned int code, EXCEPTION_POINTERS *) throw(translated)
			{
				///\todo adds more information using EXCEPTION_POINTERS *
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
						//return; // unimportant exception, continue the execution.
					case STATUS_FLOAT_OVERFLOW:
					case STATUS_FLOAT_STACK_CHECK:
					case STATUS_FLOAT_DIVIDE_BY_ZERO:
					case STATUS_FLOAT_INVALID_OPERATION:
						processor::fpu::exception_status::clear();
						throw translated(code);

					///////////
					// default

					default:
						{
							// This type of exception is usually likely followed by a bad crash of the whole process,
							// because it is caused by really bad things like wrong memory access, etc...
							// So, we automatically log them as soon as they are created, that is, even before they are thrown.
							std::ostringstream s;
							s << "crash: ";
							#if defined OPERATING_SYSTEM__MICROSOFT
								s << "thread id: " << ::GetCurrentThreadId() << ", ";
							#endif
							s << typeid(translated).name() << std::endl << code_description(code);
							psycle::host::loggers::crash(s.str());
						}
						throw translated(code);
				}
			}
		}

		char const * translated::what() const
		{
			if(!what_) what_ = new std::string(code_description(code()));
			return what_->c_str();
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
