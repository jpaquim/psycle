#pragma once

#include "Psycle.hpp"
#include "CommandManager.hpp"
#include <cassert>

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/song.h>
using namespace psycle::core;
#else 
#include <psycle/host/Song.hpp>
#endif
#include "ProgressDialog.hpp"

namespace psycle {
	namespace host {

		class ProjectData;
		class PatternView;
		class MachineView;

		class Project : private boost::noncopyable
		{
		public:
			Project(ProjectData* parent);
			~Project();

			Song& song() { return *song_; }

			// modules
			PatternView* pat_view() { return pat_view_; }
			MachineView* mac_view() { return mac_view_; }
			ProjectData* parent() {return parent_; }

			void SetActive();

			void FileImportModulefile();
			bool Export(UINT id);
			void OnFileLoadsongNamed(const std::string& fName, int fType);
			void FileLoadsongNamed(const std::string& fName);
			bool CheckUnsavedSong(const std::string& title);
			bool OnFileSave(UINT id);
			bool OnFileSaveAs(UINT id);

			void Clear();

			int beat_zoom() const { return beat_zoom_; }
			void set_beat_zoom(int zoom) { beat_zoom_ = zoom; }

			CommandManager* cmd_manager() { return &cmd_manager_; }

		private:
			void AppendToRecent(const std::string& fName);
			void OnProgress(int a, int b, std::string c);
			void OnReport(std::string a, std::string b);
			
			PatternView* pat_view_;
			MachineView* mac_view_;
			ProjectData* parent_;
			int beat_zoom_;
			Song* song_;
			CommandManager cmd_manager_;
			CProgressDialog progress_;
		};
	}
}
