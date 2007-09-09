#define NOMINMAX
#include <windows.h>
// http://www.microsoft.com/msj/0197/exception/exception.aspx
#include <eh.h> // _set_se_translator, needs /EHa, typedef void (__cdecl *_se_translator_function)(unsigned int, EXCEPTION_POINTERS*);
#include <excpt.h> // EXCEPTION_POINTERS

#include <iostream>
#include <iomanip>
#include <cassert>
#include <limits>
#if 0//__STDC_VERSION__ >= 199901
	#include <cstdint>
#else
	namespace std { typedef unsigned __int64 uint64_t; }
#endif
#include <cfloat> // for _control*

struct counters {
	std::uint64_t denormals, invalids;
};

int filter(unsigned long int code, EXCEPTION_POINTERS & e, counters & c) {
	std::cout << "in filter\n";
	assert(code == e.ExceptionRecord->ExceptionCode);
	switch(code) {
		case EXCEPTION_FLT_INVALID_OPERATION /* STATUS_FLOAT_INVALID_OPERATION */:
			++c.invalids;
			if(false) {
				std::cout << "invalid: continue search" << std::endl;
				return EXCEPTION_CONTINUE_SEARCH;
			} else {
				std::cout << "invalid: continue execution" << std::endl;
				_clear87();
				_control87(~unsigned int(), _MCW_EM);
				return EXCEPTION_CONTINUE_EXECUTION;
			}
		case EXCEPTION_FLT_DENORMAL_OPERAND /* STATUS_FLOAT_DENORMAL_OPERAND */:
			_clear87();
			++c.denormals;
			std::cout << "denormal: continue execution" << std::endl;
			return EXCEPTION_CONTINUE_EXECUTION;
		//case EXCEPTION_FLT_INEXACT_RESULT  /* STATUS_FLOAT_INEXACT_RESULT */:
		//case EXCEPTION_FLT_DIVIDE_BY_ZERO /* STATUS_FLOAT_DIVIDE_BY_ZERO */:
		//case EXCEPTION_INT_DIVIDE_BY_ZERO /* STATUS_INTEGER_DIVIDE_BY_ZERO */:
		//case STATUS_CONTROL_C_EXIT:
			std::cout << "execute handler" << std::endl;
			return EXCEPTION_EXECUTE_HANDLER;
		default:
			std::cout << "continue search" << std::endl;
			return EXCEPTION_CONTINUE_SEARCH;
	}
}

#if 0
	// You cannot turn float_control precise off when except is on. Similarly, precise cannot be turned off when fenv_access is on. To go from strict model to a fast model with the float_control pragma, use the following code:
	#pragma float_control(except, off)
	#pragma fenv_access(off)
	#pragma float_control(precise, off)
	// The following line is needed on Itanium processors
	#pragma fp_contract(on)
#else
	//To go from fast model to a strict model with the float_control pragma, use the following code:
	#pragma float_control(precise, on)
	#pragma float_control(except, on)
	#pragma fenv_access(on)
	// The following line is needed on Itanium processors.
	#pragma fp_contract(off)
#endif

int main() {
	std::cout << "===================================" << std::endl;
	__try {
		__try {
			typedef
				//long
				double
				//float
					real;
			typedef std::numeric_limits<real> l;
			assert(l::has_denorm);
			unsigned int w(30);
			std::cout
				<< std::setw(w) << "max: " << l::max() << "\n"
				<< std::setw(w) << "min: " << l::min() << "\n"
				<< std::setw(w) << "denorm min: " << l::denorm_min() << "\n"
				<< std::setw(w) << "epsilon: " << l::epsilon() << "\n"
				<< std::setw(w) << "min class: " << _fpclass(l::denorm_min()) << "\n"
				<< std::setw(w) << "min class | denorm: " << (_fpclass(l::denorm_min()) & (_FPCLASS_ND | _FPCLASS_PD)) << "\n"
				<< std::setw(w) << "min / 2 class: " << _fpclass(l::denorm_min() / 2) << "\n"
				<< std::setw(w) << "min / 2 class | zero: " << (_fpclass(l::denorm_min() / 2) & (_FPCLASS_PZ | _FPCLASS_NZ)) << "\n"
				<< std::setw(w) << "round style: " << l::round_style << "\n"
				<< std::setw(w) << "round error: " << l::round_error() << "\n"
				<< std::setw(w) << "infinity: " << l::infinity() << "\n"
				<< std::setw(w) << "signaling nan: " << l::signaling_NaN() << "\n"
				<< std::setw(w) << "quiet nan: " << l::quiet_NaN() << "\n";
			{
				(unsigned int) _clear87();
				std::cout << "fp control: " << std::hex << _control87(0, 0) << std::endl;
				//int err = _controlfp_s(&currentControl, ~_EM_OVERFLOW, _MCW_EM);
				unsigned int control(
					//_control87(~unsigned int(_EM_DENORMAL), _EM_DENORMAL)
					_control87(~unsigned int(_EM_DENORMAL), _MCW_EM)
					//_control87(~unsigned int(), _MCW_EM)
				);
				std::cout << "fp control: " << std::hex << control << std::endl;
				std::cout << "fp control: " << std::hex << _control87(0, 0) << std::endl;
				//return 1;
				unsigned int mode(SetErrorMode(0)); SetErrorMode(mode);
				std::cout << "error mode: " << std::hex << mode << std::endl;
				//mode |= SEM_NOGPFAULTERRORBOX;// | SEM_FAILCRITICALERRORS);
				SetErrorMode(mode); mode = SetErrorMode(mode);
				std::cout << "error mode: " << std::hex << mode << std::endl;
			}
			counters c = counters();
			int i(0);
			real r(l::epsilon());
			retry:
			__try {
				std::cout << "in try" << std::endl;
				#if 0
					real r(l::min() * 2);
					r += r;
					r /= 8;
					std::cout << r << "\n";
					return 1;
				#endif
					while(r > l::min() / 2) {
					//std::cout << "r: " << r << std::endl;
					real rr(r * real(1 - 1e-1));
					rr += rr;
					rr *= real(0.5);
					//std::cout << "rr: " << rr << std::endl;
					if(!(rr < r))
					{
						std::cout << "no loop variant\n";
						break;
					}
					r = rr;
					if(++i > 1e2) {
						i = 0;
						std::cout
							<< "r: " << r << "\n"
							<< "invalids: " << c.invalids << "\n"
							<< "denormals: " << c.denormals
							<< std::endl;
					}
				}
				std::cout << "loop finished\n";
			} __except(filter(GetExceptionCode(), *GetExceptionInformation(), c)) {
				std::cout
					<< "in exception handler\n"
					<< GetExceptionCode() << "\n";
				// not available here: std::cout << (GetExceptionInformation())->ExceptionRecord->ExceptionCode << "\n";
				std::cout.flush();
				goto retry;
			}
		} __finally {
			std::cout << "in finally\n";
		}
		std::cout << "after try\n";
	} __except(UnhandledExceptionFilter(GetExceptionInformation())) {
		std::cout << "in unhandled exception handler" << std::endl;
	}
	std::cout << "end of main\n";
	return 0;
}
