#pragma once

#include "Global.hpp"
#include <cassert>

namespace psycle {
	namespace host {

		class ProjectData;

		class Project
		{
		public:
			Project(ProjectData* parent, class PatternView* pat_view);
			~Project();

			Song& song() { assert(Global::_pSong); return *Global::_pSong; }
			class PatternView* pat_view();

			ProjectData* parent();

		private:
			PatternView* pat_view_;
			ProjectData* parent_;

		};
	}
}

