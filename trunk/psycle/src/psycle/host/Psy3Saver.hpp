#pragma once
#include "configuration_options.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include "Global.hpp"


namespace psycle {
	namespace core {
		class PatternEvent;
		class RiffFile;		
  }
}

namespace psycle {
	namespace host {

		class Psy3Saver {
		public:
			Psy3Saver(psycle::core::Song& song);
			~Psy3Saver();

			bool Save(psycle::core::RiffFile* pFile,bool autosave);

		private:

			void ConvertEvent(const psycle::core::PatternEvent& ev, unsigned char* data) const;
			unsigned char* CreateNewPattern(int ps);

			psycle::core::Song* song_;
			unsigned char * ppPatternData[MAX_PATTERNS];
			

		};

	}
}

#endif