#pragma once

#include <vector>
#include "project.hpp"


namespace psycle {
	namespace host {

		class ProjectData
		{
		public:
			ProjectData();
			~ProjectData();

			void Add(Project* project);
			void Remove(Project* project);
			void SetActiveProject(Project* project);
			Project* active_project() { return active_project_; }

		private:
			Project* active_project_;
			// the projects
			std::vector< Project* > projects_;

		};
	}
}

