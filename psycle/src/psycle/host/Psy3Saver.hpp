#pragma once
#include "Global.hpp"

namespace psycle {
	namespace core {
		class PatternEvent;
		class RiffFile;
		class MachineKey;
	}
	namespace host {

		class Psy3Saver {
		public:
			Psy3Saver(psycle::core::Song& song);
			~Psy3Saver() {}

			bool Save(psycle::core::RiffFile* pFile,bool autosave);

		private:
			enum OldMachineType
			{
				MACH_UNDEFINED = -1,
				MACH_MASTER = 0,
				MACH_SINE = 1, ///< now a plugin
				MACH_DIST = 2, ///< now a plugin
				MACH_SAMPLER = 3,
				MACH_DELAY = 4, ///< now a plugin
				MACH_2PFILTER = 5, ///< now a plugin
				MACH_GAIN = 6, ///< now a plugin
				MACH_FLANGER = 7, ///< now a plugin
				MACH_PLUGIN = 8,
				MACH_VST = 9,
				MACH_VSTFX = 10,
				MACH_SCOPE = 11,
				MACH_XMSAMPLER = 12,
				MACH_DUPLICATOR = 13,
				MACH_MIXER = 14,
				MACH_RECORDER = 15,
				MACH_DUMMY = 255
			};

			std::string  ModifyDllNameWithIndex(const std::string& name, int index);
			void InitTranslationList();
			std::string replaceString(const std::string& text,
                                      const std::string& old_substr,
                                      const std::string& new_substr) const;
			std::string ConvertName(const std::string& name) const;
			std::uint32_t ConvertType(const psycle::core::MachineKey& key, bool isGenerator) const;
			void ConvertEvent(const psycle::core::PatternEvent& ev, unsigned char* data) const;
			unsigned char* CreateNewPattern(int ps, int tracks, int lines);
			int ComputeLinesPerBeat();

			psycle::core::Song* song_;
			unsigned char * ppPatternData[MAX_PATTERNS];
			std::vector<std::string> underscore_plugins_;			
		};

	}	// namespace host
}	// namespace psycle
