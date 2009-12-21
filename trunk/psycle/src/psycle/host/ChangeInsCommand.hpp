#pragma once
#include "PatHelperCommand.hpp"
#include "configuration_options.hpp"

namespace psycle {
	namespace host {

		class ChangeInsCommand : public PatHelperCommand {
		public:
			ChangeInsCommand(class PatternView* pat_view, int ins);
			~ChangeInsCommand() {}

			virtual void Execute();

		private:
			int ins_;

		};

	}	// namespace host
}	// namespace psycle