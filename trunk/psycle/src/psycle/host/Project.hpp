#pragma once

#include "Global.hpp"
#include <cassert>

namespace psycle {
	namespace host {

		class Project
		{
		public:
			Project(class PatternView* pat_view);
			~Project();

			Song& song() { assert(Global::_pSong); return *Global::_pSong; }
			class PatternView* pat_view();

		private:
			PatternView* pat_view_;

		};
	}
}

