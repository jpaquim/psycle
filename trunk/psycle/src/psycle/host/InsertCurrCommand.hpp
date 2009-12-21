#pragma once
#include "PatHelperCommand.hpp"
#include "configuration_options.hpp"

namespace psycle {
	namespace host {

		class InsertCurrCommand : public PatHelperCommand {
		public:
			InsertCurrCommand(class PatternView* pat_view);
			~InsertCurrCommand() {}

			virtual void Execute();

		};

	}	// namespace host
}	// namespace psycle