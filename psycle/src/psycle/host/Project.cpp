#include "project.hpp"

#include "PatternView.hpp"
#include "ProjectData.hpp"

namespace psycle {
	namespace host {

		Project::Project(ProjectData* parent, PatternView* pat_view)
			: parent_(parent), pat_view_(pat_view)
		{
			assert(pat_view_);
		}

		Project::~Project()
		{
		}

		PatternView* Project::pat_view()
		{
			return pat_view_;
		}

		ProjectData* Project::parent()
		{
			return parent_;
		}

	}
}
