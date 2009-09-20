#pragma once
#include "configuration_options.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include "Global.hpp"


namespace psy {
	namespace core {
		class PatternEvent;
		class RiffFile;		
  }
}

namespace psycle {
	namespace host {

		class Psy3Saver {
		public:
			Psy3Saver(psy::core::Song& song);
			~Psy3Saver();

			bool Save(psy::core::RiffFile* pFile,bool autosave);

		private:

			void ConvertEvent(const psy::core::PatternEvent& ev, unsigned char* data) const;
			unsigned char* CreateNewPattern(int ps);

			psy::core::Song* song_;
			unsigned char * ppPatternData[MAX_PATTERNS];
			

		};

	}
}

#endif