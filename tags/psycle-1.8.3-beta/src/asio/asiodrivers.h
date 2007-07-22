#pragma once
#if defined ASIO
	#include LIBRARY__EXPORT
#else
	#include LIBRARY__IMPORT
#endif
#include "ginclude.h"
#if MAC
	#include "CodeFragments.hpp"
#elif WINDOWS
	#include <windows.h>
	#include "asiolist.h"
#elif SGI || BEOS
	#include "asiolist.h"
#else
	#error implement me
#endif
namespace asio
{
	class LIBRARY AsioDrivers : public 
	#if MAC
		CodeFragments
	#elif WINDOWS
		AsioDriverList
	#elif SGI || BEOS
		AsioDriverList
	#else
		#error implement me
	#endif
	{
	public:
		AsioDrivers();
		~AsioDrivers();
		bool getCurrentDriverName(char *name);
		long getDriverNames(char **names, long maxDrivers);
		bool loadDriver(char *name);
		void removeCurrentDriver();
		long getCurrentDriverIndex() {return curIndex;}
	protected:
		unsigned long connID;
		long curIndex;
	};
}
