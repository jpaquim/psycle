#pragma once
#include "PatHelperCommand.hpp"
#include "configuration_options.hpp"

namespace psycle {
	namespace host {

		class DeleteCurrCommand : public PatHelperCommand {
		public:
			DeleteCurrCommand(class PatternView* pat_view);
			~DeleteCurrCommand() {}

			virtual void Execute();

		};

	}	// namespace host
}	// namespace psycle
