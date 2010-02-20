#pragma once

#include "Psycle.hpp"
#include "project.hpp"

#include <vector>

namespace psycle {
	namespace host {

		class ProjectData
		{
		public:
			ProjectData();
			~ProjectData();

			// for future multi song
			void Add(Project* project);
			void Remove(Project* project);
			void SetActiveProject(Project* project);
			Project* active_project() { return active_project_; }

			// but for now simple one song new, load save code here
			void FileNew();			

		private:
			Project* active_project_;
			// the projects
			std::vector<Project*> projects_;

		};
	}
}

