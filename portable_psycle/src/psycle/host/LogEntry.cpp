#include "stdafx.h"
#include "logentry.h"
#include <string>

namespace psycle
{
	namespace host
	{
		LogEntry::LogEntry(int Level, std::string String) : level(Level), string(String)
		{
		}

		LogEntry::~LogEntry(void)
		{
		}
	}
}