#pragma once

#include "Global.hpp"
#include <cassert>

namespace psycle {
	namespace host {

		class ProjectData;

		class Project
		{
		public:
			Project(ProjectData* parent,
					class PatternView* pat_view,
					class MachineView* mac_view);
			~Project();

			Song& song() { assert(Global::_pSong); return *Global::_pSong; }

			// modules
			PatternView* pat_view();
			MachineView* mac_view();

			ProjectData* parent();

			void FileImportModulefile();
			bool Export(UINT id);
			void OnFileLoadsongNamed(const std::string& fName, int fType);
			void FileLoadsongNamed(const std::string& fName);
			void AppendToRecent(const std::string& fName);
			bool CheckUnsavedSong(const std::string& title);

			bool OnFileSave(UINT id);
			bool OnFileSaveAs(UINT id);

			void Clear();

		private:
			//todo make this both owner of this project
			PatternView* pat_view_;
			MachineView* mac_view_;
			ProjectData* parent_;

		};
	}
}

