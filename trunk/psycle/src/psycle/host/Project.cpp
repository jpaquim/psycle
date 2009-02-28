#include "project.hpp"

#include "PatternView.hpp"

namespace psycle {
	namespace host {

		Project::Project(PatternView* pat_view)
			: pat_view_(pat_view)
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

	}
}
