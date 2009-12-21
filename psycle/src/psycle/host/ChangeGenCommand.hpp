#pragma once
#include "PatHelperCommand.hpp"
#include "configuration_options.hpp"

namespace psycle {
	namespace host {

		class ChangeGenCommand : public PatHelperCommand {
		public:
			ChangeGenCommand(class PatternView* pat_view, int gen);
			~ChangeGenCommand() {}

			virtual void Execute();

		private:
			int gen_;

		};

	}	// namespace host
}	// namespace psycle