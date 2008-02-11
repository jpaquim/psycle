/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief






// http://www.cs.wustl.edu/~schmidt/signal-patterns.html







#pragma once
#include <operating_system/project.hpp>
#include <csignal>
namespace operating_system
{
	namespace signal
	{
		namespace signals
		{
			const int interruption_request = SIGINT;
			const int illegal_instruction_exception = SIGILL;
			const int floating_point_exception = SIGFPE;
			const int segment_violation_exception = SIGSEGV;
			const int termination_request = SIGTERM;
			const int break_request = SIGBREAK;
			const int abort = SIGABRT;
		}
		typedef void (__cdecl * handler)(int);
		void install_handler(const handler &) throw();
		void __cdecl interruption_handler(const int signal);

		/// takes the 386 exception vector and attempts to translate this number into a unix compatible signal value.
		int seh_to_signal(const int exceptionVector)
		{
			int signal;
			switch (exceptionVector)
			{
			case 0: signal = 8; break; // divide by zero
			case 1: signal = 5; break; // debug exception
			case 3: signal = 5; break; // breakpoint
			case 4: signal = 16; break; // into instruction (overflow)
			case 5: signal = 16; break; // bound instruction
			case 6: signal = 4; break; // Invalid opcode
			case 7: signal = 8; break; // coprocessor not available
			case 8: signal = 7; break; // double fault
			case 9: signal = 11; break; // coprocessor segment overrun
			case 10: signal = 11; break; // Invalid TSS
			case 11: signal = 11; break; // Segment not present
			case 12: signal = 11; break; // stack exception
			case 13: signal = 11; break; // general protection
			case 14: signal = 11; break; // page fault
			case 16: signal = 7; break; // coprocessor error
			default: signal = 7; break; // "software generated"
		}
		return signal;
	}
}

// arch-tag: c34ebd61-c307-4e88-a745-920c456e7c6b
