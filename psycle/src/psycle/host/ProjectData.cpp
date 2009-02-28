#include "ProjectData.hpp"
//#include "Global.hpp"
//#include "Configuration.hpp"

namespace psycle {
	namespace host {

		ProjectData::ProjectData()
			:  active_project_(0)
		{
		}

		ProjectData::~ProjectData()
		{
		}

		void ProjectData::Add(Project* project)
		{
			projects_.push_back(project);
			SetActiveProject(project);
		}
   
		void ProjectData::Remove(Project* project)
		{
			// todo
		}

		void ProjectData::SetActiveProject(Project* project)
		{
			active_project_ = project;
		}

	}
}
