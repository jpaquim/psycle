#pragma once

namespace psycle
{
	namespace host
	{
		class LogEntry
		{
		public:
			int level; 
			std::string string; 

			LogEntry(int Level, std::string String);

			~LogEntry(void);
		};
	}

}
