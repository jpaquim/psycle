#pragma once
#include "Global.hpp"

namespace psy {
	namespace core {
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
			psy::core::Song* song_;

		};

	}
}