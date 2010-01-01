#include "ProjectData.hpp"
//#include "Global.hpp"
//#include "Configuration.hpp"

#include "PatternView.hpp"
#include "MachineView.hpp"
#include "MainFrm.hpp"

namespace psycle {
	namespace host {

		ProjectData::ProjectData()
			:  active_project_(0)
		{
		}

		ProjectData::~ProjectData()
		{
			SetActiveProject(0);
			std::vector<Project*>::iterator it = projects_.begin();
			for (; it < projects_.end(); ++it )
				delete *it;
		}

		void ProjectData::Add(Project* project)
		{
			projects_.push_back(project);
			SetActiveProject(project);
		}
   
		void ProjectData::Remove(Project* project)
		{
			for (std::vector<Project*>::iterator ite = projects_.begin(); ite != projects_.end(); ite++) {
				if (*ite == project) {
					projects_.erase(ite);
				}
			}
		}

		void ProjectData::SetActiveProject(Project* project)
		{
			active_project_ = project;
			if (project != 0 ) {
				project->SetActive();
				project->mac_view()->main()->m_wndView._pSong=&project->song();
				project->mac_view()->main()->SetSong(&project->song());
				project->mac_view()->main()->m_wndSeq.SetProject(project);
			}
		}

		void ProjectData::FileNew()
		{
			assert(active_project_);
			if (active_project_->CheckUnsavedSong("New Song"))
			{
				active_project_->Clear();
			}
			active_project_->mac_view()->main()->StatusBarIdle();
		}

	}
}
