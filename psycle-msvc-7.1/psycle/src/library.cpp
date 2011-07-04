#include <project.h>
#if defined OPERATING_SYSTEM__MICROSOFT
	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/dynamic_link_library_functions.asp
#	include <operating_system/Logger.h>
#	if LOGGER__THRESHOLD_LEVEL <= 1
#		include <windows.h>
		bool stdcall DllMain/*EntryPoint*/(::HINSTANCE instance_handle, unsigned long int reason_for_call, void *)
		{
			if(Logger::default_threshold_level() <= 1)
			{
				char file_name [1 << 10];
				::GetModuleFileName(instance_handle, file_name, sizeof file_name);
				LOGGER(1) << file_name << ": dll entry point: reason for call: ";
			}
			switch (reason_for_call)
			{
			case DLL_PROCESS_ATTACH:
				LOGGER(1) << "new process attached";
				break;
			case DLL_THREAD_ATTACH:
				LOGGER(1) << "new thread attached";
				break;
			case DLL_THREAD_DETACH:
				LOGGER(1) << "thread detached";
				break;
			case DLL_PROCESS_DETACH:
				LOGGER(1) << "process detached";
				break;
			default:
				LOGGER(10) << "unknown reason for call: " << reason_for_call << std::endl;
				return false;
			}
			LOGGER(1) << std::endl;
			return true;
		}
#	endif
#endif
