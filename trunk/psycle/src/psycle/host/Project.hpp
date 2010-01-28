// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#pragma once

#include <psycle/core/song.h>

#include "CommandManager.hpp"
#include "ProgressDialog.hpp"
#include "Psycle.hpp"

namespace psycle {
	namespace host {

		class ProjectData;
		class PatternView;
		class MachineView;

		class Project : private boost::noncopyable {
		public:
			Project(ProjectData* parent);
			~Project();

			ProjectData* parent() { return parent_; }
			Song& song() { return *song_; }
			// modules
			PatternView* pat_view() { return pat_view_; }
			MachineView* mac_view() { return mac_view_; }

			CommandManager* cmd_manager() { return &cmd_manager_; }

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
			
		private:
			void AppendToRecent(const std::string& fName);
			void OnProgress(int a, int b, std::string c);
			void OnReport(const std::string& a, const std::string& b);
			
			PatternView* pat_view_; // owned by project
			MachineView* mac_view_; // owned by project
			ProjectData* parent_;
			int beat_zoom_;
			Song* song_;  // owned by project
			CommandManager cmd_manager_;
			CProgressDialog progress_;
		};

	}  // namespace host
}  // namespace psycle
