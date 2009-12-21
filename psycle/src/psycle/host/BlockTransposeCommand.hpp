#pragma once
#include "PatHelperCommand.hpp"
#include "configuration_options.hpp"

namespace psycle {
	namespace host {

		class BlockTransposeCommand : public PatHelperCommand {
		public:
			BlockTransposeCommand(class PatternView* pat_view, int trp);
			~BlockTransposeCommand() {}

			virtual void Execute();

		private:
			int trp_;

		};

	}	// namespace host
}	// namespace psycle