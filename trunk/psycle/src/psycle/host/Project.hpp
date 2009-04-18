#pragma once

#include "Psycle.hpp"
#include <cassert>

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/song.h>
using namespace psy::core;
#else 
#include <psycle/host/Song.hpp>
#endif

namespace psycle {
	namespace host {

		class ProjectData;
		class PatternView;
		class MachineView;

		class Project
		{
		public:
			Project(ProjectData* parent,
					PatternView* pat_view,
					MachineView* mac_view);
			~Project();

			Song& song() { return song_; }

			// modules
			PatternView* pat_view();
			MachineView* mac_view();

			ProjectData* parent();
			void SetActive();

			void FileImportModulefile();
			bool Export(UINT id);
			void OnFileLoadsongNamed(const std::string& fName, int fType);
			void FileLoadsongNamed(const std::string& fName);
			bool CheckUnsavedSong(const std::string& title);

			bool OnFileSave(UINT id);
			bool OnFileSaveAs(UINT id);

			void Clear();

			void set_lines_per_beat(int lines_per_beat) {
				lines_per_beat_ = lines_per_beat;
			}
			int lines_per_beat() const { return lines_per_beat_; }

		private:

			void AppendToRecent(const std::string& fName);

			//todo make this both owner of this project
			PatternView* pat_view_;
			MachineView* mac_view_;
			ProjectData* parent_;

			int lines_per_beat_;

			Song song_;

		};
	}
}
