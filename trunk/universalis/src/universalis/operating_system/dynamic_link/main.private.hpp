// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\file
#include <universalis/detail/project.private.hpp>
#if defined PACKAGENERIC__CONFIGURATION__OPTION__ENABLE__MONOLITHIC_COMPILATION
	#pragma once
#endif
#include <universalis/operating_system/loggers.hpp>
#if defined DIVERSALIS__COMPILER__GNU
	namespace universalis { namespace operating_system { namespace dynamic_link {
		namespace init {
			void constructor() UNIVERSALIS__COMPILER__ATTRIBUTE(constructor) UNIVERSALIS__COMPILER__HIDDEN;
			void constructor() {
				// if this library owns the logger, then it's not yet initialized and cannot be used yet.
				#if !defined UNIVERSALIS__OPERATING_SYSTEM__LOGGER__OWNED
					if(loggers::information()())
					{
						std::ostringstream s;
						s << "library attached to process: " << UNIVERSALIS__OPERATING_SYSTEM__LIBRARY__NAME;
						loggers::information()(s.str());
					}
				#endif
			}
		
			void destructor() UNIVERSALIS__COMPILER__ATTRIBUTE(destructor) UNIVERSALIS__COMPILER__HIDDEN;
			void destructor() {
				// if this library owns the logger, then it's already destroyed and cannot be used anymore.
				#if !defined UNIVERSALIS__OPERATING_SYSTEM__LOGGER__OWNED
					if(loggers::information()())
					{
						std::ostringstream s;
						s << "library detached from process: " << UNIVERSALIS__OPERATING_SYSTEM__LIBRARY__NAME;
						loggers::information()(s.str());
					}
				#endif
			}
		}
	}}}
#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
	#include <windows.h>
	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/dynamic_link_library_functions.asp
	::BOOL APIENTRY DllMain(::HMODULE module, ::DWORD reason_for_call, ::LPVOID) {
		namespace loggers = universalis::operating_system::loggers;
		std::ostringstream s;
		if(loggers::exception()()) {
			s << "library: ";
			{
				char file_name[DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__MAX_PATH];
				::GetModuleFileName(module, file_name, sizeof file_name);
				s << file_name << ": ";
			}
			s << "dll entry point: reason for call: ";
		}
		bool result(true);
		switch(reason_for_call) {
			case DLL_PROCESS_ATTACH:
				if(loggers::information()()) {
					s << "new process attached";
					loggers::information()(s.str());
				}
				break;
			case DLL_THREAD_ATTACH:
				if(loggers::trace()()) {
					s << "new thread attached";
					loggers::trace()(s.str());
				}
				break;
			case DLL_THREAD_DETACH:
				if(loggers::trace()()) {
					s << "thread detached";
					loggers::trace()(s.str());
				}
				break;
			case DLL_PROCESS_DETACH:
				if(loggers::information()()) {
					s << "process detached";
					loggers::information()(s.str());
				}
				break;
			default:
				if(loggers::exception()()) {
					s << "unknown reason for call: " << reason_for_call;
					loggers::exception()(s.str());
				}
				result = false;
		}
		return result;
	}
#else
	#error todo...
#endif

// arch-tag: 77463b94-64a2-49ba-b55b-fd9237114d8d
