#include <stdafx.h>
//#include <project.h>
#define OPERATING_SYSTEM__EXCEPTION
#include <operating_system/exception.h>
#include <operating_system/logger.h>
#include <iostream>
#if defined OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
	#include <fstream>
	#include <psycle/host/Global.h> // for psycle::host::logger
#endif
///\file
///\brief implementation file for operating_system::exception
namespace operating_system
{

	exception::exception(const std::string & what) throw() : std::runtime_error(what)
	{
		#if defined OPERATING_SYSTEM__MICROSOFT
			{
				std::ostringstream s; s << "exception: " << typeid(*this).name() << ": " << this->what();
				TRACE("%s\n", s.str().c_str());
				psycle::host::logger(psycle::host::logger::trace, s.str());
			}
			/*
			{
				std::ofstream out;
				{
					std::string module_directory;
					{
						char module_file_name[MAX_PATH];
						::GetModuleFileName(0, module_file_name, sizeof module_file_name);
						module_directory = module_file_name;
						module_directory = module_directory.substr(0, module_directory.rfind('\\'));
					}
					// overwrites
					out.open((module_directory + "/output.log.txt").c_str());
				}
				out << "exception: " << typeid(*this).name() << ": " << this->what() << std::endl;
			}
			*/
			/* annoying popup
			{
				std::ostringstream title; title << "exception: " << typeid(*this).name();
				std::ostringstream message; message << typeid(*this).name() << std::endl << this->what();
				::MessageBox(0, message.str().c_str(), title.str().c_str(), MB_OK | MB_ICONWARNING);
			}
			*/
		#else
			std::cerr << "exception: " << typeid(*this).name() << ": " << this->what() << std::endl; 
		#endif
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
				default: s << "unkown exception code: 0x" << std::hex << code << " !!!";
				}
				if(logger::default_threshold_level() <= 0) s << " (ugly microsoft system error code number translated courtesy of bohan)";
				return s.str();
			}
			void structured_exception_translator(unsigned int code, EXCEPTION_POINTERS *) throw(translated)
			{
				//if(code == EXCEPTION_BREAKPOINT) return;
				throw translated(code);
			}
		}

		translated::translated(const unsigned int & code) throw() : exception(code_description(code)) {}

		void translated::new_thread() throw()
		{
			#if defined OPERATING_SYSTEM__MICROSOFT
				// In a multithreaded environment,
				// translator functions are maintained separately for each thread.
				// Each new thread needs to install its own translator function.
				// Thus, each thread is in charge of its own translation handling.
				// To specify a custom translation function,
				// call ::_set_se_translator with the name of your translation function as its argument.
				// The translator function that you write is called once for each function invocation on the stack that has try blocks.
				// There is no default translator function.
				//#if !defined NDEBUG
					// <bohan> shit! warning C4535: calling _set_se_translator() requires /EHa; the command line options /EHc and /GX are insufficient
					::_set_se_translator(structured_exception_translator);
				//#endif
				//::SetErrorMode(SEM_FAILCRITICALERRORS);
			#endif
		}
	}
}
