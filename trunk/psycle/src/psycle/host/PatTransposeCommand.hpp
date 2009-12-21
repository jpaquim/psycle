#pragma once
#include "PatHelperCommand.hpp"
#include "configuration_options.hpp"

namespace psycle {
	namespace host {

		class PatTransposeCommand : public PatHelperCommand {
		public:
			PatTransposeCommand(class PatternView* pat_view, int trp);
			~PatTransposeCommand() {}

			virtual void Execute();

		private:
			int trp_;

		};

	}	// namespace host
}	// namespace psycle
