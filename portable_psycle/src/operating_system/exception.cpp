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
				case EXCEPTION_ACCESS_VIOLATION: s << "access violation: The thread tried to read from or write to a virtual address for which it does not have the appropriate access."; break;
				case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: s << "array bounds exceeded: The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking."; break;
				case EXCEPTION_BREAKPOINT: s << "breakpoint: A breakpoint was encountered."; break;
				case EXCEPTION_DATATYPE_MISALIGNMENT: s << "datatype misalignment: The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on."; break;
				case EXCEPTION_FLT_DENORMAL_OPERAND: s << "denormal operand: One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value."; break;
				case EXCEPTION_FLT_DIVIDE_BY_ZERO: s << "divive by zero: The thread tried to divide a floating-point value by a floating-point divisor of zero."; break;
				case EXCEPTION_FLT_INEXACT_RESULT: s << "inexact result: The result of a floating-point operation cannot be represented exactly as a decimal fraction."; break;
				case EXCEPTION_FLT_INVALID_OPERATION: s << "invalid operation: This exception represents any floating-point exception not included in this list."; break;
				case EXCEPTION_FLT_OVERFLOW: s << "overflow: The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type."; break;
				case EXCEPTION_FLT_STACK_CHECK: s << "stack check: The stack overflowed or underflowed as the result of a floating-point operation."; break;
				case EXCEPTION_FLT_UNDERFLOW: s << "underflow: The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type."; break;
				case EXCEPTION_ILLEGAL_INSTRUCTION: s << "illegal instruction: The thread tried to execute an invalid instruction."; break;
				case EXCEPTION_IN_PAGE_ERROR: s << "page error: The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network."; break;
				case EXCEPTION_INT_DIVIDE_BY_ZERO: s << "int divide by zero: The thread tried to divide an integer value by an integer divisor of zero."; break;
				case EXCEPTION_INT_OVERFLOW: s << "int overflow: The result of an integer operation caused a carry out of the most significant bit of the result."; break;
				case EXCEPTION_INVALID_DISPOSITION: s << "invalid disposition: An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception."; break;
				case EXCEPTION_NONCONTINUABLE_EXCEPTION: s << "noncontinuable exception: The thread tried to continue execution after a noncontinuable exception occurred."; break;
				case EXCEPTION_PRIV_INSTRUCTION: s << "priv instruction: The thread tried to execute an instruction whose operation is not allowed in the current machine mode."; break;
				case EXCEPTION_SINGLE_STEP: s << "single step: A trace trap or other single-instruction mechanism signaled that one instruction has been executed."; break;
				case EXCEPTION_STACK_OVERFLOW: s << "stack overflow: The thread used up its stack."; break;
				default:
					// <magnus> FormatMessage doesn't really seem to succeed but it couldn't hurt, could it?
					// is there any other system routine that can format a nice message?
					LPVOID lpMsgBuf;
					if (!FormatMessage( 
						FORMAT_MESSAGE_ALLOCATE_BUFFER | 
						FORMAT_MESSAGE_FROM_SYSTEM | 
						FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						code,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPTSTR) &lpMsgBuf,
						0,
						NULL ))
					{
						s << "unkown exception code: 0x" << std::hex << code << " !!!";
					}
					else {
						s << lpMsgBuf;
						// Free the buffer.
						LocalFree( lpMsgBuf );
					}


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
				::_set_se_translator(structured_exception_translator);
				//::SetErrorMode(SEM_FAILCRITICALERRORS);
			#endif
		}
	}
}
